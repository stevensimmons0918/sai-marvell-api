/*******************************************************************************
*              (c), Copyright 2019, Marvell International Ltd.                 *
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
* @file prvTgfFlowManager.c
*
* @brief This file includes enhanced UT cases for IPFIX Flow Manager.
*
* @version   1
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/dxCh/dxChxGen/flowManager/cpssDxChFlowManager.h>
#include <cpss/dxCh/dxChxGen/flowManager/prvCpssDxChFlowManagerDb.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>

#include <cpss/dxCh/dxChxGen/pcl/cpssDxChPcl.h>
#include <cpss/dxCh/dxChxGen/ipfix/cpssDxChIpfix.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <cpss/dxCh/dxChxGen/virtualTcam/cpssDxChVirtualTcam.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <extUtils/iterators/extUtilDevPort.h>
#include <common/tgfBridgeGen.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgMc.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


/* TBD */
#if 0
APP_DEMO_IPFIX_CONFIG_PARAM_ENT appDemoConfigParam;
static GT_U32 keyTableIndexRestore;
static GT_U32 Ipv4TcpKeyTableIndexRestore;
static GT_U32 Ipv4OtherKeyTableIndexRestore;
#endif

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/******************************* Test packets **********************************/

/****************** packet 1 (IPv4 TCP matched packet) ********************/

/* packet's User Priority */
#define PRV_TGF_UP_PACKET_CNS         0

/* default Outer VLAN ID */
#define PRV_TGF_OUTER_VLANID_CNS            1

/* default VLAN ID */
#define PRV_TGF_VLANID_CNS            5

/* stored default VID */
static GT_U16   prvTgfDefVlanId = 0;

/******************************* TCP packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacket1L2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x11}                 /* saMac */
};

/* ethertype part of packet1 */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacket1EtherTypePart = {
    TGF_ETHERTYPE_0800_IPV4_TAG_CNS
};

/* Outer VLAN_TAG part of packet1 */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacket1OuterVlanTagPart = {
    TGF_ETHERTYPE_88A8_SERVICE_TAG_CNS,                     /* etherType */
    PRV_TGF_UP_PACKET_CNS, 0, PRV_TGF_OUTER_VLANID_CNS      /* pri, cfi, VlanId */
};

/* VLAN_TAG part of packet1 */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacket1VlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                     /* etherType */
    PRV_TGF_UP_PACKET_CNS, 0, PRV_TGF_VLANID_CNS         /* pri, cfi, VlanId */
};

/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacket1Ipv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x2E,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    6,                  /* protocol */
    0x4C9B,             /* csum */
    {22, 22, 22, 22},   /* srcAddr */
    { 1,  1,  1,  3}    /* dstAddr */
};

/* packet's TCP part */
static TGF_PACKET_TCP_STC prvTgfPacket1TcpPart =
{
    1,                  /* src port */
    2,                  /* dst port */
    1,                  /* sequence number */
    2,                  /* acknowledgment number */
    0,                  /* data offset */
    0,                  /* reserved */
    0x00,               /* flags */
    4096,               /* window */
    0xC1A9,             /* csum */
    0                   /* urgent pointer */
};

/* DATA of packet */
static GT_U8 prvTgfPacket1PayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00, 0x08,
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacket1PayloadPart = {
    sizeof(prvTgfPacket1PayloadDataArr),                 /* dataLength */
    prvTgfPacket1PayloadDataArr                          /* dataPtr */
};

/* PARTS of packet IPv4 */
static TGF_PACKET_PART_STC prvTgfPacket1PartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacket1L2Part}, /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacket1VlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacket1EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacket1Ipv4Part},
    {TGF_PACKET_PART_TCP_E,       &prvTgfPacket1TcpPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacket1PayloadPart}
};

/* PACKET1 to send */
static TGF_PACKET_STC prvTgfPacket1Info = {
    (TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS
     + TGF_IPV4_HEADER_SIZE_CNS + TGF_TCP_HEADER_SIZE_CNS
     + sizeof(prvTgfPacket1PayloadDataArr)),                      /* totalLen */
    sizeof(prvTgfPacket1PartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacket1PartArray                                        /* partsArray */
};

/* PARTS of QinQ packet IPv4 */
static TGF_PACKET_PART_STC prvTgfQinQPacket1PartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacket1L2Part}, /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacket1OuterVlanTagPart},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacket1VlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacket1EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacket1Ipv4Part},
    {TGF_PACKET_PART_TCP_E,       &prvTgfPacket1TcpPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacket1PayloadPart}
};

/* PACKET1 to send */
static TGF_PACKET_STC prvTgfQinQPacket1Info = {
    (TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS
     + TGF_ETHERTYPE_SIZE_CNS + TGF_IPV4_HEADER_SIZE_CNS + TGF_TCP_HEADER_SIZE_CNS
     + sizeof(prvTgfPacket1PayloadDataArr)),                      /* totalLen */
    sizeof(prvTgfQinQPacket1PartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfQinQPacket1PartArray                                        /* partsArray */
};

/******************************* UDP packet **********************************/

/* packet's UDP part */
static TGF_PACKET_UDP_STC prvTgfPacket1UdpPart =
{
    8,                  /* src port */
    0,                  /* dst port */
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,  /* length */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS /* csum */
};

/* packet's IPv4 part */
static TGF_PACKET_IPV4_STC prvTgfPacket1Ipv4UdpPart =
{
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x36,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    TGF_PROTOCOL_UDP_E, /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS, /* csum */
    {22, 22, 22, 22},   /* srcAddr */
    { 1,  1,  1,  3}    /* dstAddr */
};

/* DATA of UDP packet */
static GT_U8 prvTgfPacket1UdpPayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacket1UdpPayloadPart = {
    sizeof(prvTgfPacket1UdpPayloadDataArr),                 /* dataLength */
    prvTgfPacket1UdpPayloadDataArr                          /* dataPtr */
};

/* PARTS of packet UDP */
static TGF_PACKET_PART_STC prvTgfIpv4UdpPacketArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacket1L2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacket1VlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacket1EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacket1Ipv4UdpPart},
    {TGF_PACKET_PART_UDP_E,       &prvTgfPacket1UdpPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacket1UdpPayloadPart}
};

static TGF_PACKET_STC prvTgfIpv4UdpPacketInfo = {
    (TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS
     + TGF_IPV4_HEADER_SIZE_CNS + TGF_UDP_HEADER_SIZE_CNS
     + sizeof(prvTgfPacket1UdpPayloadDataArr)),                      /* totalLen */
    sizeof(prvTgfIpv4UdpPacketArray) / sizeof(TGF_PACKET_PART_STC),  /* numOfParts */
    prvTgfIpv4UdpPacketArray                                         /* partsArray */
};

/* PARTS of QinQ packet UDP */
static TGF_PACKET_PART_STC prvTgfQinQIpv4UdpPacketArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacket1L2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacket1OuterVlanTagPart},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacket1VlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacket1EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacket1Ipv4UdpPart},
    {TGF_PACKET_PART_UDP_E,       &prvTgfPacket1UdpPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacket1UdpPayloadPart}
};

static TGF_PACKET_STC prvTgfQinQIpv4UdpPacketInfo = {
    (TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS
     + TGF_ETHERTYPE_SIZE_CNS + TGF_IPV4_HEADER_SIZE_CNS + TGF_UDP_HEADER_SIZE_CNS
     + sizeof(prvTgfPacket1UdpPayloadDataArr)),                      /* totalLen */
    sizeof(prvTgfQinQIpv4UdpPacketArray) / sizeof(TGF_PACKET_PART_STC),  /* numOfParts */
    prvTgfQinQIpv4UdpPacketArray                                         /* partsArray */
};

/******************************* OTHER packet **********************************/

/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4OtherPart =
{
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x2E,               /* totalLen */
    0,                  /* id */
    4,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    4,                  /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS, /* csum */
    {2, 7, 8, 9},       /* srcAddr */
    { 1,  2,  1,  3}    /* dstAddr */
};

/* PARTS of other packet IPv4 */
static TGF_PACKET_PART_STC prvTgfOtherPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacket1L2Part}, /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacket1VlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacket1EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4OtherPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacket1PayloadPart}
};

/* PACKET1 to send */
static TGF_PACKET_STC prvTgfOtherPacketInfo = {
    (TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS
     + TGF_IPV4_HEADER_SIZE_CNS
     + sizeof(prvTgfPacket1PayloadDataArr)),                          /* totalLen */
    sizeof(prvTgfOtherPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfOtherPacketPartArray                                        /* partsArray */
};

/* PARTS of QinQ other packet */
static TGF_PACKET_PART_STC prvTgfQinQOtherPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacket1L2Part}, /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacket1OuterVlanTagPart},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacket1VlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacket1EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4OtherPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacket1PayloadPart}
};

