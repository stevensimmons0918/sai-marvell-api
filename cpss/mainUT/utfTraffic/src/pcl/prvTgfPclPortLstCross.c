/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvTgfPclPortListCross.c
*
* DESCRIPTION:
*       Specific PCL features testing
*
* FILE REVISION NUMBER:
*       $Revision: 6 $
*
*******************************************************************************/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/generic/pcl/cpssPcl.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfPclGen.h>
#include <common/tgfPortGen.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfCosGen.h>
#include <common/tgfCncGen.h>
#include <common/tgfCscdGen.h>
#include <pcl/prvTgfPclMiscellanous.h>

/* There are two tests: Ingress PCL test and Egress PCL test              */

/* The Ingress PCL test:                                                  */
/* All prvTgfPortsNum ports add to VLANS 10,11,12,13 as taggged           */
/* prvTgfPortsNum rules matching source port using port list field        */
/* Each rule updates the VID                                              */
/* Each of prvTgfPortsNum ports configured for Ingress PCL                */
/* The test sends packet to each of 4 ports                               */
/* The packet modified VID hecked using egress port capture               */

/* The Egress PCL test:                                                   */
/* All prvTgfPortsNum ports add to VLANS 10,11,12,13 as taggged           */
/* prvTgfPortsNum rules matching target port using port list field        */
/* Each rule updates the VID                                              */
/* Each of prvTgfPortsNum ports configured for Egress PCL                 */
/* The test sends packet to exit from each of 4 ports                     */
/* The packet modified VID hecked using egress port capture               */

/* source VLAN Id */
#define PRV_TGF_SRC_VLANID_CNS             2

/* base of VLAN Ids */
#define PRV_TGF_VLANID_BASE_CNS            10
/* base of rule indexes */
#define PRV_TGF_RULE_INDEX_BASE_CNS         3

/* use extended or standard rule for testing */
#define PRV_TGF_RULE_EXT_USED_CNS 1

#if (PRV_TGF_RULE_EXT_USED_CNS)

#define PRV_TGF_RULE_SIZE_CNS CPSS_PCL_RULE_SIZE_EXT_E

#define PRV_TGF_RULE_IPCL_FORMAT_NOT_IP_CNS \
    PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E

#define PRV_TGF_RULE_IPCL_FORMAT_IPV4_CNS \
    PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E

#define PRV_TGF_RULE_IPCL_FORMAT_IPV6_CNS \
    PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E

#define PRV_TGF_RULE_IPCL_UNION_MEMBER_CNS ruleExtNotIpv6

#define PRV_TGF_RULE_EPCL_FORMAT_NOT_IP_CNS \
    PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E

#define PRV_TGF_RULE_EPCL_FORMAT_IPV4_CNS \
    PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E

#define PRV_TGF_RULE_EPCL_FORMAT_IPV6_CNS \
    PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_E

#define PRV_TGF_RULE_EPCL_UNION_MEMBER_CNS ruleEgrExtNotIpv6

#else /*(PRV_TGF_RULE_EXT_USED_CNS)*/

#define PRV_TGF_RULE_SIZE_CNS CPSS_PCL_RULE_SIZE_STD_E

#define PRV_TGF_RULE_IPCL_FORMAT_NOT_IP_CNS \
    PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E

#define PRV_TGF_RULE_IPCL_FORMAT_IPV4_CNS \
    PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E

#define PRV_TGF_RULE_IPCL_FORMAT_IPV6_CNS \
    PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E

#define PRV_TGF_RULE_IPCL_UNION_MEMBER_CNS ruleStdNotIp

#define PRV_TGF_RULE_EPCL_FORMAT_NOT_IP_CNS \
    PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E

#define PRV_TGF_RULE_EPCL_FORMAT_IPV4_CNS \
    PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E

#define PRV_TGF_RULE_EPCL_FORMAT_IPV6_CNS \
    PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E

#define PRV_TGF_RULE_EPCL_UNION_MEMBER_CNS ruleEgrStdNotIp


#endif /*PRV_TGF_RULE_EXT_USED_CNS*/


/******************************************************************************\
 *                            Private variables                               *
\******************************************************************************/

/* stored default VID */
static GT_U16   prvTgfDefVlanId = 0;

/* store port mapping */
static GT_BOOL portListPortMappingEnable = GT_TRUE;
static GT_U32 portListPortMappingGroup   = 0;
static GT_U32 portListPortMappingOffset  = 0x1F;


/*********************** packet (Eth Other packet) ************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC packet1TgfL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x55}                 /* saMac */
};

static TGF_PACKET_VLAN_TAG_STC packet1VlanTag =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,
    0, /* tag UP */
    0, /* tag CFI*/
    PRV_TGF_SRC_VLANID_CNS /* VID */
};

/* ethertype part of packet1 */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacket1EtherTypePart =
    {0x3333};

/* DATA of packet */
static GT_U8 packet1TgfPayloadDataArr[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC packet1TgfPayloadPart = {
    sizeof(packet1TgfPayloadDataArr),                 /* dataLength */
    packet1TgfPayloadDataArr                          /* dataPtr */
};

/* PARTS of packet1 */
static TGF_PACKET_PART_STC packet1TgfPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &packet1TgfL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E , &packet1VlanTag},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacket1EtherTypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &packet1TgfPayloadPart}
};

/* PACKET1 to send */
static TGF_PACKET_STC packet1TgfInfo = {
    (TGF_L2_HEADER_SIZE_CNS
     + TGF_VLAN_TAG_SIZE_CNS
     + TGF_ETHERTYPE_SIZE_CNS
     + sizeof(packet1TgfPayloadDataArr)),                        /* totalLen */
    (sizeof(packet1TgfPartArray) / sizeof(TGF_PACKET_PART_STC)), /* numOfParts */
    packet1TgfPartArray                                          /* partsArray */
};

/* ethertype part of packet1 */
static TGF_PACKET_ETHERTYPE_STC packet1TgfIpv4TypePart =
    {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};
static TGF_PACKET_IPV4_STC packet1TgfIpv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    (sizeof(packet1TgfPayloadDataArr) + 20),               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0x04,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS, /* csum */
    { 2,  2,  2,  2},   /* srcAddr */
    { 1,  1,  1,  3}    /* dstAddr */
};
/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC packet1TgfIpv6TypePart = {
    TGF_ETHERTYPE_86DD_IPV6_TAG_CNS
};
/* packet's IPv6 */
static TGF_PACKET_IPV6_STC packet1TgfIpv6Part = {
    6,                  /* version */
    0,                  /* trafficClass */
    0,                  /* flowLabel */
    sizeof(packet1TgfPayloadDataArr),               /* payloadLen */
    0x00,               /* nextHeader */
    0x40,               /* hopLimit */
    {0x2222, 0, 0, 0, 0, 0, 0x0000, 0x2222}, /* TGF_IPV6_ADDR srcAddr */
    {0xff00, 0, 0, 0, 0, 0, 0x0000, 0x2222}  /* TGF_IPV6_ADDR dstAddr */
};


/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

/**
* @internal prvTgfPclPortListCrossCfgVlanSet function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfPclPortListCrossCfgVlanSet
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    i;

    static GT_U8     tagArray[256] = {0};
    if (tagArray[0] == 0)
    {
        cpssOsMemSet(tagArray, 0xFF, sizeof(tagArray));
    }

    /* get default vlanId */
    rc = prvTgfBrgVlanPortVidGet(
        prvTgfDevNum, prvTgfPortsArray[0], &prvTgfDefVlanId);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgVlanPortVidSet");

    /* AUTODOC: create VLAN 2 with all tagged ports */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(
        PRV_TGF_SRC_VLANID_CNS /*VID*/,
        prvTgfPortsArray, NULL, tagArray, prvTgfPortsNum);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgDefVlanEntryWithPortsSet");

    /* range of VIDs assigned by PCL */
    for (i = 0; (i < prvTgfPortsNum); i++)
    {
        /* AUTODOC: create VLANs [10..13] with all tagged ports */
        rc = prvTgfBrgDefVlanEntryWithPortsSet(
            (GT_U16)(i + PRV_TGF_VLANID_BASE_CNS),
            prvTgfPortsArray, NULL, tagArray, prvTgfPortsNum);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgDefVlanEntryWithPortsSet");
    }

    /* AUTODOC: set PVID 2 for all ports */
    rc = prvTgfBrgVlanPvidSet(PRV_TGF_SRC_VLANID_CNS);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgVlanPvidSet");

    return GT_OK;
};

/**
* @internal prvTgfPclPortListCrossCfgVlanRestore function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfPclPortListCrossCfgVlanRestore
(
    GT_VOID
)
{
    GT_STATUS rc, rc1 = GT_OK;
    GT_U32    i;

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgFdbFlush");

    /* AUTODOC: restore default vlanId to all ports */
    rc = prvTgfBrgVlanPvidSet(prvTgfDefVlanId);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgVlanPvidSet");

    /* invalidate vlan entry (and reset vlan entry) */
    for (i = 0; (i < prvTgfPortsNum); i++)
    {
        rc = prvTgfBrgDefVlanEntryInvalidate(
            (GT_U16)(i + PRV_TGF_VLANID_BASE_CNS));
        PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgDefVlanEntryInvalidate");
    }

    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_SRC_VLANID_CNS);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgDefVlanEntryInvalidate");

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_VERIFY_RC1(rc, "tgfTrafficTableRxPcktTblClear");


    TGF_PARAM_NOT_USED(rc1); /* prevent warning: not used variable */

    return GT_OK;
}

/**
* @internal prvTgfPclPortListCrossPortMappingSaveForRestore function
* @endinternal
*
* @brief   Save Port mapping for Port List mode for restore
*
* @param[in] direction                - PCL dircetion: IPCL or EPCL
*                                       None
*/
static GT_STATUS prvTgfPclPortListCrossPortMappingSaveForRestore
(
    CPSS_PCL_DIRECTION_ENT direction
)
{
    GT_STATUS rc = GT_OK;

    if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
    {
        /* AUTODOC: Save Port Mapping data for restore */
        rc = prvTgfPclPortListPortMappingGet(prvTgfDevNum,
                                             direction,
                                             prvTgfPortsArray[0],
                                             &portListPortMappingEnable,
                                             &portListPortMappingGroup,
                                             &portListPortMappingOffset);
    }

    if (GT_OK != rc)
    {
        PRV_UTF_LOG6_MAC("\n[TGF]: prvTgfPclPortListPortMappingGet FAILED,"
                         " rc = [%d], %d, %d, %d, %d, %d\n", rc,
                         direction,
                         prvTgfPortsArray[0],
                         portListPortMappingEnable,
                         portListPortMappingGroup,
                         portListPortMappingOffset);;
    }

    return rc;
}

/**
* @internal prvTgfPclPortListCrossPortMappingSet function
* @endinternal
*
* @brief   Set Port mapping for Port List mode
*
* @param[in] direction                - PCL dircetion: IPCL or EPCL
* @param[in] enable                   - port enabling for Port List
* @param[in] group                    - port  in Port List
* @param[in] offset                   - port  in Port List
*                                       None
*/
static GT_STATUS prvTgfPclPortListCrossPortMappingSet
(
    CPSS_PCL_DIRECTION_ENT direction,
    GT_BOOL                enable,
    GT_U32                 group,
    GT_U32                 offset
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    ii;
    GT_U32    bitOffset;

    for (ii = 0; (ii < prvTgfPortsNum); ii++)
    {
        if( 0x1F < offset)
        {
            bitOffset = ii;
        }
        else
        {
            bitOffset = offset;
        }

        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
        {
            rc = prvTgfPclPortListPortMappingSet(direction,
                                                 prvTgfPortsArray[ii],
                                                 enable,
                                                 group,
                                                 bitOffset);
        }

        if (GT_OK != rc)
        {
            PRV_UTF_LOG6_MAC("\n[TGF]: prvTgfPclPortListPortMappingSet FAILED,"
                             " rc = [%d], %d, %d, %d, %d, %d\n", rc,
                             direction,
                             prvTgfPortsArray[ii],
                             enable,
                             group,
                             bitOffset);
            return rc;
        }
    }

    return rc;
}

/**
* @internal prvTgfPclPortListCrossCfgIpclRestore function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfPclPortListCrossCfgIpclRestore
(
    GT_VOID
)
{
    GT_STATUS rc, rc1 = GT_OK;
    GT_U8     i;

    /* AUTODOC: disable ingress policy per device */
    rc = prvTgfPclIngressPolicyEnable(GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclIngressPolicyEnable");

    /* AUTODOC: disable using port-list in search keys for lookup0 */
    rc = prvTgfPclLookupCfgPortListEnableSet(
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        0 /*subLookupNum*/,
        GT_FALSE /*enable*/);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclLookupCfgPortListEnableSet");

    for (i = 0; (i < prvTgfPortsNum); i++)
    {
        /* AUTODOC: disable ingress policy on all ports */
        rc = prvTgfPclPortIngressPolicyEnable(
            prvTgfPortsArray[i], GT_FALSE);
        PRV_UTF_VERIFY_RC1(rc, "prvTgfPclPortIngressPolicyEnable");
    }

    /* AUTODOC: invalidate PCL rules */
    for (i = 0; (i < prvTgfPortsNum); i++)
    {
        rc = prvTgfPclRuleValidStatusSet(
            PRV_TGF_RULE_SIZE_CNS,
            (i + PRV_TGF_RULE_INDEX_BASE_CNS), GT_FALSE);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG2_MAC(
                "\n[TGF]: prvTgfPclRuleValidStatusSet FAILED,"
                " rc = [%d], pclId = %d\n", rc, i);
            rc1 = rc;
        }
    }

    /* AUTODOC: Restore Port List Port Mapping */
    rc1 = prvTgfPclPortListCrossPortMappingSet(CPSS_PCL_DIRECTION_INGRESS_E,
                                               portListPortMappingEnable,
                                               portListPortMappingGroup,
                                               portListPortMappingOffset);

    return rc1;
}

