/*******************************************************************************
*              (C), Copyright 2001, Marvell International Ltd.                 *
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
* @file prvTgfBrgVlanPortPushedTagUsePhysicalVlanId.c
*
* @brief Test to check whether the push tag VLAn Id is mapped to physical port or
*        ePort VLAN Id based on use_physical_port_vlan_id field status.
*
* @version   1
********************************************************************************
*/
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/pcl/cpssPcl.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgVlan.h>
/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/


/* ports */
#define PRV_TGF_RX_PORT_IDX_CNS       0
#define PRV_TGF_TX_PORT_IDX_CNS       1

/* port interface */
static CPSS_INTERFACE_INFO_STC      recvPortInterface;
static CPSS_INTERFACE_INFO_STC      sendPortInterface;

static TGF_VLAN_ID  prvTgfVlanId[] = {4/* Physical VLAN Id*/, 5 /* ePort VLAN Id*/};
/******************************* Test packets *********************************/
/* L2 part of packet  */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
     {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},            /* daMac */
     {0x00, 0x00, 0x00, 0x00, 0x12, 0x02}             /* saMac */
};

/* VLAN_TAG0 part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS, /* etherType */
    1, 0, 0    /* pri, cfi, vid */ /* vid will be initialized later */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* size of incoming packet */
#define PRV_TGF_INCOMING_PACKET_SIZE_CNS (  TGF_L2_HEADER_SIZE_CNS        \
                                            + sizeof(prvTgfPayloadDataArr))

/* size of outgoing packet */
#define PRV_TGF_OUTGOING_PACKET_SIZE_CNS ( TGF_L2_HEADER_SIZE_CNS        \
                                           + TGF_VLAN_TAG_SIZE_CNS    \
                                           + sizeof(prvTgfPayloadDataArr))
/* parts of incoming  packet */
static TGF_PACKET_PART_STC prvTgfIncomingPacketPartsArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* incoming packet packet info */
static TGF_PACKET_STC prvTgfIncomingPacketInfo = {
    PRV_TGF_INCOMING_PACKET_SIZE_CNS,                                    /* totalLen */
    sizeof(prvTgfIncomingPacketPartsArray) / sizeof(TGF_PACKET_PART_STC),/* numOfParts */
    prvTgfIncomingPacketPartsArray                                       /* partsArray */
};

/* parts of outgoing packet */
static TGF_PACKET_PART_STC prvTgfOutgoingPacketPartsArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};
/* outgoing packet info */
static TGF_PACKET_STC prvTgfOutGoingPacketInfo = {
    PRV_TGF_OUTGOING_PACKET_SIZE_CNS,                                    /* totalLen */
    sizeof(prvTgfOutgoingPacketPartsArray) / sizeof(TGF_PACKET_PART_STC),/* numOfParts */
    prvTgfOutgoingPacketPartsArray                                       /* partsArray */
};

/* default number of packets to send */
static GT_U32  prvTgfBurstCount = 1;

/* some original values saved to be restored at the end of the test. */
static struct
{
    CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_ENT   prvTgfSavedVlanCmd;
    GT_BOOL                                    prvTgfSavedUsePhysicalPortEnable;
    TGF_VLAN_ID                                prvTgfSavedTagValue;
    TGF_VLAN_ID                                prvTgfSavedPhysicalPortTagValue;
    GT_U32                                     portsArray[PRV_TGF_MAX_PORTS_NUM_CNS];
} prvTgfRestoreCfg;

