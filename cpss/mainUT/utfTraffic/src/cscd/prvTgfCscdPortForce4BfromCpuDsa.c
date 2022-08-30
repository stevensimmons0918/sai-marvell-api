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
* @file prvTgfCscdPortForce4BfromCpuDsa.c
*
* @brief Test for forced 4B from_cpu DSA tag in outgoing packet (FORWARD command).
* The packet is sent:
* - from a network port to a cascade port
* Tx port is checked: it should or shouldn't contain a 4B from_cpu DSA tag in the packet
* depending on the port is configured to transmit 4B DSA tag.
*
*
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
#include <common/tgfCscdGen.h>
#include <common/tgfBridgeGen.h>
#include <cscd/prvTgfCscdDsaFrwdSrcId.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/


/* ports */
#define PRV_TGF_RX_PORT_IDX_CNS       0
#define PRV_TGF_TX_PORT_IDX_CNS       1

/* remote physical port number in the outgoing DSA tag */
#define PRV_TGF_REMOTE_PHYSICAL_PORT_IDX_CNS     4

/* remote physical device id in the outgoing DSA tag */
#define PRV_TGF_ORIG_REMOTE_DEV_CNS 16
static GT_U8 prvTgfRemoteDevNum = 16;
static GT_BOOL targetRemotedDevChanged = GT_FALSE;

/* DSA tag source-ID field */
static GT_U32 prvTgfSrcId = 1;

/* vlan used by test */
static TGF_VLAN_ID prvTgfVlanId = 5;

/******************************* Test packets *********************************/
/* L2 part of packet  */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {

     {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},            /* daMac */
     {0x00, 0x00, 0x00, 0x00, 0x00, 0x02}             /* saMac */

};

/* Byte skipped from comparison */
static GT_U32 byteNumMaskList[] = { 13 };

/* VLAN_TAG0 part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS, /* etherType */
    0, 0, 0    /* pri, cfi, vid */ /* vid will be initialized later */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[48] ={0xFF, 0xFE, 0xFD, 0xFC, 0xFB, 0xFA};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* size of packet that came from network port */
#define PRV_TGF_NETWORK_PACKET_SIZE_CNS (  TGF_L2_HEADER_SIZE_CNS        \
                                           + TGF_VLAN_TAG_SIZE_CNS    \
                                           + sizeof(prvTgfPayloadDataArr))


/* size of packet that came from cascade port */
#define PRV_TGF_CASCADE_PACKET_SIZE_CNS (  TGF_L2_HEADER_SIZE_CNS        \
                                           + TGF_DSA_TAG_SIZE_CNS - 4       \
                                           + TGF_VLAN_TAG_SIZE_CNS    \
                                           + sizeof(prvTgfPayloadDataArr))

static TGF_DSA_DSA_FROM_CPU_STC  prvTgfDsaInfo_fromCpu = {
/* struct */ /* dstInterface */
    {
        CPSS_INTERFACE_PORT_E,
        /* struct */
        {
            0,    /* hwDevNum */
            0     /* portNum */
        }, /* devPort */

        0,  /* trunkId */
        0,  /* vidx  */
        0,  /* vlanId */
        0,  /* hwDevNum */
        0,  /* fabricVidx */
        0  /* index */
    }, /* desInterface */

    0,  /*  tc  */
    CPSS_DP_GREEN_E,  /* dp  */
    GT_FALSE,  /* egrFilterEn */
    GT_FALSE,  /* cascadeControl  */
    GT_FALSE,  /*  egrFilterRegistered  */

    0,   /* srcId  */
    0,   /* srcHwDev  */

    {
        {
            GT_FALSE,                        /* excludeInterface;*/
            {
                CPSS_INTERFACE_PORT_E,
                /* struct */
                {
                    0,    /* hwDevNum */
                    0     /* portNum */
                }, /* devPort */

                0,  /* trunkId */
                0,  /* vidx  */
                0,  /* vlanId */
                0,  /* hwDevNum */
                0,  /* fabricVidx */
                0   /* index */
            }, /* desInterface */
            GT_FALSE,                        /* mirrorToAllCPUs;*/

            /* supported in extended, 4 words,  DSA tag */
            GT_FALSE,                     /*  excludedIsPhyPort; */
            PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E  /* srcIsTagged;*/

        },/*multiDest;*/

    },

    /* supported in extended, 4 words,  DSA tag */

    GT_TRUE,   /* isTrgPhyPortValid */
    0,         /* dstEport */
    0         /*  tag0TpidIndex */

};