/**
* @internal prvTgfPclPortListCrossCfgEpclRestore function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfPclPortListCrossCfgEpclRestore
(
    GT_VOID
)
{
    GT_STATUS rc, rc1 = GT_OK;
    GT_U8     i;

    rc = prvTgfPclLookupCfgPortListEnableSet(
        CPSS_PCL_DIRECTION_EGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        0 /*subLookupNum*/,
        GT_FALSE /*enable*/);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclLookupCfgPortListEnableSet");

    for (i = 0; (i < prvTgfPortsNum); i++)
    {
        /* disable egress policy per devices */
        rc = prvTgfPclEgressPclPacketTypesSet(
            prvTgfDevNum, prvTgfPortsArray[i],
            PRV_TGF_PCL_EGRESS_PKT_NON_TS_E, GT_FALSE);
        PRV_UTF_VERIFY_RC1(rc, "prvTgfPclEgressPclPacketTypesSet");
    }

    /* AUTODOC: disables egress policy per devices */
    rc = prvTgfPclEgressPolicyEnable(GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclEgressPolicyEnable");

    /* invalidate PCL rules */
    for (i = 0; (i < prvTgfPortsNum); i++)
    {
        rc = prvTgfPclRuleValidStatusSet(
            PRV_TGF_RULE_SIZE_CNS,
            prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet(i + PRV_TGF_RULE_INDEX_BASE_CNS), GT_FALSE);
        PRV_UTF_VERIFY_RC1(rc, "prvTgfPclRuleValidStatusSet");
    }

    /* AUTODOC: Restore Port List Port Mapping */
    rc1 = prvTgfPclPortListCrossPortMappingSet(CPSS_PCL_DIRECTION_EGRESS_E,
                                               portListPortMappingEnable,
                                               portListPortMappingGroup,
                                               portListPortMappingOffset);

    return rc1;
}

/**
* @internal prvTgfPclPortListCrossTrafficGenerate function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfPclPortListCrossTrafficGenerate
(
    IN  TGF_PACKET_STC *packetInfoPtr,
    IN  GT_U32           portNumSend,
    IN  GT_U32           portNumReceive,
    IN  GT_U32          expectedVid,
    OUT GT_U32         *numTriggersPtr
)
{
    GT_STATUS               rc, rc1 = GT_OK;
    CPSS_INTERFACE_INFO_STC portInterface;
    TGF_VFD_INFO_STC        vfdArray[1];

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    PRV_UTF_LOG2_MAC(
        "sending packet: sendPort [%d] receivePort [%d] \n",
        portNumSend, portNumReceive);

    *numTriggersPtr = 0;

    /* reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfEthCountersReset");

    /* setup receive portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = portNumReceive;

    /* enable capture on receive port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
            &portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    PRV_UTF_VERIFY_GT_OK(rc, "tgftrafficgeneratorporttxethcaptureset");

    /* setup transmit params */
    rc = prvTgfSetTxSetupEth(
        prvTgfDevNum, packetInfoPtr,
        1 /*burstCount*/, 0 /*numVfd*/, NULL);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfSetTxSetupEth");

    /* start transmitting */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, portNumSend);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfStartTransmitingEth");

    cpssOsTimerWkAfter(100);

    /* disable capture on receive port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
            &portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "tgfTrafficGeneratorPortTxEthCaptureSet");

    /* check if there is captured packet with specified UP and DSCP fields */
    vfdArray[0].mode          = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset        = TGF_L2_HEADER_SIZE_CNS
                              + TGF_ETHERTYPE_SIZE_CNS; /*UP,CFI,VID*/
    vfdArray[0].cycleCount    = 2;
    vfdArray[0].patternPtr[0] = (GT_U8)((expectedVid >> 8) & 0x0F);
    vfdArray[0].patternPtr[1] = (GT_U8)(expectedVid & 0xFF);


    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portNumReceive);

    /* print captured packets and check TriggerCounters */
    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(
            &portInterface, 1, vfdArray, numTriggersPtr);
    if (GT_NO_MORE != rc && GT_OK != rc)
        rc1 = rc;

    /* reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfEthCountersReset");

    return rc1;
}

/**
* @internal prvTgfPclPortListCrossCfgIpclSet function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfPclPortListCrossCfgIpclSet
(
    IN GT_VOID
)
{
    GT_STATUS                         rc = GT_OK;
    static GT_BOOL                    isDeviceInited = GT_FALSE;
    CPSS_INTERFACE_INFO_STC           interfaceInfo;
    PRV_TGF_PCL_LOOKUP_CFG_STC        lookupCfg;
    CPSS_PCL_DIRECTION_ENT            direction;
    PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT accessMode;
    PRV_TGF_PCL_RULE_FORMAT_UNT       mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT       patt;
    PRV_TGF_PCL_ACTION_STC            action;
    GT_U32                            i;

    /* -------------------------------------------------------------------------
     * 1. Enable PCL
     */

    if (GT_FALSE == isDeviceInited)
    {
        /* init PCL */
        rc = prvTgfPclInit();
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclInit");

        /* do not init next time */
        isDeviceInited = GT_TRUE;
    }

    /* AUTODOC: enables ingress policy per devices */
    rc = prvTgfPclIngressPolicyEnable(GT_TRUE);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclIngressPolicyEnable");

    /* AUTODOC: enables ingress policy for all ports */
    for (i = 0; (i < prvTgfPortsNum); i++)
    {
        rc = prvTgfPclPortIngressPolicyEnable(
            prvTgfPortsArray[i], GT_TRUE);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclPortIngressPolicyEnable");
    }


    /* configure accessModes for lookups 0_0 */
    direction  = CPSS_PCL_DIRECTION_INGRESS_E;
    accessMode = PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E;

    /* set PCL configuration table for ports */
    cpssOsMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));
    interfaceInfo.type            = CPSS_INTERFACE_PORT_E;
    interfaceInfo.devPort.hwDevNum  = prvTgfDevNum;

    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));
    lookupCfg.enableLookup           = GT_TRUE;
    lookupCfg.pclId                  = 0;
    lookupCfg.dualLookup             = GT_FALSE;
    lookupCfg.pclIdL01               = 0;
    lookupCfg.groupKeyTypes.nonIpKey =
        PRV_TGF_RULE_IPCL_FORMAT_NOT_IP_CNS;
    lookupCfg.groupKeyTypes.ipv4Key  =
        PRV_TGF_RULE_IPCL_FORMAT_IPV4_CNS;
    lookupCfg.groupKeyTypes.ipv6Key  =
        PRV_TGF_RULE_IPCL_FORMAT_IPV6_CNS;

    for (i = 0; (i < prvTgfPortsNum); i++)
    {
        /* AUTODOC: configure lookup0 ingress entry by Port area 0,1,2,3 */
        rc = prvTgfPclPortLookupCfgTabAccessModeSet(
                prvTgfPortsArray[i], direction,
                CPSS_PCL_LOOKUP_0_E, 0 /*sublookup*/, accessMode);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclPortLookupCfgTabAccessModeSet");

        interfaceInfo.devPort.portNum = prvTgfPortsArray[i];

        /* AUTODOC: set PCL config table for ports 0,1,2,3 lookup0 with: */
        /* AUTODOC:   nonIpKey=INGRESS_EXT_NOT_IPV6 */
        /* AUTODOC:   ipv4Key=INGRESS_EXT_NOT_IPV6 */
        /* AUTODOC:   ipv6Key=INGRESS_EXT_IPV6_L2 */
        rc = prvTgfPclCfgTblSet(
            &interfaceInfo, direction, CPSS_PCL_LOOKUP_0_E, &lookupCfg);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclCfgTblSet 0");
    }

    /* AUTODOC: save Port List Port Mapping for restore */
    rc = prvTgfPclPortListCrossPortMappingSaveForRestore(
                                                CPSS_PCL_DIRECTION_INGRESS_E);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* AUTODOC: set Port List Port Mapping */
    rc = prvTgfPclPortListCrossPortMappingSet(CPSS_PCL_DIRECTION_INGRESS_E,
                                              GT_TRUE, 0, 0x20);
    if (GT_OK != rc)
    {
        return rc;
    }

    cpssOsMemSet(&mask,   0, sizeof(mask));
    cpssOsMemSet(&patt,   0, sizeof(patt));
    cpssOsMemSet(&action, 0, sizeof(action));

    action.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
    action.vlan.modifyVlan = CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;

    /* mirror-capturing                           */
    /* reflects packet using loopback             */
    /* replicates packet using ingress mirroring  */
    /* sets packet's VID==0 using PVID to drop it */
    /* PCL rules modifyind VID must check the     */
    /* previous packet VID to left it dropped     */
    mask.PRV_TGF_RULE_IPCL_UNION_MEMBER_CNS.common.vid = 0xFFF;
    patt.PRV_TGF_RULE_IPCL_UNION_MEMBER_CNS.common.vid =
        PRV_TGF_SRC_VLANID_CNS;

    for (i = 0; (i < prvTgfPortsNum); i++)
    {
        /* for each rule it's own VID */
        action.vlan.vlanId = (GT_U16)(PRV_TGF_VLANID_BASE_CNS + i);

        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
        {
            mask.PRV_TGF_RULE_IPCL_UNION_MEMBER_CNS.common.portListBmp.ports[0] =
                                                                        0xFFFFFFF;
            mask.PRV_TGF_RULE_IPCL_UNION_MEMBER_CNS.common.portListBmp.ports[0] &=
                                                                        (~ (1 << i));
        }
        else
        {
            /* port list bitmap pattern - all bits zeros                     */
            /* port list bitmap mask - all bits ones beside the matched port */
            cpssOsMemSet(
                &(mask.PRV_TGF_RULE_IPCL_UNION_MEMBER_CNS.common.portListBmp),
                0xFF, sizeof(CPSS_PORTS_BMP_STC));
            /* each rule matches it's own port */
            mask.PRV_TGF_RULE_IPCL_UNION_MEMBER_CNS.common.portListBmp
                .ports[prvTgfPortsArray[i] / 32] &=
                (~ (1 << (prvTgfPortsArray[i] % 32)));
        }

        /* AUTODOC: set PCL rules 3,4,5,6 with: */
        /* AUTODOC:   format INGRESS_EXT_NOT_IPV6, cmd=FORWARD */
        /* AUTODOC:   pattern VID=2 */
        /* AUTODOC:   enable modify VLAN, action VLANs=[10..13] */
        rc = prvTgfPclRuleWithOptionsSet(
            PRV_TGF_RULE_IPCL_FORMAT_NOT_IP_CNS,
            (PRV_TGF_RULE_INDEX_BASE_CNS + i),
            0 /*ruleOptionsBmp*/,
            &mask, &patt, &action);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclRuleSet");
    }

    /* AUTODOC: enable ingress using port-list in search keys for lookup0 */
    rc = prvTgfPclLookupCfgPortListEnableSet(
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        0 /*subLookupNum*/,
        GT_TRUE /*enable*/);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclLookupCfgPortListEnableSet");

    return GT_OK;
}

