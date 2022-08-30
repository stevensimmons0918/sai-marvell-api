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
* @file prvTgfBrgUntaggedMruCheck.c
*
* @brief Untagged MRU check - basic
*
* @version   4
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <bridge/prvTgfBrgUntaggedMruCheck.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <utf/private/prvUtfExtras.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/*  VLAN Id 3 */
#define PRV_TGF_VLANID_2_CNS                    2

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS               0

/* capturing port number  */
#define PRV_TGF_CAPTURE_PORT_IDX_CNS            2

/* MRU value index */
#define PRV_TGF_MRU_INDEX_CNS                   1

/* Default MRU value index */
#define PRV_TGF_DEF_MRU_INDEX_CNS               0

/* MRU value */
#define PRV_TGF_MRU_VALUE_CNS                   512

/* MRU default value */
#define PRV_TGF_DEF_MRU_VALUE_CNS               1518

/* default number of packets to send */
static GT_U32  prvTgfBurstCount   = 1;


/* 4 bytes CRC relate to : JIRA : L2I-778 'missing 4 bytes' for the MRU calculations in L2i.
   CPSS implements the WA and aligns HW values accordingly. */
#define VLAN_MRU_HOLD_CRC_BYTES(devNum) (4)

#define SIP6_PACKET_SIZE_FROM_LLC(llc_size) (12/*MAC DA+SA*/ + 4/*vlanTag*/ + 2/*the field length*/ + llc_size)

/******************************* Test packet **********************************/
/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x02},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x11}                /* srcMac */
};

/* First VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_2_CNS                          /* pri, cfi, VlanId */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadData1Arr[] =
{
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0F, 0x10, 0x11,
    0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
    0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21,
    0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
    0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31,
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0F, 0x10, 0x11,
    0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
    0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21,
    0x22, 0x23, 0x24, 0x25
};

/* DATA of packet 2 */
static GT_U8 prvTgfPayloadData2Arr[] =
{
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0F, 0x10, 0x11,
    0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
    0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21,
    0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
    0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31,
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0F, 0x10, 0x11,
    0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
    0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21,
    0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
    0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31,
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0F, 0x10, 0x11,
    0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
    0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21,
    0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
    0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31,
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0F, 0x10, 0x11,
    0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
    0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21,
    0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
    0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31,
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0F, 0x10, 0x11,
    0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
    0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21,
    0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
    0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31,
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0F, 0x10, 0x11,
    0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
    0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21,
    0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
    0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31,
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0F, 0x10, 0x11,
    0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
    0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21,
    0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
    0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31,
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0F, 0x10, 0x11,
    0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
    0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21,
    0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
    0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31,
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0F, 0x10, 0x11,
    0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
    0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21,
    0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
    0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31,
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0F, 0x10, 0x11,
    0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
    0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21,
    0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
    0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31,
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0F, 0x10, 0x11,
    0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
    0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21,
    0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
    0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31,
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06
};

/* SIP6 : DATA of packet 3 : <byte count> = 104 (include CRC) , MRU = 99/100 --> expected pass/drop */
static GT_U8 prvTgfPayloadData3Arr[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    0x30, 0x31, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
    0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21
};

/* PAYLOAD part first packet */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart1 =
{
    sizeof(prvTgfPayloadData1Arr),                     /* dataLength */
    prvTgfPayloadData1Arr                              /* dataPtr */
};

/* PAYLOAD part second packet */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart2 =
{
    sizeof(prvTgfPayloadData2Arr),                     /* dataLength */
    prvTgfPayloadData2Arr                              /* dataPtr */
};

/* PAYLOAD part 3 packet */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart3 =
{
    sizeof(prvTgfPayloadData3Arr),                     /* dataLength */
    prvTgfPayloadData3Arr                              /* dataPtr */
};

/* PARTS of first tagged packet */
static TGF_PACKET_PART_STC prvTgfTaggedPacketPart1Array[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart1}
};

/* PARTS of second tagged packet */
static TGF_PACKET_PART_STC prvTgfTaggedPacketPart2Array[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart2}
};

/* PARTS of 3'rd tagged packet */
static TGF_PACKET_PART_STC prvTgfTaggedPacketPart3Array[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart3}
};

/* LENGTH of first TAGGED packet */
#define PRV_TGF_TAGGED_PACKET1_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadData1Arr)