/* DSA tag without command-specific part(dsaInfo) */
static TGF_PACKET_DSA_TAG_STC  prvTgfPacketDsaTagPart = {
    TGF_DSA_CMD_FROM_CPU_E, /*dsaCommand*/
    TGF_DSA_1_WORD_TYPE_E, /*dsaType*/

    /*TGF_DSA_DSA_COMMON_STC*/
    {
        0,        /*vpt*/
        0,        /*cfiBit*/
        0,        /*vid*/
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


/* parts of packet that came from network port */
static TGF_PACKET_PART_STC prvTgfNetworkPacketPartsArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* a network packet info */
static TGF_PACKET_STC prvTgfNetworkPacketInfo = {
    PRV_TGF_NETWORK_PACKET_SIZE_CNS,                                    /* totalLen */
    sizeof(prvTgfNetworkPacketPartsArray) / sizeof(TGF_PACKET_PART_STC),/* numOfParts */
    prvTgfNetworkPacketPartsArray                                       /* partsArray */
};

/* parts of packet that came from cascade port */
static TGF_PACKET_PART_STC prvTgfCascadePacketPartsArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_DSA_TAG_E,   &prvTgfPacketDsaTagPart},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};
/* a cascade packet info */
static TGF_PACKET_STC prvTgfCascadePacketInfo = {
    PRV_TGF_CASCADE_PACKET_SIZE_CNS,                                    /* totalLen */
    sizeof(prvTgfCascadePacketPartsArray) / sizeof(TGF_PACKET_PART_STC),/* numOfParts */
    prvTgfCascadePacketPartsArray                                       /* partsArray */
};


/* default number of packets to send */
static GT_U32  prvTgfBurstCount = 1;

/* some original values saved to be restored at the end of the test. */
static struct
{
    CPSS_CSCD_PORT_TYPE_ENT           prvTgfSavedRxCscdPortType;
    CPSS_CSCD_PORT_TYPE_ENT           prvTgfSavedTxCscdPortType;
    GT_BOOL                           prvTgfSavedRxSrcIdForceEnable;
    GT_BOOL                           prvTgfSavedTxSrcIdForceEnable;
    GT_HW_DEV_NUM                     remotePhysicalHwDevNum;
    GT_PHYSICAL_PORT_NUM              remotePhysicalPortNum;
    GT_U32                            prvTgfSavedRxDefSrcId;
    GT_BOOL                           portMapEnable;
    GT_BOOL                           portMapRestore;
    GT_U32                            portsArray[PRV_TGF_MAX_PORTS_NUM_CNS];
    GT_HW_DEV_NUM                     prvTgfSavedHwDevNum;
} prvTgfRestoreCfg;

