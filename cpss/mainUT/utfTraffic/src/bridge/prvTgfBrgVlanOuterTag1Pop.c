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
* @file prvTgfBrgVlanOuterTag1Pop.c
*
* @brief * @brief   Test on VLAN Tag Ingress Pop outer Tag1 bytes.
*          the inner tag 0 is ignored by egress port that configured 'nested vlan'.
*          so an egress port that is defined 'untagged' still need to egress the port with 'tag 0'.
*       relate to jira : CPSS-12052 : VID tag pop is popping the wrong VID - when egress port is 'nested vlan'
*
* @version   1
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
#include <bridge/prvTgfBrgVlanTagPop.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
#define BUILD_U32_FROM_TAG_MAC(tag)  \
    (tag.etherType << 16 |           \
     tag.pri << 13       |           \
     tag.cfi << 12       |           \
     tag.vid)

/* first VLAN Id */
#define PRV_TGF_VLANID_0_CNS          2

/* second VLAN Id */
#define PRV_TGF_VLANID_1_CNS          3

/* number of packets to send */
#define PRV_TGF_SEND_PACKETS_NUM_CNS  1

/* port number to send vlan 2 traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS     1

/* capturing port number  */
#define PRV_TGF_CAPTURE_PORT_IDX_CNS  2

/* Source ePort */
#define PRV_TGF_SRC_EPORT_CNS               ALIGN_EPORT_TO_512_TILL_MAX_DEV_MAC(0x1432)

/* TTI rule index */
#define PRV_TGF_TTI_RULE_IDX_CNS             2

/* TTI ID */
#define PRV_TGF_TTI_ID_CNS                   1

/* PCL id for restore procedure */
static GT_U32 savePclId;
/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}                /* srcMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag0Part =
{
    TGF_ETHERTYPE_88A8_SERVICE_TAG_CNS,                 /* etherType */
    0, 0, PRV_TGF_VLANID_0_CNS                          /* pri, cfi, VlanId */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag1Part =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    1, 0, PRV_TGF_VLANID_1_CNS                          /* pri, cfi, VlanId */
};


/* Ethernet Type part */
static TGF_PACKET_ETHERTYPE_STC prvTgfPackeEthertypePart =
{
    0x3456
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart =
{
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of tagged packet */
static TGF_PACKET_PART_STC prvTgfTaggedPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTag0Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTag1Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPackeEthertypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};


/* TAGGED PACKET to send */
static TGF_PACKET_STC prvTgfTaggedPacketInfo =
{
    (TGF_L2_HEADER_SIZE_CNS
        + (2 * TGF_VLAN_TAG_SIZE_CNS)
        + TGF_ETHERTYPE_SIZE_CNS
        + sizeof(prvTgfPayloadDataArr)),                               /* totalLen */
    sizeof(prvTgfTaggedPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfTaggedPacketPartArray                                        /* partsArray */
};

/******************************************************************************/


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfBrgVlanTagPopPortVlanFdbSet function
* @endinternal
*
* @brief   Set test configuration
*/
static GT_STATUS prvTgfBrgVlanTagPopPortVlanFdbSet
(
    GT_VOID
)
{
    GT_STATUS                        rc = GT_OK;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: reset counters, force Link UP */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfEthCountersReset");

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: create VLAN as TAG in Tunnel with all untagged ports */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(
        PRV_TGF_VLANID_1_CNS, PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfBrgDefVlanEntryWriteWithTaggingCmd");

    /* AUTODOC: Learn MAC_DA of with source VID to receive port (to be Known UC) */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
        prvTgfPacketL2Part.daMac,
        PRV_TGF_VLANID_1_CNS /*vlanId*/,
        prvTgfDevsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS],
        GT_TRUE /*isStatic*/);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfBrgDefFdbMacEntryOnPortSet");


    return GT_OK;
}

/**
* @internal prvTgfBrgVlanTagPopPortVlanFdbReset function
* @endinternal
*
* @brief   Set test configuration
*/
static GT_STATUS prvTgfBrgVlanTagPopPortVlanFdbReset
(
    GT_VOID
)
{
    GT_STATUS rc, rc1 = GT_OK;

    /* AUTODOC: reset counters, force Link UP */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfEthCountersReset")

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "tgfTrafficTableRxPcktTblClear")

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfBrgFdbFlush")

    /* AUTODOC: invalidate VLAN Table Entry, VID as TAG in Tunnel */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_1_CNS);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfBrgDefVlanEntryInvalidate")


    return rc1;
}

