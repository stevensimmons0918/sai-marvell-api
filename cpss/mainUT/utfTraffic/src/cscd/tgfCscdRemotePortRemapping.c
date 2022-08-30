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
* @file tgfCscdRemotePortRemapping.c
*
* @brief CPSS Cascade remote port remapping
*
* @version   5
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
#include <trafficEngine/private/prvTgfTrafficParser.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfCscdGen.h>
#include <cscd/prvTgfCscd.h>

#include <cscd/tgfCscdRemotePortRemapping.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* default VLAN0 Id */
#define PRV_TGF_VLANID_CNS                 5

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS             4

/* burst count */
#define PRV_TGF_BURST_COUNT_CNS            1

#define PRV_TGF_SEND_PORT_IDX_CNS          0
/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_NUM_CNS   prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]

/* dsa device */
#define PRV_TGF_DSA_DEV_CNS                2

/* dsa port */
#define PRV_TGF_DSA_PORT_CNS               3

/* physical port base  */
#define PRV_TGF_PHYSICAL_PORT_BASE_CNS    10

/* the number of bits from the DSA <source device> used for the mapping */
#define PRV_TGF_SRC_DEV_LSB_AMOUNT_CNS     0

/* the number of bits from the DSA <source port> used for the mapping */
#define PRV_TGF_SRC_PORT_LSB_AMOUNT_CNS    5


/* port number fdb learning expected */

/* Source Physical Port = My Physical Port <Physical Port Base> +
                            DSA <SrcPort1>[p-1:0] +
                            DSA <SrcDev>  [d-1:0]<<p
Where
    p = Global <Number of DSA Source Port Bits>
    d = Global <Number of DSA Source Device Bits>
*/
#define PRV_TGF_FDB_PORT_NUM_CNS   \
               PRV_TGF_PHYSICAL_PORT_BASE_CNS + PRV_TGF_DSA_PORT_CNS


/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},   /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x02}    /* srcMac */
};