/**
* @internal prvTgfPclPortListCrossCfgEpclSet function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfPclPortListCrossCfgEpclSet
(
    IN GT_VOID
)
{
    GT_STATUS                         rc = GT_OK;
    static GT_BOOL                    isDeviceInited = GT_FALSE;
    CPSS_INTERFACE_INFO_STC           interfaceInfo;
    PRV_TGF_PCL_LOOKUP_CFG_STC        lookupCfg;
    CPSS_PCL_DIRECTION_ENT            direction;
    PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT accessMode;
    PRV_TGF_PCL_RULE_FORMAT_UNT       mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT       patt;
    PRV_TGF_PCL_ACTION_STC            action;
    GT_U32                            i;

    /* -------------------------------------------------------------------------
     * 1. Enable PCL
     */

    if (GT_FALSE == isDeviceInited)
    {
        /* init PCL */
        rc = prvTgfPclInit();
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclInit");

        /* do not init next time */
        isDeviceInited = GT_TRUE;
    }

    /* AUTODOC: enables egress policy per devices */
    rc = prvTgfPclEgressPolicyEnable(GT_TRUE);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclEgressPolicyEnable");

    for (i = 0; (i < prvTgfPortsNum); i++)
    {
        /* AUTODOC: enables egress policy for all ports and non-tunneled packets */
        rc = prvTgfPclEgressPclPacketTypesSet(
            prvTgfDevNum, prvTgfPortsArray[i],
            PRV_TGF_PCL_EGRESS_PKT_NON_TS_E, GT_TRUE);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclEgressPclPacketTypesSet");
    }

    /* configure accessModes for lookups 0_0 */
    direction  = CPSS_PCL_DIRECTION_EGRESS_E;
    accessMode = PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E;

    /* set PCL configuration table for ports */
    cpssOsMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));
    interfaceInfo.type            = CPSS_INTERFACE_PORT_E;
    interfaceInfo.devPort.hwDevNum  = prvTgfDevNum;

    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));
    lookupCfg.enableLookup           = GT_TRUE;
    lookupCfg.pclId                  = 0;
    lookupCfg.dualLookup             = GT_FALSE;
    lookupCfg.pclIdL01               = 0;
    lookupCfg.groupKeyTypes.nonIpKey =
        PRV_TGF_RULE_EPCL_FORMAT_NOT_IP_CNS;
    lookupCfg.groupKeyTypes.ipv4Key  =
        PRV_TGF_RULE_EPCL_FORMAT_IPV4_CNS;
    lookupCfg.groupKeyTypes.ipv6Key  =
        PRV_TGF_RULE_EPCL_FORMAT_IPV6_CNS;

    for (i = 0; (i < prvTgfPortsNum); i++)
    {
        /* AUTODOC: configure lookup0 egress entry by Port area 0,1,2,3 */
        rc = prvTgfPclPortLookupCfgTabAccessModeSet(
                prvTgfPortsArray[i], direction,
                CPSS_PCL_LOOKUP_0_E, 0 /*sublookup*/, accessMode);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclPortLookupCfgTabAccessModeSet");

        interfaceInfo.devPort.portNum = prvTgfPortsArray[i];

        /* AUTODOC: set PCL config table for ports 0,1,2,3 lookup0 with: */
        /* AUTODOC:   nonIpKey=EGRESS_EXT_NOT_IPV6 */
        /* AUTODOC:   ipv4Key=EGRESS_EXT_NOT_IPV6 */
        /* AUTODOC:   ipv6Key=EGRESS_EXT_IPV6_L2 */
        rc = prvTgfPclCfgTblSet(
            &interfaceInfo, direction, CPSS_PCL_LOOKUP_0_E, &lookupCfg);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclCfgTblSet 0");
    }

    /* AUTODOC: save Port List Port Mapping for restore */
    rc = prvTgfPclPortListCrossPortMappingSaveForRestore(
                                                CPSS_PCL_DIRECTION_EGRESS_E);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* AUTODOC: set Port List Port Mapping */
    rc = prvTgfPclPortListCrossPortMappingSet(CPSS_PCL_DIRECTION_EGRESS_E,
                                              GT_TRUE, 0, 0x20);
    if (GT_OK != rc)
    {
        return rc;
    }

    cpssOsMemSet(&mask,   0, sizeof(mask));
    cpssOsMemSet(&patt,   0, sizeof(patt));
    cpssOsMemSet(&action, 0, sizeof(action));

    action.egressPolicy = GT_TRUE;
    action.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
    action.vlan.egressVlanId0Cmd =
        PRV_TGF_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E;

    for (i = 0; (i < prvTgfPortsNum); i++)
    {
        /* for each rule it's own VID */
        action.vlan.vlanId = (GT_U16)(PRV_TGF_VLANID_BASE_CNS + i);

        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
        {
            mask.PRV_TGF_RULE_EPCL_UNION_MEMBER_CNS.common.portListBmp.ports[0] =
                                                                        0xFFFFFFF;
            mask.PRV_TGF_RULE_EPCL_UNION_MEMBER_CNS.common.portListBmp.ports[0] &=
                                                                        (~ (1 << i));
        }
        else
        {
            /* port list bitmap pattern - all bits zeros                     */
            /* port list bitmap mask - all bits ones beside the matched port */
            cpssOsMemSet(
                &(mask.PRV_TGF_RULE_EPCL_UNION_MEMBER_CNS.common.portListBmp),
                0xFF, sizeof(CPSS_PORTS_BMP_STC));
            /* each rule matches it's own port */
            mask.PRV_TGF_RULE_EPCL_UNION_MEMBER_CNS.common.portListBmp
                .ports[prvTgfPortsArray[i] / 32] &=
                (~ (1 << (prvTgfPortsArray[i] % 32)));
        }

        /* AUTODOC: set PCL rules 3,4,5,6 with: */
        /* AUTODOC:   format EGRESS_EXT_NOT_IPV6, cmd=FORWARD */
        /* AUTODOC:   pattern VID=2 */
        /* AUTODOC:   enable modify VLAN, action VLANs=[10..13] */
        rc = prvTgfPclRuleWithOptionsSet(
            PRV_TGF_RULE_EPCL_FORMAT_NOT_IP_CNS,
            prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet(PRV_TGF_RULE_INDEX_BASE_CNS + i),
            0 /*ruleOptionsBmp*/,
            &mask, &patt, &action);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclRuleSet");
    }

    /* AUTODOC: enable egress using port-list in search keys for lookup0 */
    rc = prvTgfPclLookupCfgPortListEnableSet(
        CPSS_PCL_DIRECTION_EGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        0 /*subLookupNum*/,
        GT_TRUE /*enable*/);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPclLookupCfgPortListEnableSet");

    return GT_OK;
}

/**
* @internal prvTgfPclPortListCrossIpclCheck function
* @endinternal
*
* @brief   None
*/
static GT_VOID prvTgfPclPortListCrossIpclCheck
(
    IN  GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    numTriggers;
    GT_U32    expectedVid;
    GT_U32     sendPort;
    GT_U32     receivePort;
    GT_U32    i;

    for (i = 0; (i < prvTgfPortsNum); i++)
    {
        sendPort = prvTgfPortsArray[i];

        /* any port other from sendPort */
        receivePort = prvTgfPortsArray[(i + 1) % prvTgfPortsNum];

        expectedVid = (PRV_TGF_VLANID_BASE_CNS + i);

        /* AUTODOC: send 4 Ethernet packet on ports 0,1,2,3 with: */
        /* AUTODOC:   DA=00:00:00:00:34:03, SA=00:00:00:00:00:55 */
        /* AUTODOC:   VID=2, EtherType=0x3333 */
        rc = prvTgfPclPortListCrossTrafficGenerate(
            &packet1TgfInfo, sendPort, receivePort,
            expectedVid, &numTriggers);
        UTF_VERIFY_EQUAL3_STRING_MAC(
            GT_OK, rc,
            "prvTgfPclPortListModeTrafficGenerate: dev=%d ingr=%d, egr=%d",
            prvTgfDevNum, sendPort, receivePort);
        /* AUTODOC: verify traffic: */
        /* AUTODOC:   tagged 4 packets VID=[10..13] received on ports 1,2,3,0 */

        /* check triggers */
        UTF_VERIFY_EQUAL2_STRING_MAC(1, numTriggers,
            "tgfTrafficGeneratorPortTxEthTriggerCountersGet:\n"
            "   !!! Captured packet has wrong VID fields !!!\n"
            "   numTriggers = %d, rc = 0x%02X", numTriggers, rc);
    }
}

/**
* @internal prvTgfPclPortListCrossEpclCheck function
* @endinternal
*
* @brief   None
*/
static GT_VOID prvTgfPclPortListCrossEpclCheck
(
    IN  GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    numTriggers;
    GT_U32    expectedVid;
    GT_U32     sendPort;
    GT_U32     receivePort;
    GT_U32    i;

    for (i = 0; (i < prvTgfPortsNum); i++)
    {
        receivePort = prvTgfPortsArray[i];

        /* any port other from receivePort */
        sendPort = prvTgfPortsArray[(i + 1) % prvTgfPortsNum];

        expectedVid = (PRV_TGF_VLANID_BASE_CNS + i);

        /* AUTODOC: send 4 Ethernet packet on ports 1,2,3,0 with: */
        /* AUTODOC:   DA=00:00:00:00:34:03, SA=00:00:00:00:00:55 */
        /* AUTODOC:   VID=2, EtherType=0x3333 */
        rc = prvTgfPclPortListCrossTrafficGenerate(
            &packet1TgfInfo, sendPort, receivePort,
            expectedVid, &numTriggers);
        UTF_VERIFY_EQUAL3_STRING_MAC(
            GT_OK, rc,
            "prvTgfPclPortListModeTrafficGenerate: dev=%d ingr=%d, egr=%d",
            prvTgfDevNum, sendPort, receivePort);
        /* AUTODOC: verify traffic: */
        /* AUTODOC:   tagged 4 packets VID=[10..13] received on ports 0,1,2,3 */

        /* check triggers */
        UTF_VERIFY_EQUAL2_STRING_MAC(1, numTriggers,
            "tgfTrafficGeneratorPortTxEthTriggerCountersGet:\n"
            "   !!! Captured packet has wrong VID fields !!!\n"
            "   numTriggers = %d, rc = 0x%02X", numTriggers, rc);
    }
}

/* prvTgfPclPortListIpclFull */

/**
* @internal prvTgfPclPortListIpclFullSet function
* @endinternal
* @param[in] lookupNum        - PCL lookup number
* @param[in] portNum          - ingress port number
* @param[in] ruleFormat       - rule format
* @param[in] ruleIndex        - rule index
* @param[in] newVid           - new VID set by PCL action
*
* @brief   None
*/
static GT_STATUS prvTgfPclPortListIpclFullSet
(
    IN CPSS_PCL_LOOKUP_NUMBER_ENT       lookupNum,
    IN GT_PHYSICAL_PORT_NUM             portNum,
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat,
    IN GT_U32                           ruleIndex,
    IN GT_U32                           newVid
)
{
    GT_STATUS                         rc = GT_OK;
    CPSS_PCL_LOOKUP_NUMBER_ENT        cfgLookupNum;
    CPSS_INTERFACE_INFO_STC           interfaceInfo;
    PRV_TGF_PCL_LOOKUP_CFG_STC        lookupCfg;
    PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT accessMode;
    PRV_TGF_PCL_RULE_FORMAT_UNT       mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT       patt;
    PRV_TGF_PCL_ACTION_STC            action;
    GT_U16                            *maskPclId16Ptr;
    GT_U16                            *pattPclId16Ptr;
    GT_U32                            *maskPclId32Ptr;
    GT_U32                            *pattPclId32Ptr;
    CPSS_PORTS_BMP_STC                *maskPortListBmpPtr;
    CPSS_PORTS_BMP_STC                *pattPortListBmpPtr;

    /* -------------------------------------------------------------------------
     * 1. Enable PCL
     */

    /* AUTODOC: enables ingress policy per devices */
    rc = prvTgfPclIngressPolicyEnable(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_RETURN_RC_MAC(rc, GT_OK, "prvTgfPclIngressPolicyEnable");

    /* AUTODOC: enables ingress policy for all ports */
    rc = prvTgfPclPortIngressPolicyEnable(
        portNum, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_RETURN_RC_MAC(rc, GT_OK, "prvTgfPclPortIngressPolicyEnable");

    /* configure accessModes for lookups 0_0 */
    accessMode = PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E;

    /* set PCL configuration table for ports */
    cpssOsMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));
    interfaceInfo.type            = CPSS_INTERFACE_PORT_E;
    interfaceInfo.devPort.hwDevNum  = prvTgfDevNum;
    interfaceInfo.devPort.portNum   = portNum;

    rc = prvTgfPclPortLookupCfgTabAccessModeSet(
            portNum, CPSS_PCL_DIRECTION_INGRESS_E,
            lookupNum, 0 /*sublookup*/, accessMode);
    UTF_VERIFY_EQUAL0_STRING_RETURN_RC_MAC(rc, GT_OK, "prvTgfPclPortLookupCfgTabAccessModeSet");

    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));
    switch (lookupNum)
    {
        case CPSS_PCL_LOOKUP_0_E:
        case CPSS_PCL_LOOKUP_0_0_E:
        case CPSS_PCL_LOOKUP_1_E:
            lookupCfg.enableLookup           = GT_TRUE;
            lookupCfg.dualLookup             = GT_FALSE;
            lookupCfg.pclId                  = 0;
            lookupCfg.pclIdL01               = 0;
            break;
        case CPSS_PCL_LOOKUP_0_1_E:
            lookupCfg.enableLookup           = GT_TRUE;
            lookupCfg.dualLookup             = GT_TRUE;
            lookupCfg.pclId                  = 0x300; /* prevent matching by LOOKUP_0_0 */
            lookupCfg.pclIdL01               = 0;
            break;
        default:
            PRV_UTF_VERIFY_GT_OK(GT_FAIL, "wrong lookupNum");
            return GT_FAIL;
            break;
    }
    cfgLookupNum = lookupNum;
    if (! PRV_CPSS_DXCH_XCAT2_FAMILY_CHECK_MAC(prvTgfDevNum))
    {
        /* xCat3 and AC5 have common LOOKUP0 table */
        switch (cfgLookupNum)
        {
            case CPSS_PCL_LOOKUP_0_0_E:
            case CPSS_PCL_LOOKUP_0_1_E:
                cfgLookupNum  = CPSS_PCL_LOOKUP_0_E;
                break;
            default:
                break;
        }
    }
    lookupCfg.groupKeyTypes.nonIpKey =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    lookupCfg.groupKeyTypes.ipv4Key  =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;
    lookupCfg.groupKeyTypes.ipv6Key  =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;
    maskPclId16Ptr       = NULL;
    pattPclId16Ptr       = NULL;
    maskPclId32Ptr       = NULL;
    pattPclId32Ptr       = NULL;
    maskPortListBmpPtr   = NULL;
    pattPortListBmpPtr   = NULL;
    switch (ruleFormat)
    {
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E:
            lookupCfg.groupKeyTypes.nonIpKey =
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
            maskPclId16Ptr     = &(mask.ruleStdNotIp.common.pclId);
            pattPclId16Ptr     = &(patt.ruleStdNotIp.common.pclId);
            maskPortListBmpPtr = &(mask.ruleStdNotIp.common.portListBmp);
            pattPortListBmpPtr = &(patt.ruleStdNotIp.common.portListBmp);
            break;
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E:
            lookupCfg.groupKeyTypes.ipv4Key =
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;
            maskPclId16Ptr       = &(mask.ruleEgrStdIpL2Qos.common.pclId);
            pattPclId16Ptr       = &(patt.ruleEgrStdIpL2Qos.common.pclId);
            maskPortListBmpPtr = &(mask.ruleEgrStdIpL2Qos.common.portListBmp);
            pattPortListBmpPtr = &(patt.ruleEgrStdIpL2Qos.common.portListBmp);
            break;
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E:
            lookupCfg.groupKeyTypes.ipv4Key =
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E;
            maskPclId16Ptr     = &(mask.ruleStdIpv4L4.common.pclId);
            pattPclId16Ptr     = &(patt.ruleStdIpv4L4.common.pclId);
            maskPortListBmpPtr = &(mask.ruleStdIpv4L4.common.portListBmp);
            pattPortListBmpPtr = &(patt.ruleStdIpv4L4.common.portListBmp);
            break;
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E:
            lookupCfg.groupKeyTypes.nonIpKey =
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E;
            maskPclId16Ptr     = &(mask.ruleExtNotIpv6.common.pclId);
            pattPclId16Ptr     = &(patt.ruleExtNotIpv6.common.pclId);
            maskPortListBmpPtr = &(mask.ruleExtNotIpv6.common.portListBmp);
            pattPortListBmpPtr = &(patt.ruleExtNotIpv6.common.portListBmp);
            break;
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E:
            lookupCfg.groupKeyTypes.ipv6Key =
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E;
            maskPclId16Ptr     = &(mask.ruleExtIpv6L2.common.pclId);
            pattPclId16Ptr     = &(patt.ruleExtIpv6L2.common.pclId);
            maskPortListBmpPtr = &(mask.ruleExtIpv6L2.common.portListBmp);
            pattPortListBmpPtr = &(patt.ruleExtIpv6L2.common.portListBmp);
            break;
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E:
            lookupCfg.groupKeyTypes.ipv6Key =
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E;
            maskPclId16Ptr     = &(mask.ruleExtIpv6L4.common.pclId);
            pattPclId16Ptr     = &(patt.ruleExtIpv6L4.common.pclId);
            maskPortListBmpPtr = &(mask.ruleExtIpv6L4.common.portListBmp);
            pattPortListBmpPtr = &(patt.ruleExtIpv6L4.common.portListBmp);
            break;
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E:
            lookupCfg.groupKeyTypes.nonIpKey =
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E;
            maskPclId16Ptr     = &(mask.ruleStdUdb.commonStd.pclId);
            pattPclId16Ptr     = &(patt.ruleStdUdb.commonStd.pclId);
            maskPortListBmpPtr = &(mask.ruleStdUdb.commonStd .portListBmp);
            pattPortListBmpPtr = &(patt.ruleStdUdb.commonStd.portListBmp);
            break;
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E:
            lookupCfg.groupKeyTypes.nonIpKey =
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E;
            maskPclId16Ptr     = &(mask.ruleExtUdb.commonStd.pclId);
            pattPclId16Ptr     = &(patt.ruleExtUdb.commonStd.pclId);
            maskPortListBmpPtr = &(mask.ruleExtUdb.commonStd.portListBmp);
            pattPortListBmpPtr = &(patt.ruleExtUdb.commonStd.portListBmp);
            break;
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_ROUTED_ACL_QOS_E:
            lookupCfg.groupKeyTypes.ipv4Key =
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_ROUTED_ACL_QOS_E;
            maskPclId32Ptr     = &(mask.ruleStdIpv4RoutedAclQos.ingressIpCommon.pclId);
            pattPclId32Ptr     = &(patt.ruleStdIpv4RoutedAclQos.ingressIpCommon.pclId);
            maskPortListBmpPtr = &(mask.ruleStdIpv4RoutedAclQos.ingressIpCommon.portListBmp);
            pattPortListBmpPtr = &(patt.ruleStdIpv4RoutedAclQos.ingressIpCommon.portListBmp);
            break;
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV4_PORT_VLAN_QOS_E:
            lookupCfg.groupKeyTypes.ipv4Key =
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV4_PORT_VLAN_QOS_E;
            maskPclId32Ptr     = &(mask.ruleExtIpv4PortVlanQos.ingressIpCommon.pclId);
            pattPclId32Ptr     = &(patt.ruleExtIpv4PortVlanQos.ingressIpCommon.pclId);
            maskPortListBmpPtr = &(mask.ruleExtIpv4PortVlanQos.ingressIpCommon.portListBmp);
            pattPortListBmpPtr = &(patt.ruleExtIpv4PortVlanQos.ingressIpCommon.portListBmp);
            break;
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E:
            lookupCfg.groupKeyTypes.ipv6Key =
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E;
            maskPclId32Ptr     = &(mask.ruleUltraIpv6PortVlanQos.ingressIpCommon.pclId);
            pattPclId32Ptr     = &(patt.ruleUltraIpv6PortVlanQos.ingressIpCommon.pclId);
            maskPortListBmpPtr = &(mask.ruleUltraIpv6PortVlanQos.ingressIpCommon.portListBmp);
            pattPortListBmpPtr = &(patt.ruleUltraIpv6PortVlanQos.ingressIpCommon.portListBmp);
            break;
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_E:
            lookupCfg.groupKeyTypes.ipv6Key =
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_E;
            maskPclId32Ptr     = &(mask.ruleUltraIpv6RoutedAclQos.ingressIpCommon.pclId);
            pattPclId32Ptr     = &(patt.ruleUltraIpv6RoutedAclQos.ingressIpCommon.pclId);
            maskPortListBmpPtr = &(mask.ruleUltraIpv6RoutedAclQos.ingressIpCommon.portListBmp);
            pattPortListBmpPtr = &(patt.ruleUltraIpv6RoutedAclQos.ingressIpCommon.portListBmp);
            break;
        default:
            UTF_VERIFY_EQUAL0_STRING_RETURN_RC_MAC(GT_OK, GT_FAIL, "not suported ruleFormat");
            break;
    }

    rc = prvTgfPclCfgTblSet(
        &interfaceInfo, CPSS_PCL_DIRECTION_INGRESS_E, cfgLookupNum, &lookupCfg);
    UTF_VERIFY_EQUAL0_STRING_RETURN_RC_MAC(rc, GT_OK, "prvTgfPclCfgTblSet");

    cpssOsMemSet(&mask,   0, sizeof(mask));
    cpssOsMemSet(&patt,   0, sizeof(patt));
    cpssOsMemSet(&action, 0, sizeof(action));

    action.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
    action.vlan.modifyVlan = CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;
    action.vlan.vlanId = (GT_U16)newVid;

    /* important only for lookup 0_1 - to prevent 0_0 matching */
    /* PCL ID bits 9:8 of PCL ID not overlapped by port list   */
    if (maskPclId16Ptr)
    {
        *maskPclId16Ptr = 0x300;
        *pattPclId16Ptr = 0;
    }
    if (maskPclId32Ptr)
    {
        *maskPclId32Ptr = 0x300;
        *pattPclId32Ptr = 0;
    }
    /* port list - check all 28 bits */
    maskPortListBmpPtr->ports[0] = 0x0FFFFFFF;
    pattPortListBmpPtr->ports[0] = (1 << portNum);

    prvTgfPclDbLookupExtSet(CPSS_PCL_DIRECTION_INGRESS_E, lookupNum);
    rc = prvTgfPclRuleWithOptionsSet(
        ruleFormat, ruleIndex, 0 /*ruleOptionsBmp*/,
        &mask, &patt, &action);
   UTF_VERIFY_EQUAL0_STRING_RETURN_RC_MAC(rc, GT_OK, "prvTgfPclRuleSet");

    /* AUTODOC: enable ingress using port-list in search keys for lookup0 */
    rc = prvTgfPclLookupCfgPortListEnableSet(
        CPSS_PCL_DIRECTION_INGRESS_E, lookupNum,
        0 /*subLookupNum*/, GT_TRUE /*enable*/);
    UTF_VERIFY_EQUAL0_STRING_RETURN_RC_MAC(rc, GT_OK, "prvTgfPclLookupCfgPortListEnableSet");

    return GT_OK;
}