/* LENGTH of second TAGGED packet */
#define PRV_TGF_TAGGED_PACKET2_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadData2Arr)

/* LENGTH of 3'rd TAGGED packet */
#define PRV_TGF_TAGGED_PACKET3_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadData3Arr)

/* First TAGGED PACKET to send */
static TGF_PACKET_STC prvTgfTaggedPacketInfo1 =
{
    PRV_TGF_TAGGED_PACKET1_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfTaggedPacketPart1Array) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfTaggedPacketPart1Array                                        /* partsArray */
};

/* Second TAGGED PACKET to send */
static TGF_PACKET_STC prvTgfTaggedPacketInfo2 =
{
    PRV_TGF_TAGGED_PACKET2_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfTaggedPacketPart2Array) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfTaggedPacketPart2Array                                        /* partsArray */
};

/* 3'rd TAGGED PACKET to send */
static TGF_PACKET_STC prvTgfTaggedPacketInfo3 =
{
    PRV_TGF_TAGGED_PACKET3_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfTaggedPacketPart3Array) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfTaggedPacketPart3Array                                        /* partsArray */
};


/* send packet info array */
static TGF_PACKET_STC* prvTgfPacketInfoArr[3] =
{
    &prvTgfTaggedPacketInfo1, &prvTgfTaggedPacketInfo2 , &prvTgfTaggedPacketInfo3
};