/************************** PRIVATE DECLARATIONS ****************************/
/**
* @internal prvTgfCscdDsaFrwdSrcIdConfigSave function
* @endinternal
*
* @brief   Save configuration to be restored at the end of test
*/
static GT_VOID prvTgfBrgVlanPortPushedTagUsePhysicalVlanIdConfigSave
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* AUTODOC: save prvTgfPortsArray */
    cpssOsMemCpy(prvTgfRestoreCfg.portsArray, prvTgfPortsArray,
                 sizeof(prvTgfRestoreCfg.portsArray));

    /* Save Push Vlan Command */
    rc = cpssDxChBrgVlanPortPushVlanCommandGet(prvTgfDevNum,
                                               prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
                                               &prvTgfRestoreCfg.prvTgfSavedVlanCmd);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanPortPushVlanCommandSet: %d", prvTgfDevNum);

    /* Save use_physical_port_push_tag field value */
    rc = prvTgfBrgVlanPortPushedTagUsePhysicalEnableGet(prvTgfDevNum,
                                                        prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
                                                        &prvTgfRestoreCfg.prvTgfSavedUsePhysicalPortEnable);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortPushedTagUsePhysicalEnableSet: %d", prvTgfDevNum);

    /* Save pushTag VLAN-ID */
    rc = prvTgfBrgVlanPortPushedTagPhysicalValueGet(prvTgfDevNum,
                                                    prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
                                                    &prvTgfRestoreCfg.prvTgfSavedPhysicalPortTagValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortPushedTagPhysicalValueSet: %d", prvTgfDevNum);

    rc = cpssDxChBrgVlanPortPushedTagValueGet(prvTgfDevNum,
                                              prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
                                              &prvTgfRestoreCfg.prvTgfSavedTagValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanPortPushedTagValueSet: %d", prvTgfDevNum);
}

/**
* @internal prvTgfBrgVlanPortPushedTagUsePhysicalVlanIdConfigSet function
* @endinternal
*
* @brief   Setup configuration
*/
static GT_VOID prvTgfBrgVlanPortPushedTagUsePhysicalVlanIdConfigSet
(
    IN GT_BOOL     enable
)
{
    GT_STATUS                           rc;
    CPSS_PORTS_BMP_STC                  portsMembers;
    CPSS_PORTS_BMP_STC                  portsTagging;
    GT_BOOL                             isValid;
    PRV_TGF_BRG_VLAN_INFO_STC           vlanInfo;
    PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC  portsTaggingCmd;
    PRV_TGF_BRG_MAC_ENTRY_STC           macEntry;

    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Base Configuration =======\n");

    prvTgfBrgVlanPortPushedTagUsePhysicalVlanIdConfigSave();

    /* get flooding eVidx for vlan prvTgfVlanId */
    rc = prvTgfBrgVlanEntryRead(prvTgfDevNum, prvTgfVlanId[1], &portsMembers,
                          &portsTagging, &isValid, &vlanInfo, &portsTaggingCmd);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryRead");

    /* mark the ports to be added into vlan */
    CPSS_PORTS_BMP_PORT_SET_MAC(&portsMembers,
                                prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS]);

    /* mark the ports to be tagged */
    cpssOsMemCpy(&portsTagging, &portsMembers, sizeof(portsTagging));

    /* set tagging command for the ports */
    portsTaggingCmd.portsCmd[prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS]] =
                              PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E;

    /* AUTODOC: create VLAN as TAG with TAG0 tagged ports */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(prvTgfVlanId[1], PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfBrgDefVlanEntryWriteWithTaggingCmd");

    /* add a MAC entry */
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

    macEntry.key.entryType                  = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E;
    macEntry.key.key.macVlan.vlanId         = prvTgfVlanId[1];
    macEntry.dstInterface.type              = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum  = prvTgfDevNum;
    macEntry.dstInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS];
    macEntry.isStatic                       = GT_TRUE;
    macEntry.daCommand                      = PRV_TGF_PACKET_CMD_FORWARD_E;
    macEntry.saCommand                      = PRV_TGF_PACKET_CMD_FORWARD_E;
    macEntry.daRoute                        = GT_FALSE;
    macEntry.userDefined                    = 0;

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02, prvtgfVlanId and  port 1*/
    rc = prvTgfBrgFdbMacEntrySet(&macEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryWrite: %d", prvTgfDevNum);

    /*-------------------SETUP RX-PORT and TX-PORT----------------------*/

    /* Set Push Vlan Command */
    rc = cpssDxChBrgVlanPortPushVlanCommandSet(prvTgfDevNum,
                                               prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
                                               CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_PUSH_SINGLE_TAG_ON_PASSENGER_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanPortPushVlanCommandSet: %d", prvTgfDevNum);


    /* configure use_physical_port_push_tag field*/
    rc = prvTgfBrgVlanPortPushedTagUsePhysicalEnableSet(prvTgfDevNum,
                                                        prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
                                                        enable);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortPushedTagUsePhysicalEnableSet: %d", prvTgfDevNum);

    /* configure pushTag VLAN-ID */
    rc = prvTgfBrgVlanPortPushedTagPhysicalValueSet(prvTgfDevNum,
                                                    prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
                                                    prvTgfVlanId[0]);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortPushedTagPhysicalValueSet: %d", prvTgfDevNum);

    /* configure pushTag VLAN-ID */
    rc = cpssDxChBrgVlanPortPushedTagValueSet(prvTgfDevNum,
                                              prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
                                              prvTgfVlanId[1]);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanPortPushedTagValueSet: %d", prvTgfDevNum);

    if(enable == GT_TRUE)
    {
        prvTgfPacketVlanTagPart.vid = prvTgfVlanId[0];
    }
    else
    {
        prvTgfPacketVlanTagPart.vid = prvTgfVlanId[1];
    }

    /* set port for packet send */
    sendPortInterface.type             = CPSS_INTERFACE_PORT_E;
    sendPortInterface.devPort.hwDevNum = prvTgfDevNum;
    sendPortInterface.devPort.portNum  = prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS];

    /* set port for packet capture */
    recvPortInterface.type             = CPSS_INTERFACE_PORT_E;
    recvPortInterface.devPort.hwDevNum = prvTgfDevNum;
    recvPortInterface.devPort.portNum  = prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS];
}