/* PACKET1 to send */
static TGF_PACKET_STC prvTgfQinQOtherPacketInfo = {
    (TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS
     + TGF_ETHERTYPE_SIZE_CNS + TGF_IPV4_HEADER_SIZE_CNS
     + sizeof(prvTgfPacket1PayloadDataArr)),                          /* totalLen */
    sizeof(prvTgfQinQOtherPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfQinQOtherPacketPartArray                                        /* partsArray */
};

/* AUTODOC: Test for L3 Source IP for TCP packets*/
static GT_U8 testSrcIpAddr[4][4] = { {1,2,3,4},
  {2,3,4,5},
  {3,4,5,6},
  {4,5,6,7} };

/* AUTODOC: Test for L4 Source Port for TCP packets*/
static TGF_L4_PORT testSrcPort[] = {8, 18, 28, 38};

/* AUTODOC: Test for L4 Destination Port for TCP packets*/
static TGF_L4_PORT testDstPort[] = {5, 15, 25, 35};

/* AUTODOC: Test for L2 VLAN for TCP packets*/
static TGF_L4_PORT testVlanId[] = {4, 16, 64, 256};

/* AUTODOC: Test for L2 Outer VLAN */
static TGF_L4_PORT testOuterVlanId[] = {6, 36, 216, 254};

#define PRV_TGF_IPFIX_FLOW_COUNT_OF_FRAMES_TO_TRANSMIT_CNS 1
#define PRV_TGF_SEND_PORT_IDX_CNS 0
#define PRV_TGF_SEND_PORT1_IDX_CNS 1
#define PRV_TGF_SEND_PORT2_IDX_CNS 2

/**
* @internal prvTgfFlowManagerInit function
* @endinternal
*
* @brief Flow manager service initialization.
*
* @param[in] flowMngId                - unique flow manager id
* @param[in] flowFieldEnable          - to enable / disable flow field config
                                       GT_TRUE -- flow field enable
                                       GT_FALSE -- flow field disable
* @param[in] packetType               - flow based packet type
*/
GT_STATUS prvTgfFlowManagerInit
(
    IN GT_U32                                 flowMngId,
    IN GT_BOOL                                flowFieldEnable,
    IN CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_ENT packetType
)
{
    GT_STATUS                rc = GT_OK;
    GT_U32                   portNum = 0;
    GT_BOOL                  enable  = GT_TRUE;

    rc = appDemoIpfixInit(flowMngId);
    if (GT_OK != rc)
    {
        cpssOsPrintf("[%s:%d] appDemoIpfixInit FAILED,"
            "rc[%d]\n",__FUNCNAME__, __LINE__, rc);
        return rc;
    }

    switch(packetType)
    {
        case CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_TCP_E:
          /* Configuring flow field config for IPv4-TCP pkt type as all applicable fields */
          rc = appDemoIpfixIpv4TcpFlowFieldConfig(flowMngId, flowFieldEnable);
          if (GT_OK != rc)
          {
              cpssOsPrintf("[%s:%d] appDemoIpfixIpv4TcpFlowFieldConfig FAILED,"
                  "devNum [%d] rc[%d]\n",__FUNCNAME__, __LINE__, prvTgfDevNum, rc);
              return rc;
          }
          break;
        case CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_UDP_E:
            /* Configuring flow field config for IPv4-UDP pkt type as all applicable fields */
            rc = appDemoIpfixIpv4UdpFlowFieldConfig(flowMngId, flowFieldEnable);
            if (GT_OK != rc)
            {
                cpssOsPrintf("[%s:%d] appDemoIpfixIpv4UdpFlowFieldConfig FAILED,"
                    "devNum [%d] rc[%d]\n",__FUNCNAME__, __LINE__, prvTgfDevNum, rc);
                return rc;
            }
            break;
        case CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_OTHER_E:
            /* Configuring flow field config for IPv4-UDP pkt type as all applicable fields */
            rc = appDemoIpfixOtherFlowFieldConfig(flowMngId, flowFieldEnable);
            if (GT_OK != rc)
            {
                cpssOsPrintf("[%s:%d] appDemoIpfixOtherFlowFieldConfig FAILED,"
                    "devNum [%d] rc[%d]\n",__FUNCNAME__, __LINE__, prvTgfDevNum, rc);
                return rc;
            }
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            break;
    }

    rc = appDemoIpfixPortEnableSet(flowMngId, prvTgfDevNum, portNum, enable);
    if (GT_OK != rc)
    {
        cpssOsPrintf("[%s:%d] prvTgfFlowManagerPortEnableSet FAILED,"
                     "devNum [%d] rc[%d]\n",__FUNCNAME__, __LINE__,
                     prvTgfDevNum, rc);
        return rc;
    }

    return rc;
}

/**
* @internal prvTgfFlowManagerQinQEnableSet function
* @endinternal
*
* @brief Enable QinQ classification for flows of different packet types.
*
* @param[in] enableQinQ                - enable/disable QinQ
*                                       GT_TRUE - enable QinQ
*                                       GT_FALSE - disable QinQ
*/
GT_STATUS prvTgfFlowManagerQinQEnableSet
(
    IN  GT_BOOL                                enableQinQ
)
{

    appDemoIpfixQinQEnableSet(enableQinQ);
    return GT_OK;
}

/**
* @internal prvTgfFlowManagerQinQInit function
* @endinternal
*
* @brief Flow manager QinQ service initialization.
*
* @param[in] flowMngId                - unique flow manager id
* @param[in] flowFieldEnable          - to enable / disable flow field config
                                       GT_TRUE -- flow field enable
                                       GT_FALSE -- flow field disable
* @param[in] packetType               - flow based packet type
*/
GT_STATUS prvTgfFlowManagerQinQInit
(
    IN GT_U32                                 flowMngId,
    IN GT_BOOL                                flowFieldEnable,
    IN CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_ENT packetType
)
{
    GT_STATUS                rc = GT_OK;
    GT_U32                   portNum = 0;
    GT_BOOL                  enable  = GT_TRUE;

    prvTgfFlowManagerQinQEnableSet(GT_TRUE);

    rc = appDemoIpfixInit(flowMngId);
    if (GT_OK != rc)
    {
        cpssOsPrintf("[%s:%d] appDemoIpfixInit FAILED,"
            "rc[%d]\n",__FUNCNAME__, __LINE__, rc);
        return rc;
    }

    switch(packetType)
    {
        case CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_TCP_E:
          /* Configuring flow field config for IPv4-TCP pkt type as all applicable fields */
          rc = appDemoIpfixIpv4TcpFlowFieldConfig(flowMngId, flowFieldEnable);
          if (GT_OK != rc)
          {
              cpssOsPrintf("[%s:%d] appDemoIpfixIpv4TcpFlowFieldConfig FAILED,"
                  "devNum [%d] rc[%d]\n",__FUNCNAME__, __LINE__, prvTgfDevNum, rc);
              return rc;
          }
          break;
        case CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_UDP_E:
            /* Configuring flow field config for IPv4-UDP pkt type as all applicable fields */
            rc = appDemoIpfixIpv4UdpFlowFieldConfig(flowMngId, flowFieldEnable);
            if (GT_OK != rc)
            {
                cpssOsPrintf("[%s:%d] appDemoIpfixIpv4UdpFlowFieldConfig FAILED,"
                    "devNum [%d] rc[%d]\n",__FUNCNAME__, __LINE__, prvTgfDevNum, rc);
                return rc;
            }
            break;
        case CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_OTHER_E:
            /* Configuring flow field config for IPv4-UDP pkt type as all applicable fields */
            rc = appDemoIpfixOtherFlowFieldConfig(flowMngId, flowFieldEnable);
            if (GT_OK != rc)
            {
                cpssOsPrintf("[%s:%d] appDemoIpfixOtherFlowFieldConfig FAILED,"
                    "devNum [%d] rc[%d]\n",__FUNCNAME__, __LINE__, prvTgfDevNum, rc);
                return rc;
            }
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            break;
    }

    rc = appDemoIpfixPortEnableSet(flowMngId, prvTgfDevNum, portNum, enable);
    if (GT_OK != rc)
    {
        cpssOsPrintf("[%s:%d] prvTgfFlowManagerPortEnableSet FAILED,"
                     "devNum [%d] rc[%d]\n",__FUNCNAME__, __LINE__,
                     prvTgfDevNum, rc);
        return rc;
    }

    return rc;
}

/**
* @internal prvTgfFlowManagerMixedFlowInit function
* @endinternal
*
* @brief Flow manager mixed flow service initialization.
*
* @param[in] flowMngId                - unique flow manager id
* @param[in] flowFieldEnable          - to enable / disabl flow field config
                                       GT_TRUE -- flow field enable
                                       GT_FALSE -- flow field disable
*/
GT_STATUS prvTgfFlowManagerMixedFlowInit
(
    IN GT_U32                                 flowMngId,
    IN GT_BOOL                                flowFieldEnable
)
{
    GT_STATUS                rc = GT_OK;
    GT_BOOL                  enable = GT_TRUE;

    rc = appDemoIpfixInit(flowMngId);
    if (GT_OK != rc)
    {
        cpssOsPrintf("[%s:%d] prvTgfFlowManagerMixedFlowInit FAILED,"
            "rc[%d]\n",__FUNCNAME__, __LINE__, rc);
        return rc;
    }

    /* Configuring flow field config for IPv4-TCP pkt type as all applicable fields */
    rc = appDemoIpfixIpv4TcpFlowFieldConfig(flowMngId, flowFieldEnable);
    if (GT_OK != rc)
    {
        cpssOsPrintf("[%s:%d] appDemoIpfixIpv4TcpFlowFieldConfig FAILED,"
            "devNum [%d] rc[%d]\n",__FUNCNAME__, __LINE__, prvTgfDevNum, rc);
        return rc;
    }

    rc = appDemoIpfixPacketTypeEnableSet(flowMngId, CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_TCP_E, enable);
    if (GT_OK != rc)
    {
        cpssOsPrintf("[%s:%d] appDemoIpfixPacketTypeEnableSet FAILED,"
                     "devNum [%d] rc[%d]\n",__FUNCNAME__, __LINE__,
                     prvTgfDevNum, rc);
        return rc;
    }

    /* Configuring flow field config for IPv4-UDP pkt type as all applicable fields */
    rc = appDemoIpfixIpv4UdpFlowFieldConfig(flowMngId, flowFieldEnable);
    if (GT_OK != rc)
    {
        cpssOsPrintf("[%s:%d] appDemoIpfixIpv4UdpFlowFieldConfig FAILED,"
            "devNum [%d] rc[%d]\n",__FUNCNAME__, __LINE__, prvTgfDevNum, rc);
        return rc;
    }

    rc = appDemoIpfixPacketTypeEnableSet(flowMngId, CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_UDP_E, enable);
    if (GT_OK != rc)
    {
        cpssOsPrintf("[%s:%d] appDemoIpfixPacketTypeEnableSet FAILED,"
                     "devNum [%d] rc[%d]\n",__FUNCNAME__, __LINE__,
                     prvTgfDevNum, rc);
        return rc;
    }

    /* Configuring flow field config for other pkt type as all applicable fields */
    rc = appDemoIpfixOtherFlowFieldConfig(flowMngId, flowFieldEnable);
    if (GT_OK != rc)
    {
        cpssOsPrintf("[%s:%d] appDemoIpfixOtherFlowFieldConfig FAILED,"
            "devNum [%d] rc[%d]\n",__FUNCNAME__, __LINE__, prvTgfDevNum, rc);
        return rc;
    }

    rc = appDemoIpfixPacketTypeEnableSet(flowMngId, CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_OTHER_E, enable);
    if (GT_OK != rc)
    {
        cpssOsPrintf("[%s:%d] appDemoIpfixPacketTypeEnableSet FAILED,"
                     "devNum [%d] rc[%d]\n",__FUNCNAME__, __LINE__,
                     prvTgfDevNum, rc);
        return rc;
    }

    return rc;
}

/**
* @internal prvTgfFlowManagerDelete function
* @endinternal
*
* @brief Flow manager service de-initialization.
*
* @param[in] flowMngId                - unique flow manager id
*/
GT_STATUS prvTgfFlowManagerDelete
(
    IN  GT_U32                                flowMngId
)
{
    GT_STATUS rc = GT_OK;

    rc = appDemoIpfixDelete(flowMngId);
    if (GT_OK != rc)
    {
        cpssOsPrintf("[%s:%d] appDemoIpfixDelete FAILED,"
            "rc[%d]\n",__FUNCNAME__, __LINE__, rc);
        return rc;
    }
    return rc;
}

static GT_STATUS tgfFlowManagerMultiTrafficSend
(
    IN CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_ENT packetType
)
{
    GT_STATUS rc =  GT_OK;
    GT_U32 numOfTestIter = 4;
    GT_U32 i;
    GT_U32 portIter;
    GT_U32 devNum;
    GT_U32 portNum;
    GT_U32 numberOfPortsSending = 1;

    for( i = 0 ; i < numOfTestIter ; i++)
    {
        for(portIter = 0; portIter < numberOfPortsSending; portIter++)
        {
            devNum  = prvTgfDevsArray [portIter];
            portNum = prvTgfPortsArray[portIter];
        }
        switch(packetType)
        {
            case CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_TCP_E:
                cpssOsMemCpy (&prvTgfPacket1Ipv4Part.srcAddr[0], &testSrcIpAddr[i][0], sizeof(GT_U32));
                prvTgfPacket1TcpPart.srcPort = testSrcPort[i];
                prvTgfPacket1TcpPart.dstPort = testDstPort[i];
                prvTgfPacket1VlanTagPart.vid = testVlanId[i];
                /* setup Packet */
                rc = prvTgfSetTxSetupEth(devNum, &prvTgfPacket1Info, 1, 0, NULL);
                if (GT_OK != rc)
                {
                    PRV_UTF_LOG1_MAC("[TGF]: prvTgfSetTxSetupEth FAILED, rc = [%d]", rc);
                    return rc;
                }
                break;
            case CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_UDP_E:
                cpssOsMemCpy (&prvTgfPacket1Ipv4UdpPart.srcAddr[0], &testSrcIpAddr[i][0], sizeof(GT_U32));
                prvTgfPacket1UdpPart.srcPort = testSrcPort[i];
                prvTgfPacket1UdpPart.dstPort = testDstPort[i];
                prvTgfPacket1VlanTagPart.vid = testVlanId[i];
                /* setup Packet */
                rc = prvTgfSetTxSetupEth(devNum, &prvTgfIpv4UdpPacketInfo, 1, 0, NULL);
                if (GT_OK != rc)
                {
                    PRV_UTF_LOG1_MAC("[TGF]: prvTgfSetTxSetupEth FAILED, rc = [%d]", rc);
                    return rc;
                }
                break;
            case CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_OTHER_E:
                cpssOsMemCpy (&prvTgfPacketIpv4OtherPart.srcAddr[0], &testSrcIpAddr[i][0], sizeof(GT_U32));
                prvTgfPacket1VlanTagPart.vid = testVlanId[i];
                /* setup Packet */
                rc = prvTgfSetTxSetupEth(devNum, &prvTgfOtherPacketInfo, 1, 0, NULL);
                if (GT_OK != rc)
                {
                    PRV_UTF_LOG1_MAC("[TGF]: prvTgfSetTxSetupEth FAILED, rc = [%d]", rc);
                    return rc;
                }
                break;
            default:
                break;
        }

        utfPrintKeepAlive();

        /* send Packet from port portNum */
        rc = prvTgfStartTransmitingEth(devNum, portNum);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfStartTransmitingEth FAILED, rc = [%d]", rc);
            return rc;
        }
    }
    return rc;
}

static GT_STATUS tgfFlowManagerQinQMultiTrafficSend
(
    IN CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_ENT packetType
)
{
    GT_STATUS rc =  GT_OK;
    GT_U32 numOfTestIter = 4;
    GT_U32 i;
    GT_U32 portIter;
    GT_U32 devNum;
    GT_U32 portNum;
    GT_U32 numberOfPortsSending = 1;

    for( i = 0 ; i < numOfTestIter ; i++)
    {
        for(portIter = 0; portIter < numberOfPortsSending; portIter++)
        {
            devNum  = prvTgfDevsArray [portIter];
            portNum = prvTgfPortsArray[portIter];
        }
        switch(packetType)
        {
            case CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_TCP_E:
                cpssOsMemCpy (&prvTgfPacket1Ipv4Part.srcAddr[0], &testSrcIpAddr[i][0], sizeof(GT_U32));
                prvTgfPacket1TcpPart.srcPort      = testSrcPort[i];
                prvTgfPacket1TcpPart.dstPort      = testDstPort[i];
                prvTgfPacket1OuterVlanTagPart.vid = testOuterVlanId[i];
                prvTgfPacket1VlanTagPart.vid      = testVlanId[i];
                /* setup Packet */
                rc = prvTgfSetTxSetupEth(devNum, &prvTgfQinQPacket1Info, 1, 0, NULL);
                if (GT_OK != rc)
                {
                    PRV_UTF_LOG1_MAC("[TGF]: prvTgfSetTxSetupEth FAILED, rc = [%d]", rc);
                    return rc;
                }
                break;
            case CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_UDP_E:
                cpssOsMemCpy (&prvTgfPacket1Ipv4UdpPart.srcAddr[0], &testSrcIpAddr[i][0], sizeof(GT_U32));
                prvTgfPacket1UdpPart.srcPort      = testSrcPort[i];
                prvTgfPacket1UdpPart.dstPort      = testDstPort[i];
                prvTgfPacket1OuterVlanTagPart.vid = testOuterVlanId[i];
                prvTgfPacket1VlanTagPart.vid      = testVlanId[i];
                /* setup Packet */
                rc = prvTgfSetTxSetupEth(devNum, &prvTgfQinQIpv4UdpPacketInfo, 1, 0, NULL);
                if (GT_OK != rc)
                {
                    PRV_UTF_LOG1_MAC("[TGF]: prvTgfSetTxSetupEth FAILED, rc = [%d]", rc);
                    return rc;
                }
                break;
            case CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_OTHER_E:
                cpssOsMemCpy (&prvTgfPacketIpv4OtherPart.srcAddr[0], &testSrcIpAddr[i][0], sizeof(GT_U32));
                prvTgfPacket1OuterVlanTagPart.vid = testOuterVlanId[i];
                prvTgfPacket1VlanTagPart.vid      = testVlanId[i];
                /* setup Packet */
                rc = prvTgfSetTxSetupEth(devNum, &prvTgfQinQOtherPacketInfo, 1, 0, NULL);
                if (GT_OK != rc)
                {
                    PRV_UTF_LOG1_MAC("[TGF]: prvTgfSetTxSetupEth FAILED, rc = [%d]", rc);
                    return rc;
                }
                break;
            default:
                break;
        }

        utfPrintKeepAlive();

        /* send Packet from port portNum */
        rc = prvTgfStartTransmitingEth(devNum, portNum);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfStartTransmitingEth FAILED, rc = [%d]", rc);
            return rc;
        }
    }
    return rc;
}