/******************************************************************************/


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfBrgUntaggedMruCheckConfigSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfBrgUntaggedMruCheckConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;

    GT_U8       tagArray[]  = {1, 1, 1, 0};


    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: create VLAN 2 with tagged ports 0,1,2 and untagged port 3 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_2_CNS,
                                           prvTgfPortsArray,
                                           NULL, tagArray, prvTgfPortsNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* AUTODOC: set PVID 2 for port 0 */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 PRV_TGF_VLANID_2_CNS);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 PRV_TGF_VLANID_2_CNS);

    /* AUTODOC: enable FDB Learning, Scanning and Refresh operations */
    rc = prvTgfBrgFdbLsrEnableSet(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbLsrEnableSet");

    /* AUTODOC: set Independent VLAN Learning mode */
    rc = prvTgfBrgFdbMacVlanLookupModeSet(CPSS_IVL_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacVlanLookupModeSet");

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:00:02, VLAN 2, port 2 */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(prvTgfPacketL2Part.daMac,
                                          PRV_TGF_VLANID_2_CNS,
                                          prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS],
                                          GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnPortSet: %d", prvTgfDevNum);

    /* AUTODOC: for MRU profile index 1 set MRU value 512 */
    rc = prvTgfBrgVlanMruProfileValueSet(PRV_TGF_MRU_INDEX_CNS, PRV_TGF_MRU_VALUE_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMruProfileValueSet");

    /* AUTODOC: for VLAN 2 set MRU profile index 1 */
    rc = prvTgfBrgVlanMruProfileIdxSet(PRV_TGF_VLANID_2_CNS, PRV_TGF_MRU_INDEX_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMruProfileIdxSet");
}

/**
* @internal prvTgfBrgUntaggedMruCheckTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfBrgUntaggedMruCheckTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                   rc = GT_OK;
    GT_U32                      portIter = 0;
    GT_U32                      sendIter = 0;
    GT_U32                      numTriggers = 0;
    GT_U32                      packetLen = 0;
    TGF_VFD_INFO_STC            vfdArray[1];
    CPSS_INTERFACE_INFO_STC     portInterface;
    GT_U32                      numOfPackets;
    GT_U32                      packetIndex;
    GT_BOOL                     expectDrop;
    GT_U32                      currMru;

    if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum) &&
       !prvUtfIsGmCompilation()) /* no such issue of 'cut through' in GM */
    {
        numOfPackets = 7; /* extra checking */

        currMru = SIP6_PACKET_SIZE_FROM_LLC(0x0102) - 1 + VLAN_MRU_HOLD_CRC_BYTES(prvTgfDevNum);
        rc = prvTgfBrgVlanMruProfileValueSet(PRV_TGF_MRU_INDEX_CNS, currMru);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMruProfileValueSet");
    }
    else
    {
        currMru = PRV_TGF_MRU_VALUE_CNS;
        numOfPackets = 2;
    }

    /* AUTODOC: GENERATE TRAFFIC: */
    for (sendIter = 0; sendIter < numOfPackets; sendIter++)
    {
        if(sendIter == 0)
        {
            packetIndex = 0;
            expectDrop = GT_FALSE;
        }
        else
        if(sendIter == 1)
        {
            packetIndex = 1;
            expectDrop = GT_TRUE;

            if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
            {
                PRV_UTF_LOG0_MAC("\n========== LLC 'cut through' LLC field <packet len> 0x0102 instead of full packet length ==========\n\n");
            }
        }
        else
        if(sendIter == 2) /*PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum)*/
        {
            packetIndex = 1;/* same packet as before */
            expectDrop = GT_FALSE;

            PRV_UTF_LOG0_MAC("\n========== LLC 'cut through' LLC field <packet len> 0x0102 instead of full packet length ==========\n\n");

            currMru += 1;
            rc = prvTgfBrgVlanMruProfileValueSet(PRV_TGF_MRU_INDEX_CNS, currMru);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMruProfileValueSet");
        }
        else
        if(sendIter == 3) /*PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum)*/
        {
            packetIndex = 2;/* same packet as before */
            expectDrop = GT_TRUE;

            PRV_UTF_LOG0_MAC("\n========== LLC but not 'cut through' not use LLC field <packet len> 0x0001 ==========\n\n");
            currMru = SIP6_PACKET_SIZE_FROM_LLC(0x0001) - 1 + VLAN_MRU_HOLD_CRC_BYTES(prvTgfDevNum);
            rc = prvTgfBrgVlanMruProfileValueSet(PRV_TGF_MRU_INDEX_CNS, currMru);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMruProfileValueSet");

        }
        else
        if(sendIter == 4) /*PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum)*/
        {
            packetIndex = 2;
            expectDrop = GT_TRUE;

            PRV_UTF_LOG0_MAC("\n========== LLC but not 'cut through' not use LLC field <packet len> 0x0001 ==========\n\n");
            currMru += 1;
            rc = prvTgfBrgVlanMruProfileValueSet(PRV_TGF_MRU_INDEX_CNS, currMru );
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMruProfileValueSet");
        }
        else
        if(sendIter == 5) /*PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum)*/
        {
            packetIndex = 2;
            expectDrop = GT_TRUE;
            PRV_UTF_LOG0_MAC("\n========== LLC but not 'cut through' use full packet length ==========\n\n");

            currMru = prvTgfPacketInfoArr[packetIndex]->totalLen/*not include CRC!*/ + VLAN_MRU_HOLD_CRC_BYTES(prvTgfDevNum) - 1;
            rc = prvTgfBrgVlanMruProfileValueSet(PRV_TGF_MRU_INDEX_CNS, currMru );
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMruProfileValueSet");

        }
        else
        /*if(sendIter == 6) && PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum)*/
        {
            packetIndex = 2;
            expectDrop = GT_FALSE;

            PRV_UTF_LOG0_MAC("\n========== LLC but not 'cut through' use full packet length ==========\n\n");
            currMru += 1;
            rc = prvTgfBrgVlanMruProfileValueSet(PRV_TGF_MRU_INDEX_CNS, currMru );
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMruProfileValueSet");

        }
        /* clear capturing RxPcktTable */
        rc = tgfTrafficTableRxPcktTblClear();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

        /* enable capture */
        portInterface.type            = CPSS_INTERFACE_PORT_E;
        portInterface.devPort.hwDevNum  = prvTgfDevNum;
        portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS];

        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);

        /* reset counters */
        rc = prvTgfEthCountersReset(prvTgfDevNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset: %d", prvTgfDevNum);

        /* setup packet */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, prvTgfPacketInfoArr[packetIndex],
                                 prvTgfBurstCount, 0, NULL);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup");

        PRV_UTF_LOG2_MAC("\n========== Sending packet iter=%d  on port=%d==========\n\n",
                         sendIter + 1, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* AUTODOC: send 2 packets on port 0 with: */
        /* AUTODOC:   DA=00:00:00:00:00:02, SA=00:00:00:00:00:11, VID=2 */
        /* AUTODOC:   payload sizes [84,534] bytes */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* disable capture */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                     prvTgfDevNum, portInterface.devPort.portNum);

        PRV_UTF_LOG1_MAC("Expecting the packet [%s] the MRU check ... lets check counters: \n\n",
            (GT_CHAR*)((expectDrop == GT_TRUE) ? "DROPPED by" : "PASS"));

        PRV_UTF_LOG2_MAC("currMru[%d] , packet size [%d](without CRC!) \n\n",
            currMru ,
            prvTgfPacketInfoArr[packetIndex]->totalLen);

        /* AUTODOC: verify traffic: */
        /* AUTODOC:   1 packet (size<512) - received on port 2 */
        /* AUTODOC:   2 packet (size>512) - dropped */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            IN  GT_U32                         expected_packetsCountRx;
            IN  GT_U32                         expected_packetsCountTx;

            if(portIter == PRV_TGF_SEND_PORT_IDX_CNS)
            {
                expected_packetsCountRx = 1;
                expected_packetsCountTx = 1;
            }
            else
            if(portIter != PRV_TGF_CAPTURE_PORT_IDX_CNS)
            {
                expected_packetsCountRx = 0;
                expected_packetsCountTx = 0;
            }
            else
            if(expectDrop == GT_TRUE)/*(portIter == PRV_TGF_CAPTURE_PORT_IDX_CNS)*/
            {
                expected_packetsCountRx = 0;
                expected_packetsCountTx = 0;
            }
            else/* (expectDrop == GT_FALSE) and (portIter == PRV_TGF_CAPTURE_PORT_IDX_CNS)*/
            {
                expected_packetsCountRx = 1;
                expected_packetsCountTx = 1;
            }

            packetLen = prvTgfPacketInfoArr[packetIndex]->totalLen;

            /* check ETH counters */
            rc = prvTgfEthCountersCheck(prvTgfDevNum,
                                        prvTgfPortsArray[portIter],
                                        expected_packetsCountRx*prvTgfBurstCount,
                                        expected_packetsCountTx*prvTgfBurstCount,
                                        packetLen, prvTgfBurstCount);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");
        }

        /* clear VFD array */
        cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

        /* set vfd for destination MAC */
        vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
        vfdArray[0].modeExtraInfo = 0;
        vfdArray[0].offset = 0;
        vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
        cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

        /* get trigger counters */
        rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
        PRV_UTF_LOG2_MAC("numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

        /* check pattern of TriggerCounters */
        UTF_VERIFY_EQUAL2_STRING_MAC(numTriggers , (expectDrop == GT_TRUE)? 0 : 1,
                                     "get another trigger that expected: expected - %d, received - %d\n",
                                     numTriggers , (expectDrop == GT_TRUE)? 0 : 1);
    }
}