/**
* @internal prvTgfPclPortListIpclFullRestore function
* @endinternal
* @param[in] lookupNum        - PCL lookup number
* @param[in] portNum          - ingress port number
* @param[in] ruleSize         - rule size
* @param[in] ruleIndex        - rule index
*
* @brief   None
*/
static GT_STATUS prvTgfPclPortListIpclFullRestore
(
    IN CPSS_PCL_LOOKUP_NUMBER_ENT       lookupNum,
    IN GT_PHYSICAL_PORT_NUM             portNum,
    IN CPSS_PCL_RULE_SIZE_ENT           ruleSize,
    IN GT_U32                           ruleIndex
)
{
    GT_STATUS                         rc = GT_OK;
    GT_STATUS                         rc1 = GT_OK;
    CPSS_PCL_LOOKUP_NUMBER_ENT        cfgLookupNum;
    CPSS_INTERFACE_INFO_STC           interfaceInfo;
    PRV_TGF_PCL_LOOKUP_CFG_STC        lookupCfg;

    prvTgfPclDbLookupExtSet(CPSS_PCL_DIRECTION_INGRESS_E, lookupNum);
    rc = prvTgfPclRuleValidStatusSet(
        ruleSize, ruleIndex, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_SAVE_RC_MAC(GT_OK, rc, rc1, "prvTgfPclRuleValidStatusSet");

    /* set PCL configuration table for ports */
    cpssOsMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));
    interfaceInfo.type            = CPSS_INTERFACE_PORT_E;
    interfaceInfo.devPort.hwDevNum  = prvTgfDevNum;
    interfaceInfo.devPort.portNum   = portNum;
    cfgLookupNum = lookupNum;

    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));
    lookupCfg.groupKeyTypes.nonIpKey =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    lookupCfg.groupKeyTypes.ipv4Key  =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;
    lookupCfg.groupKeyTypes.ipv6Key  =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;

    if (! PRV_CPSS_DXCH_XCAT2_FAMILY_CHECK_MAC(prvTgfDevNum))
    {
        /* xCat3 and AC5 have common LOOKUP0 table */
        switch (cfgLookupNum)
        {
            case CPSS_PCL_LOOKUP_0_0_E:
            case CPSS_PCL_LOOKUP_0_1_E:
                cfgLookupNum  = CPSS_PCL_LOOKUP_0_E;
                break;
            default:
                break;
        }
    }
    rc = prvTgfPclCfgTblSet(
        &interfaceInfo, CPSS_PCL_DIRECTION_INGRESS_E, cfgLookupNum, &lookupCfg);
    UTF_VERIFY_EQUAL0_STRING_SAVE_RC_MAC(GT_OK, rc, rc1, "prvTgfPclCfgTblSet");

    rc = prvTgfPclPortIngressPolicyEnable(portNum, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_SAVE_RC_MAC(GT_OK, rc, rc1, "prvTgfPclPortIngressPolicyEnable");

    /* AUTODOC: enable ingress using port-list in search keys for lookup0 */
    rc = prvTgfPclLookupCfgPortListEnableSet(
        CPSS_PCL_DIRECTION_INGRESS_E, lookupNum,
        0 /*subLookupNum*/, GT_FALSE /*enable*/);
    UTF_VERIFY_EQUAL0_STRING_SAVE_RC_MAC(rc, GT_OK, rc1, "prvTgfPclLookupCfgPortListEnableSet");

    return rc1;
}

/**
* @internal prvTgfPclPortListIpclFullVlanSet function
* @endinternal
* @param[in] packetVid        - VID from tag of sent packets
* @param[in] newVid           - VID assigned to packet by IPCL
* @param[in] sendPortNum      - ingress port
* @param[in] receivePortNum   - egress port
*
* @brief   None
*/
static GT_STATUS prvTgfPclPortListIpclFullVlanSet
(
    IN GT_U32                           packetVid,
    IN GT_U32                           newVid,
    IN GT_PHYSICAL_PORT_NUM             sendPortNum,
    IN GT_PHYSICAL_PORT_NUM             receivePortNum
)
{
    GT_STATUS rc = GT_OK;
    GT_PHYSICAL_PORT_NUM portArr[2];
    GT_U8     tagArray[2];

    cpssOsMemSet(tagArray, 0xFF, sizeof(tagArray));
    portArr[0] = sendPortNum;
    portArr[1] = receivePortNum;

    rc = prvTgfBrgDefVlanEntryWithPortsSet(
        (GT_U16)packetVid/*VID*/,
        portArr, NULL, tagArray, 2 /*num of ports*/);
    UTF_VERIFY_EQUAL0_STRING_RETURN_RC_MAC(rc, GT_OK, "prvTgfBrgDefVlanEntryWithPortsSet");

    rc = prvTgfBrgDefVlanEntryWithPortsSet(
        (GT_U16)newVid/*VID*/,
        portArr, NULL, tagArray, 2 /*num of ports*/);
    UTF_VERIFY_EQUAL0_STRING_RETURN_RC_MAC(rc, GT_OK, "prvTgfBrgDefVlanEntryWithPortsSet");

    return GT_OK;
}

/**
* @internal prvTgfPclPortListIpclFullVlanRestore function
* @endinternal
* @param[in] packetVid        - VID from tag of sent packets
* @param[in] newVid           - VID assigned to packet by IPCL
* @param[in] sendPortNum      - ingress port
* @param[in] receivePortNum   - egress port
*
* @brief   None
*/
static GT_STATUS prvTgfPclPortListIpclFullVlanRestore
(
    IN GT_U32                           packetVid,
    IN GT_U32                           newVid
)
{
    GT_STATUS rc, rc1 = GT_OK;

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_SAVE_RC_MAC(GT_OK, rc, rc1, "prvTgfBrgFdbFlush");

    rc = prvTgfBrgDefVlanEntryInvalidate((GT_U16)packetVid);
    UTF_VERIFY_EQUAL0_STRING_SAVE_RC_MAC(GT_OK, rc, rc1, "prvTgfBrgDefVlanEntryInvalidate");

    rc = prvTgfBrgDefVlanEntryInvalidate((GT_U16)newVid);
    UTF_VERIFY_EQUAL0_STRING_SAVE_RC_MAC(GT_OK, rc, rc1, "prvTgfBrgDefVlanEntryInvalidate");

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_SAVE_RC_MAC(GT_OK, rc, rc1, "tgfTrafficTableRxPcktTblClear");

    return rc1;
}