/**
* @internal tgfPortTxFlowManagerResourcesTcpTrafficGenerate function
* @endinternal
*
* @brief Generate tcp traffic for flow manager service test.
*
*/
GT_VOID tgfPortTxFlowManagerResourcesMultiTrafficGenerate
(
    IN CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_ENT packetType
)
{
    GT_STATUS                           st = GT_OK;
    GT_U32                              portIndex;
    GT_U8                               tagArray[] = {0, 1, 0, 0};
    PRV_CPSS_DXCH_FLOW_MANAGER_STC      *flowMngDbPtr;
    GT_U32                              flowIdArr[4] = {0,1,2,3};
    GT_U32                              flowId[4] = {0,0,0,0};
    GT_U32                              flowIdCount;
    GT_U32                              flowIdGet;
    GT_U32                              iter;
    GT_BOOL                             getFirst;
    GT_BOOL                             isEqual;
    GT_U32                              sourcePort;
    CPSS_DXCH_FLOW_MANAGER_FLOW_ENTRY_STC flowEntry;
    CPSS_DXCH_FLOW_MANAGER_FLOW_STC       flowData;
    CPSS_DXCH_FLOW_MANAGER_PORT_FLOWS_LEARNT_STC flowsLearnt;

    cpssOsMemSet(&flowEntry, 0, sizeof(CPSS_DXCH_FLOW_MANAGER_FLOW_ENTRY_STC));
    cpssOsMemSet(&flowData, 0, sizeof(CPSS_DXCH_FLOW_MANAGER_FLOW_STC));
    cpssOsMemSet(&flowsLearnt, 0, sizeof(CPSS_DXCH_FLOW_MANAGER_PORT_FLOWS_LEARNT_STC));

    /* AUTODOC: create VLAN 5 with all tagged port 1, untagged ports 0,2,3 */
    st = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_CNS,
            prvTgfPortsArray, NULL, tagArray, prvTgfPortsNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvTgfBrgDefVlanEntryWithPortsSet");

    /* get default vlanId */
    st = prvTgfBrgVlanPortVidGet(prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 &prvTgfDefVlanId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvTgfBrgVlanPortVidSet");

    /* AUTODOC: set PVID 5 for port 0 */
    st = prvTgfBrgVlanPortVidSet(prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvTgfBrgVlanPortVidSet");

    st = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "tgfTrafficTableRxPcktTblClear");

    st = tgfTrafficTableRxStartCapture(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "tgfTrafficTableRxStartCapture");

    /* AUTODOC: Clear counters */
    st = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvTgfEthCountersReset");

    for (portIndex = 1; (portIndex < prvTgfPortsNum); portIndex++)
    {
        st = cpssDxChBrgMcMemberDelete(prvTgfDevNum, 0xFFF, prvTgfPortsArray[portIndex]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChBrgMcMemberDelete: %d", prvTgfPortsArray[portIndex]);
    }

    tgfTrafficTracePacketByteSet(GT_TRUE);
    /* send multiple packets with different SIP */
    st = tgfFlowManagerMultiTrafficSend(packetType);
    if (GT_OK != st)
    {
        PRV_UTF_LOG1_MAC("[TGF]: tgfFlowManagerMultiTrafficSend FAILED, rc = [%d]", st);
    }

    PM_TBD
    if(GT_FALSE == prvUtfIsPortManagerUsed())
    {
        cpssOsTimerWkAfter(1000);
    }
    else
    {
        cpssOsTimerWkAfter(10000);
    }

    flowMngDbPtr = prvCpssDxChFlowManagerDbFlowMngGet(1);

    UTF_VERIFY_EQUAL0_STRING_MAC(1,
        flowMngDbPtr->seqNoCpss, "flow sequence number is different than expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(4,
        flowMngDbPtr->messageNewFlowCount, "new flow count is different than expected");

    /* Validate flow data get next call */
    getFirst = GT_TRUE;
    st = cpssDxChFlowManagerFlowIdGetNext(1, getFirst, &flowId[0]);
    UTF_VERIFY_EQUAL0_STRING_MAC(flowMngDbPtr->flowDataDb[flowId[0]].flowId,
        flowId[0], "flow id is different than expected");
    getFirst = GT_FALSE;
    iter = 1;
    flowIdGet = 2;
    do
    {
        st = cpssDxChFlowManagerFlowIdGetNext(1, getFirst, &flowId[iter]);
        UTF_VERIFY_EQUAL0_STRING_MAC(flowMngDbPtr->flowDataDb[flowIdGet].flowId,
            flowId[iter], "flow id is different than expected");
        iter++;
        if(iter > 3)
            break;
        flowIdGet--;
    } while(st != GT_NO_MORE);

    /* validate flow counters as in cpss db */
    flowIdCount = 4;
    flowIdArr[0] = flowId[3];
    flowIdArr[1] = flowId[2];
    flowIdArr[2] = flowId[1];
    flowIdArr[3] = flowId[0];

    st = cpssDxChFlowManagerFlowCountersToDbGet(1, flowIdCount, flowIdArr);

    if(GT_FALSE == prvUtfIsPortManagerUsed())
    {
        cpssOsTimerWkAfter(1000);
    }
    else
    {
        cpssOsTimerWkAfter(10000);
    }


    /* validate flow counters as per cpss db for all 5 flows */
    UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[flowId[3]].flowId, flowId[3], "Existing flow id %d"
                                 "different than in DB %d",0x0, flowMngDbPtr->flowCounterDb[flowId[3]].flowId);
    UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[flowId[3]].packetCount, 0x01, "Existing flow packet count %d"
                                    "different than in DB %d",0x01, flowMngDbPtr->flowCounterDb[flowId[3]].packetCount);
    UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[flowId[3]].dropCount, 0x0, "Existing flow drop count %d"
                                 "different than in DB %d",0x0, flowMngDbPtr->flowCounterDb[flowId[3]].dropCount);

    UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[flowId[2]].flowId, flowId[2], "Existing flow id %d"
                                 "different than in DB %d",0x1, flowMngDbPtr->flowCounterDb[flowId[2]].flowId);
    UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[flowId[2]].packetCount, 0x01, "Existing flow packet count %d"
                                    "different than in DB %d",0x01, flowMngDbPtr->flowCounterDb[flowId[2]].packetCount);
    UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[flowId[2]].dropCount, 0x0, "Existing flow drop count %d"
                                 "different than in DB %d",0x0, flowMngDbPtr->flowCounterDb[flowId[2]].dropCount);

    UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[flowId[1]].flowId, flowId[1], "Existing flow id %d"
                                 "different than in DB %d",0x2, flowMngDbPtr->flowCounterDb[flowId[1]].flowId);
    UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[flowId[1]].packetCount, 0x01, "Existing flow packet count %d"
                                    "different than in DB %d",0x01, flowMngDbPtr->flowCounterDb[flowId[1]].packetCount);
    UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[flowId[1]].dropCount, 0x0, "Existing flow drop count %d"
                                 "different than in DB %d",0x0, flowMngDbPtr->flowCounterDb[flowId[1]].dropCount);

    UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[flowId[0]].flowId, flowId[0], "Existing flow id %d"
                                 "different than in DB %d",0x3, flowMngDbPtr->flowCounterDb[flowId[0]].flowId);
    UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[flowId[0]].packetCount, 0x01, "Existing flow packet count %d"
                                    "different than in DB %d",0x01, flowMngDbPtr->flowCounterDb[flowId[0]].packetCount);
    UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[flowId[0]].dropCount, 0x0, "Existing flow drop count %d"
                                 "different than in DB %d",0x0, flowMngDbPtr->flowCounterDb[flowId[0]].dropCount);

    if(packetType == CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_TCP_E)
    {
        UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[flowId[3]].byteCount, 0x59, "Existing flow byte count %d"
            "different than in DB %d",0x59, flowMngDbPtr->flowCounterDb[flowId[3]].byteCount);
        UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[flowId[2]].byteCount, 0x59, "Existing flow byte count %d"
            "different than in DB %d",0x59, flowMngDbPtr->flowCounterDb[flowId[2]].byteCount);
        UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[flowId[1]].byteCount, 0x59, "Existing flow byte count %d"
            "different than in DB %d",0x59, flowMngDbPtr->flowCounterDb[flowId[1]].byteCount);
        UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[flowId[0]].byteCount, 0x59, "Existing flow byte count %d"
            "different than in DB %d",0x59, flowMngDbPtr->flowCounterDb[flowId[0]].byteCount);
    }
    else if(packetType == CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_UDP_E)
    {
        UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[flowId[3]].byteCount, 0x40, "Existing flow byte count %d"
            "different than in DB %d",0x40, flowMngDbPtr->flowCounterDb[flowId[3]].byteCount);
        UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[flowId[2]].byteCount, 0x40, "Existing flow byte count %d"
            "different than in DB %d",0x40, flowMngDbPtr->flowCounterDb[flowId[2]].byteCount);
        UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[flowId[1]].byteCount, 0x40, "Existing flow byte count %d"
            "different than in DB %d",0x40, flowMngDbPtr->flowCounterDb[flowId[1]].byteCount);
        UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[flowId[0]].byteCount, 0x40, "Existing flow byte count %d"
            "different than in DB %d",0x40, flowMngDbPtr->flowCounterDb[flowId[0]].byteCount);
    }
    else if(packetType == CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_OTHER_E)
    {
        UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[flowId[3]].byteCount, 0x45, "Existing flow byte count %d"
            "different than in DB %d",0x45, flowMngDbPtr->flowCounterDb[flowId[3]].byteCount);
        UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[flowId[2]].byteCount, 0x45, "Existing flow byte count %d"
            "different than in DB %d",0x45, flowMngDbPtr->flowCounterDb[flowId[2]].byteCount);
        UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[flowId[1]].byteCount, 0x45, "Existing flow byte count %d"
            "different than in DB %d",0x45, flowMngDbPtr->flowCounterDb[flowId[1]].byteCount);
        UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[flowId[0]].byteCount, 0x45, "Existing flow byte count %d"
            "different than in DB %d",0x45, flowMngDbPtr->flowCounterDb[flowId[0]].byteCount);
    }
    else
    {
    }

    /* validate flow data for given flow id */
    st = cpssDxChFlowManagerFlowDataGet(1, flowId[3], &flowEntry, &flowData);
    UTF_VERIFY_EQUAL0_STRING_MAC(0x1e,
        flowData.keySize, "flow key size is different than expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(flowId[3],
        flowEntry.flowId, "flow id is different than expected");

    if(packetType == CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_TCP_E)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(flowId[3],
            flowData.keyId, "flow key id is different than expected");
        /* Validate IPV4-TCP flow key data */
        UTF_VERIFY_EQUAL0_STRING_MAC(0x4,
            flowMngDbPtr->flowDataDb[flowId[3]].keyData[1], "flow vlan is different than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            flowMngDbPtr->flowDataDb[flowId[3]].keyData[2], "flow source port different than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0x1,
            flowMngDbPtr->flowDataDb[flowId[3]].keyData[3], "flow applicable flow sub-template different than expected");
        isEqual = (cpssOsMemCmp(&(flowMngDbPtr->flowDataDb[flowId[3]].keyData[4]),&testSrcIpAddr[0][0], sizeof(GT_U32))==0) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL0_STRING_MAC(isEqual,
            0x1, "flow src ip different than expected");
        isEqual = (cpssOsMemCmp(&(flowMngDbPtr->flowDataDb[flowId[3]].keyData[8]),&prvTgfPacket1Ipv4Part.dstAddr[0], sizeof(GT_U32))==0) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL0_STRING_MAC(isEqual,
            0x1, "flow dst ip different than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0x8,
            flowMngDbPtr->flowDataDb[flowId[3]].keyData[13], "flow tcp src port different than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0x5,
            flowMngDbPtr->flowDataDb[flowId[3]].keyData[15], "flow tcp dst port different than expected");
    }
    else if(packetType == CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_UDP_E)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(0x1,
            flowData.keyId, "flow key id is different than expected");
        /* Validate IPV4-UDP flow key data */
        UTF_VERIFY_EQUAL0_STRING_MAC(0x4,
            flowMngDbPtr->flowDataDb[flowId[3]].keyData[1], "flow vlan is different than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            flowMngDbPtr->flowDataDb[flowId[3]].keyData[2], "flow source port different than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0x2,
            flowMngDbPtr->flowDataDb[flowId[3]].keyData[3], "flow applicable flow sub-template different than expected");
        isEqual = (cpssOsMemCmp(&(flowMngDbPtr->flowDataDb[flowId[3]].keyData[4]),&testSrcIpAddr[0][0], sizeof(GT_U32))==0) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL0_STRING_MAC(isEqual,
            0x1, "flow src ip different than expected");
        isEqual = (cpssOsMemCmp(&(flowMngDbPtr->flowDataDb[flowId[3]].keyData[8]),&prvTgfPacket1Ipv4Part.dstAddr[0], sizeof(GT_U32))==0) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL0_STRING_MAC(isEqual,
            0x1, "flow dst ip different than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0x8,
            flowMngDbPtr->flowDataDb[flowId[3]].keyData[13], "flow tcp src port different than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0x5,
            flowMngDbPtr->flowDataDb[flowId[3]].keyData[15], "flow tcp dst port different than expected");
    }
    else if(packetType == CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_OTHER_E)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(0x4,
            flowData.keyId, "flow key id is different than expected");
        /* Validate IPV4-OTHER flow key data */
        UTF_VERIFY_EQUAL0_STRING_MAC(0x4,
            flowMngDbPtr->flowDataDb[flowId[3]].keyData[1], "flow vlan is different than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            flowMngDbPtr->flowDataDb[flowId[3]].keyData[2], "flow source port different than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0x20,
            flowMngDbPtr->flowDataDb[flowId[3]].keyData[3], "flow applicable flow sub-template different than expected");
        isEqual = (cpssOsMemCmp(&(flowMngDbPtr->flowDataDb[3].keyData[4]),&(prvTgfPacket1L2Part.saMac[0]), sizeof(GT_U16)*3)==0) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL0_STRING_MAC(isEqual,
            0x1, "flow src ip different than expected");
        isEqual = (cpssOsMemCmp(&(flowMngDbPtr->flowDataDb[flowId[3]].keyData[10]),&(prvTgfPacket1L2Part.daMac[0]), sizeof(GT_U16)*3)==0) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL0_STRING_MAC(isEqual,
            0x1, "flow dst ip different than expected");
    }
    else
    {
    }

    st = cpssDxChFlowManagerFlowDataGet(1, flowId[2], &flowEntry, &flowData);
    UTF_VERIFY_EQUAL0_STRING_MAC(flowId[2],
        flowEntry.flowId, "flow id is different than expected");

    if(packetType == CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_TCP_E)
    {
        /* Validate IPV4-TCP flow key data */
        UTF_VERIFY_EQUAL0_STRING_MAC(0x10,
            flowMngDbPtr->flowDataDb[flowId[2]].keyData[1], "flow vlan is different than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            flowMngDbPtr->flowDataDb[flowId[2]].keyData[2], "flow source port different than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0x1,
            flowMngDbPtr->flowDataDb[flowId[2]].keyData[3], "flow applicable flow sub-template different than expected");
        isEqual = (cpssOsMemCmp(&(flowMngDbPtr->flowDataDb[flowId[2]].keyData[4]),&testSrcIpAddr[1][0], sizeof(GT_U32))==0) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL0_STRING_MAC(isEqual,
            0x1, "flow src ip different than expected");
        isEqual = (cpssOsMemCmp(&(flowMngDbPtr->flowDataDb[flowId[2]].keyData[8]), &prvTgfPacket1Ipv4Part.dstAddr[0], sizeof(GT_U32))==0) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL0_STRING_MAC(isEqual,
            0x1, "flow dst ip different than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0x12,
            flowMngDbPtr->flowDataDb[flowId[2]].keyData[13], "flow tcp src port different than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0xF,
            flowMngDbPtr->flowDataDb[flowId[2]].keyData[15], "flow tcp dst port different than expected");
    }
    else if(packetType == CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_UDP_E)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(0x1,
            flowData.keyId, "flow key id is different than expected");
        /* Validate IPV4-UDP flow key data */
        UTF_VERIFY_EQUAL0_STRING_MAC(0x10,
            flowMngDbPtr->flowDataDb[flowId[2]].keyData[1], "flow vlan is different than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            flowMngDbPtr->flowDataDb[flowId[2]].keyData[2], "flow source port different than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0x2,
            flowMngDbPtr->flowDataDb[flowId[2]].keyData[3], "flow applicable flow sub-template different than expected");
        isEqual = (cpssOsMemCmp(&(flowMngDbPtr->flowDataDb[flowId[2]].keyData[4]),&testSrcIpAddr[1][0], sizeof(GT_U32))==0) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL0_STRING_MAC(isEqual,
            0x1, "flow src ip different than expected");
        isEqual = (cpssOsMemCmp(&(flowMngDbPtr->flowDataDb[flowId[2]].keyData[8]), &prvTgfPacket1Ipv4Part.dstAddr[0], sizeof(GT_U32))==0) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL0_STRING_MAC(isEqual,
            0x1, "flow dst ip different than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0x12,
            flowMngDbPtr->flowDataDb[flowId[2]].keyData[13], "flow tcp src port different than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0xF,
            flowMngDbPtr->flowDataDb[flowId[2]].keyData[15], "flow tcp dst port different than expected");
    }
    else if(packetType == CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_OTHER_E)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(0x4,
            flowData.keyId, "flow key id is different than expected");
        /* Validate IPV4-OTHER flow key data */
        UTF_VERIFY_EQUAL0_STRING_MAC(0x10,
            flowMngDbPtr->flowDataDb[flowId[2]].keyData[1], "flow vlan is different than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            flowMngDbPtr->flowDataDb[flowId[2]].keyData[2], "flow source port different than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0x20,
            flowMngDbPtr->flowDataDb[flowId[2]].keyData[3], "flow applicable flow sub-template different than expected");
        isEqual = (cpssOsMemCmp(&(flowMngDbPtr->flowDataDb[flowId[2]].keyData[4]),&(prvTgfPacket1L2Part.saMac[0]), sizeof(GT_U16)*3)==0) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL0_STRING_MAC(isEqual,
            0x1, "flow src ip different than expected");
        isEqual = (cpssOsMemCmp(&(flowMngDbPtr->flowDataDb[flowId[2]].keyData[10]),&(prvTgfPacket1L2Part.daMac[0]), sizeof(GT_U16)*3)==0) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL0_STRING_MAC(isEqual,
            0x1, "flow dst ip different than expected");
    }
    else
    {
    }
    st = cpssDxChFlowManagerFlowDataGet(1, flowId[1], &flowEntry, &flowData);
    UTF_VERIFY_EQUAL0_STRING_MAC(flowId[1],
        flowEntry.flowId, "flow id is different than expected");
    if(packetType == CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_TCP_E)
    {
        /* Validate IPV4-TCP flow key data */
        UTF_VERIFY_EQUAL0_STRING_MAC(0x40,
            flowMngDbPtr->flowDataDb[flowId[1]].keyData[1], "flow vlan is different than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            flowMngDbPtr->flowDataDb[flowId[1]].keyData[2], "flow source port different than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0x1,
            flowMngDbPtr->flowDataDb[flowId[1]].keyData[3], "flow applicable flow sub-template different than expected");
        isEqual = (cpssOsMemCmp(&(flowMngDbPtr->flowDataDb[flowId[1]].keyData[4]),&testSrcIpAddr[2][0], sizeof(GT_U32))==0) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL0_STRING_MAC(isEqual,
            0x1, "flow src ip different than expected");
        isEqual = (cpssOsMemCmp(&(flowMngDbPtr->flowDataDb[flowId[1]].keyData[8]), &prvTgfPacket1Ipv4Part.dstAddr[0], sizeof(GT_U32))==0) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL0_STRING_MAC(isEqual,
            0x1, "flow dst ip different than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0x1c,
            flowMngDbPtr->flowDataDb[flowId[1]].keyData[13], "flow tcp src port different than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0x19,
            flowMngDbPtr->flowDataDb[flowId[1]].keyData[15], "flow tcp dst port different than expected");
    }
    else if(packetType == CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_UDP_E)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(0x1,
            flowData.keyId, "flow key id is different than expected");
        /* Validate IPV4-UDP flow key data */
        UTF_VERIFY_EQUAL0_STRING_MAC(0x40,
            flowMngDbPtr->flowDataDb[flowId[1]].keyData[1], "flow vlan is different than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            flowMngDbPtr->flowDataDb[flowId[1]].keyData[2], "flow source port different than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0x2,
            flowMngDbPtr->flowDataDb[flowId[1]].keyData[3], "flow applicable flow sub-template different than expected");
        isEqual = (cpssOsMemCmp(&(flowMngDbPtr->flowDataDb[flowId[1]].keyData[4]),&testSrcIpAddr[2][0], sizeof(GT_U32))==0) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL0_STRING_MAC(isEqual,
            0x1, "flow src ip different than expected");
        isEqual = (cpssOsMemCmp(&(flowMngDbPtr->flowDataDb[flowId[1]].keyData[8]), &prvTgfPacket1Ipv4Part.dstAddr[0], sizeof(GT_U32))==0) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL0_STRING_MAC(isEqual,
            0x1, "flow dst ip different than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0x1c,
            flowMngDbPtr->flowDataDb[flowId[1]].keyData[13], "flow tcp src port different than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0x19,
            flowMngDbPtr->flowDataDb[flowId[1]].keyData[15], "flow tcp dst port different than expected");
    }
    else if(packetType == CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_OTHER_E)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(0x4,
            flowData.keyId, "flow key id is different than expected");
        /* Validate IPV4-OTHER flow key data */
        UTF_VERIFY_EQUAL0_STRING_MAC(0x40,
            flowMngDbPtr->flowDataDb[flowId[1]].keyData[1], "flow vlan is different than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            flowMngDbPtr->flowDataDb[flowId[1]].keyData[2], "flow source port different than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0x20,
            flowMngDbPtr->flowDataDb[flowId[1]].keyData[3], "flow applicable flow sub-template different than expected");
        isEqual = (cpssOsMemCmp(&(flowMngDbPtr->flowDataDb[flowId[1]].keyData[4]),&(prvTgfPacket1L2Part.saMac[0]), sizeof(GT_U16)*3)==0) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL0_STRING_MAC(isEqual,
            0x1, "flow src ip different than expected");
        isEqual = (cpssOsMemCmp(&(flowMngDbPtr->flowDataDb[flowId[1]].keyData[10]),&(prvTgfPacket1L2Part.daMac[0]), sizeof(GT_U16)*3)==0) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL0_STRING_MAC(isEqual,
            0x1, "flow dst ip different than expected");
    }
    else
    {
    }

    st = cpssDxChFlowManagerFlowDataGet(1, flowId[0], &flowEntry, &flowData);
    UTF_VERIFY_EQUAL0_STRING_MAC(flowId[0],
        flowEntry.flowId, "flow id is different than expected");
    if(packetType == CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_TCP_E)
    {
        /* Validate IPV4-TCP flow key data */
        UTF_VERIFY_EQUAL0_STRING_MAC(0x1,
            flowMngDbPtr->flowDataDb[flowId[0]].keyData[0], "flow vlan is different than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            flowMngDbPtr->flowDataDb[flowId[0]].keyData[2], "flow source port different than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0x1,
            flowMngDbPtr->flowDataDb[flowId[0]].keyData[3], "flow applicable flow sub-template different than expected");
        isEqual = (cpssOsMemCmp(&(flowMngDbPtr->flowDataDb[flowId[0]].keyData[4]),&testSrcIpAddr[3][0], sizeof(GT_U32))==0) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL0_STRING_MAC(isEqual,
            0x1, "flow src ip different than expected");
        isEqual = (cpssOsMemCmp(&(flowMngDbPtr->flowDataDb[flowId[0]].keyData[8]), &prvTgfPacket1Ipv4Part.dstAddr[0], sizeof(GT_U32))==0) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL0_STRING_MAC(isEqual,
            0x1, "flow dst ip different than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0x26,
            flowMngDbPtr->flowDataDb[flowId[0]].keyData[13], "flow tcp src port different than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0x23,
            flowMngDbPtr->flowDataDb[flowId[0]].keyData[15], "flow tcp dst port different than expected");
    }
    else if(packetType == CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_UDP_E)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(0x1,
            flowData.keyId, "flow key id is different than expected");
        /* Validate IPV4-UDP flow key data */
        UTF_VERIFY_EQUAL0_STRING_MAC(0x1,
            flowMngDbPtr->flowDataDb[flowId[0]].keyData[0], "flow vlan is different than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            flowMngDbPtr->flowDataDb[flowId[0]].keyData[2], "flow source port different than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0x2,
            flowMngDbPtr->flowDataDb[flowId[0]].keyData[3], "flow applicable flow sub-template different than expected");
        isEqual = (cpssOsMemCmp(&(flowMngDbPtr->flowDataDb[flowId[0]].keyData[4]),&testSrcIpAddr[3][0], sizeof(GT_U32))==0) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL0_STRING_MAC(isEqual,
            0x1, "flow src ip different than expected");
        isEqual = (cpssOsMemCmp(&(flowMngDbPtr->flowDataDb[flowId[0]].keyData[8]), &prvTgfPacket1Ipv4Part.dstAddr[0], sizeof(GT_U32))==0) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL0_STRING_MAC(isEqual,
            0x1, "flow dst ip different than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0x26,
            flowMngDbPtr->flowDataDb[flowId[0]].keyData[13], "flow tcp src port different than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0x23,
            flowMngDbPtr->flowDataDb[flowId[0]].keyData[15], "flow tcp dst port different than expected");
    }
    else if(packetType == CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_OTHER_E)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(0x4,
            flowData.keyId, "flow key id is different than expected");
        /* Validate IPV4-OTHER flow key data */
        UTF_VERIFY_EQUAL0_STRING_MAC(0x0,
            flowMngDbPtr->flowDataDb[flowId[0]].keyData[1], "flow vlan is different than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            flowMngDbPtr->flowDataDb[flowId[0]].keyData[2], "flow source port different than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0x20,
            flowMngDbPtr->flowDataDb[flowId[0]].keyData[3], "flow applicable flow sub-template different than expected");
        isEqual = (cpssOsMemCmp(&(flowMngDbPtr->flowDataDb[flowId[0]].keyData[4]),&(prvTgfPacket1L2Part.saMac[0]), sizeof(GT_U16)*3)==0) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL0_STRING_MAC(isEqual,
            0x1, "flow src ip different than expected");
        isEqual = (cpssOsMemCmp(&(flowMngDbPtr->flowDataDb[flowId[0]].keyData[10]),&(prvTgfPacket1L2Part.daMac[0]), sizeof(GT_U16)*3)==0) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL0_STRING_MAC(isEqual,
            0x1, "flow dst ip different than expected");
    }
    else
    {
    }

    /* validate flows learnt per source port */
    getFirst = GT_TRUE;
    sourcePort = 0;
    st = cpssDxChFlowManagerSrcPortFlowIdGetNext(prvTgfDevNum, sourcePort, getFirst, &flowId[0]);
    UTF_VERIFY_EQUAL2_STRING_MAC(flowId[0], flowMngDbPtr->flowDataDb[flowId[0]].flowId, "Flow Id learnt per port expected %d and got %d",
        flowId[0], 0x4);
    getFirst = GT_FALSE;
    flowIdGet = 2;
    iter = 1;
    do
    {
        st = cpssDxChFlowManagerSrcPortFlowIdGetNext(prvTgfDevNum, sourcePort, getFirst, &flowId[iter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(flowId[iter], flowMngDbPtr->flowDataDb[flowIdGet].flowId, "Flow Id learnt per port expected %d and got %d",
            flowId[iter], flowIdGet);
        iter++;
        if(iter > 3)
            break;
        flowIdGet--;
    } while(st != GT_NO_MORE);

    /* validate flow types learnt per source port */
    st = cpssDxChFlowManagerSrcPortFlowLearntGet(prvTgfDevNum, sourcePort, &flowsLearnt);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_NO_MORE, st, "dev %d, source port %d", prvTgfDevNum, sourcePort);
    if(packetType == CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_TCP_E)
    {
        UTF_VERIFY_EQUAL2_STRING_MAC(flowsLearnt.ipv4TcpFlowsLearnt, 0x4, "IPV4 TCP flow learnt per port expected %d and got %d",
            0x4, flowsLearnt.ipv4TcpFlowsLearnt);
    }
    else if(packetType == CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_UDP_E)
    {
        UTF_VERIFY_EQUAL2_STRING_MAC(flowsLearnt.ipv4UdpFlowsLearnt, 0x4, "IPV4 UDP flow learnt per port expected %d and got %d",
            0x4, flowsLearnt.ipv4UdpFlowsLearnt);
    }
    else if(packetType == CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_OTHER_E)
    {
        UTF_VERIFY_EQUAL2_STRING_MAC(flowsLearnt.otherTypeFlowsLearnt, 0x4, "IPV4 OTHER flow learnt per port expected %d and got %d",
            0x4, flowsLearnt.otherTypeFlowsLearnt);
    }

    st = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "tgfTrafficTableRxStartCapture");

    st = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "tgfTrafficTableRxPcktTblClear");


    /* AUTODOC: restore PVID for port 0 */
    st = prvTgfBrgVlanPortVidSet(prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 prvTgfDefVlanId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvTgfBrgVlanPortVidSet");

    for (portIndex = 1; (portIndex < prvTgfPortsNum); portIndex++)
    {
        st = cpssDxChBrgMcMemberAdd(prvTgfDevNum, 0xFFF, prvTgfPortsArray[portIndex]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChBrgMcMemberAdd: %d", prvTgfPortsArray[portIndex]);
    }

}

/**
* @internal tgfPortQinQTxFlowManagerResourcesMultiTrafficGenerate function
* @endinternal
*
* @brief Generate tcp/udp/other qinq traffic for flow manager service test.
*
*/
GT_VOID tgfPortQinQTxFlowManagerResourcesMultiTrafficGenerate
(
    IN CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_ENT packetType
)
{
    GT_STATUS                           st = GT_OK;
    GT_U32                              portIndex;
    GT_U8                               tagArray[] = {0, 1, 0, 0};
    PRV_CPSS_DXCH_FLOW_MANAGER_STC      *flowMngDbPtr;
    GT_U32                              flowIdArr[4] = {0,1,2,3};
    GT_U32                              flowId[4] = {0,0,0,0};
    GT_U32                              flowIdCount;
    GT_U32                              flowIdGet;
    GT_U32                              iter;
    GT_BOOL                             getFirst;
    GT_BOOL                             isEqual;
    GT_U32                              sourcePort;
    CPSS_DXCH_FLOW_MANAGER_FLOW_ENTRY_STC flowEntry;
    CPSS_DXCH_FLOW_MANAGER_FLOW_STC       flowData;
    CPSS_DXCH_FLOW_MANAGER_PORT_FLOWS_LEARNT_STC flowsLearnt;
    cpssOsMemSet(&flowEntry, 0, sizeof(CPSS_DXCH_FLOW_MANAGER_FLOW_ENTRY_STC));
    cpssOsMemSet(&flowData, 0, sizeof(CPSS_DXCH_FLOW_MANAGER_FLOW_STC));
    cpssOsMemSet(&flowsLearnt, 0, sizeof(CPSS_DXCH_FLOW_MANAGER_PORT_FLOWS_LEARNT_STC));

    /* AUTODOC: create VLAN 5 with all tagged port 1, untagged ports 0,2,3 */
    st = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_CNS,
            prvTgfPortsArray, NULL, tagArray, prvTgfPortsNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvTgfBrgDefVlanEntryWithPortsSet");

    /* get default vlanId */
    st = prvTgfBrgVlanPortVidGet(prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 &prvTgfDefVlanId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvTgfBrgVlanPortVidSet");

    /* AUTODOC: set PVID 5 for port 0 */
    st = prvTgfBrgVlanPortVidSet(prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvTgfBrgVlanPortVidSet");

    st = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "tgfTrafficTableRxPcktTblClear");

    st = tgfTrafficTableRxStartCapture(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "tgfTrafficTableRxStartCapture");

    /* AUTODOC: Clear counters */
    st = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvTgfEthCountersReset");

    for (portIndex = 1; (portIndex < prvTgfPortsNum); portIndex++)
    {
        st = cpssDxChBrgMcMemberDelete(prvTgfDevNum, 0xFFF, prvTgfPortsArray[portIndex]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChBrgMcMemberDelete: %d", prvTgfPortsArray[portIndex]);
    }

    tgfTrafficTracePacketByteSet(GT_TRUE);
    /* send multiple packets with different SIP */
    st = tgfFlowManagerQinQMultiTrafficSend(packetType);
    if (GT_OK != st)
    {
        PRV_UTF_LOG1_MAC("[TGF]: tgfFlowManagerMultiTrafficSend FAILED, rc = [%d]", st);
    }
    cpssOsTimerWkAfter(1000);

    flowMngDbPtr = prvCpssDxChFlowManagerDbFlowMngGet(1);
    UTF_VERIFY_EQUAL0_STRING_MAC(1,
        flowMngDbPtr->seqNoCpss, "flow sequence number is different than expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(4,
        flowMngDbPtr->messageNewFlowCount, "new flow count is different than expected");

    /* Validate flow data get next call */
    getFirst = GT_TRUE;
    st = cpssDxChFlowManagerFlowIdGetNext(1, getFirst, &flowId[0]);
    UTF_VERIFY_EQUAL0_STRING_MAC(flowMngDbPtr->flowDataDb[flowId[0]].flowId,
        flowId[0], "flow id is different than expected");
    getFirst = GT_FALSE;
    iter = 1;
    flowIdGet = 2;
    do
    {
        st = cpssDxChFlowManagerFlowIdGetNext(1, getFirst, &flowId[iter]);
        UTF_VERIFY_EQUAL0_STRING_MAC(flowMngDbPtr->flowDataDb[flowIdGet].flowId,
            flowId[iter], "flow id is different than expected");
        iter++;
        if(iter > 3)
            break;
        flowIdGet--;
    } while(st != GT_NO_MORE);

    /* validate flow counters as in cpss db */
    flowIdCount = 4;
    flowIdArr[0] = flowId[3];
    flowIdArr[1] = flowId[2];
    flowIdArr[2] = flowId[1];
    flowIdArr[3] = flowId[0];

    st = cpssDxChFlowManagerFlowCountersToDbGet(1, flowIdCount, flowIdArr);
    cpssOsTimerWkAfter(1000);

    /* validate flow counters as per cpss db for all 5 flows */
    UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[flowId[3]].flowId, flowId[3], "Existing flow id %d"
                                 "different than in DB %d",0x0, flowMngDbPtr->flowCounterDb[flowId[3]].flowId);
    UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[flowId[3]].packetCount, 0x01, "Existing flow packet count %d"
                                    "different than in DB %d",0x01, flowMngDbPtr->flowCounterDb[flowId[3]].packetCount);
    UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[flowId[3]].dropCount, 0x0, "Existing flow drop count %d"
                                 "different than in DB %d",0x0, flowMngDbPtr->flowCounterDb[flowId[3]].dropCount);

    UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[flowId[2]].flowId, flowId[2], "Existing flow id %d"
                                 "different than in DB %d",0x1, flowMngDbPtr->flowCounterDb[flowId[2]].flowId);
    UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[flowId[2]].packetCount, 0x01, "Existing flow packet count %d"
                                    "different than in DB %d",0x01, flowMngDbPtr->flowCounterDb[flowId[2]].packetCount);
    UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[flowId[2]].dropCount, 0x0, "Existing flow drop count %d"
                                 "different than in DB %d",0x0, flowMngDbPtr->flowCounterDb[flowId[2]].dropCount);

    UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[flowId[1]].flowId, flowId[1], "Existing flow id %d"
                                 "different than in DB %d",0x2, flowMngDbPtr->flowCounterDb[flowId[1]].flowId);
    UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[flowId[1]].packetCount, 0x01, "Existing flow packet count %d"
                                    "different than in DB %d",0x01, flowMngDbPtr->flowCounterDb[flowId[1]].packetCount);
    UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[flowId[1]].dropCount, 0x0, "Existing flow drop count %d"
                                 "different than in DB %d",0x0, flowMngDbPtr->flowCounterDb[flowId[1]].dropCount);

    UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[flowId[0]].flowId, flowId[0], "Existing flow id %d"
                                 "different than in DB %d",0x3, flowMngDbPtr->flowCounterDb[flowId[0]].flowId);
    UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[flowId[0]].packetCount, 0x01, "Existing flow packet count %d"
                                    "different than in DB %d",0x01, flowMngDbPtr->flowCounterDb[flowId[0]].packetCount);
    UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[flowId[0]].dropCount, 0x0, "Existing flow drop count %d"
                                 "different than in DB %d",0x0, flowMngDbPtr->flowCounterDb[flowId[0]].dropCount);

    if(packetType == CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_OTHER_E)
    {
        UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[flowId[3]].byteCount, 0x49, "Existing flow byte count %d"
            "different than in DB %d",0x49, flowMngDbPtr->flowCounterDb[flowId[3]].byteCount);
        UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[flowId[2]].byteCount, 0x49, "Existing flow byte count %d"
            "different than in DB %d",0x49, flowMngDbPtr->flowCounterDb[flowId[2]].byteCount);
        UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[flowId[1]].byteCount, 0x49, "Existing flow byte count %d"
            "different than in DB %d",0x49, flowMngDbPtr->flowCounterDb[flowId[1]].byteCount);
        UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[flowId[0]].byteCount, 0x49, "Existing flow byte count %d"
            "different than in DB %d",0x49, flowMngDbPtr->flowCounterDb[flowId[0]].byteCount);
    }
    else
    {
    }

    /* validate flow data for given flow id */
    st = cpssDxChFlowManagerFlowDataGet(1, flowId[3], &flowEntry, &flowData);
    UTF_VERIFY_EQUAL0_STRING_MAC(0x1e,
        flowData.keySize, "flow key size is different than expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(flowId[3],
        flowEntry.flowId, "flow id is different than expected");

    if(packetType == CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_OTHER_E)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(0x4,
            flowData.keyId, "flow key id is different than expected");
        /* Validate IPV4-OTHER flow key data */
        UTF_VERIFY_EQUAL0_STRING_MAC(0x6,
            flowMngDbPtr->flowDataDb[flowId[3]].keyData[1], "flow outer vlan is different than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0x4,
            flowMngDbPtr->flowDataDb[flowId[3]].keyData[17], "flow inner vlan is different than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            flowMngDbPtr->flowDataDb[flowId[3]].keyData[2], "flow source port different than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0x20,
            flowMngDbPtr->flowDataDb[flowId[3]].keyData[3], "flow applicable flow sub-template different than expected");
        isEqual = (cpssOsMemCmp(&(flowMngDbPtr->flowDataDb[3].keyData[4]),&(prvTgfPacket1L2Part.saMac[0]), sizeof(GT_U16)*3)==0) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL0_STRING_MAC(isEqual,
            0x1, "flow src ip different than expected");
        isEqual = (cpssOsMemCmp(&(flowMngDbPtr->flowDataDb[flowId[3]].keyData[10]),&(prvTgfPacket1L2Part.daMac[0]), sizeof(GT_U16)*3)==0) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL0_STRING_MAC(isEqual,
            0x1, "flow dst ip different than expected");
    }
    else
    {
    }

    st = cpssDxChFlowManagerFlowDataGet(1, flowId[2], &flowEntry, &flowData);
    UTF_VERIFY_EQUAL0_STRING_MAC(flowId[2],
        flowEntry.flowId, "flow id is different than expected");

    if(packetType == CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_OTHER_E)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(0x4,
            flowData.keyId, "flow key id is different than expected");
        /* Validate IPV4-OTHER flow key data */
        UTF_VERIFY_EQUAL0_STRING_MAC(0x24,
            flowMngDbPtr->flowDataDb[flowId[2]].keyData[1], "flow outer vlan is different than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0x10,
            flowMngDbPtr->flowDataDb[flowId[2]].keyData[17], "flow inner vlan is different than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            flowMngDbPtr->flowDataDb[flowId[2]].keyData[2], "flow source port different than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0x20,
            flowMngDbPtr->flowDataDb[flowId[2]].keyData[3], "flow applicable flow sub-template different than expected");
        isEqual = (cpssOsMemCmp(&(flowMngDbPtr->flowDataDb[flowId[2]].keyData[4]),&(prvTgfPacket1L2Part.saMac[0]), sizeof(GT_U16)*3)==0) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL0_STRING_MAC(isEqual,
            0x1, "flow src ip different than expected");
        isEqual = (cpssOsMemCmp(&(flowMngDbPtr->flowDataDb[flowId[2]].keyData[10]),&(prvTgfPacket1L2Part.daMac[0]), sizeof(GT_U16)*3)==0) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL0_STRING_MAC(isEqual,
            0x1, "flow dst ip different than expected");
    }
    else
    {
    }
    st = cpssDxChFlowManagerFlowDataGet(1, flowId[1], &flowEntry, &flowData);
    UTF_VERIFY_EQUAL0_STRING_MAC(flowId[1],
        flowEntry.flowId, "flow id is different than expected");

    if(packetType == CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_OTHER_E)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(0x4,
            flowData.keyId, "flow key id is different than expected");
        /* Validate IPV4-OTHER flow key data */
        UTF_VERIFY_EQUAL0_STRING_MAC(0xd8,
            flowMngDbPtr->flowDataDb[flowId[1]].keyData[1], "flow outer vlan is different than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0x40,
            flowMngDbPtr->flowDataDb[flowId[1]].keyData[17], "flow inner vlan is different than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            flowMngDbPtr->flowDataDb[flowId[1]].keyData[2], "flow source port different than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0x20,
            flowMngDbPtr->flowDataDb[flowId[1]].keyData[3], "flow applicable flow sub-template different than expected");
        isEqual = (cpssOsMemCmp(&(flowMngDbPtr->flowDataDb[flowId[1]].keyData[4]),&(prvTgfPacket1L2Part.saMac[0]), sizeof(GT_U16)*3)==0) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL0_STRING_MAC(isEqual,
            0x1, "flow src ip different than expected");
        isEqual = (cpssOsMemCmp(&(flowMngDbPtr->flowDataDb[flowId[1]].keyData[10]),&(prvTgfPacket1L2Part.daMac[0]), sizeof(GT_U16)*3)==0) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL0_STRING_MAC(isEqual,
            0x1, "flow dst ip different than expected");
    }
    else
    {
    }

    st = cpssDxChFlowManagerFlowDataGet(1, flowId[0], &flowEntry, &flowData);
    UTF_VERIFY_EQUAL0_STRING_MAC(flowId[0],
        flowEntry.flowId, "flow id is different than expected");

    if(packetType == CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_OTHER_E)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(0x4,
            flowData.keyId, "flow key id is different than expected");
        /* Validate IPV4-OTHER flow key data */
        UTF_VERIFY_EQUAL0_STRING_MAC(0xfe,
            flowMngDbPtr->flowDataDb[flowId[0]].keyData[1], "flow outer vlan is different than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0x0,
            flowMngDbPtr->flowDataDb[flowId[0]].keyData[17], "flow inner vlan is different than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            flowMngDbPtr->flowDataDb[flowId[0]].keyData[2], "flow source port different than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0x20,
            flowMngDbPtr->flowDataDb[flowId[0]].keyData[3], "flow applicable flow sub-template different than expected");
        isEqual = (cpssOsMemCmp(&(flowMngDbPtr->flowDataDb[flowId[0]].keyData[4]),&(prvTgfPacket1L2Part.saMac[0]), sizeof(GT_U16)*3)==0) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL0_STRING_MAC(isEqual,
            0x1, "flow src ip different than expected");
        isEqual = (cpssOsMemCmp(&(flowMngDbPtr->flowDataDb[flowId[0]].keyData[10]),&(prvTgfPacket1L2Part.daMac[0]), sizeof(GT_U16)*3)==0) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL0_STRING_MAC(isEqual,
            0x1, "flow dst ip different than expected");
    }
    else
    {
    }

    /* validate flows learnt per source port */
    getFirst = GT_TRUE;
    sourcePort = 0;
    st = cpssDxChFlowManagerSrcPortFlowIdGetNext(prvTgfDevNum, sourcePort, getFirst, &flowId[0]);
    UTF_VERIFY_EQUAL2_STRING_MAC(flowId[0], flowMngDbPtr->flowDataDb[flowId[0]].flowId, "Flow Id learnt per port expected %d and got %d",
        flowId[0], 0x4);
    getFirst = GT_FALSE;
    flowIdGet = 2;
    iter = 1;
    do
    {
        st = cpssDxChFlowManagerSrcPortFlowIdGetNext(prvTgfDevNum, sourcePort, getFirst, &flowId[iter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(flowId[iter], flowMngDbPtr->flowDataDb[flowIdGet].flowId, "Flow Id learnt per port expected %d and got %d",
            flowId[iter], flowIdGet);
        iter++;
        if(iter > 3)
            break;
        flowIdGet--;
    } while(st != GT_NO_MORE);

    /* validate flow types learnt per source port */
    st = cpssDxChFlowManagerSrcPortFlowLearntGet(prvTgfDevNum, sourcePort, &flowsLearnt);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_NO_MORE, st, "dev %d, source port %d", prvTgfDevNum, sourcePort);
    if(packetType == CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_TCP_E)
    {
        UTF_VERIFY_EQUAL2_STRING_MAC(flowsLearnt.ipv4TcpFlowsLearnt, 0x4, "IPV4 TCP flow learnt per port expected %d and got %d",
            0x4, flowsLearnt.ipv4TcpFlowsLearnt);
    }
    else if(packetType == CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_UDP_E)
    {
        UTF_VERIFY_EQUAL2_STRING_MAC(flowsLearnt.ipv4UdpFlowsLearnt, 0x4, "IPV4 UDP flow learnt per port expected %d and got %d",
            0x4, flowsLearnt.ipv4UdpFlowsLearnt);
    }
    else if(packetType == CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_OTHER_E)
    {
        UTF_VERIFY_EQUAL2_STRING_MAC(flowsLearnt.otherTypeFlowsLearnt, 0x4, "IPV4 OTHER flow learnt per port expected %d and got %d",
            0x4, flowsLearnt.otherTypeFlowsLearnt);
    }
    st = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "tgfTrafficTableRxStartCapture");

    st = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: restore PVID  for port 0 */
    st = prvTgfBrgVlanPortVidSet(prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        prvTgfDefVlanId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvTgfBrgVlanPortVidSet");

    for (portIndex = 1; (portIndex < prvTgfPortsNum); portIndex++)
    {
        st = cpssDxChBrgMcMemberAdd(prvTgfDevNum, 0xFFF, prvTgfPortsArray[portIndex]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChBrgMcMemberAdd: %d", prvTgfPortsArray[portIndex]);
    }

}


/**
* @internal tgfPortTxFlowManagerResourcesMixedTrafficGenerate function
* @endinternal
*
* @brief Generate mixed traffic for flow manager service test.
*
*/
GT_VOID tgfPortTxFlowManagerResourcesMixedTrafficGenerate
(
)
{
    GT_STATUS                           st = GT_OK;
    GT_U32                              portIndex;
    CPSS_PORT_MAC_COUNTER_SET_STC       portCounters;
    CPSS_DXCH_FLOW_MANAGER_FLOW_ENTRY_STC flowEntry;
    CPSS_DXCH_FLOW_MANAGER_FLOW_STC       flowData;
    CPSS_DXCH_FLOW_MANAGER_PORT_FLOWS_LEARNT_STC flowsLearnt;
    GT_U32                              prvTgfPacketCount = 2;
    GT_U32                              ii;
    PRV_CPSS_DXCH_FLOW_MANAGER_STC      *flowMngDbPtr;
    GT_U32                              flowIdArr[3] = {0,1,2};
    GT_U32                              flowId[3] = {0,0,0};
    GT_U32                              flowIdCount;
    GT_U32                              flowIdGet;
    GT_U32                              iter;
    GT_BOOL                             getFirst;
    GT_BOOL                             isEqual;
    GT_U32                              sourcePort;
    GT_U32                              i;
    GT_U8                               srcAddr[4] = {4,5,6,7};
    GT_U8                               dstAddr[4] = {1,1,1,3};

    cpssOsBzero((GT_CHAR*) &flowEntry, sizeof(flowEntry));
    cpssOsBzero((GT_CHAR*) &flowData, sizeof(flowData));
    cpssOsBzero((GT_CHAR*) &flowsLearnt, sizeof(flowsLearnt));

    /* AUTODOC: Clear counters */
    st = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvTgfEthCountersReset");

    for (portIndex = 1; (portIndex < prvTgfPortsNum); portIndex++)
    {
        st = cpssDxChBrgMcMemberDelete(prvTgfDevNum, 0xFFF, prvTgfPortsArray[portIndex]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChBrgMcMemberDelete: %d", prvTgfPortsArray[portIndex]);
    }

    st = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "tgfTrafficTableRxPcktTblClear");

    st = tgfTrafficTableRxStartCapture(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "tgfTrafficTableRxStartCapture");

    /* AUTODOC: Transmit 1 IPFIX flow (with DIP 5.6.7.8) to
       port 0. */
    st = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacket1Info,
        PRV_TGF_IPFIX_FLOW_COUNT_OF_FRAMES_TO_TRANSMIT_CNS, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
        "prvTgfSetTxSetupEth: %d, %d, %d, %d", prvTgfDevNum,
        PRV_TGF_IPFIX_FLOW_COUNT_OF_FRAMES_TO_TRANSMIT_CNS, 0, NULL);

    for(ii = 0; ii < prvTgfPacketCount; ii++)
    {
        st = prvTgfStartTransmitingEth(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
            "ERROR of StartTransmitting: %d, %d",
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    }

    st = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfIpv4UdpPacketInfo,
        PRV_TGF_IPFIX_FLOW_COUNT_OF_FRAMES_TO_TRANSMIT_CNS, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
        "prvTgfSetTxSetupEth: %d, %d, %d, %d", prvTgfDevNum,
        PRV_TGF_IPFIX_FLOW_COUNT_OF_FRAMES_TO_TRANSMIT_CNS, 0, NULL);

    for(ii = 0; ii < prvTgfPacketCount; ii++)
    {
        st = prvTgfStartTransmitingEth(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_SEND_PORT1_IDX_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
            "ERROR of StartTransmitting: %d, %d",
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT1_IDX_CNS]);
    }

    st = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfOtherPacketInfo,
        PRV_TGF_IPFIX_FLOW_COUNT_OF_FRAMES_TO_TRANSMIT_CNS, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
        "prvTgfSetTxSetupEth: %d, %d, %d, %d", prvTgfDevNum,
        PRV_TGF_IPFIX_FLOW_COUNT_OF_FRAMES_TO_TRANSMIT_CNS, 0, NULL);

    for(ii = 0; ii < prvTgfPacketCount; ii++)
    {
        st = prvTgfStartTransmitingEth(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_SEND_PORT2_IDX_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
            "ERROR of StartTransmitting: %d, %d",
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT2_IDX_CNS]);
    }
    cpssOsTimerWkAfter(300);

    for (portIndex = 0; (portIndex < 3); portIndex++)
    {
        st = prvTgfReadPortCountersEth(
            prvTgfDevsArray[portIndex],
            prvTgfPortsArray[portIndex],
            GT_FALSE /*enablePrint*/,
            &portCounters);
        if (st != GT_OK)
        {
            PRV_UTF_LOG2_MAC(
                "prvTgfReadPortCountersEth failed, dev: %d, port: %d\n",
                prvTgfDevsArray[portIndex],
                prvTgfPortsArray[portIndex]);
        }
        PRV_UTF_LOG6_MAC(
            "dev %d port %d RX packets %d, bytes %d, TX packets %d, bytes %d\n",
            prvTgfDevsArray[portIndex], prvTgfPortsArray[portIndex],
            portCounters.goodPktsRcv.l[0], portCounters.goodOctetsRcv.l[0],
            portCounters.goodPktsSent.l[0], portCounters.goodOctetsSent.l[0]);
    }

    flowMngDbPtr = prvCpssDxChFlowManagerDbFlowMngGet(1);

    UTF_VERIFY_EQUAL0_STRING_MAC(1,
        flowMngDbPtr->seqNoCpss, "flow sequence number is different than expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(3,
        flowMngDbPtr->messageNewFlowCount, "new flow count is different than expected");

    /* Validate flow data get next call */
    getFirst = GT_TRUE;
    st = cpssDxChFlowManagerFlowIdGetNext(1, getFirst, &flowId[0]);
    UTF_VERIFY_EQUAL0_STRING_MAC(flowMngDbPtr->flowDataDb[flowId[0]].flowId,
        flowId[0], "flow id is different than expected");
    getFirst = GT_FALSE;
    iter = 1;
    flowIdGet = 1;
    do
    {
        st = cpssDxChFlowManagerFlowIdGetNext(1, getFirst, &flowId[iter]);
        UTF_VERIFY_EQUAL0_STRING_MAC(flowMngDbPtr->flowDataDb[flowIdGet].flowId,
            flowId[iter], "flow id is different than expected");
        iter++;
        if(iter > 2)
            break;
        flowIdGet--;
    } while(st != GT_NO_MORE);

    /* validate flow counters as in cpss db */
    flowIdCount = 3;
    flowIdArr[0] = flowId[2];
    flowIdArr[1] = flowId[1];
    flowIdArr[2] = flowId[0];
    st = cpssDxChFlowManagerFlowCountersToDbGet(1, flowIdCount, flowIdArr);
    cpssOsTimerWkAfter(1000);

    /* validate flow counters as per cpss db for all 5 flows */
    /* Validate IPV4-TCP flows */
    UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[flowId[2]].flowId, flowId[2], "Existing flow id %d"
                                 "different than in DB %d", flowId[2], flowMngDbPtr->flowCounterDb[flowId[2]].flowId);
    UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[flowId[2]].packetCount, 0x01, "Existing flow packet count %d"
                                    "different than in DB %d",0x01, flowMngDbPtr->flowCounterDb[0].packetCount);
    UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[flowId[2]].byteCount, 0x59, "Existing flow byte count %d"
                                    "different than in DB %d",0x59, flowMngDbPtr->flowCounterDb[0].byteCount);
    UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[flowId[2]].dropCount, 0x1, "Existing flow drop count %d"
                                 "different than in DB %d",0x1, flowMngDbPtr->flowCounterDb[0].dropCount);

    /* Validate IPV4-UDP flows */
    UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[flowId[1]].flowId, flowId[1], "Existing flow id %d"
                                 "different than in DB %d", flowId[1], flowMngDbPtr->flowCounterDb[flowId[1]].flowId);
    UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[1].packetCount, 0x02, "Existing flow packet count %d"
                                    "different than in DB %d",0x02, flowMngDbPtr->flowCounterDb[1].packetCount);
    UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[1].byteCount, 0x80, "Existing flow byte count %d"
                                    "different than in DB %d",0x80, flowMngDbPtr->flowCounterDb[1].byteCount);
    UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[1].dropCount, 0x0, "Existing flow drop count %d"
                                 "different than in DB %d", 0x0, flowMngDbPtr->flowCounterDb[1].dropCount);

    /* Validate IPV4-OTHER flows */
    UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[flowId[0]].flowId, flowId[0], "Existing flow id %d"
                                 "different than in DB %d",0x2, flowMngDbPtr->flowCounterDb[2].flowId);
    UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[2].packetCount, 0x02, "Existing flow packet count %d"
                                    "different than in DB %d",0x02, flowMngDbPtr->flowCounterDb[2].packetCount);
    UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[2].dropCount, 0x0, "Existing flow drop count %d"
                                 "different than in DB %d",0x0, flowMngDbPtr->flowCounterDb[2].dropCount);
    UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[2].byteCount, 0x8a, "Existing flow byte count %d"
                                 "different than in DB %d",0x8a, flowMngDbPtr->flowCounterDb[2].byteCount);

    /* validate flow data for given flow id */
    /* IPV4-OTHER packet type flow key data validation */
    st = cpssDxChFlowManagerFlowDataGet(1, flowId[0], &flowEntry, &flowData);
    UTF_VERIFY_EQUAL0_STRING_MAC(0x1e,
        flowData.keySize, "flow key size is different than expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0x4,
        flowData.keyId, "flow key id is different than expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0x2,
        flowEntry.flowId, "flow id is different than expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0x1,
        flowMngDbPtr->flowDataDb[flowId[0]].keyData[0], "flow vlan is different than expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPortsArray[PRV_TGF_SEND_PORT2_IDX_CNS],
        flowMngDbPtr->flowDataDb[flowId[0]].keyData[2], "flow source port different than expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0x20,
        flowMngDbPtr->flowDataDb[flowId[0]].keyData[3], "flow applicable flow sub-template different than expected");

    /* Validate IPV4-UDP flow key data */
    st = cpssDxChFlowManagerFlowDataGet(1, flowId[1], &flowEntry, &flowData);
    UTF_VERIFY_EQUAL0_STRING_MAC(0x1e,
        flowData.keySize, "flow key size is different than expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0x1,
        flowData.keyId, "flow key id is different than expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0x1,
        flowEntry.flowId, "flow id is different than expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0x1,
        flowMngDbPtr->flowDataDb[flowId[1]].keyData[0], "flow vlan is different than expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPortsArray[PRV_TGF_SEND_PORT1_IDX_CNS],
        flowMngDbPtr->flowDataDb[flowId[1]].keyData[2], "flow source port different than expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0x2,
        flowMngDbPtr->flowDataDb[flowId[1]].keyData[3], "flow applicable flow sub-template different than expected");
    for (i = 0; i < 4; i++)
    {
        if(flowMngDbPtr->flowDataDb[flowId[1]].keyData[4+i] != srcAddr[i])
        {
            isEqual = GT_FALSE;
            break;
        }
        isEqual = GT_TRUE;
    }
    UTF_VERIFY_EQUAL0_STRING_MAC(isEqual,
        0x1, "flow src ip different than expected");
    for (i = 0; i < 4; i++)
    {
        if(flowMngDbPtr->flowDataDb[flowId[1]].keyData[8+i] != dstAddr[i])
        {
            isEqual = GT_FALSE;
            break;
        }
        isEqual = GT_TRUE;
    }
    UTF_VERIFY_EQUAL0_STRING_MAC(isEqual,
        0x1, "flow dst ip different than expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0x26,
        flowMngDbPtr->flowDataDb[flowId[1]].keyData[13], "flow udp src port different than expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0x23,
        flowMngDbPtr->flowDataDb[flowId[1]].keyData[15], "flow udp dst port different than expected");

    /* Validate IPV4-TCP flow key data */
    st = cpssDxChFlowManagerFlowDataGet(1, flowId[2], &flowEntry, &flowData);
    UTF_VERIFY_EQUAL0_STRING_MAC(0x1e,
        flowData.keySize, "flow key size is different than expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0x0,
        flowData.keyId, "flow key id is different than expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0x0,
        flowEntry.flowId, "flow id is different than expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0x1,
        flowMngDbPtr->flowDataDb[flowId[2]].keyData[0], "flow vlan is different than expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        flowMngDbPtr->flowDataDb[flowId[2]].keyData[2], "flow source port different than expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0x1,
        flowMngDbPtr->flowDataDb[flowId[2]].keyData[3], "flow applicable flow sub-template different than expected");
    for (i = 0; i < 4; i++)
    {
        if(flowMngDbPtr->flowDataDb[flowId[2]].keyData[4+i] != srcAddr[i])
        {
            isEqual = GT_FALSE;
            break;
        }
        isEqual = GT_TRUE;
    }
    UTF_VERIFY_EQUAL0_STRING_MAC(isEqual,
        0x1, "flow src ip different than expected");
    for (i = 0; i < 4; i++)
    {
        if(flowMngDbPtr->flowDataDb[flowId[2]].keyData[8+i] != dstAddr[i])
        {
            isEqual = GT_FALSE;
            break;
        }
        isEqual = GT_TRUE;
    }
    UTF_VERIFY_EQUAL0_STRING_MAC(isEqual,
        0x1, "flow dst ip different than expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0x26,
        flowMngDbPtr->flowDataDb[flowId[2]].keyData[13], "flow tcp src port different than expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0x23,
        flowMngDbPtr->flowDataDb[flowId[2]].keyData[15], "flow tcp dst port different than expected");

    /* validate flows learnt per source port */
    getFirst = GT_TRUE;
    sourcePort = 0;
    st = cpssDxChFlowManagerSrcPortFlowIdGetNext(prvTgfDevNum, sourcePort, getFirst, &flowId[0]);
    UTF_VERIFY_EQUAL2_STRING_MAC(flowId[0], flowMngDbPtr->flowDataDb[flowId[0]].flowId, "Flow Id learnt per port expected %d and got %d",
        flowId[0], 0x0);
    sourcePort = 12;
    st = cpssDxChFlowManagerSrcPortFlowIdGetNext(prvTgfDevNum, sourcePort, getFirst, &flowId[0]);
    UTF_VERIFY_EQUAL2_STRING_MAC(flowId[0], flowMngDbPtr->flowDataDb[flowId[0]].flowId, "Flow Id learnt per port expected %d and got %d",
        flowId[0], 0x1);
    sourcePort = 36;
    st = cpssDxChFlowManagerSrcPortFlowIdGetNext(prvTgfDevNum, sourcePort, getFirst, &flowId[0]);
    UTF_VERIFY_EQUAL2_STRING_MAC(flowId[0], flowMngDbPtr->flowDataDb[flowId[0]].flowId, "Flow Id learnt per port expected %d and got %d",
        flowId[0], 0x2);

    /* validate flow types learnt per source port */
    sourcePort = 0;
    st = cpssDxChFlowManagerSrcPortFlowLearntGet(prvTgfDevNum, sourcePort, &flowsLearnt);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_NO_MORE, st, "dev %d, source port %d", prvTgfDevNum, sourcePort);
    sourcePort = 12;
    st = cpssDxChFlowManagerSrcPortFlowLearntGet(prvTgfDevNum, sourcePort, &flowsLearnt);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_NO_MORE, st, "dev %d, source port %d", prvTgfDevNum, sourcePort);
    sourcePort = 36;
    st = cpssDxChFlowManagerSrcPortFlowLearntGet(prvTgfDevNum, sourcePort, &flowsLearnt);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_NO_MORE, st, "dev %d, source port %d", prvTgfDevNum, sourcePort);
    UTF_VERIFY_EQUAL2_STRING_MAC(flowsLearnt.ipv4TcpFlowsLearnt, 0x1, "IPV4 TCP flow learnt per port expected %d and got %d",
        0x1, flowsLearnt.ipv4TcpFlowsLearnt);
    UTF_VERIFY_EQUAL2_STRING_MAC(flowsLearnt.ipv4UdpFlowsLearnt, 0x1, "IPV4 UDP flow learnt per port expected %d and got %d",
        0x1, flowsLearnt.ipv4UdpFlowsLearnt);
    UTF_VERIFY_EQUAL2_STRING_MAC(flowsLearnt.otherTypeFlowsLearnt, 0x1, "IPV4 OTHER flow learnt per port expected %d and got %d",
        0x1, flowsLearnt.otherTypeFlowsLearnt);

    st = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "tgfTrafficTableRxStartCapture");

    st = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "tgfTrafficTableRxPcktTblClear");

    for (portIndex = 1; (portIndex < prvTgfPortsNum); portIndex++)
    {
        st = cpssDxChBrgMcMemberAdd(prvTgfDevNum, 0xFFF, prvTgfPortsArray[portIndex]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChBrgMcMemberAdd: %d", prvTgfPortsArray[portIndex]);
    }
}