/**
* @internal prvTgfBrgVlanTagPopTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] prvTgfPacketInfoPtr      - (pointer to) packet info
*                                       None
*/
static GT_VOID prvTgfBrgVlanTagPopTrafficGenerate
(
    IN TGF_PACKET_STC *prvTgfPacketInfoPtr
)
{
    GT_STATUS            rc         = GT_OK;

    /* AUTODOC: Transmit Packets With Capture*/

    rc = prvTgfTransmitPacketsWithCapture(
        prvTgfDevsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        prvTgfPacketInfoPtr,
        1      /*burstCount*/,
        0      /*numVfd*/,
        NULL   /*vfdArray*/,
        prvTgfDevsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS],
        TGF_CAPTURE_MODE_MIRRORING_E,
        200                                /*captureOnTime*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfTransmitPacketsWithCapture");
}

/**
* @internal checkEgressTagsAfterPopWithNestedVlan function
* @endinternal
*
* @brief   Checks traffic egress with single tag0 although poped one tag and
*       in vlan egress port defined as 'untagged'
*
*                                       None
*/
static GT_VOID checkEgressTagsAfterPopWithNestedVlan( void )
{
    GT_STATUS                       rc         = GT_OK;
    CPSS_INTERFACE_INFO_STC         portInterface;
    TGF_VFD_INFO_STC                vfdArray[9/*8+1*/];/*one more for ether type*/
    GT_U32                          numTriggersBmp;
    GT_U32                          ii;
    TGF_ETHER_TYPE                  etherType;
    GT_U32                          egressTag;

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* set vfd for destination MAC */
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;

    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS];
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS];

    egressTag = BUILD_U32_FROM_TAG_MAC(prvTgfPacketVlanTag1Part);

    ii = 0;

    /* one for egressTag */
    vfdArray[ii].offset = 12; /* after mac addresses  */
    vfdArray[ii].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[ii].cycleCount = 4;
    vfdArray[ii].patternPtr[0] = (GT_U8)((egressTag >> 24) & 0xFF);
    vfdArray[ii].patternPtr[1] = (GT_U8)((egressTag >> 16) & 0xFF);
    vfdArray[ii].patternPtr[2] = (GT_U8)((egressTag >>  8) & 0xFF);
    vfdArray[ii].patternPtr[3] = (GT_U8)((egressTag >>  0) & 0xFF);

    ii = 1;
    etherType = prvTgfPackeEthertypePart.etherType;

    /* one for ethertype */
    vfdArray[ii].offset = 12 + ii*4; /* after mac addresses  */
    vfdArray[ii].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[ii].cycleCount = 2;
    vfdArray[ii].patternPtr[0] = (GT_U8)((etherType >>  8) & 0xFF);
    vfdArray[ii].patternPtr[1] = (GT_U8)((etherType >>  0) & 0xFF);

    ii++;

    rc =  tgfTrafficGeneratorPortTxEthTriggerCountersGet(
            &portInterface,
            ii /*numVfd*/,
            vfdArray,
            &numTriggersBmp);
    if (rc != GT_NO_MORE)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d",
            prvTgfDevNum);
    }

    UTF_VERIFY_EQUAL1_STRING_MAC(
        ((1<<ii)- 1), numTriggersBmp, "NOT all [%d] triggers are matched \n",
        ii);

}