/**
* @internal prvTgfBrgVlanPortPushedTagUsePhysicalVlanIdConfigRestore function
* @endinternal
*
* @brief   Restore configuration
*/
static GT_VOID prvTgfBrgVlanPortPushedTagUsePhysicalVlanIdConfigRestore
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* delete VLAN */
    rc = prvTgfBrgVlanEntryInvalidate(prvTgfVlanId[1]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate: %d %d",
                                 prvTgfDevNum, prvTgfVlanId[1]);

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* Deconfigure push tag command */
    rc = cpssDxChBrgVlanPortPushVlanCommandSet(prvTgfDevNum,
                                               prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
                                               prvTgfRestoreCfg.prvTgfSavedVlanCmd);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanPortPushVlanCommandSet: %d", prvTgfDevNum);

    /* configure use_physical_port_push_tag field*/
    rc = prvTgfBrgVlanPortPushedTagUsePhysicalEnableSet(prvTgfDevNum,
                                                        prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
                                                        prvTgfRestoreCfg.prvTgfSavedUsePhysicalPortEnable);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortPushedTagUsePhysicalEnableSet: %d", prvTgfDevNum);

    /* de-configure pushTag VLAN-ID */
    rc = cpssDxChBrgVlanPortPushedTagValueSet(prvTgfDevNum,
                                              prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
                                              prvTgfRestoreCfg.prvTgfSavedTagValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanPortPushedTagValueSet: %d", prvTgfDevNum);

    rc = prvTgfBrgVlanPortPushedTagPhysicalValueSet(prvTgfDevNum,
                                                    prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
                                                    prvTgfRestoreCfg.prvTgfSavedPhysicalPortTagValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortPushedTagPhysicalValueSet: %d", prvTgfDevNum);

    /* AUTODOC: clear FDB. FDB entries could be created implicitly. */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush");
}

/**
* @internal prvTgfBrgVlanPortPushedTagUsePhysicalVlanIdCheckOnPorts function
* @endinternal
*
* @brief   Check counters on the ports.
*
* @param[in] prvTgfPhysicalPortEnable - status of use physical port based push tag VLAN Id
*/
static GT_VOID prvTgfBrgVlanPortPushedTagUsePhysicalVlanIdCheckOnPorts
(
    IN GT_BOOL prvTgfPhysicalPortEnable
)
{
    GT_U32                  expRx;
    GT_U32                  expTx;
    GT_U32                  packetSize;
    GT_U32                  i;
    GT_STATUS               rc;
    GT_U32                  actualNumOfPackets;/*actual number of packet captured from the interface*/
    TGF_VFD_INFO_STC        vfdArray[1];
    GT_U32                  numTriggers = 0;

    /* AUTODOC: Check ethernet counters */
    for (i = 0; i < 2 ; ++i)
    {
        if (PRV_TGF_RX_PORT_IDX_CNS == i)
        {
            packetSize = PRV_TGF_INCOMING_PACKET_SIZE_CNS;
            expRx = expTx = 1;
        }
        else if(PRV_TGF_TX_PORT_IDX_CNS == i)
        {
            packetSize = PRV_TGF_OUTGOING_PACKET_SIZE_CNS;
            expRx = expTx = 1;
        }
        else
        {
            expRx = expTx = 0;
            packetSize = 0;
        }

        rc = prvTgfEthCountersCheck(prvTgfDevNum, prvTgfPortsArray[i],
                                    expRx, expTx, packetSize, prvTgfBurstCount);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfEthCountersCheck " \
                                     "Port=%02d, expected Rx=%02d, Tx=%02d\n",
                                     prvTgfPortsArray[i], expRx, expTx);
    }

    /* Configure vfdArray */
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS - 1;
    vfdArray[0].cycleCount = 1;

    if(prvTgfPhysicalPortEnable == GT_TRUE)
    {
        vfdArray[0].patternPtr[0] = (GT_U8)prvTgfVlanId[0];
    }
    else
    {
        vfdArray[0].patternPtr[0] = (GT_U8)prvTgfVlanId[1];
    }

    /* AUTODOC: check the captured packet */
    rc = tgfTrafficGeneratorPortTxEthCaptureCompare(
                                        &recvPortInterface,
                                        &prvTgfOutGoingPacketInfo,
                                        prvTgfBurstCount,/*numOfPackets*/
                                        1/* vfdNum */,
                                        vfdArray /* vfdArray */,
                                        NULL,
                                        0,
                                        &actualNumOfPackets,
                                        &numTriggers);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                "tgfTrafficGeneratorPortTxEthCaptureCompare: %d",
                                 recvPortInterface.devPort.portNum);
}