/**
* @internal prvTgfPclPortListIpclFullTrafficSendAndCheck function
* @endinternal
* @param[in] packetVid        - VID from tag of sent packets
* @param[in] newVid           - VID assigned to packet by IPCL
* @param[in] sendPortNum      - ingress port
* @param[in] receivePortNum   - egress port
* @param[in] packetType       - 0 - ethernet, 1 - IPV4, 2 - IPV6
*
* @brief   None
*/
static GT_STATUS prvTgfPclPortListIpclFullTrafficSendAndCheck
(
    IN GT_U32                           packetVid,
    IN GT_U32                           newVid,
    IN GT_PHYSICAL_PORT_NUM             sendPortNum,
    IN GT_PHYSICAL_PORT_NUM             receivePortNum,
    IN GT_U32                           packetType
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    numTriggers;

    /* thise structures addresses used in initialization - should be static */
    /* this structure updated below - VID field */
    static TGF_PACKET_VLAN_TAG_STC localPktVlanTag =
    {
        TGF_ETHERTYPE_8100_VLAN_TAG_CNS,
        0, /* tag UP */
        0, /* tag CFI*/
        0 /* VID updated from parameters */
    };

    /* PARTS of ETH packet */
    static TGF_PACKET_PART_STC ethPktTgfPartArray[] = {
        {TGF_PACKET_PART_L2_E,        &packet1TgfL2Part},
        {TGF_PACKET_PART_VLAN_TAG_E , &localPktVlanTag},
        {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacket1EtherTypePart},
        {TGF_PACKET_PART_PAYLOAD_E,   &packet1TgfPayloadPart}
    };
    /* ETH PACKET to send */
    static TGF_PACKET_STC ethPktTgfInfo = {
        TGF_PACKET_AUTO_CALC_LENGTH_CNS,                            /* totalLen */
        (sizeof(ethPktTgfPartArray) / sizeof(TGF_PACKET_PART_STC)), /* numOfParts */
        ethPktTgfPartArray                                          /* partsArray */
    };

    /* PARTS of IPV4 packet */
    static TGF_PACKET_PART_STC ipv4PktTgfPartArray[] = {
        {TGF_PACKET_PART_L2_E,        &packet1TgfL2Part},
        {TGF_PACKET_PART_VLAN_TAG_E , &localPktVlanTag},
        {TGF_PACKET_PART_ETHERTYPE_E, &packet1TgfIpv4TypePart},
        {TGF_PACKET_PART_IPV4_E,      &packet1TgfIpv4Part},
        {TGF_PACKET_PART_PAYLOAD_E,   &packet1TgfPayloadPart}
    };

    /* IPV4 PACKET to send */
    static TGF_PACKET_STC ipv4PktTgfInfo = {
        TGF_PACKET_AUTO_CALC_LENGTH_CNS,                             /* totalLen */
        (sizeof(ipv4PktTgfPartArray) / sizeof(TGF_PACKET_PART_STC)), /* numOfParts */
        ipv4PktTgfPartArray                                          /* partsArray */
    };

    /* PARTS of IPV6 packet */
    static TGF_PACKET_PART_STC ipv6PktTgfPartArray[] = {
        {TGF_PACKET_PART_L2_E,        &packet1TgfL2Part},
        {TGF_PACKET_PART_VLAN_TAG_E , &localPktVlanTag},
        {TGF_PACKET_PART_ETHERTYPE_E, &packet1TgfIpv6TypePart},
        {TGF_PACKET_PART_IPV6_E,      &packet1TgfIpv6Part},
        {TGF_PACKET_PART_PAYLOAD_E,   &packet1TgfPayloadPart}
    };

    /* IPV6 PACKET to send */
    static TGF_PACKET_STC ipv6PktTgfInfo = {
        TGF_PACKET_AUTO_CALC_LENGTH_CNS,                             /* totalLen */
        (sizeof(ipv6PktTgfPartArray) / sizeof(TGF_PACKET_PART_STC)), /* numOfParts */
        ipv6PktTgfPartArray                                          /* partsArray */
    };

    TGF_PACKET_STC  *pktTgfInfoPtr;

    localPktVlanTag.vid = (GT_U16)packetVid;

    switch (packetType)
    {
        default:
        case 0: pktTgfInfoPtr = &ethPktTgfInfo; break;
        case 1: pktTgfInfoPtr = &ipv4PktTgfInfo; break;
        case 2: pktTgfInfoPtr = &ipv6PktTgfInfo; break;
    }

    rc = prvTgfPclPortListCrossTrafficGenerate(
        pktTgfInfoPtr, sendPortNum, receivePortNum,
        newVid, &numTriggers);
    UTF_VERIFY_EQUAL3_STRING_NO_RETURN_MAC(
        GT_OK, rc,
        "prvTgfPclPortListModeTrafficGenerate: dev=%d ingr=%d, egr=%d",
        prvTgfDevNum, sendPortNum, receivePortNum);
    if (rc != GT_OK) { return rc;}

    /* check triggers */
    UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(1, numTriggers,
        "tgfTrafficGeneratorPortTxEthTriggerCountersGet:\n"
        "   !!! Captured packet has wrong VID fields !!!\n"
        "   numTriggers = %d, rc = 0x%02X", numTriggers, rc);
    if (1 > numTriggers) { return GT_FAIL;}

    return GT_OK;
}

/**
* @internal prvTgfPclPortListIpclFullTrafficSendAndCheck function
* @endinternal
* @param[in] lookupNum        - PCL lookup number
* @param[in] sendPortNum      - ingress port
* @param[in] receivePortNum   - egress port
* @param[in] ruleFormat       - rule format
* @param[in] ruleIndex        - rule index
* @param[in] packetVid        - VID from tag of sent packets
* @param[in] newVid           - VID assigned to packet by IPCL
*
* @brief   None
*/
static GT_STATUS prvTgfPclPortListIpclFullTestCase
(
    IN CPSS_PCL_LOOKUP_NUMBER_ENT       lookupNum,
    IN GT_PHYSICAL_PORT_NUM             sendPortNum,
    IN GT_PHYSICAL_PORT_NUM             receivePortNum,
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat,
    IN GT_U32                           ruleIndex,
    IN GT_U32                           packetVid,
    IN GT_U32                           newVid
)
{
    GT_STATUS               rc = GT_OK;
    GT_STATUS               rc1 = GT_OK;
    CPSS_PCL_RULE_SIZE_ENT  ruleSize;
    GT_U32                  packetType;

    PRV_UTF_LOG0_MAC("\n********prvTgfPclPortListIpclFullTestCase\n");
    PRV_UTF_LOG3_MAC(
        "lookupNum %d sendPortNum %d receivePortNum %d\n",
        lookupNum, sendPortNum, receivePortNum);
    PRV_UTF_LOG4_MAC(
        "ruleFormat %d ruleIndex %d packetVid %d newVid %d\n",
        ruleFormat, ruleIndex, packetVid, newVid);
    switch (ruleFormat)
    {
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E:
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E:
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E:
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E:
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_ROUTED_ACL_QOS_E:
            ruleSize = CPSS_PCL_RULE_SIZE_STD_E;
            break;
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E:
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E:
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E:
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E:
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV4_PORT_VLAN_QOS_E:
            ruleSize = CPSS_PCL_RULE_SIZE_EXT_E;
            break;
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E:
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_E:
            ruleSize = CPSS_PCL_RULE_SIZE_ULTRA_E;
            break;
        default:
            ruleSize = CPSS_PCL_RULE_SIZE_STD_E;
            UTF_VERIFY_EQUAL0_STRING_RETURN_RC_MAC(GT_OK, GT_FAIL, "not suported ruleFormat");
            break;
    }
    switch (ruleFormat)
    {
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E:
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E:
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E:
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E:
            packetType = 0; /* ETH */
            break;
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E:
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E:
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_ROUTED_ACL_QOS_E:
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV4_PORT_VLAN_QOS_E:
            packetType = 1; /* IPV4 */
            break;
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E:
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E:
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E:
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_E:
            packetType = 2; /* IPV6 */
            break;
        default:
            packetType = 0; /* ETH */
            UTF_VERIFY_EQUAL0_STRING_RETURN_RC_MAC(GT_OK, GT_FAIL, "not suported ruleFormat");
            break;
    }

    rc = prvTgfPclPortListIpclFullVlanSet(
        packetVid, newVid, sendPortNum, receivePortNum);
    if (rc != GT_OK)
    {
        return rc;
    }
    rc = prvTgfPclPortListIpclFullSet(
        lookupNum, sendPortNum, ruleFormat, ruleIndex, newVid);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvTgfPclPortListIpclFullTrafficSendAndCheck(
        packetVid, newVid, sendPortNum, receivePortNum, packetType);
    UTF_VERIFY_EQUAL0_STRING_SAVE_RC_MAC(
        GT_OK, rc, rc1, "prvTgfPclPortListIpclFullTrafficSendAndCheck");

    rc = prvTgfPclPortListIpclFullRestore(
        lookupNum, sendPortNum, ruleSize, ruleIndex);
    UTF_VERIFY_EQUAL0_STRING_SAVE_RC_MAC(
        GT_OK, rc, rc1, "prvTgfPclPortListIpclFullRestore");

    rc = prvTgfPclPortListIpclFullVlanRestore(packetVid, newVid);
    UTF_VERIFY_EQUAL0_STRING_SAVE_RC_MAC(
        GT_OK, rc, rc1, "prvTgfPclPortListIpclFullVlanRestore");

    return rc1;
}

/* prvTgfPclPortListIpcl3Lookups */