/**
* @internal prvTgfBrgVlanTagPopTtiConfigSet function
* @endinternal
*
* @brief   build TTI Basic rule
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS prvTgfBrgVlanTagPopTtiConfigSet
(
    GT_VOID
)
{
    GT_STATUS                rc = GT_OK;
    PRV_TGF_TTI_ACTION_2_STC ttiAction;
    PRV_TGF_TTI_RULE_UNT     ttiPattern;
    PRV_TGF_TTI_RULE_UNT     ttiMask;
    PRV_TGF_TTI_KEY_TYPE_ENT       tunnelPacketType = PRV_TGF_TTI_KEY_ETH_E;

    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);

    /* AUTODOC: set the TTI Rule Pattern, Mask and Action for TCAM location */

    cpssOsMemSet(&ttiAction, 0, sizeof(ttiAction));
    cpssOsMemSet(&ttiMask, 0, sizeof(ttiMask));
    cpssOsMemSet(&ttiPattern, 0, sizeof(ttiPattern));


    /* AUTODOC: set TTI Action */
    /* AUTODOC:   cmd FORWARD, assign source ePort */
    ttiAction.command                           = CPSS_PACKET_CMD_FORWARD_E;
    ttiAction.tag0VlanCmd                       = PRV_TGF_TTI_VLAN_MODIFY_UNTAGGED_E;
    ttiAction.tag0VlanPrecedence                = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
    ttiAction.sourceEPortAssignmentEnable       = GT_TRUE;
    ttiAction.sourceEPort                       = PRV_TGF_SRC_EPORT_CNS;

    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    /* AUTODOC: set the TTI Rule Pattern, Mask and Action for TTI TCAM location */
    /* AUTODOC: rule is empty and matches all packets */
    ttiMask.eth.common.pclId    = 0x3FF;
    ttiPattern.eth.common.pclId = PRV_TGF_TTI_ID_CNS;

    /* AUTODOC: enable the TTI lookup for given packet type at the port */
    rc = prvTgfTtiPortLookupEnableSet(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        tunnelPacketType, GT_TRUE);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfTtiPortLookupEnableSet");

    /* save PCL id */
    rc = prvTgfTtiPclIdGet(
        prvTgfDevNum, tunnelPacketType, &savePclId);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfTtiPclIdGet");

    rc = prvTgfTtiPclIdSet(
        prvTgfDevNum, tunnelPacketType, PRV_TGF_TTI_ID_CNS);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfTtiPclIdSet");

    rc = prvTgfTtiRule2Set(
        PRV_TGF_TTI_RULE_IDX_CNS, tunnelPacketType,
        &ttiPattern, &ttiMask, &ttiAction);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfTtiRule2Set");

    rc = prvTgfPclEPortIngressPolicyEnable(prvTgfDevNum, PRV_TGF_SRC_EPORT_CNS , GT_TRUE);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfPclEPortIngressPolicyEnable");

    return GT_OK;
}

static void prvTgfBrgTpidConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U32  tpid0Index = 1;
    GT_U32  tpid1Index = 7;
    TGF_ETHER_TYPE tpid1 = prvTgfPacketVlanTag0Part.etherType;
    GT_PORT_NUM  portNum = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    GT_PORT_NUM  newEport = PRV_TGF_SRC_EPORT_CNS;
    GT_U32  profile0 = tpid0Index;
    GT_U32  profile1 = tpid1Index;

/*
    1. set TPID of tag 1 into tpid table index 7.
    2. bind ingress port tag 0 to profile 1
       bind ingress port tag 1 to profile 7
       bind new eport    tag 0 to profile 1
       bind new eport    tag 1 to profile 0
    3. set tag 0 profile 1 with bmp of index 1
       set tag 1 profile 7 with bmp of index 7
       set tag 1 profile 0 with EMPTRY bmp
*/
/*
    1. set TPID of tag 1 into tpid table index 7.
*/
    /* set TPID table index 7 for the tag1 recognition */
    rc = prvTgfBrgVlanTpidEntrySet(CPSS_DIRECTION_INGRESS_E, tpid1Index,tpid1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"prvTgfBrgVlanTpidEntrySet: 0x%x",tpid1);


