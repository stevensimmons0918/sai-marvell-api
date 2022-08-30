/*******************************************************************************
 *              (c), Copyright 2020, Marvell International Ltd.                 *
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
 * @file tgfTrafficGenLuaUtil.c
 *
 * @brief Traffic generator utility APIs
 *
 * @version   1
 ********************************************************************************
 */
#include <cpssCommon/cpssPresteraDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <utf/private/prvUtfHelpers.h>
#include <trafficEngine/tgfTrafficGenerator.h>
#include <cpss/dxCh/dxChxGen/pha/cpssDxChPha.h>
#include <trafficEngine/tgfTrafficTable.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfTrunkGen.h>
#include <common/tgfIpGen.h>
#include <oam/prvTgfOamGeneral.h>
#include <extUtils/trafficEngine/prvTgfLog.h>

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Parts[2] = {
    {
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x01},               /* dstMac */
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x02}                /* srcMac */
    },
    {
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x01},               /* dstMac */
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x02}                /* srcMac */
    }};

static TGF_ETHER_TYPE      etherType[] = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS,
                                          TGF_ETHERTYPE_86DD_IPV6_TAG_CNS};

/* VLAN_TAG part - Tag0 */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag0Parts[2] = {
    {
        TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
        0, 0, 1                                             /* pri, cfi, VlanId */
    },
    {
        TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
        0, 0, 1                                             /* pri, cfi, VlanId */
    }};

/* wildcard structure, to store l2 custom tag */
static TGF_PACKET_WILDCARD_STC prvTgfWildCard[2];

static TGF_PROT protocols[] = {0x06, 0x11};

/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4Parts[2] = {
    {
        4,                                      /* version */
        5,                                      /* headerLen */
        0,                                      /* typeOfService */
        0x2A,                                   /* totalLen */
        0,                                      /* id */
        0,                                      /* flags */
        0,                                      /* offset */
        0x40,                                   /* timeToLive */
        0xff,                                   /* protocol */
        TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,      /* cksum */
        {0xE0, 0, 0, 1},                        /* srcAddr */
        {0xE0, 0, 0, 2}                         /* dstAddr */
    },
    {
        4,                                      /* version */
        5,                                      /* headerLen */
        0,                                      /* typeOfService */
        0x2A,                                   /* totalLen */
        0,                                      /* id */
        0,                                      /* flags */
        0,                                      /* offset */
        0x40,                                   /* timeToLive */
        0xff,                                   /* protocol */
        TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,      /* cksum */
        {0xE0, 0, 0, 1},                        /* srcAddr */
        {0xE0, 0, 0, 2}                         /* dstAddr */
    }};

#define IPV6_DUMMY_PROTOCOL_CNS  0x3b

/* packet's IPv6 */
static TGF_PACKET_IPV6_STC prvTgfPacketIpv6Parts[2] = {
    {
        6,                                      /* version */
        0,                                      /* trafficClass */
        0,                                      /* flowLabel */
        0x02,                                   /* payloadLen */
        IPV6_DUMMY_PROTOCOL_CNS,                /* nextHeader */
        0x40,                                   /* hopLimit */
        {0x6545, 0, 0, 0, 0, 0, 0x0000, 0x3212},/* TGF_IPV6_ADDR srcAddr */
        {0xff05, 0, 0, 0, 0, 0, 0xccdd, 0xeeff} /* TGF_IPV6_ADDR dstAddr */
    },
    {
        6,                                      /* version */
        0,                                      /* trafficClass */
        0,                                      /* flowLabel */
        0x02,                                   /* payloadLen */
        IPV6_DUMMY_PROTOCOL_CNS,                /* nextHeader */
        0x40,                                   /* hopLimit */
        {0x6545, 0, 0, 0, 0, 0, 0x0000, 0x3212},/* TGF_IPV6_ADDR srcAddr */
        {0xff05, 0, 0, 0, 0, 0, 0xccdd, 0xeeff} /* TGF_IPV6_ADDR dstAddr */
    }};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc,
    0xdd, 0xee, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x11, 0x11
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* packet's TCP part */
static TGF_PACKET_TCP_STC prvTgfPacketTcpParts[2] = {
    {
        1,                  /* src port */
        2,                  /* dst port */
        1,                  /* sequence number */
        2,                  /* acknowledgment number */
        0,                  /* data offset */
        0,                  /* reserved */
        0x00,               /* flags */
        4096,               /* window */
        TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,/* cksum */
        0                   /* urgent pointer */
    },
    {
        1,                  /* src port */
        2,                  /* dst port */
        1,                  /* sequence number */
        2,                  /* acknowledgment number */
        0,                  /* data offset */
        0,                  /* reserved */
        0x00,               /* flags */
        4096,               /* window */
        TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,/* cksum */
        0                   /* urgent pointer */
    }};

/* packet's UDP part */
static TGF_PACKET_UDP_STC prvTgfPacketUdpParts[2] = {
    {
        8,                  /* src port */
        0,                  /* dst port */
        TGF_PACKET_AUTO_CALC_LENGTH_CNS,  /* length */
        TGF_PACKET_AUTO_CALC_CHECKSUM_CNS /* cksum */
    },
    {
        8,                  /* src port */
        0,                  /* dst port */
        TGF_PACKET_AUTO_CALC_LENGTH_CNS,  /* length */
        TGF_PACKET_AUTO_CALC_CHECKSUM_CNS /* cksum */
    }};