static TGF_PACKET_DSA_TAG_STC  prvTgfPacketDsaTagPart = {
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

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] =
{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,
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
    {TGF_PACKET_PART_DSA_TAG_E,  &prvTgfPacketDsaTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};


/******************************************************/

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_DSA_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfo =
{
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};

/*************************** Restore config ***********************************/

/* parameters that is needed to be restored */
static struct
{
    CPSS_CSCD_PORT_TYPE_ENT                                  portType;
    PRV_TGF_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_ENT   portAssignMode;
    GT_U32                                                   srcDevLsbAmount;
    GT_U32                                                   srcPortLsbAmount;
    GT_U32                                                   phyPortBase;
    GT_BOOL                                                  assignSrcDevEnable;
    GT_U32                                                   portsArray[PRV_TGF_MAX_PORTS_NUM_CNS];
} prvTgfRestoreCfg;


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal tgfCscdRemotePortRemappingConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         configure Port 0 as DSA port,
*         set <Number Of DSA Source Port Bits>  = 5;
*         <Number Of DSA Source Device Bits> = 0;
*         <Physical Port Base> = 10;
*/
GT_VOID tgfCscdRemotePortRemappingConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                 rc;
    GT_U32                    portIter;

    /* AUTODOC: save prvTgfPortsArray */
    cpssOsMemCpy(prvTgfRestoreCfg.portsArray, prvTgfPortsArray,
                 sizeof(prvTgfRestoreCfg.portsArray));

    /* AUTODOC: re-arrange prvTgfPortsArray to ensure some ports (will be
     * configured cascade) are not remote ports */
    if (UTF_CPSS_DXCH_XCAT3X_CHECK_MAC(prvTgfDevNum))
    {
        rc = prvTgfDefPortsArrange(GT_FALSE, PRV_TGF_SEND_PORT_IDX_CNS, -1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortsArray rearrangement can't be done");
    }

    /* set port type */
    rc = prvTgfCscdPortTypeGet(prvTgfDevNum, PRV_TGF_SEND_PORT_NUM_CNS, CPSS_PORT_DIRECTION_RX_E,
                                                        &prvTgfRestoreCfg.portType);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeGet");

    rc = prvTgfCscdPortTypeSet(prvTgfDevNum, CPSS_PORT_DIRECTION_RX_E, PRV_TGF_SEND_PORT_NUM_CNS,
                                                        CPSS_CSCD_PORT_DSA_MODE_EXTEND_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet");


    /* set port assignment mode */
    rc = prvTgfCscdMyPhysicalPortAssignmentModeGet(prvTgfDevNum, PRV_TGF_SEND_PORT_NUM_CNS,
                                                    &prvTgfRestoreCfg.portAssignMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdMyPhysicalPortAssignmentModeGet");

    rc = prvTgfCscdMyPhysicalPortAssignmentModeSet(prvTgfDevNum, PRV_TGF_SEND_PORT_NUM_CNS,
                            PRV_TGF_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_DSA_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdMyPhysicalPortAssignmentModeSet");


    /* set port mapping dsa source lsb amount */
    rc = prvTgfCscdMyPhysicalPortMappingDsaSrcLsbAmountGet(
                  &prvTgfRestoreCfg.srcDevLsbAmount, &prvTgfRestoreCfg.srcPortLsbAmount);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdMyPhysicalPortMappingDsaSrcLsbAmountGet");

    rc = prvTgfCscdMyPhysicalPortMappingDsaSrcLsbAmountSet(PRV_TGF_SRC_DEV_LSB_AMOUNT_CNS,
                                                           PRV_TGF_SRC_PORT_LSB_AMOUNT_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdMyPhysicalPortMappingDsaSrcLsbAmountSet");


    /* set ingress physical port base */
    rc = prvTgfCscdMyPhysicalPortBaseGet(prvTgfDevNum, PRV_TGF_SEND_PORT_NUM_CNS,
                                                  &prvTgfRestoreCfg.phyPortBase);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdMyPhysicalPortBaseGet");

    rc = prvTgfCscdMyPhysicalPortBaseSet(prvTgfDevNum, PRV_TGF_SEND_PORT_NUM_CNS,
                                                PRV_TGF_PHYSICAL_PORT_BASE_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdMyPhysicalPortBaseSet");


    /* set enable the assignment of the packet source device to be the local device number */
    rc = prvTgfCscdMyPhysicalPortAssignSrcDevEnableGet(prvTgfDevNum, PRV_TGF_SEND_PORT_NUM_CNS,
                                                  &prvTgfRestoreCfg.assignSrcDevEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdMyPhysicalPortAssignSrcDevEnableGet");

    rc = prvTgfCscdMyPhysicalPortAssignSrcDevEnableSet(prvTgfDevNum,
                                PRV_TGF_SEND_PORT_NUM_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdMyPhysicalPortAssignSrcDevEnableSet");


    /*AUTODOC:  create VLAN with untagged ports [0,1,2,3] */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d", PRV_TGF_VLANID_CNS);

    /*AUTODOC:  clear counters */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        /* reset ethernet counters */
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }
}


/**
* @internal tgfCscdRemotePortRemappingTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port[0] packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         fdb learning return match on port 13
*/
GT_VOID tgfCscdRemotePortRemappingTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                   rc = GT_OK;
    GT_BOOL                     isOk;
    PRV_TGF_MAC_ENTRY_KEY_STC   macEntryKey;
    PRV_TGF_BRG_MAC_ENTRY_STC   prvTgfMacEntry;

    prvTgfPacketDsaTagPart.dsaInfo.forward.srcHwDev         = PRV_TGF_DSA_DEV_CNS;
    prvTgfPacketDsaTagPart.dsaInfo.forward.source.portNum = PRV_TGF_DSA_PORT_CNS;

    /* AUTODOC: setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo, PRV_TGF_BURST_COUNT_CNS, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                                 prvTgfDevNum, PRV_TGF_BURST_COUNT_CNS, 0, NULL);

    /* AUTODOC: clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: send Ethernet packet from port 0 with: */
    /* AUTODOC: DA=00:00:00:00:00:56, SA=00:00:00:00:00:02, VID=1 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, PRV_TGF_SEND_PORT_NUM_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                                 prvTgfDevNum, PRV_TGF_SEND_PORT_NUM_CNS);

    /* check FDB learning */
    macEntryKey.entryType = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntryKey.key.macVlan.vlanId = PRV_TGF_VLANID_CNS;

    cpssOsMemCpy(macEntryKey.key.macVlan.macAddr.arEther, prvTgfPacketL2Part.saMac, sizeof(TGF_MAC_ADDR));
    cpssOsMemSet(&prvTgfMacEntry, 0, sizeof(prvTgfMacEntry));

    rc = prvTgfBrgFdbMacEntryGet(&macEntryKey, &prvTgfMacEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryGet failed");

    if (GT_OK == rc)
    {
        /* add support for errata in BC2 => implemented as .dsaTagOrigSrcPortNotUpdated */
        if(GT_TRUE == prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_CPSS_PP_E_ARCH_CNS))
        {
            /*not UTF_BOBCAT2_E - keep existing beahvior */
            isOk = (prvTgfMacEntry.dstInterface.devPort.hwDevNum  == prvTgfDevNum) &&
               (prvTgfMacEntry.dstInterface.devPort.portNum == PRV_TGF_FDB_PORT_NUM_CNS);

            UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, isOk, "got wrong fdb learning results");
        }
        else
        {
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(prvTgfDevNum) == GT_FALSE)
            {
                /*UTF_BOBCAT2_E - check the source port is not changed*/
                isOk = (prvTgfMacEntry.dstInterface.devPort.hwDevNum  == prvTgfDevNum) &&
                       (prvTgfMacEntry.dstInterface.devPort.portNum == PRV_TGF_DSA_PORT_CNS);

                UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, isOk, "got wrong fdb learning results");
            }
        }

    }
}

/**
* @internal tgfCscdRemotePortRemappingConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID tgfCscdRemotePortRemappingConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS   rc;

    /* restore port type */
    rc = prvTgfCscdPortTypeSet(prvTgfDevNum, CPSS_PORT_DIRECTION_RX_E, PRV_TGF_SEND_PORT_NUM_CNS,
                                                    prvTgfRestoreCfg.portType);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet config restore");

    /* restore port assignment mode */
    rc = prvTgfCscdMyPhysicalPortAssignmentModeSet(prvTgfDevNum, PRV_TGF_SEND_PORT_NUM_CNS,
                                                    prvTgfRestoreCfg.portAssignMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdMyPhysicalPortAssignmentModeSet config restore");

    /* restore port mapping dsa source lsb amount */
    rc = prvTgfCscdMyPhysicalPortMappingDsaSrcLsbAmountSet(prvTgfRestoreCfg.srcDevLsbAmount,
                                                           prvTgfRestoreCfg.srcPortLsbAmount);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdMyPhysicalPortMappingDsaSrcLsbAmountSet config restore");

    /* restore ingress physical port base */
    rc = prvTgfCscdMyPhysicalPortBaseSet(prvTgfDevNum, PRV_TGF_SEND_PORT_NUM_CNS, prvTgfRestoreCfg.phyPortBase);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdMyPhysicalPortBaseSet config restore");

    /* restore enable the assignment of the packet source device to be the local device number */
    rc = prvTgfCscdMyPhysicalPortAssignSrcDevEnableSet(prvTgfDevNum, PRV_TGF_SEND_PORT_NUM_CNS,
                                                  prvTgfRestoreCfg.assignSrcDevEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdMyPhysicalPortAssignSrcDevEnableSet config restore");

    /* invalidate VLAN entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d", PRV_TGF_VLANID_CNS);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d", GT_TRUE);

    /* AUTODOC: restore prvTgfPortsArray */
    cpssOsMemCpy(prvTgfPortsArray, prvTgfRestoreCfg.portsArray,
                 sizeof(prvTgfRestoreCfg.portsArray));

}