/*
    2. bind ingress port tag 0 to profile 1
       bind ingress port tag 1 to profile 7
       bind new eport    tag 0 to profile 1
       bind new eport    tag 1 to profile 0
*/
    /* set pre-tti-lookup-ingress-eport table */
    rc = prvTgfBrgVlanPortIngressTpidProfileSet(prvTgfDevNum,portNum,
                CPSS_VLAN_ETHERTYPE0_E,GT_TRUE,/* pre-tti lookup */profile0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"prvTgfBrgVlanPortIngressTpidProfileGet: 0x%x",portNum);
    /* set port-tti-lookup-ingress-eport table */
    rc = prvTgfBrgVlanPortIngressTpidProfileSet(prvTgfDevNum,newEport,
                CPSS_VLAN_ETHERTYPE0_E,GT_FALSE,/* post-tti lookup */profile0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"prvTgfBrgVlanPortIngressTpidProfileGet: 0x%x",portNum);

    /* set pre-tti-lookup-ingress-eport table */
    rc = prvTgfBrgVlanPortIngressTpidProfileSet(prvTgfDevNum,portNum,
                CPSS_VLAN_ETHERTYPE1_E,GT_TRUE,/* pre-tti lookup */profile1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"prvTgfBrgVlanPortIngressTpidProfileGet: 0x%x",portNum);
    /* set port-tti-lookup-ingress-eport table */
    rc = prvTgfBrgVlanPortIngressTpidProfileSet(prvTgfDevNum,newEport,
                CPSS_VLAN_ETHERTYPE1_E,GT_FALSE,/* post-tti lookup */0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"prvTgfBrgVlanPortIngressTpidProfileGet: 0x%x",portNum);

/*
    3. set tag 0 profile 1 with bmp of index 1
       set tag 1 profile 7 with bmp of index 7
       set tag 1 profile 0 with EMPTRY bmp
*/

    rc = prvTgfBrgVlanIngressTpidProfileSet(prvTgfDevNum,profile0,CPSS_VLAN_ETHERTYPE0_E,1<<tpid0Index);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"prvTgfBrgVlanIngressTpidProfileGet: 0x%x",1<<tpid0Index);

    rc = prvTgfBrgVlanIngressTpidProfileSet(prvTgfDevNum,profile1,CPSS_VLAN_ETHERTYPE1_E,1<<tpid1Index);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"prvTgfBrgVlanIngressTpidProfileGet: 0x%x",1<<tpid1Index);

    rc = prvTgfBrgVlanIngressTpidProfileSet(prvTgfDevNum,0,CPSS_VLAN_ETHERTYPE1_E,0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"prvTgfBrgVlanIngressTpidProfileGet: 0x%x",0);

}

