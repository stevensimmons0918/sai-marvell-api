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
* @file prvTgfIpv4Ipv6UcMcRoutingAddDelAndFillLpm.c
*
* @brief IPV4 & IPV6 UC Routing when filling the Lpm and using many Add and
* Delete operations.
*
* @version   1
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfIpGen.h>
#include <common/tgfBridgeGen.h>
#include <ip/prvTgfIpv4Ipv6UcMcRoutingAddDelAndFillLpm.h>

/* the LPM DB id for LPM Table */
static GT_U32        prvTgfLpmDBId             = 0;

/* number of prefixes expected to be added in a device with 40K LPM lines
   blocksAllocationMethod = do_not_share
   maxNumOfPbrEntries = 0
   firstOctetIndexToChange = 6 --> changing octet 6 then octet 7 sequentially */
/* expected value when SWAP is used at the beginning of memory allocation     --> 9127
   expected value when SWAP is used before allocating a new memory  block     --> 8702
   expected value when Shrink is done - defragmentationEnable=GT_TRUE         --> 9127
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE   --> 8702 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_40K_LINES_6_7_UC_OCTET_CNS     9127

/* number of prefixes expected to be added in a device with 40K LPM lines
   blocksAllocationMethod = do_not_share
   maxNumOfPbrEntries = 0
   firstOctetIndexToChange = 3 --> changing MC source octet 12 then octet 13 sequentially */
/* expected value when SWAP is used at the beginning of memory allocation     --> 1363
   expected value when SWAP is used before allocating a new memory  block     --> 1363
   expected value when Shrink is done - defragmentationEnable=GT_TRUE         --> 1363
   expected value when Shrink+Merge is done - defragmentationEnable=GT_TRUE   --> 1363
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE   --> 1363 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_40K_LINES_12_13_OCTET_MC_SRC_CHANGE_CNS    1363
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_12_13_OCTET_MC_SRC_CHANGE_MIN_FALCON_CNS        1726
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_12_13_OCTET_MC_SRC_CHANGE_FALCON_GM_CNS         0 /* find the value after running GM */

/* number of prefixes expected to be added in a device with 40K LPM lines
   blocksAllocationMethod = do_not_share
   maxNumOfPbrEntries = 0
   firstOctetIndexToChange = 14 --> changing MC source octet 15 then octet 14 sequentially */
/* expected value when SWAP is used at the beginning of memory allocation     --> 9127
   expected value when SWAP is used before allocating a new memory  block     --> 8702
   expected value when Shrink is done - defragmentationEnable=GT_TRUE         --> 9127
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE   --> 8702 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_40K_LINES_15_14_OCTET_MC_SRC_CHANGE_CNS    9127
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_15_14_OCTET_MC_SRC_CHANGE_MIN_FALCON_CNS        12312
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_15_14_OCTET_MC_SRC_CHANGE_FALCON_GM_CNS         1 /* find the value after running GM */

/* number of prefixes expected to be added in a device with 40K LPM lines
   blocksAllocationMethod = do_not_share
   maxNumOfPbrEntries = 0
   firstOctetIndexToChange = 3 --> changing MC source octet 3 then octet 2 sequentially */
/* expected value when SWAP is used at the beginning of memory allocation     --> 30631
   expected value when SWAP is used before allocating a new memory  block     --> 30206
   expected value when Shrink is done - defragmentationEnable=GT_TRUE         --> 30631
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 30206 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_40K_LINES_3_2_OCTET_MC_SRC_CHANGE_CNS   30631
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_3_2_OCTET_MC_SRC_CHANGE_MIN_FALCON_CNS       46104
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_3_2_OCTET_MC_SRC_CHANGE_FALCON_GM_CNS        8/* find the value after running GM */

/* number of prefixes expected to be added in a device with 40K LPM lines
   blocksAllocationMethod = do_not_share
   maxNumOfPbrEntries = 0
   firstOctetIndexToChange = 0 --> changing MC source octet 0 then octet 1 sequentially */
/* expected value when SWAP is used at the beginning of memory allocation     --> 4773
   expected value when SWAP is used before allocating a new memory  block     --> 4773
   expected value when 2 SWAP areas is in used                                --> 4773
   expected value when Shrink is done - defragmentationEnable=GT_TRUE         --> 4773
   expected value when Shrink+Merge is done - defragmentationEnable=GT_TRUE   --> 4773
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE   --> 4773 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_40K_LINES_0_1_OCTET_MC_SRC_CHANGE_CNS  4773
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_0_1_OCTET_MC_SRC_CHANGE_MIN_FALCON_CNS      3887 /*with merge. 3839 - without merge */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_0_1_OCTET_MC_SRC_CHANGE_FALCON_GM_CNS       9/* find the value after running GM */

/* number of prefixes expected to be added in a device with 40K LPM lines
   blocksAllocationMethod = do_not_share
   maxNumOfPbrEntries = 0
   firstOctetIndexToChange = 3 --> changing MC group octet 12 then octet 13 sequentially */
/* expected value when SWAP is used at the beginning of memory allocation     --> 585
   expected value when SWAP is used before allocating a new memory  block     --> 585
   expected value when 2 SWAP areas is in used                                --> 511
   expected value when Shrink is done - defragmentationEnable=GT_TRUE         --> 511
   expected value when Shrink+Merge is done - defragmentationEnable=GT_TRUE   --> 511
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE   --> 585 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_40K_LINES_12_13_OCTET_MC_GRP_CHANGE_CNS     511
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_12_13_OCTET_MC_GRP_CHANGE_MIN_FALCON_CNS         508 /* with merge. 375 - without merge */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_12_13_OCTET_MC_GRP_CHANGE_FALCON_GM_CNS          5/* find the value after running GM */

/* number of prefixes expected to be added in a device with 40K LPM lines
   blocksAllocationMethod = do_not_shares
   maxNumOfPbrEntries = 0
   firstOctetIndexToChange = 14 --> changing MC group octet 15 then octet 14 sequentially */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_40K_LINES_15_14_OCTET_MC_GRP_CHANGE_CNS     681
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_15_14_OCTET_MC_GRP_CHANGE_MIN_FALCON_CNS         633 /* with merge. 622 - without merge */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_15_14_OCTET_MC_GRP_CHANGE_FALCON_GM_CNS          6/* find the value after running GM */


/* number of prefixes expected to be added in a device with 40K LPM lines
   blocksAllocationMethod = do_not_share
   maxNumOfPbrEntries = 0
   firstOctetIndexToChange = 3 --> changing MC group octet 3 then octet 2 sequentially */
/* expected value when SWAP is used at the beginning of memory allocation     --> 2727
   expected value when SWAP is used before allocating a new memory  block     --> 2727
   expected value when 2 SWAP areas is in used                                --> 2727
   expected value when Shrink is done - defragmentationEnable=GT_TRUE         --> 2727
   expected value when Shrink+Merge is done - defragmentationEnable=GT_TRUE   --> 2727
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE   --> 2727 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_40K_LINES_3_2_OCTET_MC_GRP_CHANGE_CNS      2727
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_3_2_OCTET_MC_GRP_CHANGE_MIN_FALCON_CNS          2558 /*2554 - without compressed root bucket */

/* number of prefixes expected to be added in a device with 40K LPM lines
   blocksAllocationMethod = do_not_share
   maxNumOfPbrEntries = 0
   firstOctetIndexToChange = 0 --> changing MC group octet 0 then octet 1 sequentially */
/* expected value when SWAP is used at the beginning of memory allocation     --> 4773
   expected value when SWAP is used before allocating a new memory  block     --> 4773
   expected value when 2 SWAP areas is in used                                --> 4773
   expected value when Shrink is done - defragmentationEnable=GT_TRUE         --> 4773
   expected value when Shrink+Merge is done - defragmentationEnable=GT_TRUE   --> 4773
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE   --> 4773 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_40K_LINES_0_1_OCTET_MC_GRP_CHANGE_CNS     4773
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_LINES_0_1_OCTET_MC_GRP_CHANGE_MIN_FALCON_CN    3905 /* 3906 -without compressed root bucket */

/* number of prefixes expected to be added in a device with 40K LPM lines
   blocksAllocationMethod = share
   maxNumOfPbrEntries = 0
   firstOctetIndexToChange = 6 --> changing UC octet 6 then octet 7 sequentially */
/* expected value when SWAP is used at the beginning of memory allocation     --> 35049
   expected value when SWAP is used before allocating a new memory  block     --> 34558
   expected value when 2 SWAP areas is in used                                --> 34814
   expected value when defragmentationEnable=GT_TRUE                          --> 34814
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE   --> 34558 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_40K_LINES_6_7_UC_OCTET_CNS     34814

/* number of prefixes expected to be added in a device with 40K LPM lines
   blocksAllocationMethod = share
   maxNumOfPbrEntries = 0
   firstOctetIndexToChange = 14 --> changing MC source octet 15 then octet 14 sequentially */
/* expected value when SWAP is used at the beginning of memory allocation     --> 35042
   expected value when SWAP is used before allocating a new memory  block     --> 34558
   expected value when 2 SWAP areas is in used                               --> 34814
   expected value when defragmentationEnable=GT_TRUE                         --> 34814
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 34558 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_40K_LINES_15_14_OCTET_MC_SRC_CHANGE_CNS    34814
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_15_14_OCTET_MC_SRC_CHANGE_MIN_FALCON_CNS        49920 /* 48640 - without compressed root bucket */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_15_14_OCTET_MC_SRC_CHANGE_FALCON_GM_CNS         2 /* find the value after running GM */

/* number of prefixes expected to be added in a device with 40K LPM lines
   blocksAllocationMethod = share
   maxNumOfPbrEntries = 0
   firstOctetIndexToChange = 3 --> changing MC source octet 12 then octet 13 sequentially */
/* expected value when SWAP is used at the beginning of memory allocation     --> 5224
   expected value when SWAP is used before allocating a new memory  block     --> 5191
   expected value when 2 SWAP areas is in used                                --> 5170
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 5195 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_40K_LINES_12_13_OCTET_MC_SRC_CHANGE_CNS    5170
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_12_13_OCTET_MC_SRC_CHANGE_MIN_FALCON_CNS        4183 /* 4161 - without compressed root bucket */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_12_13_OCTET_MC_SRC_CHANGE_FALCON_GM_CNS         3 /* find the value after running GM */

/* number of prefixes expected to be added in a device with 40K LPM lines
   blocksAllocationMethod = share
   maxNumOfPbrEntries = 0
   firstOctetIndexToChange = 3 --> changing MC source octet 3 then octet 2 sequentially */
/* expected value when SWAP is used at the beginning of memory allocation     --> 35043
   expected value when SWAP is used before allocating a new memory  block     --> 34558
   expected value when 2 SWAP areas is in used                                --> 34814
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 34558 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_40K_LINES_3_2_OCTET_MC_SRC_CHANGE_CNS      34814
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_3_2_OCTET_MC_SRC_CHANGE_MIN_FALCON_CNS          49920 /* 49673 - without compressed root bucket */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_3_2_OCTET_MC_SRC_CHANGE_FALCON_GM_CNS           10/* find the value after running GM */

/* number of prefixes expected to be added in a device with 40K LPM lines
   blocksAllocationMethod = share
   maxNumOfPbrEntries = 0
   firstOctetIndexToChange = 0 --> changing MC source octet 0 then octet 1 sequentially */
/* expected value when SWAP is used at the beginning of memory allocation     --> 5282
   expected value when SWAP is used before allocating a new memory  block     --> 5287
   expected value when 2 SWAP areas is in used                                --> 5251
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE   --> 5285 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_40K_LINES_0_1_OCTET_MC_SRC_CHANGE_CNS          5261
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_0_1_OCTET_MC_SRC_CHANGE_MIN_FALCON_CNS              4339 /* 4408 - without compressed root bucket */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_0_1_OCTET_MC_SRC_CHANGE_FALCON_GM_CNS               11/* find the value after running GM */


/* number of prefixes expected to be added in a device with 40K LPM lines
   blocksAllocationMethod = share
   maxNumOfPbrEntries = 0
   firstOctetIndexToChange = 14 --> changing MC group octet 15 then octet 14 sequentially */
/* expected value when SWAP is used at the beginning of memory allocation     --> 778
   expected value when SWAP is used before allocating a new memory  block     --> 778
   expected value when 2 SWAP areas is in used                                --> 773
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE   --> 778 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_40K_LINES_15_14_OCTET_MC_GRP_CHANGE_CNS    773
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_15_14_OCTET_MC_GRP_CHANGE_MIN_FALCON_CNS        737
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_15_14_OCTET_MC_GRP_CHANGE_FALCON_GM_CNS         4/* find the value after running GM */

/* number of prefixes expected to be added in a device with 40K LPM lines
   blocksAllocationMethod = share
   maxNumOfPbrEntries = 0
   firstOctetIndexToChange = 3 --> changing MC group octet 12 then octet 13 sequentially */
/* expected value when SWAP is used at the beginning of memory allocation     --> 686
   expected value when SWAP is used before allocating a new memory  block     --> 686
   expected value when 2 SWAP areas is in used                                --> 682
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE   --> 686 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_40K_LINES_12_13_OCTET_MC_GRP_CHANGE_CNS    682
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_12_13_OCTET_MC_GRP_CHANGE_MIN_FALCON_CNS        650
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_12_13_OCTET_MC_GRP_CHANGE_FALCON_GM_CNS         7/* find the value after running GM */

/* number of prefixes expected to be added in a device with 40K LPM lines
   blocksAllocationMethod = share
   maxNumOfPbrEntries = 0
   firstOctetIndexToChange = 3 --> changing MC group octet 3 then octet 2 sequentially */
/* expected value when SWAP is used at the beginning of memory allocation     --> 3063
   expected value when SWAP is used before allocating a new memory  block     --> 3098
   expected value when 2 SWAP areas is in used                                --> 3049
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE  --> 3034 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_40K_LINES_3_2_OCTET_MC_GRP_CHANGE_CNS     3049
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_LINES_3_2_OCTET_MC_GRP_CHANGE_MIN_FALCON_CNS   2825 /* 2816 - without compressed root bucket */

/* number of prefixes expected to be added in a device with 40K LPM lines
   blocksAllocationMethod = share
   maxNumOfPbrEntries = 0
   firstOctetIndexToChange = 0 --> changing MC group octet 0 then octet 1 sequentially */
/* expected value when SWAP is used at the beginning of memory allocation     --> 5282
   expected value when SWAP is used before allocating a new memory  block     --> 5287
   expected value when 2 SWAP areas is in used                                --> 5251
   expected value when SWAP is not in used - defragmentationEnable=GT_FALSE   --> 5285 */
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_40K_LINES_0_1_OCTET_MC_GRP_CHANGE_CNS      5261
#define PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_LINES_0_1_OCTET_MC_GRP_CHANGE_MIN_FALCON_CNS    4122 /* 4109 - withgout compressed root bucket */

/* ********************************** */
/*  parameters relevan to Lpm Ha test */
/* ********************************** */
/* default vidx number */
#define PRV_TGF_DEF_VIDX_CNS              0

/* default VLAN Id */
#define PRV_TGF_SEND_VLANID_CNS           5

/* nextHop VLAN Id */
#define PRV_TGF_NEXTHOPE_VLANID_CNS       6

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS            4

/* port index for subscriber 1 */
#define PRV_TGF_MC_1_PORT_IDX_CNS         2

/* port index for subscriber 2 */
#define PRV_TGF_MC_2_PORT_IDX_CNS         3

/* port index for subscriber 2 */
#define PRV_TGF_MC_3_PORT_IDX_CNS         1

/* the counter set for a route entry is linked to */
#define PRV_TGF_COUNT_SET_CNS     CPSS_IP_CNT_SET0_E

/* MLL pair pointer indexes */
static GT_U32        prvTgfMllPointerIndex     = 5;

/* the Route entry index for ipv4 UC Route entry Table */
static GT_U32        prvTgfUcIpv4RouteEntryBaseIndex = 5;
/* the Route entry index for ipv6 UC Route entry Table */
static GT_U32        prvTgfUcIpv6RouteEntryBaseIndex = 6;
/* the Route entry index for ipv4 MC Route entry Table */
static GT_U32        prvTgfMcIpv4RouteEntryBaseIndex = 7;
/* the Route entry index for ipv6 MC Route entry Table */
static GT_U32        prvTgfMcIpv6RouteEntryBaseIndex = 8;

/* the Arp Address index of the Router ARP Table
   to write to the Route entry Arp nextHopARPPointer field */
static GT_U32        prvTgfRouterArpIndex      = 1;
static GT_ETHERADDR  prvTgfArpMacAddrGet;

/* default number of packets to send */
static GT_U32        prvTgfBurstCount   = 1;

/******************************* IPV4 UC Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}                /* saMac */
};
/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketPartIpv4 = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x2E,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0xFF,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,             /* csum */
    { 2,  2,  2,  4},   /* srcAddr */
    { 1,  1,  1,  3}    /* dstAddr */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_SEND_VLANID_CNS                       /* pri, cfi, VlanId */
};
/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};

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

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS \
    + TGF_IPV4_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* the ARP MAC address to write to the Router ARP Table */
static TGF_MAC_ADDR  prvTgfArpMac = {0x00, 0x00, 0x00, 0x00, 0x00, 0x22};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_SKIP_E ,      NULL},/*reserved for DSA tag (instead of the vlan tag)*/
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketPartIpv4},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/*capture type*/
static TGF_CAPTURE_MODE_ENT captureType = TGF_CAPTURE_MODE_MIRRORING_E;

/******************************* IPV4 MC Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part_ipv4Mc = {
    {0x01, 0x00, 0x5E, 0x01, 0x01, 0x01},               /* daMac = macGrp */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x33}                /* saMac */
};

/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketPart_ipv4Mc = {
    4,                                 /* version */
    5,                                 /* headerLen */
    0,                                 /* typeOfService */
    0x2A,                              /* totalLen = 42 */
    0,                                 /* id */
    0,                                 /* flags */
    0,                                 /* offset */
    0x40,                              /* timeToLive = 64 */
    0x04,                              /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS, /* csum */
    { 1,  1,  1,  1},                  /* srcAddr = ipSrc */
    { 224,1,  1,  1}                   /* dstAddr = ipGrp */
};
/* DATA of packet len = 22 (0x16) */
static GT_U8 prvTgfPayloadDataArr_ipv4Mc[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00
};
/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart_ipv4Mc = {
    sizeof(prvTgfPayloadDataArr_ipv4Mc),                       /* dataLength */
    prvTgfPayloadDataArr_ipv4Mc                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray_ipv4Mc[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part_ipv4Mc},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketPart_ipv4Mc},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart_ipv4Mc}
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_IPV4MC_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS \
    + TGF_IPV4_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataArr_ipv4Mc)

/******************************* IPV6 UC Test packet **********************************/

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePartIpv6 =
{
    TGF_ETHERTYPE_86DD_IPV6_TAG_CNS
};

/* packet's IPv6 */
static TGF_PACKET_IPV6_STC prvTgfPacketPartIpv6 = {
    6,                  /* version */
    0,                  /* trafficClass */
    0,                  /* flowLabel */
    0x02,               /* payloadLen */
    0x3b,               /* nextHeader */
    0x40,               /* hopLimit */
    {0x6545, 0, 0, 0, 0, 0, 0x0000, 0x3212}, /* TGF_IPV6_ADDR srcAddr */
    {0x1122, 0, 0, 0, 0, 0, 0xccdd, 0xeeff}  /* TGF_IPV6_ADDR dstAddr */
};
/* DATA of packet */
static GT_U8 prvTgfPayloadDataArrIpv6[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05
};
/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPartIpv6 = {
    sizeof(prvTgfPayloadDataArrIpv6),                       /* dataLength */
    prvTgfPayloadDataArrIpv6                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArrayIpv6[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePartIpv6},
    {TGF_PACKET_PART_IPV6_E,      &prvTgfPacketPartIpv6},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPartIpv6}
};

/******************************* IPV6 MC Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part_ipv6Mc = {
    {0x33, 0x33, 0x00, 0x00, 0x22, 0x22},               /* daMac = macGrp */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}                /* saMac */
};

/* packet's IPv6 */
static TGF_PACKET_IPV6_STC prvTgfPacketPart_ipv6Mc = {
    6,                  /* version */
    0,                  /* trafficClass */
    0,                  /* flowLabel */
    0x02,               /* payloadLen */
    0x3b,               /* nextHeader */
    0x40,               /* hopLimit */
    {0x2222, 0, 0, 0, 0, 0, 0x0000, 0x2222}, /* TGF_IPV6_ADDR srcAddr */
    {0xff00, 0, 0, 0, 0, 0, 0x0000, 0x2222}  /* TGF_IPV6_ADDR dstAddr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray_ipv6Mc[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part_ipv6Mc},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePartIpv6},
    {TGF_PACKET_PART_IPV6_E,      &prvTgfPacketPart_ipv6Mc},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_IPV6MC_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS \
    + TGF_IPV6_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/**
* @internal prvTgfIpLpmIpv4Ipv6UcPrefixAddSomePrefix function
* @endinternal
*
* @brief   Add some ipv4 and ipv6 Prefixes
*/
GT_STATUS prvTgfIpLpmIpv4Ipv6UcPrefixAddSomePrefix
(
    GT_VOID
)
{
        GT_STATUS rc = GT_OK;
        GT_BOOL                                 override;
        GT_BOOL                                 defragmentationEnable;
        PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
        GT_IPV6ADDR                             unIpv6AddrArray[4];
        GT_U32                                  ipv6PrefixLenArray[4];
        GT_IPADDR                               unIpv4AddrArray[4];
        GT_U32                                  ipv4PrefixLenArray[4];
        GT_U32                                  i=0;

        cpssOsMemSet(&nextHopInfo, '\0', sizeof(PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
        if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
        {
            nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_E;
        }
        else
        {
            nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E;
        }
        nextHopInfo.ipLttEntry.numOfPaths = 0;
        nextHopInfo.ipLttEntry.routeEntryBaseIndex = 1;
        nextHopInfo.ipLttEntry.ucRPFCheckEnable = GT_FALSE;
        nextHopInfo.ipLttEntry.sipSaCheckMismatchEnable = GT_FALSE;
        nextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;


        cpssOsMemSet(&unIpv6AddrArray, '\0', sizeof(unIpv6AddrArray));
        unIpv6AddrArray[0].arIP[15] = 0x1;
        ipv6PrefixLenArray[0]=128;

        unIpv6AddrArray[1].arIP[1] = 3;
        ipv6PrefixLenArray[1]=64;

        unIpv6AddrArray[2].arIP[1] = 3;
        unIpv6AddrArray[2].arIP[15] = 0x1;
        ipv6PrefixLenArray[2]=128;

        unIpv6AddrArray[3].arIP[0] = 0xfe;
        unIpv6AddrArray[3].arIP[1] = 0x80;
        ipv6PrefixLenArray[3]=10;

        override = GT_FALSE;
        defragmentationEnable = GT_TRUE;

        for (i=0;i<4;i++)
        {
            rc = prvTgfIpLpmIpv6UcPrefixAdd(prvTgfLpmDBId, 0, unIpv6AddrArray[i], ipv6PrefixLenArray[i], &nextHopInfo, override, defragmentationEnable);
            if (rc == GT_OK)
            {
                cpssOsPrintf("[ipv6 unicast added]  = [%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]/%d,\n\r",
                             unIpv6AddrArray[i].arIP[0]
                            ,unIpv6AddrArray[i].arIP[1]
                            ,unIpv6AddrArray[i].arIP[2]
                            ,unIpv6AddrArray[i].arIP[3]
                            ,unIpv6AddrArray[i].arIP[4]
                            ,unIpv6AddrArray[i].arIP[5]
                            ,unIpv6AddrArray[i].arIP[6]
                            ,unIpv6AddrArray[i].arIP[7]
                            ,unIpv6AddrArray[i].arIP[8]
                            ,unIpv6AddrArray[i].arIP[9]
                            ,unIpv6AddrArray[i].arIP[10]
                            ,unIpv6AddrArray[i].arIP[11]
                            ,unIpv6AddrArray[i].arIP[12]
                            ,unIpv6AddrArray[i].arIP[13]
                            ,unIpv6AddrArray[i].arIP[14]
                            ,unIpv6AddrArray[i].arIP[15]
                            ,ipv6PrefixLenArray[i]);
            }
        }

        /* IPV4 */

        cpssOsMemSet(&unIpv4AddrArray, '\0', sizeof(unIpv4AddrArray));
        unIpv4AddrArray[0].arIP[0] = 127;
        ipv4PrefixLenArray[0]=8;

        unIpv4AddrArray[1].arIP[0] = 192;
        ipv4PrefixLenArray[1]=8;

        unIpv4AddrArray[2].arIP[0] = 192;
        unIpv4AddrArray[2].arIP[1] = 94;
        unIpv4AddrArray[2].arIP[2] = 20;
        ipv4PrefixLenArray[2]=24;

        unIpv4AddrArray[3].arIP[0] = 192;
        unIpv4AddrArray[3].arIP[1] = 94;
        unIpv4AddrArray[3].arIP[2] = 20;
        unIpv4AddrArray[3].arIP[3] = 39;
        ipv4PrefixLenArray[3]=32;

        for (i=0;i<4;i++)
        {
            rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, 0, unIpv4AddrArray[i],ipv4PrefixLenArray[i], &nextHopInfo, override,defragmentationEnable);
            if (rc == GT_OK)
            {
                cpssOsPrintf("[ipv4 unicast added] = [%d]:[%d]:[%d]:[%d]/%d,\n\r",
                          unIpv4AddrArray[i].arIP[0]
                         ,unIpv4AddrArray[i].arIP[1]
                         ,unIpv4AddrArray[i].arIP[2]
                         ,unIpv4AddrArray[i].arIP[3]
                         ,ipv4PrefixLenArray[i]);

            }
        }
        return rc;
}

/**
* @internal prvTgfIpLpmIpv6UcPrefix64 function
* @endinternal
*
* @brief   Add ipv6 UC entries with prefixLen 64
*
* @param[in] totalNumIpUc             - number of ipv6 UC entries to add
* @param[in] firstOctetIndexToChange  - the first octet we want to change during the change.
*                                      next octet changed will be firstOctetIndexToChange+1
*                                      and so on
* @param[in] delete                   - is this an add or  operation
* @param[in] FirstByteIndex           - A value different then 0 will be given to the prefix
*                                      in location of  FirstByteIndex
* @param[in] order                    - GT_TRUE: increment the octets using u32Ip representation of the prefix
*                                      GT_FALSE:increment the octets using arIP representation of the prefix
*                                       None
*/
GT_VOID prvTgfIpLpmIpv6UcPrefix64
(
    GT_U32  totalNumIpUc,
    GT_U32  firstOctetIndexToChange,
    GT_BOOL delete,
    GT_U32  FirstByteIndex,
    GT_BOOL order
)
{
    GT_STATUS rc = GT_BAD_PARAM;
    GT_U32    i;

    GT_IPV6ADDR                             unIpv6Addr;
    GT_U32                                  ipPrefixLen;
    GT_BOOL                                 override;
    GT_BOOL                                 defragmentationEnable;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    PRV_TGF_LPM_RAM_CONFIG_STC              ramDbCfg;
    GT_U32                                  numOfPrefixesExpected=0;
    GT_U32                                  counter = 0;

    if (totalNumIpUc>= 256*256)
    {
       UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "totalNumIpUc should be less than 256*256, totalNumIpUc=%d\n\r",totalNumIpUc);
       return;
    }
    if (firstOctetIndexToChange>= 15)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "index should be less than 15, firstOctetIndexToChange=%d\n\r",firstOctetIndexToChange);
        return;
    }

    rc = prvTgfIpLpmRamDefaultConfigCalc(prvTgfDevNum,&ramDbCfg);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmRamDefaultConfigCalc: %d", prvTgfDevNum);

    PRV_UTF_LOG1_MAC("\n\n==== Testing Allocation Method %s\n\n ",
                     (ramDbCfg.blocksAllocationMethod == PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E)
                     ? "WITHOUT block sharing" : "WITH block sharing ");

    if(ramDbCfg.blocksAllocationMethod==PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E)
    {
        if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
        {
            numOfPrefixesExpected = 15379;
        }
        else
        {
            switch(ramDbCfg.blocksSizeArray[0])
            {
            case (_2K*4):
                /* each block memory has 2K lines (not including the blocks holding PBR)
                multiply by max of 20 blocks (in case there is no PBR) --> approximately 40K Lines */
                numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_40K_LINES_6_7_UC_OCTET_CNS;
                break;
            default:
                /* the test does not support at the moment any other configuration */
                rc = GT_NOT_SUPPORTED;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
                break;
            }

        }
    }
    else
    {
        /* if different octets of the same protocol can share the same block,
           we will have much more prefixes added then in case of no sharing */
        if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
        {
            numOfPrefixesExpected = 49920;
        }
        else
        {
            switch(ramDbCfg.blocksSizeArray[0])
            {
            case (_2K*4):
                /* each block memory has 2K lines (not including the blocks holding PBR)
                multiply by max of 20 blocks (in case there is no PBR) --> approximately 40K Lines */
                numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_40K_LINES_6_7_UC_OCTET_CNS;
                break;
            default:
                /* the test does not support at the moment any other configuration */
                rc = GT_NOT_SUPPORTED;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
                break;
            }
        }
    }

    cpssOsMemSet(&nextHopInfo, '\0', sizeof(nextHopInfo));
    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
       /* disable LPM validity check */
        prvTgfIpValidityCheckEnable(GT_FALSE);

        nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_E;
        defragmentationEnable = GT_TRUE;
    }
    else
    {
        nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E;
        defragmentationEnable = GT_TRUE;
    }
    nextHopInfo.ipLttEntry.numOfPaths = 0;
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = 0;
    nextHopInfo.ipLttEntry.ucRPFCheckEnable = GT_FALSE;
    nextHopInfo.ipLttEntry.sipSaCheckMismatchEnable = GT_FALSE;
    nextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;

    cpssOsMemSet(&unIpv6Addr, '\0', sizeof(GT_IPV6ADDR));
    unIpv6Addr.arIP[FirstByteIndex] = 0x5;
    ipPrefixLen = 64;
    override = GT_FALSE;


    cpssOsPrintf("test will take a while please wait....\n");
    for (i = 0; i < totalNumIpUc; i ++)
    {
        if(order == GT_FALSE)
        {
            if(i%256 == 0)
            {
                if(i!=0)
                {
                    (unIpv6Addr.arIP[firstOctetIndexToChange])++;
                }

                unIpv6Addr.arIP[firstOctetIndexToChange+1] = 0;
            }
            else
            {
                (unIpv6Addr.arIP[firstOctetIndexToChange+1])++;
            }
        }
        else
        {
            (unIpv6Addr.u32Ip[firstOctetIndexToChange]) =i+1;
        }

        if (delete==GT_TRUE)
        {
            rc = prvTgfIpLpmIpv6UcPrefixDel(prvTgfLpmDBId, 0, unIpv6Addr, ipPrefixLen);
        }
        else
        {
            rc = prvTgfIpLpmIpv6UcPrefixAdd(prvTgfLpmDBId, 0, unIpv6Addr, ipPrefixLen, &nextHopInfo, override, defragmentationEnable);
        }

        if ((rc == GT_OK) || (rc == GT_ALREADY_EXIST))
        {
            /* remove remark in case debug is needed
              cpssOsPrintf("[ipv6 unicast added/deleted][%d] = [%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d],\n\r",
                         i,
                         unIpv6Addr.arIP[0]
                         ,unIpv6Addr.arIP[1]
                         ,unIpv6Addr.arIP[2]
                         ,unIpv6Addr.arIP[3]
                         ,unIpv6Addr.arIP[4]
                         ,unIpv6Addr.arIP[5]
                         ,unIpv6Addr.arIP[6]
                         ,unIpv6Addr.arIP[7]
                         ,unIpv6Addr.arIP[8]
                         ,unIpv6Addr.arIP[9]
                         ,unIpv6Addr.arIP[10]
                         ,unIpv6Addr.arIP[11]
                         ,unIpv6Addr.arIP[12]
                         ,unIpv6Addr.arIP[13]
                         ,unIpv6Addr.arIP[14]
                         ,unIpv6Addr.arIP[15]);*/
            if (rc == GT_OK)
            {
                counter++;
            }
        }
        else
        {
            break;
        }

        if ((PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))&&(counter%500==0))
        {
            /* enable LPM validity check */
            prvTgfIpValidityCheckEnable(GT_TRUE);

            rc = prvTgfIpValidityCheck(prvTgfLpmDBId,0,CPSS_IP_PROTOCOL_IPV6_E,CPSS_UNICAST_E);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck: %d", prvTgfDevNum);

            /* disable LPM validity check */
            prvTgfIpValidityCheckEnable(GT_FALSE);
        }
    }
    if (i!=numOfPrefixesExpected)
    {
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "\n We did not succeed in adding/deleting the amount wanted. added/deleted(%d/%d).\n",i,numOfPrefixesExpected);
    }
    else
    {
        cpssOsPrintf("\n We succeed in adding/deleting the amount wanted. added/deleted(%d/%d).\n",i,numOfPrefixesExpected);
    }
    PRV_UTF_LOG0_MAC("==== HW Octet to Block Mapping");
    rc = prvTgfIpLpmDbgHwOctetPerBlockPrint(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerBlockPrint: %d", prvTgfDevNum);

    PRV_UTF_LOG0_MAC("==== HW Lpm Lines Counters usage ");
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters: %d", prvTgfDevNum);

    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        /* enable LPM validity check */
        prvTgfIpValidityCheckEnable(GT_TRUE);
    }

    return;
}