/* TBD Control Packet */
#if 0 /* Control traffic yet to be supported */
/* ethertype part of ARP packet */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketArpEtherTypePart = {
    TGF_ETHERTYPE_0806_ARP_TAG_CNS
};

/* packet's ARP header */
static TGF_PACKET_ARP_STC prvTgfPacketArpPart = {
    0x01,                                   /* HW Type */
    0x0800,                                 /* Protocol (IPv4= 0x0800) */
    0x06,                                   /* HW Len = 6 */
    0x04,                                   /* Proto Len = 4 */
    0x01,                                   /* Opcode */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x11},   /* HW Address of Sender (MAC SA) */
    {0x11, 0x12, 0x13, 0x14},               /* Protocol Address of Sender(SIP)*/
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},   /* HW Address of Target (MAC DA) */
    {0x30, 0x31, 0x32, 0x33}                /* Protocol Address of Target(DIP)*/
};

/* PARTS of packet ARP */
static TGF_PACKET_PART_STC prvTgfPacketArpPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Parts[0]}, /* type, partPtr */
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketArpEtherTypePart},
    {TGF_PACKET_PART_ARP_E,       &prvTgfPacketArpPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* LENGTH of packet ARP */
#define PRV_TGF_PACKET_ARP_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS \
    + TGF_ARP_HEADER_SIZE_CNS + sizeof(prvTgfPacketPayloadPart)

static TGF_PACKET_STC prvTgfPacketArpInfo = {
    PRV_TGF_PACKET_ARP_LEN_CNS,                                  /* totalLen */
    sizeof(prvTgfPacketArpPartArray)/sizeof(TGF_PACKET_PART_STC),/* numOfParts*/
    prvTgfPacketArpPartArray                                     /* partsArray*/
};
#endif

static TGF_PACKET_PART_STC              global_partsArray[10];

#define TGF_TRAFFIC_GEN_INC_MAC(l2)     \
{                                       \
    l2->saMac[5] += 1;                  \
    if(l2->saMac[5] == 1)               \
    {                                   \
        l2->saMac[4] += 1;              \
        if(l2->saMac[4] == 1)           \
        {                               \
            l2->saMac[3] += 1;          \
            if(l2->saMac[3] == 1)       \
            {                           \
                l2->saMac[2] += 1;      \
            }                           \
        }                               \
    }                                   \
}

#define TGF_TRAFFIC_GEN_INC_IP(ip)      \
{                                       \
    ip->srcAddr[3] += 1;                \
    if(ip->srcAddr[3] == 1)             \
    {                                   \
        ip->srcAddr[2] += 1;            \
        if(ip->srcAddr[2] == 1)         \
        {                               \
            ip->srcAddr[1] += 1;        \
            if(ip->srcAddr[1] == 1)     \
            {                           \
                ip->srcAddr[0] += 1;    \
            }                           \
        }                               \
    }                                   \
}

#define PRINT_VLAN_TAG(col1, col2, tpid, tci)       \
    printf("%10s : %-15s: 0x%04x:0x%04x\n", col1, col2, tpid, tci)

#define PRINT_ARR_MAC(col1, col2, arr, maxIndex)    \
{                                                   \
    GT_U32 n;                                       \
    printf("%10s : %-15s: ", col1, col2);           \
    for (n=0; n<maxIndex-1; n++)                    \
    {                                               \
        printf("0x%02x:",arr[n]);                   \
    }                                               \
    printf("%02x\n",arr[n]);                        \
}

#define PRINT_PORT_MAC(col1, col2, port)            \
    printf("%10s : %-15s: 0x%04x\n", col1, col2, port)

/**
 * @internal prvTgfTrafficGenPrintPkt function
 * @endinternal
 *
 * @brief  print packet details
 */
static GT_VOID prvTgfTrafficGenPrintPkt
(
    IN TGF_PACKET_STC                   *packetInfoPtr,
    IN TGF_TRAFFIC_GEN_PKT_TYPE_ENT     mode
)
{
    GT_U32                              index;
    TGF_PACKET_L2_STC                   *l2Ptr;
    TGF_PACKET_VLAN_TAG_STC             *vlanPtr;
    TGF_PACKET_WILDCARD_STC             *wildCardPtr;
    TGF_PACKET_IPV4_STC                 *ipv4Ptr;
    TGF_PACKET_IPV6_STC                 *ipv6Ptr;
    TGF_PACKET_TCP_STC                  *tcpPtr;
    TGF_PACKET_UDP_STC                  *udpPtr;

    for(index = 0; index<packetInfoPtr->numOfParts; index++)
    {
        switch(packetInfoPtr->partsArray[index].type)
        {
            case TGF_PACKET_PART_L2_E:
                l2Ptr = packetInfoPtr->partsArray[index].partPtr;
                if(index == 0)
                {
                    if((mode >= TGF_TRAFFIC_GEN_PKT_TYPE_L2_IN_L2_E) &&
                            (mode <= TGF_TRAFFIC_GEN_PKT_TYPE_L4_IN_L4_E))
                    {
                        PRINT_ARR_MAC("Outer: L2", "DA", l2Ptr->daMac, 6);
                    }
                    else
                    {
                        PRINT_ARR_MAC("L2", "DA", l2Ptr->daMac, 6);
                    }
                }
                else
                {
                    PRINT_ARR_MAC("Inner: L2", "DA", l2Ptr->daMac, 6);
                }

                PRINT_ARR_MAC(""  , "SA", l2Ptr->saMac, 6);
                break;
            case TGF_PACKET_PART_VLAN_TAG_E:
                vlanPtr = packetInfoPtr->partsArray[index].partPtr;
                PRINT_VLAN_TAG("", "TAG(TPID:TCI)", vlanPtr->etherType,
                        (vlanPtr->pri << 13 | vlanPtr->cfi << 12 | vlanPtr->vid));
                break;
            case TGF_PACKET_PART_WILDCARD_E:
                wildCardPtr = packetInfoPtr->partsArray[index].partPtr;
                PRINT_ARR_MAC("", "custom Tag", wildCardPtr->bytesPtr, wildCardPtr->numOfBytes);
                break;
            case TGF_PACKET_PART_IPV4_E:
                ipv4Ptr = packetInfoPtr->partsArray[index].partPtr;
                PRINT_ARR_MAC("L3", "src ipv4", ipv4Ptr->srcAddr, 4);
                PRINT_ARR_MAC(""  , "dst ipv4", ipv4Ptr->dstAddr, 4);
                break;
            case TGF_PACKET_PART_IPV6_E:
                ipv6Ptr = packetInfoPtr->partsArray[index].partPtr;
                PRINT_ARR_MAC("L3", "src ipv6", ipv6Ptr->srcAddr, 6);
                PRINT_ARR_MAC(""  , "dst ipv6", ipv6Ptr->dstAddr, 6);
                break;
            case TGF_PACKET_PART_TCP_E:
                tcpPtr = packetInfoPtr->partsArray[index].partPtr;
                PRINT_PORT_MAC("L4", "TCP Src Port", tcpPtr->srcPort);
                PRINT_PORT_MAC(""  , "TCP Dst Port", tcpPtr->dstPort);
                break;
            case TGF_PACKET_PART_UDP_E:
                udpPtr = packetInfoPtr->partsArray[index].partPtr;
                PRINT_PORT_MAC("L4", "UDP Src Port", udpPtr->srcPort);
                PRINT_PORT_MAC(""  , "UDP Dst Port", udpPtr->dstPort);
                break;
            default:
                break;
        }
    }
}

/**
 * @internal prvTgfTrafficGenUpdatePkt function
 * @endinternal
 *
 * @brief  update packet
 */
static GT_VOID prvTgfTrafficGenUpdatePkt
(
    IN TGF_TRAFFIC_GEN_STREAM_TYPE_ENT       streamType,
    IN TGF_PACKET_STC                       *packetInfoPtr,
    IN GT_U32                                iterationNum
)
{
    GT_U32                      index;
    TGF_PACKET_L2_STC          *l2Ptr;
    TGF_PACKET_IPV4_STC        *ipv4Ptr;
    TGF_PACKET_IPV6_STC        *ipv6Ptr;
    GT_U32                      rand32Bits;

    rand32Bits = (cpssOsRand() & 0x7FFF) << 15 |
        (cpssOsRand() & 0x7FFF)       |
        iterationNum << 30;

    switch(streamType)
    {
        case TGF_TRAFFIC_GEN_STREAM_TYPE_INCR_SA_MAC_E:
            l2Ptr = packetInfoPtr->partsArray[0].partPtr;
            TGF_TRAFFIC_GEN_INC_MAC(l2Ptr);
            break;
        case TGF_TRAFFIC_GEN_STREAM_TYPE_RAND_SA_MAC_E:
            l2Ptr = packetInfoPtr->partsArray[0].partPtr;
            l2Ptr->saMac[2] = (GT_U8)(rand32Bits >> 24);
            l2Ptr->saMac[3] = (GT_U8)(rand32Bits >> 16);
            l2Ptr->saMac[4] = (GT_U8)(rand32Bits >>  8);
            l2Ptr->saMac[5] = (GT_U8)(rand32Bits >>  0);
            break;
        case TGF_TRAFFIC_GEN_STREAM_TYPE_INCR_SOURCE_IP_E:
            for(index=0; index<TGF_PACKET_PART_MAX_E; index++)
            {
                if(packetInfoPtr->partsArray[index].type == TGF_PACKET_PART_IPV6_E)
                {
                    ipv6Ptr = packetInfoPtr->partsArray[index].partPtr;
                    TGF_TRAFFIC_GEN_INC_IP(ipv6Ptr);
                    break;
                }
                else if(packetInfoPtr->partsArray[index].type == TGF_PACKET_PART_IPV4_E)
                {
                    ipv4Ptr = packetInfoPtr->partsArray[index].partPtr;
                    TGF_TRAFFIC_GEN_INC_IP(ipv4Ptr);
                    break;
                }
            }
            break;
        case TGF_TRAFFIC_GEN_STREAM_TYPE_RAND_SOURCE_IP_E:
            for(index=0; index<TGF_PACKET_PART_MAX_E; index++)
            {
                if(packetInfoPtr->partsArray[index].type == TGF_PACKET_PART_IPV6_E)
                {
                    ipv6Ptr = packetInfoPtr->partsArray[index].partPtr;
                    ipv6Ptr->srcAddr[6] = (GT_U16)(rand32Bits >> 16);
                    ipv6Ptr->srcAddr[7] = (GT_U16)(rand32Bits >> 0);
                    break;
                }
                else if(packetInfoPtr->partsArray[index].type == TGF_PACKET_PART_IPV4_E)
                {
                    ipv4Ptr = packetInfoPtr->partsArray[index].partPtr;
                    ipv4Ptr->srcAddr[0] = (GT_U8)(rand32Bits >> 24);
                    ipv4Ptr->srcAddr[1] = (GT_U8)(rand32Bits >> 16);
                    ipv4Ptr->srcAddr[2] = (GT_U8)(rand32Bits >> 8);
                    ipv4Ptr->srcAddr[3] = (GT_U8)(rand32Bits >> 0);
                    break;
                }
            }
            break;
        default:
            break;
    }
}

/**
 * @internal enableTgfLog function
 * @endinternal
 *
 * @brief  Enable TGF logs
 */
static GT_VOID enableTgfLog(GT_BOOL enable)
{
    TGF_LOG_BINDINGS_STC tgfBindings = {
        prvUtfLogMessage,
        utfEqualVerify,
        utfFailureMsgLog,
        NULL, NULL, NULL };
    if(enable)
    {
        prvTgfLogBindFunctions(&tgfBindings);
    }
    else
    {
        prvTgfLogBindFunctions(NULL);
    }
}

/**
 * @internal prvTgfTrafficGenSendPkt function
 * @endinternal
 *
 * @brief  Send packet
 */
static GT_VOID prvTgfTrafficGenSendPkt
(
    IN GT_U32                               pktCnt,
    IN TGF_TRAFFIC_GEN_STREAM_TYPE_ENT      streamType,
    IN GT_PORT_NUM                          portId,
    IN TGF_PACKET_STC                       *packetInfoPtr
)
{
    GT_STATUS                   rc;
    GT_U32                      index;
    GT_BOOL                     logEnabled;
    GT_BOOL                     linkChangeStatusGet;

    enableTgfLog(GT_TRUE);
    logEnabled = GT_TRUE;

    /* case 1 - single packet
     * case 2 - burst of more than 1 packet without any update */
    if((pktCnt <= 1) || (streamType == TGF_TRAFFIC_GEN_STREAM_TYPE_INPUT_PKT_E))
    {
        /* AUTODOC: setup Packet */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, packetInfoPtr, pktCnt, 0, NULL);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup:");

        /* Send Packet */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, portId);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                prvTgfDevNum, portId);
    }
    /* case 3 - more than 1 packet with stream type */
    else
    {
        /* For SIP6 and above devices port functionality is new and every link
         * change event dumps link change log thus flodding the console
         * hence preventing such message dump.
         */
        linkChangeStatusGet = prvWrAppPrintLinkChangeFlagGet();
        prvWrAppPrintLinkChangeFlagSet(GT_FALSE);

        for(index = 0; index<pktCnt; index++)
        {
            /* Setup Packet */
            rc = prvTgfSetTxSetupEth(prvTgfDevNum, packetInfoPtr, 1, 0, NULL);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup:");

            /* Only print packets for the start and end - in case of stream */
            if((index == 0) || (index == (pktCnt-1)))
            {
                enableTgfLog(GT_TRUE);
                logEnabled = GT_TRUE;
            }
            else if (logEnabled)
            {
                enableTgfLog(GT_FALSE);
            }

            /* Send Packet */
            rc = prvTgfStartTransmitingEth(prvTgfDevNum, portId);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                    prvTgfDevNum, portId);

            /* Update the packet */
            prvTgfTrafficGenUpdatePkt(streamType, packetInfoPtr, index);
        }
        /* restore link change notification to its default state */
        prvWrAppPrintLinkChangeFlagSet(linkChangeStatusGet);
    }
    if(logEnabled)
    {
        enableTgfLog(GT_FALSE);
    }
}