/**
* @internal prvTgfBrgUntaggedMruCommandCheckTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic - test MRU packet command
*/
GT_VOID prvTgfBrgUntaggedMruCommandCheckTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                   rc = GT_OK;
    GT_U32                      portIter = 0;
    GT_U32                      sendIter = 0;
    GT_U32                      numTriggers = 0;
    GT_U32                      packetLen = 0;
    TGF_VFD_INFO_STC            vfdArray[1];
    CPSS_INTERFACE_INFO_STC     portInterface;
    GT_U32                      numOfPackets;
    GT_U32                      packetIndex;
    GT_BOOL                     expectDrop;
    GT_U32                      currMru;

    if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum) &&
       !prvUtfIsGmCompilation()) /* no such issue of 'cut through' in GM */
    {
        numOfPackets = 7; /* extra checking */

        currMru = SIP6_PACKET_SIZE_FROM_LLC(0x0102) - 1 + VLAN_MRU_HOLD_CRC_BYTES(prvTgfDevNum);
        rc = prvTgfBrgVlanMruProfileValueSet(PRV_TGF_MRU_INDEX_CNS, currMru);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMruProfileValueSet");
    }
    else
    {
        currMru = PRV_TGF_MRU_VALUE_CNS;
        numOfPackets = 2;
    }

    /* AUTODOC: GENERATE TRAFFIC: */
    for (sendIter = 0; sendIter < numOfPackets; sendIter++)
    {
        if(sendIter == 0)
        {
            packetIndex = 0;
            expectDrop = GT_FALSE;
        }
        else
        if(sendIter == 1)
        {
            /* AUTODOC: for VLAN 2 set MRU packet command */
            rc = prvTgfBrgVlanMruCommandSet(prvTgfDevNum, CPSS_PACKET_CMD_FORWARD_E);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMruProfileIdxSet");

            packetIndex = 1;
            expectDrop = GT_FALSE;

            if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
            {
                PRV_UTF_LOG0_MAC("\n========== LLC 'cut through' LLC field <packet len> 0x0102 instead of full packet length ==========\n\n");
            }
        }
        else
        if(sendIter == 2) /*PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum)*/
        {
            packetIndex = 1;/* same packet as before */
            expectDrop = GT_FALSE;

            PRV_UTF_LOG0_MAC("\n========== LLC 'cut through' LLC field <packet len> 0x0102 instead of full packet length ==========\n\n");

            currMru += 1;
            rc = prvTgfBrgVlanMruProfileValueSet(PRV_TGF_MRU_INDEX_CNS, currMru);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMruProfileValueSet");
        }
        else
        if(sendIter == 3) /*PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum)*/
        {
            /* AUTODOC: for VLAN 2 set MRU packet command */
            rc = prvTgfBrgVlanMruCommandSet(prvTgfDevNum, CPSS_PACKET_CMD_TRAP_TO_CPU_E);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMruProfileIdxSet");

            packetIndex = 2;/* same packet as before */
            expectDrop = GT_TRUE;

            PRV_UTF_LOG0_MAC("\n========== LLC but not 'cut through' not use LLC field <packet len> 0x0001 ==========\n\n");
            currMru = SIP6_PACKET_SIZE_FROM_LLC(0x0001) - 1 + VLAN_MRU_HOLD_CRC_BYTES(prvTgfDevNum);
            rc = prvTgfBrgVlanMruProfileValueSet(PRV_TGF_MRU_INDEX_CNS, currMru);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMruProfileValueSet");

        }
        else
        if(sendIter == 4) /*PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum)*/
        {
            /* AUTODOC: for VLAN 2 set MRU packet command */
            rc = prvTgfBrgVlanMruCommandSet(prvTgfDevNum, CPSS_PACKET_CMD_MIRROR_TO_CPU_E);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMruProfileIdxSet");

            packetIndex = 2;
            expectDrop = GT_FALSE;

            PRV_UTF_LOG0_MAC("\n========== LLC but not 'cut through' not use LLC field <packet len> 0x0001 ==========\n\n");
            currMru += 1;
            rc = prvTgfBrgVlanMruProfileValueSet(PRV_TGF_MRU_INDEX_CNS, currMru );
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMruProfileValueSet");
        }
        else
        if(sendIter == 5) /*PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum)*/
        {
            /* AUTODOC: for VLAN 2 set MRU packet command */
            rc = prvTgfBrgVlanMruCommandSet(prvTgfDevNum, CPSS_PACKET_CMD_DROP_HARD_E);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMruProfileIdxSet");

            packetIndex = 2;
            expectDrop = GT_TRUE;
            PRV_UTF_LOG0_MAC("\n========== LLC but not 'cut through' use full packet length ==========\n\n");

            currMru = prvTgfPacketInfoArr[packetIndex]->totalLen/*not include CRC!*/ + VLAN_MRU_HOLD_CRC_BYTES(prvTgfDevNum) - 1;
            rc = prvTgfBrgVlanMruProfileValueSet(PRV_TGF_MRU_INDEX_CNS, currMru );
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMruProfileValueSet");

        }
        else
        /*if(sendIter == 6) && PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum)*/
        {
            packetIndex = 2;
            expectDrop = GT_FALSE;

            PRV_UTF_LOG0_MAC("\n========== LLC but not 'cut through' use full packet length ==========\n\n");
            currMru += 1;
            rc = prvTgfBrgVlanMruProfileValueSet(PRV_TGF_MRU_INDEX_CNS, currMru );
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMruProfileValueSet");

        }
        /* clear capturing RxPcktTable */
        rc = tgfTrafficTableRxPcktTblClear();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

        /* enable capture */
        portInterface.type            = CPSS_INTERFACE_PORT_E;
        portInterface.devPort.hwDevNum  = prvTgfDevNum;
        portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS];

        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CAPTURE_PORT_IDX_CNS]);

        /* reset counters */
        rc = prvTgfEthCountersReset(prvTgfDevNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset: %d", prvTgfDevNum);

        /* setup packet */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, prvTgfPacketInfoArr[packetIndex],
                                 prvTgfBurstCount, 0, NULL);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup");

        PRV_UTF_LOG2_MAC("\n========== Sending packet iter=%d  on port=%d==========\n\n",
                         sendIter + 1, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* AUTODOC: send 2 packets on port 0 with: */
        /* AUTODOC:   DA=00:00:00:00:00:02, SA=00:00:00:00:00:11, VID=2 */
        /* AUTODOC:   payload sizes [84,534] bytes */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* disable capture */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                     prvTgfDevNum, portInterface.devPort.portNum);

        PRV_UTF_LOG1_MAC("Expecting the packet [%s] the MRU check ... lets check counters: \n\n",
            (GT_CHAR*)((expectDrop == GT_TRUE) ? "DROPPED by" : "PASS"));

        PRV_UTF_LOG2_MAC("currMru[%d] , packet size [%d](without CRC!) \n\n",
            currMru ,
            prvTgfPacketInfoArr[packetIndex]->totalLen);

        /* AUTODOC: verify traffic: */
        /* AUTODOC:   1 packet (size<512) - received on port 2 */
        /* AUTODOC:   2 packet (size>512) - dropped */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            IN  GT_U32                         expected_packetsCountRx;
            IN  GT_U32                         expected_packetsCountTx;

            if(portIter == PRV_TGF_SEND_PORT_IDX_CNS)
            {
                expected_packetsCountRx = 1;
                expected_packetsCountTx = 1;
            }
            else
            if(portIter != PRV_TGF_CAPTURE_PORT_IDX_CNS)
            {
                expected_packetsCountRx = 0;
                expected_packetsCountTx = 0;
            }
            else
            if(expectDrop == GT_TRUE)/*(portIter == PRV_TGF_CAPTURE_PORT_IDX_CNS)*/
            {
                expected_packetsCountRx = 0;
                expected_packetsCountTx = 0;
            }
            else/* (expectDrop == GT_FALSE) and (portIter == PRV_TGF_CAPTURE_PORT_IDX_CNS)*/
            {
                expected_packetsCountRx = 1;
                expected_packetsCountTx = 1;
            }

            packetLen = prvTgfPacketInfoArr[packetIndex]->totalLen;

            /* check ETH counters */
            rc = prvTgfEthCountersCheck(prvTgfDevNum,
                                        prvTgfPortsArray[portIter],
                                        expected_packetsCountRx*prvTgfBurstCount,
                                        expected_packetsCountTx*prvTgfBurstCount,
                                        packetLen, prvTgfBurstCount);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");
        }

        /* clear VFD array */
        cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

        /* set vfd for destination MAC */
        vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
        vfdArray[0].modeExtraInfo = 0;
        vfdArray[0].offset = 0;
        vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
        cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

        /* get trigger counters */
        rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
        PRV_UTF_LOG2_MAC("numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

        /* check pattern of TriggerCounters */
        UTF_VERIFY_EQUAL2_STRING_MAC(numTriggers , (expectDrop == GT_TRUE)? 0 : 1,
                                     "get another trigger that expected: expected - %d, received - %d\n",
                                     numTriggers , (expectDrop == GT_TRUE)? 0 : 1);
    }

    /* AUTODOC: Restore MRU packet command  for VLAN 2*/
    rc = prvTgfBrgVlanMruCommandSet(prvTgfDevNum, CPSS_PACKET_CMD_DROP_HARD_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMruProfileIdxSet");

}

/**
* @internal prvTgfBrgUntaggedMruCheckConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgUntaggedMruCheckConfigRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: restore default MRU value for MRU profile 1 */
    rc = prvTgfBrgVlanMruProfileValueSet(PRV_TGF_MRU_INDEX_CNS, PRV_TGF_DEF_MRU_VALUE_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMruProfileValueSet");

    /* AUTODOC: restore MRU profile index for a VLAN 2 */
    rc = prvTgfBrgVlanMruProfileIdxRestore(PRV_TGF_VLANID_2_CNS, PRV_TGF_DEF_MRU_INDEX_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMruProfileIdxRestore");

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_2_CNS );
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_2_CNS);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d",
                                 prvTgfDevNum, GT_TRUE);

    /* AUTODOC: restore default PVID for port 0 */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], 1);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], 1);
}