/**
* @internal prvTgfPclPortListIpcl3LookupsCfgSet function
* @endinternal
* @param[in] portNum          - ingress port number
* @param[in] ruleFormat       - rule format
* @param[in] baseRuleIndex    - base index of 6 rules
* @param[in] baseCncIndex     - base index of 6 CNC counters
*
* @brief   Port enable, 3 CFG tables, 6 rules
*/
static GT_STATUS prvTgfPclPortListIpcl3LookupsCfgSet
(
    IN GT_PHYSICAL_PORT_NUM             portNum,
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat,
    IN GT_U32                           baseRuleIndex,
    IN GT_U32                           baseCncIndex
)
{
    GT_STATUS                         rc = GT_OK;
    CPSS_INTERFACE_INFO_STC           interfaceInfo;
    PRV_TGF_PCL_LOOKUP_CFG_STC        lookupCfg;
    PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT accessMode;
    PRV_TGF_PCL_RULE_FORMAT_UNT       mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT       patt;
    PRV_TGF_PCL_ACTION_STC            action;
    GT_U16                            *maskPclId16Ptr;
    GT_U16                            *pattPclId16Ptr;
    GT_U32                            *maskPclId32Ptr;
    GT_U32                            *pattPclId32Ptr;
    CPSS_PORTS_BMP_STC                *maskPortListBmpPtr;
    CPSS_PORTS_BMP_STC                *pattPortListBmpPtr;
    GT_U32                            i;
    static const CPSS_PCL_LOOKUP_NUMBER_ENT lookupArr[3] =
    {
        CPSS_PCL_LOOKUP_0_0_E, CPSS_PCL_LOOKUP_0_1_E, CPSS_PCL_LOOKUP_1_E
    };

    /* -------------------------------------------------------------------------
     * 1. Enable PCL
     */

    /* AUTODOC: enables ingress policy per devices */
    rc = prvTgfPclIngressPolicyEnable(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_RETURN_RC_MAC(rc, GT_OK, "prvTgfPclIngressPolicyEnable");

    /* AUTODOC: enables ingress policy for all ports */
    rc = prvTgfPclPortIngressPolicyEnable(
        portNum, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_RETURN_RC_MAC(rc, GT_OK, "prvTgfPclPortIngressPolicyEnable");

    /* configure accessModes for lookups 0_0 */
    accessMode = PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E;

    /* set PCL configuration table for ports */
    cpssOsMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));
    interfaceInfo.type            = CPSS_INTERFACE_PORT_E;
    interfaceInfo.devPort.hwDevNum  = prvTgfDevNum;
    interfaceInfo.devPort.portNum   = portNum;

    for (i = 0; (i < 3); i++)
    {
        rc = prvTgfPclPortLookupCfgTabAccessModeSet(
                portNum, CPSS_PCL_DIRECTION_INGRESS_E,
                lookupArr[i], 0 /*sublookup*/, accessMode);
        UTF_VERIFY_EQUAL0_STRING_RETURN_RC_MAC(rc, GT_OK, "prvTgfPclPortLookupCfgTabAccessModeSet");
    }

    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));

    lookupCfg.groupKeyTypes.nonIpKey =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    lookupCfg.groupKeyTypes.ipv4Key  =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;
    lookupCfg.groupKeyTypes.ipv6Key  =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;

    switch (ruleFormat)
    {
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E:
            lookupCfg.groupKeyTypes.nonIpKey =
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
            break;
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E:
            lookupCfg.groupKeyTypes.ipv4Key =
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;
            break;
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E:
            lookupCfg.groupKeyTypes.ipv4Key =
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E;
            break;
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E:
            lookupCfg.groupKeyTypes.nonIpKey =
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E;
            break;
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E:
            lookupCfg.groupKeyTypes.ipv6Key =
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E;
            break;
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E:
            lookupCfg.groupKeyTypes.ipv6Key =
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E;
            break;
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E:
            lookupCfg.groupKeyTypes.nonIpKey =
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E;
            break;
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E:
            lookupCfg.groupKeyTypes.nonIpKey =
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E;
            break;
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_ROUTED_ACL_QOS_E:
            lookupCfg.groupKeyTypes.ipv4Key =
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_ROUTED_ACL_QOS_E;
            break;
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV4_PORT_VLAN_QOS_E:
            lookupCfg.groupKeyTypes.ipv4Key =
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV4_PORT_VLAN_QOS_E;
            break;
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E:
            lookupCfg.groupKeyTypes.ipv6Key =
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E;
            break;
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_E:
            lookupCfg.groupKeyTypes.ipv6Key =
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_E;
            break;
        default:
            UTF_VERIFY_EQUAL0_STRING_RETURN_RC_MAC(GT_OK, GT_FAIL, "not suported ruleFormat");
            break;
    }

    lookupCfg.enableLookup           = GT_TRUE;
    lookupCfg.dualLookup             = GT_TRUE;
    lookupCfg.pclId                  = 0x1F1;
    lookupCfg.pclIdL01               = 0x2F2;

    rc = prvTgfPclCfgTblSet(
        &interfaceInfo, CPSS_PCL_DIRECTION_INGRESS_E, CPSS_PCL_LOOKUP_0_E, &lookupCfg);
    UTF_VERIFY_EQUAL0_STRING_RETURN_RC_MAC(rc, GT_OK, "prvTgfPclCfgTblSet");

    lookupCfg.enableLookup           = GT_TRUE;
    lookupCfg.dualLookup             = GT_FALSE;
    lookupCfg.pclId                  = 0x3F3;
    lookupCfg.pclIdL01               = 0;

    rc = prvTgfPclCfgTblSet(
        &interfaceInfo, CPSS_PCL_DIRECTION_INGRESS_E, CPSS_PCL_LOOKUP_1_E, &lookupCfg);
    UTF_VERIFY_EQUAL0_STRING_RETURN_RC_MAC(rc, GT_OK, "prvTgfPclCfgTblSet");

    maskPclId16Ptr       = NULL;
    pattPclId16Ptr       = NULL;
    maskPclId32Ptr       = NULL;
    pattPclId32Ptr       = NULL;
    maskPortListBmpPtr   = NULL;
    pattPortListBmpPtr   = NULL;
    switch (ruleFormat)
    {
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E:
            maskPclId16Ptr     = &(mask.ruleStdNotIp.common.pclId);
            pattPclId16Ptr     = &(patt.ruleStdNotIp.common.pclId);
            maskPortListBmpPtr = &(mask.ruleStdNotIp.common.portListBmp);
            pattPortListBmpPtr = &(patt.ruleStdNotIp.common.portListBmp);
            break;
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E:
            maskPclId16Ptr       = &(mask.ruleEgrStdIpL2Qos.common.pclId);
            pattPclId16Ptr       = &(patt.ruleEgrStdIpL2Qos.common.pclId);
            maskPortListBmpPtr = &(mask.ruleEgrStdIpL2Qos.common.portListBmp);
            pattPortListBmpPtr = &(patt.ruleEgrStdIpL2Qos.common.portListBmp);
            break;
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E:
            maskPclId16Ptr     = &(mask.ruleStdIpv4L4.common.pclId);
            pattPclId16Ptr     = &(patt.ruleStdIpv4L4.common.pclId);
            maskPortListBmpPtr = &(mask.ruleStdIpv4L4.common.portListBmp);
            pattPortListBmpPtr = &(patt.ruleStdIpv4L4.common.portListBmp);
            break;
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E:
            maskPclId16Ptr     = &(mask.ruleExtNotIpv6.common.pclId);
            pattPclId16Ptr     = &(patt.ruleExtNotIpv6.common.pclId);
            maskPortListBmpPtr = &(mask.ruleExtNotIpv6.common.portListBmp);
            pattPortListBmpPtr = &(patt.ruleExtNotIpv6.common.portListBmp);
            break;
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E:
            maskPclId16Ptr     = &(mask.ruleExtIpv6L2.common.pclId);
            pattPclId16Ptr     = &(patt.ruleExtIpv6L2.common.pclId);
            maskPortListBmpPtr = &(mask.ruleExtIpv6L2.common.portListBmp);
            pattPortListBmpPtr = &(patt.ruleExtIpv6L2.common.portListBmp);
            break;
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E:
            maskPclId16Ptr     = &(mask.ruleExtIpv6L4.common.pclId);
            pattPclId16Ptr     = &(patt.ruleExtIpv6L4.common.pclId);
            maskPortListBmpPtr = &(mask.ruleExtIpv6L4.common.portListBmp);
            pattPortListBmpPtr = &(patt.ruleExtIpv6L4.common.portListBmp);
            break;
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E:
            maskPclId16Ptr     = &(mask.ruleStdUdb.commonStd.pclId);
            pattPclId16Ptr     = &(patt.ruleStdUdb.commonStd.pclId);
            maskPortListBmpPtr = &(mask.ruleStdUdb.commonStd .portListBmp);
            pattPortListBmpPtr = &(patt.ruleStdUdb.commonStd.portListBmp);
            break;
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E:
            maskPclId16Ptr     = &(mask.ruleExtUdb.commonStd.pclId);
            pattPclId16Ptr     = &(patt.ruleExtUdb.commonStd.pclId);
            maskPortListBmpPtr = &(mask.ruleExtUdb.commonStd.portListBmp);
            pattPortListBmpPtr = &(patt.ruleExtUdb.commonStd.portListBmp);
            break;
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_ROUTED_ACL_QOS_E:
            maskPclId32Ptr     = &(mask.ruleStdIpv4RoutedAclQos.ingressIpCommon.pclId);
            pattPclId32Ptr     = &(patt.ruleStdIpv4RoutedAclQos.ingressIpCommon.pclId);
            maskPortListBmpPtr = &(mask.ruleStdIpv4RoutedAclQos.ingressIpCommon.portListBmp);
            pattPortListBmpPtr = &(patt.ruleStdIpv4RoutedAclQos.ingressIpCommon.portListBmp);
            break;
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV4_PORT_VLAN_QOS_E:
            maskPclId32Ptr     = &(mask.ruleExtIpv4PortVlanQos.ingressIpCommon.pclId);
            pattPclId32Ptr     = &(patt.ruleExtIpv4PortVlanQos.ingressIpCommon.pclId);
            maskPortListBmpPtr = &(mask.ruleExtIpv4PortVlanQos.ingressIpCommon.portListBmp);
            pattPortListBmpPtr = &(patt.ruleExtIpv4PortVlanQos.ingressIpCommon.portListBmp);
            break;
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E:
            maskPclId32Ptr     = &(mask.ruleUltraIpv6PortVlanQos.ingressIpCommon.pclId);
            pattPclId32Ptr     = &(patt.ruleUltraIpv6PortVlanQos.ingressIpCommon.pclId);
            maskPortListBmpPtr = &(mask.ruleUltraIpv6PortVlanQos.ingressIpCommon.portListBmp);
            pattPortListBmpPtr = &(patt.ruleUltraIpv6PortVlanQos.ingressIpCommon.portListBmp);
            break;
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_E:
            maskPclId32Ptr     = &(mask.ruleUltraIpv6RoutedAclQos.ingressIpCommon.pclId);
            pattPclId32Ptr     = &(patt.ruleUltraIpv6RoutedAclQos.ingressIpCommon.pclId);
            maskPortListBmpPtr = &(mask.ruleUltraIpv6RoutedAclQos.ingressIpCommon.portListBmp);
            pattPortListBmpPtr = &(patt.ruleUltraIpv6RoutedAclQos.ingressIpCommon.portListBmp);
            break;
        default:
            UTF_VERIFY_EQUAL0_STRING_RETURN_RC_MAC(GT_OK, GT_FAIL, "not suported ruleFormat");
            break;
    }

    cpssOsMemSet(&mask,   0, sizeof(mask));
    cpssOsMemSet(&patt,   0, sizeof(patt));
    cpssOsMemSet(&action, 0, sizeof(action));

    action.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
    action.matchCounter.enableMatchCount = GT_TRUE;
    /*action.matchCounter.matchCounterIndex - different in all rules */

    /* Rules0-3 Lookups0-2 port_list disable */

    /* port list - don't check all 28 bits */
    maskPortListBmpPtr->ports[0] = 0;
    pattPortListBmpPtr->ports[0] = 0;

    if (maskPclId16Ptr)
    {
        *maskPclId16Ptr = 0x3FF;
    }
    if (maskPclId32Ptr)
    {
        *maskPclId32Ptr = 0x3FF;
    }

    /* Rule0 Lookup_0_0 port_list disable */
    action.matchCounter.matchCounterIndex = (baseCncIndex + 0);

    if (maskPclId16Ptr)
    {
        *pattPclId16Ptr = 0x1F1;
    }
    if (maskPclId32Ptr)
    {
        *pattPclId32Ptr = 0x1F1;
    }

    prvTgfPclDbLookupExtSet(CPSS_PCL_DIRECTION_INGRESS_E, CPSS_PCL_LOOKUP_0_0_E);
    rc = prvTgfPclRuleWithOptionsSet(
        ruleFormat, (baseRuleIndex + 0), 0 /*ruleOptionsBmp*/,
        &mask, &patt, &action);
    UTF_VERIFY_EQUAL0_STRING_RETURN_RC_MAC(rc, GT_OK, "prvTgfPclRuleSet");

    /* Rule1 Lookup_0_1 port_list disable */
    action.matchCounter.matchCounterIndex = (baseCncIndex + 1);

    if (maskPclId16Ptr)
    {
       *pattPclId16Ptr = 0x2F2;
    }
    if (maskPclId32Ptr)
    {
       *pattPclId32Ptr = 0x2F2;
    }

    prvTgfPclDbLookupExtSet(CPSS_PCL_DIRECTION_INGRESS_E, CPSS_PCL_LOOKUP_0_1_E);
    rc = prvTgfPclRuleWithOptionsSet(
        ruleFormat, (baseRuleIndex + 1), 0 /*ruleOptionsBmp*/,
        &mask, &patt, &action);
    UTF_VERIFY_EQUAL0_STRING_RETURN_RC_MAC(rc, GT_OK, "prvTgfPclRuleSet");

    /* Rule2 Lookup_1 port_list disable */
    action.matchCounter.matchCounterIndex = (baseCncIndex + 2);

    if (maskPclId16Ptr)
    {
        *pattPclId16Ptr = 0x3F3;
    }
    if (maskPclId32Ptr)
    {
        *pattPclId32Ptr = 0x3F3;
    }

    prvTgfPclDbLookupExtSet(CPSS_PCL_DIRECTION_INGRESS_E, CPSS_PCL_LOOKUP_1_E);
    rc = prvTgfPclRuleWithOptionsSet(
        ruleFormat, (baseRuleIndex + 2), 0 /*ruleOptionsBmp*/,
        &mask, &patt, &action);
    UTF_VERIFY_EQUAL0_STRING_RETURN_RC_MAC(rc, GT_OK, "prvTgfPclRuleSet");

    /* Rules3-5 Lookups0-3 port_list enable */

    /* port list - check all 28 bits */
    maskPortListBmpPtr->ports[0] = 0x0FFFFFFF;
    pattPortListBmpPtr->ports[0] = (1 << portNum);

    /* PCL ID bits 9:8 of PCL ID not overlapped by port list   */
    if (maskPclId16Ptr)
    {
        *maskPclId16Ptr = 0x300;
    }
    if (maskPclId32Ptr)
    {
        *maskPclId32Ptr = 0x300;
    }

    /* Rule3 Lookup_0_0 port_list enable */
    action.matchCounter.matchCounterIndex = (baseCncIndex + 3);

    /* PCL ID bits 9:8 of PCL ID not overlapped by port list   */
    if (maskPclId16Ptr)
    {
        *pattPclId16Ptr = 0x100;
    }
    if (maskPclId32Ptr)
    {
        *pattPclId32Ptr = 0x100;
    }

    prvTgfPclDbLookupExtSet(CPSS_PCL_DIRECTION_INGRESS_E, CPSS_PCL_LOOKUP_0_0_E);
    rc = prvTgfPclRuleWithOptionsSet(
        ruleFormat, (baseRuleIndex + 3), 0 /*ruleOptionsBmp*/,
        &mask, &patt, &action);
    UTF_VERIFY_EQUAL0_STRING_RETURN_RC_MAC(rc, GT_OK, "prvTgfPclRuleSet");

   /* Rule4 Lookup_0_1 port_list enable */
    action.matchCounter.matchCounterIndex = (baseCncIndex + 4);

    /* PCL ID bits 9:8 of PCL ID not overlapped by port list   */
    if (maskPclId16Ptr)
    {
        *pattPclId16Ptr = 0x200;
    }
    if (maskPclId32Ptr)
    {
        *pattPclId32Ptr = 0x200;
    }

    prvTgfPclDbLookupExtSet(CPSS_PCL_DIRECTION_INGRESS_E, CPSS_PCL_LOOKUP_0_1_E);
    rc = prvTgfPclRuleWithOptionsSet(
        ruleFormat, (baseRuleIndex + 4), 0 /*ruleOptionsBmp*/,
        &mask, &patt, &action);
    UTF_VERIFY_EQUAL0_STRING_RETURN_RC_MAC(rc, GT_OK, "prvTgfPclRuleSet");

   /* Rule5 Lookup_1 port_list enable */
    action.matchCounter.matchCounterIndex = (baseCncIndex + 5);

    /* PCL ID bits 9:8 of PCL ID not overlapped by port list   */
    if (maskPclId16Ptr)
    {
        *pattPclId16Ptr = 0x300;
    }
    if (maskPclId32Ptr)
    {
        *pattPclId32Ptr = 0x300;
    }

    prvTgfPclDbLookupExtSet(CPSS_PCL_DIRECTION_INGRESS_E, CPSS_PCL_LOOKUP_1_E);
    rc = prvTgfPclRuleWithOptionsSet(
        ruleFormat, (baseRuleIndex + 5), 0 /*ruleOptionsBmp*/,
        &mask, &patt, &action);
    UTF_VERIFY_EQUAL0_STRING_RETURN_RC_MAC(rc, GT_OK, "prvTgfPclRuleSet");

    return GT_OK;
}