/**
 * @internal prvTgfTrafficGenValidateCustomTag_l4_udp function
 * @endinternal
 *
 * @brief  Parse the custom UDP flags and update the parts ptr
 */
static GT_STATUS prvTgfTrafficGenValidateCustomTag_l4_udp
(
    IN  GT_U8                    *customUdpFlagsPtr,
    OUT TGF_PACKET_UDP_STC       *udpPartsPtr
)
{
    udpPartsPtr->length = customUdpFlagsPtr[0] << 8 |
                            customUdpFlagsPtr[1];
    udpPartsPtr->csum   = customUdpFlagsPtr[2] << 8 |
                            customUdpFlagsPtr[3];
    return GT_OK;
}

/**
 * @internal prvTgfTrafficGenValidateCustomTag_l4_tcp function
 * @endinternal
 *
 * @brief  Parse the custom TCP flags and update the parts ptr
 */
static GT_STATUS prvTgfTrafficGenValidateCustomTag_l4_tcp
(
    IN  GT_U8                    *customTcpFlagsPtr,
    OUT TGF_PACKET_TCP_STC       *tcpPartsPtr
)
{
    tcpPartsPtr->sequenceNum    = customTcpFlagsPtr[0] << 24 |
                                    customTcpFlagsPtr[1] << 16 |
                                    customTcpFlagsPtr[2] << 8 |
                                    customTcpFlagsPtr[3];
    tcpPartsPtr->acknowledgeNum = customTcpFlagsPtr[4] << 24 |
                                    customTcpFlagsPtr[5] << 16 |
                                    customTcpFlagsPtr[6] << 8 |
                                    customTcpFlagsPtr[7];
    tcpPartsPtr->dataOffset     = U32_GET_FIELD_MAC(customTcpFlagsPtr[8], 4, 4);
    tcpPartsPtr->reserved       = U32_GET_FIELD_MAC(customTcpFlagsPtr[8], 1, 3);
    tcpPartsPtr->flags          = U32_GET_FIELD_MAC(customTcpFlagsPtr[8], 0, 1) << 1 |
                                    customTcpFlagsPtr[9];
    tcpPartsPtr->windowSize     = (customTcpFlagsPtr[10] << 8) | customTcpFlagsPtr[11];
    tcpPartsPtr->csum           = (customTcpFlagsPtr[12] << 8) | customTcpFlagsPtr[13];
    tcpPartsPtr->urgentPtr      = (customTcpFlagsPtr[14] << 8) | customTcpFlagsPtr[15];
    return GT_OK;
}