/**
* @internal prvTgfIpLpmIpv6UcPrefix128 function
* @endinternal
*
* @brief   Add and delete ipv6 UC entries with prefixLen 128
*
* @param[in] first_totalNumIpUc       - number of ipv6 UC entries to add
* @param[in] second_totalNumIpUc      - number of ipv6 UC entries to add
* @param[in] firstOctetIndexToChange  - the first octet we want to change during the change.
*                                      next octet changed will be firstOctetIndexToChange+1
*                                      and so on
* @param[in] delete                   - is this an add or  operation
* @param[in] FirstByteIndex           - A value different then 0 will be given to the prefix
*                                      in location of  FirstByteIndex
* @param[in] order                    - GT_TRUE: increment the octets using u32Ip representation of the prefix
*                                      GT_FALSE:increment the octets using arIP representation of the prefix
*                                       None
*/
GT_VOID prvTgfIpLpmIpv6UcPrefix128
(
    GT_U32  first_totalNumIpUc,
    GT_U32  second_totalNumIpUc,
    GT_U32  firstOctetIndexToChange,
    GT_BOOL delete,
    GT_U32  FirstByteIndex,
    GT_BOOL order
)
{
    GT_STATUS rc = GT_BAD_PARAM;
    GT_U32    i;

    GT_IPV6ADDR                             unIpv6Addr;
    GT_U32                                  ipPrefixLen;
    GT_BOOL                                 override;
    GT_BOOL                                 defragmentationEnable;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;

    if (first_totalNumIpUc>= 256*256)
    {
       UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "first_totalNumIpUc should be less than 256*256, first_totalNumIpUc=%d\n\r",first_totalNumIpUc);
       return;
    }
    if (second_totalNumIpUc>= 256*256)
    {
       UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "second_totalNumIpUc should be less than 256*256, totalNumIpUc=%d\n\r",second_totalNumIpUc);
       return;
    }
    if (firstOctetIndexToChange>= 15)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "index should be less than 15, firstOctetIndexToChange=%d\n\r",firstOctetIndexToChange);
        return;
    }

    cpssOsMemSet(&nextHopInfo, '\0', sizeof(nextHopInfo));
    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_E;
    }
    else
    {
        nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E;
    }
    nextHopInfo.ipLttEntry.numOfPaths = 0;
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = 0;
    nextHopInfo.ipLttEntry.ucRPFCheckEnable = GT_FALSE;
    nextHopInfo.ipLttEntry.sipSaCheckMismatchEnable = GT_FALSE;
    nextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;

    cpssOsMemSet(&unIpv6Addr, '\0', sizeof(GT_IPV6ADDR));
    unIpv6Addr.arIP[FirstByteIndex] = 0x5;
    ipPrefixLen = 128;
    override = GT_FALSE;
    defragmentationEnable = GT_TRUE;

    cpssOsPrintf("test will take a while please wait....\n");
    for (i = 0; i < first_totalNumIpUc; i ++)
    {
        if(order == GT_FALSE)
        {
            if (i % 256 == 0)
            {
                if(i!=0)
                {
                    (unIpv6Addr.arIP[firstOctetIndexToChange])++;
                }

                unIpv6Addr.arIP[firstOctetIndexToChange+1] = 0;
            }

            else
            {
                (unIpv6Addr.arIP[firstOctetIndexToChange+1])++;
            }
        }
        else
        {
            (unIpv6Addr.u32Ip[firstOctetIndexToChange]) =i+1;
        }

        rc = prvTgfIpLpmIpv6UcPrefixAdd(prvTgfLpmDBId, 0, unIpv6Addr,ipPrefixLen, &nextHopInfo, override, defragmentationEnable);

        if ((rc == GT_OK) || (rc == GT_ALREADY_EXIST))
        {
            /*  remove remark in case debug is needed
              cpssOsPrintf("[ipv6 unicast added][%d] = [%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d],\n\r",
                         i,
                         unIpv6Addr.arIP[0]
                         ,unIpv6Addr.arIP[1]
                         ,unIpv6Addr.arIP[2]
                         ,unIpv6Addr.arIP[3]
                         ,unIpv6Addr.arIP[4]
                         ,unIpv6Addr.arIP[5]
                         ,unIpv6Addr.arIP[6]
                         ,unIpv6Addr.arIP[7]
                         ,unIpv6Addr.arIP[8]
                         ,unIpv6Addr.arIP[9]
                         ,unIpv6Addr.arIP[10]
                         ,unIpv6Addr.arIP[11]
                         ,unIpv6Addr.arIP[12]
                         ,unIpv6Addr.arIP[13]
                         ,unIpv6Addr.arIP[14]
                         ,unIpv6Addr.arIP[15]);
                         */
        }
        else
        {
            break;
        }
    }
    if (i!=first_totalNumIpUc)
    {
         UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "\n We did not succeed in adding the amount wanted. added(%d/%d).\n",i,first_totalNumIpUc);
    }
    else
    {
        cpssOsPrintf("\n We succeed in adding the amount wanted. added(%d/%d).\n",i,first_totalNumIpUc);
    }

   cpssOsMemSet(&unIpv6Addr, '\0', sizeof(GT_IPV6ADDR));
   unIpv6Addr.arIP[FirstByteIndex] = 0x6;

   cpssOsPrintf("test will take a while please wait....\n");
   for (i = 0; i < second_totalNumIpUc; i ++)
   {
        if(i%256 == 0)
        {
            if(i!=0)
                (unIpv6Addr.arIP[firstOctetIndexToChange])++;

            unIpv6Addr.arIP[firstOctetIndexToChange+1] = 0;
        }

        else
        {
            (unIpv6Addr.arIP[firstOctetIndexToChange+1])++;
        }

        rc = prvTgfIpLpmIpv6UcPrefixAdd(prvTgfLpmDBId, 0, unIpv6Addr,ipPrefixLen, &nextHopInfo, override, defragmentationEnable);

        if ((rc == GT_OK) || (rc == GT_ALREADY_EXIST))
        {
            /*  remove remark in case debug is needed
            cpssOsPrintf("[ipv6 unicast added][%d] = [%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d],\n\r",
                    i,
                    unIpv6Addr.arIP[0]
                    ,unIpv6Addr.arIP[1]
                    ,unIpv6Addr.arIP[2]
                    ,unIpv6Addr.arIP[3]
                    ,unIpv6Addr.arIP[4]
                    ,unIpv6Addr.arIP[5]
                    ,unIpv6Addr.arIP[6]
                    ,unIpv6Addr.arIP[7]
                    ,unIpv6Addr.arIP[8]
                    ,unIpv6Addr.arIP[9]
                    ,unIpv6Addr.arIP[10]
                    ,unIpv6Addr.arIP[11]
                    ,unIpv6Addr.arIP[12]
                    ,unIpv6Addr.arIP[13]
                    ,unIpv6Addr.arIP[14]
                    ,unIpv6Addr.arIP[15]);
                    */
        }
        else
        {
            break;
        }
   }
   if (i!=second_totalNumIpUc)
   {
       UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "\n We did not succeed in adding the amount wanted. added(%d/%d).\n",i,second_totalNumIpUc);
   }
   else
   {
       cpssOsPrintf("\n We succeed in adding the amount wanted. added(%d/%d).\n",i,second_totalNumIpUc);
   }

   if (delete == GT_TRUE)
   {

        cpssOsMemSet(&unIpv6Addr, '\0', sizeof(GT_IPV6ADDR));

        /* a fixed ipSrc */
        unIpv6Addr.arIP[FirstByteIndex] = 0x5;

        cpssOsPrintf("\n<<<<<<<<START DELETE<<<<<<<\n\r");
        for (i = 0; i < first_totalNumIpUc; i ++)
        {
            if(i%256 == 0)
            {
                if(i!=0)
                (unIpv6Addr.arIP[firstOctetIndexToChange])++;

                unIpv6Addr.arIP[firstOctetIndexToChange+1] = 0;
            }

            else
                (unIpv6Addr.arIP[firstOctetIndexToChange+1])++;


            rc = prvTgfIpLpmIpv6UcPrefixDel(prvTgfLpmDBId, 0, unIpv6Addr, ipPrefixLen);

            if ((rc == GT_OK))
            {
                /*  remove remark in case debug is needed
                cpssOsPrintf("[ipv6 unicast deleted][%d] = [%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d],\n\r",
                             i,
                             unIpv6Addr.arIP[0]
                             ,unIpv6Addr.arIP[1]
                             ,unIpv6Addr.arIP[2]
                             ,unIpv6Addr.arIP[3]
                             ,unIpv6Addr.arIP[4]
                             ,unIpv6Addr.arIP[5]
                             ,unIpv6Addr.arIP[6]
                             ,unIpv6Addr.arIP[7]
                             ,unIpv6Addr.arIP[8]
                             ,unIpv6Addr.arIP[9]
                             ,unIpv6Addr.arIP[10]
                             ,unIpv6Addr.arIP[11]
                             ,unIpv6Addr.arIP[12]
                             ,unIpv6Addr.arIP[13]
                             ,unIpv6Addr.arIP[14]
                             ,unIpv6Addr.arIP[15]);
                             */
            }
            else
            {
                break;
            }
        }
        if (i!=second_totalNumIpUc)
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "\n We did not succeed in deleting the amount wanted. deleted(%d/%d).\n",i,second_totalNumIpUc);
        }
        else
        {
            cpssOsPrintf("\n We succeed in deleting the amount wanted. deleted(%d/%d).\n",i,second_totalNumIpUc);
        }

        cpssOsMemSet(&unIpv6Addr, '\0', sizeof(GT_IPV6ADDR));

        /* a fixed ipSrc */
        unIpv6Addr.arIP[FirstByteIndex] = 0x6;

        cpssOsPrintf("\n<<<<<<<<START DELETE<<<<<<<\n\r");
        for (i = 0; i < second_totalNumIpUc; i ++)
        {
            if(i%256 == 0)
            {
                if(i!=0)
                (unIpv6Addr.arIP[firstOctetIndexToChange])++;

                unIpv6Addr.arIP[firstOctetIndexToChange+1] = 0;
            }

            else
                (unIpv6Addr.arIP[firstOctetIndexToChange+1])++;

            rc = prvTgfIpLpmIpv6UcPrefixDel(prvTgfLpmDBId, 0, unIpv6Addr,ipPrefixLen);

            if ((rc == GT_OK))
            {
                /*  remove remark in case debug is needed
                cpssOsPrintf("[ipv6 unicast deleted][%d] = [%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d],\n\r",
                             i,
                             unIpv6Addr.arIP[0]
                             ,unIpv6Addr.arIP[1]
                             ,unIpv6Addr.arIP[2]
                             ,unIpv6Addr.arIP[3]
                             ,unIpv6Addr.arIP[4]
                             ,unIpv6Addr.arIP[5]
                             ,unIpv6Addr.arIP[6]
                             ,unIpv6Addr.arIP[7]
                             ,unIpv6Addr.arIP[8]
                             ,unIpv6Addr.arIP[9]
                             ,unIpv6Addr.arIP[10]
                             ,unIpv6Addr.arIP[11]
                             ,unIpv6Addr.arIP[12]
                             ,unIpv6Addr.arIP[13]
                             ,unIpv6Addr.arIP[14]
                             ,unIpv6Addr.arIP[15]);
                             */
            }
            else
            {
                break;
            }
        }
        if (i!=second_totalNumIpUc)
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "\n We did not succeed in deleting the amount wanted. deleted(%d/%d).\n",i,second_totalNumIpUc);
        }
        else
        {
            cpssOsPrintf("\n We succeed in deleting the amount wanted. deleted(%d/%d).\n",i,second_totalNumIpUc);
        }
     }
    return;
}

/**
* @internal prvTgfIpLpmIpv6McPrefixSrcChange function
* @endinternal
*
* @brief   Test how many IPv6 MC could be added into a LPM DB,
*         when source is changed and group stay with same value
* @param[in] totalNumIpMc             - number of ipv6 MC entries to add
* @param[in] firstOctetIndexToChange  - the first octet we want to change during the prefix change.
* @param[in] secondOctetIndexToChange - the second octet we want to change during the prefix change.
* @param[in] prefixLengthGrp          - prefix len for the group
* @param[in] prefixLengthSrc          - prefix len for the src
* @param[in] delete                   - is this an add or  operation
* @param[in] order                    - GT_TRUE: increment the octets using u32Ip representation of the prefix
*                                      GT_FALSE:increment the octets using arIP representation of the prefix
*                                       None
*/
GT_VOID prvTgfIpLpmIpv6McPrefixSrcChange
(
    GT_U32  totalNumIpMc,
    GT_U32  firstOctetIndexToChange,
    GT_U32  secondOctetIndexToChange,
    GT_U32  prefixLengthGrp,
    GT_U32  prefixLengthSrc,
    GT_BOOL delete,
    GT_BOOL order
)
{
    GT_STATUS                       rc=GT_BAD_PARAM;
    GT_U32                          i;
    IN GT_IPV6ADDR                  ipGroup ;
    IN GT_U32                       ipGroupPrefixLen;
    IN GT_IPV6ADDR                  ipSrc;
    IN GT_U32                       ipSrcPrefixLen;
    IN PRV_TGF_IP_LTT_ENTRY_STC     mcRouteLttEntry;
    IN GT_BOOL                      override;
    IN GT_BOOL                      defragmentationEnable;
    PRV_TGF_LPM_RAM_CONFIG_STC      ramDbCfg;
    GT_U32                          numOfPrefixesExpected=0;

    if (totalNumIpMc>= 256*256)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "totalNumIpMc should be less than 256*256, totalNumIpUc=%d\n\r",totalNumIpMc);
        return;
    }
    if ((firstOctetIndexToChange>15)||(secondOctetIndexToChange>15))
    {
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "index should be less than 15, firstOctetIndexToChange=%d, secondOctetIndexToChange=%d\n\r",
                         firstOctetIndexToChange,secondOctetIndexToChange);
        return;
    }
    if(firstOctetIndexToChange==secondOctetIndexToChange)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "indexes should be different\n\r");
        return;
    }

    rc = prvTgfIpLpmRamDefaultConfigCalc(prvTgfDevNum,&ramDbCfg);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmRamDefaultConfigCalc: %d", prvTgfDevNum);

    PRV_UTF_LOG1_MAC("\n\n==== Testing Allocation Method %s\n\n ",
                     (ramDbCfg.blocksAllocationMethod == PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E)
                     ? "WITHOUT block sharing" : "WITH block sharing ");

    if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        if(ramDbCfg.blocksAllocationMethod==PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E)
        {
#ifdef GM_USED /*GM do not support new memory banks logic */
            if ((firstOctetIndexToChange==15)&&(secondOctetIndexToChange==14))
            {
                numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_15_14_OCTET_MC_SRC_CHANGE_FALCON_GM_CNS;
            }
            else
            {
                if ((firstOctetIndexToChange==3)&&(order==GT_TRUE))
                {
                    numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_12_13_OCTET_MC_SRC_CHANGE_FALCON_GM_CNS;

                }
                else
                    rc = GT_NOT_SUPPORTED;
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
            }
#else
            switch (ramDbCfg.lpmRamConfigInfo[0].sharedMemCnfg)
            {
            case PRV_TGF_CFG_SHARED_TABLE_MODE_MIN_L3_MAX_L2_NO_EM_E: /* 20 with 640 lines */
                if ((firstOctetIndexToChange==15)&&(secondOctetIndexToChange==14))
                {
                    numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_15_14_OCTET_MC_SRC_CHANGE_MIN_FALCON_CNS;
                }
                else
                {
                    if ((firstOctetIndexToChange==3)&&(order==GT_TRUE))
                    {
                        numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_12_13_OCTET_MC_SRC_CHANGE_MIN_FALCON_CNS;

                    }
                    else
                        rc = GT_NOT_SUPPORTED;
                        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
                }
                break;
            default:
                rc = GT_NOT_SUPPORTED;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
                break;
            }
#endif
        }
        else
        {
#ifdef GM_USED /*GM do not support new memory banks logic */
            if ((firstOctetIndexToChange==15)&&(secondOctetIndexToChange==14))
            {
                numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_15_14_OCTET_MC_SRC_CHANGE_FALCON_GM_CNS;
            }
            else
            {
                if ((firstOctetIndexToChange==3)&&(order==GT_TRUE))
                {
                    numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_12_13_OCTET_MC_SRC_CHANGE_FALCON_GM_CNS;

                }
                else
                    rc = GT_NOT_SUPPORTED;
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
            }
#else
            switch (ramDbCfg.lpmRamConfigInfo[0].sharedMemCnfg)
            {
            case PRV_TGF_CFG_SHARED_TABLE_MODE_MIN_L3_MAX_L2_NO_EM_E: /* 20 with 640 lines */
                if ((firstOctetIndexToChange==15)&&(secondOctetIndexToChange==14))
                {
                    numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_15_14_OCTET_MC_SRC_CHANGE_MIN_FALCON_CNS;
                }
                else
                {
                    if ((firstOctetIndexToChange==3)&&(order==GT_TRUE))
                    {
                        numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_12_13_OCTET_MC_SRC_CHANGE_MIN_FALCON_CNS;
                    }
                    else
                        rc = GT_NOT_SUPPORTED;
                        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
                }
                break;
            default:
                rc = GT_NOT_SUPPORTED;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
                break;
            }
#endif
        }
    }
    else
    {
        if(ramDbCfg.blocksAllocationMethod==PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E)
        {
            switch(ramDbCfg.blocksSizeArray[0])
            {
            case (_2K*4):
                /* each block memory has 2K lines (not including the blocks holding PBR)
                multiply by max of 20 blocks (in case there is no PBR) --> approximately 40K Lines */
                if ((firstOctetIndexToChange==15)&&(secondOctetIndexToChange==14))
                {
                    numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_40K_LINES_15_14_OCTET_MC_SRC_CHANGE_CNS;
                }
                else
                {
                    if ((firstOctetIndexToChange==3)&&(order==GT_TRUE))
                    {
                        numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_40K_LINES_12_13_OCTET_MC_SRC_CHANGE_CNS;

                    }
                    else
                        rc = GT_NOT_SUPPORTED;
                        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
                }
                break;
            default:
                /* the test does not support at the moment any other configuration */
                rc = GT_NOT_SUPPORTED;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
                break;
            }
        }
        else
        {
            /* if different octets of the same protocol can share the same block,
               we will have much more prefixes added then in case of no sharing */

            switch(ramDbCfg.blocksSizeArray[0])
            {
            case (_2K*4):
                /* each block memory has 2K lines (not including the blocks holding PBR)
                multiply by max of 20 blocks (in case there is no PBR) --> approximately 40K Lines */
                if ((firstOctetIndexToChange==15)&&(secondOctetIndexToChange==14))
                {
                    numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_40K_LINES_15_14_OCTET_MC_SRC_CHANGE_CNS;
                }
                else
                {
                    if ((firstOctetIndexToChange==3)&&(order==GT_TRUE))
                    {
                        numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_40K_LINES_12_13_OCTET_MC_SRC_CHANGE_CNS;
                    }
                    else
                        rc = GT_NOT_SUPPORTED;
                        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
                }
                break;
            default:
                /* the test does not support at the moment any other configuration */
                rc = GT_NOT_SUPPORTED;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
                break;
            }
        }
    }

    /*create a MC prefixes*/
    cpssOsMemSet(&mcRouteLttEntry, '\0', sizeof(PRV_TGF_IP_LTT_ENTRY_STC));
    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        mcRouteLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_E;
    }
    else
    {
        mcRouteLttEntry.routeType = PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E;
    }
    mcRouteLttEntry.numOfPaths = 0;
    mcRouteLttEntry.routeEntryBaseIndex = 0;
    mcRouteLttEntry.ucRPFCheckEnable = GT_FALSE;
    mcRouteLttEntry.sipSaCheckMismatchEnable = GT_FALSE;
    mcRouteLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;

    cpssOsMemSet(&ipGroup, '\0', sizeof(GT_IPV6ADDR));
    cpssOsMemSet(&ipSrc, '\0', sizeof(GT_IPV6ADDR));

    ipGroupPrefixLen = prefixLengthGrp;  /*128;*/
    ipSrcPrefixLen = prefixLengthSrc;    /*128; */
    override = GT_FALSE;
    defragmentationEnable = GT_TRUE;

    ipGroup.arIP[1] = 0x03;
    ipGroup.arIP[2] = 0x00;
    ipGroup.arIP[14] = 0x01;
    ipGroup.arIP[15] = 0x05;

    ipGroup.arIP[0] = 0xff;


    for (i = 0; i < totalNumIpMc; i ++)
    {
        if(order == GT_FALSE)
        {
            if (i % 256 == 0)
            {
                if(i!=0)
                    (ipSrc.arIP[secondOctetIndexToChange])++;

                ipSrc.arIP[firstOctetIndexToChange] = 0;
            }

            else
                (ipSrc.arIP[firstOctetIndexToChange])++;
        }
        else
        {
             (ipSrc.u32Ip[firstOctetIndexToChange]) = i;
        }


        if (delete==GT_TRUE)
        {
            rc = prvTgfIpLpmIpv6McEntryDel(prvTgfLpmDBId, 0, ipGroup, ipGroupPrefixLen, ipSrc, ipSrcPrefixLen);
        }
        else
        {
            rc = prvTgfIpLpmIpv6McEntryAdd(prvTgfLpmDBId, 0, ipGroup, ipGroupPrefixLen, ipSrc, ipSrcPrefixLen,
                                           &mcRouteLttEntry,override,defragmentationEnable);
        }

        if ((rc == GT_OK) || (rc == GT_ALREADY_EXIST))
        {
             /*  remove remark in case debug is needed
            cpssOsPrintf("[S, G][%d] = [%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d],\n\r",
                         i,
                         ipSrc.arIP[0]
                         ,ipSrc.arIP[1]
                         ,ipSrc.arIP[2]
                         ,ipSrc.arIP[3]
                         ,ipSrc.arIP[4]
                         ,ipSrc.arIP[5]
                         ,ipSrc.arIP[6]
                         ,ipSrc.arIP[7]
                         ,ipSrc.arIP[8]
                         ,ipSrc.arIP[9]
                         ,ipSrc.arIP[10]
                         ,ipSrc.arIP[11]
                         ,ipSrc.arIP[12]
                         ,ipSrc.arIP[13]
                         ,ipSrc.arIP[14]
                         ,ipSrc.arIP[15]);
                         */
        }
        else
            {
                break;
            }
    }
    if (i!=numOfPrefixesExpected)
    {
         if (delete==GT_TRUE)
             UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "\n We did not succeed in deleting the amount wanted. deleted(%d/%d).\n",i,numOfPrefixesExpected);
         else
             UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "\n We did not succeed in adding the amount wanted. added(%d/%d).\n",i,numOfPrefixesExpected);
    }
    else
    {
        if (delete==GT_TRUE)
            cpssOsPrintf("\n We succeed in deleting the amount wanted. deleted(%d/%d).\n",i,numOfPrefixesExpected);
        else
            cpssOsPrintf("\n We succeed in adding the amount wanted. added(%d/%d).\n",i,numOfPrefixesExpected);
    }

    PRV_UTF_LOG0_MAC("==== HW Octet to Block Mapping");
    rc = prvTgfIpLpmDbgHwOctetPerBlockPrint(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerBlockPrint: %d", prvTgfDevNum);

    PRV_UTF_LOG0_MAC("==== HW Lpm Lines Counters usage ");
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters: %d", prvTgfDevNum);

    return;
}



/**
* @internal prvTgfIpLpmIpv6McPrefixGroupChange function
* @endinternal
*
* @brief   Test how many IPv6 MC could be added into a LPM DB,
*         when group is changed and source stay with same value
* @param[in] totalNumIpMc             - number of ipv6 MC entries to add
* @param[in] firstOctetIndexToChange  - the first octet we want to change during the prefix change.
* @param[in] firstOctetIndexToChange  - the second octet we want to change during the prefix change.
* @param[in] prefixLengthGrp          - prefix len for the group
* @param[in] prefixLengthSrc          - prefix len for the src
* @param[in] delete                   - is this an add or  operation
* @param[in] order                    - GT_TRUE: increment the octets using u32Ip representation of the prefix
*                                      GT_FALSE:increment the octets using arIP representation of the prefix
*                                       None
*/
GT_VOID prvTgfIpLpmIpv6McPrefixGroupChange
(
    GT_U32  totalNumIpMc,
    GT_U32  firstOctetIndexToChange,
    GT_U32  secondOctetIndexToChange,
    GT_U32  prefixLengthGrp,
    GT_U32  prefixLengthSrc,
    GT_BOOL delete,
    GT_BOOL order
)
{
    GT_STATUS                       rc = GT_BAD_PARAM;
    GT_U32                          i;
    IN GT_IPV6ADDR                  ipGroup ;
    IN GT_U32                       ipGroupPrefixLen;
    IN GT_IPV6ADDR                  ipSrc;
    IN GT_U32                       ipSrcPrefixLen;
    IN PRV_TGF_IP_LTT_ENTRY_STC     mcRouteLttEntry;
    IN GT_BOOL                      override;
    IN GT_BOOL                      defragmentationEnable;
    PRV_TGF_LPM_RAM_CONFIG_STC      ramDbCfg;
    GT_U32                          numOfPrefixesExpected=0;

    if (totalNumIpMc>= 256*256)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "totalNumIpMc should be less than 256*256, totalNumIpUc=%d\n\r",totalNumIpMc);
        return;
    }
    if ((firstOctetIndexToChange>15)||(secondOctetIndexToChange>15))
    {
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "index should be less than 15, firstOctetIndexToChange=%d, secondOctetIndexToChange=%d\n\r",
                         firstOctetIndexToChange,secondOctetIndexToChange);
        return;
    }
    if(firstOctetIndexToChange==secondOctetIndexToChange)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "indexes should be different\n\r");
        return;
    }

    rc = prvTgfIpLpmRamDefaultConfigCalc(prvTgfDevNum,&ramDbCfg);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmRamDefaultConfigCalc: %d", prvTgfDevNum);

    PRV_UTF_LOG1_MAC("\n\n==== Testing Allocation Method %s\n\n ",
                     (ramDbCfg.blocksAllocationMethod == PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E)
                     ? "WITHOUT block sharing" : "WITH block sharing ");

    if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        if(ramDbCfg.blocksAllocationMethod==PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E)
        {
#ifdef GM_USED /*GM do not support new memory banks logic */
            if ((firstOctetIndexToChange==15)&&(secondOctetIndexToChange==14))
            {
                numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_15_14_OCTET_MC_GRP_CHANGE_FALCON_GM_CNS;
            }
            else
            {
                if ((firstOctetIndexToChange==3)&&(order==GT_TRUE))
                {
                    numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_12_13_OCTET_MC_GRP_CHANGE_FALCON_GM_CNS;

                }
                else
                    rc = GT_NOT_SUPPORTED;
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
            }
#else
            switch (ramDbCfg.lpmRamConfigInfo[0].sharedMemCnfg)
            {
            case PRV_TGF_CFG_SHARED_TABLE_MODE_MIN_L3_MAX_L2_NO_EM_E: /* 20 with 640 lines */
                if ((firstOctetIndexToChange==15)&&(secondOctetIndexToChange==14))
                {
                    numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_15_14_OCTET_MC_GRP_CHANGE_MIN_FALCON_CNS;
                }
                else
                {
                    if ((firstOctetIndexToChange==3)&&(order==GT_TRUE))
                    {
                        numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_12_13_OCTET_MC_GRP_CHANGE_MIN_FALCON_CNS;

                    }
                    else
                        rc = GT_NOT_SUPPORTED;
                        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
                }
                break;
            default:
                rc = GT_NOT_SUPPORTED;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
                break;
            }
#endif
        }
        else
        {
#ifdef GM_USED /*GM do not support new memory banks logic */
            if ((firstOctetIndexToChange==15)&&(secondOctetIndexToChange==14))
            {
                numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_15_14_OCTET_MC_GRP_CHANGE_FALCON_GM_CNS;
            }
            else
            {
                if ((firstOctetIndexToChange==3)&&(order==GT_TRUE))
                {
                    numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_12_13_OCTET_MC_GRP_CHANGE_FALCON_GM_CNS;

                }
                else
                    rc = GT_NOT_SUPPORTED;
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
            }
#else
            switch (ramDbCfg.lpmRamConfigInfo[0].sharedMemCnfg)
            {
            case PRV_TGF_CFG_SHARED_TABLE_MODE_MIN_L3_MAX_L2_NO_EM_E: /* 20 with 640 lines */
                if ((firstOctetIndexToChange==15)&&(secondOctetIndexToChange==14))
                {
                    numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_15_14_OCTET_MC_GRP_CHANGE_MIN_FALCON_CNS;
                }
                else
                {
                    if ((firstOctetIndexToChange==3)&&(order==GT_TRUE))
                    {
                        numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_12_13_OCTET_MC_GRP_CHANGE_MIN_FALCON_CNS;
                    }
                    else
                        rc = GT_NOT_SUPPORTED;
                        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
                }
                break;
            default:
                rc = GT_NOT_SUPPORTED;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
                break;
            }
#endif
        }
    }
    else
    {
        if(ramDbCfg.blocksAllocationMethod==PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E)
        {
            switch(ramDbCfg.blocksSizeArray[0])
            {
            case (_2K*4):
                /* each block memory has 2K lines (not including the blocks holding PBR)
                multiply by max of 20 blocks (in case there is no PBR) --> approximately 40K Lines */
                if ((firstOctetIndexToChange==15)&&(secondOctetIndexToChange==14))
                {
                    numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_40K_LINES_15_14_OCTET_MC_GRP_CHANGE_CNS;
                }
                else
                {
                    if ((firstOctetIndexToChange==3)&&(order==GT_TRUE))
                    {
                        numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_40K_LINES_12_13_OCTET_MC_GRP_CHANGE_CNS;

                    }
                    else
                    {
                        rc = GT_NOT_SUPPORTED;
                        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
                    }
                }
                break;
            default:
                /* the test does not support at the moment any other configuration */
                rc = GT_NOT_SUPPORTED;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
                break;
            }
        }
        else
        {
            /* if different octets of the same protocol can share the same block,
               we will have much more prefixes added then in case of no sharing */

            switch(ramDbCfg.blocksSizeArray[0])
            {
            case (_2K*4):
                /* each block memory has 2K lines (not including the blocks holding PBR)
                multiply by max of 20 blocks (in case there is no PBR) --> approximately 40K Lines */
                if ((firstOctetIndexToChange==15)&&(secondOctetIndexToChange==14))
                {
                    numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_40K_LINES_15_14_OCTET_MC_GRP_CHANGE_CNS;
                }
                else
                {
                    if ((firstOctetIndexToChange==3)&&(order==GT_TRUE))
                    {
                        numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_40K_LINES_12_13_OCTET_MC_GRP_CHANGE_CNS;
                    }
                    else
                    {
                        rc = GT_NOT_SUPPORTED;
                        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
                    }
                }

                break;
            default:
                /* the test does not support at the moment any other configuration */
                rc = GT_NOT_SUPPORTED;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
                break;
            }
        }
    }
    /*create a MC prefixes*/
    cpssOsMemSet(&mcRouteLttEntry, '\0', sizeof(PRV_TGF_IP_LTT_ENTRY_STC));
    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        mcRouteLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_E;
    }
    else
    {
        mcRouteLttEntry.routeType = PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E;
    }
    mcRouteLttEntry.numOfPaths = 0;
    mcRouteLttEntry.routeEntryBaseIndex = 0;
    mcRouteLttEntry.ucRPFCheckEnable = GT_FALSE;
    mcRouteLttEntry.sipSaCheckMismatchEnable = GT_FALSE;
    mcRouteLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;

    cpssOsMemSet(&ipGroup, '\0', sizeof(GT_IPV6ADDR));
    cpssOsMemSet(&ipSrc, '\0', sizeof(GT_IPV6ADDR));

    ipGroupPrefixLen = prefixLengthGrp;
    ipSrcPrefixLen = prefixLengthSrc;
    override = GT_FALSE;
    defragmentationEnable = GT_TRUE;

    ipGroup.arIP[0] = 0xff;

    /* a fixed ipSrc */
    ipSrc.arIP[1] = 0x03;
    ipSrc.arIP[2] = 0x00;
    ipSrc.arIP[14] = 0x01;
    ipSrc.arIP[15] = 0x05;

    for (i = 0; i < totalNumIpMc; i ++)
    {
        if(order == GT_FALSE)
        {
            if(i%256 == 0)
            {
                if(i!=0)
                    (ipGroup.arIP[secondOctetIndexToChange])++;

                ipGroup.arIP[firstOctetIndexToChange] = 0;
            }

            else
                (ipGroup.arIP[firstOctetIndexToChange])++;
        }
        else
        {
             (ipGroup.u32Ip[firstOctetIndexToChange]) =i;
        }

        if (delete==GT_TRUE)
        {
            rc = prvTgfIpLpmIpv6McEntryDel(prvTgfLpmDBId, 0, ipGroup, ipGroupPrefixLen, ipSrc, ipSrcPrefixLen);
        }
        else
        {
            rc = prvTgfIpLpmIpv6McEntryAdd(prvTgfLpmDBId, 0, ipGroup, ipGroupPrefixLen, ipSrc, ipSrcPrefixLen,
                                           &mcRouteLttEntry,override,defragmentationEnable);
        }

        if ((rc == GT_OK) || (rc == GT_ALREADY_EXIST))
        {
             /*  remove remark in case debug is needed
            cpssOsPrintf("[S, G][%d] = [%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d],\n\r",
                         i,
                         ipGroup.arIP[0]
                         ,ipGroup.arIP[1]
                         ,ipGroup.arIP[2]
                         ,ipGroup.arIP[3]
                         ,ipGroup.arIP[4]
                         ,ipGroup.arIP[5]
                         ,ipGroup.arIP[6]
                         ,ipGroup.arIP[7]
                         ,ipGroup.arIP[8]
                         ,ipGroup.arIP[9]
                         ,ipGroup.arIP[10]
                         ,ipGroup.arIP[11]
                         ,ipGroup.arIP[12]
                         ,ipGroup.arIP[13]
                         ,ipGroup.arIP[14]
                         ,ipGroup.arIP[15]);
                         */
        }
        else
        {
             break;
        }
    }
    if (i<numOfPrefixesExpected)
    {
        if (delete==GT_TRUE)
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "\n We did not succeed in deleting the amount wanted. deleted(%d/%d).\n",i,numOfPrefixesExpected);
        else
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "\n We did not succeed in adding the amount wanted. added(%d/%d).\n",i,numOfPrefixesExpected);
    }
    else
    {
        if (delete==GT_TRUE)
            cpssOsPrintf("\n We succeed in deleting the amount wanted. deleted(%d/%d).\n",i,numOfPrefixesExpected);
        else
            cpssOsPrintf("\n We succeed in adding the amount wanted. added(%d/%d).\n",i,numOfPrefixesExpected);
    }
    PRV_UTF_LOG0_MAC("==== HW Octet to Block Mapping");
    rc = prvTgfIpLpmDbgHwOctetPerBlockPrint(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerBlockPrint: %d", prvTgfDevNum);

    PRV_UTF_LOG0_MAC("==== HW Lpm Lines Counters usage ");
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters: %d", prvTgfDevNum);

    return;
}