static void prvTgfBrgTpidConfigRestore
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U32  tpid0Index = 1;
    GT_U32  tpid1Index = 7;
    TGF_ETHER_TYPE tpid1 = prvTgfPacketVlanTag0Part.etherType;
    GT_PORT_NUM  portNum = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    GT_PORT_NUM  newEport = PRV_TGF_SRC_EPORT_CNS;
    GT_U32  profile0 = tpid0Index;
    GT_U32  profile1 = tpid1Index;

    /* set TPID table index 7 for the tag1 recognition */
    rc = prvTgfBrgVlanTpidEntrySet(CPSS_DIRECTION_INGRESS_E, tpid1Index,0x8100);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"prvTgfBrgVlanTpidEntrySet: 0x%x",tpid1);

    /* set pre-tti-lookup-ingress-eport table */
    rc = prvTgfBrgVlanPortIngressTpidProfileSet(prvTgfDevNum,portNum,
                CPSS_VLAN_ETHERTYPE0_E,GT_TRUE,/* pre-tti lookup */0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"prvTgfBrgVlanPortIngressTpidProfileGet: 0x%x",portNum);
    /* set port-tti-lookup-ingress-eport table */
    rc = prvTgfBrgVlanPortIngressTpidProfileSet(prvTgfDevNum,newEport,
                CPSS_VLAN_ETHERTYPE0_E,GT_FALSE,/* post-tti lookup */0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"prvTgfBrgVlanPortIngressTpidProfileGet: 0x%x",portNum);

    /* set pre-tti-lookup-ingress-eport table */
    rc = prvTgfBrgVlanPortIngressTpidProfileSet(prvTgfDevNum,portNum,
                CPSS_VLAN_ETHERTYPE1_E,GT_TRUE,/* pre-tti lookup */0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"prvTgfBrgVlanPortIngressTpidProfileGet: 0x%x",portNum);
    /* set port-tti-lookup-ingress-eport table */
    rc = prvTgfBrgVlanPortIngressTpidProfileSet(prvTgfDevNum,newEport,
                CPSS_VLAN_ETHERTYPE1_E,GT_FALSE,/* post-tti lookup */0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"prvTgfBrgVlanPortIngressTpidProfileGet: 0x%x",portNum);

    rc = prvTgfBrgVlanIngressTpidProfileSet(prvTgfDevNum,profile0,CPSS_VLAN_ETHERTYPE0_E,0xff);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"prvTgfBrgVlanIngressTpidProfileGet: 0x%x",0xff);

    rc = prvTgfBrgVlanIngressTpidProfileSet(prvTgfDevNum,profile1,CPSS_VLAN_ETHERTYPE1_E,0xff);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"prvTgfBrgVlanIngressTpidProfileGet: 0x%x",0xff);

    rc = prvTgfBrgVlanIngressTpidProfileSet(prvTgfDevNum,0,CPSS_VLAN_ETHERTYPE1_E,0xff);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"prvTgfBrgVlanIngressTpidProfileGet: 0x%x",0xff);

}

/**
* @internal prvTgfBrgVlanTagPopTtiConfigReset function
* @endinternal
*
* @brief   invalidate TTI Basic rule
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS prvTgfBrgVlanTagPopTtiConfigReset
(
    GT_VOID
)
{
    GT_STATUS   rc, rc1 = GT_OK;
    PRV_TGF_TTI_KEY_TYPE_ENT       tunnelPacketType = PRV_TGF_TTI_KEY_ETH_E;

    PRV_UTF_LOG0_MAC("======= Resetting TTI Configuration =======\n");

    /* AUTODOC: Invalidate the TTI Rule */

    /* AUTODOC: disable the TTI lookup for given packet type at the port */
    rc = prvTgfTtiPortLookupEnableSet(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        tunnelPacketType, GT_FALSE);
    PRV_UTF_VERIFY_LOG_NO_STOP_MAC(
        rc1, rc, "prvTgfTtiPortLookupEnableSet")

    /* restore PCL id */
    rc = prvTgfTtiPclIdSet(
        prvTgfDevNum, tunnelPacketType, savePclId);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfTtiPclIdSet");

    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI_RULE_IDX_CNS, GT_FALSE);
    PRV_UTF_VERIFY_LOG_NO_STOP_MAC(
        rc1, rc, "prvTgfTtiRuleValidStatusSet")

    rc = prvTgfPclEPortIngressPolicyEnable(prvTgfDevNum, PRV_TGF_SRC_EPORT_CNS , GT_FALSE);
    PRV_UTF_VERIFY_LOG_NO_STOP_MAC(
        rc1, rc, "prvTgfPclEPortIngressPolicyEnable")

    prvTgfTtiTcamIndexIsRelativeSet(GT_FALSE);

    return rc1;
}

