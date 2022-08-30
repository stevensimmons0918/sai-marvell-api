/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvTgfBrgNestedVlanPerTargetPort.c
*
* DESCRIPTION: Nested Vlan per ePort enhanced UT
*
*
* FILE REVISION NUMBER:
*       $Revision: 0 $
*
*******************************************************************************/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/bridge/cpssGenBrgVlanTypes.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfIpGen.h>
#include <common/tgfNstGen.h>
#include <common/tgfTrunkGen.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgStp.h>

#include <bridge/prvTgfBrgNestedVlanPerTargetPort.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgNestVlan.h>
/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* default VLAN0 Id */
#define PRV_TGF_VLANID_CNS  5

/* 4 bytes extra of TPID*/
#define TGF_TPID_LEN_CNS                 4

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS         0

/* port number to send traffic to */
#define PRV_TGF_EGRESS_PORT_IDX_CNS        1

/* count of packets to send */
#define PRV_TGF_SEND_COUNT_CNS 1

/* number of ports in vlan */
static GT_U32 prvTgfNestedVlanTestPortsNum = 2;

/* default number of packets to send */
static GT_U32  prvTgfBurstCount   = 1;

/* port bitmap VLAN members */
static CPSS_PORTS_BMP_STC localPortsVlanMembers = {{0, 0}};

/* original Vlan Nested Mode per port */
static GT_BOOL originalVlanNestedMode;

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x02}                /* srcMac */
};

/* First VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag0Part = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_CNS                            /* pri, cfi, VlanId */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
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


/**
* @internal prvTgfBrgNestedVlanPerTargetPortVlanConfig function
* @endinternal
*
* @brief   Set VLAN entry with recieved port isolation command.
*
* @param[in] vlanId                   -  to be configured
* @param[in] portIsolationCmd         - port isolation command
* @param[in] portsMembers             - VLAN's ports Members
*                                       None
*/
static GT_VOID prvTgfBrgNestedVlanPerTargetPortVlanConfig
(
    IN GT_U16                               vlanId,
    IN CPSS_PORTS_BMP_STC                   portsMembers
)
{
    GT_STATUS                   rc = GT_OK;
    CPSS_PORTS_BMP_STC          portsTagging;
    PRV_TGF_BRG_VLAN_INFO_STC   vlanInfo;
    GT_U32                      portIter  = 0;
    PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd;

    /* clear entry */
    cpssOsMemSet(&vlanInfo, 0, sizeof(vlanInfo));
    cpssOsMemSet(&portsTaggingCmd, 0, sizeof(PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC));
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTagging);

    /* set vlan entry */
    vlanInfo.unkSrcAddrSecBreach  = GT_FALSE;
    vlanInfo.unregNonIpMcastCmd   = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unregIpv4McastCmd    = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unregIpv6McastCmd    = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unkUcastCmd          = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unregIpv4BcastCmd    = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unregNonIpv4BcastCmd = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.ipv4IgmpToCpuEn      = GT_FALSE;
    vlanInfo.mirrToRxAnalyzerEn   = GT_FALSE;
    vlanInfo.ipv6IcmpToCpuEn      = GT_FALSE;
    vlanInfo.ipCtrlToCpuEn        = PRV_TGF_BRG_IP_CTRL_NONE_E;
    vlanInfo.ipv4IpmBrgMode       = CPSS_BRG_IPM_SGV_E;
    vlanInfo.ipv6IpmBrgMode       = CPSS_BRG_IPM_SGV_E;
    vlanInfo.ipv4IpmBrgEn         = GT_FALSE;
    vlanInfo.ipv6IpmBrgEn         = GT_FALSE;
    vlanInfo.ipv6SiteIdMode       = CPSS_IP_SITE_ID_INTERNAL_E;
    vlanInfo.ipv4UcastRouteEn     = GT_FALSE;
    vlanInfo.ipv4McastRouteEn     = GT_FALSE;
    vlanInfo.ipv6UcastRouteEn     = GT_FALSE;
    vlanInfo.ipv6McastRouteEn     = GT_FALSE;
    vlanInfo.stgId                = 0;
    vlanInfo.autoLearnDisable     = GT_TRUE;/* working in controlled learning */
    vlanInfo.naMsgToCpuEn         = GT_TRUE;/* working in controlled learning */
    vlanInfo.mruIdx               = 0;
    vlanInfo.bcastUdpTrapMirrEn   = GT_FALSE;
    vlanInfo.floodVidx            = 0xFFF;
    vlanInfo.floodVidxMode        = PRV_TGF_BRG_VLAN_FLOOD_VIDX_MODE_ALL_FLOODED_TRAFFIC_E;
    vlanInfo.ucastLocalSwitchingEn = GT_FALSE;
    vlanInfo.mcastLocalSwitchingEn = GT_FALSE;

    /* AUTODOC: ports[0,1] are VLAN Members with TAG0 */
    for (portIter = 0; portIter < prvTgfNestedVlanTestPortsNum; portIter++)
    {
		portsTaggingCmd.portsCmd[prvTgfPortsArray[portIter]] = PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E;
    }

    /* set vlan entry */
    rc = prvTgfBrgVlanEntryWrite(prvTgfDevNum,
                                 vlanId,
                                 &portsMembers,
                                 &portsTagging,
                                 &vlanInfo,
                                 &portsTaggingCmd);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryWrite: %d", vlanId);
}