/**
 * @internal prvTgfTrafficGenValidateCustomTag_l3_ipv4 function
 * @endinternal
 *
 * @brief  Parse the custom IPv4 flags and update the parts ptr
 */
static GT_STATUS prvTgfTrafficGenValidateCustomTag_l3_ipv4
(
    IN  GT_U8                    *customIpv4FlagsPtr,
    OUT TGF_PACKET_IPV4_STC      *ipv4PartsPtr
)
{
    /* Version - 4 Bits */
    if(U32_GET_FIELD_MAC(customIpv4FlagsPtr[0], 4, 4) != 4)
    {
        return GT_FAIL;
    }
    ipv4PartsPtr->headerLen     = U32_GET_FIELD_MAC(customIpv4FlagsPtr[0], 0, 4);
    ipv4PartsPtr->typeOfService = U32_GET_FIELD_MAC(customIpv4FlagsPtr[1], 2, 6);
    ipv4PartsPtr->totalLen      = (customIpv4FlagsPtr[2] << 8) | customIpv4FlagsPtr[3];
    ipv4PartsPtr->id            = (customIpv4FlagsPtr[4] << 8) | customIpv4FlagsPtr[5];
    ipv4PartsPtr->flags         = U32_GET_FIELD_MAC(customIpv4FlagsPtr[6], 5, 3);
    ipv4PartsPtr->offset        = U32_GET_FIELD_MAC(customIpv4FlagsPtr[6], 0, 5) << 8 |
                                                    customIpv4FlagsPtr[7];
    ipv4PartsPtr->timeToLive    = customIpv4FlagsPtr[8];
    ipv4PartsPtr->protocol      = customIpv4FlagsPtr[9];
    ipv4PartsPtr->csum          = customIpv4FlagsPtr[10] << 8 | customIpv4FlagsPtr[11];
    return GT_OK;
}

