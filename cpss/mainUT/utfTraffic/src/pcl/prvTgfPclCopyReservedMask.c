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
* @file prvTgfPclCopyReservedMask.c
*
* @brief CopyReserved feature support test - TTI/IPCL/EPCL
*        Includes Flow_Id extension (16 bits) Test for SIP6_10 and above devices
*
* @version   1
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficEngine.h>
#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfTcamGen.h>
#include <common/tgfConfigGen.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfPclGen.h>

#include <pcl/prvTgfPclCopyReservedMask.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* port to send traffic to */
#define PRV_TGF_RX_PORT_IDX_CNS   0

/* port number to receive traffic from */
#define PRV_TGF_TX_PORT_IDX_CNS   1

/* PCL rule & action index */
#define IPCL_MATCH_INDEX_0_CNS     0
#define EPCL_MATCH_INDEX_CNS       prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet(5)
/* tunnel term entry index */
#define PRV_TGF_TTI_INDEX_CNS      10
/* Flow ID */
#define PRV_TGF_TTI_ACTION_FLOW_ID_CNS 0xF3FA

#define COPY_RESERVED_MASK_CNS                       0x3FFFF
#define COPY_RESERVED_ACTION_CNS                     0x3FFFF
#define TTI_COPY_RESERVED_ACTION_CNS                 0x7FF
#define COPY_RESERVED_SIP_6_10_MASK_CNS              0x7FFFF
#define COPY_RESERVED_SIP_6_10_ACTION_CNS            0x7FFFF
#define TTI_COPY_RESERVED_SIP_6_10_ACTION_CNS        0xFFF

/* default number of packets to send */
static GT_U32   prvTgfBurstCount = 1;

/************************* General packet's parts *****************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x00, 0x00, 0x33, 0x44, 0x55},                /* daMac */
    {0x00, 0x00, 0x00, 0x03, 0x04, 0x05}                 /* saMac */
};

/* Data of packet */
static GT_U8 prvTgfPacketPayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f
};

/* Payload part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart =
{
    sizeof(prvTgfPacketPayloadDataArr), /* dataLength */
    prvTgfPacketPayloadDataArr          /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    (TGF_L2_HEADER_SIZE_CNS + sizeof(prvTgfPacketPayloadDataArr))

/* packet to send */
static TGF_PACKET_STC prvTgfPacketInfo =
{
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,          /* totalLen */
    (sizeof(prvTgfPacketPartArray)
        / sizeof(prvTgfPacketPartArray[0])), /* numOfParts */
    prvTgfPacketPartArray                    /* partsArray */
};

/******************************************************************************/

/* RESTORE VALUE */
static GT_U32 ingrCopyReservedMask;
static PRV_TGF_TTI_MAC_MODE_ENT origMacMode = 0;