/**
* @internal prvTgfBrgVlanTagPopTtiTest_popTag1 function
* @endinternal
*
* @brief   Test on VLAN Tag Ingress Pop outer Tag1 bytes.
*          the inner tag 0 is ignored by egress port that configured 'nested vlan'.
*          so an egress port that is defined 'untagged' still need to egress the port with 'tag 0'.
*
*          None
*/
GT_VOID prvTgfBrgVlanTagPopTtiTest_popTag1
(
    void
)
{
    GT_STATUS rc;
    GT_U32  srcEPort = 0xFFFF;
    GT_U32  ingressPacketLength;/*length of ingress packet (including CRC) */
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;
    GT_CHAR*    namesPtr[] = {"pop-4-bytes outer Tag1"};
    GT_U32  numBytesNumeric;/* number of bytes to pop (numeric ... not ENUM) */

    numBytesNumeric = 4;

    PRV_UTF_LOG1_MAC("test [%s] with TTI action + L2 re-parsing for tags after the pop \n",
        namesPtr[0]);

    /* AUTODOC: set VLANs, FDB, Capture */
    rc = prvTgfBrgVlanTagPopPortVlanFdbSet();
    if (rc != GT_OK) goto label_restore;

    /* AUTODOC: TTI rule to set source ePort */
    rc = prvTgfBrgVlanTagPopTtiConfigSet();
    if (rc != GT_OK) goto label_restore;

    prvTgfBrgTpidConfigSet();

    srcEPort = PRV_TGF_SRC_EPORT_CNS;

    /* AUTODOC: set the egress port as nested vlan */
    rc = prvTgfBrgNestVlanPortTargetEnableSet(prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS],
        GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfBrgNestVlanPortTargetEnableSet: %d",srcEPort);

    /* AUTODOC: set needed number of Tag Bytes to Pop */
    rc = prvTgfBrgVlanPortNumOfTagWordsToPopSet(
        prvTgfDevNum, srcEPort, 1/*pop single word*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfBrgVlanPortNumOfTagWordsToPopSet: %s",namesPtr[0]);

    /* reset egress port counters before sending packet */
    rc = prvTgfResetCountersEth(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfResetCountersEth: %d, %d",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);

    /* AUTODOC: send packet */
    prvTgfBrgVlanTagPopTrafficGenerate(&prvTgfTaggedPacketInfo);

    /* calculate the number of bytes in the packet that we sent */
    rc = tgfTrafficEnginePacketLengthCalc(&prvTgfTaggedPacketInfo,&ingressPacketLength);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "tgfTrafficEnginePacketLengthCalc: rc = %d",
                                 rc);

    /* read egress port counters */
    rc = prvTgfReadPortCountersEth(prvTgfDevNum,
                                   prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS],
                                   GT_TRUE, &portCntrs);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfReadPortCountersEth: %d, %d",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);
    /*check that single packet egress the proper port */
    UTF_VERIFY_EQUAL1_STRING_MAC(1, portCntrs.goodPktsRcv.l[0],
                 "get another goodPktsRcv counter than expected on port %d",
                            prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);

    /* we expect to remove x tags that are 4x bytes */
    UTF_VERIFY_EQUAL1_STRING_MAC((ingressPacketLength - numBytesNumeric), portCntrs.goodOctetsRcv.l[0],
                 "get another goodPktsRcv counter than expected on port %d",
                            prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);

    /* Checks traffic egress with single tag0 although poped one tag and
        in vlan egress port defined as 'untagged'*/
    checkEgressTagsAfterPopWithNestedVlan();

    /* AUTODOC: restore the egress port as non-nested vlan */
    rc = prvTgfBrgNestVlanPortTargetEnableSet(prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS],
        GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfBrgNestVlanPortTargetEnableSet: %d",srcEPort);

    label_restore:

    prvTgfBrgTpidConfigRestore();

    if(srcEPort != 0xFFFF)
    {
        /* AUTODOC: restore to 0 Tag Bytes to Pop */
        rc = prvTgfBrgVlanPortNumOfTagWordsToPopSet(
            prvTgfDevNum, srcEPort, PRV_TGF_BRG_VLAN_NUM_BYTES_TO_POP_NONE_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"prvTgfBrgVlanPortNumOfTagWordsToPopSet");
    }

    /* AUTODOC: TTI rule to set source ePort */
    prvTgfBrgVlanTagPopTtiConfigReset();

    /* AUTODOC: reset VLANs, FDB, Capture */
    prvTgfBrgVlanTagPopPortVlanFdbReset();
}