/**
* @internal prvTgfBrgVlanPortPushedTagUsePhysicalVlanIdTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
static GT_VOID prvTgfBrgVlanPortPushedTagUsePhysicalVlanIdTrafficGenerate
(
    IN GT_BOOL prvTgfPhysicalPortEnable
)
{
    GT_U32                          portIter;
    GT_STATUS                       rc;

    /* reset ethernet counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum,
                                    prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d",
                                     prvTgfPortsArray[portIter]);
    }

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "tgfTrafficTableRxPcktTblClear error");

    /* AUTODOC: setup a packet to be sent */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfIncomingPacketInfo,
                             prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth");

    /* Start capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&recvPortInterface,
                                                TGF_CAPTURE_MODE_MIRRORING_E,
                                                GT_TRUE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                           "tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d, %d",
                                 recvPortInterface.devPort.hwDevNum,
                                 recvPortInterface.devPort.portNum,
                                 GT_TRUE);
    /* send packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS]);

    /* wait for packets come to CPU */
    (void) tgfTrafficGeneratorRxInCpuNumWait(prvTgfBurstCount, 500, NULL);

    /* Stop capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&recvPortInterface,
                                                TGF_CAPTURE_MODE_MIRRORING_E,
                                                GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                           "tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d, %d",
                                 recvPortInterface.devPort.hwDevNum,
                                 recvPortInterface.devPort.portNum,
                                 GT_FALSE);
    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n",
                     recvPortInterface.devPort.portNum);

    prvTgfBrgVlanPortPushedTagUsePhysicalVlanIdCheckOnPorts(prvTgfPhysicalPortEnable);
}

/************************** PUBLIC DECLARATIONS *******************************/
/**
* @internal prvTgfBrgVlanPortPushedTagUsePhysicalVlanIdTest function
* @endinternal
*
* @brief    Check outgoing packet with a 4B VLAN Tag with VLAN Id
*           based on use_physical_port_vid field status
*/
GT_VOID prvTgfBrgVlanPortPushedTagUsePhysicalVlanIdTest
(
    GT_VOID
)
{
    prvTgfBrgVlanPortPushedTagUsePhysicalVlanIdConfigSet(GT_TRUE /* use_physical_port_vid field state */);
    prvTgfBrgVlanPortPushedTagUsePhysicalVlanIdTrafficGenerate(GT_TRUE  /* use_physical_port_vid field state */ );
    prvTgfBrgVlanPortPushedTagUsePhysicalVlanIdConfigRestore();

    prvTgfBrgVlanPortPushedTagUsePhysicalVlanIdConfigSet(GT_FALSE  /* use_physical_port_vid field state */);
    prvTgfBrgVlanPortPushedTagUsePhysicalVlanIdTrafficGenerate(GT_FALSE  /* use_physical_port_vid field state */);
    prvTgfBrgVlanPortPushedTagUsePhysicalVlanIdConfigRestore();
}