/**
* @internal prvTgfIpLpmIpv6UcPrefix128AddDelMix function
* @endinternal
*
* @brief   Test how many IPv6 UC could be added into a LPM DB,
*         when doing scenario of Add and Delete prefixes in a mixed manner
* @param[in] totalNumIpUc             - number of ipv6 Uc entries to add
* @param[in] ipFirstByteValue         - value for the first octet
* @param[in] firstOctetIndexToChange  - the first octet we want to change during the prefix change.
* @param[in] mixNum                   - the index from were we want to mix the add and delete operation
* @param[in] order                    - GT_TRUE: increment the octets using u32Ip representation of the prefix
*                                      GT_FALSE:increment the octets using arIP representation of the prefix
*                                       None
*/
GT_VOID prvTgfIpLpmIpv6UcPrefix128AddDelMix
(
    GT_U32  totalNumIpUc,
    GT_U8   ipFirstByteValue,
    GT_U32  firstOctetIndexToChange,
    GT_U32  mixNum,
    GT_BOOL order
)
{
    GT_U32      entryNumNotMix = 0;
    GT_STATUS   rc = GT_BAD_PARAM;
    GT_U32      i = 0;

    GT_IPV6ADDR                             unIpv6Addr;
    GT_IPV6ADDR                             unIpv6AddrDel;
    GT_U32                                  ipPrefixLen;
    GT_BOOL                                 override;
    GT_BOOL                                 defragmentationEnable;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    GT_U32 numOfPrefixesDeleted=0;
    GT_U32 numOfPrefixesAdded=0;

    entryNumNotMix = totalNumIpUc - mixNum;

    if ((totalNumIpUc+entryNumNotMix)>= 256*256)

    {
       UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "totalNumIpUc-mixNum should be less than (256*256)/2, totalNumIpUc=%d,mixNum=%d\n\r",totalNumIpUc,mixNum);
       return;
    }
    if (firstOctetIndexToChange>= 15)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "index should be less than 15, firstOctetIndexToChange=%d\n\r",firstOctetIndexToChange);
        return;
    }
    if (mixNum >= totalNumIpUc)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "mixNum should be less than totalNumIpUc\n\r");
        return;
    }

    cpssOsMemSet(&nextHopInfo, '\0', sizeof(nextHopInfo));
    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_E;
    }
    else
    {
        nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E;
    }

    nextHopInfo.ipLttEntry.numOfPaths = 0;
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = 0;
    nextHopInfo.ipLttEntry.ucRPFCheckEnable = GT_FALSE;
    nextHopInfo.ipLttEntry.sipSaCheckMismatchEnable = GT_FALSE;
    nextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;

    cpssOsMemSet(&unIpv6Addr, '\0', sizeof(GT_IPV6ADDR));
    unIpv6Addr.arIP[0] = ipFirstByteValue;
    cpssOsMemSet(&unIpv6AddrDel, '\0', sizeof(GT_IPV6ADDR));
    unIpv6AddrDel.arIP[0] = ipFirstByteValue;
    ipPrefixLen = 128;
    override = GT_FALSE;
    defragmentationEnable = GT_TRUE;


    for (i = 0; i < totalNumIpUc + entryNumNotMix; i ++)
    {
        if(order == 0)
        {
           if(i <= totalNumIpUc)
           {
                if(i%256 == 0)
                {
                    if(i!=0)
                    (unIpv6Addr.arIP[firstOctetIndexToChange])++;

                    unIpv6Addr.arIP[firstOctetIndexToChange+1] = 0;
                }

                else
                (unIpv6Addr.arIP[firstOctetIndexToChange+1])++;
           }

           if(i>=entryNumNotMix)
           {
                if((i-entryNumNotMix)%256 == 0)
                {
                    if(i!=entryNumNotMix)
                        (unIpv6AddrDel.arIP[firstOctetIndexToChange])++;

                    unIpv6AddrDel.arIP[firstOctetIndexToChange+1] = 0;
                }

                else
                    (unIpv6AddrDel.arIP[firstOctetIndexToChange+1])++;
           }

        }
        else
        {
            if(i <= totalNumIpUc)
                (unIpv6Addr.u32Ip[firstOctetIndexToChange]) =i+1;

            if(i>entryNumNotMix)
                (unIpv6AddrDel.u32Ip[firstOctetIndexToChange]) =i+1;
        }


        if (i<entryNumNotMix)
        {

            rc = prvTgfIpLpmIpv6UcPrefixAdd(prvTgfLpmDBId, 0, unIpv6Addr,ipPrefixLen, &nextHopInfo, override, defragmentationEnable);

            /*  remove remark in case debug is needed
            cpssOsPrintf("[ipv6 unicast added  ][%d] = [%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d],\n\r",
                         i,
                         unIpv6Addr.arIP[0]
                         ,unIpv6Addr.arIP[1]
                         ,unIpv6Addr.arIP[2]
                         ,unIpv6Addr.arIP[3]
                         ,unIpv6Addr.arIP[4]
                         ,unIpv6Addr.arIP[5]
                         ,unIpv6Addr.arIP[6]
                         ,unIpv6Addr.arIP[7]
                         ,unIpv6Addr.arIP[8]
                         ,unIpv6Addr.arIP[9]
                         ,unIpv6Addr.arIP[10]
                         ,unIpv6Addr.arIP[11]
                         ,unIpv6Addr.arIP[12]
                         ,unIpv6Addr.arIP[13]
                         ,unIpv6Addr.arIP[14]
                         ,unIpv6Addr.arIP[15]); */

            if(rc!=GT_OK)
            {
                break;
            }
            else
            {
                numOfPrefixesAdded++;
            }
        }
        else if (i>=totalNumIpUc)
        {
            rc = prvTgfIpLpmIpv6UcPrefixDel(prvTgfLpmDBId, 0, unIpv6AddrDel,ipPrefixLen);

            /*  remove remark in case debug is needed
            cpssOsPrintf("[ipv6 unicast deleted][%d] = [%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d],\n\r",
                         i,
                         unIpv6AddrDel.arIP[0]
                         ,unIpv6AddrDel.arIP[1]
                         ,unIpv6AddrDel.arIP[2]
                         ,unIpv6AddrDel.arIP[3]
                         ,unIpv6AddrDel.arIP[4]
                         ,unIpv6AddrDel.arIP[5]
                         ,unIpv6AddrDel.arIP[6]
                         ,unIpv6AddrDel.arIP[7]
                         ,unIpv6AddrDel.arIP[8]
                         ,unIpv6AddrDel.arIP[9]
                         ,unIpv6AddrDel.arIP[10]
                         ,unIpv6AddrDel.arIP[11]
                         ,unIpv6AddrDel.arIP[12]
                         ,unIpv6AddrDel.arIP[13]
                         ,unIpv6AddrDel.arIP[14]
                         ,unIpv6AddrDel.arIP[15]);*/
            if (rc != GT_OK)
            {
                break;
            }
            else
            {
                numOfPrefixesDeleted++;
            }
        }
        else
        {
             /* Mix add and delete */
             rc = prvTgfIpLpmIpv6UcPrefixAdd(prvTgfLpmDBId, 0, unIpv6Addr,ipPrefixLen, &nextHopInfo, override, defragmentationEnable);
             /*  remove remark in case debug is needed
             cpssOsPrintf("[ipv6 unicast added  ][%d] = [%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d],\n\r",
                             i,
                             unIpv6Addr.arIP[0]
                             ,unIpv6Addr.arIP[1]
                             ,unIpv6Addr.arIP[2]
                             ,unIpv6Addr.arIP[3]
                             ,unIpv6Addr.arIP[4]
                             ,unIpv6Addr.arIP[5]
                             ,unIpv6Addr.arIP[6]
                             ,unIpv6Addr.arIP[7]
                             ,unIpv6Addr.arIP[8]
                             ,unIpv6Addr.arIP[9]
                             ,unIpv6Addr.arIP[10]
                             ,unIpv6Addr.arIP[11]
                             ,unIpv6Addr.arIP[12]
                             ,unIpv6Addr.arIP[13]
                             ,unIpv6Addr.arIP[14]
                             ,unIpv6Addr.arIP[15]);*/

            if (rc != GT_OK)
            {
                break;
            }
            else
            {
                numOfPrefixesAdded++;
            }

            rc = prvTgfIpLpmIpv6UcPrefixDel(prvTgfLpmDBId, 0, unIpv6AddrDel,ipPrefixLen);
            /*  remove remark in case debug is needed
            cpssOsPrintf("[ipv6 unicast deleted][%d] = [%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d],\n\r",
                     i,
                     unIpv6AddrDel.arIP[0]
                     ,unIpv6AddrDel.arIP[1]
                     ,unIpv6AddrDel.arIP[2]
                     ,unIpv6AddrDel.arIP[3]
                     ,unIpv6AddrDel.arIP[4]
                     ,unIpv6AddrDel.arIP[5]
                     ,unIpv6AddrDel.arIP[6]
                     ,unIpv6AddrDel.arIP[7]
                     ,unIpv6AddrDel.arIP[8]
                     ,unIpv6AddrDel.arIP[9]
                     ,unIpv6AddrDel.arIP[10]
                     ,unIpv6AddrDel.arIP[11]
                     ,unIpv6AddrDel.arIP[12]
                     ,unIpv6AddrDel.arIP[13]
                     ,unIpv6AddrDel.arIP[14]
                     ,unIpv6AddrDel.arIP[15]);*/
            if (rc != GT_OK)
            {
                break;
            }
            else
            {
                numOfPrefixesDeleted++;
            }
        }
    }
   if (numOfPrefixesAdded!=totalNumIpUc)
    {
       UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "\n We did not succeed in adding/deleting the amount wanted. added(%d/%d),  deleted(%d/%d).\n",numOfPrefixesAdded,totalNumIpUc,numOfPrefixesDeleted,totalNumIpUc);
    }
    else
    {
              cpssOsPrintf("\n We succeed in adding/deleting the amount wanted. added(%d/%d),deleted(%d/%d).\n",numOfPrefixesAdded,totalNumIpUc,numOfPrefixesDeleted,totalNumIpUc);
    }
    return;
}


/**
* @internal prvTgfIpLpmIpv4McPrefixSrcChange function
* @endinternal
*
* @brief   Test how many IPv4 MC could be added into a LPM DB,
*         when source is changed and group stay with same value
* @param[in] totalNumIpMc             - number of ipv4 MC entries to add
* @param[in] firstOctetIndexToChange  - the first octet we want to change during the prefix change.
* @param[in] secondOctetIndexToChange - the second octet we want to change during the prefix change.
* @param[in] prefixLengthGrp          - prefix len for the group
* @param[in] prefixLengthSrc          - prefix len for the src
* @param[in] delete                   - is this an add or  operation
* @param[in] order                    - GT_TRUE: increment the octets using u32Ip representation of the prefix
*                                      GT_FALSE:increment the octets using arIP representation of the prefix
*                                       None
*/
GT_VOID prvTgfIpLpmIpv4McPrefixSrcChange
(
    GT_U32  totalNumIpMc,
    GT_U32  firstOctetIndexToChange,
    GT_U32  secondOctetIndexToChange,
    GT_U32  prefixLengthGrp,
    GT_U32  prefixLengthSrc,
    GT_BOOL delete,
    GT_BOOL order
)
{
    GT_STATUS                       rc=GT_BAD_PARAM;
    GT_U32                          i;
    IN GT_IPADDR                    ipGroup ;
    IN GT_U32                       ipGroupPrefixLen;
    IN GT_IPADDR                    ipSrc;
    IN GT_U32                       ipSrcPrefixLen;
    IN PRV_TGF_IP_LTT_ENTRY_STC     mcRouteLttEntry;
    IN GT_BOOL                      override;
    IN GT_BOOL                      defragmentationEnable;
    PRV_TGF_LPM_RAM_CONFIG_STC      ramDbCfg;
    GT_U32                          numOfPrefixesExpected=0;

    if (totalNumIpMc>= 256*256)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "totalNumIpMc should be less than 256*256, totalNumIpUc=%d\n\r",totalNumIpMc);
        return;
    }
    if ((firstOctetIndexToChange>3)||(secondOctetIndexToChange>3))
    {
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "index should be less than 3, firstOctetIndexToChange=%d, secondOctetIndexToChange=%d\n\r",
                         firstOctetIndexToChange,secondOctetIndexToChange);
        return;
    }
    if(firstOctetIndexToChange==secondOctetIndexToChange)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "indexes should be different\n\r");
        return;
    }

    rc = prvTgfIpLpmRamDefaultConfigCalc(prvTgfDevNum,&ramDbCfg);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmRamDefaultConfigCalc: %d", prvTgfDevNum);

    PRV_UTF_LOG1_MAC("\n\n==== Testing Allocation Method %s\n\n ",
                     (ramDbCfg.blocksAllocationMethod == PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E)
                     ? "WITHOUT block sharing" : "WITH block sharing ");

    if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        if(ramDbCfg.blocksAllocationMethod==PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E)
        {
#ifdef GM_USED /*GM do not support new memory banks logic */
            if ((firstOctetIndexToChange==3)&&(secondOctetIndexToChange==2))
            {
                numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_3_2_OCTET_MC_SRC_CHANGE_FALCON_GM_CNS;
            }
            else
            {
                if ((firstOctetIndexToChange==0)&&(order==GT_TRUE))
                {
                    numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_0_1_OCTET_MC_SRC_CHANGE_FALCON_GM_CNS;

                }
                else
                    rc = GT_NOT_SUPPORTED;
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
            }
#else
            switch (ramDbCfg.lpmRamConfigInfo[0].sharedMemCnfg)
            {
            case PRV_TGF_CFG_SHARED_TABLE_MODE_MIN_L3_MAX_L2_NO_EM_E: /* 20 with 640 lines */
                if ((firstOctetIndexToChange==3)&&(secondOctetIndexToChange==2))
                {
                    numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_3_2_OCTET_MC_SRC_CHANGE_MIN_FALCON_CNS;
                }
                else
                {
                    if ((firstOctetIndexToChange==0)&&(order==GT_TRUE))
                    {
                        numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_0_1_OCTET_MC_SRC_CHANGE_MIN_FALCON_CNS;

                    }
                    else
                        rc = GT_NOT_SUPPORTED;
                        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
                }
                break;
            default:
                rc = GT_NOT_SUPPORTED;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
                break;
            }
#endif
        }
        else
        {
#ifdef GM_USED /*GM do not support new memory banks logic */
            if ((firstOctetIndexToChange==3)&&(secondOctetIndexToChange==2))
            {
                numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_3_2_OCTET_MC_SRC_CHANGE_FALCON_GM_CNS;
            }
            else
            {
                if ((firstOctetIndexToChange==0)&&(order==GT_TRUE))
                {
                    numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_0_1_OCTET_MC_SRC_CHANGE_FALCON_GM_CNS;

                }
                else
                    rc = GT_NOT_SUPPORTED;
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
            }
#else
            switch (ramDbCfg.lpmRamConfigInfo[0].sharedMemCnfg)
            {
            case PRV_TGF_CFG_SHARED_TABLE_MODE_MIN_L3_MAX_L2_NO_EM_E: /* 20 with 640 lines */
                if ((firstOctetIndexToChange==3)&&(secondOctetIndexToChange==2))
                {
                    numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_3_2_OCTET_MC_SRC_CHANGE_MIN_FALCON_CNS;
                }
                else
                {
                    if ((firstOctetIndexToChange==0)&&(order==GT_TRUE))
                    {
                        numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_0_1_OCTET_MC_SRC_CHANGE_MIN_FALCON_CNS;
                    }
                    else
                        rc = GT_NOT_SUPPORTED;
                        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
                }
                break;
            default:
                rc = GT_NOT_SUPPORTED;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
                break;
            }
#endif
        }
    }
    else
    {
        if(ramDbCfg.blocksAllocationMethod==PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E)
        {
            switch(ramDbCfg.blocksSizeArray[0])
            {
            case (_2K*4):
                /* each block memory has 2K lines (not including the blocks holding PBR)
                multiply by max of 20 blocks (in case there is no PBR) --> approximately 40K Lines */
                if ((firstOctetIndexToChange==3)&&(secondOctetIndexToChange==2))
                {
                    numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_40K_LINES_3_2_OCTET_MC_SRC_CHANGE_CNS;
                }
                else
                {
                    if ((firstOctetIndexToChange==0)&&(order==GT_TRUE))
                    {
                        numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_40K_LINES_0_1_OCTET_MC_SRC_CHANGE_CNS;

                    }
                    else
                    {
                        rc = GT_NOT_SUPPORTED;
                        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
                    }
                }
                break;
            default:
                /* the test does not support at the moment any other configuration */
                rc = GT_NOT_SUPPORTED;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
                break;
            }
        }
        else
        {
            /* if different octets of the same protocol can share the same block,
               we will have much more prefixes added then in case of no sharing */

            switch(ramDbCfg.blocksSizeArray[0])
            {
            case (_2K*4):
                /* each block memory has 2K lines (not including the blocks holding PBR)
                multiply by max of 20 blocks (in case there is no PBR) --> approximately 40K Lines */
                if ((firstOctetIndexToChange==3)&&(secondOctetIndexToChange==2))
                {
                    numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_40K_LINES_3_2_OCTET_MC_SRC_CHANGE_CNS;
                }
                else
                {
                    if ((firstOctetIndexToChange==0)&&(order==GT_TRUE))
                    {
                        numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_40K_LINES_0_1_OCTET_MC_SRC_CHANGE_CNS;
                    }
                    else
                    {
                        rc = GT_NOT_SUPPORTED;
                        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
                    }
                }

                break;
            default:
                /* the test does not support at the moment any other configuration */
                rc = GT_NOT_SUPPORTED;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
                break;
            }
        }
    }
    /*create a MC prefixes*/
    cpssOsMemSet(&mcRouteLttEntry, '\0', sizeof(PRV_TGF_IP_LTT_ENTRY_STC));
    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        mcRouteLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_E;
    }
    else
    {
        mcRouteLttEntry.routeType = PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E;
    }
    mcRouteLttEntry.numOfPaths = 0;
    mcRouteLttEntry.routeEntryBaseIndex = 0;
    mcRouteLttEntry.ucRPFCheckEnable = GT_FALSE;
    mcRouteLttEntry.sipSaCheckMismatchEnable = GT_FALSE;
    mcRouteLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;

    cpssOsMemSet(&ipGroup, '\0', sizeof(GT_IPADDR));
    cpssOsMemSet(&ipSrc, '\0', sizeof(GT_IPADDR));

    ipGroupPrefixLen = prefixLengthGrp;
    ipSrcPrefixLen = prefixLengthSrc;
    override = GT_FALSE;
    defragmentationEnable = GT_TRUE;

    ipGroup.arIP[0] = 224;
    ipGroup.arIP[1] = 6;
    ipGroup.arIP[2] = 0;
    ipGroup.arIP[3] = 0;

    for (i = 0; i < totalNumIpMc; i ++)
    {
        if(order == GT_FALSE)
        {
            if (i % 256 == 0)
            {
                if(i!=0)
                    (ipSrc.arIP[secondOctetIndexToChange])++;

                ipSrc.arIP[firstOctetIndexToChange] = 0;
            }

            else
                (ipSrc.arIP[firstOctetIndexToChange])++;
        }
        else
        {
             ipSrc.u32Ip = i;
        }


        if (delete==GT_TRUE)
        {
            rc = prvTgfIpLpmIpv4McEntryDel(prvTgfLpmDBId, 0, ipGroup, ipGroupPrefixLen, ipSrc, ipSrcPrefixLen);
        }
        else
        {
            rc = prvTgfIpLpmIpv4McEntryAdd(prvTgfLpmDBId, 0, ipGroup, ipGroupPrefixLen, ipSrc, ipSrcPrefixLen,
                                           &mcRouteLttEntry,override,defragmentationEnable);
        }

        if ((rc == GT_OK) || (rc == GT_ALREADY_EXIST))
        {
             /*  remove remark in case debug is needed
            cpssOsPrintf("[S, G][%d] = [%d]:[%d]:[%d]:[%d],\n\r",
                         i,
                         ipSrc.arIP[0]
                         ,ipSrc.arIP[1]
                         ,ipSrc.arIP[2]
                         ,ipSrc.arIP[3]);
                         */
        }
        else
            {
                break;
            }
    }
    if (i!=numOfPrefixesExpected)
    {
         if (delete==GT_TRUE)
             UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "\n We did not succeed in deleting the amount wanted. deleted(%d/%d).\n",i,numOfPrefixesExpected);
         else
             UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "\n We did not succeed in adding the amount wanted. added(%d/%d).\n",i,numOfPrefixesExpected);
    }
    else
    {
        if (delete==GT_TRUE)
            cpssOsPrintf("\n We succeed in deleting the amount wanted. deleted(%d/%d).\n",i,numOfPrefixesExpected);
        else
            cpssOsPrintf("\n We succeed in adding the amount wanted. added(%d/%d).\n",i,numOfPrefixesExpected);
    }
    PRV_UTF_LOG0_MAC("==== HW Octet to Block Mapping");
    rc = prvTgfIpLpmDbgHwOctetPerBlockPrint(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerBlockPrint: %d", prvTgfDevNum);

    PRV_UTF_LOG0_MAC("==== HW Lpm Lines Counters usage ");
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters: %d", prvTgfDevNum);

    PRV_UTF_LOG0_MAC("==== HW Lpm Lines Counters usage ");
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmMemoryAndCounters(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters: %d", prvTgfDevNum);

    return;
}



/**
* @internal prvTgfIpLpmIpv4McPrefixGroupChange function
* @endinternal
*
* @brief   Test how many IPv4 MC could be added into a LPM DB,
*         when group is changed and source stay with same value
* @param[in] totalNumIpMc             - number of ipv4 MC entries to add
* @param[in] firstOctetIndexToChange  - the first octet we want to change during the prefix change.
* @param[in] firstOctetIndexToChange  - the second octet we want to change during the prefix change.
* @param[in] prefixLengthGrp          - prefix len for the group
* @param[in] prefixLengthSrc          - prefix len for the src
* @param[in] delete                   - is this an add or  operation
* @param[in] order                    - GT_TRUE: increment the octets using u32Ip representation of the prefix
*                                      GT_FALSE:increment the octets using arIP representation of the prefix
*                                       None
*/
GT_VOID prvTgfIpLpmIpv4McPrefixGroupChange
(
    GT_U32  totalNumIpMc,
    GT_U32  firstOctetIndexToChange,
    GT_U32  secondOctetIndexToChange,
    GT_U32  prefixLengthGrp,
    GT_U32  prefixLengthSrc,
    GT_BOOL delete,
    GT_BOOL order
)
{
    GT_STATUS                       rc = GT_BAD_PARAM;
    GT_U32                          i;
    IN GT_IPADDR                    ipGroup ;
    IN GT_U32                       ipGroupPrefixLen;
    IN GT_IPADDR                    ipSrc;
    IN GT_U32                       ipSrcPrefixLen;
    IN PRV_TGF_IP_LTT_ENTRY_STC     mcRouteLttEntry;
    IN GT_BOOL                      override;
    IN GT_BOOL                      defragmentationEnable;
    PRV_TGF_LPM_RAM_CONFIG_STC      ramDbCfg;
    GT_U32                          numOfPrefixesExpected=0;

    if (totalNumIpMc>= 256*256)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "totalNumIpMc should be less than 256*256, totalNumIpUc=%d\n\r",totalNumIpMc);
        return;
    }
    if ((firstOctetIndexToChange>3)||(secondOctetIndexToChange>3))
    {
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "index should be less than 3, firstOctetIndexToChange=%d, secondOctetIndexToChange=%d\n\r",
                         firstOctetIndexToChange,secondOctetIndexToChange);
        return;
    }
    if(firstOctetIndexToChange==secondOctetIndexToChange)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "indexes should be different\n\r");
        return;
    }

    rc = prvTgfIpLpmRamDefaultConfigCalc(prvTgfDevNum,&ramDbCfg);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmRamDefaultConfigCalc: %d", prvTgfDevNum);

    PRV_UTF_LOG1_MAC("\n\n==== Testing Allocation Method %s\n\n ",
                     (ramDbCfg.blocksAllocationMethod == PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E)
                     ? "WITHOUT block sharing" : "WITH block sharing ");

    if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
     if(ramDbCfg.blocksAllocationMethod==PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E)
     {
         switch (ramDbCfg.lpmRamConfigInfo[0].sharedMemCnfg)
         {
         case PRV_TGF_CFG_SHARED_TABLE_MODE_MIN_L3_MAX_L2_NO_EM_E: /* 20 with 640 lines */
            if ((firstOctetIndexToChange==3)&&(secondOctetIndexToChange==2))
            {
                numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_3_2_OCTET_MC_GRP_CHANGE_MIN_FALCON_CNS;
            }
            else
            {
                if ((firstOctetIndexToChange==0)&&(order==GT_TRUE))
                {
                    numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_LINES_0_1_OCTET_MC_GRP_CHANGE_MIN_FALCON_CN;

                }
                else
                {
                    rc = GT_NOT_SUPPORTED;
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
                }
            }
            break;
         default:
             rc = GT_NOT_SUPPORTED;
             UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
             break;
         }
     }
     else
     {
         switch (ramDbCfg.lpmRamConfigInfo[0].sharedMemCnfg)
         {
         case PRV_TGF_CFG_SHARED_TABLE_MODE_MIN_L3_MAX_L2_NO_EM_E: /* 20 with 640 lines */
             if ((firstOctetIndexToChange==3)&&(secondOctetIndexToChange==2))
             {
                 numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_LINES_3_2_OCTET_MC_GRP_CHANGE_MIN_FALCON_CNS;
             }
             else
             {
                 if ((firstOctetIndexToChange==0)&&(order==GT_TRUE))
                 {
                     numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_LINES_0_1_OCTET_MC_GRP_CHANGE_MIN_FALCON_CNS;
                 }
                 else
                 {
                     rc = GT_NOT_SUPPORTED;
                     UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
                 }
             }
             break;
         default:
             rc = GT_NOT_SUPPORTED;
             UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
             break;
         }
     }
    }
    else
    {
        if(ramDbCfg.blocksAllocationMethod==PRV_TGF_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E)
        {
            switch(ramDbCfg.blocksSizeArray[0])
            {
            case (_2K*4):
                /* each block memory has 2K lines (not including the blocks holding PBR)
                multiply by max of 20 blocks (in case there is no PBR) --> approximately 40K Lines */
                if ((firstOctetIndexToChange==3)&&(secondOctetIndexToChange==2))
                {
                    numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_40K_LINES_3_2_OCTET_MC_GRP_CHANGE_CNS;
                }
                else
                {
                    if ((firstOctetIndexToChange==0)&&(order==GT_TRUE))
                    {
                        numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_NOT_ALLOW_SHARING_WITH_40K_LINES_0_1_OCTET_MC_GRP_CHANGE_CNS;

                    }
                    else
                    {
                        rc = GT_NOT_SUPPORTED;
                        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
                    }
                }
                break;
            default:
                /* the test does not support at the moment any other configuration */
                rc = GT_NOT_SUPPORTED;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
                break;
            }
        }
        else
        {
            /* if different octets of the same protocol can share the same block,
               we will have much more prefixes added then in case of no sharing */

            switch(ramDbCfg.blocksSizeArray[0])
            {
            case (_2K*4):
                /* each block memory has 2K lines (not including the blocks holding PBR)
                multiply by max of 20 blocks (in case there is no PBR) --> approximately 40K Lines */
                if ((firstOctetIndexToChange==3)&&(secondOctetIndexToChange==2))
                {
                    numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_40K_LINES_3_2_OCTET_MC_GRP_CHANGE_CNS;
                }
                else
                {
                    if ((firstOctetIndexToChange==0)&&(order==GT_TRUE))
                    {
                        numOfPrefixesExpected = PRV_TGF_EXPECTED_PREFIXES_TO_BE_ADDED_ALLOW_SHARING_WITH_40K_LINES_0_1_OCTET_MC_GRP_CHANGE_CNS;
                    }
                    else
                    {
                        rc = GT_NOT_SUPPORTED;
                        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
                    }
                }
                break;
            default:
                /* the test does not support at the moment any other configuration */
                rc = GT_NOT_SUPPORTED;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "the test does not support at the moment any other configuration: %d", prvTgfDevNum);
                break;
            }
        }
    }
    /*create a MC prefixes*/
    cpssOsMemSet(&mcRouteLttEntry, '\0', sizeof(PRV_TGF_IP_LTT_ENTRY_STC));
    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        mcRouteLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_E;
    }
    else
    {
        mcRouteLttEntry.routeType = PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E;
    }
    mcRouteLttEntry.numOfPaths = 0;
    mcRouteLttEntry.routeEntryBaseIndex = 0;
    mcRouteLttEntry.ucRPFCheckEnable = GT_FALSE;
    mcRouteLttEntry.sipSaCheckMismatchEnable = GT_FALSE;
    mcRouteLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;

    cpssOsMemSet(&ipGroup, '\0', sizeof(GT_IPADDR));
    cpssOsMemSet(&ipSrc, '\0', sizeof(GT_IPADDR));

    ipGroupPrefixLen = prefixLengthGrp;
    ipSrcPrefixLen = prefixLengthSrc;
    override = GT_FALSE;
    defragmentationEnable = GT_TRUE;

    ipGroup.arIP[0] = 224;
    ipGroup.arIP[1] = 6;

    /* a fixed ipSrc */
    ipSrc.arIP[0] = 0;
    ipSrc.arIP[1] = 0;
    ipSrc.arIP[2] = 0;
    ipSrc.arIP[3] = 0;

    for (i = 0; i < totalNumIpMc; i ++)
    {
        if(order == GT_FALSE)
        {
            if(i%256 == 0)
            {
                if(i!=0)
                    (ipGroup.arIP[secondOctetIndexToChange])++;

                ipGroup.arIP[firstOctetIndexToChange] = 0;
            }

            else
                (ipGroup.arIP[firstOctetIndexToChange])++;
        }
        else
        {
              ipSrc.u32Ip = i;
        }

        if (delete==GT_TRUE)
        {
            rc = prvTgfIpLpmIpv4McEntryDel(prvTgfLpmDBId, 0, ipGroup, ipGroupPrefixLen, ipSrc, ipSrcPrefixLen);
        }
        else
        {
            rc = prvTgfIpLpmIpv4McEntryAdd(prvTgfLpmDBId, 0, ipGroup, ipGroupPrefixLen, ipSrc, ipSrcPrefixLen,
                                           &mcRouteLttEntry,override,defragmentationEnable);
        }

        if ((rc == GT_OK) || (rc == GT_ALREADY_EXIST))
        {
             /*  remove remark in case debug is needed
            cpssOsPrintf("[S, G][%d] = [%d]:[%d]:[%d]:[%d],\n\r",
                         i,
                         ipGroup.arIP[0]
                         ,ipGroup.arIP[1]
                         ,ipGroup.arIP[2]
                         ,ipGroup.arIP[3]);
                         */
        }
        else
        {
             break;
        }
    }
    if (i!=numOfPrefixesExpected)
    {
        if (delete==GT_TRUE)
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "\n We did not succeed in deleting the amount wanted. deleted(%d/%d).\n",i,numOfPrefixesExpected);
        else
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "\n We did not succeed in adding the amount wanted. added(%d/%d).\n",i,numOfPrefixesExpected);
    }
    else
    {
        if (delete==GT_TRUE)
            cpssOsPrintf("\n We succeed in deleting the amount wanted. deleted(%d/%d).\n",i,numOfPrefixesExpected);
        else
            cpssOsPrintf("\n We succeed in adding the amount wanted. added(%d/%d).\n",i,numOfPrefixesExpected);
    }
    PRV_UTF_LOG0_MAC("==== HW Octet to Block Mapping");
    rc = prvTgfIpLpmDbgHwOctetPerBlockPrint(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerBlockPrint: %d", prvTgfDevNum);

    PRV_UTF_LOG0_MAC("==== HW Lpm Lines Counters usage ");
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters: %d", prvTgfDevNum);

    return;
}

/**
* @internal prvTgfIpLpmIpv6UcOrderAndDisorder function
* @endinternal
*
* @brief   Add some ipv6 Prefixes in order and disorder manner.
*         Verify that we get same number of prefixes as expected
* @param[in] totalNumIpUc             - number of ipv6 UC entries to add
* @param[in] randomEnable             - GT_TRUE: increment the octets in a disordered manner
*                                      GT_FALSE:increment the octets in a sequentially manner
* @param[in] defragEnable             - GT_TRUE: do defrag incase there is no more space
*                                      GT_FALSE:do NOT defrag incase there is no more space
*                                       None
*/
GT_VOID prvTgfIpLpmIpv6UcOrderAndDisorder
(
    GT_U32  totalNumIpUc,
    GT_BOOL randomEnable,
    GT_BOOL defragEnable
)
{
    GT_U32                                  *prefixesArrayPtr = NULL;
    GT_U32                                  ulLoop = 0;
    GT_U32                                  ulIndex = 0;
    GT_U32                                  temp = 0;
    GT_IPV6ADDR                             ipv6Addr;
    GT_U32                                  ipPrefixLen;
    GT_BOOL                                 override;
    GT_BOOL                                 defragmentationEnable;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    GT_STATUS                               rc = GT_OK;
    GT_U32                                  numberOfAddedPrefixes=0;
    GT_U32                                  minNumberOfPrefixesExpected;
    GT_U32                                  maxNumberOfPrefixesExpected;


    if (randomEnable==GT_FALSE && defragEnable==GT_FALSE)
    {
        PRV_UTF_LOG0_MAC("\n==== Adding prefixes sequentially WITHOUT defrag \n");
        if (totalNumIpUc<8701)
        {
            minNumberOfPrefixesExpected = totalNumIpUc;
            maxNumberOfPrefixesExpected = totalNumIpUc;
        }
        else
        {
            if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
            {
                minNumberOfPrefixesExpected = 9130;
                maxNumberOfPrefixesExpected = 9130;
            }
            else
            {
                minNumberOfPrefixesExpected = 8701;
                maxNumberOfPrefixesExpected = 8701;
            }

        }
    }
    else
    {
        if (randomEnable==GT_FALSE && defragEnable==GT_TRUE)
        {
            PRV_UTF_LOG0_MAC("\n==== Adding prefixes sequentially WITH defrag \n");
            if (totalNumIpUc<9126)
            {
                minNumberOfPrefixesExpected = totalNumIpUc;
                maxNumberOfPrefixesExpected = totalNumIpUc;
            }
            else
            {
                if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
                {
                    minNumberOfPrefixesExpected = 12311;
                    maxNumberOfPrefixesExpected = 12311;
                }
                else
                {
                    minNumberOfPrefixesExpected = 9126;
                    maxNumberOfPrefixesExpected = 9126;
                }
            }
        }
        else
        {
            if (randomEnable==GT_TRUE && defragEnable==GT_FALSE)
            {
                PRV_UTF_LOG0_MAC("\n==== Adding prefixes randomly WITHOUT defrag\n ");
                if (totalNumIpUc<3000)
                {
                    minNumberOfPrefixesExpected = totalNumIpUc;
                    maxNumberOfPrefixesExpected = totalNumIpUc;
                }
                else
                {
                    if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
                    {
                        minNumberOfPrefixesExpected = 1300;
                        maxNumberOfPrefixesExpected = 1700;
                    }
                    else
                    {
                        minNumberOfPrefixesExpected = 3000;
                        maxNumberOfPrefixesExpected = 6000;
                    }
                }
            }
            else
            {
                PRV_UTF_LOG0_MAC("\n==== Adding prefixes randomly WITH defrag \n ");
                if (totalNumIpUc<6000)
                {
                    minNumberOfPrefixesExpected = totalNumIpUc;
                    maxNumberOfPrefixesExpected = totalNumIpUc;
                }
                else
                {
                    if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
                    {
                        minNumberOfPrefixesExpected = 1500;
                        maxNumberOfPrefixesExpected = 2200;
                    }
                    else
                    {
                        minNumberOfPrefixesExpected = 6000;
                        maxNumberOfPrefixesExpected = 9126;
                    }
                }
            }
        }
    }
    prefixesArrayPtr = cpssOsMalloc(sizeof(GT_U32) * totalNumIpUc);

    /* xx::1 */
    for (ulLoop = 0; ulLoop < totalNumIpUc; ulLoop++)
    {
        prefixesArrayPtr[ulLoop] = ulLoop + 1;
    }

    PRV_UTF_LOG0_MAC("\n==== Flushing IPv4 and IPv6 Uc and Mc Prefixes \n");
    cpssDxChIpLpmIpv6UcPrefixesFlush(0,0);
    cpssDxChIpLpmIpv4UcPrefixesFlush(0,0);
    cpssDxChIpLpmIpv4McEntriesFlush(0,0);
    cpssDxChIpLpmIpv6McEntriesFlush(0,0);

    if(randomEnable == GT_TRUE)
    {
        cpssOsSrand(cpssOsTime());
        for (ulLoop = totalNumIpUc; ulLoop > 0; ulLoop--)
        {
            ulIndex = cpssOsRand()%totalNumIpUc;
            temp = prefixesArrayPtr[ulIndex];
            prefixesArrayPtr[ulIndex] = prefixesArrayPtr[ulLoop - 1];
            prefixesArrayPtr[ulLoop - 1] = temp;
        }
    }

    cpssOsMemSet(&nextHopInfo, '\0', sizeof(nextHopInfo));
    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_E;
    }
    else
    {
        nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E;
    }
    nextHopInfo.ipLttEntry.numOfPaths = 0;
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = 0;
    nextHopInfo.ipLttEntry.ucRPFCheckEnable = GT_FALSE;
    nextHopInfo.ipLttEntry.sipSaCheckMismatchEnable = GT_FALSE;
    nextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;

    cpssOsMemSet(&ipv6Addr, '\0', sizeof(GT_IPV6ADDR));
    ipv6Addr.arIP[1] = 0x3;
    ipPrefixLen = 128;
    override = GT_FALSE;
    defragmentationEnable = defragEnable;


    for (ulLoop = 0; ulLoop < totalNumIpUc; ulLoop++)
    {
        ipv6Addr.arIP[15] = (GT_U8)(prefixesArrayPtr[ulLoop]%256);
        ipv6Addr.arIP[14] = (GT_U8)(prefixesArrayPtr[ulLoop]/256);
        rc = prvTgfIpLpmIpv6UcPrefixAdd(0, 0, ipv6Addr,ipPrefixLen, &nextHopInfo,
                                       override, defragmentationEnable);

        if ((rc == GT_OK) || (rc == GT_ALREADY_EXIST))
        {
            if (rc == GT_OK)
            {
                numberOfAddedPrefixes++;
            }
        }
        else
        {
            break;
        }
    }
    if ((numberOfAddedPrefixes>=minNumberOfPrefixesExpected)&&(numberOfAddedPrefixes<=maxNumberOfPrefixesExpected))
    {
        cpssOsPrintf("\ncpssDxChIpLpmIpv6UcPrefixAdd OK numberOfAddedPrefixes= %d /minExpected=%d / maxExpected=%d / totalNumIpUc=%d\n",numberOfAddedPrefixes, minNumberOfPrefixesExpected, maxNumberOfPrefixesExpected, totalNumIpUc);
    }
    else
    {
        cpssOsPrintf("\ncpssDxChIpLpmIpv6UcPrefixAdd FAIL numberOfAddedPrefixes= %d /minExpected=%d / maxExpected=%d / totalNumIpUc=%d\n",numberOfAddedPrefixes, minNumberOfPrefixesExpected, maxNumberOfPrefixesExpected, totalNumIpUc);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_FAIL, "prvTgfIpLpmIpv6UcOrderAndDisorder: %d", prvTgfDevNum);
    }
    cpssOsFree(prefixesArrayPtr);

    return;
}