/**
* @internal tgfPortTxFlowManagerResourcesMixedPortTrafficGenerate function
* @endinternal
*
* @brief Generate mixed traffic for flow manager service test.
*
*/
GT_VOID tgfPortTxFlowManagerResourcesMixedPortTrafficGenerate
(
)
{
    GT_STATUS                           st = GT_OK;
    GT_U32                              portIndex;
    CPSS_PORT_MAC_COUNTER_SET_STC       portCounters;
    CPSS_DXCH_FLOW_MANAGER_FLOW_ENTRY_STC flowEntry;
    CPSS_DXCH_FLOW_MANAGER_FLOW_STC       flowData;
    CPSS_DXCH_FLOW_MANAGER_PORT_FLOWS_LEARNT_STC flowsLearnt;
    GT_U32                              prvTgfPacketCount = 2;
    GT_U32                              ii;
    PRV_CPSS_DXCH_FLOW_MANAGER_STC      *flowMngDbPtr;
    GT_U32                              flowIdArr[3] = {0,1,2};
    GT_U32                              flowId[3] = {0,0,0};
    GT_U32                              flowIdCount;
    GT_U32                              flowIdGet;
    GT_U32                              iter;
    GT_BOOL                             getFirst;
    GT_BOOL                             isEqual;
    GT_U32                              sourcePort;
    GT_U32                              i;
    GT_U8                               srcAddr[4] = {4,5,6,7};
    GT_U8                               dstAddr[4] = {1,1,1,3};

    cpssOsBzero((GT_CHAR*) &flowEntry, sizeof(flowEntry));
    cpssOsBzero((GT_CHAR*) &flowData, sizeof(flowData));
    cpssOsBzero((GT_CHAR*) &flowsLearnt, sizeof(flowsLearnt));

    /* AUTODOC: Clear counters */
    st = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvTgfEthCountersReset");

    for (portIndex = 1; (portIndex < prvTgfPortsNum); portIndex++)
    {
        st = cpssDxChBrgMcMemberDelete(prvTgfDevNum, 0xFFF, prvTgfPortsArray[portIndex]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChBrgMcMemberDelete: %d", prvTgfPortsArray[portIndex]);
    }

    st = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "tgfTrafficTableRxPcktTblClear");

    st = tgfTrafficTableRxStartCapture(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "tgfTrafficTableRxStartCapture");

    /* AUTODOC: Transmit 1 IPFIX flow (with DIP 5.6.7.8) to
       port 0. */
    st = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacket1Info,
        PRV_TGF_IPFIX_FLOW_COUNT_OF_FRAMES_TO_TRANSMIT_CNS, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
        "prvTgfSetTxSetupEth: %d, %d, %d, %d", prvTgfDevNum,
        PRV_TGF_IPFIX_FLOW_COUNT_OF_FRAMES_TO_TRANSMIT_CNS, 0, NULL);

    for(ii = 0; ii < prvTgfPacketCount; ii++)
    {
        st = prvTgfStartTransmitingEth(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
            "ERROR of StartTransmitting: %d, %d",
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    }

    st = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfIpv4UdpPacketInfo,
        PRV_TGF_IPFIX_FLOW_COUNT_OF_FRAMES_TO_TRANSMIT_CNS, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
        "prvTgfSetTxSetupEth: %d, %d, %d, %d", prvTgfDevNum,
        PRV_TGF_IPFIX_FLOW_COUNT_OF_FRAMES_TO_TRANSMIT_CNS, 0, NULL);

    for(ii = 0; ii < prvTgfPacketCount; ii++)
    {
        st = prvTgfStartTransmitingEth(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
            "ERROR of StartTransmitting: %d, %d",
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT1_IDX_CNS]);
    }

    st = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfOtherPacketInfo,
        PRV_TGF_IPFIX_FLOW_COUNT_OF_FRAMES_TO_TRANSMIT_CNS, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, st,
        "prvTgfSetTxSetupEth: %d, %d, %d, %d", prvTgfDevNum,
        PRV_TGF_IPFIX_FLOW_COUNT_OF_FRAMES_TO_TRANSMIT_CNS, 0, NULL);

    for(ii = 0; ii < prvTgfPacketCount; ii++)
    {
        st = prvTgfStartTransmitingEth(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
            "ERROR of StartTransmitting: %d, %d",
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT2_IDX_CNS]);
    }
    cpssOsTimerWkAfter(300);

    for (portIndex = 0; (portIndex < 3); portIndex++)
    {
        st = prvTgfReadPortCountersEth(
            prvTgfDevsArray[portIndex],
            prvTgfPortsArray[portIndex],
            GT_FALSE /*enablePrint*/,
            &portCounters);
        if (st != GT_OK)
        {
            PRV_UTF_LOG2_MAC(
                "prvTgfReadPortCountersEth failed, dev: %d, port: %d\n",
                prvTgfDevsArray[portIndex],
                prvTgfPortsArray[portIndex]);
        }
        PRV_UTF_LOG6_MAC(
            "dev %d port %d RX packets %d, bytes %d, TX packets %d, bytes %d\n",
            prvTgfDevsArray[portIndex], prvTgfPortsArray[portIndex],
            portCounters.goodPktsRcv.l[0], portCounters.goodOctetsRcv.l[0],
            portCounters.goodPktsSent.l[0], portCounters.goodOctetsSent.l[0]);
    }

    flowMngDbPtr = prvCpssDxChFlowManagerDbFlowMngGet(1);

    UTF_VERIFY_EQUAL0_STRING_MAC(1,
        flowMngDbPtr->seqNoCpss, "flow sequence number is different than expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(3,
        flowMngDbPtr->messageNewFlowCount, "new flow count is different than expected");

    /* Validate flow data get next call */
    getFirst = GT_TRUE;
    st = cpssDxChFlowManagerFlowIdGetNext(1, getFirst, &flowId[0]);
    UTF_VERIFY_EQUAL0_STRING_MAC(flowMngDbPtr->flowDataDb[flowId[0]].flowId,
        flowId[0], "flow id is different than expected");
    getFirst = GT_FALSE;
    iter = 1;
    flowIdGet = 1;
    do
    {
        st = cpssDxChFlowManagerFlowIdGetNext(1, getFirst, &flowId[iter]);
        UTF_VERIFY_EQUAL0_STRING_MAC(flowMngDbPtr->flowDataDb[flowIdGet].flowId,
            flowId[iter], "flow id is different than expected");
        iter++;
        if(iter > 2)
            break;
        flowIdGet--;
    } while(st != GT_NO_MORE);

    /* validate flow counters as in cpss db */
    flowIdCount = 3;
    flowIdArr[0] = flowId[2];
    flowIdArr[1] = flowId[1];
    flowIdArr[2] = flowId[0];
    st = cpssDxChFlowManagerFlowCountersToDbGet(1, flowIdCount, flowIdArr);
    cpssOsTimerWkAfter(1000);

    /* validate flow counters as per cpss db for all 5 flows */
    /* Validate IPV4-TCP flows */
    UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[flowId[2]].flowId, flowId[2], "Existing flow id %d"
                                 "different than in DB %d",0x0, flowMngDbPtr->flowCounterDb[0].flowId);
    UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[0].packetCount, 0x01, "Existing flow packet count %d"
                                    "different than in DB %d",0x01, flowMngDbPtr->flowCounterDb[0].packetCount);
    UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[0].byteCount, 0x59, "Existing flow byte count %d"
                                    "different than in DB %d",0x59, flowMngDbPtr->flowCounterDb[0].byteCount);
    UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[0].dropCount, 0x1, "Existing flow drop count %d"
                                 "different than in DB %d",0x1, flowMngDbPtr->flowCounterDb[0].dropCount);

    /* Validate IPV4-UDP flows */
    UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[flowId[1]].flowId, flowId[1], "Existing flow id %d"
                                 "different than in DB %d",0x1, flowMngDbPtr->flowCounterDb[1].flowId);
    UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[1].packetCount, 0x02, "Existing flow packet count %d"
                                    "different than in DB %d", 0x02, flowMngDbPtr->flowCounterDb[1].packetCount);
    UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[1].byteCount, 0x80, "Existing flow byte count %d"
                                    "different than in DB %d",0x80, flowMngDbPtr->flowCounterDb[1].byteCount);
    UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[1].dropCount, 0x0, "Existing flow drop count %d"
                                 "different than in DB %d", 0x0, flowMngDbPtr->flowCounterDb[1].dropCount);

    /* Validate IPV4-OTHER flows */
    UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[flowId[0]].flowId, flowId[0], "Existing flow id %d"
                                 "different than in DB %d",0x2, flowMngDbPtr->flowCounterDb[2].flowId);
    UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[2].packetCount, 0x02, "Existing flow packet count %d"
                                    "different than in DB %d",0x02, flowMngDbPtr->flowCounterDb[2].packetCount);
    UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[2].dropCount, 0x0, "Existing flow drop count %d"
                                 "different than in DB %d",0x0, flowMngDbPtr->flowCounterDb[2].dropCount);
    UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[2].byteCount, 0x8a, "Existing flow byte count %d"
                                 "different than in DB %d",0x8a, flowMngDbPtr->flowCounterDb[2].byteCount);

    /* validate flow data for given flow id */
    /* IPV4-OTHER packet type flow key data validation */
    st = cpssDxChFlowManagerFlowDataGet(1, flowId[0], &flowEntry, &flowData);
    UTF_VERIFY_EQUAL0_STRING_MAC(0x1e,
        flowData.keySize, "flow key size is different than expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0x4,
        flowData.keyId, "flow key id is different than expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0x2,
        flowEntry.flowId, "flow id is different than expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0x1,
        flowMngDbPtr->flowDataDb[flowId[0]].keyData[0], "flow vlan is different than expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        flowMngDbPtr->flowDataDb[flowId[0]].keyData[2], "flow source port different than expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0x20,
        flowMngDbPtr->flowDataDb[flowId[0]].keyData[3], "flow applicable flow sub-template different than expected");

    /* Validate IPV4-UDP flow key data */
    st = cpssDxChFlowManagerFlowDataGet(1, flowId[1], &flowEntry, &flowData);
    UTF_VERIFY_EQUAL0_STRING_MAC(0x1e,
        flowData.keySize, "flow key size is different than expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0x1,
        flowData.keyId, "flow key id is different than expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0x1,
        flowEntry.flowId, "flow id is different than expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0x1,
        flowMngDbPtr->flowDataDb[flowId[1]].keyData[0], "flow vlan is different than expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        flowMngDbPtr->flowDataDb[flowId[1]].keyData[2], "flow source port different than expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0x2,
        flowMngDbPtr->flowDataDb[flowId[1]].keyData[3], "flow applicable flow sub-template different than expected");
    for (i = 0; i < 4; i++)
    {
        if(flowMngDbPtr->flowDataDb[flowId[1]].keyData[4+i] != srcAddr[i])
        {
            isEqual = GT_FALSE;
            break;
        }
        isEqual = GT_TRUE;
    }
    UTF_VERIFY_EQUAL0_STRING_MAC(isEqual,
        0x1, "flow src ip different than expected");
    for (i = 0; i < 4; i++)
    {
        if(flowMngDbPtr->flowDataDb[flowId[1]].keyData[8+i] != dstAddr[i])
        {
            isEqual = GT_FALSE;
            break;
        }
        isEqual = GT_TRUE;
    }
    UTF_VERIFY_EQUAL0_STRING_MAC(isEqual,
        0x1, "flow dst ip different than expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0x26,
        flowMngDbPtr->flowDataDb[flowId[1]].keyData[13], "flow udp src port different than expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0x23,
        flowMngDbPtr->flowDataDb[flowId[1]].keyData[15], "flow udp dst port different than expected");

    /* Validate IPV4-TCP flow key data */
    st = cpssDxChFlowManagerFlowDataGet(1, flowId[2], &flowEntry, &flowData);
    UTF_VERIFY_EQUAL0_STRING_MAC(0x1e,
        flowData.keySize, "flow key size is different than expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0x0,
        flowData.keyId, "flow key id is different than expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0x0,
        flowEntry.flowId, "flow id is different than expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0x1,
        flowMngDbPtr->flowDataDb[flowId[2]].keyData[0], "flow vlan is different than expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        flowMngDbPtr->flowDataDb[flowId[2]].keyData[2], "flow source port different than expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0x1,
        flowMngDbPtr->flowDataDb[flowId[2]].keyData[3], "flow applicable flow sub-template different than expected");
    for (i = 0; i < 4; i++)
    {
        if(flowMngDbPtr->flowDataDb[flowId[2]].keyData[4+i] != srcAddr[i])
        {
            isEqual = GT_FALSE;
            break;
        }
        isEqual = GT_TRUE;
    }
    UTF_VERIFY_EQUAL0_STRING_MAC(isEqual,
        0x1, "flow src ip different than expected");
    for (i = 0; i < 4; i++)
    {
        if(flowMngDbPtr->flowDataDb[flowId[2]].keyData[8+i] != dstAddr[i])
        {
            isEqual = GT_FALSE;
            break;
        }
        isEqual = GT_TRUE;
    }
    UTF_VERIFY_EQUAL0_STRING_MAC(isEqual,
        0x1, "flow dst ip different than expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0x26,
        flowMngDbPtr->flowDataDb[flowId[2]].keyData[13], "flow tcp src port different than expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0x23,
        flowMngDbPtr->flowDataDb[flowId[2]].keyData[15], "flow tcp dst port different than expected");

    /* validate flows learnt per source port */
    getFirst = GT_TRUE;
    sourcePort = 0;
    st = cpssDxChFlowManagerSrcPortFlowIdGetNext(prvTgfDevNum, sourcePort, getFirst, &flowId[0]);
    UTF_VERIFY_EQUAL2_STRING_MAC(flowId[0], flowMngDbPtr->flowDataDb[flowId[0]].flowId, "Flow Id learnt per port expected %d and got %d",
        flowId[0], flowMngDbPtr->flowDataDb[flowId[0]].flowId);
    getFirst = GT_FALSE;
    st = cpssDxChFlowManagerSrcPortFlowIdGetNext(prvTgfDevNum, sourcePort, getFirst, &flowId[1]);
    UTF_VERIFY_EQUAL2_STRING_MAC(flowId[1], flowMngDbPtr->flowDataDb[flowId[1]].flowId, "Flow Id learnt per port expected %d and got %d",
        flowId[1], flowMngDbPtr->flowDataDb[flowId[1]].flowId);
    st = cpssDxChFlowManagerSrcPortFlowIdGetNext(prvTgfDevNum, sourcePort, getFirst, &flowId[2]);
    UTF_VERIFY_EQUAL2_STRING_MAC(flowId[2], flowMngDbPtr->flowDataDb[flowId[2]].flowId, "Flow Id learnt per port expected %d and got %d",
        flowId[2], flowMngDbPtr->flowDataDb[flowId[2]].flowId);

    /* validate flow types learnt per source port */
    sourcePort = 0;
    st = cpssDxChFlowManagerSrcPortFlowLearntGet(prvTgfDevNum, sourcePort, &flowsLearnt);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_NO_MORE, st, "dev %d, source port %d", prvTgfDevNum, sourcePort);
    UTF_VERIFY_EQUAL2_STRING_MAC(flowsLearnt.ipv4TcpFlowsLearnt, 0x1, "IPV4 TCP flow learnt per port expected %d and got %d",
        0x1, flowsLearnt.ipv4TcpFlowsLearnt);
    UTF_VERIFY_EQUAL2_STRING_MAC(flowsLearnt.ipv4UdpFlowsLearnt, 0x1, "IPV4 UDP flow learnt per port expected %d and got %d",
        0x1, flowsLearnt.ipv4UdpFlowsLearnt);
    UTF_VERIFY_EQUAL2_STRING_MAC(flowsLearnt.otherTypeFlowsLearnt, 0x1, "IPV4 OTHER flow learnt per port expected %d and got %d",
        0x1, flowsLearnt.otherTypeFlowsLearnt);

    st = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "tgfTrafficTableRxStartCapture");

    st = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "tgfTrafficTableRxPcktTblClear");

    for (portIndex = 1; (portIndex < prvTgfPortsNum); portIndex++)
    {
        st = cpssDxChBrgMcMemberAdd(prvTgfDevNum, 0xFFF, prvTgfPortsArray[portIndex]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChBrgMcMemberAdd: %d", prvTgfPortsArray[portIndex]);
    }

}