/**
* @internal prvTgfPclPortListIpcl3LookupsCfgRestore function
* @endinternal
* @param[in] portNum          - ingress port number
* @param[in] ruleFormat       - rule format
* @param[in] baseRuleIndex    - base index of 6 rules
*
* @brief   None
*/
static GT_STATUS prvTgfPclPortListIpcl3LookupsCfgRestore
(
    IN GT_PHYSICAL_PORT_NUM             portNum,
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat,
    IN GT_U32                           baseRuleIndex
)
{
    GT_STATUS                         rc = GT_OK;
    GT_STATUS                         rc1 = GT_OK;
    CPSS_INTERFACE_INFO_STC           interfaceInfo;
    PRV_TGF_PCL_LOOKUP_CFG_STC        lookupCfg;
    CPSS_PCL_RULE_SIZE_ENT            ruleSize;
    GT_U32                            i;
    CPSS_PCL_LOOKUP_NUMBER_ENT        lookup[3] =
    {
        CPSS_PCL_LOOKUP_0_0_E, CPSS_PCL_LOOKUP_0_1_E, CPSS_PCL_LOOKUP_1_E
    };

    /* set PCL configuration table for ports */
    cpssOsMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));
    interfaceInfo.type              = CPSS_INTERFACE_PORT_E;
    interfaceInfo.devPort.hwDevNum  = prvTgfDevNum;
    interfaceInfo.devPort.portNum   = portNum;

    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));
    lookupCfg.groupKeyTypes.nonIpKey =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    lookupCfg.groupKeyTypes.ipv4Key  =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;
    lookupCfg.groupKeyTypes.ipv6Key  =
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;

    rc = prvTgfPclCfgTblSet(
        &interfaceInfo, CPSS_PCL_DIRECTION_INGRESS_E, CPSS_PCL_LOOKUP_0_E, &lookupCfg);
    UTF_VERIFY_EQUAL0_STRING_SAVE_RC_MAC(GT_OK, rc, rc1, "prvTgfPclCfgTblSet");

    rc = prvTgfPclCfgTblSet(
        &interfaceInfo, CPSS_PCL_DIRECTION_INGRESS_E, CPSS_PCL_LOOKUP_1_E, &lookupCfg);
    UTF_VERIFY_EQUAL0_STRING_SAVE_RC_MAC(GT_OK, rc, rc1, "prvTgfPclCfgTblSet");

    rc = prvTgfPclPortIngressPolicyEnable(portNum, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_SAVE_RC_MAC(GT_OK, rc, rc1, "prvTgfPclPortIngressPolicyEnable");

    switch (ruleFormat)
    {
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E:
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E:
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E:
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E:
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_ROUTED_ACL_QOS_E:
            ruleSize = CPSS_PCL_RULE_SIZE_STD_E;
            break;
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E:
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E:
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E:
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E:
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV4_PORT_VLAN_QOS_E:
            ruleSize = CPSS_PCL_RULE_SIZE_EXT_E;
            break;
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E:
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_E:
            ruleSize = CPSS_PCL_RULE_SIZE_ULTRA_E;
            break;
        default:
            ruleSize = CPSS_PCL_RULE_SIZE_STD_E;
            UTF_VERIFY_EQUAL0_STRING_SAVE_RC_MAC(GT_OK, GT_FAIL, rc1, "not suported ruleFormat");
            break;
    }
    for (i = 0; (i < 6); i++)
    {
        prvTgfPclDbLookupExtSet(CPSS_PCL_DIRECTION_INGRESS_E, lookup[i % 3]);
        rc = prvTgfPclRuleValidStatusSet(
            ruleSize, (baseRuleIndex + i), GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_SAVE_RC_MAC(GT_OK, rc, rc1, "prvTgfPclRuleValidStatusSet");
    }

    return rc1;
}

/**
* @internal prvTgfPclPortListIpcl3LookupsPortListEnableSet function
* @endinternal
* @param[in] lookupsBitmap    - bitmap of lookups with enabled port list mode
*
* @brief None
*/
static GT_STATUS prvTgfPclPortListIpcl3LookupsPortListEnableSet
(
    IN GT_U32                           lookupsBitmap
)
{
    GT_STATUS    rc = GT_OK;
    GT_STATUS    rc1 = GT_OK;
    GT_U32       i;
    GT_BOOL      enable;
    static const CPSS_PCL_LOOKUP_NUMBER_ENT lookupArr[3] =
    {
        CPSS_PCL_LOOKUP_0_0_E, CPSS_PCL_LOOKUP_0_1_E, CPSS_PCL_LOOKUP_1_E
    };

    for (i = 0; (i < 3); i++)
    {
        enable = (lookupsBitmap & (1 << i)) ? GT_TRUE : GT_FALSE;
        rc = prvTgfPclLookupCfgPortListEnableSet(
            CPSS_PCL_DIRECTION_INGRESS_E, lookupArr[i],
            0 /*subLookupNum*/, enable);
        UTF_VERIFY_EQUAL0_STRING_SAVE_RC_MAC(rc, GT_OK, rc1, "prvTgfPclLookupCfgPortListEnableSet");
    }

    return rc1;
}

/**
* @internal prvTgfPclPortListIpcl3LookupsCncEnableSet function
* @endinternal
* @param[in]  enable   - enable CNC block0 for all 3 IPCL lookup clients
*
* @brief None
*/
static GT_STATUS prvTgfPclPortListIpcl3LookupsCncEnableSet
(
    IN GT_BOOL enable
)
{
    GT_STATUS    rc = GT_OK;
    GT_STATUS    rc1 = GT_OK;
    GT_U32       i;
    GT_U32       rangesBmpArr[2];

    static const PRV_TGF_CNC_CLIENT_ENT clientArr[3] =
    {
        PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E,
        PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_1_E,
        PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_E,
    };

    rangesBmpArr[0] = (enable == GT_FALSE) ? 0 : 1;
    rangesBmpArr[1] = 0;

    for (i = 0; (i < 3); i++)
    {
        rc = prvTgfCncBlockClientEnableSet(
            0 /*blockNum*/, clientArr[i], enable);
        UTF_VERIFY_EQUAL0_STRING_SAVE_RC_MAC(rc, GT_OK, rc1, "prvTgfCncBlockClientEnableSet");
        rc = prvTgfCncBlockClientRangesSet(
            0 /*blockNum*/, clientArr[i], rangesBmpArr);
        UTF_VERIFY_EQUAL0_STRING_SAVE_RC_MAC(rc, GT_OK, rc1, "prvTgfCncBlockClientRangesSet");
    }
    return rc1;
}

/**
* @internal prvTgfPclPortListIpcl3LookupsCncCountersClear function
* @endinternal
* @param[in] baseCncIndex     - base index of 6 CNC counters
*
* @brief None
*/
static GT_STATUS prvTgfPclPortListIpcl3LookupsCncCountersClear
(
    IN GT_U32     baseCncIndex
)
{
    GT_STATUS                rc = GT_OK;
    GT_STATUS                rc1 = GT_OK;
    GT_U32                   i;
    PRV_TGF_CNC_COUNTER_STC  counter;

    cpssOsMemSet(&counter, 0, sizeof(counter));
    for (i = 0; (i < 6); i++)
    {
        rc = prvTgfCncCounterSet(
            0 /*blockNum*/, (baseCncIndex + i),
            PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
        UTF_VERIFY_EQUAL0_STRING_SAVE_RC_MAC(rc, GT_OK, rc1, "prvTgfCncCounterSet");
    }
    return rc1;
}

/**
* @internal prvTgfPclPortListIpcl3LookupsCncCountersBitmapGet function
* @endinternal
* @param[in] baseCncIndex     - base index of 6 CNC counters
*
* @brief None
*/
static GT_STATUS prvTgfPclPortListIpcl3LookupsCncCountersBitmapGet
(
    IN  GT_U32     baseCncIndex,
    OUT GT_U32     *bitmapPtr
)
{
    GT_STATUS                rc = GT_OK;
    GT_STATUS                rc1 = GT_OK;
    GT_U32                   i;
    PRV_TGF_CNC_COUNTER_STC  counter;

    *bitmapPtr = 0;
    for (i = 0; (i < 6); i++)
    {
        rc = prvTgfCncCounterGet(
            0 /*blockNum*/, (baseCncIndex + i),
            PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
        UTF_VERIFY_EQUAL0_STRING_SAVE_RC_MAC(rc, GT_OK, rc1, "prvTgfCncCounterGet");
        if (counter.packetCount.l[0])
        {
            *bitmapPtr |= (1 << i);
        }
    }
    return rc1;
}

/**
* @internal prvTgfPclPortListIpcl3LookupsPacketSend function
* @endinternal
* @param[in] sendPortNum      - ingress port
* @param[in] ruleFormat       - rule format
*
* @brief   None
*/
static GT_STATUS prvTgfPclPortListIpcl3LookupsPacketSend
(
    IN GT_PHYSICAL_PORT_NUM             sendPortNum,
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat
)
{
    GT_STATUS               rc = GT_OK;
    GT_STATUS               rc1 = GT_OK;
    TGF_PACKET_STC          *pktTgfInfoPtr;

    /* PARTS of ETH packet */
    static TGF_PACKET_PART_STC ethPktTgfPartArray[] = {
        {TGF_PACKET_PART_L2_E,        &packet1TgfL2Part},
        {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacket1EtherTypePart},
        {TGF_PACKET_PART_PAYLOAD_E,   &packet1TgfPayloadPart}
    };
    /* ETH PACKET to send */
    static TGF_PACKET_STC ethPktTgfInfo = {
        TGF_PACKET_AUTO_CALC_LENGTH_CNS,                            /* totalLen */
        (sizeof(ethPktTgfPartArray) / sizeof(TGF_PACKET_PART_STC)), /* numOfParts */
        ethPktTgfPartArray                                          /* partsArray */
    };

    /* PARTS of IPV4 packet */
    static TGF_PACKET_PART_STC ipv4PktTgfPartArray[] = {
        {TGF_PACKET_PART_L2_E,        &packet1TgfL2Part},
        {TGF_PACKET_PART_ETHERTYPE_E, &packet1TgfIpv4TypePart},
        {TGF_PACKET_PART_IPV4_E,      &packet1TgfIpv4Part},
        {TGF_PACKET_PART_PAYLOAD_E,   &packet1TgfPayloadPart}
    };

    /* IPV4 PACKET to send */
    static TGF_PACKET_STC ipv4PktTgfInfo = {
        TGF_PACKET_AUTO_CALC_LENGTH_CNS,                             /* totalLen */
        (sizeof(ipv4PktTgfPartArray) / sizeof(TGF_PACKET_PART_STC)), /* numOfParts */
        ipv4PktTgfPartArray                                          /* partsArray */
    };

    /* PARTS of IPV6 packet */
    static TGF_PACKET_PART_STC ipv6PktTgfPartArray[] = {
        {TGF_PACKET_PART_L2_E,        &packet1TgfL2Part},
        {TGF_PACKET_PART_ETHERTYPE_E, &packet1TgfIpv6TypePart},
        {TGF_PACKET_PART_IPV6_E,      &packet1TgfIpv6Part},
        {TGF_PACKET_PART_PAYLOAD_E,   &packet1TgfPayloadPart}
    };

    /* IPV6 PACKET to send */
    static TGF_PACKET_STC ipv6PktTgfInfo = {
        TGF_PACKET_AUTO_CALC_LENGTH_CNS,                             /* totalLen */
        (sizeof(ipv6PktTgfPartArray) / sizeof(TGF_PACKET_PART_STC)), /* numOfParts */
        ipv6PktTgfPartArray                                          /* partsArray */
    };


    switch (ruleFormat)
    {
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E:
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E:
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E:
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E:
            pktTgfInfoPtr = &ethPktTgfInfo; /* ETH */
            break;
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E:
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E:
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_ROUTED_ACL_QOS_E:
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV4_PORT_VLAN_QOS_E:
            pktTgfInfoPtr = &ipv4PktTgfInfo; /* IPV4 */
            break;
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E:
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E:
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E:
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_E:
            pktTgfInfoPtr = &ipv6PktTgfInfo; /* IPV6 */
            break;
        default:
            pktTgfInfoPtr = &ethPktTgfInfo; /* ETH */
            UTF_VERIFY_EQUAL0_STRING_SAVE_RC_MAC(GT_OK, GT_FAIL, rc1, "not suported ruleFormat");
            break;
    }


    PRV_UTF_LOG1_MAC("sending packet: sendPort [%d] \n", sendPortNum);

    /* setup transmit params */
    rc = prvTgfSetTxSetupEth(
        prvTgfDevNum, pktTgfInfoPtr,
        1 /*burstCount*/, 0 /*numVfd*/, NULL);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfSetTxSetupEth");

    /* start transmitting */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, sendPortNum);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfStartTransmitingEth");

    cpssOsTimerWkAfter(100);

    return rc1;
}

/**
* @internal prvTgfPclPortListIpcl3LookupsTestCase function
* @endinternal
* @param[in] sendPortNum      - ingress port
* @param[in] ruleFormat       - rule format
* @param[in] baseRuleIndex    - base index of 6 rules
* @param[in] baseCncIndex     - base index of 6 CNC counters
*
* @brief   None
*/
static GT_STATUS prvTgfPclPortListIpcl3LookupsTestCase
(
    IN GT_PHYSICAL_PORT_NUM             sendPortNum,
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat,
    IN GT_U32                           baseRuleIndex,
    IN GT_U32                           baseCncIndex
)
{
    GT_STATUS               rc = GT_OK;
    GT_STATUS               rc1 = GT_OK;
    GT_U32                  lookupsBitmap;
    GT_U32                  countersBitmap;
    static const GT_U32     expectedCountersBitmap[8] =
    {
        /*0*/ 0x07, /*1*/ 0x0E, /*2*/ 0x15, /*3*/ 0x1C,
        /*4*/ 0x23, /*5*/ 0x2A, /*6*/ 0x31, /*7*/ 0x38
    };

    PRV_UTF_LOG0_MAC("\n********prvTgfPclPortListIpcl3LookupsTestCase\n");
    PRV_UTF_LOG4_MAC(
        "sendPortNum %d ruleFormat %d baseRuleIndex %d baseCncIndex %d\n",
        sendPortNum, ruleFormat, baseRuleIndex, baseCncIndex);
    rc = prvTgfPclPortListIpcl3LookupsCfgSet(
        sendPortNum, ruleFormat, baseRuleIndex, baseCncIndex);
    if (rc != GT_OK)
    {
        rc1 = rc;
    }

    rc = prvTgfPclPortListIpcl3LookupsCncEnableSet(GT_TRUE);
    if (rc != GT_OK)
    {
        rc1 = rc;
    }

    for (lookupsBitmap = 0; (lookupsBitmap < 8); lookupsBitmap++)
    {
        rc = prvTgfPclPortListIpcl3LookupsPortListEnableSet(lookupsBitmap);
        if (rc != GT_OK)
        {
            rc1 = rc;
            break;
        }
        /* clear CNC counters */
        rc = prvTgfPclPortListIpcl3LookupsCncCountersClear(baseCncIndex);
        if (rc != GT_OK)
        {
            rc1 = rc;
            break;
        }
        /* send packet */
        rc = prvTgfPclPortListIpcl3LookupsPacketSend(sendPortNum, ruleFormat);
        if (rc != GT_OK)
        {
            rc1 = rc;
            break;
        }
        /* check CNC counters */
        rc = prvTgfPclPortListIpcl3LookupsCncCountersBitmapGet(
            baseCncIndex, &countersBitmap);
        if (rc != GT_OK)
        {
            rc1 = rc;
            break;
        }
        UTF_VERIFY_EQUAL0_STRING_SAVE_RC_MAC(
            expectedCountersBitmap[lookupsBitmap], countersBitmap/*rc*/, countersBitmap/*rc1*/,
            "expectedCountersBitmap[lookupsBitmap] != countersBitmap");
        if (expectedCountersBitmap[lookupsBitmap] != countersBitmap)
        {
            rc1 = GT_FAIL;
            break;
        }
    }

    rc = prvTgfPclPortListIpcl3LookupsPortListEnableSet(0);
    if (rc != GT_OK)
    {
        rc1 = rc;
    }
    rc = prvTgfPclPortListIpcl3LookupsCncCountersClear(baseCncIndex);
    if (rc != GT_OK)
    {
        rc1 = rc;
    }
    rc = prvTgfPclPortListIpcl3LookupsCncEnableSet(GT_FALSE);
    if (rc != GT_OK)
    {
        rc1 = rc;
    }
    rc = prvTgfPclPortListIpcl3LookupsCfgRestore(
        sendPortNum, ruleFormat, baseRuleIndex);
    if (rc != GT_OK)
    {
        rc1 = rc;
    }

    return rc1;
}

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/*******************************************************************************
* prvTgfPclPortListCrossIpclTest
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*       None
*
* COMMENTS:
*
*******************************************************************************/
GT_VOID prvTgfPclPortListCrossIpclTest
(
    GT_VOID
)
{
    GT_STATUS                            rc, rc1 = GT_OK;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* set common configuration */
    rc = prvTgfPclPortListCrossCfgVlanSet();
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclPortListCrossCfgVlanSet");

    /* set IPCL configuration */
    rc = prvTgfPclPortListCrossCfgIpclSet();
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclPortListCrossCfgIpclSet");

    /* -------------------------------------------------------------------------
     * 2. Generate traffic
     */

    /* AUTODOC: GENERATE TRAFFIC: */
    prvTgfPclPortListCrossIpclCheck();

    /* -------------------------------------------------------------------------
     * 3. Restore configuration
     */

    /* AUTODOC: RESTORE CONFIGURATION: */
    rc = prvTgfPclPortListCrossCfgIpclRestore();
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfPclPortListCrossCfgIpclRestore");

    rc = prvTgfPclPortListCrossCfgVlanRestore();
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfPclPortListCrossCfgVlanRestore");

    TGF_PARAM_NOT_USED(rc1); /* prevent warning: not used variable */
}

/*******************************************************************************
* prvTgfPclPortListCrossEpclTest
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*       None
*
* COMMENTS:
*
*******************************************************************************/
GT_VOID prvTgfPclPortListCrossEpclTest
(
    GT_VOID
)
{
    GT_STATUS                            rc, rc1 = GT_OK;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* set common configuration */
    rc = prvTgfPclPortListCrossCfgVlanSet();
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclPortListCrossCfgVlanSet");

    /* set IPCL configuration */
    rc = prvTgfPclPortListCrossCfgEpclSet();
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclPortListCrossCfgEpclSet");

    /* -------------------------------------------------------------------------
     * 2. Generate traffic
     */

    /* AUTODOC: GENERATE TRAFFIC: */
    prvTgfPclPortListCrossEpclCheck();

    /* -------------------------------------------------------------------------
     * 3. Restore configuration
     */

    /* AUTODOC: RESTORE CONFIGURATION: */
    rc = prvTgfPclPortListCrossCfgEpclRestore();
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfPclPortListCrossCfgEpclRestore");

    rc = prvTgfPclPortListCrossCfgVlanRestore();
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfPclPortListCrossCfgVlanRestore");

    TGF_PARAM_NOT_USED(rc1); /* prevent warning: not used variable */
}

/*******************************************************************************
* prvTgfPclPortListIpclFullTest
*
* INPUTS:
* @param[in] portBitMap        - bitmap of tested ports - ports0...27 supported
* @param[in] lookupBitMap      - bitmap of tested lookups (IPCL0..2)
* @param[in] ruleFormatBitMap  - bitmap of tested rule formats (below):
**      0 - PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
**      1 - PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E,
**      2 - PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E,
**      3 - PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E,
**      4 - PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E,
**      5 - PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E,
**      6 - PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E,
**      7 - PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_80B_E,
**      8 - PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_ROUTED_ACL_QOS_E,
**      9 - PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV4_PORT_VLAN_QOS_E,
**     10 - PRV_TGF_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E ,
**     11 - PRV_TGF_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_E
*
* OUTPUTS:
*       None
*
* RETURNS:
*       None
*
* COMMENTS:
*
*******************************************************************************/
GT_VOID prvTgfPclPortListIpclFullTest
(
    IN GT_U32 portBitMap,
    IN GT_U32 lookupBitMap,
    IN GT_U32 ruleFormatBitMap
)
{
    GT_STATUS                        rc = GT_OK;
    CPSS_PCL_LOOKUP_NUMBER_ENT       lookupNum;
    GT_PHYSICAL_PORT_NUM             sendPortNum;
    GT_PHYSICAL_PORT_NUM             receivePortNum;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat;
    GT_U32                           ruleIndex = 8;
    GT_U32                           packetVid = 2;
    GT_U32                           newVid    = 3;
    GT_U32                           lookupIdx;
    GT_U32                           ruleFormatIdx;
    CPSS_PORT_SPEED_ENT              speed;
    CPSS_INTERFACE_INFO_STC          portInterface;
    GT_BOOL                          isLinkUp;
    GT_U32                           i;
    CPSS_CSCD_PORT_TYPE_ENT          portType;

    static const CPSS_PCL_LOOKUP_NUMBER_ENT lookupArr[3] =
    {
        CPSS_PCL_LOOKUP_0_0_E, CPSS_PCL_LOOKUP_0_1_E, CPSS_PCL_LOOKUP_1_E
    };
    static const PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormatArr[] =
    {
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_ROUTED_ACL_QOS_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV4_PORT_VLAN_QOS_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E ,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_E
    };
    static const GT_U32 ruleFormatArrSize = (sizeof(ruleFormatArr) / sizeof(ruleFormatArr[0]));

    for (sendPortNum = 0; (sendPortNum < 28); sendPortNum++)
    {
        if ((portBitMap & (1 << sendPortNum)) == 0) continue;

        portInterface.type              = CPSS_INTERFACE_PORT_E;
        portInterface.devPort.hwDevNum  = prvTgfDevNum;
        portInterface.devPort.portNum   = sendPortNum;

        /* bypass not configured ports */
        rc = prvTgfPortSpeedGet(prvTgfDevNum, sendPortNum, &speed);
        if (rc != GT_OK) continue;

        rc = prvTgfCscdPortTypeGet(prvTgfDevNum, sendPortNum, CPSS_PORT_DIRECTION_BOTH_E, &portType);
        if (rc != GT_OK) continue;

        /* skip cascade ports */
        if (portType != CPSS_CSCD_PORT_NETWORK_E) continue;

        rc = prvTgfPortLinkStatusGet(prvTgfDevNum, sendPortNum, &isLinkUp);
        if (rc != GT_OK) continue;
        if (isLinkUp == GT_FALSE)
        {
            rc = tgfTrafficGeneratorPortForceLinkUpEnableSet(&portInterface, GT_TRUE);
            UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, rc, "tgfTrafficGeneratorPortForceLinkUpEnableSet");
            for (i = 0; (i < 1000); i++)
            {
                /* wait for link up */
                cpssOsTimerWkAfter(10);
                prvTgfPortLinkStatusGet(prvTgfDevNum, sendPortNum, &isLinkUp);
                if (isLinkUp != GT_FALSE) break;
            }
            if (isLinkUp == GT_FALSE)
            {
                PRV_UTF_LOG1_MAC("Fail to force link up port %d \n", sendPortNum);
            }
        }

        receivePortNum =
            (sendPortNum != prvTgfPortsArray[0])
                ? prvTgfPortsArray[0] : prvTgfPortsArray[1];
        for (lookupIdx = 0; (lookupIdx < 3); lookupIdx++)
        {
            if ((lookupBitMap & (1 << lookupIdx)) == 0) continue;
            lookupNum = lookupArr[lookupIdx];
            for (ruleFormatIdx = 0; (ruleFormatIdx < ruleFormatArrSize); ruleFormatIdx++)
            {
                if ((ruleFormatBitMap & (1 << ruleFormatIdx)) == 0) continue;
                ruleFormat = ruleFormatArr[ruleFormatIdx];
                rc = prvTgfPclPortListIpclFullTestCase(
                    lookupNum, sendPortNum, receivePortNum,
                    ruleFormat, ruleIndex, packetVid, newVid);
                if (rc != GT_OK)
                {
                    if (isLinkUp == GT_FALSE)
                    {
                        rc = tgfTrafficGeneratorPortForceLinkUpEnableSet(&portInterface, GT_FALSE);
                        UTF_VERIFY_EQUAL0_STRING_MAC(
                            GT_OK, rc, "tgfTrafficGeneratorPortForceLinkUpEnableSet");
                    }
                    return;
                }
            }
        }
        if (isLinkUp == GT_FALSE)
        {
            rc = tgfTrafficGeneratorPortForceLinkUpEnableSet(&portInterface, GT_FALSE);
            UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, rc, "tgfTrafficGeneratorPortForceLinkUpEnableSet");
        }
    }
}