/**
* @internal prvTgfIpLpmIpv4UcOrderAndDisorder function
* @endinternal
*
* @brief   Add some ipv4 Prefixes in order and disorder manner.
*         Verify that we get same number of prefixes as expected
* @param[in] totalNumIpUc             - number of ipv4 UC entries to add
* @param[in] randomEnable             - GT_TRUE: increment the octets in a disordered manner
*                                      GT_FALSE:increment the octets in a sequentially manner
* @param[in] defragEnable             - GT_TRUE: do defrag incase there is no more space
*                                      GT_FALSE:do NOT defrag incase there is no more space
*                                       None
*/
GT_VOID prvTgfIpLpmIpv4UcOrderAndDisorder
(
    GT_U32  totalNumIpUc,
    GT_BOOL randomEnable,
    GT_BOOL defragEnable
)
{
    GT_U32                                  *prefixesArrayPtr = NULL;
    GT_U32                                  ulLoop = 0;
    GT_U32                                  ulIndex = 0;
    GT_U32                                  temp = 0;
    GT_IPADDR                               ipv4Addr;
    GT_U32                                  ipPrefixLen;
    GT_BOOL                                 override;
    GT_BOOL                                 defragmentationEnable;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    GT_STATUS                               rc = GT_OK;
    GT_U32                                  numberOfAddedPrefixes=0,i=0;
    GT_U8                                   j=0;
    GT_U32                                  minNumberOfPrefixesExpected;
    GT_U32                                  maxNumberOfPrefixesExpected;

    if (randomEnable==GT_FALSE && defragEnable==GT_FALSE)
    {
        PRV_UTF_LOG0_MAC("\n==== Adding prefixes sequentially WITHOUT defrag \n");
        if (totalNumIpUc<30205)
        {
            minNumberOfPrefixesExpected = totalNumIpUc;
            maxNumberOfPrefixesExpected = totalNumIpUc;
        }
        else
        {
            if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
            {
                minNumberOfPrefixesExpected = 257064;
                maxNumberOfPrefixesExpected = 257064;
            }
            else
            {
                minNumberOfPrefixesExpected = 30205;
                maxNumberOfPrefixesExpected = 30205;
            }
        }
    }
    else
    {
        if (randomEnable==GT_FALSE && defragEnable==GT_TRUE)
        {
            PRV_UTF_LOG0_MAC("\n==== Adding prefixes sequentially WITH defrag \n");
            if (totalNumIpUc<30630)
            {
                minNumberOfPrefixesExpected = totalNumIpUc;
                maxNumberOfPrefixesExpected = totalNumIpUc;
            }
            else
            {
                if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
                {
                    minNumberOfPrefixesExpected = 260010;
                    maxNumberOfPrefixesExpected = 260010;
                }
                else
                {
                    minNumberOfPrefixesExpected = 30630;
                    maxNumberOfPrefixesExpected = 30630;
                }
            }
        }
        else
        {
            if (randomEnable==GT_TRUE && defragEnable==GT_FALSE)
            {
                PRV_UTF_LOG0_MAC("\n==== Adding prefixes randomly WITHOUT defrag\n ");
                if (totalNumIpUc<15000)
                {
                    minNumberOfPrefixesExpected = totalNumIpUc;
                    maxNumberOfPrefixesExpected = totalNumIpUc;
                }
                else
                {
                    if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
                    {
                        minNumberOfPrefixesExpected = 241100;
                        maxNumberOfPrefixesExpected = 245500;
                    }
                    else
                    {
                        minNumberOfPrefixesExpected = 15000;
                        maxNumberOfPrefixesExpected = 20000;
                    }
                }
            }
            else
            {
                PRV_UTF_LOG0_MAC("\n==== Adding prefixes randomly WITH defrag \n ");
                if (totalNumIpUc<20000)
                {
                    minNumberOfPrefixesExpected = totalNumIpUc;
                    maxNumberOfPrefixesExpected = totalNumIpUc;
                }
                else
                {
                    if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
                    {
                        minNumberOfPrefixesExpected = 250000;
                        maxNumberOfPrefixesExpected = 251400;
                    }
                    else
                    {
                        minNumberOfPrefixesExpected = 20000;
                        maxNumberOfPrefixesExpected = 30630;
                    }
                }
            }
        }
    }
    prefixesArrayPtr = cpssOsMalloc(sizeof(GT_U32) * totalNumIpUc);

    /* xx::1 */
    for (ulLoop = 0; ulLoop < totalNumIpUc; ulLoop++)
    {
        prefixesArrayPtr[ulLoop] = ulLoop + 1;
    }

    PRV_UTF_LOG0_MAC("\n==== Flushing IPv4 and IPv6 Uc and Mc Prefixes \n");
    cpssDxChIpLpmIpv6UcPrefixesFlush(0,0);
    cpssDxChIpLpmIpv4UcPrefixesFlush(0,0);
    cpssDxChIpLpmIpv4McEntriesFlush(0,0);
    cpssDxChIpLpmIpv6McEntriesFlush(0,0);

    if(randomEnable == GT_TRUE)
    {
        cpssOsSrand(cpssOsTime());
        for (ulLoop = totalNumIpUc; ulLoop > 0; ulLoop--)
        {
            ulIndex = cpssOsRand()%totalNumIpUc;
            temp = prefixesArrayPtr[ulIndex];
            prefixesArrayPtr[ulIndex] = prefixesArrayPtr[ulLoop - 1];
            prefixesArrayPtr[ulLoop - 1] = temp;
        }
    }

    cpssOsMemSet(&nextHopInfo, '\0', sizeof(nextHopInfo));
    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_E;
    }
    else
    {
        nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E;
    }
    nextHopInfo.ipLttEntry.numOfPaths = 0;
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = 0;
    nextHopInfo.ipLttEntry.ucRPFCheckEnable = GT_FALSE;
    nextHopInfo.ipLttEntry.sipSaCheckMismatchEnable = GT_FALSE;
    nextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;

    cpssOsMemSet(&ipv4Addr, '\0', sizeof(GT_IPADDR));
    ipv4Addr.arIP[3] = 0x1;
    ipPrefixLen = 32;
    override = GT_FALSE;
    defragmentationEnable = defragEnable;

    /* disable LPM validity check */
    prvTgfIpValidityCheckEnable(GT_FALSE);

    for (ulLoop = 0; ulLoop < totalNumIpUc; ulLoop++)
    {
        ipv4Addr.arIP[3] = (GT_U8)(prefixesArrayPtr[ulLoop]%256);
        ipv4Addr.arIP[2] = (GT_U8)(prefixesArrayPtr[ulLoop]/256);
        rc = prvTgfIpLpmIpv4UcPrefixAdd(0, 0, ipv4Addr,ipPrefixLen, &nextHopInfo,
                                       override, defragmentationEnable);

        if ((rc == GT_OK) || (rc == GT_ALREADY_EXIST))
        {
            if (rc == GT_OK)
            {
                numberOfAddedPrefixes++;
            }
        }
        else
        {
            break;
        }
        if (numberOfAddedPrefixes%5000==0)
        {
            /* enable LPM validity check */
            prvTgfIpValidityCheckEnable(GT_TRUE);

            rc = prvTgfIpValidityCheck(prvTgfLpmDBId,0,CPSS_IP_PROTOCOL_IPV4_E,CPSS_UNICAST_E);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck: %d", prvTgfDevNum);

            /* disable LPM validity check */
            prvTgfIpValidityCheckEnable(GT_FALSE);
        }
    }
    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        /*sip6 have a much bigger memory - so try to add as many entries as possible
          by manipulation the values of the octets */
        if ((ulLoop==totalNumIpUc)&&
            ((rc == GT_OK) || (rc == GT_ALREADY_EXIST)))
        {
            for (j=0;j<3;j++)
            {
                ipv4Addr.arIP[0] = j+1;
                for (i = 1; i < 3; i++)
                {
                    if (!((rc == GT_OK) || (rc == GT_ALREADY_EXIST)))
                    {
                        break;
                    }
                    /* need to add more prefixes */
                    for (ulLoop = 0; ulLoop < totalNumIpUc; ulLoop++)
                    {
                        ipv4Addr.arIP[i+1] = (GT_U8)(prefixesArrayPtr[ulLoop]%256);
                        ipv4Addr.arIP[i] = (GT_U8)(prefixesArrayPtr[ulLoop]/256);
                        rc = prvTgfIpLpmIpv4UcPrefixAdd(0, 0, ipv4Addr,ipPrefixLen, &nextHopInfo,
                                                       override, defragmentationEnable);

                        if ((rc == GT_OK) || (rc == GT_ALREADY_EXIST))
                        {
                            if (rc == GT_OK)
                            {
                                numberOfAddedPrefixes++;
                            }
                        }
                        else
                        {
                            break;
                        }
                        if (numberOfAddedPrefixes%5000==0)
                        {
                            /* enable LPM validity check */
                            prvTgfIpValidityCheckEnable(GT_TRUE);

                            rc = prvTgfIpValidityCheck(prvTgfLpmDBId,0,CPSS_IP_PROTOCOL_IPV4_E,CPSS_UNICAST_E);
                            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck: %d", prvTgfDevNum);

                            /* disable LPM validity check */
                            prvTgfIpValidityCheckEnable(GT_FALSE);
                        }
                    }
                }
            }
        }
    }
    if ((numberOfAddedPrefixes>=minNumberOfPrefixesExpected)&&(numberOfAddedPrefixes<=maxNumberOfPrefixesExpected))
    {
        cpssOsPrintf("\ncpssDxChIpLpmIpv4UcPrefixAdd OK numberOfAddedPrefixes= %d /minExpected=%d / maxExpected=%d / totalNumIpUc=%d\n",numberOfAddedPrefixes, minNumberOfPrefixesExpected, maxNumberOfPrefixesExpected, totalNumIpUc);
    }
    else
    {
        cpssOsPrintf("\ncpssDxChIpLpmIpv4UcPrefixAdd FAIL numberOfAddedPrefixes= %d /minExpected=%d / maxExpected=%d / totalNumIpUc=%d\n",numberOfAddedPrefixes, minNumberOfPrefixesExpected, maxNumberOfPrefixesExpected, totalNumIpUc);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_FAIL, "prvTgfIpLpmIpv4UcOrderAndDisorder: %d", prvTgfDevNum);
    }
    cpssOsFree(prefixesArrayPtr);

    return;
}

/**
* @internal prvTgfIpLpmIpv4UcDisorder function
* @endinternal
*
* @brief   Add some ipv4 Prefixes in disorder manner.
*         Verify that we get same number of prefixes as expected
* @param[in] totalNumIpUc             - number of ipv4 UC entries to add
*                                      randomEnable      - GT_TRUE: increment the octets in a disordered manner
*                                      GT_FALSE:increment the octets in a sequentially manner
* @param[in] defragEnable             - GT_TRUE: do defrag incase there is no more space
*                                      GT_FALSE:do NOT defrag incase there is no more space
*                                       None
*/
GT_VOID prvTgfIpLpmIpv4UcDisorder
(
    GT_U32  totalNumIpUc,
    GT_BOOL defragEnable
)
{
    GT_STATUS                               rc=GT_OK;
    GT_IPADDR                               ipv4Addr;
    GT_BOOL                                 defragmentationEnable;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    GT_U32                                  numberOfAddedPrefixes=0;
    GT_U32                                  minNumberOfPrefixesExpected;
    GT_U32                                  maxNumberOfPrefixesExpected;

    if (defragEnable==GT_FALSE)
    {
        PRV_UTF_LOG0_MAC("\n==== Adding prefixes randomly WITHOUT defrag\n ");
        if (totalNumIpUc<3200)
        {
            minNumberOfPrefixesExpected = totalNumIpUc;
            maxNumberOfPrefixesExpected = totalNumIpUc;
        }
        else
        {
            if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
            {
                /* small memory configuration */
                minNumberOfPrefixesExpected = 3150;
                maxNumberOfPrefixesExpected = 3800;
            }
            else
            {
                minNumberOfPrefixesExpected = 3200;
                maxNumberOfPrefixesExpected = 3500;
            }
        }
    }
    else
    {
        PRV_UTF_LOG0_MAC("\n==== Adding prefixes randomly WITH defrag \n ");
        if (totalNumIpUc<3500)
        {
            minNumberOfPrefixesExpected = totalNumIpUc;
            maxNumberOfPrefixesExpected = totalNumIpUc;
        }
        else
        {
            if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
            {
                /* small memory configuration */
                minNumberOfPrefixesExpected = 2900;
                maxNumberOfPrefixesExpected = 3000;
            }
            else
            {
                minNumberOfPrefixesExpected = 3500;
                maxNumberOfPrefixesExpected = 3700;
            }
        }
    }

    PRV_UTF_LOG0_MAC("\n==== Flushing IPv4 and IPv6 Uc and Mc Prefixes \n");
    cpssDxChIpLpmIpv6UcPrefixesFlush(0,0);
    cpssDxChIpLpmIpv4UcPrefixesFlush(0,0);
    cpssDxChIpLpmIpv4McEntriesFlush(0,0);
    cpssDxChIpLpmIpv6McEntriesFlush(0,0);

    cpssOsMemSet(&nextHopInfo, '\0', sizeof(nextHopInfo));
    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_E;
    }
    else
    {
        nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E;
    }
    nextHopInfo.ipLttEntry.numOfPaths = 0;
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = 0;
    nextHopInfo.ipLttEntry.ucRPFCheckEnable = GT_FALSE;
    nextHopInfo.ipLttEntry.sipSaCheckMismatchEnable = GT_FALSE;
    nextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;

    cpssOsMemSet(&ipv4Addr, '\0', sizeof(GT_IPADDR));
    ipv4Addr.arIP[3] = 0x1;
    defragmentationEnable = defragEnable;

    rc = prvTgfIpLpmDbgIpv4UcPrefixAddManyRandom(prvTgfLpmDBId,
                                               0,
                                               &ipv4Addr,
                                               8,
                                               totalNumIpUc,
                                               GT_TRUE,
                                               defragmentationEnable,
                                               &numberOfAddedPrefixes);

    if ((numberOfAddedPrefixes>=minNumberOfPrefixesExpected)&&(numberOfAddedPrefixes<=maxNumberOfPrefixesExpected))
    {
        cpssOsPrintf("\n cpssDxChIpLpmDbgIpv4UcPrefixAddManyRandom OK numberOfAddedPrefixes= %d /minExpected=%d / maxExpected=%d / totalNumIpUc=%d\n",numberOfAddedPrefixes, minNumberOfPrefixesExpected, maxNumberOfPrefixesExpected, totalNumIpUc);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcOrderAndDisorder: %d", prvTgfDevNum);
    }
    else
    {
        cpssOsPrintf("\n cpssDxChIpLpmDbgIpv4UcPrefixAddManyRandom FAIL numberOfAddedPrefixes= %d /minExpected=%d / maxExpected=%d / totalNumIpUc=%d\n",numberOfAddedPrefixes, minNumberOfPrefixesExpected, maxNumberOfPrefixesExpected, totalNumIpUc);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_FAIL, "prvTgfIpLpmIpv4UcOrderAndDisorder: %d", prvTgfDevNum);
    }
    return;
}


/**
* @internal prvTgfIpLpmIpv6UcAddSpecificPrefix function
* @endinternal
*
* @brief   Add specific ipv6 Uc entry with prefix length = 72
*
* @param[in] defragEnable             - GT_TRUE: do defrag incase there is no more space
*                                      GT_FALSE:do NOT defrag incase there is no more space
*                                       None
*/
GT_VOID prvTgfIpLpmIpv6UcAddSpecificPrefix
(
    GT_BOOL defragEnable
)
{
    GT_IPV6ADDR                             ipv6Addr;
    GT_U32                                  ipPrefixLen;
    GT_BOOL                                 override;
    GT_BOOL                                 defragmentationEnable;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    GT_STATUS                               rc = GT_OK;
    GT_U8                                   i=0;

    cpssOsMemSet(&nextHopInfo, '\0', sizeof(nextHopInfo));
    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_E;
    }
    else
    {
        nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E;
    }
    nextHopInfo.ipLttEntry.numOfPaths = 0;
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = 0;
    nextHopInfo.ipLttEntry.ucRPFCheckEnable = GT_FALSE;
    nextHopInfo.ipLttEntry.sipSaCheckMismatchEnable = GT_FALSE;
    nextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;

    cpssOsMemSet(&ipv6Addr, '\0', sizeof(GT_IPV6ADDR));
    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        /* we have a much bigger memory, so need to use full match to use more blocks */
        ipPrefixLen = 128;
    }
    else
    {
        ipPrefixLen = 72;
    }
    override = GT_FALSE;
    defragmentationEnable = defragEnable;
    ipv6Addr.arIP[0] = 0x22;
    ipv6Addr.arIP[1] = 0x22;
    ipv6Addr.arIP[2] = 0x22;
    ipv6Addr.arIP[3] = 0x22;
    ipv6Addr.arIP[4] = 0x22;
    ipv6Addr.arIP[5] = 0x22;
    ipv6Addr.arIP[6] = 0x22;
    ipv6Addr.arIP[7] = 0x22;
    ipv6Addr.arIP[8] = 0x22;
    ipv6Addr.arIP[9] = 0x22;
    ipv6Addr.arIP[10] = 0x22;
    ipv6Addr.arIP[11] = 0x22;
    ipv6Addr.arIP[12] = 0x22;
    ipv6Addr.arIP[13] = 0x22;
    ipv6Addr.arIP[14] = 0x22;
    ipv6Addr.arIP[15] = 0x22;

    rc = prvTgfIpLpmIpv6UcPrefixAdd(0, 0, ipv6Addr,ipPrefixLen, &nextHopInfo,
                                    override, defragmentationEnable);

    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        /* for sip6 we have a bigger memory, this lead to many free lines that can fit an ipv6 entry
           inorder to create a situation we get out of PP we need to add ipv6 entries that will need
           a block bigger then 2 lines*/
        for (i=0;i<4;i++)
        {
            ipv6Addr.arIP[2] = 0x22+(i+1)*0x11;
            rc = prvTgfIpLpmIpv6UcPrefixAdd(0, 0, ipv6Addr,ipPrefixLen, &nextHopInfo,
                                    override, defragmentationEnable);
            if (rc!=GT_OK)
            {
                if (rc == GT_ALREADY_EXIST)
                {
                    /* second time we call prvTgfIpLpmIpv6UcAddSpecificPrefix with defrag=true
                       only the forth prefix was not added (i==3) when we called
                       prvTgfIpLpmIpv6UcAddSpecificPrefix with defrag=false */
                    continue;
                }
                else
                {
                    break;
                }
            }
        }
    }
    if (defragmentationEnable==GT_FALSE)
    {
        if (rc==GT_OUT_OF_PP_MEM)
        {
            /* this is the expected value */
            cpssOsPrintf("\n prvTgfIpLpmIpv6UcAddSpecificPrefix return GT_OUT_OF_PP_MEM when defragmentationEnable=%d as expected\n\n",defragmentationEnable);
            rc = GT_OK;
        }
        else
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OUT_OF_PP_MEM, rc, "prvTgfIpLpmIpv6UcAddSpecificPrefix,defragmentationEnable: %d", defragmentationEnable);
        }
    }
    else
    {
        if (rc!=GT_OK)
        {
            if (rc==GT_OUT_OF_PP_MEM)
            {
                /* this is the expected value */
                cpssOsPrintf("\n prvTgfIpLpmIpv6UcAddSpecificPrefix return GT_OUT_OF_PP_MEM when defragmentationEnable=%d as expected\n\n",defragmentationEnable);
                rc = GT_OK;
            }
            else
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6UcAddSpecificPrefix,defragmentationEnable: %d", defragmentationEnable);
            }
        }
        else
        {
            cpssOsPrintf("\n prvTgfIpLpmIpv6UcAddSpecificPrefix return GT_OK when defragmentationEnable=%d\n\n",defragmentationEnable);
        }
    }
    return;
}

/**
* @internal prvTgfIpLpmIpv4UcOrderAndPredefinedOneLoop function
* @endinternal
*
* @brief   Add some Predefined and sequentially ipv4 Prefixes in order manner.
*         Verify that we get same number of prefixes as expected
* @param[in] ipv4Addr                 - ip address to start the loop from
* @param[in] ipPrefixLen              - prefix length
* @param[in] nextHopInfoPtr           - (pointer to) the next hop information
* @param[in] startLoop                - beginning of loop index
* @param[in] endLoop                  - end of loop index
* @param[in] numOfExpectedPrefixes    - number of prefixes expected to be added/deleted
* @param[in] firstOctetToChange       - first octet we want to change in the ip address
* @param[in] secondOctetToChange      - second octet we want to change in the ip address
* @param[in] addFlag                  - GT_TRUE: add prefixes
*                                      GT_FALSE: delete prefixes
* @param[in] defragEnable             - GT_TRUE: use defrag mechanism when adding the prefix
*                                      GT_FALSE: do not use defrag mechanism when adding the prefix
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail
*/
GT_STATUS prvTgfIpLpmIpv4UcOrderAndPredefinedOneLoop
(
    GT_IPADDR                               ipv4Addr,
    GT_U32                                  ipPrefixLen,
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    *nextHopInfoPtr,
    GT_U32                                  startLoop,
    GT_U32                                  endLoop,
    GT_U32                                  numOfExpectedPrefixes,
    GT_U32                                  firstOctetToChange,
    GT_U32                                  secondOctetToChange,
    GT_BOOL                                 addFlag,
    GT_BOOL                                 defragEnable
)
{
    GT_U32                                  *prefixesArrayPtr = NULL;
    GT_U32                                  ulLoop = 0;
    GT_BOOL                                 override;
    GT_STATUS                               rc = GT_OK;
    GT_U32                                  numberOfAddedPrefixes=0;

    prefixesArrayPtr = cpssOsMalloc(sizeof(GT_U32) * endLoop);

    for (ulLoop = 0; ulLoop < endLoop; ulLoop++)
    {
        prefixesArrayPtr[ulLoop] = ulLoop + 1;
    }

    override = GT_FALSE;

    /* add the prefixes */
    for (ulLoop = startLoop; ulLoop < endLoop; ulLoop++)
    {
        ipv4Addr.arIP[firstOctetToChange] = (GT_U8)(prefixesArrayPtr[ulLoop]%256);
        ipv4Addr.arIP[secondOctetToChange] = (GT_U8)(prefixesArrayPtr[ulLoop]/256);
        if (addFlag==GT_TRUE)
        {
            rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, 0, ipv4Addr, ipPrefixLen, nextHopInfoPtr, override, defragEnable);
        }
        else
        {
            rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId, 0, ipv4Addr, ipPrefixLen);
        }

        if ((rc == GT_OK) || (rc == GT_ALREADY_EXIST))
        {
            if (rc == GT_OK)
            {
                numberOfAddedPrefixes++;
            }
        }
        else
        {
            break;
        }
    }
    if (numberOfAddedPrefixes>=numOfExpectedPrefixes)
    {
        if (addFlag==GT_TRUE)
        {
            cpssOsPrintf("[last ipv4 unicast added] = [%d]:[%d]:[%d]:[%d] numberOfAddedPrefixes=%d,\n\r",
                     ipv4Addr.arIP[0],ipv4Addr.arIP[1],ipv4Addr.arIP[2],ipv4Addr.arIP[3],numberOfAddedPrefixes);
        }
        else
        {
            cpssOsPrintf("[last ipv4 unicast deleted] = [%d]:[%d]:[%d]:[%d] numberOfDeletedPrefixes=%d,\n\r",
                     ipv4Addr.arIP[0],ipv4Addr.arIP[1],ipv4Addr.arIP[2],ipv4Addr.arIP[3],numberOfAddedPrefixes);
        }
    }
    else
    {
        if (addFlag==GT_TRUE)
        {
            cpssOsPrintf("\ncpssDxChIpLpmIpv4UcPrefixAdd FAIL numberOfAddedPrefixes= %d \n",numberOfAddedPrefixes);
        }
        else
        {
            cpssOsPrintf("\ncpssDxChIpLpmIpv4UcPrefixDel FAIL numberOfDeletedPrefixes= %d \n",numberOfAddedPrefixes);
        }
        return GT_FAIL;
    }

    /* print the block mapping state */
    cpssOsPrintf("==== HW Lpm Lines Counters usage after last ipv4Addr=%d:%d:%d:%d \n",
                 ipv4Addr.arIP[0],ipv4Addr.arIP[1],ipv4Addr.arIP[2],ipv4Addr.arIP[3]);
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters(prvTgfLpmDBId);

    cpssOsFree(prefixesArrayPtr);

    return rc;
}