/************************** PRIVATE DECLARATIONS ****************************/
/**
* @internal prvTgfCscdDsaFrwdSrcIdConfigSave function
* @endinternal
*
* @brief   Save configuration to be restored at the end of test
*/
static GT_VOID prvTgfCscdPortForce4BfromCpuDsaConfigSave
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* AUTODOC: save prvTgfPortsArray */
    cpssOsMemCpy(prvTgfRestoreCfg.portsArray, prvTgfPortsArray,
                 sizeof(prvTgfRestoreCfg.portsArray));

    /* save a configuration of a rx-port to be a cascade port */
    rc = prvTgfCscdPortTypeGet(prvTgfDevNum,
                               prvTgfRestoreCfg.portsArray[PRV_TGF_RX_PORT_IDX_CNS],
                               CPSS_PORT_DIRECTION_RX_E,
                               &prvTgfRestoreCfg.prvTgfSavedRxCscdPortType);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeGet");

    /* save a configuration of a tx-port to be a cascade port */
    rc = prvTgfCscdPortTypeGet(prvTgfDevNum,
                                   prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
                                   CPSS_PORT_DIRECTION_TX_E,
                                   &prvTgfRestoreCfg.prvTgfSavedTxCscdPortType);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeGet");

    /* save  default source id value of rx port */
    rc = prvTgfBrgSrcIdPortDefaultSrcIdGet(prvTgfDevNum,
                                      prvTgfRestoreCfg.portsArray[PRV_TGF_RX_PORT_IDX_CNS],
                                      &prvTgfRestoreCfg.prvTgfSavedRxDefSrcId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgSrcIdPortDefaultSrcIdGet");

    /* save a ForceSrcId state of rx port */
    rc = prvTgfBrgSrcIdPortSrcIdForceEnableGet(prvTgfDevNum,
                                      prvTgfRestoreCfg.portsArray[PRV_TGF_RX_PORT_IDX_CNS],
                                      &prvTgfRestoreCfg.prvTgfSavedRxSrcIdForceEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgSrcIdPortSrcIdForceEnableGet");

    /* save Hardware device number */
    rc = prvUtfHwDeviceNumberGet(prvTgfDevNum, &prvTgfRestoreCfg.prvTgfSavedHwDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvUtfHwDeviceNumberGet");


    /* force link up for all ports */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");


}

/**
* @internal prvTgfCscdPortForce4BfromCpuDsaConfigSet function
* @endinternal
*
* @brief   Setup configuration
*/
static GT_VOID prvTgfCscdPortForce4BfromCpuDsaConfigSet
(
    GT_VOID
)
{
    GT_STATUS                           rc;
    CPSS_PORTS_BMP_STC                  portsMembers;
    CPSS_PORTS_BMP_STC                  portsTagging;
    GT_BOOL                             isValid;
    PRV_TGF_BRG_VLAN_INFO_STC           vlanInfo;
    PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC  portsTaggingCmd;
    GT_U16    floodVidx               = 0;
    PRV_TGF_BRG_MAC_ENTRY_STC           macEntry;

    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Base Configuration =======\n");

    prvTgfCscdPortForce4BfromCpuDsaConfigSave();

    PRV_UTF_LOG2_MAC("We'll use vlanId=%d, srcId=%d ",
                     prvTgfVlanId, prvTgfSrcId);

    /* get flooding eVidx for vlan prvTgfVlanId */
    rc = prvTgfBrgVlanEntryRead(prvTgfDevNum, prvTgfVlanId, &portsMembers,
                          &portsTagging, &isValid, &vlanInfo, &portsTaggingCmd);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryRead");

    floodVidx = vlanInfo.floodVidx;

    /* mark the ports to be added into vlan */

    CPSS_PORTS_BMP_PORT_SET_MAC(&portsMembers,
                                prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS]);

    CPSS_PORTS_BMP_PORT_SET_MAC(&portsMembers,
                                prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS]);

    /* mark the ports to be tagged */
    cpssOsMemCpy(&portsTagging, &portsMembers, sizeof(portsTagging));


    /* set tagging command for the ports */

    portsTaggingCmd.portsCmd[prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS]] =
                              PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E;

    portsTaggingCmd.portsCmd[prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS]] =
                              PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E;


    /* AUTODOC: complete an initialization of packets structures  */

    prvTgfPacketVlanTagPart.vid                                         = prvTgfVlanId;
    prvTgfPacketVlanTagPart.cfi                                         = 0;

    prvTgfPacketDsaTagPart.commonParams.vid                              = prvTgfVlanId;
    prvTgfPacketDsaTagPart.commonParams.cfiBit                           = 0;
    prvTgfDsaInfo_fromCpu.dstInterface.vlanId                            = prvTgfVlanId;
    prvTgfDsaInfo_fromCpu.dstInterface.vidx                              = floodVidx;

    /*the device map table is not accessed for LOCAL DEVICE traffic*/
    if (prvTgfDevNum == prvTgfRemoteDevNum)
    {
        targetRemotedDevChanged = GT_TRUE;
        prvTgfRemoteDevNum = prvTgfDevNum - 1;
    }
    prvTgfDsaInfo_fromCpu.dstInterface.devPort.hwDevNum                  = prvTgfRemoteDevNum;
    prvTgfDsaInfo_fromCpu.dstInterface.devPort.portNum                   = PRV_TGF_REMOTE_PHYSICAL_PORT_IDX_CNS;

    prvTgfDsaInfo_fromCpu.tc                                             = 0;
    prvTgfDsaInfo_fromCpu.srcId                                          = prvTgfSrcId;
    prvTgfDsaInfo_fromCpu.srcHwDev                                       = prvTgfRestoreCfg.prvTgfSavedHwDevNum;

    prvTgfPacketDsaTagPart.dsaInfo.fromCpu.extDestInfo.devPort.dstIsTagged          = GT_TRUE;
    prvTgfPacketDsaTagPart.dsaInfo.fromCpu.extDestInfo.devPort.mailBoxToNeighborCPU = GT_FALSE;

    /* bind the DSA tag FROM_CPU part at run time because a union is used */
    prvTgfPacketDsaTagPart.dsaInfo.fromCpu = prvTgfDsaInfo_fromCpu;

    /* AUTODOC: create VLAN as TAG with TAG0 tagged ports */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(
         prvTgfVlanId, PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfBrgDefVlanEntryWriteWithTaggingCmd");

    /* add a MAC entry */
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

    macEntry.key.entryType                  = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E;
    macEntry.key.key.macVlan.vlanId         = prvTgfVlanId;
    macEntry.dstInterface.type              = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum  = prvTgfDevNum;
    macEntry.dstInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS];
    macEntry.isStatic                       = GT_TRUE;
    macEntry.daCommand                      = PRV_TGF_PACKET_CMD_FORWARD_E;
    macEntry.saCommand                      = PRV_TGF_PACKET_CMD_FORWARD_E;
    macEntry.daRoute                        = GT_FALSE;
    macEntry.sourceId                       = prvTgfSrcId;
    macEntry.userDefined                    = 0;
    macEntry.daQosIndex                     = 0;
    macEntry.saQosIndex                     = 0;
    macEntry.daSecurityLevel                = 0;
    macEntry.saSecurityLevel                = 0;
    macEntry.appSpecificCpuCode             = GT_FALSE;
    macEntry.mirrorToRxAnalyzerPortEn       = GT_FALSE;
    macEntry.fdbRoutingInfo.qosProfileMarkingEnable = GT_FALSE;

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02, prvtgfVlanId and  port 1*/
    rc = prvTgfBrgFdbMacEntrySet(&macEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryWrite: %d", prvTgfDevNum);

    /*-------------------SETUP RX-PORT and TX-PORT  SOURCE-ID VALUE----------------------*/

    /* AUTODOC: setup a source id that will be assigned to incoming packets */
    rc = prvTgfBrgSrcIdPortDefaultSrcIdSet(prvTgfDevNum,
                        prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS], prvTgfDsaInfo_fromCpu.srcId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgSrcIdPortDefaultSrcIdSet");

    /* Enable ForceSrcId on rx port to avoid override of srcId by FDB */
    rc = prvTgfBrgSrcIdPortSrcIdForceEnableSet(prvTgfDevNum,
                            prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgSrcIdPortSrcIdForceEnableSet");

    /* AUTODOC: add tx port to source-id group prvTgfSrcId */
    rc = prvTgfBrgSrcIdGroupPortAdd(prvTgfDevNum, prvTgfDsaInfo_fromCpu.srcId,
                                    prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgSrcIdGroupPortAdd(tx)");

    /* AUTODOC: setup rx port to be a network port */

    rc = prvTgfCscdPortTypeSet(prvTgfDevNum, CPSS_PORT_DIRECTION_RX_E,
                                   prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS],
                                   CPSS_CSCD_PORT_NETWORK_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet");

    /* AUTODOC: setup tx port to be a cascade port */

    rc = prvTgfCscdPortTypeSet(prvTgfDevNum, CPSS_PORT_DIRECTION_TX_E,
                                   prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
                                   CPSS_CSCD_PORT_DSA_MODE_1_WORD_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet");

    rc = prvTgfCscdCentralizedChassisRemotePhysicalPortMapEnableGet(prvTgfDevNum,
                                                                    prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
                                                                    &(prvTgfRestoreCfg.portMapEnable));
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCscdCentralizedChassisRemotePhysicalPortMapEnableGet: %d", prvTgfDevNum);


    rc = prvTgfCscdCentralizedChassisRemotePhysicalPortMapEnableSet(prvTgfDevNum,
                                                                    prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
                                                                    GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCscdCentralizedChassisRemotePhysicalPortMapEnableSet: %d", prvTgfDevNum);


    rc = prvTgfCscdCentralizedChassisRemotePhysicalPortMapGet(prvTgfDevNum,
                                                              prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
                                                              &(prvTgfRestoreCfg.remotePhysicalHwDevNum),
                                                              &(prvTgfRestoreCfg.remotePhysicalPortNum));
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCscdCentralizedChassisRemotePhysicalPortMapGet: %d", prvTgfDevNum);

    rc = prvTgfCscdCentralizedChassisRemotePhysicalPortMapSet(prvTgfDevNum,
                                                              prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
                                                              prvTgfRemoteDevNum,
                                                              PRV_TGF_REMOTE_PHYSICAL_PORT_IDX_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCscdCentralizedChassisRemotePhysicalPortMapSet: %d", prvTgfDevNum);


    rc = prvTgfCscdPortForce4BfromCpuDsaEnableGet(prvTgfDevNum,
                                                  prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
                                                  &(prvTgfRestoreCfg.portMapRestore));
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCscdPortForce4BfromCpuDsaEnableGet: %d", prvTgfDevNum);


    /* enabling the Forced 4B from_cpu dsa tag*/
    rc = prvTgfCscdPortForce4BfromCpuDsaEnableSet(prvTgfDevNum,
                                                  prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
                                                  GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCscdPortForce4BfromCpuDsaEnableSet: %d", prvTgfDevNum);


}


/**
* @internal prvTgfCscdPortForce4BfromCpuDsaConfigRestore function
* @endinternal
*
* @brief   Restore configuration
*/
static GT_VOID prvTgfCscdPortForce4BfromCpuDsaConfigRestore
(
    GT_VOID
)
{
    CPSS_INTERFACE_INFO_STC iface;
    GT_STATUS rc;

    cpssOsMemSet(&iface, 0, sizeof(iface));

    /* delete VLAN */
    rc = prvTgfBrgVlanEntryInvalidate(prvTgfVlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, prvTgfVlanId);


    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");


    /* AUTODOC: disable packet capturing on tx port */
    iface.type              = CPSS_INTERFACE_PORT_E;
    iface.devPort.hwDevNum  = prvTgfDevNum;
    iface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS];

    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&iface,
                                              TGF_CAPTURE_MODE_PCL_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "tgfTrafficGeneratorPortTxEthCaptureSet: %d",
                                 iface.devPort.portNum);

    /* AUTODOC: restore a cascade configuration of the rx-port */
    rc = prvTgfCscdPortTypeSet(prvTgfDevNum,
                               CPSS_PORT_DIRECTION_RX_E,
                               prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS],
                               prvTgfRestoreCfg.prvTgfSavedRxCscdPortType);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet");


    /* AUTODOC: restore a cascade configuration of the tx-port */
    rc = prvTgfCscdPortTypeSet(prvTgfDevNum,
                                   CPSS_PORT_DIRECTION_TX_E,
                                   prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
                                   prvTgfRestoreCfg.prvTgfSavedTxCscdPortType);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet");

    rc = prvTgfCscdCentralizedChassisRemotePhysicalPortMapEnableSet(prvTgfDevNum,
                                                                    prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
                                                                    prvTgfRestoreCfg.portMapEnable);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCscdCentralizedChassisRemotePhysicalPortMapEnableSet: %d", prvTgfDevNum);

    rc = prvTgfCscdCentralizedChassisRemotePhysicalPortMapSet(prvTgfDevNum,
                                                              prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
                                                              prvTgfRestoreCfg.remotePhysicalHwDevNum,
                                                              prvTgfRestoreCfg.remotePhysicalPortNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCscdCentralizedChassisRemotePhysicalPortMapSet: %d", prvTgfDevNum);


    rc = prvTgfCscdPortForce4BfromCpuDsaEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
                                                  prvTgfRestoreCfg.portMapRestore);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCscdPortForce4BfromCpuDsaEnableSet: %d", prvTgfDevNum);

    /* AUTODOC: restore default source id value of rx port */
    rc = prvTgfBrgSrcIdPortDefaultSrcIdSet(prvTgfDevNum,
                                      prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS],
                                      prvTgfRestoreCfg.prvTgfSavedRxDefSrcId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgSrcIdPortDefaultSrcIdSet");

    /* AUTODOC: restore a ForceSrcId state of rx port */
    rc = prvTgfBrgSrcIdPortSrcIdForceEnableSet(prvTgfDevNum,
                                      prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS],
                                      prvTgfRestoreCfg.prvTgfSavedRxSrcIdForceEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgSrcIdPortSrcIdForceEnableGet");

    /* AUTODOC: clear FDB. FDB entries could be created implicitly. */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush");

    if (targetRemotedDevChanged)
    {
        prvTgfRemoteDevNum = PRV_TGF_ORIG_REMOTE_DEV_CNS;
    }

}

/**
* @internal prvTgfCscdPortForce4BfromCpuDsaSendPacket function
* @endinternal
*
* @brief   Send the packet
*/
static GT_VOID prvTgfCscdPortForce4BfromCpuDsaSendPacket
(
    GT_VOID
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


    rc = prvTgfStartTransmitingEth(prvTgfDevNum,
                                   prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "StartTransmitting: %d\n",
                                 prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS]);
}


/**
* @internal prvTgfCscdPortForce4BfromCpuDsaCheckOnPorts function
* @endinternal
*
* @brief   Check counters on the ports.
*
* @param[in] isExpected               - whether the packet should be found on the tx port.
* @param[in] isSentWithDsa            - whether the packet setn with DSA-tag.
* @param[in] isExpWithDsa             - whether the packet should be received with DSA-tag.
*                                       None
*/
static GT_VOID prvTgfCscdPortForce4BfromCpuDsaCheckOnPorts
(
    IN GT_BOOL isExpected,
    IN GT_BOOL isSentWithDsa,
    IN GT_BOOL isExpWithDsa
)
{
    CPSS_INTERFACE_INFO_STC iface;
    TGF_PACKET_STC          *packetInfoPtr = NULL;
    GT_U32                  expRx;
    GT_U32                  expTx;
    GT_U32                  packetSize;
    GT_U32                  actualNumOfPackets;
    GT_U32                  i;
    GT_STATUS               rc;

    cpssOsMemSet(&iface, 0, sizeof(iface));

    if (GT_TRUE == isExpected)
    {
        /* wait the packets come to CPU */
        tgfTrafficGeneratorRxInCpuNumWait(prvTgfBurstCount, 500, NULL);
    }

    /* AUTODOC: Check ethernet counters */
    for (i = 0; i < prvTgfPortsNum; ++i)
    {
        if (PRV_TGF_RX_PORT_IDX_CNS == i)
        {
            packetSize = (GT_TRUE == isSentWithDsa) ?
                                     PRV_TGF_CASCADE_PACKET_SIZE_CNS
                                     : PRV_TGF_NETWORK_PACKET_SIZE_CNS;
            expRx = expTx = 1;

        }
        else if ( GT_TRUE == isExpected && PRV_TGF_TX_PORT_IDX_CNS == i)
        {
            packetSize = (GT_TRUE == isExpWithDsa) ?
                                     PRV_TGF_CASCADE_PACKET_SIZE_CNS
                                     : PRV_TGF_NETWORK_PACKET_SIZE_CNS;
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

    /* AUTODOC: Check egress packet bytes are 'as expected' */
    if (GT_TRUE == isExpected)
    {
        iface.type              = CPSS_INTERFACE_PORT_E;
        iface.devPort.hwDevNum  = prvTgfDevNum;
        iface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS];

        /* AUTODOC: check the captured packet */
        packetInfoPtr = (GT_TRUE == isExpWithDsa) ?
                             &prvTgfCascadePacketInfo : &prvTgfNetworkPacketInfo;

        rc = tgfTrafficGeneratorPortTxEthCaptureCompare(
                                      &iface,
                                      packetInfoPtr,
                                      prvTgfBurstCount,/*numOfPackets*/
                                      0/* vfdNum */,
                                      NULL /* vfdArray */,
                                      byteNumMaskList, /* bytesNum's skip list */
                                      sizeof(byteNumMaskList)/sizeof(byteNumMaskList[0]),    /* length of skip list */
                                      &actualNumOfPackets,
                                      NULL/* onFirstPacketNumTriggersBmpPtr */);
        rc = rc == GT_NO_MORE ? GT_OK : rc;
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                     "tgfTrafficGeneratorPortTxEthTriggerCountersGet:"
                                     " port = %d, rc = 0x%02X\n", iface.devPort.portNum, rc);

        UTF_VERIFY_EQUAL3_STRING_MAC(prvTgfBurstCount, actualNumOfPackets,
                   "Triggers number: port #%d, expected - %d, received - %d\n",
                   iface.devPort.portNum, prvTgfBurstCount, actualNumOfPackets);

    }
}


/**
* @internal prvTgfCscdPortForce4BfromCpuDsaTwoAndCheck function
* @endinternal
*
* @brief   Add/remove the tx port from source-id group, send 2 packet and check.
*
* @param[in] isSentWithDsa            - packet is sent with(GT_TRUE) or without(GT_FALSE)
*                                      DSA tag
* @param[in] isExpWithDsa             - packet is expected with(GT_TRUE) or without(GT_FALSE)
*                                      DSA tag
*                                       None
*/
static GT_VOID prvTgfCscdPortForce4BfromCpuDsaSendTwoAndCheck
(
    IN GT_BOOL isSentWithDsa,
    IN GT_BOOL isExpWithDsa
)
{
    GT_STATUS rc;

    /* AUTODOC: setup a packet to be sent (with or without DSA tag) */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum,
                   (GT_TRUE == isSentWithDsa ? &prvTgfCascadePacketInfo
                                             : &prvTgfNetworkPacketInfo),
                   prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth");


    /* AUTODOC: send the packet and check the tx port */


    prvTgfCscdPortForce4BfromCpuDsaSendPacket();

    prvTgfCscdPortForce4BfromCpuDsaCheckOnPorts(GT_TRUE, isSentWithDsa, isExpWithDsa);

}

/**
* @internal prvTgfCscdPortForce4BfromCpuDsaTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
static GT_VOID prvTgfCscdPortForce4BfromCpuDsaTrafficGenerate
(
    GT_VOID
)
{
    CPSS_INTERFACE_INFO_STC iface;
    GT_STATUS               rc;

    cpssOsMemSet(&iface, 0, sizeof(iface));

    /* AUTODOC: generate traffic */

    /* AUTODOC: enable packet capturing on tx port. */
    iface.type              = CPSS_INTERFACE_PORT_E;
    iface.devPort.hwDevNum  = prvTgfDevNum;
    iface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS];

    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&iface,
                                                 TGF_CAPTURE_MODE_PCL_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "tgfTrafficGeneratorPortTxEthCaptureSet");

    /**************************************************************************/
    /* AUTODOC: CASE 1. From network port to cascade port */

    PRV_UTF_LOG0_MAC("\nCASE 1. From a network port to a cascade port\n");

    /* AUTODOC: setup a packet without DSA tag to be sent */
    prvTgfCscdPortForce4BfromCpuDsaSendTwoAndCheck(GT_FALSE , GT_TRUE);

}

/************************** PUBLIC DECLARATIONS *******************************/
/**
* @internal prvTgfCscdPortForce4BfromCpuDsaTest function
* @endinternal
*
* @brief    Check outgoing packet with a 4B from_cpu DSA tag with command FORWARD
*/GT_VOID prvTgfCscdPortForce4BfromCpuDsaTest
(
    GT_VOID
)
{
    prvTgfCscdPortForce4BfromCpuDsaConfigSet();
    prvTgfCscdPortForce4BfromCpuDsaTrafficGenerate();
    prvTgfCscdPortForce4BfromCpuDsaConfigRestore();
}