/*******************************************************************************
* prvTgfPclPortListIpcl3LookupsTest
*
* INPUTS:
* @param[in] portBitMap        - bitmap of tested ports - ports0...27 supported
* @param[in] ruleFormatBitMap  - bitmap of tested rule formats (below):
**      0 - PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
**      1 - PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E,
**      2 - PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E,
**      3 - PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E,
**      4 - PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E,
**      5 - PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E,
**      6 - PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E,
**      7 - PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_80B_E,
**      8 - PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_ROUTED_ACL_QOS_E,
**      9 - PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV4_PORT_VLAN_QOS_E,
**     10 - PRV_TGF_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E ,
**     11 - PRV_TGF_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_E
*
* OUTPUTS:
*       None
*
* RETURNS:
*       None
*
* COMMENTS:
*
*******************************************************************************/
GT_VOID prvTgfPclPortListIpcl3LookupsTest
(
    IN GT_U32 portBitMap,
    IN GT_U32 ruleFormatBitMap
)
{
    GT_STATUS                        rc = GT_OK;
    GT_PHYSICAL_PORT_NUM             sendPortNum;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat;
    IN GT_U32                        baseRuleIndex = 8;
    IN GT_U32                        baseCncIndex  = 3;
    GT_U32                           ruleFormatIdx;
    CPSS_PORT_SPEED_ENT              speed;
    CPSS_INTERFACE_INFO_STC          portInterface;
    GT_BOOL                          isLinkUp;
    GT_U32                           i;
    CPSS_CSCD_PORT_TYPE_ENT          portType;

    static const PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormatArr[] =
    {
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_UDB_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_ROUTED_ACL_QOS_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV4_PORT_VLAN_QOS_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E ,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_E
    };
    static const GT_U32 ruleFormatArrSize = (sizeof(ruleFormatArr) / sizeof(ruleFormatArr[0]));

    for (sendPortNum = 0; (sendPortNum < 28); sendPortNum++)
    {
        if ((portBitMap & (1 << sendPortNum)) == 0) continue;

        portInterface.type              = CPSS_INTERFACE_PORT_E;
        portInterface.devPort.hwDevNum  = prvTgfDevNum;
        portInterface.devPort.portNum   = sendPortNum;

        /* bypass not configured ports */
        rc = prvTgfPortSpeedGet(prvTgfDevNum, sendPortNum, &speed);
        if (rc != GT_OK) continue;

        rc = prvTgfCscdPortTypeGet(prvTgfDevNum, sendPortNum, CPSS_PORT_DIRECTION_BOTH_E, &portType);
        if (rc != GT_OK) continue;

        /* skip cascade ports */
        if (portType != CPSS_CSCD_PORT_NETWORK_E) continue;

        rc = prvTgfPortLinkStatusGet(prvTgfDevNum, sendPortNum, &isLinkUp);
        if (rc != GT_OK) continue;

        if (isLinkUp == GT_FALSE)
        {
            rc = tgfTrafficGeneratorPortForceLinkUpEnableSet(&portInterface, GT_TRUE);
            UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, rc, "tgfTrafficGeneratorPortForceLinkUpEnableSet");
            for (i = 0; (i < 1000); i++)
            {
                /* wait for link up */
                cpssOsTimerWkAfter(10);
                prvTgfPortLinkStatusGet(prvTgfDevNum, sendPortNum, &isLinkUp);
                if (isLinkUp != GT_FALSE) break;
            }
            if (isLinkUp == GT_FALSE)
            {
                PRV_UTF_LOG1_MAC("Fail to force link up port %d \n", sendPortNum);
            }
        }

        for (ruleFormatIdx = 0; (ruleFormatIdx < ruleFormatArrSize); ruleFormatIdx++)
        {
            if ((ruleFormatBitMap & (1 << ruleFormatIdx)) == 0) continue;
            ruleFormat = ruleFormatArr[ruleFormatIdx];

            rc = prvTgfPclPortListIpcl3LookupsTestCase(
                sendPortNum, ruleFormat, baseRuleIndex, baseCncIndex);
            if (rc != GT_OK)
            {
                if (isLinkUp == GT_FALSE)
                {
                    rc = tgfTrafficGeneratorPortForceLinkUpEnableSet(&portInterface, GT_FALSE);
                    UTF_VERIFY_EQUAL0_STRING_MAC(
                        GT_OK, rc, "tgfTrafficGeneratorPortForceLinkUpEnableSet");
                }
                return;
            }
        }
        if (isLinkUp == GT_FALSE)
        {
            rc = tgfTrafficGeneratorPortForceLinkUpEnableSet(&portInterface, GT_FALSE);
            UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, rc, "tgfTrafficGeneratorPortForceLinkUpEnableSet");
        }
    }
}