/**
* @internal prvTgfBrgNestedVlanPerTargetPortConfigurationSet function
* @endinternal
*
* @brief
*/
GT_VOID prvTgfBrgNestedVlanPerTargetPortConfigurationSet
(
    GT_VOID
)
{
    GT_U32                  portIter = 0;

    /* ports prvTgfPortsArray[0,1] are VLAN Members */
    cpssOsMemSet(&localPortsVlanMembers, 0, sizeof(localPortsVlanMembers));
    for (portIter = 0; portIter < prvTgfNestedVlanTestPortsNum; portIter++)
    {
        CPSS_PORTS_BMP_PORT_SET_MAC(&localPortsVlanMembers,prvTgfPortsArray[portIter]);
    }

    /* create VLAN 5 with tag0*/
    prvTgfBrgNestedVlanPerTargetPortVlanConfig(PRV_TGF_VLANID_CNS, localPortsVlanMembers);

}




/**
* @internal prvTgfBrgVlanManipulationTestPacketSend function
* @endinternal
*
* @brief   Function sends packet, performs trace and check expected results.
*
* @param[in] portInterfacePtr         - (pointer to) port interface (port only is legal param)
* @param[in] packetInfoPtr            - (pointer to) the packet info
* @param[in] vfdNum                   - VFD number
* @param[in] vfdArray[]               - VFD array with expected results
* @param[in] srcDstIndex              - index in Src and Dst MAC address array
*                                       None
*/