/**
* @internal prvTgfIpLpmIpv4UcCheckMergeBanks function
* @endinternal
*
* @brief   Add some Predefined and sequentially ipv4 Prefixes in order manner.
*         Verify that we get same number of prefixes as expected
*         test the Merge bank mechanism
* @param[in] totalNumIpUc             - number of ipv4 UC entries to add
*                                       None
*/
GT_VOID prvTgfIpLpmIpv4UcCheckMergeBanks
(
    GT_U32  totalNumIpUc
)
{
    GT_IPADDR                               ipv4Addr;
    GT_IPV6ADDR                             ipv6Addr;
    GT_U8                                   i=0;
    GT_U32                                  ipPrefixLen=32;
    GT_U32                                  ipv6PrefixLen=128;
    GT_BOOL                                 override;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    GT_STATUS                               rc = GT_OK;

    GT_U32                                  startLoop;
    GT_U32                                  endLoop;
    GT_U32                                  firstOctetToChange;
    GT_U32                                  secondOctetToChange;
    GT_BOOL                                 addFlag;
    GT_BOOL                                 defragEnable;
    GT_U32                                  numOfExpectedPrefixes;

    /* IPV4 */

    PRV_UTF_LOG0_MAC("\n==== Adding prefixes sequentially WITHOUT defrag \n");

    PRV_UTF_LOG0_MAC("\n==== Flushing IPv4 and IPv6 Uc and Mc Prefixes \n");
    cpssDxChIpLpmIpv6UcPrefixesFlush(0,0);
    cpssDxChIpLpmIpv4UcPrefixesFlush(0,0);
    cpssDxChIpLpmIpv4McEntriesFlush(0,0);
    cpssDxChIpLpmIpv6McEntriesFlush(0,0);

    cpssOsMemSet(&nextHopInfo, '\0', sizeof(nextHopInfo));
    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_E;
    }
    else
    {
        nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E;
    }
    nextHopInfo.ipLttEntry.numOfPaths = 0;
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = 0;
    nextHopInfo.ipLttEntry.ucRPFCheckEnable = GT_FALSE;
    nextHopInfo.ipLttEntry.sipSaCheckMismatchEnable = GT_FALSE;
    nextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;

    cpssOsMemSet(&ipv6Addr, '\0', sizeof(GT_IPV6ADDR));
    ipv6Addr.arIP[0]=0x22;
    ipv6Addr.arIP[1]=0x22;
    ipv6Addr.arIP[14]=0x44;
    ipv6Addr.arIP[15]=0x44;

    override = GT_FALSE;
    defragEnable = GT_FALSE;

    rc = prvTgfIpLpmIpv6UcPrefixAdd(prvTgfLpmDBId, 0, ipv6Addr,ipv6PrefixLen, &nextHopInfo, override, defragEnable);
    if (rc == GT_OK)
    {
        cpssOsPrintf("[ipv6 unicast added] = [%d]:[%d]:[%d]:[%d]:"
                     "[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:"
                     "[%d]:[%d]:[%d]:[%d]/%d,\n\r",
                 ipv6Addr.arIP[0],
                 ipv6Addr.arIP[1],
                 ipv6Addr.arIP[2],
                 ipv6Addr.arIP[3],
                 ipv6Addr.arIP[4],
                 ipv6Addr.arIP[5],
                 ipv6Addr.arIP[6],
                 ipv6Addr.arIP[7],
                 ipv6Addr.arIP[8],
                 ipv6Addr.arIP[9],
                 ipv6Addr.arIP[10],
                 ipv6Addr.arIP[11],
                 ipv6Addr.arIP[12],
                 ipv6Addr.arIP[13],
                 ipv6Addr.arIP[14],
                 ipv6Addr.arIP[15],
                 ipv6PrefixLen);
    }
    else
    {
        cpssOsPrintf("\n prvTgfIpLpmIpv6UcPrefixAdd FAIL \n");
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_FAIL, "prvTgfIpLpmIpv4UcCheckMergeBanks: %d", prvTgfDevNum);
    }

    cpssOsMemSet(&ipv4Addr, '\0', sizeof(GT_IPADDR));
    ipv4Addr.arIP[0] = 3;
    ipv4Addr.arIP[1] = 3;
    ipv4Addr.arIP[2] = 3;
    ipv4Addr.arIP[3] = 3;
    ipPrefixLen = 32;

    /* add the first prefix 3.3.3.3 */
    rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, 0, ipv4Addr,ipPrefixLen, &nextHopInfo, override, defragEnable);
    if (rc == GT_OK)
    {
        cpssOsPrintf("[ipv4 unicast added] = [%d]:[%d]:[%d]:[%d]/%d,\n\r",
                 ipv4Addr.arIP[0],
                 ipv4Addr.arIP[1],
                 ipv4Addr.arIP[2],
                 ipv4Addr.arIP[3],
                 ipPrefixLen);
    }
    else
    {
        cpssOsPrintf("\n prvTgfIpLpmIpv4UcPrefixAdd FAIL 3.3.3.3 was not added\n");
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_FAIL, "prvTgfIpLpmIpv4UcCheckMergeBanks: %d", prvTgfDevNum);
    }

    /* print the block mapping state */
    cpssOsPrintf("==== HW Lpm Lines Counters usage - 3.3.3.3 ADDED  ");
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters: %d", prvTgfDevNum);

    /* we want to add prefixes for all banks in octet 3 in such
       a way that we will be able to merge between the banks
       we start from bank 4 to 18, each loop i add  prefixes to bank i and i+1
       bank 0 is mapped to octet 0
       bank 1 is mapped to octet 1
       bank 2 is mapped to octet 2
       bank 3 is mapped to octet 3*/
    for (i=4;i<19;i++)
    {
        ipv4Addr.arIP[0] = 0;
        ipv4Addr.arIP[1] = 0;
        ipv4Addr.arIP[2] = 0;
        ipv4Addr.arIP[3] = 1;
        ipPrefixLen = 32;
        startLoop=(i-4)*1500;
        endLoop=startLoop+1500;
        numOfExpectedPrefixes=1500;
        firstOctetToChange=3;
        secondOctetToChange=2;
        addFlag=GT_TRUE;
        defragEnable=GT_FALSE;
         /* add 0.0.0.1 - 0.0.x.y*/
        rc = prvTgfIpLpmIpv4UcOrderAndPredefinedOneLoop(ipv4Addr, ipPrefixLen, &nextHopInfo,
                                                        startLoop,endLoop,numOfExpectedPrefixes,
                                                        firstOctetToChange,secondOctetToChange,
                                                        addFlag,defragEnable);

        if (rc==GT_OK)
        {
            cpssOsPrintf("\n prvTgfIpLpmIpv4UcCheckMergeBanks loop_%d OK\n",i);
        }
        else
        {
            cpssOsPrintf("\n prvTgfIpLpmIpv4UcCheckMergeBanks FAIL on loop_%d \n",i);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcCheckMergeBanks: %d", prvTgfDevNum);
        }


        ipv4Addr.arIP[0] = i;
        ipv4Addr.arIP[1] = i;
        ipv4Addr.arIP[2] = i;
        ipv4Addr.arIP[3] = i;
        ipPrefixLen = 32;
        startLoop=0;
        endLoop=460;
        numOfExpectedPrefixes=460;
        /* add 4.4.4.4 - 4.4.x.y or 5.5.5.5 -5.5.x.y or i.i.i.i - i.i.x.y*/
           rc = prvTgfIpLpmIpv4UcOrderAndPredefinedOneLoop(ipv4Addr, ipPrefixLen, &nextHopInfo,
                                                        startLoop,endLoop,numOfExpectedPrefixes,
                                                        firstOctetToChange,secondOctetToChange,
                                                        addFlag,defragEnable);
        if (rc==GT_OK)
        {
            cpssOsPrintf("\n prvTgfIpLpmIpv4UcCheckMergeBanks second loop_%d OK\n",i);
        }
        else
        {
            cpssOsPrintf("\n prvTgfIpLpmIpv4UcCheckMergeBanks FAIL on second loop_%d\n",i);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcCheckMergeBanks: %d", prvTgfDevNum);
        }

    }

    ipv4Addr.arIP[0] = 0;
    ipv4Addr.arIP[1] = 0;
    ipv4Addr.arIP[2] = 0;
    ipv4Addr.arIP[3] = 1;
    ipPrefixLen = 32;
    startLoop=(i-4)*1500;
    endLoop=totalNumIpUc;
    numOfExpectedPrefixes=466;
    defragEnable=GT_TRUE;
    /* add 0.0.0.1 - 0.0.x.y*/
    rc = prvTgfIpLpmIpv4UcOrderAndPredefinedOneLoop(ipv4Addr, ipPrefixLen, &nextHopInfo,
                                                    startLoop,endLoop,numOfExpectedPrefixes,
                                                    firstOctetToChange,secondOctetToChange,
                                                    addFlag,defragEnable);
    if (rc==GT_OK)
    {
        cpssOsPrintf("\n prvTgfIpLpmIpv4UcCheckMergeBanks endLoop=totalNumIpUc OK\n");
    }
    else
    {
        cpssOsPrintf("\n prvTgfIpLpmIpv4UcCheckMergeBanks FAIL on endLoop=totalNumIpUc \n");
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcCheckMergeBanks: %d", prvTgfDevNum);
    }

    /* free some of the prefixes */
    ipv4Addr.arIP[0] = 0;
    ipv4Addr.arIP[1] = 0;
    ipv4Addr.arIP[2] = 0;
    ipv4Addr.arIP[3] = 1;
    ipPrefixLen = 32;
    startLoop=0;
    endLoop=(i-4)*1500+466;
    numOfExpectedPrefixes=(i-4)*1500+466;
    addFlag = GT_FALSE;
    /* add 0.0.0.1 - 0.0.x.y*/
    rc = prvTgfIpLpmIpv4UcOrderAndPredefinedOneLoop(ipv4Addr, ipPrefixLen, &nextHopInfo,
                                                    startLoop,endLoop,numOfExpectedPrefixes,
                                                    firstOctetToChange,secondOctetToChange,
                                                    addFlag,defragEnable);
    if (rc==GT_OK)
    {
        cpssOsPrintf("\n prvTgfIpLpmIpv4UcCheckMergeBanks endLoop=(i-4)*1500+500 OK\n");
    }
    else
    {
        cpssOsPrintf("\n prvTgfIpLpmIpv4UcCheckMergeBanks FAIL on endLoop=(i-4)*1500+500 \n");
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcCheckMergeBanks: %d", prvTgfDevNum);
    }

    /* try to add prefixes with prefixLen/16
       if defrag=false we should get a fail
       if defrag=true we should have shrink+merge and succeed */
    ipv4Addr.arIP[0] = 20;
    ipv4Addr.arIP[1] = 20;
    ipv4Addr.arIP[2] = 12;
    ipv4Addr.arIP[3] = 13;
    ipPrefixLen = 16;
    startLoop = 0;
    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        endLoop=12000;
        numOfExpectedPrefixes=11016;
    }
    else
    {
        endLoop=3000;
        numOfExpectedPrefixes=1533;
    }

    firstOctetToChange = 1;
    secondOctetToChange = 0;
    addFlag = GT_TRUE;
    defragEnable=GT_FALSE;
    /* add 20.20.12.13 - x.y/16*/
    rc = prvTgfIpLpmIpv4UcOrderAndPredefinedOneLoop(ipv4Addr, ipPrefixLen, &nextHopInfo,
                                                    startLoop,endLoop,numOfExpectedPrefixes,
                                                    firstOctetToChange,secondOctetToChange
                                                    ,addFlag,defragEnable);
    if (rc==GT_OK)
    {
        cpssOsPrintf("\n prvTgfIpLpmIpv4UcCheckMergeBanks prefixLen/16 defrag=false OK\n");
    }
    else
    {
        cpssOsPrintf("\n prvTgfIpLpmIpv4UcCheckMergeBanks FAIL on prefixLen/16 defrag=false  \n");
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_FAIL, "prvTgfIpLpmIpv4UcCheckMergeBanks: %d", prvTgfDevNum);
    }

    cpssDxChIpLpmDbgHwOctetPerProtocolPrint(0);
    /* print the block mapping state */
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters: %d", prvTgfDevNum);


     /* after we get a fail that there is no more space in memory
       we try to add some more prefixes with defragEnable=GT_TRUE
       we can see that now we can add mutch more */
    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        startLoop=11016;
        endLoop=40000;
        numOfExpectedPrefixes=2978;
    }
    else
    {
        startLoop=1533;
        endLoop=40000;
        numOfExpectedPrefixes=21926;
    }
    addFlag = GT_TRUE;
    defragEnable=GT_TRUE;

   /* if we only use simple defrag we can add 381 -
       this was checked manualy by skipping the merge part in the code
       if we use defrag + merge we can add 21926 meaning the merge
       added 21545 more than simple shrink */

    /* add 7.7.12.13 - x.y/16*/
    rc = prvTgfIpLpmIpv4UcOrderAndPredefinedOneLoop(ipv4Addr, ipPrefixLen, &nextHopInfo,
                                                    startLoop,endLoop,numOfExpectedPrefixes,
                                                    firstOctetToChange,secondOctetToChange,
                                                    addFlag,defragEnable);
    if (rc==GT_OK)
    {
        cpssOsPrintf("\n prvTgfIpLpmIpv4UcCheckMergeBanks prefixLen/16 defrag=true OK\n");
    }
    else
    {
        cpssOsPrintf("\n prvTgfIpLpmIpv4UcCheckMergeBanks FAIL prefixLen/16 defrag=true \n");
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcCheckMergeBanks: %d", prvTgfDevNum);
    }
    cpssDxChIpLpmDbgHwOctetPerProtocolPrint(0);
    /* print the block mapping state */
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters: %d", prvTgfDevNum);
    return;
}
/**
* @internal prvTgfIpLpmIpv4UcOrderAndPredefined function
* @endinternal
*
* @brief   Add some Predefined and sequentially ipv4 Prefixes in order manner.
*         Verify that we get same number of prefixes as expected
* @param[in] totalNumIpUc             - number of ipv4 UC entries to add
* @param[in] flushFlag                - GT_TRUE: flush all entries before the test
*                                      GT_FALSE: do not flush all entries before the test
*                                       None
*/
GT_VOID prvTgfIpLpmIpv4UcOrderAndPredefined
(
    GT_U32  totalNumIpUc,
    GT_BOOL flushFlag
)
{
    GT_U32                                  *prefixesArrayPtr = NULL;
    GT_U32                                  ulLoop = 0;
    GT_IPADDR                               ipv4Addr;
    GT_U32                                  ipPrefixLen;
    GT_BOOL                                 override;
    GT_BOOL                                 defragmentationEnable;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    GT_STATUS                               rc = GT_OK;
    GT_U32                                  numberOfAddedPrefixes=0;
    GT_U32                                  numberOfPrefixesExpected;

    GT_IPADDR                               unIpv4AddrArray[4];
    GT_U32                                  ipv4PrefixLenArray[4];

    GT_IPADDR                               lastIpAddrAdded;
    GT_U32                                  numOfPrefixesAdded;

    /* IPV4 */

    cpssOsMemSet(&unIpv4AddrArray, '\0', sizeof(unIpv4AddrArray));
    /* first prefix to be added */
    unIpv4AddrArray[0].arIP[0] = 1;
    unIpv4AddrArray[0].arIP[1] = 1;
    unIpv4AddrArray[0].arIP[2] = 1;
    unIpv4AddrArray[0].arIP[3] = 3;
    ipv4PrefixLenArray[0]=32;

    /* second prefix to be added after first bank is full
       and before we mapped a new bank to the octet */
    unIpv4AddrArray[1].arIP[0] = 4;
    unIpv4AddrArray[1].arIP[1] = 4;
    unIpv4AddrArray[1].arIP[2] = 4;
    unIpv4AddrArray[1].arIP[3] = 4;
    ipv4PrefixLenArray[1]=32;

    /* third prefix to be added after second bank is full
       and before we mapped a new bank to the octet */
    unIpv4AddrArray[2].arIP[0] = 5;
    unIpv4AddrArray[2].arIP[1] = 5;
    unIpv4AddrArray[2].arIP[2] = 5;
    unIpv4AddrArray[2].arIP[3] = 5;
    ipv4PrefixLenArray[2]=32;


    PRV_UTF_LOG0_MAC("\n==== Adding prefixes sequentially WITHOUT defrag \n");
    if (totalNumIpUc<28925)
    {
        numberOfPrefixesExpected = totalNumIpUc;
    }
    else
    {
         if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
         {
             numberOfPrefixesExpected = 41942;
         }
         else
         {
             numberOfPrefixesExpected = 28925;
         }
    }

    prefixesArrayPtr = cpssOsMalloc(sizeof(GT_U32) * totalNumIpUc);

    /* xx::1 */
    for (ulLoop = 0; ulLoop < totalNumIpUc; ulLoop++)
    {
        prefixesArrayPtr[ulLoop] = ulLoop + 1;
    }

    if (flushFlag==GT_TRUE)
    {
        PRV_UTF_LOG0_MAC("\n==== Flushing IPv4 and IPv6 Uc and Mc Prefixes \n");
        cpssDxChIpLpmIpv6UcPrefixesFlush(0,0);
        cpssDxChIpLpmIpv4UcPrefixesFlush(0,0);
        cpssDxChIpLpmIpv4McEntriesFlush(0,0);
        cpssDxChIpLpmIpv6McEntriesFlush(0,0);
    }

    cpssOsMemSet(&nextHopInfo, '\0', sizeof(nextHopInfo));
    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_E;
    }
    else
    {
        nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E;
    }
    nextHopInfo.ipLttEntry.numOfPaths = 0;
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = 0;
    nextHopInfo.ipLttEntry.ucRPFCheckEnable = GT_FALSE;
    nextHopInfo.ipLttEntry.sipSaCheckMismatchEnable = GT_FALSE;
    nextHopInfo.ipLttEntry.ipv6MCGroupScopeLevel = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;

    cpssOsMemSet(&ipv4Addr, '\0', sizeof(GT_IPADDR));
    ipv4Addr.arIP[3] = 0x1;
    ipPrefixLen = 32;
    override = GT_FALSE;
    defragmentationEnable = GT_FALSE;

    /* add the first prefix 1.1.1.3 */
    rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, 0, unIpv4AddrArray[0],ipv4PrefixLenArray[0], &nextHopInfo, override,defragmentationEnable);
    if ((rc == GT_OK)||(rc==GT_ALREADY_EXIST))
    {
        cpssOsPrintf("[ipv4 unicast added] = [%d]:[%d]:[%d]:[%d]/%d,\n\r",
                  unIpv4AddrArray[0].arIP[0]
                 ,unIpv4AddrArray[0].arIP[1]
                 ,unIpv4AddrArray[0].arIP[2]
                 ,unIpv4AddrArray[0].arIP[3]
                 ,ipv4PrefixLenArray[0]);

    }
    else
    {
        cpssOsPrintf("\n prvTgfIpLpmIpv4UcPrefixAdd FAIL 1.1.1.3 was not added\n");
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_FAIL, "prvTgfIpLpmIpv4UcOrderAndPredefined: %d", prvTgfDevNum);
    }


    /* print the block mapping state */
    cpssOsPrintf("==== HW Lpm Lines Counters usage - 1.1.1.3 ADDED  ");
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters: %d", prvTgfDevNum);

    for (ulLoop = 0; ulLoop < 1532; ulLoop++)
    {
        ipv4Addr.arIP[3] = (GT_U8)(prefixesArrayPtr[ulLoop]%256);
        ipv4Addr.arIP[2] = (GT_U8)(prefixesArrayPtr[ulLoop]/256);
        rc = prvTgfIpLpmIpv4UcPrefixAdd(0, 0, ipv4Addr,ipPrefixLen, &nextHopInfo,
                                       override, defragmentationEnable);

        if ((rc == GT_OK) || (rc == GT_ALREADY_EXIST))
        {
            if (rc == GT_OK)
            {
                numberOfAddedPrefixes++;
            }
        }
        else
        {
            break;
        }
    }
    if (numberOfAddedPrefixes==1532)
    {
        cpssOsPrintf("\ncpssDxChIpLpmIpv4UcPrefixAdd OK numberOfAddedPrefixes= %d \n",numberOfAddedPrefixes);
    }
    else
    {
        cpssOsPrintf("\ncpssDxChIpLpmIpv4UcPrefixAdd FAIL numberOfAddedPrefixes= %d \n",numberOfAddedPrefixes);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_FAIL, "prvTgfIpLpmIpv4UcOrderAndPredefined: %d", prvTgfDevNum);
    }


    /* print the block mapping state */
    cpssOsPrintf("==== HW Lpm Lines Counters usage - 4.4.4.4 BEFORE ADDED  ");
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters: %d", prvTgfDevNum);


    /* add the second prefix in the list and some more 4.4.4.4 - 4.4.x.y*/
    rc = prvTgfIpLpmIpv4UcPrefixAddManyByOctet(prvTgfLpmDBId, 0, &unIpv4AddrArray[1],55,500, 3, 2, 1, 0, &lastIpAddrAdded,&numOfPrefixesAdded);
    if (rc == GT_OK)
    {
        cpssOsPrintf("[last ipv4 unicast added] = [%d]:[%d]:[%d]:[%d] numOfPrefixesAdded=%d,\n\r",
                  lastIpAddrAdded.arIP[0]
                 ,lastIpAddrAdded.arIP[1]
                 ,lastIpAddrAdded.arIP[2]
                 ,lastIpAddrAdded.arIP[3]
                 ,numOfPrefixesAdded);

    }
    else
    {
        cpssOsPrintf("\n prvTgfIpLpmIpv4UcPrefixAddManyByOctet FAIL numberOfAddedPrefixes= %d \n",numberOfAddedPrefixes);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_FAIL, "prvTgfIpLpmIpv4UcOrderAndPredefined: %d", prvTgfDevNum);
    }

     /* print the block mapping state */
    cpssOsPrintf("==== HW Lpm Lines Counters usage - 4.4.4.4 AFTER ADDED  ");
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters: %d", prvTgfDevNum);

    for (ulLoop = 1532; ulLoop < 3324; ulLoop++)
    {
        ipv4Addr.arIP[3] = (GT_U8)(prefixesArrayPtr[ulLoop]%256);
        ipv4Addr.arIP[2] = (GT_U8)(prefixesArrayPtr[ulLoop]/256);
        rc = prvTgfIpLpmIpv4UcPrefixAdd(0, 0, ipv4Addr,ipPrefixLen, &nextHopInfo,
                                       override, defragmentationEnable);

        if ((rc == GT_OK) || (rc == GT_ALREADY_EXIST))
        {
            if (rc == GT_OK)
            {
                numberOfAddedPrefixes++;
            }
        }
        else
        {
            break;
        }
    }
    if (numberOfAddedPrefixes==3324)
    {
        cpssOsPrintf("\ncpssDxChIpLpmIpv4UcPrefixAdd OK numberOfAddedPrefixes= %d \n",numberOfAddedPrefixes);
    }
    else
    {
        cpssOsPrintf("\ncpssDxChIpLpmIpv4UcPrefixAdd FAIL numberOfAddedPrefixes= %d \n",numberOfAddedPrefixes);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_FAIL, "prvTgfIpLpmIpv4UcOrderAndPredefined: %d", prvTgfDevNum);
    }

    /* print the block mapping state */
    cpssOsPrintf("==== HW Lpm Lines Counters usage - 5.5.5.5 BEFORE ADDED  ");
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters: %d", prvTgfDevNum);


    /* add the second prefix in the list and some more 5.5.5.5 - 5.5.x.y*/
    rc = prvTgfIpLpmIpv4UcPrefixAddManyByOctet(prvTgfLpmDBId, 0, &unIpv4AddrArray[2],66,500, 3, 2, 1, 0, &lastIpAddrAdded,&numOfPrefixesAdded);
    if (rc == GT_OK)
    {
        cpssOsPrintf("[last ipv4 unicast added] = [%d]:[%d]:[%d]:[%d] numOfPrefixesAdded=%d,\n\r",
                  lastIpAddrAdded.arIP[0]
                 ,lastIpAddrAdded.arIP[1]
                 ,lastIpAddrAdded.arIP[2]
                 ,lastIpAddrAdded.arIP[3]
                 ,numOfPrefixesAdded);

    }
    else
    {
        cpssOsPrintf("\n prvTgfIpLpmIpv4UcPrefixAddManyByOctet FAIL numberOfAddedPrefixes= %d \n",numberOfAddedPrefixes);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_FAIL, "prvTgfIpLpmIpv4UcOrderAndPredefined: %d", prvTgfDevNum);
    }

     /* print the block mapping state */
    cpssOsPrintf("==== HW Lpm Lines Counters usage - 5.5.5.5 AFTER ADDED  ");
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters: %d", prvTgfDevNum);

    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
         /* disable LPM validity check */
         prvTgfIpValidityCheckEnable(GT_FALSE);
    }
    for (ulLoop = 3324; ulLoop < totalNumIpUc; ulLoop++)
    {
        ipv4Addr.arIP[3] = (GT_U8)(prefixesArrayPtr[ulLoop]%256);
        ipv4Addr.arIP[2] = (GT_U8)(prefixesArrayPtr[ulLoop]/256);
        rc = prvTgfIpLpmIpv4UcPrefixAdd(0, 0, ipv4Addr,ipPrefixLen, &nextHopInfo,
                                       override, defragmentationEnable);

        if ((rc == GT_OK) || (rc == GT_ALREADY_EXIST))
        {
            if (rc == GT_OK)
            {
                numberOfAddedPrefixes++;
            }
        }
        else
        {
            break;
        }
        if ((PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))&&
            ((numberOfAddedPrefixes%5000==0)||(ulLoop==3324)||(ulLoop==(totalNumIpUc-1))))
        {
            /* enable LPM validity check */
            prvTgfIpValidityCheckEnable(GT_TRUE);

            rc = prvTgfIpValidityCheck(prvTgfLpmDBId,0,CPSS_IP_PROTOCOL_IPV4_E,CPSS_UNICAST_E);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck: %d", prvTgfDevNum);

            /* disable LPM validity check */
            prvTgfIpValidityCheckEnable(GT_FALSE);
        }
    }
    if (numberOfAddedPrefixes==numberOfPrefixesExpected)
    {
        cpssOsPrintf("\ncpssDxChIpLpmIpv4UcPrefixAdd OK numberOfAddedPrefixes= %d / Expected=%d / totalNumIpUc=%d\n",numberOfAddedPrefixes, numberOfPrefixesExpected, totalNumIpUc);
    }
    else
    {
        cpssOsPrintf("\ncpssDxChIpLpmIpv4UcPrefixAdd FAIL numberOfAddedPrefixes= %d / Expected=%d / totalNumIpUc=%d\n",numberOfAddedPrefixes, numberOfPrefixesExpected, totalNumIpUc);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_FAIL, "prvTgfIpLpmIpv4UcOrderAndDisorder: %d", prvTgfDevNum);
    }

    /* print the block mapping state */
    cpssOsPrintf("==== HW Lpm Lines Counters usage - before remove\n  ");
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters: %d", prvTgfDevNum);

    /* free some of the prefixes */
    for (ulLoop = 0; ulLoop < 3324; ulLoop++)
    {
        ipv4Addr.arIP[3] = (GT_U8)(prefixesArrayPtr[ulLoop]%256);
        ipv4Addr.arIP[2] = (GT_U8)(prefixesArrayPtr[ulLoop]/256);
        rc = prvTgfIpLpmIpv4UcPrefixDel(0, 0, ipv4Addr,ipPrefixLen);

        if (rc!=GT_OK)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcOrderAndDisorder: %d", prvTgfDevNum);
        }
        if ((PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))&&
           ((ulLoop%500==0)||(ulLoop==0)||(ulLoop==3323)))
        {
            /* enable LPM validity check */
            prvTgfIpValidityCheckEnable(GT_TRUE);

            rc = prvTgfIpValidityCheck(prvTgfLpmDBId,0,CPSS_IP_PROTOCOL_IPV4_E,CPSS_UNICAST_E);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck: %d", prvTgfDevNum);

            /* disable LPM validity check */
            prvTgfIpValidityCheckEnable(GT_FALSE);
        }
    }
    /* print the block mapping state */
    cpssOsPrintf("==== HW Lpm Lines Counters usage after remove\n ");
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters: %d", prvTgfDevNum);


    /* try to add prefixes with prefixLen/8
       if defrag=false we should get a fail
       if defrag=true we should have shrink+merge and succeed */
    ipv4Addr.arIP[2]=12;
    ipv4Addr.arIP[3]=13;
    ipPrefixLen = 16;
    numberOfAddedPrefixes=0;

    /* add some of the prefixes */
    for (ulLoop = 0; ulLoop < 5000; ulLoop++)
    {
        ipv4Addr.arIP[1] = (GT_U8)(prefixesArrayPtr[ulLoop]%256);
        ipv4Addr.arIP[0] = (GT_U8)(prefixesArrayPtr[ulLoop]/256);
        rc = prvTgfIpLpmIpv4UcPrefixAdd(0, 0, ipv4Addr,ipPrefixLen, &nextHopInfo,
                               override, defragmentationEnable);

        if ((rc == GT_OK) || (rc == GT_ALREADY_EXIST))
        {
            if (rc == GT_OK)
            {
                numberOfAddedPrefixes++;
            }
        }
        else
        {
            break;
        }
        if ((PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))&&
            ((numberOfAddedPrefixes%500==0)||(ulLoop==0)||(ulLoop==4999)))
        {
            /* enable LPM validity check */
            prvTgfIpValidityCheckEnable(GT_TRUE);

            rc = prvTgfIpValidityCheck(prvTgfLpmDBId,0,CPSS_IP_PROTOCOL_IPV4_E,CPSS_UNICAST_E);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck: %d", prvTgfDevNum);

            /* disable LPM validity check */
            prvTgfIpValidityCheckEnable(GT_FALSE);
        }
    }

    /* in case defragmentationEnable==GT_FALSE */
     if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
     {
         numberOfPrefixesExpected = 1662;
     }
     else
     {
         numberOfPrefixesExpected = 1689;
     }
     if (numberOfAddedPrefixes==numberOfPrefixesExpected)
    {
        cpssOsPrintf("\ncpssDxChIpLpmIpv4UcPrefixAdd OK numberOfAddedPrefixes= %d / Expected=%d / totalNumIpUc=%d\n",numberOfAddedPrefixes, numberOfPrefixesExpected, totalNumIpUc);
    }
    else
    {
        cpssOsPrintf("\ncpssDxChIpLpmIpv4UcPrefixAdd FAIL numberOfAddedPrefixes= %d / Expected=%d / totalNumIpUc=%d\n",numberOfAddedPrefixes, numberOfPrefixesExpected, totalNumIpUc);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_FAIL, "prvTgfIpLpmIpv4UcOrderAndDisorder: %d", prvTgfDevNum);
    }
    cpssOsPrintf("\n prvTgfIpLpmDbgHwOctetPerProtocolPrint \n");
    prvTgfIpLpmDbgHwOctetPerProtocolPrint(0);
    cpssOsPrintf("\n prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmMemoryAndCounters \n");
    prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmMemoryAndCounters(0);


    defragmentationEnable=GT_TRUE;
    /* after we get a fail that there is no more space in memory
       we try to add some more prefixes with defragmentationEnable=GT_TRUE
       we can see that now we can add mutch more */
    for (ulLoop = numberOfPrefixesExpected; ulLoop < 10000; ulLoop++)
    {
        ipv4Addr.arIP[1] = (GT_U8)(prefixesArrayPtr[ulLoop]%256);
        ipv4Addr.arIP[0] = (GT_U8)(prefixesArrayPtr[ulLoop]/256);
        rc = prvTgfIpLpmIpv4UcPrefixAdd(0, 0, ipv4Addr,ipPrefixLen, &nextHopInfo,
                               override, defragmentationEnable);

        if ((rc == GT_OK) || (rc == GT_ALREADY_EXIST))
        {
            if (rc == GT_OK)
            {
                numberOfAddedPrefixes++;
            }
        }
        else
        {
            break;
        }
        if ((PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))&&
            ((numberOfAddedPrefixes%500==0)||(ulLoop==numberOfPrefixesExpected)||(ulLoop==9999)))
        {
            /* enable LPM validity check */
            prvTgfIpValidityCheckEnable(GT_TRUE);

            rc = prvTgfIpValidityCheck(prvTgfLpmDBId,0,CPSS_IP_PROTOCOL_IPV4_E,CPSS_UNICAST_E);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpValidityCheck: %d", prvTgfDevNum);

            /* disable LPM validity check */
            prvTgfIpValidityCheckEnable(GT_FALSE);
        }
    }

   /* if we only use simple defrag we can add 1958 -
       this was checked manualy by skipping the merge part in the code
       if we use defrag + merge we can add 5542 meaning the merge
       added 3584 more than simple shrink */
    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        /* merge is still not supported for falcon */
        numberOfPrefixesExpected = 4846; /* with merge. 1774 - without merge */
    }
    else
    {
        numberOfPrefixesExpected = 5542;
    }
    if (numberOfAddedPrefixes==numberOfPrefixesExpected)
    {
        cpssOsPrintf("\ncpssDxChIpLpmIpv4UcPrefixAdd OK numberOfAddedPrefixes= %d / Expected=%d / totalNumIpUc=%d\n",numberOfAddedPrefixes, numberOfPrefixesExpected, totalNumIpUc);
    }
    else
    {
        cpssOsPrintf("\ncpssDxChIpLpmIpv4UcPrefixAdd FAIL numberOfAddedPrefixes= %d / Expected=%d / totalNumIpUc=%d\n",numberOfAddedPrefixes, numberOfPrefixesExpected, totalNumIpUc);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_FAIL, "prvTgfIpLpmIpv4UcOrderAndDisorder: %d", prvTgfDevNum);
    }
    cpssOsPrintf("\n prvTgfIpLpmDbgHwOctetPerProtocolPrint \n");
    prvTgfIpLpmDbgHwOctetPerProtocolPrint(0);
    cpssOsPrintf("\n prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmMemoryAndCounters \n");
    prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmMemoryAndCounters(0);

    cpssOsFree(prefixesArrayPtr);

    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        /* enable LPM validity check */
        prvTgfIpValidityCheckEnable(GT_TRUE);
    }
    return;
}

/**
* @internal prvTgfIpLpmIpUcOrderOctet0To6 function
* @endinternal
*
* @brief  Add some sequentially ipv6 Prefixes in order manner.
*         Verify that we get same number of prefixes as expected
* @param[in] totalNumIpUc             - number of ipv4 UC entries to add

*/
GT_VOID prvTgfIpLpmIpUcOrderOctet0To6
(
    GT_U32                      totalNumIpUc,
    CPSS_IP_PROTOCOL_STACK_ENT  protocolStack,
    CPSS_UNICAST_MULTICAST_ENT  prefixType,
    GT_U32                      prefixLength,
    GT_U32                      srcPrefixLength
)
{
    GT_STATUS                               rc = GT_OK;
    GT_U32                                  numberOfPrefixesExpected;

    GT_IPV6ADDR                             unIpv6Addr;
    GT_IPV6ADDR                             lastIpAddrAdded;
    GT_U32                                  numOfPrefixesAdded=0;


    PRV_UTF_LOG0_MAC("\n==== Adding prefixes sequentially WITH defrag \n");
    if (totalNumIpUc<1364)
    {
        numberOfPrefixesExpected = totalNumIpUc;
    }
    else
    {
        if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
        {
            numberOfPrefixesExpected = 1590;
        }
        else
        {
            numberOfPrefixesExpected = 1364;
        }
    }

    cpssOsMemSet(&unIpv6Addr.arIP[0],0,sizeof(unIpv6Addr.arIP));

    PRV_UTF_LOG0_MAC("\n==== Flushing IPv4 and IPv6 Uc and Mc Prefixes \n");
    cpssDxChIpLpmIpv6UcPrefixesFlush(0,0);
    cpssDxChIpLpmIpv4UcPrefixesFlush(0,0);
    cpssDxChIpLpmIpv4McEntriesFlush(0,0);
    cpssDxChIpLpmIpv6McEntriesFlush(0,0);

    /* change global prefixLength used in the prvTgfIpLpmIpv6UcPrefixAddManyByOctet API */
    rc = prvTgfIpLpmDbgPrefixLengthSet(protocolStack,prefixType,prefixLength,srcPrefixLength);
    if (rc!=GT_OK)
    {
        cpssOsPrintf("\n prvTgfIpLpmDbgPrefixLengthSet FAIL numberOfAddedPrefixes= %d \n",numOfPrefixesAdded);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_FAIL, "prvTgfIpLpmDbgPrefixLengthSet: %d", prvTgfDevNum);
    }
    /* add the prefixes */
    rc = prvTgfIpLpmIpv6UcPrefixAddManyByOctet(prvTgfLpmDBId, 0, &unIpv6Addr, 55, totalNumIpUc,
                                               0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
                                               &lastIpAddrAdded,&numOfPrefixesAdded);

    if (rc == GT_OK)
    {
        cpssOsPrintf("[last ipv6 unicast added] = [%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d]:[%d] numOfPrefixesAdded=%d,\n\r",
                  lastIpAddrAdded.arIP[0]
                 ,lastIpAddrAdded.arIP[1]
                 ,lastIpAddrAdded.arIP[2]
                 ,lastIpAddrAdded.arIP[3]
                 ,lastIpAddrAdded.arIP[4]
                 ,lastIpAddrAdded.arIP[5]
                 ,lastIpAddrAdded.arIP[6]
                 ,lastIpAddrAdded.arIP[7]
                 ,lastIpAddrAdded.arIP[8]
                 ,lastIpAddrAdded.arIP[9]
                 ,lastIpAddrAdded.arIP[10]
                 ,lastIpAddrAdded.arIP[11]
                 ,lastIpAddrAdded.arIP[12]
                 ,lastIpAddrAdded.arIP[13]
                 ,lastIpAddrAdded.arIP[14]
                 ,lastIpAddrAdded.arIP[15]
                 ,numOfPrefixesAdded);
    }
    else
    {
        cpssOsPrintf("\n prvTgfIpLpmIpv4UcPrefixAddManyByOctet FAIL numberOfAddedPrefixes= %d \n",numOfPrefixesAdded);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_FAIL, "prvTgfIpLpmIpv4UcOrderAndPredefined: %d", prvTgfDevNum);
    }

     /* print the block mapping state */
    cpssOsPrintf("==== HW Lpm Lines Counters usage ");
    cpssOsPrintf("\n prvTgfIpLpmDbgHwOctetPerProtocolPrint \n");
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrint(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerProtocolPrint: %d", prvTgfDevNum);
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters: %d", prvTgfDevNum);

    if (numOfPrefixesAdded==numberOfPrefixesExpected)
    {
        cpssOsPrintf("\n prvTgfIpLpmIpv6UcPrefixAddManyByOctet OK numberOfAddedPrefixes= %d / Expected=%d / totalNumIpUc=%d\n",numOfPrefixesAdded, numberOfPrefixesExpected, totalNumIpUc);
    }
    else
    {
        cpssOsPrintf("\n prvTgfIpLpmIpv6UcPrefixAddManyByOctet FAIL numberOfAddedPrefixes= %d / Expected=%d / totalNumIpUc=%d\n",numOfPrefixesAdded, numberOfPrefixesExpected, totalNumIpUc);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_FAIL, "prvTgfIpLpmIpv4UcOrderAndDisorder: %d", prvTgfDevNum);
    }

    rc = prvTgfIpLpmValidityCheck(prvTgfLpmDBId, 0, protocolStack , prefixType, GT_TRUE);
    if (rc!=GT_OK)
    {
        cpssOsPrintf("\n prvTgfIpLpmValidityCheck FAIL numberOfAddedPrefixes= %d / Expected=%d / totalNumIpUc=%d\n",numOfPrefixesAdded, numberOfPrefixesExpected, totalNumIpUc);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_FAIL, "prvTgfIpLpmValidityCheck: %d", prvTgfDevNum);
    }


    PRV_UTF_LOG0_MAC("\n==== Flushing IPv4 and IPv6 Uc and Mc Prefixes \n");
    cpssDxChIpLpmIpv6UcPrefixesFlush(0,0);
    cpssDxChIpLpmIpv4UcPrefixesFlush(0,0);
    cpssDxChIpLpmIpv4McEntriesFlush(0,0);
    cpssDxChIpLpmIpv6McEntriesFlush(0,0);

    cpssOsPrintf("==== HW Lpm Lines Counters usage ");
    cpssOsPrintf("\n prvTgfIpLpmDbgHwOctetPerProtocolPrint \n");
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrint(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerProtocolPrint: %d", prvTgfDevNum);
    cpssOsPrintf("\n prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters \n");
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters: %d", prvTgfDevNum);

    /* change global prefixLength to default */
    rc = prvTgfIpLpmDbgPrefixLengthSet(protocolStack,prefixType,128,128);
    if(rc != GT_OK)
    {
        cpssOsPrintf("\n prvTgfIpLpmDbgPrefixLengthSet FAIL numberOfAddedPrefixes= %d \n",numOfPrefixesAdded);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_FAIL, "prvTgfIpLpmDbgPrefixLengthSet: %d", prvTgfDevNum);
    }
    return;
}

/* default LPM DB and Virtual Router configuration */
static struct
{
    GT_BOOL                                     isSupportIpv4Uc;
    GT_BOOL                                     isSupportIpv6Uc;
    GT_BOOL                                     isSupportIpv4Mc;
    GT_BOOL                                     isSupportIpv6Mc;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT        defIpv4UcNextHopInfo;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT        defIpv6UcNextHopInfo;
    PRV_TGF_IP_LTT_ENTRY_STC                    defIpv4McRouteLttEntry;
    PRV_TGF_IP_LTT_ENTRY_STC                    defIpv6McRouteLttEntry;
    PRV_TGF_IP_TCAM_SHADOW_TYPE_ENT             shadowType;
    CPSS_IP_PROTOCOL_STACK_ENT                  protocolStack;
    PRV_TGF_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC  indexesRange;
    GT_BOOL                                     partitionEnable;
    PRV_TGF_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC  tcamLpmManagerCapcityCfg;
    GT_BOOL                                     isTcamManagerInternal;
} prvTgfDefLpmDBVrCfg;