/**
 * @internal prvTgfTrafficGenValidateCustomTag_l3_ipv6 function
 * @endinternal
 *
 * @brief  Parse the custom IPv6 flags and update the parts ptr
 */
static GT_STATUS prvTgfTrafficGenValidateCustomTag_l3_ipv6
(
    IN  GT_U8                    *customIpv6FlagsPtr,
    OUT TGF_PACKET_IPV6_STC      *ipv6PartsPtr
)
{
    /* Version - 4 Bits */
    if(U32_GET_FIELD_MAC(customIpv6FlagsPtr[0], 4, 4) != 6)
    {
        return GT_FAIL;
    }
    ipv6PartsPtr->trafficClass  = U32_GET_FIELD_MAC(customIpv6FlagsPtr[0], 0, 4) << 4 |
                                  U32_GET_FIELD_MAC(customIpv6FlagsPtr[1], 4, 4);

    ipv6PartsPtr->flowLabel     = U32_GET_FIELD_MAC(customIpv6FlagsPtr[1], 0, 4) << 16 |
                                    (customIpv6FlagsPtr[2] << 8) |
                                    customIpv6FlagsPtr[3];
    ipv6PartsPtr->payloadLen    = (customIpv6FlagsPtr[4] << 8) | customIpv6FlagsPtr[5];
    ipv6PartsPtr->nextHeader    = customIpv6FlagsPtr[6];
    ipv6PartsPtr->hopLimit      = customIpv6FlagsPtr[7];
    return GT_OK;
}

/**
 * @internal prvTgfTrafficGenFramePkt_parts function
 * @endinternal
 *
 * @brief  Frame packet parts
 */