/*******************************************************************************/
/**
* @internal prvTgfPclCopyReservedTestPclRulesAndActionSet function
* @endinternal
*
* @brief   Set PCL rules and actions
*/
static GT_VOID prvTgfPclCopyReservedTestPclRulesAndActionSet
(
    IN    GT_U32      copyReserved,
    IN    GT_BOOL     ttiBasedCopyReservedAssignEn
)
{
    GT_STATUS rc;
    PRV_TGF_PCL_RULE_FORMAT_UNT         mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT         pattern;
    PRV_TGF_PCL_ACTION_STC              action;
    PRV_TGF_PCL_UDB_SELECT_STC          udbSelect;
    PRV_TGF_TTI_ACTION_2_STC            ttiAction;
    PRV_TGF_TTI_RULE_UNT                ttiMask;
    PRV_TGF_TTI_RULE_UNT                ttiPattern;
    GT_U32                              udbIndexArr[5] = {40,41,42,43,44};

    cpssOsMemSet(&udbSelect, 0, sizeof(udbSelect));
    cpssOsMemSet(&action, 0, sizeof(action));
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&ttiAction, 0, sizeof(ttiAction));
    cpssOsMemSet(&ttiMask, 0, sizeof(ttiMask));
    cpssOsMemSet(&ttiPattern, 0, sizeof(ttiPattern));

    /* Mapping UDB_0/1/2 to UDB_40/41/42 */
    udbSelect.udbSelectArr[0] = udbIndexArr[0];
    udbSelect.udbSelectArr[1] = udbIndexArr[1];
    udbSelect.udbSelectArr[2] = udbIndexArr[2];
    if(PRV_CPSS_SIP_6_10_CHECK_MAC(prvTgfDevNum) && (ttiBasedCopyReservedAssignEn != GT_TRUE))
    {
        udbSelect.udbSelectArr[3] = udbIndexArr[3];
        udbSelect.udbSelectArr[4] = udbIndexArr[4];
    }

    if(PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E)
    {
        GT_U32  ii;
        for(ii = 0 ; ii < 10; ii++)
        {
            udbSelect.udbSelectArr[ii] = ii;
        }
        udbIndexArr[0] = udbSelect.udbSelectArr[0];
        udbIndexArr[1] = udbSelect.udbSelectArr[1];
        udbIndexArr[2] = udbSelect.udbSelectArr[2];
        udbIndexArr[3] = udbSelect.udbSelectArr[3];
        udbIndexArr[4] = udbSelect.udbSelectArr[4];
    }

    if(ttiBasedCopyReservedAssignEn)
    {
        prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);

        rc = prvTgfTtiMacModeGet(prvTgfDevNum,PRV_TGF_TTI_KEY_ETH_E,&origMacMode);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeGet");

        rc = prvTgfTtiMacModeSet(PRV_TGF_TTI_KEY_ETH_E,PRV_TGF_TTI_MAC_MODE_SA_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeSet");

        /* enable the TTI lookup for PRV_TGF_TTI_KEY_ETH_E */
        rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS],
                                          PRV_TGF_TTI_KEY_ETH_E, GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

        /* AUTODOC: TTI Mask for MAC SA */
        cpssOsMemSet(ttiMask.eth.common.mac.arEther, 0xFF, sizeof(GT_ETHERADDR));

        /* AUTODOC: TTI Pattern for MAC SA */
        cpssOsMemCpy(ttiPattern.eth.common.mac.arEther,
                     prvTgfPacketL2Part.saMac,
                     sizeof(GT_ETHERADDR));

        /* TTI actions - FORWARD packet, assign copyReserved = 0x7FF */
        ttiAction.command = CPSS_PACKET_CMD_FORWARD_E;
        ttiAction.copyReservedAssignmentEnable = GT_TRUE;
        ttiAction.copyReserved = copyReserved;
        rc = prvTgfTtiRule2Set(PRV_TGF_TTI_INDEX_CNS, PRV_TGF_TTI_KEY_ETH_E,
                               &ttiPattern, &ttiMask, &ttiAction);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet");
    }
    else
    {
        /* IPCL actions - FORWARD packet, assign copyReserved = 0x3FFFF */
        action.copyReserved.assignEnable = GT_TRUE;
        action.copyReserved.copyReserved = copyReserved;

        if(PRV_CPSS_SIP_6_10_CHECK_MAC(prvTgfDevNum))
        {
            action.flowId = PRV_TGF_TTI_ACTION_FLOW_ID_CNS;
        }

        if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
        {
            /* Enable ingress PCL action <triggerInterrupt> */
            action.triggerInterrupt = GT_TRUE;
        }
    }

    action.pktCmd       = CPSS_PACKET_CMD_FORWARD_E;
    action.bypassBridge = GT_TRUE;
    action.redirect.redirectCmd=PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
    action.redirect.data.outIf.outInterface.type = CPSS_INTERFACE_PORT_E;
    action.redirect.data.outIf.outInterface.devPort.hwDevNum = prvTgfDevNum;
    action.redirect.data.outIf.outInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS];

    /* AUTODOC: IPCL Mask for MAC SA */
    cpssOsMemSet(mask.ruleStdNotIp.macSa.arEther, 0xFF, sizeof(GT_ETHERADDR));

    /* AUTODOC: IPCL Pattern for MAC SA */
    cpssOsMemCpy(pattern.ruleStdNotIp.macSa.arEther,
                prvTgfPacketL2Part.saMac,
                sizeof(GT_ETHERADDR));

    rc = prvTgfPclRuleSet(
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                IPCL_MATCH_INDEX_0_CNS, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d",
                                     PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                                     IPCL_MATCH_INDEX_0_CNS);

    cpssOsMemSet(&action, 0, sizeof(action));
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));

    /* AUTODOC: Metadata UDBs configuration for copyReserved */
    /* EPCL UDBs Metadata:(724:705) -  copyReserved(20 bits) */
    rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
                                 PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                 CPSS_PCL_DIRECTION_EGRESS_E,
                                 udbIndexArr[0],
                                 PRV_TGF_PCL_OFFSET_METADATA_E,
                                 88);   /* Anchor offset: 88 Bit offset: 7:1 */
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc,
                                "prvTgfPclUserDefinedByteSet: packetType %d, udbIndex %d,"
                                "OffsetType %d, Offset %d\n",
                                 PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                 udbIndexArr[0], PRV_TGF_PCL_OFFSET_METADATA_E, 88);

    rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
                                 PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                 CPSS_PCL_DIRECTION_EGRESS_E,
                                 udbIndexArr[1],
                                 PRV_TGF_PCL_OFFSET_METADATA_E,
                                 89);  /* Anchor offset: 89 Bit offset: 7:0 */
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc,
                                "prvTgfPclUserDefinedByteSet: packetType %d, udbIndex %d,"
                                "OffsetType %d, Offset %d\n",
                                 PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                 udbIndexArr[1], PRV_TGF_PCL_OFFSET_METADATA_E, 89);

    rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
                                 PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                 CPSS_PCL_DIRECTION_EGRESS_E,
                                 udbIndexArr[2],
                                 PRV_TGF_PCL_OFFSET_METADATA_E,
                                 90); /* Anchor offset: 90 Bit offset: 4:0 */
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc,
                                 "prvTgfPclUserDefinedByteSet: packetType %d, udbIndex %d,"
                                 "OffsetType %d, Offset %d\n",
                                 PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                 udbIndexArr[2], PRV_TGF_PCL_OFFSET_METADATA_E, 90);

    if(PRV_CPSS_SIP_6_10_CHECK_MAC(prvTgfDevNum) && (ttiBasedCopyReservedAssignEn != GT_TRUE))
    {
        rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
                                 PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                 CPSS_PCL_DIRECTION_EGRESS_E,
                                 udbIndexArr[3],
                                 PRV_TGF_PCL_OFFSET_METADATA_E,
                                 82); /* Anchor offset: 82 Bit offset: 7:0 */
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc,
                                 "prvTgfPclUserDefinedByteSet: packetType %d, udbIndex %d,"
                                 "OffsetType %d, Offset %d\n",
                                 PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                 udbIndexArr[3], PRV_TGF_PCL_OFFSET_METADATA_E, 82);

        rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
                                 PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                 CPSS_PCL_DIRECTION_EGRESS_E,
                                 udbIndexArr[4],
                                 PRV_TGF_PCL_OFFSET_METADATA_E,
                                 83); /* Anchor offset: 83 Bit offset: 7:0 */
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc,
                                 "prvTgfPclUserDefinedByteSet: packetType %d, udbIndex %d,"
                                 "OffsetType %d, Offset %d\n",
                                 PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                 udbIndexArr[4], PRV_TGF_PCL_OFFSET_METADATA_E, 83);
    }

    rc = prvTgfPclUserDefinedBytesSelectSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
                                            PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E, CPSS_PCL_LOOKUP_0_E,
                                            &udbSelect);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedBytesSelectSet: %d, %d",
                                 PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E, CPSS_PCL_LOOKUP_0_E);

    /* AUTODOC: mask and pattern for copyReserved */
    mask.ruleEgrUdbOnly.udb[0] = 0xFE;
    mask.ruleEgrUdbOnly.udb[1] = 0xFF;
    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        mask.ruleEgrUdbOnly.udb[2] = 0x1F;
    }
    else if (PRV_CPSS_SIP_5_20_CHECK_MAC(prvTgfDevNum))
    {
        mask.ruleEgrUdbOnly.udb[2] = 0xF;
    }
    else
    {
        mask.ruleEgrUdbOnly.udb[2] = 0x3;
    }

    if (PRV_CPSS_SIP_6_10_CHECK_MAC(prvTgfDevNum) && (ttiBasedCopyReservedAssignEn != GT_TRUE))
    {
        mask.ruleEgrUdbOnly.udb[3] = 0xFF;
        mask.ruleEgrUdbOnly.udb[4] = 0xFF;
    }

    /* Copy Reserved value used is 0x3FFFF and is 18bits whereas descriptor
     * copy reserved is total 20 bits where the lsb 2 bits are reserved. UDB0
     * for copy reserved uses bits 1:7 where bits 1:2 are reserved and so need
     * to left shift by 3 to write 5 msb bits. UDB1 for copy reserved uses bits
     * 0:7 and since 5 bits are already written in UDB0 so right shit by 5 to
     * write next 8 bits to UDB1. Now 13 bits of copy reserved are written to
     * UDB0 and UDB1 so remaining 5 bits are written to UDB2.
     * This logic changes for SIP_6_10 and above devices which has total of
     * 19 copy reserved bits which can be written instead of 18 bits.
     */
    if (PRV_CPSS_SIP_6_10_CHECK_MAC(prvTgfDevNum))
    {
        pattern.ruleEgrUdbOnly.udb[0] =  (copyReserved << 2) & 0xFE;
        pattern.ruleEgrUdbOnly.udb[1] =  (copyReserved >> 6) & 0xFF;
        pattern.ruleEgrUdbOnly.udb[2] =  (copyReserved >> 14) & 0x1F;
    }
    else
    {
        pattern.ruleEgrUdbOnly.udb[0] =  (copyReserved << 3) & 0xFE;
        pattern.ruleEgrUdbOnly.udb[1] =  (copyReserved >> 5) & 0xFF;
        pattern.ruleEgrUdbOnly.udb[2] =  (copyReserved >> 13) & 0x1F;
    }

    if (PRV_CPSS_SIP_6_10_CHECK_MAC(prvTgfDevNum) && (ttiBasedCopyReservedAssignEn != GT_TRUE))
    {
        pattern.ruleEgrUdbOnly.udb[3] =  PRV_TGF_TTI_ACTION_FLOW_ID_CNS & 0xFF;
        pattern.ruleEgrUdbOnly.udb[4] =  (PRV_TGF_TTI_ACTION_FLOW_ID_CNS >> 8) & 0xFF;
    }

    /* AUTODOC: EPCL action - drop the packet */
    action.egressPolicy = GT_TRUE;
    action.pktCmd       = CPSS_PACKET_CMD_DROP_HARD_E;

    rc = prvTgfPclRuleSet(
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
            EPCL_MATCH_INDEX_CNS, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d",
                                 PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
                                 EPCL_MATCH_INDEX_CNS);
}
/**
* @internal prvTgfPclCopyReservedMaskTestConfigurationSet function
* @endinternal
*
* @brief   initial configurations
*/
GT_VOID prvTgfPclCopyReservedMaskTestConfigurationSet
(
     IN CPSS_PCL_LOOKUP_NUMBER_ENT lookupNum,
     IN GT_BOOL                    ttiBasedCopyReservedAssignEn
)
{
    GT_STATUS   rc;
    GT_U32      numOfEvents;

    /* AUTODOC: SETUP CONFIGURATION: */
    /* AUTODOC: Init IPCL Engine for port 0 */
    rc = prvTgfPclDefPortInit(
            prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS],
            CPSS_PCL_DIRECTION_INGRESS_E,
            lookupNum,
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E     /*nonIpKey*/,
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv4Key*/,
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E  /*ipv6Key*/);
    UTF_VERIFY_EQUAL3_STRING_MAC(
            GT_OK, rc, "prvTgfPclDefPortInit: %d, %d, %d",
            prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS],
            CPSS_PCL_DIRECTION_INGRESS_E);

    if(ttiBasedCopyReservedAssignEn == GT_FALSE)
    {
        /* AUTODOC: Save copyReserved mask for restore (for ingress)*/
        rc = prvTgfPclCopyReservedMaskGet(prvTgfDevNum,
                                          CPSS_PCL_DIRECTION_INGRESS_E,
                                          lookupNum,
                                          &ingrCopyReservedMask);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfPclCopyReservedMaskGet: %d, %d, %d, %d",
                                     prvTgfDevNum, CPSS_PCL_DIRECTION_INGRESS_E,
                                     lookupNum,
                                     &ingrCopyReservedMask);

        /* AUTODOC: Set copyReserved mask to 0x3FFFF for both ingress and egress direction */
        rc = prvTgfPclCopyReservedMaskSet(prvTgfDevNum, CPSS_PCL_DIRECTION_INGRESS_E,
                                          lookupNum, PRV_CPSS_SIP_6_10_CHECK_MAC(prvTgfDevNum) ? COPY_RESERVED_SIP_6_10_MASK_CNS : COPY_RESERVED_MASK_CNS);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclCopyReservedMaskSet: %d, %d",
                                     lookupNum, COPY_RESERVED_MASK_CNS);

        if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
        {
            /* Check the occurance of the event */
            rc = utfGenEventCounterGet(prvTgfDevNum,
                                       CPSS_PP_PCL_ACTION_TRIGGERED_E,GT_TRUE,&numOfEvents);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "utfGenEventCounterGet: %d", CPSS_PP_PCL_ACTION_TRIGGERED_E);
        }
    }
    /* AUTODOC: Init EPCL Engine for port 1 */
    rc = prvTgfPclDefPortInit(
        prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_EGRESS_E,
        lookupNum,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E  /*nonIpKey*/,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E /*ipv4Key*/,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E /*ipv6Key*/);
    UTF_VERIFY_EQUAL3_STRING_MAC(
            GT_OK, rc, "prvTgfPclDefPortInit: %d, %d, %d",
            prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
            CPSS_PCL_DIRECTION_EGRESS_E);

    /* Set PCL rules and actions */
    if(ttiBasedCopyReservedAssignEn)
    {
        prvTgfPclCopyReservedTestPclRulesAndActionSet(PRV_CPSS_SIP_6_10_CHECK_MAC(prvTgfDevNum) ? TTI_COPY_RESERVED_SIP_6_10_ACTION_CNS :
                                                      TTI_COPY_RESERVED_ACTION_CNS, ttiBasedCopyReservedAssignEn);
    }
    else
    {
        prvTgfPclCopyReservedTestPclRulesAndActionSet(PRV_CPSS_SIP_6_10_CHECK_MAC(prvTgfDevNum) ? COPY_RESERVED_SIP_6_10_ACTION_CNS :
                                                      COPY_RESERVED_ACTION_CNS, ttiBasedCopyReservedAssignEn);
    }
}
/**
* @internal prvTgfPclCopyReservedMaskTestConfigurationRestore function
* @endinternal
*
* @brief   Test restore configurations
*/
GT_VOID prvTgfPclCopyReservedMaskTestConfigurationRestore
(
    IN CPSS_PCL_LOOKUP_NUMBER_ENT lookupNum,
    IN GT_BOOL                    ttiBasedCopyReservedAssignEn
)
{
    GT_STATUS   rc;
    GT_U32                              udbIndexArr[5] = {40,41,42,44,45};

    if(PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E)
    {
        GT_U32  ii;
        for(ii = 0 ; ii < 5; ii++)
        {
            udbIndexArr[ii] = ii;
        }
    }

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: invalidate PCL rule */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E,
                                     IPCL_MATCH_INDEX_0_CNS,
                                     GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                 CPSS_PCL_RULE_SIZE_STD_E,
                                 IPCL_MATCH_INDEX_0_CNS,
                                 GT_FALSE);

    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_10_BYTES_E,
                                     EPCL_MATCH_INDEX_CNS,
                                     GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                 CPSS_PCL_RULE_SIZE_10_BYTES_E,
                                 EPCL_MATCH_INDEX_CNS,
                                 GT_FALSE);

    /* AUTODOC: Clear IPCL Engine configuration tables */
    prvTgfPclRestore();

    /* AUTODOC: Disables PCL ingress Policy */
    rc = prvTgfPclIngressPolicyEnable(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclIngressPolicyEnable: %d", GT_FALSE);

    /* AUTODOC: Disables PCL egress Policy */
    rc = prvTgfPclEgressPolicyEnable(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclEgressPolicyEnable: %d", GT_FALSE);

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d",
                                 GT_TRUE);

    if(ttiBasedCopyReservedAssignEn != GT_TRUE)
    {
        /* AUTODOC: restore copyReserved bit mask value */
        rc = prvTgfPclCopyReservedMaskSet(prvTgfDevNum,
                                          CPSS_PCL_DIRECTION_INGRESS_E,
                                          lookupNum,
                                          ingrCopyReservedMask);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfPclCopyReservedMaskSet: %d, %d, %d, %d",
                                     prvTgfDevNum, CPSS_PCL_DIRECTION_INGRESS_E,
                                     lookupNum,
                                     ingrCopyReservedMask);
    }
    else
    {
        /* disable the TTI lookup for PRV_TGF_TTI_KEY_ETH_E */
        rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS],
                                          PRV_TGF_TTI_KEY_ETH_E, GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

        /* restore mac mode for the ethernet packets  */
        rc = prvTgfTtiMacModeSet(PRV_TGF_TTI_KEY_ETH_E,origMacMode);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeSet");
    }

    if (PRV_CPSS_SIP_6_10_CHECK_MAC(prvTgfDevNum))
    {
        /* AUTODOC: restore UDB CFG table */
        rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
                                     PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                     CPSS_PCL_DIRECTION_EGRESS_E,
                                     udbIndexArr[3],
                                     PRV_TGF_PCL_OFFSET_INVALID_E,
                                     82);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d", prvTgfDevNum);

        rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
                                     PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                     CPSS_PCL_DIRECTION_EGRESS_E,
                                     udbIndexArr[4],
                                     PRV_TGF_PCL_OFFSET_INVALID_E,
                                     83);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d", prvTgfDevNum);
    }
    rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
                                 PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                 CPSS_PCL_DIRECTION_EGRESS_E,
                                 udbIndexArr[0],
                                 PRV_TGF_PCL_OFFSET_INVALID_E,
                                 88);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d", prvTgfDevNum);

    rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
                                 PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                 CPSS_PCL_DIRECTION_EGRESS_E,
                                 udbIndexArr[1],
                                 PRV_TGF_PCL_OFFSET_INVALID_E,
                                 89);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d", prvTgfDevNum);

    rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
                                 PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                 CPSS_PCL_DIRECTION_EGRESS_E,
                                 udbIndexArr[2],
                                 PRV_TGF_PCL_OFFSET_INVALID_E,
                                 90);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d", prvTgfDevNum);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");
}