static PRV_TGF_IP_TCAM_SHADOW_TYPE_ENT             shadowTypeParam;
static CPSS_IP_PROTOCOL_STACK_ENT                  protocolStackParam;
static PRV_TGF_LPM_RAM_CONFIG_STC                  ramDbCfgParam;

static GT_U32                                      numOfDevsParam;
static GT_U8                                       devListArray[PRV_CPSS_MAX_PP_DEVICES_CNS];

/* parameters that is needed to be restored */
static struct
{
    GT_U16                               vid;
    CPSS_PORTS_BMP_STC                   defPortMembers;
} prvTgfRestoreCfg;

/**
* @internal prvTgfIpLpmHaBasicRoutingConfigurationSet function
* @endinternal
*
* @brief   Set LTT Route Configuration for ipv4/6 UC/MC
*
* @param[in] prvUtfVrfId              - virtual router index
* @param[in] sendPortNum              - port sending traffic
* @param[in] nextHopPortNum           - port receiving traffic
* @param[in] firstCall                - GT_TRUE: callinf this
*                                       API for the first time
*                                       GT_FALSE: callimg this
*                                       API for the second time
*/
static GT_VOID prvTgfIpLpmHaBasicRoutingConfigurationSet
(
    GT_U32   prvUtfVrfId,
    GT_U8    sendPortIndex,
    GT_U32   nextHopPortNum,
    GT_BOOL  firstCall
)
{
    GT_STATUS                               rc = GT_OK;
    GT_ETHERADDR                            arpMacAddr;
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           routeEntriesArray[1];
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           *regularEntryPtr;

    GT_HW_DEV_NUM                           targetHwDevice;
    GT_PORT_NUM                             targetPort;
    GT_U8                                   tagArray[] = {1, 1, 1, 1};
    PRV_TGF_BRG_MAC_ENTRY_STC               macEntry;
    PRV_TGF_BRG_MAC_ENTRY_STC               macEntryGet;
    PRV_TGF_MAC_ENTRY_KEY_STC               macEntryKey;

    PRV_TGF_IP_MLL_PAIR_STC                 mllPairEntry;
    PRV_TGF_IP_MLL_STC                      *mllNodePtr;
    PRV_TGF_IP_MC_ROUTE_ENTRY_STC           mcRouteEntryArray[1];
    CPSS_PORTS_BMP_STC                      portMembers = {{0, 0}};

    PRV_TGF_PORT_INDEX_CHECK_FOR_VOID_FUNC_MAC(sendPortIndex);

    /* AUTODOC: SETUP CONFIGURATION: */
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portMembers);

   /* AUTODOC: create VLAN 5 with tagged ports 0,1 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_SEND_VLANID_CNS,prvTgfPortsArray, NULL, tagArray, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* AUTODOC: create VLAN 6 with tagged ports 2,3 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_NEXTHOPE_VLANID_CNS,prvTgfPortsArray + 2, NULL, tagArray + 2, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* AUTODOC: enable VLAN based MAC learning */
    rc = prvTgfBrgVlanLearningStateSet(PRV_TGF_SEND_VLANID_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanLearningStateSet: %d", prvTgfDevNum);

    if(firstCall==GT_TRUE)
    {
        /* save default vlanId for restore */
        rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum,
                                     prvTgfPortsArray[sendPortIndex],
                                     &prvTgfRestoreCfg.vid);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidGet: %d, %d",
                                     prvTgfDevNum, &prvTgfRestoreCfg.vid);

        /* save default port members for MC group */
        rc = prvTgfBrgMcEntryRead(prvTgfDevNum, PRV_TGF_DEF_VIDX_CNS,
                                  &(prvTgfRestoreCfg.defPortMembers));
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgMcEntryRead: %d, %d",
                                     prvTgfDevNum, PRV_TGF_DEF_VIDX_CNS);

    }

    /* AUTODOC: set PVID 5 for port 0 */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum,
                                 prvTgfPortsArray[sendPortIndex],
                                 PRV_TGF_SEND_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[sendPortIndex]);

    /* create bmp with ports for MC group */
    CPSS_PORTS_BMP_PORT_SET_MAC(&portMembers, prvTgfPortsArray[PRV_TGF_MC_1_PORT_IDX_CNS]);
    CPSS_PORTS_BMP_PORT_SET_MAC(&portMembers, prvTgfPortsArray[PRV_TGF_MC_2_PORT_IDX_CNS]);
    CPSS_PORTS_BMP_PORT_SET_MAC(&portMembers, prvTgfPortsArray[PRV_TGF_MC_3_PORT_IDX_CNS]);

    /* write MC Group entry */
    rc = prvTgfBrgMcEntryWrite(prvTgfDevNum, PRV_TGF_DEF_VIDX_CNS, &portMembers);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgMcEntryWrite: %d, %d",
                                 prvTgfDevNum, PRV_TGF_DEF_VIDX_CNS);

    /* -------------------------------------------------------------------------
     * Set the FDB entry With DA_ROUTE
     */

    /* create a macEntry with .daRoute = GT_TRUE */
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));

    macEntry.key.entryType                  = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntry.key.key.macVlan.vlanId         = PRV_TGF_SEND_VLANID_CNS;
    macEntry.dstInterface.type              = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum    = prvTgfDevNum;
    macEntry.dstInterface.devPort.portNum   = prvTgfPortsArray[1];
    macEntry.isStatic                       = GT_TRUE;
    macEntry.daCommand                      = PRV_TGF_PACKET_CMD_FORWARD_E;
    macEntry.saCommand                      = PRV_TGF_PACKET_CMD_FORWARD_E;
    macEntry.daRoute                        = GT_TRUE;
    macEntry.sourceId                       = 0;
    macEntry.userDefined                    = 0;
    macEntry.daQosIndex                     = 0;
    macEntry.saQosIndex                     = 0;
    macEntry.daSecurityLevel                = 0;
    macEntry.saSecurityLevel                = 0;
    macEntry.appSpecificCpuCode             = GT_FALSE;
    macEntry.mirrorToRxAnalyzerPortEn       = GT_FALSE;

    /* AUTODOC: add FDB entry with MAC {0x00, 0x00, 0x00, 0x00, 0x34, 0x02}, VLAN 5, port 1 */
    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));
    rc = prvTgfBrgFdbMacEntrySet(&macEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryWrite: %d", prvTgfDevNum);

    /* AUTODOC: add FDB entry with MAC {0x01, 0x00, 0x5E, 0x01, 0x01, 0x01}, VLAN 5, port 1 */
    rc = prvTgfBrgDefFdbMacEntryOnVidxSet(prvTgfPacketL2Part_ipv4Mc.daMac,
                                          PRV_TGF_SEND_VLANID_CNS,
                                          PRV_TGF_DEF_VIDX_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnVidxSet: %d, %d",
                                 prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    /* AUTODOC: add FDB entry with MAC {0x33, 0x33, 0x00, 0x00, 0x22, 0x22}, VLAN 5, port 1 */
    rc = prvTgfBrgDefFdbMacEntryOnVidxSet(prvTgfPacketL2Part_ipv6Mc.daMac,
                                          PRV_TGF_SEND_VLANID_CNS,
                                          PRV_TGF_DEF_VIDX_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnVidxSet: %d, %d",
                                 prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    cpssOsMemSet(&macEntryKey, 0, sizeof(TGF_MAC_ADDR));
    cpssOsMemCpy(macEntryKey.key.macVlan.macAddr.arEther, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));
    macEntryKey.entryType = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntryKey.key.macVlan.vlanId = PRV_TGF_SEND_VLANID_CNS;

    rc = prvTgfBrgFdbMacEntryGet(&macEntryKey,&macEntryGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryGet: %d", prvTgfDevNum);

    PRV_UTF_LOG0_MAC("==== Setting LTT Route Configuration ====\n");

    targetHwDevice  = prvTgfDevNum;
    targetPort      = nextHopPortNum;

    /* -------------------------------------------------------------------------
     * 1. Enable Routing
     */

    /* AUTODOC: enable IPv4 UC Routing on port 0(sendPort) */
    rc = prvTgfIpPortRoutingEnable(sendPortIndex, CPSS_IP_UNICAST_E,CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d", prvTgfDevNum, sendPortIndex);
    /* AUTODOC: enable IPv6 UC Routing on port 0(sendPort) */
    rc = prvTgfIpPortRoutingEnable(sendPortIndex, CPSS_IP_UNICAST_E,CPSS_IP_PROTOCOL_IPV6_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d", prvTgfDevNum, sendPortIndex);
    /* AUTODOC: enable IPv4 MC Routing on port 0(sendPort) */
    rc = prvTgfIpPortRoutingEnable(sendPortIndex, CPSS_IP_MULTICAST_E,CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d", prvTgfDevNum, sendPortIndex);
    /* AUTODOC: enable IPv6 MC Routing on port 0(sendPort) */
    rc = prvTgfIpPortRoutingEnable(sendPortIndex, CPSS_IP_MULTICAST_E,CPSS_IP_PROTOCOL_IPV6_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d", prvTgfDevNum, sendPortIndex);


    /* AUTODOC: update VRF Id in vlan[PRV_TGF_SEND_VLANID_CNS] */
    rc = prvTgfBrgVlanVrfIdSet(PRV_TGF_SEND_VLANID_CNS, prvUtfVrfId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanVrfIdSet: %d %d",prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);


    /* AUTODOC: enable IPv4 Unicast Routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);
    /* AUTODOC: enable IPv6 Unicast Routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV6_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);
        /* AUTODOC: enable IPv4 Multitcast Routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_MULTICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);
        /* AUTODOC: enable IPv6 Multitcast Routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_MULTICAST_E, CPSS_IP_PROTOCOL_IPV6_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    /* -------------------------------------------------------------------------
     * 2. Create the Route entry (Next hop) in Route table and Router ARP Table
     */

    /* AUTODOC: write a ARP MAC 00:00:00:00:00:22 to the Router ARP Table */
    cpssOsMemCpy(arpMacAddr.arEther, prvTgfArpMac, sizeof(TGF_MAC_ADDR));

   /* Create the Route entry (Next hop) in FDB table and Router ARP Table */
    rc = prvTgfIpRouterArpAddrRead(prvTgfDevNum,prvTgfRouterArpIndex,&prvTgfArpMacAddrGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrRead: %d", prvTgfDevNum);

    rc = prvTgfIpRouterArpAddrWrite(prvTgfRouterArpIndex, &arpMacAddr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

    /* write a UC Route entry to the Route Table */
    cpssOsMemSet(routeEntriesArray, 0, sizeof(routeEntriesArray));

    regularEntryPtr = &routeEntriesArray[0];
    regularEntryPtr->cmd                        = CPSS_PACKET_CMD_ROUTE_E;
    regularEntryPtr->cpuCodeIndex               = 0;
    regularEntryPtr->appSpecificCpuCodeEnable   = GT_FALSE;
    regularEntryPtr->unicastPacketSipFilterEnable = GT_FALSE;
    regularEntryPtr->ttlHopLimitDecEnable       = GT_FALSE;
    regularEntryPtr->ttlHopLimDecOptionsExtChkByPass = GT_FALSE;
    regularEntryPtr->ingressMirror              = GT_FALSE;
    regularEntryPtr->qosProfileMarkingEnable    = GT_FALSE;
    regularEntryPtr->qosProfileIndex            = 0;
    regularEntryPtr->qosPrecedence              = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
    regularEntryPtr->modifyUp                   = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
    regularEntryPtr->modifyDscp                 = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
    regularEntryPtr->countSet                   = PRV_TGF_COUNT_SET_CNS;
    regularEntryPtr->trapMirrorArpBcEnable      = GT_FALSE;
    regularEntryPtr->sipAccessLevel             = 0;
    regularEntryPtr->dipAccessLevel             = 0;
    regularEntryPtr->ICMPRedirectEnable         = GT_FALSE;
    regularEntryPtr->scopeCheckingEnable        = GT_FALSE;
    regularEntryPtr->siteId                     = CPSS_IP_SITE_ID_INTERNAL_E;
    regularEntryPtr->mtuProfileIndex            = 0;
    regularEntryPtr->isTunnelStart              = GT_FALSE;
    regularEntryPtr->nextHopVlanId              = PRV_TGF_NEXTHOPE_VLANID_CNS;

    regularEntryPtr->nextHopInterface.type              = CPSS_INTERFACE_PORT_E;
    regularEntryPtr->nextHopInterface.devPort.hwDevNum  = targetHwDevice;
    regularEntryPtr->nextHopInterface.devPort.portNum   = targetPort;

    regularEntryPtr->nextHopARPPointer                  = prvTgfRouterArpIndex;
    regularEntryPtr->nextHopTunnelPointer               = 0;

    /* AUTODOC: add ipv4 UC route entry with nexthop VLAN 6 and nexthop port 3 */
    rc = prvTgfIpUcRouteEntriesWrite(prvTgfUcIpv4RouteEntryBaseIndex, routeEntriesArray, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: %d", prvTgfDevNum);

    /* AUTODOC: add ipv6 UC route entry with nexthop VLAN 6 and nexthop port 3 */
    rc = prvTgfIpUcRouteEntriesWrite(prvTgfUcIpv6RouteEntryBaseIndex, routeEntriesArray, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: %d", prvTgfDevNum);


    /* create first and second MC Link List */
    cpssOsMemSet(&mllPairEntry, 0, sizeof(mllPairEntry));

    mllNodePtr = &mllPairEntry.firstMllNode;
    mllNodePtr->mllRPFFailCommand                = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    mllNodePtr->nextHopInterface.type            = CPSS_INTERFACE_PORT_E;
    mllNodePtr->nextHopInterface.devPort.hwDevNum  = prvTgfDevNum;
    mllNodePtr->nextHopInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_MC_1_PORT_IDX_CNS];
    mllNodePtr->nextHopVlanId                    = PRV_TGF_NEXTHOPE_VLANID_CNS;
    mllNodePtr->ttlHopLimitThreshold             = 4;
    mllNodePtr->last                             = GT_FALSE;

    mllNodePtr = &mllPairEntry.secondMllNode;
    mllNodePtr->mllRPFFailCommand                = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    mllNodePtr->nextHopInterface.type            = CPSS_INTERFACE_PORT_E;
    mllNodePtr->nextHopInterface.devPort.hwDevNum  = prvTgfDevNum;
    mllNodePtr->nextHopInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_MC_2_PORT_IDX_CNS];
    mllNodePtr->nextHopVlanId                    = PRV_TGF_NEXTHOPE_VLANID_CNS;
    mllNodePtr->ttlHopLimitThreshold             = 4;
    mllNodePtr->last                             = GT_TRUE;

    mllPairEntry.nextPointer                     = 0;

    /* AUTODOC: create MC Link List 5: */
    /* AUTODOC:   1. port 2, VLAN 6 */
    /* AUTODOC:   2. port 3, VLAN 6 */
    rc = prvTgfIpMllPairWrite(prvTgfMllPointerIndex,
                              PRV_TGF_PAIR_READ_WRITE_WHOLE_E,
                              &mllPairEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMllPairWrite");

    /* create a MC Route entry */
    cpssOsMemSet(&mcRouteEntryArray[0], 0, sizeof(mcRouteEntryArray[0]));
    mcRouteEntryArray[0].cmd                = CPSS_PACKET_CMD_ROUTE_E;
    mcRouteEntryArray[0].countSet           = CPSS_IP_CNT_SET2_E;
    mcRouteEntryArray[0].RPFFailCommand     = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    mcRouteEntryArray[0].ipv4MllPtr         = prvTgfMllPointerIndex;
    mcRouteEntryArray[0].ipv6ExternalMllPtr = prvTgfMllPointerIndex;
    mcRouteEntryArray[0].ipv6InternalMllPtr = prvTgfMllPointerIndex + 1;

    /* AUTODOC: add MC route entry with index 7, MLL Ptr 5 */
    rc = prvTgfIpMcRouteEntriesWrite(prvTgfMcIpv4RouteEntryBaseIndex,1,CPSS_IP_PROTOCOL_IPV4_E,mcRouteEntryArray);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite");

    /* AUTODOC: add MC route entry with index 7, MLL Ptr 5 */
    rc = prvTgfIpMcRouteEntriesWrite(prvTgfMcIpv6RouteEntryBaseIndex,1,CPSS_IP_PROTOCOL_IPV6_E,mcRouteEntryArray);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite");
}
/**
* @internal prvTgfIpLpmHaBasicRoutingConfigurationRestore
*           function
* @endinternal
*
* @brief   Set LTT Route Configuration for ipv4/6 UC/MC
*
* @param[in] sendPortNum              - port sending traffic
*
*/
static GT_VOID prvTgfIpLpmHaBasicRoutingConfigurationRestore
(
    GT_U8    sendPortIndex
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    mllCntSet_0 = 0, mllCntSet_1 = 1;
    PRV_TGF_IP_COUNTER_SET_INTERFACE_CFG_STC cntIntCfg;

    PRV_TGF_PORT_INDEX_CHECK_FOR_VOID_FUNC_MAC(sendPortIndex);

    /* delete the FDB entry */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d", prvTgfDevNum);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: disable IPv4 UC Routing on port 0(sendPort) */
    rc = prvTgfIpPortRoutingEnable(sendPortIndex, CPSS_IP_UNICAST_E,CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d", prvTgfDevNum, sendPortIndex);
    /* AUTODOC: disable IPv6 Uc Routing on port 0(sendPort) */
    rc = prvTgfIpPortRoutingEnable(sendPortIndex, CPSS_IP_UNICAST_E,CPSS_IP_PROTOCOL_IPV6_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d", prvTgfDevNum, sendPortIndex);
    /* AUTODOC: disable IPv4 MC Routing on port 0(sendPort) */
    rc = prvTgfIpPortRoutingEnable(sendPortIndex, CPSS_IP_MULTICAST_E,CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d", prvTgfDevNum, sendPortIndex);
    /* AUTODOC: disable IPv6 MC Routing on port 0(sendPort) */
    rc = prvTgfIpPortRoutingEnable(sendPortIndex, CPSS_IP_MULTICAST_E,CPSS_IP_PROTOCOL_IPV6_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d", prvTgfDevNum, sendPortIndex);


    /* AUTODOC: disable IPv4 Unicast Routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);
    /* AUTODOC: disable IPv6 Unicast Routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV6_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);
    /* AUTODOC: disable IPv4 Multitcast Routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_MULTICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);
    /* AUTODOC: disable IPv6 Multitcast Routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_MULTICAST_E, CPSS_IP_PROTOCOL_IPV6_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    /* set default MLL counters mode */
    cpssOsMemSet(&cntIntCfg, 0, sizeof(cntIntCfg));

    rc = prvTgfIpSetMllCntInterface(mllCntSet_0, &cntIntCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpSetMllCntInterface");

    rc = prvTgfIpSetMllCntInterface(mllCntSet_1, &cntIntCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpSetMllCntInterface");

    /* restore MC Group entry */
    rc = prvTgfBrgMcEntryWrite(prvTgfDevNum, PRV_TGF_DEF_VIDX_CNS,
                               &(prvTgfRestoreCfg.defPortMembers));
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgMcEntryWrite: %d, %d",
                                 prvTgfDevNum, PRV_TGF_DEF_VIDX_CNS);

    /* AUTODOC: restore default vlanId to all ports */
    /*rc = prvTgfBrgVlanPvidSet(prvTgfRestoreCfg.vid);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPvidSet: %d, %d",
                                 prvTgfDevNum, prvTgfRestoreCfg.vid);*/

    /* AUTODOC: enable VLAN based MAC learning */
    rc = prvTgfBrgVlanLearningStateSet(PRV_TGF_SEND_VLANID_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanLearningStateSet: %d", prvTgfDevNum);

    /* default vlanId restore */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum,
                                 prvTgfPortsArray[sendPortIndex],
                                 prvTgfRestoreCfg.vid);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d",
                                 prvTgfDevNum, prvTgfRestoreCfg.vid);

    /* invalidate and reset vlan entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_SEND_VLANID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"ERROR of prvTgfBrgDefVlanEntryInvalidate");
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_NEXTHOPE_VLANID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"ERROR of prvTgfBrgDefVlanEntryInvalidate");

    /* AUTODOC: restore a ARP MAC  to the Router ARP Table */
    rc = prvTgfIpRouterArpAddrWrite(prvTgfRouterArpIndex, &prvTgfArpMacAddrGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);
}

static GT_U32 ipv4UcAdded=0;
static GT_U32 ipv4McAdded=0;
static GT_U32 ipv6UcAdded=0;
static GT_U32 ipv6McAdded=0;

/**
* @internal prvTgfIpLpmHaAddBasicConfig function
* @endinternal
*
* @brief  Add some sequentially ipv4/ipv6 UC/MC Prefixes in
*         order manner. Verify that we get same number of
*         prefixes as expected
* @param[in] totalNumIpUc   - number of ipv4/ipv6 UC/MC
*                           entries to add
* @param[in] sendPortIndex  - port index to send traffic from
* @param[in] nextHopPortNum - port to send traffic to
* @param[in] firstCall      - GT_TRUE: callinf this
*                                      API for the first time
*                             GT_FALSE: callimg this
*                                      API for the second time
* @param[in] useBulk        - GT_TRUE: use bulk to add prefixes
*                             GT_FALSE: add prefixes one by one
*/
GT_VOID prvTgfIpLpmHaAddBasicConfig
(
    GT_U32  totalNumIpUc,
    GT_U8   sendPortIndex,
    GT_U32  nextHopPortNum,
    GT_BOOL firstCall,
    GT_BOOL useBulk
)
{
    GT_STATUS   rc=GT_OK;
    GT_IPADDR   ipAddr;
    GT_IPV6ADDR ipv6Addr;
    GT_U32      routeEntryBaseMemAddr;
    GT_U32      numOfPrefixesToAdd;
    GT_U32      numOfPrefixesAdded;
    GT_IPADDR   lastIpAddrAdded;
    GT_IPV6ADDR lastIpv6AddrAdded;
    GT_IPADDR   grpStartIpAddr;
    GT_IPADDR   srcStartIpAddr;
    GT_IPV6ADDR ipv6GrpStartIpAddr;
    GT_IPV6ADDR ipv6SrcStartIpAddr;
    GT_IPADDR   grpLastIpAddrAdded;
    GT_IPADDR   srcLastIpAddrAdded;
    GT_IPV6ADDR ipv6GrpLastIpAddrAdded;
    GT_IPV6ADDR ipv6SrcLastIpAddrAdded;

    GT_U32      numOfPrefixesAlreadyExist;
    GT_U32      numOfPrefixesNotAddedDueToOutOfPpMem;
    GT_U32      bulkTime;

    if (firstCall==GT_TRUE)
    {
        numOfPrefixesToAdd = totalNumIpUc;
    }
    else
    {
        /* use keept number of added prefixes for use in the replay phase
           we can not replay more then the original data - we will get a fail */
        numOfPrefixesToAdd = ipv4UcAdded;
    }

    prvTgfIpLpmHaBasicRoutingConfigurationSet(0,sendPortIndex,nextHopPortNum,firstCall);

    ipAddr.arIP[0]=0;
    ipAddr.arIP[1]=0;
    ipAddr.arIP[2]=0;
    ipAddr.arIP[3]=1;

    /* add 1000 prefixes - ipv4 */
    routeEntryBaseMemAddr=5;
    if (useBulk==GT_TRUE)
    {
        rc = prvTgfIpLpmIpv4UcPrefixAddManyByOctetBulk(prvTgfLpmDBId, 0, &ipAddr,
                                                   routeEntryBaseMemAddr,
                                                   numOfPrefixesToAdd,
                                                   3,2,1,0,
                                                   &lastIpAddrAdded,
                                                   &numOfPrefixesAdded,
                                                   &numOfPrefixesAlreadyExist,
                                                   &numOfPrefixesNotAddedDueToOutOfPpMem,
                                                   &bulkTime);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmIpv4UcPrefixAddManyByOctetBulk\n");
    }
    else
    {
        rc = prvTgfIpLpmIpv4UcPrefixAddManyByOctet(prvTgfLpmDBId, 0, &ipAddr,
                                                   routeEntryBaseMemAddr,
                                                   numOfPrefixesToAdd,
                                                   3,2,1,0,
                                                   &lastIpAddrAdded,
                                                   &numOfPrefixesAdded);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmIpv4UcPrefixAddManyByOctet\n");
    }

    UTF_VERIFY_EQUAL0_STRING_MAC(numOfPrefixesToAdd, numOfPrefixesAdded, "numOfPrefixesToAdd!numOfPrefixesAdded \n");
    ipv4UcAdded = numOfPrefixesAdded;

    cpssOsMemSet(&ipv6Addr.arIP,0,sizeof(ipv6Addr.arIP));
    ipv6Addr.arIP[15]=1;

    /* add 1000 prefixes - ipv6 */
    routeEntryBaseMemAddr=6;
    if (firstCall==GT_TRUE)
    {
        numOfPrefixesToAdd = totalNumIpUc;
    }
    else
    {
        /* use keept number of added prefixes for use in the replay phase
           we can not replay more then the original data - we will get a fail */
        numOfPrefixesToAdd = ipv6UcAdded;
    }
    if (useBulk==GT_TRUE)
    {
        rc=prvTgfIpLpmIpv6UcPrefixAddManyByOctetBulk(prvTgfLpmDBId,0,&ipv6Addr,
                                                   routeEntryBaseMemAddr,
                                                   numOfPrefixesToAdd,
                                                   15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,
                                                   &lastIpv6AddrAdded,
                                                   &numOfPrefixesAdded,
                                                   &numOfPrefixesAlreadyExist,
                                                   &numOfPrefixesNotAddedDueToOutOfPpMem,
                                                   &bulkTime);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmIpv6UcPrefixAddManyByOctetBulk\n");
    }
    else
    {
        rc=prvTgfIpLpmIpv6UcPrefixAddManyByOctet(prvTgfLpmDBId,0,&ipv6Addr,
                                                   routeEntryBaseMemAddr,
                                                   numOfPrefixesToAdd,
                                                   15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,
                                                   &lastIpv6AddrAdded,
                                                   &numOfPrefixesAdded);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmIpv6UcPrefixAddManyByOctet\n");
    }

    UTF_VERIFY_EQUAL0_STRING_MAC(numOfPrefixesToAdd, numOfPrefixesAdded, "numOfPrefixesToAdd!numOfPrefixesAdded \n");
    ipv6UcAdded = numOfPrefixesAdded;

    if (firstCall==GT_TRUE)
    {
        numOfPrefixesToAdd = totalNumIpUc;
    }
    else
    {
        /* use keept number of added prefixes for use in the replay phase
           we can not replay more then the original data - we will get a fail */
        numOfPrefixesToAdd = ipv4McAdded;
    }

    routeEntryBaseMemAddr=7;

    grpStartIpAddr.arIP[0]=224;
    grpStartIpAddr.arIP[1]=1;
    grpStartIpAddr.arIP[2]=1;
    grpStartIpAddr.arIP[3]=1;

    srcStartIpAddr.arIP[0]=0;
    srcStartIpAddr.arIP[1]=0;
    srcStartIpAddr.arIP[2]=0;
    srcStartIpAddr.arIP[3]=1;

    rc=prvTgfIpLpmIpv4McPrefixAddManyByOctet(prvTgfLpmDBId,0,&grpStartIpAddr,&srcStartIpAddr,
                                               routeEntryBaseMemAddr,
                                               numOfPrefixesToAdd,GT_FALSE,3,2,1,0,
                                               GT_TRUE,3,2,1,0,
                                               &grpLastIpAddrAdded,
                                               &srcLastIpAddrAdded,
                                               &numOfPrefixesAdded);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmIpv4McPrefixAddManyByOctet\n");
    UTF_VERIFY_EQUAL0_STRING_MAC(numOfPrefixesToAdd, numOfPrefixesAdded, "numOfPrefixesToAdd!numOfPrefixesAdded \n");

    ipv4McAdded = numOfPrefixesAdded;

    if (firstCall == GT_TRUE)
    {
        /* keep number of added prefixes for use in the replay phase
           we can not replay more then the original data - we will get a fail */
        numOfPrefixesToAdd = totalNumIpUc;
    }
    else
    {
        /* use keept number of added prefixes for use in the replay phase
           we can not replay more then the original data - we will get a fail */
        numOfPrefixesToAdd = ipv6McAdded;
    }

    routeEntryBaseMemAddr=8;
    cpssOsMemSet(&ipv6GrpStartIpAddr.arIP,0,sizeof(ipv6GrpStartIpAddr.arIP));
    cpssOsMemSet(&ipv6SrcStartIpAddr.arIP,0,sizeof(ipv6SrcStartIpAddr.arIP));

    ipv6GrpStartIpAddr.arIP[0]=0xFF;
    ipv6GrpStartIpAddr.arIP[1]=0xFF;
    ipv6GrpStartIpAddr.arIP[14]=0x22;
    ipv6GrpStartIpAddr.arIP[15]=0x22;

    ipv6SrcStartIpAddr.arIP[15]=1;
    rc=prvTgfIpLpmIpv6McPrefixAddManyByOctet(prvTgfLpmDBId,0,&ipv6GrpStartIpAddr,&ipv6SrcStartIpAddr,
                                               routeEntryBaseMemAddr,
                                               numOfPrefixesToAdd,GT_FALSE,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,
                                               GT_TRUE,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,
                                               &ipv6GrpLastIpAddrAdded,
                                               &ipv6SrcLastIpAddrAdded,
                                               &numOfPrefixesAdded);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmIpv6McPrefixAddManyByOctet\n");
    UTF_VERIFY_EQUAL0_STRING_MAC(numOfPrefixesToAdd, numOfPrefixesAdded, "numOfPrefixesToAdd!numOfPrefixesAdded \n");
    ipv6McAdded = numOfPrefixesAdded;

    /* print the memory
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrint(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmDbgHwOctetPerProtocolPrint\n");
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters\n");*/
}

/**
* @internal prvTgfIpLpmHaBulkAddBasicConfig function
* @endinternal
*
* @brief  Add some sequentially ipv4/ipv6 UC/MC Prefixes in
*         order manner. Verify that we get same number of
*         prefixes as expected
* @param[in] totalNumIpUc   - number of ipv4/ipv6 UC/MC
*                           entries to add
* @param[in] sendPortIndex  - port index to send traffic from
* @param[in] nextHopPortNum - port to send traffic to
* @param[in] firstCall      - GT_TRUE: callinf this
*                                      API for the first time
*                             GT_FALSE: callimg this
*                                      API for the second time
* @param[in] useBulk        - GT_TRUE: use bulk to add prefixes
*                             GT_FALSE: add prefixes one by one
*/
GT_VOID prvTgfIpLpmHaBulkAddBasicConfig
(
    GT_U32  totalNumIpUc,
    GT_U8   sendPortIndex,
    GT_U32  nextHopPortNum,
    GT_BOOL firstCall,
    GT_BOOL useBulk
)
{
    GT_STATUS   rc=GT_OK;
    GT_IPADDR   ipAddr;
    GT_IPV6ADDR ipv6Addr;
    GT_U32      routeEntryBaseMemAddr;
    GT_U32      numOfPrefixesToAdd;
    GT_U32      numOfPrefixesAdded;
    GT_IPADDR   lastIpAddrAdded;
    GT_IPV6ADDR lastIpv6AddrAdded;
    GT_U32      numOfPrefixesAlreadyExist;
    GT_U32      numOfPrefixesNotAddedDueToOutOfPpMem;
    GT_U32      bulkTime;

    if (firstCall==GT_TRUE)
    {
        numOfPrefixesToAdd = totalNumIpUc;
    }
    else
    {
        /* use keept number of added prefixes for use in the replay phase
           we can not replay more then the original data - we will get a fail */
        numOfPrefixesToAdd = ipv4UcAdded;
    }

    prvTgfIpLpmHaBasicRoutingConfigurationSet(0,sendPortIndex,nextHopPortNum,firstCall);

    ipAddr.arIP[0]=0;
    ipAddr.arIP[1]=0;
    ipAddr.arIP[2]=0;
    ipAddr.arIP[3]=1;

    /* add 1000 prefixes - ipv4 */
    routeEntryBaseMemAddr=5;
    if (useBulk==GT_TRUE)
    {
        rc = prvTgfIpLpmIpv4UcPrefixAddManyByOctetBulk(prvTgfLpmDBId, 0, &ipAddr,
                                                   routeEntryBaseMemAddr,
                                                   numOfPrefixesToAdd,
                                                   3,2,1,0,
                                                   &lastIpAddrAdded,
                                                   &numOfPrefixesAdded,
                                                   &numOfPrefixesAlreadyExist,
                                                   &numOfPrefixesNotAddedDueToOutOfPpMem,
                                                   &bulkTime);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmIpv4UcPrefixAddManyByOctetBulk\n");
    }
    else
    {
        rc = prvTgfIpLpmIpv4UcPrefixAddManyByOctet(prvTgfLpmDBId, 0, &ipAddr,
                                                   routeEntryBaseMemAddr,
                                                   numOfPrefixesToAdd,
                                                   3,2,1,0,
                                                   &lastIpAddrAdded,
                                                   &numOfPrefixesAdded);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmIpv4UcPrefixAddManyByOctet\n");
    }

    UTF_VERIFY_EQUAL0_STRING_MAC(numOfPrefixesToAdd, numOfPrefixesAdded, "numOfPrefixesToAdd!numOfPrefixesAdded \n");
    ipv4UcAdded = numOfPrefixesAdded;

    cpssOsMemSet(&ipv6Addr.arIP,0,sizeof(ipv6Addr.arIP));
    ipv6Addr.arIP[15]=1;

    /* add 1000 prefixes - ipv6 */
    routeEntryBaseMemAddr=6;
    if (firstCall==GT_TRUE)
    {
        numOfPrefixesToAdd = totalNumIpUc;
    }
    else
    {
        /* use keept number of added prefixes for use in the replay phase
           we can not replay more then the original data - we will get a fail */
        numOfPrefixesToAdd = ipv6UcAdded;
    }
    if (useBulk==GT_TRUE)
    {
        rc=prvTgfIpLpmIpv6UcPrefixAddManyByOctetBulk(prvTgfLpmDBId,0,&ipv6Addr,
                                                   routeEntryBaseMemAddr,
                                                   numOfPrefixesToAdd,
                                                   15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,
                                                   &lastIpv6AddrAdded,
                                                   &numOfPrefixesAdded,
                                                   &numOfPrefixesAlreadyExist,
                                                   &numOfPrefixesNotAddedDueToOutOfPpMem,
                                                   &bulkTime);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmIpv6UcPrefixAddManyByOctetBulk\n");
    }
    else
    {
        rc=prvTgfIpLpmIpv6UcPrefixAddManyByOctet(prvTgfLpmDBId,0,&ipv6Addr,
                                                   routeEntryBaseMemAddr,
                                                   numOfPrefixesToAdd,
                                                   15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,
                                                   &lastIpv6AddrAdded,
                                                   &numOfPrefixesAdded);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmIpv6UcPrefixAddManyByOctet\n");
    }

    UTF_VERIFY_EQUAL0_STRING_MAC(numOfPrefixesToAdd, numOfPrefixesAdded, "numOfPrefixesToAdd!numOfPrefixesAdded \n");
    ipv6UcAdded = numOfPrefixesAdded;

    /* print the memory
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrint(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmDbgHwOctetPerProtocolPrint\n");
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters\n");*/
}

/**
* @internal prvTgfIpLpmHaSystemRecoveryStateSet function
* @endinternal
*
* @brief  Keep LPM DB values for reconstruct and
*       set flag for HA process.
*/
GT_VOID prvTgfIpLpmHaSystemRecoveryStateSet()
{
    GT_STATUS   rc=GT_OK;
    CPSS_SYSTEM_RECOVERY_INFO_STC               tempSystemRecovery_Info;

    cpssOsMemSet(&tempSystemRecovery_Info, 0, sizeof(CPSS_SYSTEM_RECOVERY_INFO_STC));
    tempSystemRecovery_Info.systemRecoveryProcess=CPSS_SYSTEM_RECOVERY_PROCESS_HA_E;
    tempSystemRecovery_Info.systemRecoveryState=CPSS_SYSTEM_RECOVERY_INIT_STATE_E;

    /* get LPM DB config */
    rc = prvTgfIpLpmRamDBConfigGet(prvTgfLpmDBId,&shadowTypeParam,&protocolStackParam,&ramDbCfgParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmDBConfigGet\n");

    /* retrieves configuration of the Virtual Router */
    rc = prvTgfIpLpmVirtualRouterGet(prvTgfLpmDBId, 0,
                                     &(prvTgfDefLpmDBVrCfg.isSupportIpv4Uc),
                                     &(prvTgfDefLpmDBVrCfg.defIpv4UcNextHopInfo),
                                     &(prvTgfDefLpmDBVrCfg.isSupportIpv6Uc),
                                     &(prvTgfDefLpmDBVrCfg.defIpv6UcNextHopInfo),
                                     &(prvTgfDefLpmDBVrCfg.isSupportIpv4Mc),
                                     &(prvTgfDefLpmDBVrCfg.defIpv4McRouteLttEntry),
                                     &(prvTgfDefLpmDBVrCfg.isSupportIpv6Mc),
                                     &(prvTgfDefLpmDBVrCfg.defIpv6McRouteLttEntry));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmVirtualRouterGet\n");

    /* Get device list array from skip list for given LMP DB id */
    numOfDevsParam = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = prvTgfIpLpmDBDevListGet(prvTgfLpmDBId, &numOfDevsParam, devListArray);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmDBDevListGet\n");

    rc=cpssSystemRecoveryStateSet(&tempSystemRecovery_Info);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on cpssSystemRecoveryStateSet\n");
}

/**
* @internal prvTgfIpLpmHaDelBasicConfig function
* @endinternal
*
* @brief  Delete some sequentially ipv4/ipv6 UC/MC Prefixes in
*         order manner. Verify that we delete same number of
*         prefixes as expected
* @param[in] totalNumIpUc - number of ipv4/ipv6 UC/MC entries
*                           to delete
*/
GT_VOID prvTgfIpLpmHaDelBasicConfig
(
    GT_U32                      totalNumIpUc
)
{
    GT_STATUS   rc=GT_OK;
    GT_IPADDR   ipAddr;
    GT_IPV6ADDR ipv6Addr;
    GT_U32      numOfPrefixesToDel;
    GT_U32      numOfPrefixesDeleted;
    GT_IPADDR   lastIpAddrDeleted;
    GT_IPV6ADDR lastIpv6AddrDeleted;
    GT_IPADDR   grpStartIpAddr;
    GT_IPADDR   srcStartIpAddr;
    GT_IPV6ADDR ipv6GrpStartIpAddr;
    GT_IPV6ADDR ipv6SrcStartIpAddr;
    GT_IPADDR   grpLastIpAddrDeleted;
    GT_IPADDR   srcLastIpAddrDeleted;
    GT_IPV6ADDR ipv6GrpLastIpAddrDeleted;
    GT_IPV6ADDR ipv6SrcLastIpAddrDeleted;

    ipAddr.arIP[0]=0;
    ipAddr.arIP[1]=0;
    ipAddr.arIP[2]=0;
    ipAddr.arIP[3]=1;

    /* delete all prefixes
       delete VR
       delete LPM DB */
    numOfPrefixesToDel=totalNumIpUc;

    rc=prvTgfIpLpmIpv4UcPrefixDelManyByOctet(prvTgfLpmDBId,0,&ipAddr,
                                                numOfPrefixesToDel,
                                                3,2,1,0,
                                                &lastIpAddrDeleted,
                                                &numOfPrefixesDeleted);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmIpv4UcPrefixDelManyByOctet\n");
    UTF_VERIFY_EQUAL0_STRING_MAC(numOfPrefixesToDel, numOfPrefixesDeleted, "numOfPrefixesToDel!numOfPrefixesDeleted \n");

    cpssOsMemSet(&ipv6Addr.arIP,0,sizeof(ipv6Addr.arIP));
    ipv6Addr.arIP[15]=1;

    /* delete 1000 prefixes - ipv6 */
    numOfPrefixesToDel=totalNumIpUc;
    rc=prvTgfIpLpmIpv6UcPrefixDelManyByOctet(prvTgfLpmDBId,0,&ipv6Addr,
                                                  numOfPrefixesToDel,
                                                  15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,
                                                  &lastIpv6AddrDeleted,
                                                  &numOfPrefixesDeleted);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmIpv6UcPrefixDelManyByOctet\n");
    UTF_VERIFY_EQUAL0_STRING_MAC(numOfPrefixesToDel, numOfPrefixesDeleted, "numOfPrefixesToDel!numOfPrefixesDeleted \n");

    grpStartIpAddr.arIP[0]=224;
    grpStartIpAddr.arIP[1]=1;
    grpStartIpAddr.arIP[2]=1;
    grpStartIpAddr.arIP[3]=1;

    srcStartIpAddr.arIP[0]=0;
    srcStartIpAddr.arIP[1]=0;
    srcStartIpAddr.arIP[2]=0;
    srcStartIpAddr.arIP[3]=1;
    rc=prvTgfIpLpmIpv4McPrefixDelManyByOctet(prvTgfLpmDBId,0,&grpStartIpAddr,&srcStartIpAddr,
                                                  numOfPrefixesToDel,GT_FALSE,3,2,1,0,
                                                  GT_TRUE,3,2,1,0,
                                                  &grpLastIpAddrDeleted,
                                                  &srcLastIpAddrDeleted,
                                                  &numOfPrefixesDeleted);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmIpv4McPrefixDelManyByOctet\n");
    UTF_VERIFY_EQUAL0_STRING_MAC(numOfPrefixesToDel, numOfPrefixesDeleted, "numOfPrefixesToDel!numOfPrefixesDeleted \n");

    cpssOsMemSet(&ipv6GrpStartIpAddr.arIP,0,sizeof(ipv6GrpStartIpAddr.arIP));
    cpssOsMemSet(&ipv6SrcStartIpAddr.arIP,0,sizeof(ipv6SrcStartIpAddr.arIP));

    ipv6GrpStartIpAddr.arIP[0]=0xFF;
    ipv6GrpStartIpAddr.arIP[1]=0xFF;
    ipv6GrpStartIpAddr.arIP[14]=0x22;
    ipv6GrpStartIpAddr.arIP[15]=0x22;

    ipv6SrcStartIpAddr.arIP[15]=1;
    rc=prvTgfIpLpmIpv6McPrefixDelManyByOctet(prvTgfLpmDBId,0,&ipv6GrpStartIpAddr,&ipv6SrcStartIpAddr,
                                              numOfPrefixesToDel,GT_FALSE,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,
                                              GT_TRUE,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,
                                              &ipv6GrpLastIpAddrDeleted,
                                              &ipv6SrcLastIpAddrDeleted,
                                              &numOfPrefixesDeleted);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmIpv6McPrefixDelManyByOctet\n");
    UTF_VERIFY_EQUAL0_STRING_MAC(numOfPrefixesToDel, numOfPrefixesDeleted, "numOfPrefixesToDel!numOfPrefixesDeleted \n");

    /* delete virtual router */
    rc=prvTgfIpLpmVirtualRouterDel(prvTgfLpmDBId, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmVirtualRouterDel\n");

    /* Delete device list array for given LMP DB id */
    rc = prvTgfIpLpmDBDevsListRemove(prvTgfLpmDBId, devListArray , numOfDevsParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmDBDevsListRemove\n");

    rc=prvTgfIpLpmDBDelete(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmDBDelete\n");
}

/**
* @internal prvTgfIpLpmHaBulkDelBasicConfig function
* @endinternal
*
* @brief  Delete some sequentially ipv4/ipv6 UC Prefixes in
*         order manner. Verify that we delete same number of
*         prefixes as expected
* @param[in] totalNumIpUc - number of ipv4/ipv6 UC entries
*                           to delete
*/
GT_VOID prvTgfIpLpmHaBulkDelBasicConfig
(
    GT_U32                      totalNumIpUc
)
{
    GT_STATUS   rc=GT_OK;
    GT_IPADDR   ipAddr;
    GT_IPV6ADDR ipv6Addr;
    GT_U32      numOfPrefixesToDel;
    GT_U32      numOfPrefixesDeleted;
    GT_IPADDR   lastIpAddrDeleted;
    GT_IPV6ADDR lastIpv6AddrDeleted;

    ipAddr.arIP[0]=0;
    ipAddr.arIP[1]=0;
    ipAddr.arIP[2]=0;
    ipAddr.arIP[3]=1;

    /* delete all prefixes
       delete VR
       delete LPM DB */
    numOfPrefixesToDel=totalNumIpUc;

    rc=prvTgfIpLpmIpv4UcPrefixDelManyByOctet(prvTgfLpmDBId,0,&ipAddr,
                                                numOfPrefixesToDel,
                                                3,2,1,0,
                                                &lastIpAddrDeleted,
                                                &numOfPrefixesDeleted);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmIpv4UcPrefixDelManyByOctet\n");
    UTF_VERIFY_EQUAL0_STRING_MAC(numOfPrefixesToDel, numOfPrefixesDeleted, "numOfPrefixesToDel!numOfPrefixesDeleted \n");

    cpssOsMemSet(&ipv6Addr.arIP,0,sizeof(ipv6Addr.arIP));
    ipv6Addr.arIP[15]=1;

    /* delete 1000 prefixes - ipv6 */
    numOfPrefixesToDel=totalNumIpUc;
    rc=prvTgfIpLpmIpv6UcPrefixDelManyByOctet(prvTgfLpmDBId,0,&ipv6Addr,
                                                  numOfPrefixesToDel,
                                                  15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,
                                                  &lastIpv6AddrDeleted,
                                                  &numOfPrefixesDeleted);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmIpv6UcPrefixDelManyByOctet\n");
    UTF_VERIFY_EQUAL0_STRING_MAC(numOfPrefixesToDel, numOfPrefixesDeleted, "numOfPrefixesToDel!numOfPrefixesDeleted \n");

    /* delete virtual router */
    rc=prvTgfIpLpmVirtualRouterDel(prvTgfLpmDBId, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmVirtualRouterDel\n");

    /* Delete device list array for given LMP DB id */
    rc = prvTgfIpLpmDBDevsListRemove(prvTgfLpmDBId, devListArray , numOfDevsParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmDBDevsListRemove\n");

    rc=prvTgfIpLpmDBDelete(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmDBDelete\n");
}

/**
* @internal prvTgfIpLpmHaAddConfigAfterDelete function
* @endinternal
*
* @brief  Reconstruct LPM DB values after delete
*/
GT_VOID prvTgfIpLpmHaAddConfigAfterDelete()
{
    GT_STATUS   rc=GT_OK;

   /* create LPM DB, Add device list and Add VR */
    rc = prvTgfIpLpmRamDBCreate(prvTgfLpmDBId,protocolStackParam,&ramDbCfgParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmRamDBCreate\n");

    rc = prvTgfIpLpmDBDevListAdd(prvTgfLpmDBId, devListArray, numOfDevsParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmDBDevListAdd\n");

    rc=prvTgfIpLpmVirtualRouterAdd(prvTgfLpmDBId, 0,
                                   &(prvTgfDefLpmDBVrCfg.defIpv4UcNextHopInfo),
                                   &(prvTgfDefLpmDBVrCfg.defIpv6UcNextHopInfo),
                                   &(prvTgfDefLpmDBVrCfg.defIpv4McRouteLttEntry),
                                   &(prvTgfDefLpmDBVrCfg.defIpv6McRouteLttEntry),
                                   NULL);/* FCOE not supported for SIP6 */
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmVirtualRouterAdd\n");
}

/**
* @internal prvTgfIpLpmHaSyncSwHw function
* @endinternal
*
* @brief  Update HW address with SW shadow and Sync SW DB from
*         HW data.
*/
GT_VOID prvTgfIpLpmHaSyncSwHw()
{
    GT_STATUS   rc=GT_OK;
    CPSS_SYSTEM_RECOVERY_INFO_STC               tempSystemRecovery_Info;

    /* call function to sync shadow and HW */
    rc = prvTgfLpmRamSyncSwHwForHa();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfLpmRamSyncSwHwForHa\n");

    /*rc = prvTgfIpLpmDbgHwOctetPerProtocolPrint(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmDbgHwOctetPerProtocolPrint\n");

    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters\n");*/

    /* set init flag */
    cpssOsMemSet(&tempSystemRecovery_Info, 0, sizeof(CPSS_SYSTEM_RECOVERY_INFO_STC));
    tempSystemRecovery_Info.systemRecoveryProcess=CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E;
    tempSystemRecovery_Info.systemRecoveryState=CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E;

    rc=cpssSystemRecoveryStateSet(&tempSystemRecovery_Info);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on cpssSystemRecoveryStateSet\n");

}

static GT_VOID prvTgfIpLpmHaTrafficGenerate
(
    GT_U8       sendPortIndex,
    GT_U32      nextHopPortNum,
    GT_IPADDR   *ipAddr,
    GT_IPADDR   *grpStartIpAddr,
    GT_IPADDR   *srcStartIpAddr,
    GT_IPV6ADDR *ipv6Addr,
    GT_IPV6ADDR *ipv6GrpStartIpAddr,
    GT_IPV6ADDR *ipv6SrcStartIpAddr
)
{
    GT_STATUS rc = GT_OK;

    GT_U32                          partsCount  = 0;
    GT_U32                          packetSize  = 0;
    TGF_PACKET_PART_STC             *packetPartsPtr;
    TGF_PACKET_STC                  packetInfo;
    GT_U32                          portIter    = 0;
    TGF_VFD_INFO_STC                vfdArray[1];
    CPSS_INTERFACE_INFO_STC         portInterface;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   expectedCntrs;
    PRV_TGF_IP_COUNTER_SET_STC      ipCounters;
    GT_U32      numVfd = 0;/* number of VFDs in vfdArray */
    GT_U8           packetBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
    GT_U32          buffLen  = TGF_RX_BUFFER_MAX_SIZE_CNS;
    GT_U32          packetActualLength = 0;
    GT_U8           devNum;
    GT_U8           queue;
    TGF_NET_DSA_STC rxParam;
    GT_U32          i=0;

    /* four loops of the same code: ipv4 UC, Ipv4 MC, IPv6 UC, ipv6 MC*/
    for (i=0;i<4;i++)
    {
        for (portIter = 0; portIter < 4; portIter++)
        {
            rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
            rc = prvTgfCountersIpSet(prvTgfPortsArray[portIter], portIter);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCountersIpSet: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
        }

        /* setup nexthope portInterface for capturing */
        portInterface.type              = CPSS_INTERFACE_PORT_E;
        portInterface.devPort.hwDevNum  = prvTgfDevNum;
        portInterface.devPort.portNum   = nextHopPortNum;

        /* enable capture on next hop port/trunk */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, captureType, GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d\n",
                                     prvTgfDevNum, captureType);

        /* clear capturing RxPcktTable */
        rc = tgfTrafficTableRxPcktTblClear();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

        switch (i)
        {
        case 0:
            /* ipv4 UC packet 0.0.1.17 */
            cpssOsMemCpy(prvTgfPacketPartIpv4.dstAddr, ipAddr->arIP, sizeof(ipAddr->arIP));
            partsCount = sizeof(prvTgfPacketPartArray) / sizeof(prvTgfPacketPartArray[0]);
            packetPartsPtr = prvTgfPacketPartArray;
            break;
        case 1:
            /* ipv4 MC packet 224.1.1.1 0.0.2.34 */
            cpssOsMemCpy(prvTgfPacketPart_ipv4Mc.dstAddr, grpStartIpAddr->arIP, sizeof(grpStartIpAddr->arIP));
            cpssOsMemCpy(prvTgfPacketPart_ipv4Mc.srcAddr, srcStartIpAddr->arIP, sizeof(srcStartIpAddr->arIP));
            partsCount = sizeof(prvTgfPacketPartArray_ipv4Mc) / sizeof(prvTgfPacketPartArray_ipv4Mc[0]);
            packetPartsPtr = prvTgfPacketPartArray_ipv4Mc;
            break;
        case 2:
            /* ipv6 UC 0.0.0.0.0.0.0.0.0.0.0.0.0.0.3.51 */
            prvTgfPacketPartIpv6.dstAddr[0] = ipv6Addr->arIP[1]<<8 | ipv6Addr->arIP[0];
            prvTgfPacketPartIpv6.dstAddr[1] = ipv6Addr->arIP[2]<<8 | ipv6Addr->arIP[3];
            prvTgfPacketPartIpv6.dstAddr[2] = ipv6Addr->arIP[4]<<8 | ipv6Addr->arIP[5];
            prvTgfPacketPartIpv6.dstAddr[3] = ipv6Addr->arIP[6]<<8 | ipv6Addr->arIP[6];
            prvTgfPacketPartIpv6.dstAddr[4] = ipv6Addr->arIP[8]<<8 | ipv6Addr->arIP[9];
            prvTgfPacketPartIpv6.dstAddr[5] = ipv6Addr->arIP[10]<<8 | ipv6Addr->arIP[11];
            prvTgfPacketPartIpv6.dstAddr[6] = ipv6Addr->arIP[12]<<8 | ipv6Addr->arIP[12];
            prvTgfPacketPartIpv6.dstAddr[7] = ipv6Addr->arIP[14]<<8 | ipv6Addr->arIP[15];
            partsCount = sizeof(prvTgfPacketPartArrayIpv6) / sizeof(prvTgfPacketPartArrayIpv6[0]);
            packetPartsPtr = prvTgfPacketPartArrayIpv6;
            break;
        case 3:
            /* ipv6 MC  */
            prvTgfPacketPart_ipv6Mc.dstAddr[0] = ipv6GrpStartIpAddr->arIP[1]<<8 | ipv6GrpStartIpAddr->arIP[0];
            prvTgfPacketPart_ipv6Mc.dstAddr[1] = ipv6GrpStartIpAddr->arIP[2]<<8 | ipv6GrpStartIpAddr->arIP[3];
            prvTgfPacketPart_ipv6Mc.dstAddr[2] = ipv6GrpStartIpAddr->arIP[4]<<8 | ipv6GrpStartIpAddr->arIP[5];
            prvTgfPacketPart_ipv6Mc.dstAddr[3] = ipv6GrpStartIpAddr->arIP[6]<<8 | ipv6GrpStartIpAddr->arIP[6];
            prvTgfPacketPart_ipv6Mc.dstAddr[4] = ipv6GrpStartIpAddr->arIP[8]<<8 | ipv6GrpStartIpAddr->arIP[9];
            prvTgfPacketPart_ipv6Mc.dstAddr[5] = ipv6GrpStartIpAddr->arIP[10]<<8 | ipv6GrpStartIpAddr->arIP[11];
            prvTgfPacketPart_ipv6Mc.dstAddr[6] = ipv6GrpStartIpAddr->arIP[12]<<8 | ipv6GrpStartIpAddr->arIP[12];
            prvTgfPacketPart_ipv6Mc.dstAddr[7] = ipv6GrpStartIpAddr->arIP[14]<<8 | ipv6GrpStartIpAddr->arIP[15];

            prvTgfPacketPart_ipv6Mc.srcAddr[0] = ipv6SrcStartIpAddr->arIP[1]<<8 | ipv6SrcStartIpAddr->arIP[0];
            prvTgfPacketPart_ipv6Mc.srcAddr[1] = ipv6SrcStartIpAddr->arIP[2]<<8 | ipv6SrcStartIpAddr->arIP[3];
            prvTgfPacketPart_ipv6Mc.srcAddr[2] = ipv6SrcStartIpAddr->arIP[4]<<8 | ipv6SrcStartIpAddr->arIP[5];
            prvTgfPacketPart_ipv6Mc.srcAddr[3] = ipv6SrcStartIpAddr->arIP[6]<<8 | ipv6SrcStartIpAddr->arIP[6];
            prvTgfPacketPart_ipv6Mc.srcAddr[4] = ipv6SrcStartIpAddr->arIP[8]<<8 | ipv6SrcStartIpAddr->arIP[9];
            prvTgfPacketPart_ipv6Mc.srcAddr[5] = ipv6SrcStartIpAddr->arIP[10]<<8 | ipv6SrcStartIpAddr->arIP[11];
            prvTgfPacketPart_ipv6Mc.srcAddr[6] = ipv6SrcStartIpAddr->arIP[12]<<8 | ipv6SrcStartIpAddr->arIP[12];
            prvTgfPacketPart_ipv6Mc.srcAddr[7] = ipv6SrcStartIpAddr->arIP[14]<<8 | ipv6SrcStartIpAddr->arIP[15];

            partsCount = sizeof(prvTgfPacketPartArray_ipv6Mc) / sizeof(prvTgfPacketPartArray_ipv6Mc[0]);
            packetPartsPtr = prvTgfPacketPartArray_ipv6Mc;
            break;
        default:
            break;
        }

        /* calculate packet size */
        rc = prvTgfPacketSizeGet(packetPartsPtr, partsCount, &packetSize);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet:");

        /* build packet */
        packetInfo.totalLen   = packetSize;
        packetInfo.numOfParts = partsCount;
        packetInfo.partsArray = packetPartsPtr;

        prvTgfBurstCount = 1;
        /* clear VFD array */
        cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));
        numVfd = 0;

        /* setup packet */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, prvTgfBurstCount, numVfd, &vfdArray[0]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

        /* set transmit timings */
        rc = prvTgfSetTxSetup2Eth(1, 1);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetup2Eth: %d", prvTgfDevNum);

        /* AUTODOC: send packet from port 0 with: */

        /* send packet -- send from specific port -- even if member of a trunk */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[sendPortIndex]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d\n",
                                     prvTgfDevNum, prvTgfPortsArray[sendPortIndex]);
        cpssOsTimerWkAfter(1000);

        /* read and check ethernet counters */
        for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
        {
            GT_BOOL isOk;

            /* read counters */
            rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d\n",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);

            if (i==0||i==2)/* uc packets */
            {
                if ((portIter == 3) || (portIter == 0))
                {
                    /* packetSize is not changed */
                        expectedCntrs.goodOctetsSent.l[0] = (packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                        expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
                        expectedCntrs.ucPktsSent.l[0] = prvTgfBurstCount;
                        expectedCntrs.mcPktsSent.l[0] = 0;
                        expectedCntrs.ucPktsRcv.l[0]  = prvTgfBurstCount;
                        expectedCntrs.mcPktsRcv.l[0]  = 0;
                        expectedCntrs.brdcPktsSent.l[0]   = 0;
                        expectedCntrs.goodOctetsRcv.l[0]  = (packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                        expectedCntrs.goodPktsRcv.l[0]    = prvTgfBurstCount;
                        expectedCntrs.brdcPktsRcv.l[0]    = 0;

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
            }
            else /* MC packets */
            {
                /* packet send from port0 to mll (port2,3) and it is flooded in vlan 5 */
                if ((portIter == 0)||(portIter == 3))
                {
                    /* packetSize is not changed */
                        expectedCntrs.goodOctetsSent.l[0] = (packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                        expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
                        expectedCntrs.ucPktsSent.l[0] = 0;
                        expectedCntrs.mcPktsSent.l[0] = prvTgfBurstCount;
                        expectedCntrs.ucPktsRcv.l[0]  = 0;
                        expectedCntrs.mcPktsRcv.l[0]  = prvTgfBurstCount;
                        expectedCntrs.brdcPktsSent.l[0]   = 0;
                        expectedCntrs.goodOctetsRcv.l[0]  = (packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                        expectedCntrs.goodPktsRcv.l[0]    = prvTgfBurstCount;
                        expectedCntrs.brdcPktsRcv.l[0]    = 0;

                }
                else/* ((portIter == 1)||(portIter == 2)) */
                {
                    /* packetSize is not changed */
                    expectedCntrs.goodOctetsSent.l[0] = (packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                    expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
                    expectedCntrs.ucPktsSent.l[0] = 0;
                    expectedCntrs.mcPktsSent.l[0] = prvTgfBurstCount;
                    expectedCntrs.ucPktsRcv.l[0]  = 0;
                    expectedCntrs.mcPktsRcv.l[0]  = 0;
                    expectedCntrs.brdcPktsSent.l[0]   = 0;
                    expectedCntrs.goodOctetsRcv.l[0]  = 0;
                    expectedCntrs.goodPktsRcv.l[0]    = 0;
                    expectedCntrs.brdcPktsRcv.l[0]    = 0;
                }
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

        rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_CAPTURE_E,
                                           GT_TRUE, GT_TRUE, packetBuff,
                                           &buffLen, &packetActualLength,
                                           &devNum, &queue, &rxParam);
        PRV_UTF_LOG2_MAC(" ************************************\nCpuCode = %d \n CmdIsToCpu = %d \n ************************************ \n",
                                rxParam.cpuCode, rxParam.dsaCmdIsToCpu);

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

        /* disable capture on nexthope port , before check the packets to the CPU */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, captureType, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d\n",
                                     prvTgfDevNum, captureType);
    }
}

/**
* @internal prvTgfIpLpmHaValidityCheckAndTrafficSend function
* @endinternal
*
* @brief  Check HW and SW validity and send traffic to some
*         predifined entries.
*
* @param[in] sendPortIndex  - port to send traffic from
* @param[in] nextHopPortNum - port to send traffic to
*/
GT_VOID prvTgfIpLpmHaValidityCheckAndTrafficSend
(
    GT_U8   sendPortIndex,
    GT_U32  nextHopPortNum
)
{
    GT_STATUS   rc=GT_OK;
    GT_IPADDR   ipAddr;
    GT_IPV6ADDR ipv6Addr;
    GT_IPADDR   grpStartIpAddr;
    GT_IPADDR   srcStartIpAddr;
    GT_IPV6ADDR ipv6GrpStartIpAddr;
    GT_IPV6ADDR ipv6SrcStartIpAddr;

    /* for SIP5 we have a stack issue overflow - after the problem will be fixed we can return this check */
    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        rc = prvTgfIpLpmValidityCheck(prvTgfLpmDBId, 0, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E, GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmValidityCheck ipv4\n");

        rc = prvTgfIpLpmValidityCheck(prvTgfLpmDBId, 0, CPSS_IP_PROTOCOL_IPV6_E, CPSS_UNICAST_E,GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmValidityCheck ipv6 \n");

        rc = prvTgfIpLpmValidityCheck(prvTgfLpmDBId, 0, CPSS_IP_PROTOCOL_IPV4_E, CPSS_MULTICAST_E,GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmValidityCheck ipv4\n");

        rc = prvTgfIpLpmValidityCheck(prvTgfLpmDBId, 0, CPSS_IP_PROTOCOL_IPV6_E, CPSS_MULTICAST_E,GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmValidityCheck ipv6 \n");
    }
    /* send ipv4 UC packet */
    ipAddr.arIP[0]=0;
    ipAddr.arIP[1]=0;
    ipAddr.arIP[2]=1;
    ipAddr.arIP[3]=0x11;

    /* send ipv4 MC packet */
    grpStartIpAddr.arIP[0]=224;
    grpStartIpAddr.arIP[1]=1;
    grpStartIpAddr.arIP[2]=1;
    grpStartIpAddr.arIP[3]=1;

    srcStartIpAddr.arIP[0]=0;
    srcStartIpAddr.arIP[1]=0;
    srcStartIpAddr.arIP[2]=2;
    srcStartIpAddr.arIP[3]=0x22;

    /* send ipv6 UC packet */
    cpssOsMemSet(&ipv6Addr.arIP,0,sizeof(ipv6Addr.arIP));
    ipv6Addr.arIP[14]=3;
    ipv6Addr.arIP[15]=0x33;

    /* send ipv6 MC packet */
    cpssOsMemSet(&ipv6GrpStartIpAddr.arIP,0,sizeof(ipv6GrpStartIpAddr.arIP));
    cpssOsMemSet(&ipv6SrcStartIpAddr.arIP,0,sizeof(ipv6SrcStartIpAddr.arIP));

    ipv6GrpStartIpAddr.arIP[0]=0xFF;
    ipv6GrpStartIpAddr.arIP[1]=0xFF;
    ipv6GrpStartIpAddr.arIP[14]=0x22;
    ipv6GrpStartIpAddr.arIP[15]=0x22;

    ipv6SrcStartIpAddr.arIP[14]=0x03;
    ipv6SrcStartIpAddr.arIP[15]=0xB6;

    prvTgfIpLpmHaTrafficGenerate(sendPortIndex,nextHopPortNum,&ipAddr,
                                 &grpStartIpAddr,&srcStartIpAddr,&ipv6Addr,
                                 &ipv6GrpStartIpAddr,&ipv6SrcStartIpAddr);
}

static GT_VOID prvTgfIpLpmHaBulkTrafficGenerate
(
    GT_U8       sendPortIndex,
    GT_U32      nextHopPortNum,
    GT_IPADDR   *ipAddr,
    GT_IPV6ADDR *ipv6Addr
)
{
    GT_STATUS rc = GT_OK;

    GT_U32                          partsCount  = 0;
    GT_U32                          packetSize  = 0;
    TGF_PACKET_PART_STC             *packetPartsPtr;
    TGF_PACKET_STC                  packetInfo;
    GT_U32                          portIter    = 0;
    TGF_VFD_INFO_STC                vfdArray[1];
    CPSS_INTERFACE_INFO_STC         portInterface;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   expectedCntrs;
    PRV_TGF_IP_COUNTER_SET_STC      ipCounters;
    GT_U32      numVfd = 0;/* number of VFDs in vfdArray */
    GT_U8           packetBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
    GT_U32          buffLen  = TGF_RX_BUFFER_MAX_SIZE_CNS;
    GT_U32          packetActualLength = 0;
    GT_U8           devNum;
    GT_U8           queue;
    TGF_NET_DSA_STC rxParam;
    GT_U32          i=0;

    /* four loops of the same code: ipv4 UC, IPv6 UC */
    for (i=0;i<2;i++)
    {
        for (portIter = 0; portIter < 4; portIter++)
        {
            rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
            rc = prvTgfCountersIpSet(prvTgfPortsArray[portIter], portIter);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCountersIpSet: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
        }

        /* setup nexthope portInterface for capturing */
        portInterface.type              = CPSS_INTERFACE_PORT_E;
        portInterface.devPort.hwDevNum  = prvTgfDevNum;
        portInterface.devPort.portNum   = nextHopPortNum;

        /* enable capture on next hop port/trunk */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, captureType, GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d\n",
                                     prvTgfDevNum, captureType);

        /* clear capturing RxPcktTable */
        rc = tgfTrafficTableRxPcktTblClear();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

        switch (i)
        {
        case 0:
            /* ipv4 UC packet 0.0.1.17 */
            cpssOsMemCpy(prvTgfPacketPartIpv4.dstAddr, ipAddr->arIP, sizeof(ipAddr->arIP));
            partsCount = sizeof(prvTgfPacketPartArray) / sizeof(prvTgfPacketPartArray[0]);
            packetPartsPtr = prvTgfPacketPartArray;
            break;
        case 1:
            /* ipv6 UC 0.0.0.0.0.0.0.0.0.0.0.0.0.0.3.51 */
            prvTgfPacketPartIpv6.dstAddr[0] = ipv6Addr->arIP[1]<<8 | ipv6Addr->arIP[0];
            prvTgfPacketPartIpv6.dstAddr[1] = ipv6Addr->arIP[2]<<8 | ipv6Addr->arIP[3];
            prvTgfPacketPartIpv6.dstAddr[2] = ipv6Addr->arIP[4]<<8 | ipv6Addr->arIP[5];
            prvTgfPacketPartIpv6.dstAddr[3] = ipv6Addr->arIP[6]<<8 | ipv6Addr->arIP[6];
            prvTgfPacketPartIpv6.dstAddr[4] = ipv6Addr->arIP[8]<<8 | ipv6Addr->arIP[9];
            prvTgfPacketPartIpv6.dstAddr[5] = ipv6Addr->arIP[10]<<8 | ipv6Addr->arIP[11];
            prvTgfPacketPartIpv6.dstAddr[6] = ipv6Addr->arIP[12]<<8 | ipv6Addr->arIP[12];
            prvTgfPacketPartIpv6.dstAddr[7] = ipv6Addr->arIP[14]<<8 | ipv6Addr->arIP[15];
            partsCount = sizeof(prvTgfPacketPartArrayIpv6) / sizeof(prvTgfPacketPartArrayIpv6[0]);
            packetPartsPtr = prvTgfPacketPartArrayIpv6;
            break;
        default:
            break;
        }

        /* calculate packet size */
        rc = prvTgfPacketSizeGet(packetPartsPtr, partsCount, &packetSize);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet:");

        /* build packet */
        packetInfo.totalLen   = packetSize;
        packetInfo.numOfParts = partsCount;
        packetInfo.partsArray = packetPartsPtr;

        prvTgfBurstCount = 1;
        /* clear VFD array */
        cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));
        numVfd = 0;

        /* setup packet */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, prvTgfBurstCount, numVfd, &vfdArray[0]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

        /* set transmit timings */
        rc = prvTgfSetTxSetup2Eth(1, 1);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetup2Eth: %d", prvTgfDevNum);

        /* AUTODOC: send packet from port 0 with: */

        /* send packet -- send from specific port -- even if member of a trunk */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[sendPortIndex]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d\n",
                                     prvTgfDevNum, prvTgfPortsArray[sendPortIndex]);
        cpssOsTimerWkAfter(1000);

        /* read and check ethernet counters */
        for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
        {
            GT_BOOL isOk;

            /* read counters */
            rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d\n",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);

            if ((portIter == 3) || (portIter == 0))
            {
                /* packetSize is not changed */
                    expectedCntrs.goodOctetsSent.l[0] = (packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                    expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
                    expectedCntrs.ucPktsSent.l[0] = prvTgfBurstCount;
                    expectedCntrs.mcPktsSent.l[0] = 0;
                    expectedCntrs.ucPktsRcv.l[0]  = prvTgfBurstCount;
                    expectedCntrs.mcPktsRcv.l[0]  = 0;
                    expectedCntrs.brdcPktsSent.l[0]   = 0;
                    expectedCntrs.goodOctetsRcv.l[0]  = (packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                    expectedCntrs.goodPktsRcv.l[0]    = prvTgfBurstCount;
                    expectedCntrs.brdcPktsRcv.l[0]    = 0;

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

        rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_CAPTURE_E,
                                           GT_TRUE, GT_TRUE, packetBuff,
                                           &buffLen, &packetActualLength,
                                           &devNum, &queue, &rxParam);
        PRV_UTF_LOG2_MAC(" ************************************\nCpuCode = %d \n CmdIsToCpu = %d \n ************************************ \n",
                                rxParam.cpuCode, rxParam.dsaCmdIsToCpu);

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

        /* disable capture on nexthope port , before check the packets to the CPU */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, captureType, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d\n",
                                     prvTgfDevNum, captureType);
    }
}
/**
* @internal prvTgfIpLpmHaBulkValidityCheckAndTrafficSend function
* @endinternal
*
* @brief  Check HW and SW validity and send traffic to some
*         predifined entries.
*
* @param[in] sendPortIndex  - port to send traffic from
* @param[in] nextHopPortNum - port to send traffic to
*/
GT_VOID prvTgfIpLpmHaBulkValidityCheckAndTrafficSend
(
    GT_U8   sendPortIndex,
    GT_U32  nextHopPortNum
)
{
    GT_STATUS   rc=GT_OK;
    GT_IPADDR   ipAddr;
    GT_IPV6ADDR ipv6Addr;

    /* for SIP5 we have a stack issue overflow - after the problem will be fixed we can return this check */
    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        rc = prvTgfIpLpmValidityCheck(prvTgfLpmDBId, 0, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E, GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmValidityCheck ipv4\n");

        rc = prvTgfIpLpmValidityCheck(prvTgfLpmDBId, 0, CPSS_IP_PROTOCOL_IPV6_E, CPSS_UNICAST_E,GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmValidityCheck ipv6 \n");
    }
    /* send ipv4 UC packet */
    ipAddr.arIP[0]=0;
    ipAddr.arIP[1]=0;
    ipAddr.arIP[2]=1;
    ipAddr.arIP[3]=0x11;

   /* send ipv6 UC packet */
    cpssOsMemSet(&ipv6Addr.arIP,0,sizeof(ipv6Addr.arIP));
    ipv6Addr.arIP[14]=3;
    ipv6Addr.arIP[15]=0x33;

    prvTgfIpLpmHaBulkTrafficGenerate(sendPortIndex,nextHopPortNum,&ipAddr,&ipv6Addr);
}

/**
* @internal prvTgfIpLpmHaAddBasicConfigRestore function
* @endinternal
*
* @brief  Set LPM HA to completion state and delete all prefixes
*
* @param[in] sendPortIndex  - port to send traffic from
*/
GT_VOID prvTgfIpLpmHaAddBasicConfigRestore
(
    GT_U8   sendPortIndex
)
{
    GT_STATUS   rc=GT_OK;

    /* flush all prefixes */
    rc = prvTgfIpLpmIpv4UcPrefixesFlush(prvTgfLpmDBId,0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmIpv4UcPrefixesFlush \n");

    rc = prvTgfIpLpmIpv6UcPrefixesFlush(prvTgfLpmDBId,0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmIpv6UcPrefixesFlush \n");

    rc = prvTgfIpLpmIpv4McEntriesFlush(prvTgfLpmDBId,0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmIpv4McEntriesFlush \n");

    rc = prvTgfIpLpmIpv6McEntriesFlush(prvTgfLpmDBId,0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmIpv6McEntriesFlush \n");

    prvTgfIpLpmHaBasicRoutingConfigurationRestore(sendPortIndex);
}

/* default LPM DB and Virtual Router configuration */
static struct
{
    GT_BOOL                                     isSupportIpv4Uc;
    GT_BOOL                                     isSupportIpv6Uc;
    GT_BOOL                                     isSupportIpv4Mc;
    GT_BOOL                                     isSupportIpv6Mc;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT        defIpv4UcNextHopInfo;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT        defIpv6UcNextHopInfo;
    PRV_TGF_IP_LTT_ENTRY_STC                    defIpv4McRouteLttEntry;
    PRV_TGF_IP_LTT_ENTRY_STC                    defIpv6McRouteLttEntry;
    PRV_TGF_IP_TCAM_SHADOW_TYPE_ENT             shadowType;
    CPSS_IP_PROTOCOL_STACK_ENT                  protocolStack;
    PRV_TGF_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC  indexesRange;
    GT_BOOL                                     partitionEnable;
    PRV_TGF_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC  tcamLpmManagerCapcityCfg;
    GT_BOOL                                     isTcamManagerInternal;
} prvTgfDefLpmDBVrCfg_second;

/**
* @internal prvTgfIpLpmHaWith2VrfBasicRoutingConfigurationSet
*           function
* @endinternal
*
* @brief   Set LTT Route Configuration for ipv4/6 UC/MC
*
* @param[in] prvUtfVrfId              - virtual router index
* @param[in] sendPortNum              - port sending traffic
* @param[in] nextHopPortNum           - port receiving traffic
* @param[in] firstCall                - GT_TRUE: callinf this
*                                       API for the first time
*                                       GT_FALSE: callimg this
*                                       API for the second time
*/
static GT_VOID prvTgfIpLpmHaWith2VrfBasicRoutingConfigurationSet
(
    GT_U32   prvUtfVrfId,
    GT_U32   prvUtfVrfId_second,
    GT_U8    sendPortIndex,
    GT_U32   nextHopPortNum,
    GT_BOOL  firstCall
)
{
    GT_STATUS                               rc = GT_OK;

    prvTgfIpLpmHaBasicRoutingConfigurationSet(prvUtfVrfId,sendPortIndex,nextHopPortNum,firstCall);
    if(firstCall==GT_TRUE)
    {
        /* create second vrId in the system with the same configuration as default vrfId */
        /* retrieves configuration of the Virtual Router */
        rc = prvTgfIpLpmVirtualRouterGet(prvTgfLpmDBId, prvUtfVrfId,
                                         &(prvTgfDefLpmDBVrCfg_second.isSupportIpv4Uc),
                                         &(prvTgfDefLpmDBVrCfg_second.defIpv4UcNextHopInfo),
                                         &(prvTgfDefLpmDBVrCfg_second.isSupportIpv6Uc),
                                         &(prvTgfDefLpmDBVrCfg_second.defIpv6UcNextHopInfo),
                                         &(prvTgfDefLpmDBVrCfg_second.isSupportIpv4Mc),
                                         &(prvTgfDefLpmDBVrCfg_second.defIpv4McRouteLttEntry),
                                         &(prvTgfDefLpmDBVrCfg_second.isSupportIpv6Mc),
                                         &(prvTgfDefLpmDBVrCfg_second.defIpv6McRouteLttEntry));
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmVirtualRouterGet\n");

        /* add new vrfId */
        rc=prvTgfIpLpmVirtualRouterAdd(prvTgfLpmDBId, prvUtfVrfId_second,
                                       &(prvTgfDefLpmDBVrCfg_second.defIpv4UcNextHopInfo),
                                       &(prvTgfDefLpmDBVrCfg_second.defIpv6UcNextHopInfo),
                                       &(prvTgfDefLpmDBVrCfg_second.defIpv4McRouteLttEntry),
                                       &(prvTgfDefLpmDBVrCfg_second.defIpv6McRouteLttEntry),
                                       NULL);/* FCOE not supported for SIP6 */
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmVirtualRouterAdd\n");
    }
}
/**
* @internal
*           prvTgfIpLpmHaWith2VrfBasicRoutingConfigurationRestore
*           function
* @endinternal
*
* @brief   Set LTT Route Configuration for ipv4/6 UC/MC
*
* @param[in] sendPortNum         - port sending traffic
*
*/
static GT_VOID prvTgfIpLpmHaWith2VrfBasicRoutingConfigurationRestore
(
    GT_U8    sendPortIndex
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    prvUtfVrfId_second=10;

    prvTgfIpLpmHaBasicRoutingConfigurationRestore(sendPortIndex);

    /* delete second virtual router */
    rc=prvTgfIpLpmVirtualRouterDel(prvTgfLpmDBId, prvUtfVrfId_second);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmVirtualRouterDel for prvUtfVrfId_second\n");

}
/**
* @internal prvTgfIpLpmHaWith2VrfAddBasicConfig function
* @endinternal
*
* @brief  Add some sequentially ipv4/ipv6 UC/MC Prefixes in
*         order manner. Verify that we get same number of
*         prefixes as expected
* @param[in] sendPortIndex  - port index to send traffic from
* @param[in] nextHopPortNum - port to send traffic to
* @param[in] firstCall                - GT_TRUE: callinf this
*                                       API for the first time
*                                       GT_FALSE: callimg this
*                                       API for the second time
*/
GT_VOID prvTgfIpLpmHaWith2VrfAddBasicConfig
(
    GT_U8   sendPortIndex,
    GT_U32  nextHopPortNum,
    GT_BOOL firstCall
)
{
    GT_STATUS   rc=GT_OK;
    GT_IPADDR   ipAddr;
    GT_IPV6ADDR ipv6Addr;
    GT_U32      routeEntryBaseMemAddr;
    GT_U32      numOfPrefixesToAdd,numOfPrefixesToDel;
    GT_U32      numOfPrefixesAdded,numOfPrefixesDeleted;
    GT_IPADDR   lastIpAddrAdded,lastIpAddrDeleted;
    GT_IPV6ADDR lastIpv6AddrAdded;
    GT_IPADDR   grpStartIpAddr;
    GT_IPADDR   srcStartIpAddr;
    GT_IPV6ADDR ipv6GrpStartIpAddr;
    GT_IPV6ADDR ipv6SrcStartIpAddr;
    GT_IPADDR   grpLastIpAddrAdded;
    GT_IPADDR   srcLastIpAddrAdded;
    GT_IPV6ADDR ipv6GrpLastIpAddrAdded;
    GT_IPV6ADDR ipv6SrcLastIpAddrAdded;
    GT_U32      defaultVrfId=0;/* created in init */
    GT_U32      secondVrfId =10;/* need to be created in the test */
    GT_U32      i=0;
    GT_U32      vrId;

    prvTgfIpLpmHaWith2VrfBasicRoutingConfigurationSet(defaultVrfId,secondVrfId,sendPortIndex,nextHopPortNum,firstCall);

    /* loop on 2 virtual routers id*/
    while (i<2)
    {
        ipAddr.arIP[0]=0;
        ipAddr.arIP[1]=0;
        ipAddr.arIP[2]=1;
        ipAddr.arIP[3]=1;

        if (i==0)
        {
            vrId=defaultVrfId;
        }
        else
        {
            vrId=secondVrfId;
        }
        if (firstCall==GT_TRUE)
        {
            numOfPrefixesToAdd=100;

             /* add 100 prefixes - ipv4 0.0.1.1 - 0.0.1.100 */
            routeEntryBaseMemAddr=5;

            rc=prvTgfIpLpmIpv4UcPrefixAddManyByOctet(prvTgfLpmDBId,vrId,&ipAddr,
                                                       routeEntryBaseMemAddr,
                                                       numOfPrefixesToAdd,
                                                       3,2,1,0,
                                                       &lastIpAddrAdded,
                                                       &numOfPrefixesAdded);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmIpv4UcPrefixAddManyByOctet\n");
            UTF_VERIFY_EQUAL0_STRING_MAC(numOfPrefixesToAdd, numOfPrefixesAdded, "numOfPrefixesToAdd!numOfPrefixesAdded \n");

            /* delete 20 prefixes 0.0.1.21 - 0.0.1.40
               delete VR
               delete LPM DB */
            ipAddr.arIP[3]=21;

            numOfPrefixesToDel=20;
            rc=prvTgfIpLpmIpv4UcPrefixDelManyByOctet(prvTgfLpmDBId,vrId,&ipAddr,
                                                        numOfPrefixesToDel,
                                                        3,2,1,0,
                                                        &lastIpAddrDeleted,
                                                        &numOfPrefixesDeleted);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmIpv4UcPrefixDelManyByOctet\n");
            UTF_VERIFY_EQUAL0_STRING_MAC(numOfPrefixesToDel, numOfPrefixesDeleted, "numOfPrefixesToDel!numOfPrefixesDeleted \n");

        }
        else
        {
            /* just add what there is in the HW */
            numOfPrefixesToAdd=20;

             /* add 20 prefixes in case of second call - ipv4 0.0.1.1 - 0.0.1.20*/
            routeEntryBaseMemAddr=5;

            rc=prvTgfIpLpmIpv4UcPrefixAddManyByOctet(prvTgfLpmDBId,vrId,&ipAddr,
                                                       routeEntryBaseMemAddr,
                                                       numOfPrefixesToAdd,
                                                       3,2,1,0,
                                                       &lastIpAddrAdded,
                                                       &numOfPrefixesAdded);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmIpv4UcPrefixAddManyByOctet\n");
            UTF_VERIFY_EQUAL0_STRING_MAC(numOfPrefixesToAdd, numOfPrefixesAdded, "numOfPrefixesToAdd!numOfPrefixesAdded \n");


            /* add 60 prefixes - ipv4 0.0.1.41 - 0.0.1.100 */
            ipAddr.arIP[3]=41;
            numOfPrefixesToAdd=60;
            rc=prvTgfIpLpmIpv4UcPrefixAddManyByOctet(prvTgfLpmDBId,vrId,&ipAddr,
                                                       routeEntryBaseMemAddr,
                                                       numOfPrefixesToAdd,
                                                       3,2,1,0,
                                                       &lastIpAddrAdded,
                                                       &numOfPrefixesAdded);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmIpv4UcPrefixAddManyByOctet\n");
            UTF_VERIFY_EQUAL0_STRING_MAC(numOfPrefixesToAdd, numOfPrefixesAdded, "numOfPrefixesToAdd!numOfPrefixesAdded \n");

        }

        cpssOsMemSet(&ipv6Addr.arIP,0,sizeof(ipv6Addr.arIP));
        ipv6Addr.arIP[14]=3;
        ipv6Addr.arIP[15]=0x33;

        /* add ipv6 0.0.0.0.0.0.0.0.0.0.0.0.0.3.51*/
        routeEntryBaseMemAddr=6;
        numOfPrefixesToAdd=1;
        rc=prvTgfIpLpmIpv6UcPrefixAddManyByOctet(prvTgfLpmDBId,vrId,&ipv6Addr,
                                                   routeEntryBaseMemAddr,
                                                   numOfPrefixesToAdd,
                                                   15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,
                                                   &lastIpv6AddrAdded,
                                                   &numOfPrefixesAdded);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmIpv6UcPrefixAddManyByOctet\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(numOfPrefixesToAdd, numOfPrefixesAdded, "numOfPrefixesToAdd!numOfPrefixesAdded \n");

        /* add ipv4 MC 224.1.1.1/0.0.2.34 */
        routeEntryBaseMemAddr=7;

        grpStartIpAddr.arIP[0]=224;
        grpStartIpAddr.arIP[1]=1;
        grpStartIpAddr.arIP[2]=1;
        grpStartIpAddr.arIP[3]=1;

        srcStartIpAddr.arIP[0]=0;
        srcStartIpAddr.arIP[1]=0;
        srcStartIpAddr.arIP[2]=2;
        srcStartIpAddr.arIP[3]=0x22;

        rc=prvTgfIpLpmIpv4McPrefixAddManyByOctet(prvTgfLpmDBId,vrId,&grpStartIpAddr,&srcStartIpAddr,
                                                   routeEntryBaseMemAddr,
                                                   numOfPrefixesToAdd,GT_FALSE,3,2,1,0,
                                                   GT_TRUE,3,2,1,0,
                                                   &grpLastIpAddrAdded,
                                                   &srcLastIpAddrAdded,
                                                   &numOfPrefixesAdded);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmIpv4McPrefixAddManyByOctet\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(numOfPrefixesToAdd, numOfPrefixesAdded, "numOfPrefixesToAdd!numOfPrefixesAdded \n");

        routeEntryBaseMemAddr=8;
        cpssOsMemSet(&ipv6GrpStartIpAddr.arIP,0,sizeof(ipv6GrpStartIpAddr.arIP));
        cpssOsMemSet(&ipv6SrcStartIpAddr.arIP,0,sizeof(ipv6SrcStartIpAddr.arIP));

        /*add ipv6 0.0.0.0.0.0.0.0.0.0.0.0.0.3.51*/
        ipv6GrpStartIpAddr.arIP[0]=0xFF;
        ipv6GrpStartIpAddr.arIP[1]=0xFF;
        ipv6GrpStartIpAddr.arIP[14]=0x22;
        ipv6GrpStartIpAddr.arIP[15]=0x22;

        ipv6SrcStartIpAddr.arIP[0] =0;
        ipv6SrcStartIpAddr.arIP[1] =0;
        ipv6SrcStartIpAddr.arIP[14]=4;
        ipv6SrcStartIpAddr.arIP[15]=0x44;
        rc=prvTgfIpLpmIpv6McPrefixAddManyByOctet(prvTgfLpmDBId,vrId,&ipv6GrpStartIpAddr,&ipv6SrcStartIpAddr,
                                                   routeEntryBaseMemAddr,
                                                   numOfPrefixesToAdd,GT_FALSE,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,
                                                   GT_TRUE,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,
                                                   &ipv6GrpLastIpAddrAdded,
                                                   &ipv6SrcLastIpAddrAdded,
                                                   &numOfPrefixesAdded);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmIpv6McPrefixAddManyByOctet\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(numOfPrefixesToAdd, numOfPrefixesAdded, "numOfPrefixesToAdd!numOfPrefixesAdded \n");

        i++;
    }
    /* print the memory - for debug use */
    /*rc = prvTgfIpLpmDbgHwOctetPerProtocolPrint(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmDbgHwOctetPerProtocolPrint\n");
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters\n");
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmMemoryAndCounters(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmMemoryAndCounters\n");*/
}

/**
* @internal prvTgfIpLpmHaWith2VrfSystemRecoveryStateSet function
* @endinternal
*
* @brief  Keep LPM DB values for reconstruct and
*       set flag for HA process.
*/
GT_VOID prvTgfIpLpmHaWith2VrfSystemRecoveryStateSet()
{
    GT_STATUS   rc=GT_OK;
    CPSS_SYSTEM_RECOVERY_INFO_STC               tempSystemRecovery_Info;

    cpssOsMemSet(&tempSystemRecovery_Info, 0, sizeof(CPSS_SYSTEM_RECOVERY_INFO_STC));
    tempSystemRecovery_Info.systemRecoveryProcess=CPSS_SYSTEM_RECOVERY_PROCESS_HA_E;
    tempSystemRecovery_Info.systemRecoveryState=CPSS_SYSTEM_RECOVERY_INIT_STATE_E;

    /* get LPM DB config */
    rc = prvTgfIpLpmRamDBConfigGet(prvTgfLpmDBId,&shadowTypeParam,&protocolStackParam,&ramDbCfgParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmDBConfigGet\n");

    /* retrieves configuration of the Virtual Router */
    rc = prvTgfIpLpmVirtualRouterGet(prvTgfLpmDBId, 0,
                                     &(prvTgfDefLpmDBVrCfg.isSupportIpv4Uc),
                                     &(prvTgfDefLpmDBVrCfg.defIpv4UcNextHopInfo),
                                     &(prvTgfDefLpmDBVrCfg.isSupportIpv6Uc),
                                     &(prvTgfDefLpmDBVrCfg.defIpv6UcNextHopInfo),
                                     &(prvTgfDefLpmDBVrCfg.isSupportIpv4Mc),
                                     &(prvTgfDefLpmDBVrCfg.defIpv4McRouteLttEntry),
                                     &(prvTgfDefLpmDBVrCfg.isSupportIpv6Mc),
                                     &(prvTgfDefLpmDBVrCfg.defIpv6McRouteLttEntry));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmVirtualRouterGet\n");

    /* Get device list array from skip list for given LMP DB id */
    numOfDevsParam = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = prvTgfIpLpmDBDevListGet(prvTgfLpmDBId, &numOfDevsParam, devListArray);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmDBDevListGet\n");

    rc=cpssSystemRecoveryStateSet(&tempSystemRecovery_Info);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on cpssSystemRecoveryStateSet\n");
}

/**
* @internal prvTgfIpLpmHaWith2VrfDelBasicConfig function
* @endinternal
*
* @brief  Delete some sequentially ipv4/ipv6 UC/MC Prefixes in
*         order manner. Verify that we delete same number of
*         prefixes as expected
*/
GT_VOID prvTgfIpLpmHaWith2VrfDelBasicConfig
(
    GT_VOID
)
{
    GT_STATUS   rc=GT_OK;
    GT_IPADDR   ipAddr;
    GT_IPV6ADDR ipv6Addr;
    GT_U32      numOfPrefixesToDel;
    GT_U32      numOfPrefixesDeleted;
    GT_IPADDR   lastIpAddrDeleted;
    GT_IPV6ADDR lastIpv6AddrDeleted;
    GT_IPADDR   grpStartIpAddr;
    GT_IPADDR   srcStartIpAddr;
    GT_IPV6ADDR ipv6GrpStartIpAddr;
    GT_IPV6ADDR ipv6SrcStartIpAddr;
    GT_IPADDR   grpLastIpAddrDeleted;
    GT_IPADDR   srcLastIpAddrDeleted;
    GT_IPV6ADDR ipv6GrpLastIpAddrDeleted;
    GT_IPV6ADDR ipv6SrcLastIpAddrDeleted;
    GT_U32      defaultVrfId=0, secondVrfId=10;
    GT_U32      vrId;
    GT_U32      i=0;

    /* loop on 2 virtual routers id*/
    while (i<2)
    {
        if (i==0)
        {
            vrId=defaultVrfId;
        }
        else
        {
            vrId=secondVrfId;
        }

        ipAddr.arIP[0]=0;
        ipAddr.arIP[1]=0;
        ipAddr.arIP[2]=1;
        ipAddr.arIP[3]=1;

        /* delete 20 prefixes in case of second call - ipv4 0.0.1.1 - 0.0.1.20*/
        /* just delete what there is in the HW */
        numOfPrefixesToDel=20;
        rc=prvTgfIpLpmIpv4UcPrefixDelManyByOctet(prvTgfLpmDBId,vrId,&ipAddr,
                                                   numOfPrefixesToDel,
                                                   3,2,1,0,
                                                   &lastIpAddrDeleted,
                                                   &numOfPrefixesDeleted);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmIpv4UcPrefixDelManyByOctet\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(numOfPrefixesToDel, numOfPrefixesDeleted, "numOfPrefixesToDel!numOfPrefixesDeleted \n");

        /* delete 41 prefixes - ipv4 0.0.0.60 - 0.0.0.100 */
        ipAddr.arIP[3]=41;
        numOfPrefixesToDel=60;
        rc=prvTgfIpLpmIpv4UcPrefixDelManyByOctet(prvTgfLpmDBId,vrId,&ipAddr,
                                                   numOfPrefixesToDel,
                                                   3,2,1,0,
                                                   &lastIpAddrDeleted,
                                                   &numOfPrefixesDeleted);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmIpv4UcPrefixDelManyByOctet\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(numOfPrefixesToDel, numOfPrefixesDeleted, "numOfPrefixesToDel!numOfPrefixesDeleted \n");

        cpssOsMemSet(&ipv6Addr.arIP,0,sizeof(ipv6Addr.arIP));
        ipv6Addr.arIP[14]=3;
        ipv6Addr.arIP[15]=0x33;

        /* add ipv6 0.0.0.0.0.0.0.0.0.0.0.0.0.3.51*/
        numOfPrefixesToDel=1;
        rc=prvTgfIpLpmIpv6UcPrefixDelManyByOctet(prvTgfLpmDBId,vrId,&ipv6Addr,
                                              numOfPrefixesToDel,
                                              15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,
                                              &lastIpv6AddrDeleted,
                                              &numOfPrefixesDeleted);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmIpv6UcPrefixDelManyByOctet\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(numOfPrefixesToDel, numOfPrefixesDeleted, "numOfPrefixesToDel!numOfPrefixesDeleted \n");

        /* delete ipv4 MC 224.1.1.1/0.0.2.34 */
        grpStartIpAddr.arIP[0]=224;
        grpStartIpAddr.arIP[1]=1;
        grpStartIpAddr.arIP[2]=1;
        grpStartIpAddr.arIP[3]=1;

        srcStartIpAddr.arIP[0]=0;
        srcStartIpAddr.arIP[1]=0;
        srcStartIpAddr.arIP[2]=2;
        srcStartIpAddr.arIP[3]=0x22;

        numOfPrefixesToDel=1;
        rc=prvTgfIpLpmIpv4McPrefixDelManyByOctet(prvTgfLpmDBId,vrId,&grpStartIpAddr,&srcStartIpAddr,
                                              numOfPrefixesToDel,GT_FALSE,3,2,1,0,
                                              GT_TRUE,3,2,1,0,
                                              &grpLastIpAddrDeleted,
                                              &srcLastIpAddrDeleted,
                                              &numOfPrefixesDeleted);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmIpv4McPrefixDelManyByOctet\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(numOfPrefixesToDel, numOfPrefixesDeleted, "numOfPrefixesToDel!numOfPrefixesDeleted \n");

        cpssOsMemSet(&ipv6GrpStartIpAddr.arIP,0,sizeof(ipv6GrpStartIpAddr.arIP));
        cpssOsMemSet(&ipv6SrcStartIpAddr.arIP,0,sizeof(ipv6SrcStartIpAddr.arIP));

        /*add ipv6 0.0.0.0.0.0.0.0.0.0.0.0.0.3.51*/
        ipv6GrpStartIpAddr.arIP[0]=0xFF;
        ipv6GrpStartIpAddr.arIP[1]=0xFF;
        ipv6GrpStartIpAddr.arIP[14]=0x22;
        ipv6GrpStartIpAddr.arIP[15]=0x22;

        ipv6SrcStartIpAddr.arIP[0] =0;
        ipv6SrcStartIpAddr.arIP[1] =0;
        ipv6SrcStartIpAddr.arIP[14]=4;
        ipv6SrcStartIpAddr.arIP[15]=0x44;

        numOfPrefixesToDel=1;
        rc=prvTgfIpLpmIpv6McPrefixDelManyByOctet(prvTgfLpmDBId,vrId,&ipv6GrpStartIpAddr,&ipv6SrcStartIpAddr,
                                          numOfPrefixesToDel,GT_FALSE,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,
                                          GT_TRUE,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,
                                          &ipv6GrpLastIpAddrDeleted,
                                          &ipv6SrcLastIpAddrDeleted,
                                          &numOfPrefixesDeleted);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmIpv6McPrefixDelManyByOctet\n");
        UTF_VERIFY_EQUAL0_STRING_MAC(numOfPrefixesToDel, numOfPrefixesDeleted, "numOfPrefixesToDel!numOfPrefixesDeleted \n");

        /* delete virtual router */
        rc=prvTgfIpLpmVirtualRouterDel(prvTgfLpmDBId, vrId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmVirtualRouterDel\n");

        i++;
    }

    /* Delete device list array for given LMP DB id */
    rc = prvTgfIpLpmDBDevsListRemove(prvTgfLpmDBId, devListArray , numOfDevsParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmDBDevsListRemove\n");

    rc=prvTgfIpLpmDBDelete(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmDBDelete\n");
}

/**
* @internal prvTgfIpLpmHaWith2VrfAddConfigAfterDelete function
* @endinternal
*
* @brief  Reconstruct LPM DB values after delete
*/
GT_VOID prvTgfIpLpmHaWith2VrfAddConfigAfterDelete()
{
    GT_STATUS   rc=GT_OK;
    GT_U32      prvUtfVrfId_second=10;

   /* create LPM DB, Add device list and Add VR */
    rc = prvTgfIpLpmRamDBCreate(prvTgfLpmDBId,protocolStackParam,&ramDbCfgParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmRamDBCreate\n");

    rc = prvTgfIpLpmDBDevListAdd(prvTgfLpmDBId, devListArray, numOfDevsParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmDBDevListAdd\n");

    rc=prvTgfIpLpmVirtualRouterAdd(prvTgfLpmDBId, 0,
                                   &(prvTgfDefLpmDBVrCfg.defIpv4UcNextHopInfo),
                                   &(prvTgfDefLpmDBVrCfg.defIpv6UcNextHopInfo),
                                   &(prvTgfDefLpmDBVrCfg.defIpv4McRouteLttEntry),
                                   &(prvTgfDefLpmDBVrCfg.defIpv6McRouteLttEntry),
                                   NULL);/* FCOE not supported for SIP6 */
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmVirtualRouterAdd\n");

    rc=prvTgfIpLpmVirtualRouterAdd(prvTgfLpmDBId, prvUtfVrfId_second,
                                   &(prvTgfDefLpmDBVrCfg_second.defIpv4UcNextHopInfo),
                                   &(prvTgfDefLpmDBVrCfg_second.defIpv6UcNextHopInfo),
                                   &(prvTgfDefLpmDBVrCfg_second.defIpv4McRouteLttEntry),
                                   &(prvTgfDefLpmDBVrCfg_second.defIpv6McRouteLttEntry),
                                   NULL);/* FCOE not supported for SIP6 */
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmVirtualRouterAdd\n");
}

/**
* @internal prvTgfIpLpmHaWith2VrfValidityCheckAndTrafficSend
*           function
* @endinternal
*
* @brief  Check HW and SW validity and send traffic to some
*         predifined entries.
*
* @param[in] sendPortIndex  - port to send traffic from
* @param[in] nextHopPortNum - port to send traffic to
*/
GT_VOID prvTgfIpLpmHaWith2VrfValidityCheckAndTrafficSend
(
    GT_U8   sendPortIndex,
    GT_U32  nextHopPortNum
)
{
    GT_STATUS   rc=GT_OK;
    GT_IPADDR   ipAddr;
    GT_IPV6ADDR ipv6Addr;
    GT_IPADDR   grpStartIpAddr;
    GT_IPADDR   srcStartIpAddr;
    GT_IPV6ADDR ipv6GrpStartIpAddr;
    GT_IPV6ADDR ipv6SrcStartIpAddr;
    GT_U32      prvUtfVrfId_second=10;
    GT_U32      vrId;
    GT_U32      i=0;

    /* for SIP5 we have a stack issue overflow - after the problem will be fixed we can return this check */
    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        /* loop on 2 virtual routers id*/
        while (i<2)
        {
            if (i==0)
            {
                vrId=0;
            }
            else
            {
                vrId=prvUtfVrfId_second;
            }
            rc = prvTgfIpLpmValidityCheck(prvTgfLpmDBId, vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E,GT_TRUE);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmValidityCheck ipv4\n");

            rc = prvTgfIpLpmValidityCheck(prvTgfLpmDBId, vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_UNICAST_E,GT_TRUE);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmValidityCheck ipv6 \n");

            rc = prvTgfIpLpmValidityCheck(prvTgfLpmDBId, vrId, CPSS_IP_PROTOCOL_IPV4_E, CPSS_MULTICAST_E,GT_TRUE);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmValidityCheck ipv4\n");

            rc = prvTgfIpLpmValidityCheck(prvTgfLpmDBId, vrId, CPSS_IP_PROTOCOL_IPV6_E, CPSS_MULTICAST_E,GT_TRUE);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmValidityCheck ipv6 \n");
            i++;
        }
    }

    /* send ipv4 UC packet */
    ipAddr.arIP[0]=0;
    ipAddr.arIP[1]=0;
    ipAddr.arIP[2]=1;
    ipAddr.arIP[3]=0x11;

    /* send ipv4 MC packet */
    grpStartIpAddr.arIP[0]=224;
    grpStartIpAddr.arIP[1]=1;
    grpStartIpAddr.arIP[2]=1;
    grpStartIpAddr.arIP[3]=1;

    srcStartIpAddr.arIP[0]=0;
    srcStartIpAddr.arIP[1]=0;
    srcStartIpAddr.arIP[2]=2;
    srcStartIpAddr.arIP[3]=0x22;

    /* send ipv6 UC packet */
    cpssOsMemSet(&ipv6Addr.arIP,0,sizeof(ipv6Addr.arIP));
    ipv6Addr.arIP[14]=3;
    ipv6Addr.arIP[15]=0x33;

    /* send ipv6 MC packet */
    cpssOsMemSet(&ipv6GrpStartIpAddr.arIP,0,sizeof(ipv6GrpStartIpAddr.arIP));
    cpssOsMemSet(&ipv6SrcStartIpAddr.arIP,0,sizeof(ipv6SrcStartIpAddr.arIP));

    ipv6GrpStartIpAddr.arIP[0]=0xFF;
    ipv6GrpStartIpAddr.arIP[1]=0xFF;
    ipv6GrpStartIpAddr.arIP[14]=0x22;
    ipv6GrpStartIpAddr.arIP[15]=0x22;

    ipv6SrcStartIpAddr.arIP[14]=4;
    ipv6SrcStartIpAddr.arIP[15]=0x44;

    prvTgfIpLpmHaTrafficGenerate(sendPortIndex,nextHopPortNum,&ipAddr,
                                 &grpStartIpAddr,&srcStartIpAddr,&ipv6Addr,
                                 &ipv6GrpStartIpAddr,&ipv6SrcStartIpAddr);
}

/**
* @internal prvTgfIpLpmHaWith2VrfAddBasicConfigRestore function
* @endinternal
*
* @brief  Set LPM HA to completion state and delete all prefixes
*
* @param[in] sendPortIndex  - port to send traffic from
*/
GT_VOID prvTgfIpLpmHaWith2VrfAddBasicConfigRestore
(
    GT_U8   sendPortIndex
)
{
    GT_STATUS   rc=GT_OK;
    GT_U32      prvUtfVrfId_second=10;
    GT_U32      vrId;
    GT_U32      i=0;

     /* print the memory - for debug use */
    /*rc = prvTgfIpLpmDbgHwOctetPerProtocolPrint(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmDbgHwOctetPerProtocolPrint\n");
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters\n");
    rc = prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmMemoryAndCounters(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmDbgHwOctetPerProtocolPrintLpmMemoryAndCounters\n");*/

    /* loop on 2 virtual routers id*/
    while (i<2)
    {
        if (i==0)
        {
            vrId=0;
        }
        else
        {
            vrId=prvUtfVrfId_second;
        }
        /* flush all prefixes */
        rc = prvTgfIpLpmIpv4UcPrefixesFlush(prvTgfLpmDBId,vrId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmIpv4UcPrefixesFlush \n");

        rc = prvTgfIpLpmIpv6UcPrefixesFlush(prvTgfLpmDBId,vrId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmIpv6UcPrefixesFlush \n");

        rc = prvTgfIpLpmIpv4McEntriesFlush(prvTgfLpmDBId,vrId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmIpv4McEntriesFlush \n");

        rc = prvTgfIpLpmIpv6McEntriesFlush(prvTgfLpmDBId,vrId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmIpv6McEntriesFlush \n");

        i++;
    }
    prvTgfIpLpmHaWith2VrfBasicRoutingConfigurationRestore(sendPortIndex);
}