static GT_STATUS prvTgfTrafficGenFramePkt_parts
(
    IN  TGF_TRAFFIC_GEN_PARAMS_STC   *dataPtr,
    IN  TGF_TRAFFIC_GEN_PKT_TYPE_ENT  pktType,
    IN  GT_BOOL                       isTunnelPkt,
    OUT TGF_PACKET_STC               *packetPtr
)
{
    /* Indicates index in the parts array */
    GT_U8                       index = packetPtr->numOfParts;
    /* Indicates the position of static packet parts memory */
    GT_U8                       pktPartsIndex;
    GT_STATUS                   rc;

    /* pointing to different static memory while framing the tunnel packet */
    pktPartsIndex = (isTunnelPkt == GT_TRUE)?1:0;

    /* L2 data */
    cpssOsMemCpy(&prvTgfPacketL2Parts[pktPartsIndex].saMac, &dataPtr->saMac, sizeof(dataPtr->saMac));
    cpssOsMemCpy(&prvTgfPacketL2Parts[pktPartsIndex].daMac, &dataPtr->daMac, sizeof(dataPtr->daMac));
    global_partsArray[index].partPtr    = &prvTgfPacketL2Parts[pktPartsIndex];
    global_partsArray[index].type       = TGF_PACKET_PART_L2_E;
    packetPtr->numOfParts               += 1;
    packetPtr->totalLen                 += TGF_L2_HEADER_SIZE_CNS;
    index += 1;

    /* In case VLAN ID and custom tag is given,
     * VLAN ID option is over written by the l2-custom tags */
    if(dataPtr->vlanId != 0)
    {
        prvTgfPacketVlanTag0Parts[pktPartsIndex].vid = dataPtr->vlanId;
        global_partsArray[index].partPtr    = &prvTgfPacketVlanTag0Parts[pktPartsIndex];
        global_partsArray[index].type       = TGF_PACKET_PART_VLAN_TAG_E;
        packetPtr->numOfParts               += 1;
        packetPtr->totalLen                 += TGF_VLAN_TAG_SIZE_CNS;
        index += 1;
    }
    else if((dataPtr->l2tagSize != 0) && ((dataPtr->l2tagSize & 3) == 0))
    {
        prvTgfWildCard[pktPartsIndex].numOfBytes    = dataPtr->l2tagSize;
        prvTgfWildCard[pktPartsIndex].bytesPtr      = dataPtr->customTagL2;
        global_partsArray[index].partPtr            = &prvTgfWildCard[pktPartsIndex];
        global_partsArray[index].type               = TGF_PACKET_PART_WILDCARD_E;
        packetPtr->numOfParts                       += 1;
        packetPtr->totalLen                         += dataPtr->l2tagSize;
        index += 1;
    }

    /* L3 data */
    if(pktType > TGF_TRAFFIC_GEN_PKT_TYPE_L2_E)
    {
        if(dataPtr->isIpv6)
        {
            global_partsArray[index].partPtr    = &etherType[1];
            global_partsArray[index].type       = TGF_PACKET_PART_ETHERTYPE_E;
            index +=1;
            packetPtr->numOfParts += 1;
            cpssOsMemCpy(&prvTgfPacketIpv6Parts[pktPartsIndex].srcAddr,
                    &dataPtr->ipSrcAddr.ipv6Addr,
                    sizeof(dataPtr->ipSrcAddr.ipv6Addr));
            cpssOsMemCpy(&prvTgfPacketIpv6Parts[pktPartsIndex].dstAddr,
                    &dataPtr->ipDstAddr.ipv6Addr,
                    sizeof(dataPtr->ipDstAddr.ipv6Addr));
            global_partsArray[index].partPtr    = &prvTgfPacketIpv6Parts[pktPartsIndex];
            global_partsArray[index].type       = TGF_PACKET_PART_IPV6_E;
            packetPtr->totalLen                 += TGF_IPV6_HEADER_SIZE_CNS;
            if(dataPtr->isCustomTagL3)
            {
                rc = prvTgfTrafficGenValidateCustomTag_l3_ipv6(
                        dataPtr->customTagL3.ipv6,
                        &prvTgfPacketIpv6Parts[pktPartsIndex]);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }
        }
        else
        {
            global_partsArray[index].partPtr    = &etherType[0];
            global_partsArray[index].type       = TGF_PACKET_PART_ETHERTYPE_E;
            index +=1;
            packetPtr->numOfParts += 1;
            cpssOsMemCpy(&prvTgfPacketIpv4Parts[pktPartsIndex].srcAddr,
                    &dataPtr->ipSrcAddr.ipv4Addr,
                    sizeof(dataPtr->ipSrcAddr.ipv4Addr));
            cpssOsMemCpy(&prvTgfPacketIpv4Parts[pktPartsIndex].dstAddr,
                    &dataPtr->ipDstAddr.ipv4Addr,
                    sizeof(dataPtr->ipDstAddr.ipv4Addr));
            global_partsArray[index].partPtr    = &prvTgfPacketIpv4Parts[pktPartsIndex];
            global_partsArray[index].type       = TGF_PACKET_PART_IPV4_E;
            packetPtr->totalLen                 += TGF_IPV4_HEADER_SIZE_CNS;
            if(dataPtr->isCustomTagL4)
            {
                rc = prvTgfTrafficGenValidateCustomTag_l3_ipv4(
                        dataPtr->customTagL3.ipv4,
                        &prvTgfPacketIpv4Parts[pktPartsIndex]);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }
        }
        packetPtr->numOfParts += 1;
        index += 1;
    }

    /* L4 data */
    if(pktType > TGF_TRAFFIC_GEN_PKT_TYPE_L3_E)
    {
        if(dataPtr->isTcp)
        {
            if(dataPtr->isIpv6)
            {
                prvTgfPacketIpv6Parts[pktPartsIndex].nextHeader = protocols[0];
            }
            else
            {
                prvTgfPacketIpv4Parts[pktPartsIndex].protocol = protocols[0];
            }
            prvTgfPacketTcpParts[pktPartsIndex].srcPort = dataPtr->tcpSrcPort;
            prvTgfPacketTcpParts[pktPartsIndex].dstPort = dataPtr->tcpDstPort;
            global_partsArray[index].partPtr    = &prvTgfPacketTcpParts[pktPartsIndex];
            global_partsArray[index].type       = TGF_PACKET_PART_TCP_E;
            packetPtr->totalLen                 += TGF_TCP_HEADER_SIZE_CNS;
            rc = prvTgfTrafficGenValidateCustomTag_l4_tcp(
                    dataPtr->customTagL4.tcp,
                    &prvTgfPacketTcpParts[pktPartsIndex]);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
        else
        {
            if(dataPtr->isIpv6)
            {
                prvTgfPacketIpv6Parts[pktPartsIndex].nextHeader = protocols[1];
            }
            else
            {
                prvTgfPacketIpv4Parts[pktPartsIndex].protocol = protocols[1];
            }
            prvTgfPacketUdpParts[pktPartsIndex].srcPort = dataPtr->udpSrcPort;
            prvTgfPacketUdpParts[pktPartsIndex].dstPort = dataPtr->udpDstPort;
            global_partsArray[index].partPtr    = &prvTgfPacketUdpParts[pktPartsIndex];
            global_partsArray[index].type       = TGF_PACKET_PART_UDP_E;
            packetPtr->totalLen                 += TGF_UDP_HEADER_SIZE_CNS;
            rc = prvTgfTrafficGenValidateCustomTag_l4_udp(
                    dataPtr->customTagL4.udp,
                    &prvTgfPacketUdpParts[pktPartsIndex]);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
        packetPtr->numOfParts += 1;
        /* NO further data, index += 1; */
    }
    return GT_OK;
}

/**
 * @internal prvTgfTrafficGenFramePkt function
 * @endinternal
 *
 * @brief  Frame packet
 */
static GT_STATUS prvTgfTrafficGenFramePkt
(
    IN  TGF_TRAFFIC_GEN_PARAMS_STC   *paramsPtr,
    IN  TGF_TRAFFIC_GEN_PARAMS_STC   *tunnelParamsPtr,
    OUT TGF_PACKET_STC               *packetPtr
)
{
    TGF_TRAFFIC_GEN_PKT_TYPE_ENT    inner_pkt_type = TGF_TRAFFIC_GEN_PKT_TYPE_L2_E;
    GT_STATUS                       rc = GT_OK;
    GT_BOOL                         isFreeReq = GT_FALSE;

    switch(paramsPtr->pktType)
    {
        case TGF_TRAFFIC_GEN_PKT_TYPE_L2_E:
            inner_pkt_type = TGF_TRAFFIC_GEN_PKT_TYPE_L2_E;
            printf("Sending packet: <L2> + Payload\n\n");
            break;
        case TGF_TRAFFIC_GEN_PKT_TYPE_L3_E:
            inner_pkt_type = TGF_TRAFFIC_GEN_PKT_TYPE_L3_E;
            printf("Sending packet: <L2> + <L3> + Payload\n\n");
            break;
        case TGF_TRAFFIC_GEN_PKT_TYPE_L4_E:
            inner_pkt_type = TGF_TRAFFIC_GEN_PKT_TYPE_L4_E;
            printf("Sending packet: <L2> + <L3> + <L4> + Payload\n\n");
            break;
        case TGF_TRAFFIC_GEN_PKT_TYPE_L2_IN_L2_E:
            inner_pkt_type           = TGF_TRAFFIC_GEN_PKT_TYPE_L2_E;
            tunnelParamsPtr->pktType = TGF_TRAFFIC_GEN_PKT_TYPE_L2_E;
            printf("Sending packet: <L2> + <L2> + Payload\n\n");
            break;
        case TGF_TRAFFIC_GEN_PKT_TYPE_L2_IN_L3_E:
            inner_pkt_type           = TGF_TRAFFIC_GEN_PKT_TYPE_L2_E;
            tunnelParamsPtr->pktType = TGF_TRAFFIC_GEN_PKT_TYPE_L3_E;
            printf("Sending packet: <L2> + <L3> + <L2> + Payload\n\n");
            break;
        case TGF_TRAFFIC_GEN_PKT_TYPE_L2_IN_L4_E:
            inner_pkt_type          = TGF_TRAFFIC_GEN_PKT_TYPE_L2_E;
            tunnelParamsPtr->pktType = TGF_TRAFFIC_GEN_PKT_TYPE_L4_E;
            printf("Sending packet: <L2> + <L3> + <L4> + <L2> + Payload\n\n");
            break;
        case TGF_TRAFFIC_GEN_PKT_TYPE_L3_IN_L2_E:
            inner_pkt_type          = TGF_TRAFFIC_GEN_PKT_TYPE_L3_E;
            tunnelParamsPtr->pktType = TGF_TRAFFIC_GEN_PKT_TYPE_L2_E;
            printf("Sending packet: <L2> + <L2> + <L3> + Payload\n\n");
            break;
        case TGF_TRAFFIC_GEN_PKT_TYPE_L3_IN_L3_E:
            inner_pkt_type       = TGF_TRAFFIC_GEN_PKT_TYPE_L3_E;
            tunnelParamsPtr->pktType = TGF_TRAFFIC_GEN_PKT_TYPE_L3_E;
            printf("Sending packet: <L2> + <L3> + <L2> + <L3> + Payload\n\n");
            break;
        case TGF_TRAFFIC_GEN_PKT_TYPE_L3_IN_L4_E:
            inner_pkt_type       = TGF_TRAFFIC_GEN_PKT_TYPE_L3_E;
            tunnelParamsPtr->pktType = TGF_TRAFFIC_GEN_PKT_TYPE_L4_E;
            printf("Sending packet: <L2> + <L3> + <L4> + <L2> + <L3> + Payload\n\n");
            break;
        case TGF_TRAFFIC_GEN_PKT_TYPE_L4_IN_L2_E:
            inner_pkt_type       = TGF_TRAFFIC_GEN_PKT_TYPE_L4_E;
            tunnelParamsPtr->pktType = TGF_TRAFFIC_GEN_PKT_TYPE_L2_E;
            printf("Sending packet: <L2> + <L2> + <L3> + <L4> + Payload\n\n");
            break;
        case TGF_TRAFFIC_GEN_PKT_TYPE_L4_IN_L3_E:
            inner_pkt_type       = TGF_TRAFFIC_GEN_PKT_TYPE_L4_E;
            tunnelParamsPtr->pktType = TGF_TRAFFIC_GEN_PKT_TYPE_L3_E;
            printf("Sending packet: <L2> + <L3> + <L2> + <L3> + <L4> + Payload\n\n");
            break;
        case TGF_TRAFFIC_GEN_PKT_TYPE_L4_IN_L4_E:
            inner_pkt_type       = TGF_TRAFFIC_GEN_PKT_TYPE_L4_E;
            tunnelParamsPtr->pktType = TGF_TRAFFIC_GEN_PKT_TYPE_L4_E;
            printf("Sending packet: <L2> + <L3> + <L4> + <L2> + <L3> + <L4> + Payload\n\n");
            break;
        case TGF_TRAFFIC_GEN_PKT_TYPE_CTRL_ARP_E:
            printf("Sending packet: ARP\n\n");
            break;
        case TGF_TRAFFIC_GEN_PKT_TYPE_CTRL_ICMP_E:
            printf("Sending packet: ICMP\n\n");
            break;
        default:
            break;
    }

    /* Reset Packet memory before set */
    cpssOsMemSet(global_partsArray, 0x0, sizeof(global_partsArray));
    packetPtr->numOfParts   = 0;
    packetPtr->totalLen     = 0;
    packetPtr->partsArray   = global_partsArray;

    /* Frame the outer(Tunnel info) packet only in case of tunneled packet */
    if((paramsPtr->pktType >= TGF_TRAFFIC_GEN_PKT_TYPE_L2_IN_L2_E) &&
            (paramsPtr->pktType <= TGF_TRAFFIC_GEN_PKT_TYPE_L4_IN_L4_E))
    {
        rc = prvTgfTrafficGenFramePkt_parts(tunnelParamsPtr, tunnelParamsPtr->pktType, GT_TRUE, packetPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /* Frame the original packet(inner packet in case of tunnel)
     * for non-control packet */
    if(paramsPtr->pktType <= TGF_TRAFFIC_GEN_PKT_TYPE_L4_IN_L4_E)
    {
        rc = prvTgfTrafficGenFramePkt_parts(paramsPtr, inner_pkt_type, GT_FALSE, packetPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /* Fill the Payload data at the end
     * If packet size is given and the payload length is high, Allocate the payload
     * TBD - Need to allocate from pool buffer ? */
    if((paramsPtr->pktSize != 0) &&
        ((packetPtr->totalLen + sizeof(prvTgfPayloadDataArr)) < paramsPtr->pktSize))
    {
        prvTgfPacketPayloadPart.dataPtr = cpssOsMalloc(paramsPtr->pktSize - packetPtr->totalLen);
        if(prvTgfPacketPayloadPart.dataPtr == NULL)
        {
            return GT_OUT_OF_CPU_MEM;
        }
        prvTgfPacketPayloadPart.dataLength = paramsPtr->pktSize - packetPtr->totalLen;
        isFreeReq = GT_TRUE;
    }
    global_partsArray[packetPtr->numOfParts].partPtr    = &prvTgfPacketPayloadPart;
    global_partsArray[packetPtr->numOfParts].type       = TGF_PACKET_PART_PAYLOAD_E;
    packetPtr->numOfParts                               += 1;
    packetPtr->totalLen                                 += sizeof(prvTgfPayloadDataArr);

    if(isFreeReq)
    {
        cpssOsFree(prvTgfPacketPayloadPart.dataPtr);
        prvTgfPacketPayloadPart.dataPtr     = prvTgfPayloadDataArr;
        prvTgfPacketPayloadPart.dataLength  = sizeof(prvTgfPayloadDataArr);
    }
    return GT_OK;
}

/**
 * @internal prvTgfTrafficGenWithOptions function
 * @endinternal
 *
 * @brief  API for framing packet and sending to the interface.
 */
GT_STATUS prvTgfTrafficGenWithOptions
(
    IN TGF_TRAFFIC_GEN_PARAMS_STC   *params,
    IN TGF_TRAFFIC_GEN_PARAMS_STC   *tunnelParams
)
{
    GT_STATUS                       rc = GT_OK;
    TGF_PACKET_STC                  packet;

    /* Frame the packet according to the options */
    rc = prvTgfTrafficGenFramePkt(params, tunnelParams, &packet);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Print the packet input details */
    prvTgfTrafficGenPrintPkt(&packet, params->pktType);

    /* Send the packet to the desired interface */
    prvTgfTrafficGenSendPkt(params->pktCnt, params->streamType, params->port, &packet);

    return rc;
}