/**
* @internal prvTgfPclCopyReservedMaskTestPacketSend function
* @endinternal
*
* @brief   Send packet to selected port (by port index).
*
* @param[in] portIndex                - port indexing place in test array ports.
* @param[in] prvTgfPacketInfoPtr            - (pointer to) the packet info
*                                       None
*/
static GT_VOID prvTgfPclCopyReservedMaskTestPacketSend
(
    IN GT_U32           sendPortIndex,
    IN GT_U32           recvPortIndex,
    IN TGF_PACKET_STC   *prvTgfPacketInfoPtr
)
{
    GT_STATUS   rc;
    GT_U32                   portIter;
    CPSS_INTERFACE_INFO_STC  portInterface;

    for (portIter = 0; portIter < PRV_TGF_PORTS_NUM_CNS; portIter++)
    {
        /* reset ethernet counters */
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth");
    }

    /* Clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, prvTgfPacketInfoPtr, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                                 prvTgfDevNum, prvTgfBurstCount, 0, 0);

    /* set port for packet capture */
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[recvPortIndex];

    /* Start capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface,
                                                TGF_CAPTURE_MODE_MIRRORING_E,
                                                GT_TRUE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                           "tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d, %d",
                                 portInterface.devPort.hwDevNum,
                                 portInterface.devPort.portNum,
                                 GT_TRUE);
    /* send packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[sendPortIndex]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[sendPortIndex]);

    /* Stop capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface,
                                                TGF_CAPTURE_MODE_MIRRORING_E,
                                                GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                           "tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d, %d",
                                 portInterface.devPort.hwDevNum,
                                 portInterface.devPort.portNum,
                                 GT_FALSE);

    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n",
                     portInterface.devPort.portNum);

}
/**
* @internal prvTgfPclCopyReservedMaskTestGenerateTraffic function
* @endinternal
*
* @brief   Test generate traffic and check results
*/
GT_VOID prvTgfPclCopyReservedMaskTestGenerateTraffic
(
   IN GT_BOOL    ttiBasedCopyReservedAssignEn
)
{

    GT_STATUS                       rc;
    CPSS_INTERFACE_INFO_STC         portInterface;
    GT_U32                          numOfEvents = 0;
    GT_U32  actualCapturedNumOfPackets;

    /* set port for packet capture */
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS];

    /* AUTODOC: Send packet with MAC SA                             */
    /* AUTODOC: - expect match in IPCL -> Forward packet and assign copyReserved to Ox3AA */
    /* AUTODOC: - expect match in EPCL -> drop packet         */
    prvTgfPclCopyReservedMaskTestPacketSend(PRV_TGF_RX_PORT_IDX_CNS,
                                        PRV_TGF_TX_PORT_IDX_CNS,
                                        &prvTgfPacketInfo);

    /* print captured packets and check TriggerCounters */
    rc = tgfTrafficGeneratorPortTxEthCaptureCompare( &portInterface,
                                        &prvTgfPacketInfo,
                                        0,/*numOfPackets*/
                                        0, /*vfdNum*/
                                        NULL /*vfdArray*/,
                                        NULL, /* bytesNum's skip list */
                                        0,    /* length of skip list */
                                        &actualCapturedNumOfPackets,
                                        NULL/*onFirstPacketNumTriggersBmpPtr*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthCaptureCompare");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, actualCapturedNumOfPackets,"Expect packet to be dropped by EPCL action");

    if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum) && ttiBasedCopyReservedAssignEn != GT_TRUE)
    {
        /* Check the occurance of the event */
        rc = utfGenEventCounterGet(prvTgfDevNum,
                                   CPSS_PP_PCL_ACTION_TRIGGERED_E,GT_TRUE,&numOfEvents);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "utfGenEventCounterGet");
        UTF_VERIFY_EQUAL1_STRING_MAC(1, numOfEvents, " No of events count dosen't match %d", numOfEvents);

        /* AUTODOC: Change MAC SA to MAC SA 1*/
        prvTgfPacketL2Part.saMac[4] += 1;

        /* AUTODOC: Send packet with MAC SA 1 */
        /* AUTODOC: - expect no match in IPCL */
        prvTgfPclCopyReservedMaskTestPacketSend(PRV_TGF_RX_PORT_IDX_CNS,
                                            PRV_TGF_TX_PORT_IDX_CNS,
                                            &prvTgfPacketInfo);

        if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
        {
            /* Check the occurance of the event */
            rc = utfGenEventCounterGet(prvTgfDevNum,
                                       CPSS_PP_PCL_ACTION_TRIGGERED_E,GT_TRUE,&numOfEvents);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "utfGenEventCounterGet");
            UTF_VERIFY_EQUAL1_STRING_MAC(0, numOfEvents, " No of events count dosen't match %d", numOfEvents);
        }

        /* AUTODOC: Change back MAC SA 1 to MAC SA */
        prvTgfPacketL2Part.saMac[4] -= 1;
    }
}
/**
* @internal prvTgfPclCopyReservedMaskTest function
* @endinternal
*
* @brief   copyReserved assignment masking in IPCL/EPCL action test
*/
GT_VOID prvTgfPclCopyReservedMaskTest
(
    GT_VOID
)
{
    /* Check for Lookup 0 */
    prvTgfPclCopyReservedMaskTestConfigurationSet(CPSS_PCL_LOOKUP_NUMBER_0_E, GT_FALSE);
    prvTgfPclCopyReservedMaskTestGenerateTraffic(GT_FALSE);
    prvTgfPclCopyReservedMaskTestConfigurationRestore(CPSS_PCL_LOOKUP_NUMBER_0_E, GT_FALSE);

    if(!HARD_WIRE_TCAM_MAC(prvTgfDevNum))
    {
        /* Check for Lookup 1 */
        prvTgfPclCopyReservedMaskTestConfigurationSet(CPSS_PCL_LOOKUP_NUMBER_1_E, GT_FALSE);
        prvTgfPclCopyReservedMaskTestGenerateTraffic(GT_FALSE);
        prvTgfPclCopyReservedMaskTestConfigurationRestore(CPSS_PCL_LOOKUP_NUMBER_1_E, GT_FALSE);
    }

    /* Check for Lookup 0 */
    prvTgfPclCopyReservedMaskTestConfigurationSet(CPSS_PCL_LOOKUP_NUMBER_0_E, GT_TRUE /*ttiBasedCopyReservedAssignEn*/);
    prvTgfPclCopyReservedMaskTestGenerateTraffic(GT_TRUE);
    prvTgfPclCopyReservedMaskTestConfigurationRestore(CPSS_PCL_LOOKUP_NUMBER_0_E, GT_TRUE);
}