static GT_VOID prvTgfBrgNestedVlanPerTargetPortPacketSend
(
    IN CPSS_INTERFACE_INFO_STC  *portInterfacePtr,
    IN TGF_PACKET_STC          	*packetInfoPtr,
    IN TGF_VFD_INFO_STC         vfdCheck

)
{
    GT_STATUS rc          = GT_OK;
    GT_U32    portIter    = 0;
    GT_U32    numTriggers = 0;
    GT_U8     vfdNum_new = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;
    TGF_VFD_INFO_STC *currentVfdPtr;

    vfdNum_new = 2;
    currentVfdPtr = &vfdCheck;

    /* AUTODOC: GENERATE TRAFFIC: */
    /* reset counters */
    for (portIter = 0; portIter < prvTgfNestedVlanTestPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: send packet from port 1 with: */
    /* setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, packetInfoPtr, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                                 prvTgfDevNum, prvTgfBurstCount, 0, NULL);

    /* send Packet from port portNum */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum,  prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    for (portIter = 0; portIter < prvTgfNestedVlanTestPortsNum; portIter++)
    {
        /* AUTODOC: read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        /* AUTODOC: check counters */
        UTF_VERIFY_EQUAL0_STRING_MAC((PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS) * prvTgfBurstCount, portCntrs.goodOctetsSent.l[0],
                                     "get another goodOctetsSent counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected");
    }

    /* AUTODOC: reset counters */
    for (portIter = 0; portIter < prvTgfNestedVlanTestPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }


    /* AUTODOC: Enable capturing of egress port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(portInterfacePtr,
                                                TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "tgfTrafficGeneratorPortTxEthCaptureSet: %d", prvTgfDevNum);

    /* AUTODOC: Save Nested Vlan mode on egress port*/
    rc = cpssDxChBrgNestVlanPortTargetEnableGet(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_EGRESS_PORT_IDX_CNS], &originalVlanNestedMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChBrgNestVlanPortTargetEnableGet: %d", prvTgfPortsArray[PRV_TGF_EGRESS_PORT_IDX_CNS]);


    /* AUTODOC: Enable Nested Vlan on egress port*/
    rc = cpssDxChBrgNestVlanPortTargetEnableSet(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_EGRESS_PORT_IDX_CNS],GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChBrgNestVlanPortTargetEnableSet: %d", prvTgfPortsArray[PRV_TGF_EGRESS_PORT_IDX_CNS]);

    /* AUTODOC: send Packet from port 1 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum,  prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    for (portIter = 0; portIter < prvTgfNestedVlanTestPortsNum; portIter++)
    {
        /* AUTODOC: read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            /* AUTODOC: check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC((PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS) * prvTgfBurstCount, portCntrs.goodOctetsRcv.l[0],
                                         "get another goodOctetsRcv counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            continue;
        }

        /* AUTODOC: check Tx counters */
        UTF_VERIFY_EQUAL0_STRING_MAC((PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS + TGF_TPID_LEN_CNS) * prvTgfBurstCount, portCntrs.goodOctetsSent.l[0],
                                     "get another goodOctetsSent counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected");
    }

	/* print captured packets and check TriggerCounters */
    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(portInterfacePtr, vfdNum_new, currentVfdPtr, &numTriggers);
    rc = rc == GT_NO_MORE ? GT_OK : rc;
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,"tgfTrafficGeneratorPortTxEthTriggerCountersGet:"
                                 " port = %d, rc = 0x%02X\n", portInterfacePtr->devPort.portNum, rc);

    /* AUTODOC: check if captured packet has the same pattern as vfdArray */
    UTF_VERIFY_EQUAL0_STRING_MAC((1 << vfdNum_new) - 1, numTriggers, "Packet VLAN TAG0/1 pattern is wrong");

    /* AUTODOC: reset counters */
    for (portIter = 0; portIter < prvTgfNestedVlanTestPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: Disable capturing of egress port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(portInterfacePtr,
                                                TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "tgfTrafficGeneratorPortTxEthCaptureSet: %d", prvTgfDevNum);


}



/**
* @internal prvTgfBrgNestedVlanPerTargetPortTrafficGenerate function
* @endinternal
*
*/
GT_VOID prvTgfBrgNestedVlanPerTargetPortTrafficGenerate
(
    GT_VOID
)
{
    CPSS_INTERFACE_INFO_STC portInterface;
    TGF_VFD_INFO_STC        vfdCheck;

    /* AUTODOC: set port for packet capture */
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGRESS_PORT_IDX_CNS];

    /* AUTODOC: setup packet with: */
    /* AUTODOC: TAG0: ethertype = 0x8100, vlan = 5 */

    /* AUTODOC: set VFD array of expected fields value in received packet*/
    vfdCheck.cycleCount = 4;
    vfdCheck.mode = TGF_VFD_MODE_STATIC_E;
    vfdCheck.modeExtraInfo = 0;
    vfdCheck.offset = 16;
    vfdCheck.patternPtr[0] = 0x81;
    vfdCheck.patternPtr[1] = 0x00;
    vfdCheck.patternPtr[2] = 0x00;
    vfdCheck.patternPtr[3] = 0x05;

    /* send packet */
    prvTgfBrgNestedVlanPerTargetPortPacketSend(&portInterface, &prvTgfPacketInfo, vfdCheck);
}




/**
* @internal prvTgfBrgNestedVlanPerTargetPortConfigurationRestore function
* @endinternal
*
* @brief
*/
GT_VOID prvTgfBrgNestedVlanPerTargetPortConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS               rc = GT_OK;

    /* AUTODOC: Restore Nested Vlan on egress port*/
    rc = cpssDxChBrgNestVlanPortTargetEnableSet(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_EGRESS_PORT_IDX_CNS],originalVlanNestedMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChBrgNestVlanPortTargetEnableSet: %d", prvTgfPortsArray[PRV_TGF_EGRESS_PORT_IDX_CNS]);

    /* AUTODOC: reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    /* AUTODOC: invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"prvTgfBrgDefVlanEntryInvalidate: %d", PRV_TGF_VLANID_CNS);

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d", GT_TRUE);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");
}
