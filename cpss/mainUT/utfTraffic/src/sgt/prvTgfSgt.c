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
* @file prvTgfSgt.c
*
* @brief Security group tag test cases
*
* @version   1
********************************************************************************
*/
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
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

/* SGT value - Indicating Group based policy ID */
#define PRV_TGF_SGT_VALUE_CNS           0xF1A8

#define IPV6_DUMMY_PROTOCOL_CNS         0x3b
#define COPY_RESERVED_MASK_CNS          0x3F   /* [6:1] bit position */
#define PRV_TGF_IPCL_0_ID               0
#define PRV_TGF_IPCL_1_ID               1
#define PRV_TGF_EPCL_PCL_ID_CNS         1000;

/* VLAN ID */
#define PRV_TGF_VLANID_2_CNS            2
#define PRV_TGF_DEF_VLANID_CNS          1

#define PRV_TGF_BURST_COUNT_CNS         1
#define PRV_TGF_SOURCE_ID_CNS           1

/* Tunnel start entry index */
#define PRV_TGF_TUNNEL_START_INDEX_CNS      8

/* Template profile index */
#define PRV_TGF_TEMPLATE_PROFILE_INDEX_CNS  1

/* first bit of source ID that is taken for target index in EPCL */
#define PRV_TGF_SRC_ID_LSB_CNS 6

/* index to EPCL Configuration table in SRC_TRG mode */
#define PRV_TGF_EPCL_SRC_TRG_INDEX(_srcIndex, _dstIndex)      ((_dstIndex * 64) + _srcIndex)

/* index to EPCL Rule Index */
#define PRV_TGF_EPCL_RULE_INDEX(_index)      (prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet(10 + _index))


/* Send & Receive port */
typedef enum
{
    PRV_TGF_SGT_SEND_PORT_1_IDX_CNS       = 0,
    PRV_TGF_SGT_SEND_PORT_2_IDX_CNS       = 1,
    PRV_TGF_SGT_RECEIVE_PORT_1_IDX_CNS    = 2,
    PRV_TGF_SGT_RECEIVE_PORT_2_IDX_CNS    = 3
} PRV_TGF_SGT_PORT_IDX_ENT;

/* Hawk PHA thread ID */
typedef enum{
     THR0_DisablePha           /* Restore */
    ,THR1_SGT_NetAddMSB = 1
    ,THR2_SGT_NetFix
    ,THR3_SGT_NetRemove
    ,THR4_SGT_eDSAFix
    ,THR5_SGT_eDSARemove
    ,THR6_SGT_GBPFixIPv4
    ,THR7_SGT_GBPFixIPv6
    ,THR8_SGT_GBPRemoveIPv4
    ,THR9_SGT_GBPRemoveIPv6
}SGT_THREAD_ID_ENT;

/* parameters that are needed to be restored */
static struct
{
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC                egressInfo;
    PRV_TGF_TUNNEL_START_ENTRY_UNT                  tunnelEntry;
    CPSS_TUNNEL_TYPE_ENT                            tunnelType;
    PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT    cfgTabAccMode;
    PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT    cfgTabAccMode_2ndPort;
    PRV_TGF_PTP_TS_TAG_PORT_CFG_STC                 tsTagPortCfg[2];
} prvTgfSgtRestoreCfg;

/* the Virtual Router index */
static GT_U32        prvUtfVrfId                = 0;
static GT_U32        prvTgfLpmDBId              = 0;
/* the Route entry index for UC Route entry Table */
static GT_U32           prvTgfRouteEntryBaseIndex = 10;

/* FDB MAC entry */
static CPSS_INTERFACE_INFO_STC      egressPortInterface;
static GT_U8                        packetBuf[256];
static GT_U8                        expectedSgtTag[] = {0x89, 0x09, 0x01, 0x01, 0x00, 0x01, 0xf1, 0xa8};

#if 0
static TGF_MAC_ADDR        PORT_1_MAC = {0x00, 0x00, 0x00, 0x00, 0x11, 0xAA};
static TGF_MAC_ADDR        PORT_3_MAC = {0x00, 0x00, 0x00, 0x00, 0x33, 0xCC};
#endif
/* Bridging - FDB Entry */
static TGF_MAC_ADDR        PORT_2_MAC = {0x00, 0x00, 0x00, 0x00, 0x22, 0xBB};
static TGF_MAC_ADDR        PORT_4_MAC = {0x00, 0x00, 0x00, 0x00, 0x44, 0xDD};

/* Routing Rule to distinguish */
static TGF_IPV6_ADDR       pkt_dip_arr[2] = {
    /* User Port*/
    {0x2001, 0, 0, 0, 0, 0, 0xdb8,  0x2},
    {0x2001, 0, 0, 0, 0, 0, 0xdb8,  0x3}
};

/* Tunnel entry */
static TGF_IPV6_ADDR prvTgfTunnelStartSrcIPv6   = {0x2001, 0x2002, 0x2003, 0x2004, 0x2005, 0x2006, 0x2007, 0x2008};
static TGF_IPV6_ADDR prvTgfTunnelStartDstIPv6   = {0x3001, 0x3002, 0x3003, 0x3004, 0x3005, 0x3006, 0x3007, 0x3008};
static TGF_IPV4_ADDR prvTgfTunnelDstIp          = {1, 1, 1, 2};
static TGF_IPV4_ADDR prvTgfTunnelSrcIp          = {2, 2, 3, 3};
static PRV_TGF_TUNNEL_START_GEN_IP_PROFILE_STC  profileData;
static PRV_TGF_BRG_EGRESS_PORT_INFO_STC         egressInfo;
static PRV_TGF_TUNNEL_START_ENTRY_UNT           tunnelEntry;
static GT_U32                                   arpIndex = 1;
static TGF_MAC_ADDR                             prvTgfArpMac = {0x00, 0x00, 0x00, 0x00, 0x00, 0xaa};
static GT_U32                                   prvTgfSgtDstIndexArr[] = {54, 52, 49};
static GT_U32                                   prvTgfSgtSrcIndexArr[] = {0,  62, 62};
static CPSS_DXCH_BRG_VLAN_8B_TAG_PARSE_CONFIG_STC parseConfig = {
        1,                                      /* enable               */
        4,                                      /* extractedValueOffset */
        {0x01, 0x01, 0x00, 0x01, 0xF1, 0xA8},   /* tagTciValue          */
        {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},   /* tagTciMask           */
        GT_FALSE
    };

/* IPCL Rule to distinguish */
static TGF_IPV6_ADDR       pkt_sip_arr[2] = {
    /* User Port*/
    {0x2001, 0, 0, 0, 0, 0, 0xdb8,  0x1},
    {0x2001, 0, 0, 0, 0, 0, 0xdb8,  0x91}
};

/* TODO - SIP is not working so using SA MAC for IPCL */
static TGF_MAC_ADDR sa_mac[2] = {
    {0x00, 0x00, 0x00, 0x00, 0xdb, 0x1},
    {0x00, 0x00, 0x00, 0x00, 0xdb, 0x91}
};

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x11, 0xAA},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}                /* srcMac */
};

/* packet's UDP part */
static TGF_PACKET_UDP_STC prvTgfPacketUdpPart =
{
    0x2000,                           /* src port */
    0x4790,                           /* dst port */
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,  /* length */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS /* csum */
};

/* VLAN_TAG part - Tag0 */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag0Part = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_2_CNS                          /* pri, cfi, VlanId */
};

/* SGT TAG part */
static TGF_PACKET_SGT_TAG_STC prvTgfPacketSgtTagPart = {
    TGF_ETHERTYPE_8909_SGT_TAG_CNS, /* etherType */
    1,                              /* version   */
    16,                             /* Length    */
    1,                              /* SGT Option*/
    PRV_TGF_SGT_VALUE_CNS           /* SGT Value */
};

/* IPv6 packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart_Ipv6 = {
    TGF_ETHERTYPE_86DD_IPV6_TAG_CNS
};

/* packet's IPv6 */
static TGF_PACKET_IPV6_STC prvTgfPacketIpv6Part = {
    6,                       /* version */
    0,                       /* trafficClass */
    0,                       /* flowLabel */
    0x02,                    /* payloadLen */
    TGF_PROTOCOL_UDP_E,      /* nextHeader */
    0x40,                    /* hopLimit */
    {0x6545, 0, 0, 0, 0, 0, 0x0000, 0x3212}, /* TGF_IPV6_ADDR srcAddr */
    {0xff05, 0, 0, 0, 0, 0, 0xccdd, 0xeeff}  /* TGF_IPV6_ADDR dstAddr */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc,
    0xdd, 0xee, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* LENGTH of tagged packet */
#define PRV_TGF_PACKET_LEN_MAC(tagLen) \
    TGF_L2_HEADER_SIZE_CNS + (tagLen) + TGF_IPV6_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

#define PRV_TGF_VXLAN_OFFSET_MAC(tagLen) \
    TGF_L2_HEADER_SIZE_CNS + (tagLen) + TGF_ETHERTYPE_SIZE_CNS + TGF_IPV6_HEADER_SIZE_CNS + TGF_UDP_HEADER_SIZE_CNS

/************************ Untagged packet ********************************/
/* PARTS of untagged packet */
static TGF_PACKET_PART_STC prvTgfPacketUnTagPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart_Ipv6},
    {TGF_PACKET_PART_IPV6_E,      &prvTgfPacketIpv6Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Untagged PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfoUnTag =
{
    PRV_TGF_PACKET_LEN_MAC(0),                                        /* totalLen */
    sizeof(prvTgfPacketUnTagPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketUnTagPartArray                                        /* partsArray */
};

/************************ Single tagged packet ********************************/
/* PARTS of single tagged packet */
static TGF_PACKET_PART_STC prvTgfPacketSingleTagPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTag0Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart_Ipv6},
    {TGF_PACKET_PART_IPV6_E,      &prvTgfPacketIpv6Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Single tagged PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfoSingleTag =
{
    PRV_TGF_PACKET_LEN_MAC(TGF_VLAN_TAG_SIZE_CNS),                        /* totalLen */
    sizeof(prvTgfPacketSingleTagPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketSingleTagPartArray                                        /* partsArray */
};

#if 0
#define PRV_TGF_VLANID_3_CNS            3
/* VLAN_TAG part - Tag1 */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag1Part = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_3_CNS                          /* pri, cfi, VlanId */
};

/***************** Double tagged packet (Tag0, Tag1 - no SGT Tag) ****************/
/* PARTS of double tagged packet */
static TGF_PACKET_PART_STC prvTgfPacketDoubleTagPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTag0Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTag1Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart_Ipv6},
    {TGF_PACKET_PART_IPV6_E,      &prvTgfPacketIpv6Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Double tagged PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfoDoubleTag =
{
    PRV_TGF_PACKET_LEN_MAC(2*TGF_VLAN_TAG_SIZE_CNS),                      /* totalLen */
    sizeof(prvTgfPacketDoubleTagPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketDoubleTagPartArray                                        /* partsArray */
};
#endif

/***************** Double tagged packet (Tag0, Tag1 - SGT Tag) ****************/
/* PARTS of TAG0 + SGT tagged packet */
static TGF_PACKET_PART_STC prvTgfPacketSgtAndVlanTagPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTag0Part},
    {TGF_PACKET_PART_SGT_TAG_E,   &prvTgfPacketSgtTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart_Ipv6},
    {TGF_PACKET_PART_IPV6_E,      &prvTgfPacketIpv6Part},
    {TGF_PACKET_PART_UDP_E,       &prvTgfPacketUdpPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* TAG0 + SGT tagged PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfoSgtAndVlanTag =
{
    PRV_TGF_PACKET_LEN_MAC(TGF_VLAN_TAG_SIZE_CNS+TGF_SGT_TAG_SIZE_CNS),       /* totalLen */
    sizeof(prvTgfPacketSgtAndVlanTagPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketSgtAndVlanTagPartArray                                        /* partsArray */
};

/* packet's UDP header */
static TGF_PACKET_UDP_STC prvTgfPacketTunnelInputUdpPart = {
    0x1010,
    0x12b6, /* UDP Destination = 4790 (VXLAN-GPE) */
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS
};

/* packet's VXLAN-GPE header - Holding SGT tag value in VNI field */
static TGF_PACKET_VXLAN_GPE_STC prvTgfPacketTunnelInputVxlanGpePart = {
    0x80,               /* Flags */
    0x0,                /* Reserved */
    0x00,               /* Next Protocol */
    0xF1A8,             /* VXLAN Network Identifier (VNI) - 24Bits */
    0x0                 /* Reserved */
};

/* Tunnel section: L2 part */
static TGF_PACKET_L2_STC prvTgfPacketTunnelL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x44, 0x04},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x45, 0x05}                /* saMac */
};

/* PARTS of input packet: |L2|EtherType|IPv6|UDP|VXLAN-GPE|payload passenger| */
static TGF_PACKET_PART_STC prvTgfPacketIpv6TunnelWithVxlanPartArray[] = {
    {TGF_PACKET_PART_L2_E,           &prvTgfPacketTunnelL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,     &prvTgfPacketVlanTag0Part},
    {TGF_PACKET_PART_ETHERTYPE_E,    &prvTgfPacketEtherTypePart_Ipv6},
    {TGF_PACKET_PART_IPV6_E,         &prvTgfPacketIpv6Part},
    {TGF_PACKET_PART_UDP_E,          &prvTgfPacketTunnelInputUdpPart},
    {TGF_PACKET_PART_VXLAN_GPE_E,    &prvTgfPacketTunnelInputVxlanGpePart},
    {TGF_PACKET_PART_L2_E,           &prvTgfPacketL2Part},
    {TGF_PACKET_PART_PAYLOAD_E,      &prvTgfPacketPayloadPart}
};

/* TAG0 + SGT tagged PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfoIpv6WithVxlan =
{
    PRV_TGF_PACKET_LEN_MAC(TGF_L2_HEADER_SIZE_CNS+TGF_UDP_HEADER_SIZE_CNS+TGF_VXLAN_GPE_HEADER_SIZE_CNS),       /* totalLen */
    sizeof(prvTgfPacketIpv6TunnelWithVxlanPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketIpv6TunnelWithVxlanPartArray                                        /* partsArray */
};

static GT_VOID tgfSgtUseSrcTrgMode_test1_config
(
    IN GT_BOOL      srcEpgFdb,
    IN GT_U32       sgtIndex
);

static CPSS_DXCH_BRG_FDB_EPG_CONFIG_STC    fdbEpgConfigRestore;
/* TTI Rule index */
static GT_U32   prvTgfTtiRuleIndex = 1;

/*************** Utility APIs *******************/
/**
* @internal prvTgfSgtTpidConfig function
* @endinternal
*
* @brief  Add SGT Tag to global TPID index
*/
static GT_VOID prvTgfSgtTpidConfig
(
    IN GT_BOOL              status,
    IN CPSS_DIRECTION_ENT   direction,
    IN GT_U32               tpidIndex,
    IN GT_PORT_NUM          egressPort
)
{
    GT_STATUS           rc;
    TGF_ETHER_TYPE      etherType;

    /* +-------------------+----------------+----------------+--------------------+-------------------+-----------------+
     * | EtherType(16Bits) | Version(8Bits) | Length(12Bits) | OptionType(12Bits) | SGT Value(14Bits) | OtherCmd(2Bits) |
     * +-------------------+----------------+----------------+--------------------+-------------------+-----------------+
     * | 0x8909            | 0x01           | 0x010          | 0x001              | 0xF1A8            |               0 |
     * +-------------------+----------------+----------------+--------------------+-------------------+-----------------+
     */

    if(status)
    {
        parseConfig.enable = 1;
        etherType = TGF_ETHERTYPE_8909_SGT_TAG_CNS;
    }
    else
    {
        parseConfig.enable = 0;
        etherType = 0x0000;
    }

    rc = cpssDxChBrgVlanPort8BTagInfoSet(prvTgfDevNum, direction , tpidIndex, &parseConfig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanPort8BTagInfoSet");

    if(direction == CPSS_DIRECTION_EGRESS_E)
    {
        rc = prvTgfBrgVlanEgressEtherTypeSet(CPSS_VLAN_ETHERTYPE1_E,
                etherType);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEgressEtherTypeSet");

        rc = prvTgfBrgVlanPortEgressTpidSet(prvTgfDevNum, egressPort,
                CPSS_VLAN_ETHERTYPE1_E, tpidIndex);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortEgressTpidSet");
    }
    else
    {
        rc = prvTgfBrgVlanIngressEtherTypeSet(CPSS_VLAN_ETHERTYPE1_E,
                etherType, 4094);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanIngressEtherTypeSet");
    }

    rc = cpssDxChBrgVlanTpidTagTypeSet(prvTgfDevNum, direction,
            tpidIndex, CPSS_BRG_TPID_SIZE_TYPE_8_BYTES_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanTpidTagTypeSet");
}

/**
* @internal prvTgfSgtPtpConfig function
* @endinternal
*
* @brief  Enable timestamping on port
*/
static GT_VOID prvTgfSgtPtpConfig
(
    IN PRV_TGF_PTP_TS_TAG_MODE_ENT      tagMode,
    IN GT_BOOL                          restoreEnable
)
{
    PRV_TGF_PTP_TS_TAG_PORT_CFG_STC     tsTagPortCfg, *tsTagPortCfgPtr;
    GT_STATUS                           rc;
    GT_U8                               portLoop;
    GT_U8                               index = 0;

    /*set per port configuration timestamp tag*/
    tsTagPortCfg.tsReceptionEnable  = GT_TRUE;
    tsTagPortCfg.tsPiggyBackEnable  = GT_FALSE;
    tsTagPortCfg.tsTagMode          = tagMode;

    for(portLoop=PRV_TGF_SGT_RECEIVE_PORT_1_IDX_CNS;
            portLoop<=PRV_TGF_SGT_RECEIVE_PORT_2_IDX_CNS;portLoop++,index++)
    {
        if(restoreEnable)
        {
            /* Restore Structure */
            tsTagPortCfgPtr = &prvTgfSgtRestoreCfg.tsTagPortCfg[index];
        }
        else
        {
            /* Set restore point */
            rc = prvTgfPtpTsTagPortCfgGet(
                    prvTgfPortsArray[portLoop],
                    &prvTgfSgtRestoreCfg.tsTagPortCfg[index]);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsTagPortCfgGet");
            tsTagPortCfgPtr = &tsTagPortCfg;
        }

        /* Configure timestamp enable */
        rc = prvTgfPtpTsTagPortCfgSet(
                prvTgfPortsArray[portLoop],
                tsTagPortCfgPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsTagPortCfgSet");
    }
}

/**
* @internal prvTgfSgtPclUdbConfig function
* @endinternal
*
* @brief  Add PCL UDP configuration
*/
static GT_VOID prvTgfSgtPclUdbConfig(GT_VOID)
{

    GT_U32 udbData[4][2]={/*UDB Index, Anchor offset */
        /* TODO - is the offset align is correct check with PHA */
        {57,       28},
        {56,       30},
        {55,       29},
        {54,       103}};
    GT_U32              udbDataIndex;
    GT_STATUS           rc;

    /***** UDB configuration(UDB 55, 56, 57 *************/
    for (udbDataIndex=0; udbDataIndex<4; udbDataIndex++)
    {
        rc = prvTgfPclUserDefinedByteSet(
                PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_E,
                PRV_TGF_PCL_PACKET_TYPE_IPV6_E,
                CPSS_PCL_DIRECTION_EGRESS_E,
                udbData[udbDataIndex][0],
                PRV_TGF_PCL_OFFSET_METADATA_E,
                udbData[udbDataIndex][1]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                "prvTgfPclUserDefinedByteSet: %d", prvTgfDevNum);
    }
}

static GT_BOOL                     getFirst = GT_TRUE;
static GT_BOOL                     dropExpected = GT_FALSE;

/* Types of SGT Tag Verification for egress packet */
typedef enum {
    PRV_TGF_SGT_VERIFY_SGT_E,               /* SGT TAG                          */
    PRV_TGF_SGT_VERIFY_SGT_WITH_TAG_0_E,    /* VLAN TAG0 + SGT TAG              */
    PRV_TGF_SGT_VERIFY_SGT_WITH_TAG_0_1_E,  /* VLAN TAG0 + VLAN TAG1 + SGT TAG  */
    PRV_TGF_SGT_VERIFY_NO_SGT_E,            /* NO SGT tag, No VLAN              */
    PRV_TGF_SGT_VERIFY_NO_SGT_WITH_TAG0_E,  /* NO SGT tag. With VLAN Tag0       */
    PRV_TGF_SGT_VERIFY_SGT_IN_TUNNEL_E,     /* SGT tag value at tunnel offset   */
    PRV_TGF_SGT_VERIFY_NO_SGT_IN_TUNNEL_E   /* SGT tag value at tunnel offset   */
} PRV_TGF_SGT_VERIFY_ENT;

/**
* @internal prvTgfSgtSendAndCapturePkt function
* @endinternal
*
* @brief  Send packet
*/
static GT_VOID prvTgfSgtSendAndCapturePkt
(
    IN GT_PORT_NUM        inPortId,
    IN GT_PORT_NUM        outPortId,
    IN TGF_PACKET_STC    *packetInfoPtr
)
{
    GT_STATUS                   rc;
    GT_U32                      packetLen = sizeof(packetBuf);
    GT_U32                      packetActualLength = 0;
    GT_U8                       queue = 0;
    GT_U8                       dev = 0;
    TGF_NET_DSA_STC             rxParam;

#if 0
    CPSS_PORT_MAC_COUNTER_SET_STC       portCntrs;
    GT_U8                               portIter;
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
    }
#endif

    /* Update egress portInterface for capturing */
    egressPortInterface.type              = CPSS_INTERFACE_PORT_E;
    egressPortInterface.devPort.hwDevNum  = prvTgfDevNum;
    egressPortInterface.devPort.portNum   = outPortId;

    /* AUTODOC: enable capture on egress port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&egressPortInterface,
            TGF_CAPTURE_MODE_MIRRORING_E,
            GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "Failure in tgftrafficgeneratorporttxethcaptureset port - %d",
            egressPortInterface.devPort.portNum);

    /* AUTODOC: setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, packetInfoPtr, 1, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup:");

    /* AUTODOC: send Packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, inPortId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
            prvTgfDevNum, inPortId);

    /* AUTODOC: Disable capture on egress */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&egressPortInterface,
            TGF_CAPTURE_MODE_MIRRORING_E,
            GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "Failure in tgftrafficgeneratorporttxethcaptureset port - %d",
            egressPortInterface.devPort.portNum);

    rc = tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet(&egressPortInterface,
            TGF_PACKET_TYPE_CAPTURE_E,
            getFirst, GT_TRUE, packetBuf,
            &packetLen, &packetActualLength,
            &dev, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC((dropExpected)?GT_NO_MORE:GT_OK, rc,
            "Error: tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet");
    if((getFirst == GT_TRUE) && (dropExpected != GT_TRUE))
    {
        getFirst = GT_FALSE;
    }
}

/**
* @internal prvTgfSgtPhaThreadTypeGet function
* @endinternal
*
* @brief  PHA thread Type to get (from Thread ID
*/
static CPSS_DXCH_PHA_THREAD_TYPE_ENT prvTgfSgtPhaThreadTypeGet
(
    IN GT_U32                           threadId
)
{
    CPSS_DXCH_PHA_THREAD_TYPE_ENT   extType = CPSS_DXCH_PHA_THREAD_TYPE___LAST___E;
    switch(threadId)
    {
        case THR1_SGT_NetAddMSB:
            extType = CPSS_DXCH_PHA_THREAD_TYPE_SGT_NETWORK_ADD_MSB_E;
            break;
        case THR2_SGT_NetFix:
            extType = CPSS_DXCH_PHA_THREAD_TYPE_SGT_NETWORK_FIX_E;
            break;
        case THR3_SGT_NetRemove:
            extType = CPSS_DXCH_PHA_THREAD_TYPE_SGT_NETWORK_REMOVE_E;
            break;
        case THR4_SGT_eDSAFix:
            extType = CPSS_DXCH_PHA_THREAD_TYPE_SGT_EDSA_FIX_E;
            break;
        case THR5_SGT_eDSARemove:
            extType = CPSS_DXCH_PHA_THREAD_TYPE_SGT_EDSA_REMOVE_E;
            break;
        case THR6_SGT_GBPFixIPv4:
            extType = CPSS_DXCH_PHA_THREAD_TYPE_SGT_GBP_FIX_IPV4_E;
            break;
        case THR7_SGT_GBPFixIPv6:
            extType = CPSS_DXCH_PHA_THREAD_TYPE_SGT_GBP_FIX_IPV6_E;
            break;
        case THR8_SGT_GBPRemoveIPv4:
            extType = CPSS_DXCH_PHA_THREAD_TYPE_SGT_GBP_REMOVE_IPV4_E;
            break;
        case THR9_SGT_GBPRemoveIPv6:
            extType = CPSS_DXCH_PHA_THREAD_TYPE_SGT_GBP_REMOVE_IPV6_E;
            break;
    }
    return extType;
}

/* Debug Flag */
static GT_BOOL     prvTgfSgtSkipPha = GT_FALSE;
GT_BOOL prvTgfSgtSkipPhaSet
(
    IN GT_BOOL newSkipPha
)
{
    GT_BOOL oldSkipPha  = prvTgfSgtSkipPha;
    prvTgfSgtSkipPha    = newSkipPha;
    return oldSkipPha;
}

/**
* @internal prvTgfSgtPhaConfig function
* @endinternal
*
* @brief  PHA config To set:
*/
static GT_VOID prvTgfSgtPhaConfig
(
    IN GT_PORT_NUM                      portNum,
    IN GT_U32                           threadId
)
{
    GT_STATUS                               rc = GT_OK;
    CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC    commonInfo;
    CPSS_DXCH_PHA_THREAD_INFO_UNT           extInfo;
    CPSS_DXCH_PHA_THREAD_TYPE_ENT           extType;
    GT_U32                                  i;

    /* DEBUG Code */
    if(prvTgfSgtSkipPha)
    {
        return;
    }

    if(PRV_CPSS_SIP_6_30_CHECK_MAC(prvTgfDevNum))
    {
        switch(threadId)
        {
            case THR1_SGT_NetAddMSB:
                /* Add SGT TPID to global table */
                prvTgfSgtTpidConfig(GT_TRUE, CPSS_DIRECTION_EGRESS_E, 1, portNum);
                break;
            case THR3_SGT_NetRemove:
                prvTgfSgtTpidConfig(GT_FALSE, CPSS_DIRECTION_EGRESS_E, 1, portNum);
                break;
            case THR7_SGT_GBPFixIPv6:
                profileData.templateDataSize = PRV_TGF_TUNNEL_START_TEMPLATE_DATA_SIZE_16B_E; /* 16 bytes */
                profileData.templateDataBitsCfg[7] = PRV_TGF_TUNNEL_START_TEMPLATE_DATA_CFG_CONST_1_E;
                /* Setting 11 & 12 byte for Tag1 */
                for(i=0; i<8; i++)
                {
                    profileData.templateDataBitsCfg[2*8+i] = PRV_TGF_TUNNEL_START_TEMPLATE_DATA_CFG_TAG1_1_E;
                    profileData.templateDataBitsCfg[3*8+i] = PRV_TGF_TUNNEL_START_TEMPLATE_DATA_CFG_TAG1_0_E;
                }
                rc = prvTgfTunnelStartGenProfileTableEntrySet(prvTgfDevNum,
                        PRV_TGF_TEMPLATE_PROFILE_INDEX_CNS,
                        &profileData);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfTunnelStartGenProfileTableEntrySet");
                break;
            case THR0_DisablePha:
                cpssOsMemSet(&profileData.templateDataBitsCfg, 0, PRV_TGF_TUNNEL_START_TEMPLATE_DATA_MAX_SIZE_CNS);
                rc = prvTgfTunnelStartGenProfileTableEntrySet(prvTgfDevNum,
                        PRV_TGF_TEMPLATE_PROFILE_INDEX_CNS,
                        &profileData);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfTunnelStartGenProfileTableEntrySet");
                break;
            default:
                break;
        }
        return;
    }

    if(threadId == THR0_DisablePha)
    {
        rc = cpssDxChPhaPortThreadIdSet(prvTgfDevNum,
                portNum,
                GT_FALSE,
                0 /* threadId */);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                "[TGF]: cpssDxChPhaPortThreadIdSet FAILED for thread ID = [%d]", threadId);
        return;
    }
    else
    {
        /*AUTODOC: Assign thread to the ePort */
        rc = cpssDxChPhaPortThreadIdSet(prvTgfDevNum,
                portNum,
                GT_TRUE,
                threadId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                "[TGF]: cpssDxChPhaPortThreadIdSet FAILED for thread ID = [%d]", threadId);
    }

    cpssOsMemSet(&extInfo,   0, sizeof(extInfo));     /* No template */
    cpssOsMemSet(&extInfo,   0, sizeof(extInfo));
    cpssOsMemSet(&commonInfo,0, sizeof(CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC));
    commonInfo.stallDropCode                      = CPSS_NET_FIRST_USER_DEFINED_E;
    commonInfo.statisticalProcessingFactor        = 0;
    commonInfo.busyStallMode                      = CPSS_DXCH_PHA_BUSY_STALL_MODE_ORDERED_BYPASS_E;

    if((threadId == THR1_SGT_NetAddMSB) || (threadId == THR2_SGT_NetFix))
    {
        extInfo.sgtNetwork.etherType    = prvTgfPacketSgtTagPart.etherType;
        extInfo.sgtNetwork.version      = prvTgfPacketSgtTagPart.version;
        extInfo.sgtNetwork.length       = prvTgfPacketSgtTagPart.length;
        extInfo.sgtNetwork.optionType   = prvTgfPacketSgtTagPart.optionType;
    }

    extType = prvTgfSgtPhaThreadTypeGet(threadId);
    UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(extType, CPSS_DXCH_PHA_THREAD_TYPE___LAST___E,
            "[TGF]: prvTgfSgtPhaThreadTypeGet FAILED");

    /* AUTODOC: Set the thread entry */
    rc = cpssDxChPhaThreadIdEntrySet(prvTgfDevNum,
                                     threadId,
                                     &commonInfo,
                                     extType,
                                     &extInfo);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"[TGF]: cpssDxChPhaThreadIdEntrySet FAILED for thread ID = [%d]", threadId);
}

static GT_U32       globalEpgNumber;

/**
* @internal prvTgfSgtFdbEntryAdd function
* @endinternal
*
* @brief   Add FDB entry.
*
* @param[in] vlanId                   - to be configured
* @param[in] portIdx                  - port index
* @param[in] macAddr                  - MAC address
*/
static GT_VOID prvTgfSgtFdbEntryAdd
(
    IN GT_U16           vlanId,
    IN GT_U8            portIdx,
    IN TGF_MAC_ADDR     macAddr
)
{
    GT_STATUS                       rc;
    PRV_TGF_BRG_MAC_ENTRY_STC       prvTgfMacEntry;
    /* Add MAC Entry */

    /* clear entry */
    cpssOsMemSet(&prvTgfMacEntry, 0, sizeof(prvTgfMacEntry));

    prvTgfMacEntry.key.entryType                    = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    prvTgfMacEntry.key.key.macVlan.vlanId           = vlanId;
    prvTgfMacEntry.dstInterface.type                = CPSS_INTERFACE_PORT_E;
    prvTgfMacEntry.dstInterface.devPort.hwDevNum    = prvTgfDevNum;
    prvTgfMacEntry.dstInterface.devPort.portNum     = prvTgfPortsArray[portIdx];
    prvTgfMacEntry.daRoute                          = GT_TRUE;
    prvTgfMacEntry.epgNumber                        = globalEpgNumber;

    cpssOsMemCpy(prvTgfMacEntry.key.key.macVlan.macAddr.arEther, macAddr, sizeof(TGF_MAC_ADDR));
    rc = prvTgfBrgFdbMacEntrySet(&prvTgfMacEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntrySet");
}

PRV_TGF_BRG_VLAN_EPORT_TAG_STATE_MODE_ENT   restoreTagStateMode[2];
PRV_TGF_BRG_VLAN_PORT_TAG_CMD_ENT           restoreTagcmd[2];
/**
* @internal prvTgfSgtBridgeRestore function
* @endinternal
*
* @brief   Common bridge level restore for SGT test cases
*/
static GT_VOID prvTgfSgtBridgeRestore()
{
    GT_STATUS       rc;
    GT_U32          i,j;

    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgFdbFlush");

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_2_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_2_CNS);

    /* Restore Tag state */
    for(j=0, i=PRV_TGF_SGT_RECEIVE_PORT_1_IDX_CNS;i<=PRV_TGF_SGT_RECEIVE_PORT_2_IDX_CNS;i++,j++)
    {
        rc = prvTgfBrgVlanEgressPortTagStateModeSet(
                prvTgfDevNum,
                prvTgfPortsArray[i],
                restoreTagStateMode[j]);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                "prvTgfBrgVlanEgressPortTagStateModeSet");

        rc = prvTgfBrgVlanEgressPortTagStateSet(prvTgfDevNum,
                prvTgfPortsArray[i],
                restoreTagcmd[j]);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                "prvTgfBrgVlanEgressPortTagStateSet");
    }
}

/**
* @internal prvTgfSgtBridgeConfig function
* @endinternal
*
* @brief   Common bridge level config for SGT test cases
*/
static GT_VOID prvTgfSgtBridgeConfig
(
    IN PRV_TGF_BRG_VLAN_PORT_TAG_CMD_ENT recv_1_tag_state,
    IN PRV_TGF_BRG_VLAN_PORT_TAG_CMD_ENT recv_2_tag_state,
    IN GT_U16                            recv_1_vlan,
    IN GT_U16                            recv_2_vlan
)
{
    GT_STATUS                   rc        = GT_OK;
    GT_U8               tagArray[]  = {1, 0, 1, 0};
    GT_U8                       i, j;

    /* AUTODOC: create VLAN 2 with (1,3 Tagged and 2,4 untagged) */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_2_CNS,
            prvTgfPortsArray, NULL, tagArray, 4);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfBrgDefVlanEntryWithPortsSet: %d", PRV_TGF_VLANID_2_CNS);

    for(j=0, i=PRV_TGF_SGT_RECEIVE_PORT_1_IDX_CNS;i<=PRV_TGF_SGT_RECEIVE_PORT_2_IDX_CNS;i++,j++)
    {
        rc = prvTgfBrgVlanEgressPortTagStateModeGet(
                prvTgfDevNum,
                prvTgfPortsArray[i],
                &restoreTagStateMode[j]);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                "prvTgfBrgVlanEgressPortTagStateModeGet");

        rc = prvTgfBrgVlanEgressPortTagStateModeSet(
                prvTgfDevNum,
                prvTgfPortsArray[i],
                PRV_TGF_BRG_VLAN_EPORT_TAG_STATE_EPORT_MODE_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                "prvTgfBrgVlanEgressPortTagStateModeSet");

        rc = prvTgfBrgVlanEgressPortTagStateGet(prvTgfDevNum,
                prvTgfPortsArray[i],
                &restoreTagcmd[j]);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                "prvTgfBrgVlanEgressPortTagStateGet");

        rc = prvTgfBrgVlanEgressPortTagStateSet(prvTgfDevNum,
                prvTgfPortsArray[i],
                (i==PRV_TGF_SGT_RECEIVE_PORT_1_IDX_CNS)?recv_1_tag_state:recv_2_tag_state);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                "prvTgfBrgVlanEgressPortTagStateSet");
    }

    /* FDB Entry to bridge the packet to egress port_1 */
    prvTgfSgtFdbEntryAdd(recv_1_vlan,
            PRV_TGF_SGT_RECEIVE_PORT_1_IDX_CNS,
            PORT_2_MAC);

    /* FDB Entry to bridge the packet to egress port_2 */
    prvTgfSgtFdbEntryAdd(recv_2_vlan,
            PRV_TGF_SGT_RECEIVE_PORT_2_IDX_CNS,
            PORT_4_MAC);
}

/**
* @internal prvTgfSgtVerification function
* @endinternal
*
* @brief  SGT Test cases for user port - config
*  sgtType         - Indicates SGT tag verification type
*  inputOffset     - if sgtType == TUNNEL/NO_TUNNEL
*                       indicaes vxlan starting offset inside packet
*                    else
*                       indicates starting size of timestamp tag
*
*
*/
static GT_VOID prvTgfSgtVerification
(
    IN PRV_TGF_SGT_VERIFY_ENT    sgtType,
    IN GT_U32                    inputOffset
)
{
    GT_STATUS                           rc = GT_OK;
    GT_U32                              offset;
    GT_U8                               loop;
    GT_BOOL                             sgtTaggedInTunnel;
    GT_U8                               expNumOfVlanTagsBeforeSGT;

    /* Tunneled Packet */
    if((sgtType == PRV_TGF_SGT_VERIFY_SGT_IN_TUNNEL_E) ||
        (sgtType == PRV_TGF_SGT_VERIFY_NO_SGT_IN_TUNNEL_E))
    {
        offset = inputOffset;
        sgtTaggedInTunnel = ((packetBuf[offset] >> 7) == 1) ? GT_TRUE : GT_FALSE;
        offset += 2;
        rc = cpssOsMemCmp(&packetBuf[offset], &expectedSgtTag[6], 2) == 0 ? GT_OK: GT_FAIL;
        if(sgtType == PRV_TGF_SGT_VERIFY_SGT_IN_TUNNEL_E)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, sgtTaggedInTunnel,
                    "Tunnled Packet's SGT tagging state is unexpected, packet Data [0x%x]",
                    packetBuf[offset]);
            /* SGT expected inside tunnel */
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                    "Tunnled Packet's SGT TAG not matching with expected, packet Data [0x%x 0x%x]",
                    packetBuf[offset], packetBuf[offset+1]);
        }
        else
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_FALSE, sgtTaggedInTunnel,
                    "Tunnled Packet's SGT tagging state is unexpected, packet Data [0x%x]",
                    packetBuf[offset]);
            /* SGT Not expected inside tunnel */
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_FAIL, rc,
                    "Tunnled Packet's SGT TAG unexpected, packet Data [%x %x]",
                    packetBuf[offset], packetBuf[offset+1]);
        }
        return;
    }
    {
        /* Move the offset to check vlan tags details */
        offset = TGF_L2_HEADER_SIZE_CNS + inputOffset;
    }

    switch(sgtType)
    {
        case PRV_TGF_SGT_VERIFY_SGT_WITH_TAG_0_E:
        case PRV_TGF_SGT_VERIFY_NO_SGT_WITH_TAG0_E:
            expNumOfVlanTagsBeforeSGT = 1;
            break;
        case PRV_TGF_SGT_VERIFY_SGT_WITH_TAG_0_1_E:
            expNumOfVlanTagsBeforeSGT = 2;
            break;
        default: /* PRV_TGF_SGT_VERIFY_SGT_E */
            expNumOfVlanTagsBeforeSGT = 0;
            break;
    }

    for(loop = 0; loop < 4; loop++, offset+=TGF_VLAN_TAG_SIZE_CNS)
    {
        rc = cpssOsMemCmp(&packetBuf[offset], expectedSgtTag, 2) == 0 ?
            GT_OK : GT_FAIL;
        if(rc == GT_OK)
        {
            break;
        }

        if(loop < expNumOfVlanTagsBeforeSGT)
        {
            /* Compare vlan etherType - should match */
            UTF_VERIFY_EQUAL0_STRING_MAC(TGF_ETHERTYPE_8100_VLAN_TAG_CNS,
                    ((packetBuf[offset] << 8) | packetBuf[offset+1]),
                    "Packet TAG ethertype did not match");
        }
        else
        {
            /* Compare vlan etherType - should not match */
            UTF_VERIFY_NOT_EQUAL0_STRING_MAC(TGF_ETHERTYPE_8100_VLAN_TAG_CNS,
                    ((packetBuf[offset] << 8) | packetBuf[offset+1]),
                    "Packet TAG ethertype did not match");
        }
    }

    /* Verify SGT Tag expectation */
    if((sgtType == PRV_TGF_SGT_VERIFY_SGT_WITH_TAG_0_E) ||
            (sgtType == PRV_TGF_SGT_VERIFY_SGT_WITH_TAG_0_1_E) ||
            (sgtType == PRV_TGF_SGT_VERIFY_SGT_E))
    {
        rc = cpssOsMemCmp(&packetBuf[offset], expectedSgtTag, sizeof(expectedSgtTag)) == 0 ?
            GT_OK: GT_FAIL;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                "Packet not matching with expected, Offset = [%d]",
                offset);
    }
    else
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(rc, GT_FAIL, "SGT Not expected, But found");
    }
}

/***************************************************************/
/******** Test APIs (Config, TrafficAndVerify, Restore) ********/
/***************************************************************/
/**
* @internal prvTgfSgtTtiConfig function
* @endinternal
*
* @brief  TTI config To set:
*/
static GT_VOID prvTgfSgtTtiConfig
(
    IN GT_PORT_NUM                      portNum,
    IN GT_BOOL                          isIpv6Tunnel
)
{
    GT_STATUS                               rc = GT_OK;
    GT_U8                                   i;

    /* AUTODOC: Save ePort attributes configuration */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoGet(prvTgfDevNum,
                                                       portNum,
                                                      &(prvTgfSgtRestoreCfg.egressInfo));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgEportToPhysicalPortEgressPortInfoGet");

    /* AUTODOC: Set target port attributes configuration */
    /* AUTODOC: Tunnel Start = ENABLE, tunnelStartPtr = 8, tsPassenger = ETHERNET */
    cpssOsMemSet(&egressInfo,0,sizeof(egressInfo));
    egressInfo = prvTgfSgtRestoreCfg.egressInfo;
    egressInfo.tunnelStart            = GT_TRUE;
    egressInfo.tunnelStartPtr         = PRV_TGF_TUNNEL_START_INDEX_CNS;
    egressInfo.tsPassengerPacketType  = PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E;
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                                       portNum,
                                                       &egressInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgEportToPhysicalPortEgressPortInfoSet");

    /* AUTODOC: configure tunnel start profile for VXLAN-GPE */
    profileData.templateDataSize = PRV_TGF_TUNNEL_START_TEMPLATE_DATA_SIZE_8B_E; /* 8 bytes */
    rc = prvTgfTunnelStartGenProfileTableEntrySet(prvTgfDevNum,
                                                  PRV_TGF_TEMPLATE_PROFILE_INDEX_CNS,
                                                  &profileData);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfTunnelStartGenProfileTableEntrySet");

    if(isIpv6Tunnel)
    {
        /* AUTODOC: Set Tunnel Start entry 8 with: */
        /* AUTODOC: Type=CPSS_TUNNEL_GENERIC_IPV6_E, tagEnable=TRUE, vlanId=2, ipHeaderProtocol=UDP */
        /* AUTODOC: udpSrcPort = 0x2000, udpDstPort=BCEF, */
        cpssOsMemSet(&tunnelEntry, 0, sizeof(tunnelEntry));
        /* Set Tunnel IPv6 addresses */
        for (i=0; i<8; i++)
        {
            tunnelEntry.ipv6Cfg.destIp.arIP[i*2]     = (GT_U8)(prvTgfTunnelStartDstIPv6[i] >> 8);
            tunnelEntry.ipv6Cfg.destIp.arIP[i*2 + 1] = (GT_U8) prvTgfTunnelStartDstIPv6[i];
            tunnelEntry.ipv6Cfg.srcIp.arIP[i*2]      = (GT_U8)(prvTgfTunnelStartSrcIPv6[i] >> 8);
            tunnelEntry.ipv6Cfg.srcIp.arIP[i*2 + 1]  = (GT_U8) prvTgfTunnelStartSrcIPv6[i];
        }

        tunnelEntry.ipv6Cfg.tagEnable        = GT_FALSE;
        tunnelEntry.ipv6Cfg.vlanId           = PRV_TGF_VLANID_2_CNS;
        tunnelEntry.ipv6Cfg.ttl              = 20;
        tunnelEntry.ipv6Cfg.ipHeaderProtocol = PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E;
        tunnelEntry.ipv6Cfg.profileIndex     = PRV_TGF_TEMPLATE_PROFILE_INDEX_CNS;
        tunnelEntry.ipv6Cfg.udpSrcPort       = 0x2000;
        tunnelEntry.ipv6Cfg.udpDstPort       = 0xBCEF;
        cpssOsMemCpy(tunnelEntry.ipv6Cfg.macDa.arEther, prvTgfPacketTunnelL2Part.daMac, sizeof(TGF_MAC_ADDR));

        /* Restore */
        rc = prvTgfTunnelStartEntryGet(prvTgfDevNum,
                PRV_TGF_TUNNEL_START_INDEX_CNS,
                &prvTgfSgtRestoreCfg.tunnelType,
                &prvTgfSgtRestoreCfg.tunnelEntry);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfTunnelStartEntryGet");

        rc = prvTgfTunnelStartEntrySet(PRV_TGF_TUNNEL_START_INDEX_CNS,
                CPSS_TUNNEL_GENERIC_IPV6_E,
                &tunnelEntry);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfTunnelStartEntrySet");
    }
    else
    {
        cpssOsMemCpy(tunnelEntry.ipv4Cfg.macDa.arEther, prvTgfPacketTunnelL2Part.daMac, sizeof(TGF_MAC_ADDR));
        cpssOsMemCpy(tunnelEntry.ipv4Cfg.destIp.arIP, prvTgfTunnelDstIp, sizeof(TGF_IPV4_ADDR));
        cpssOsMemCpy(tunnelEntry.ipv4Cfg.srcIp.arIP, prvTgfTunnelSrcIp, sizeof(TGF_IPV4_ADDR));

        tunnelEntry.ipv4Cfg.tagEnable        = GT_FALSE;
        tunnelEntry.ipv4Cfg.vlanId           = PRV_TGF_VLANID_2_CNS;
        tunnelEntry.ipv4Cfg.ttl              = 20;
        tunnelEntry.ipv4Cfg.ipHeaderProtocol = PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E;
        tunnelEntry.ipv4Cfg.profileIndex     = PRV_TGF_TEMPLATE_PROFILE_INDEX_CNS;
        tunnelEntry.ipv4Cfg.udpSrcPort       = 0x2000;
        tunnelEntry.ipv4Cfg.udpDstPort       = 0xBCEF;
        rc = prvTgfTunnelStartEntrySet(PRV_TGF_TUNNEL_START_INDEX_CNS, CPSS_TUNNEL_GENERIC_IPV4_E, &tunnelEntry);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfTunnelStartEntrySet");
    }
}

/**
* @internal prvTgfSgtUserIpclConfig function
* @endinternal
*
* @brief  IPCL config To set:
*              SourceID[0]         = 1
*              tag1 (VID, UP, CFI) = 0xF1A8
*/
static GT_VOID prvTgfSgtUserIpclConfig(GT_VOID)
{
    GT_STATUS                           rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT         mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT         pattern;
    PRV_TGF_PCL_ACTION_STC              action;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT    ruleFormat;
    GT_U8                               cfi1;
    GT_U32                              i;

    PRV_UTF_LOG0_MAC("======= Setting IPCL Configuration =======\n");

    ruleFormat = PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E;
    /* AUTODOC: init PCL Engine for Ingress PCL for ingress port */
    rc = prvTgfPclDefPortInit(
            prvTgfPortsArray[PRV_TGF_SGT_SEND_PORT_2_IDX_CNS],
            CPSS_PCL_DIRECTION_INGRESS_E,
            CPSS_PCL_LOOKUP_0_E,
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E /*nonIpKey*/,
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E /*ipv4Key*/,
            ruleFormat  /*ipv6Key*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclDefPortInit");

    /************************* mask & pattern (first rule) **********************************/
    /* Match SIP */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    for(i=0; i<6; i++)
    {
        pattern.ruleExtIpv6L2.macSa.arEther[i] = sa_mac[0][i];
        mask.ruleExtIpv6L2.macSa.arEther[i] = 0xFF;
    }
    /*********************** PCL action **************************************/
    /* Redirect packet to target port, bypass bridge */
    cpssOsMemSet(&action, 0, sizeof(action));
    action.pktCmd                                   = CPSS_PACKET_CMD_FORWARD_E;
    action.redirect.redirectCmd                     = PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
    action.redirect.data.outIf.outInterface.type    = CPSS_INTERFACE_PORT_E;
    action.redirect.data.outIf.outInterface.devPort.hwDevNum = prvTgfDevNum;
    action.redirect.data.outIf.outInterface.devPort.portNum  = prvTgfPortsArray[PRV_TGF_SGT_RECEIVE_PORT_1_IDX_CNS];

    /* Source ID assignment */
    action.sourceId.assignSourceId = GT_TRUE;
    action.sourceId.sourceIdValue  = PRV_TGF_SOURCE_ID_CNS;

    /* Tag1 - vid Update */
    action.vlan.ingressVlanId1Cmd  = PRV_TGF_PCL_ACTION_INGRESS_VLAN_ID1_CMD_ALL_E;
    action.vlan.vlanId1            = U32_GET_FIELD_MAC(prvTgfPacketSgtTagPart.sgtValue, 0, 12);

    /* Tag1 - cfi Update */
    cfi1                           = U32_GET_FIELD_MAC(prvTgfPacketSgtTagPart.sgtValue, 12, 1);
    action.vlan.cfi1               = (cfi1 == 1)?CPSS_DXCH_PCL_ACTION_VLAN_INGRESS_CFI_SET_1_E:CPSS_DXCH_PCL_ACTION_VLAN_INGRESS_CFI_SET_0_E;

    /* Tag1 - up1 Update */
    action.qos.ingressUp1Cmd       = CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;
    action.qos.up1                 = U32_GET_FIELD_MAC(prvTgfPacketSgtTagPart.sgtValue, 13, 3);

    rc = prvTgfPclRuleSet(ruleFormat, PRV_TGF_IPCL_0_ID, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclRuleSet, prvTgfDevNum:%d,"
            "ruleFormat:%d, ruleIndex:%d", prvTgfDevNum, ruleFormat, PRV_TGF_IPCL_0_ID);
}

/**
* @internal prvTgfSgtUserPort_test_config function
* @endinternal
*
* @brief  SGT Test cases for user port - config
*/
static GT_VOID prvTgfSgtUserPort_test_config(GT_VOID)
{
    /* Common bridge related config */
    prvTgfSgtBridgeConfig(PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E,
            PRV_TGF_BRG_VLAN_PORT_TAG1_CMD_E,
            PRV_TGF_VLANID_2_CNS,
            PRV_TGF_DEF_VLANID_CNS);

    /* PCL UDB Config */
    prvTgfSgtPclUdbConfig();

    /* IPCL0 - To set tag1 and sourceID[0] */
    prvTgfSgtUserIpclConfig();

    /* TTI Config - 2nd port to add vxlan tunnel */
    prvTgfSgtTtiConfig(prvTgfPortsArray[PRV_TGF_SGT_RECEIVE_PORT_2_IDX_CNS], GT_TRUE/* is Tunnel IPV6 */);

    /* FDB Entry to bridge the packet to egress port_1 */
    prvTgfSgtFdbEntryAdd(PRV_TGF_DEF_VLANID_CNS,
            PRV_TGF_SGT_RECEIVE_PORT_1_IDX_CNS,
            PORT_2_MAC);
}

/**
* @internal prvTgfSgtUserPort_test_trafficAndVerify function
* @endinternal
*
* @brief  SGT Test cases for user port - traffic and verify
*/
static GT_VOID prvTgfSgtUserPort_test_trafficAndVerify
(
    IN GT_U8        timStampTagSize
)
{
    GT_STATUS                   rc;
    GT_U8                       portIter;
    GT_PORT_NUM                 ingressPort, egressPort;

    /* AUTODOC: Reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");
    /*******************************************************************/
    /****** Case 1 : User Port(Untagged) -> Network Port(Tagged) *******/
    /*******************************************************************/
    cpssOsMemCpy(prvTgfPacketL2Part.daMac, PORT_2_MAC, sizeof(prvTgfPacketL2Part.daMac));
    cpssOsMemCpy(prvTgfPacketIpv6Part.srcAddr, pkt_sip_arr[0], sizeof(prvTgfPacketIpv6Part.srcAddr));
    cpssOsMemCpy(prvTgfPacketL2Part.saMac, sa_mac[0], sizeof(prvTgfPacketL2Part.saMac));
    ingressPort     = prvTgfPortsArray[PRV_TGF_SGT_SEND_PORT_2_IDX_CNS];
    egressPort      = prvTgfPortsArray[PRV_TGF_SGT_RECEIVE_PORT_1_IDX_CNS];
    getFirst        = GT_TRUE;

    prvTgfSgtPhaConfig(egressPort, THR1_SGT_NetAddMSB);
    prvTgfSgtSendAndCapturePkt(ingressPort, egressPort, &prvTgfPacketInfoUnTag);
    prvTgfSgtVerification(PRV_TGF_SGT_VERIFY_SGT_WITH_TAG_0_E, timStampTagSize);

    /*******************************************************************/
    /****** Case 2 : User Port(Untagged) -> Network Port(Tagged) *******/
    /*******************************************************************/
    prvTgfSgtPhaConfig(egressPort, THR3_SGT_NetRemove);
    prvTgfSgtSendAndCapturePkt(ingressPort, egressPort, &prvTgfPacketInfoUnTag);
    prvTgfSgtVerification(PRV_TGF_SGT_VERIFY_NO_SGT_WITH_TAG0_E, timStampTagSize);

    /*******************************************************************/
    /****** Case 3 : User Port(Untagged) -> Tunnel Port ****************/
    /*******************************************************************/
    egressPort      = prvTgfPortsArray[PRV_TGF_SGT_RECEIVE_PORT_2_IDX_CNS];
    getFirst        = GT_TRUE;
    cpssOsMemCpy(prvTgfPacketL2Part.daMac, PORT_4_MAC, sizeof(prvTgfPacketL2Part.daMac));
    cpssOsMemCpy(prvTgfPacketIpv6Part.srcAddr, pkt_sip_arr[1], sizeof(prvTgfPacketIpv6Part.srcAddr));

    prvTgfSgtPhaConfig(egressPort, THR7_SGT_GBPFixIPv6);
    prvTgfSgtSendAndCapturePkt(ingressPort, egressPort, &prvTgfPacketInfoUnTag);
    prvTgfSgtVerification(PRV_TGF_SGT_VERIFY_SGT_IN_TUNNEL_E, PRV_TGF_VXLAN_OFFSET_MAC(0)+timStampTagSize);

    /*******************************************************************/
    /********* Case 4 : User Port(Untagged) -> Tunnel Port****** *******/
    /*******************************************************************/
    prvTgfSgtPhaConfig(egressPort, THR0_DisablePha);
    prvTgfSgtSendAndCapturePkt(ingressPort, egressPort, &prvTgfPacketInfoUnTag);
    prvTgfSgtVerification(PRV_TGF_SGT_VERIFY_NO_SGT_IN_TUNNEL_E, PRV_TGF_VXLAN_OFFSET_MAC(0)+timStampTagSize);
}

/**
* @internal prvTgfSgtUserPort_test_restore function
* @endinternal
*
* @brief  SGT Test cases for user port - Restore
*/
static GT_VOID prvTgfSgtUserPort_test_restore(GT_VOID)
{
    GT_STATUS                                               rc;

    PRV_UTF_LOG0_MAC("======= Restore configurations =======\n");
    /* -------------------------------------------------------------------------
     * 1. Restore Bridge Configuration
     */
    /* AUTODOC: flush FDB include static entries */
    prvTgfSgtBridgeRestore();

    /* -------------------------------------------------------------------------
     * 2. Restore TTI Configuration
     */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_SGT_RECEIVE_PORT_2_IDX_CNS],
            &(prvTgfSgtRestoreCfg.egressInfo));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "Failure in prvTgfBrgEportToPhysicalPortEgressPortInfoSet");

    rc = prvTgfTunnelStartEntrySet(
            PRV_TGF_TUNNEL_START_INDEX_CNS,
            prvTgfSgtRestoreCfg.tunnelType,
            &prvTgfSgtRestoreCfg.tunnelEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfTunnelStartEntrySet");

    /* -------------------------------------------------------------------------
     * 3. Restore PHA Configuration
     */
    prvTgfSgtPhaConfig(prvTgfPortsArray[PRV_TGF_SGT_RECEIVE_PORT_1_IDX_CNS], THR0_DisablePha);
    prvTgfSgtPhaConfig(prvTgfPortsArray[PRV_TGF_SGT_RECEIVE_PORT_2_IDX_CNS], THR0_DisablePha);

    /* -------------------------------------------------------------------------
     * 4. Restore IPCL Configuration
     */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E, PRV_TGF_IPCL_0_ID, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet, index = %d", 0);

    /* -------------------------------------------------------------------------
     * 5. Restore base Configuration
     */
    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: Disable RX capture */
    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in tgfTrafficTableRxStartCapture");

    /* AUTODOC: reset hw counters */
    rc = prvTgfCommonAllBridgeCntrsReset();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfCommonAllCntrsReset");
}

/**
* @internal prvTgfSgtRoutingConfig function
* @endinternal
*
* @brief  Routing related config
*
*/
static GT_VOID prvTgfSgtRoutingConfig(GT_VOID)
{
    GT_STATUS                               rc;
    GT_U8                                   ii, dipIndex;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    GT_IPV6ADDR                             ipAddr;
    GT_ETHERADDR                            arpMacAddr;
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           routeEntriesArray[1];
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           *regularEntryPtr;

    /* AUTODOC: enable IPv6 Unicast Routing on sending port1 */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SGT_SEND_PORT_1_IDX_CNS, CPSS_IP_UNICAST_E,
                                    CPSS_IP_PROTOCOL_IPV6_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                    prvTgfDevNum, PRV_TGF_SGT_SEND_PORT_1_IDX_CNS);

    /* AUTODOC: enable IPv6 Unicast Routing on sending port2 */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SGT_SEND_PORT_2_IDX_CNS, CPSS_IP_UNICAST_E,
                                    CPSS_IP_PROTOCOL_IPV6_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                    prvTgfDevNum, PRV_TGF_SGT_SEND_PORT_2_IDX_CNS);

    /* AUTODOC: enable IPv6 Unicast Routing on Vlan 2 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_VLANID_2_CNS, CPSS_IP_UNICAST_E,
                                    CPSS_IP_PROTOCOL_IPV6_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                    prvTgfDevNum, PRV_TGF_VLANID_2_CNS);

    /* write ARP MAC address to the Router ARP Table */
    cpssOsMemCpy(arpMacAddr.arEther, prvTgfArpMac, sizeof(TGF_MAC_ADDR));
    rc = prvTgfIpRouterArpAddrWrite(arpIndex, &arpMacAddr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

    /* write a UC Route entry to the Route Table */
    cpssOsMemSet(routeEntriesArray, 0, sizeof(routeEntriesArray));
    regularEntryPtr = &routeEntriesArray[0];
    regularEntryPtr->cmd                        = CPSS_PACKET_CMD_ROUTE_E;
    regularEntryPtr->nextHopVlanId              = PRV_TGF_VLANID_2_CNS;
    regularEntryPtr->nextHopInterface.type      = CPSS_INTERFACE_PORT_E;
    regularEntryPtr->nextHopInterface.devPort.hwDevNum = prvTgfDevNum;
    regularEntryPtr->nextHopInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_SGT_RECEIVE_PORT_1_IDX_CNS];
    regularEntryPtr->nextHopARPPointer          = arpIndex;

    /* AUTODOC: add UC route entry with nexthop VLAN 2 and nexthop port 3 */
    rc = prvTgfIpUcRouteEntriesWrite(prvTgfRouteEntryBaseIndex, routeEntriesArray, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: %d", prvTgfDevNum);

    /* AUTODOC: add UC route entry with nexthop VLAN 2 and nexthop port 4 */
    regularEntryPtr->nextHopInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_SGT_RECEIVE_PORT_2_IDX_CNS];
    rc = prvTgfIpUcRouteEntriesWrite(prvTgfRouteEntryBaseIndex+1, routeEntriesArray, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: %d", prvTgfDevNum);

    /* Create a new Ipv6 prefix in a Virtual Router for the specified LPM DB */
    /* fill a nexthope info for the prefix */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex          = prvTgfRouteEntryBaseIndex;
    nextHopInfo.ipLttEntry.routeType                    = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

    for(dipIndex=0; dipIndex<2; dipIndex++)
    {
        /* fill a destination IPv6 address for the prefix */
        for (ii = 0; ii < 8; ii++)
        {
            ipAddr.arIP[ii * 2]     = (GT_U8)(pkt_dip_arr[dipIndex][ii] >> 8);
            ipAddr.arIP[ii * 2 + 1] = (GT_U8) pkt_dip_arr[dipIndex][ii];
        }
        /* call CPSS function */
        rc = prvTgfIpLpmIpv6UcPrefixAdd(prvTgfLpmDBId, prvUtfVrfId,
                ipAddr, 128, &nextHopInfo, GT_FALSE, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6UcPrefixAdd: %d", prvTgfDevNum);
        nextHopInfo.ipLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndex+1;
    }
}

/**
* @internal prvTgfSgtNetworkIpclConfig function
* @endinternal
*
* @brief  IPCL config To set:
*              copyReserveed
*              tag1 (VID, UP, CFI) = 0xF1A8
*/
static GT_VOID prvTgfSgtNetworkIpclConfig
(
    IN GT_PORT_NUM      ingressPort
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT         mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT         pattern;
    PRV_TGF_PCL_ACTION_STC              action;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT    ruleFormat;

    PRV_UTF_LOG0_MAC("======= Setting IPCL Configuration =======\n");

    /* AUTODOC: init PCL Engine for Ingress PCL for ingress port */
    rc = prvTgfPclDefPortInit(
            ingressPort,
            CPSS_PCL_DIRECTION_INGRESS_E,
            CPSS_PCL_LOOKUP_0_E,
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E /*nonIpKey*/,
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E /*ipv4Key*/,
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E  /*ipv6Key*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclDefPortInit");

    /************************* mask & pattern (first rule) **********************************/
    /* Match Tag1 (0xF1A8) */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    pattern.ruleExtIpv6L2.vid1  = U32_GET_FIELD_MAC(prvTgfPacketSgtTagPart.sgtValue, 0, 12);
    mask.ruleExtIpv6L2.vid1     = 0xFFF;
    pattern.ruleExtIpv6L2.up1   = U32_GET_FIELD_MAC(prvTgfPacketSgtTagPart.sgtValue, 13, 3);
    mask.ruleExtIpv6L2.up1      = 0x7;

    /*********************** PCL action **************************************/
    /* Redirect packet to target port, bypass bridge */
    cpssOsMemSet(&action, 0, sizeof(action));
    ruleFormat                                      = PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E;
    action.pktCmd                                   = CPSS_PACKET_CMD_FORWARD_E;

    /* Copy Reserved Assignment */
    action.copyReserved.assignEnable = GT_TRUE;
    action.copyReserved.copyReserved = PRV_CPSS_SIP_6_10_CHECK_MAC(prvTgfDevNum) ? prvTgfSgtSrcIndexArr[1] : prvTgfSgtSrcIndexArr[1]>>1;

    /* AUTODOC: Set copyReserved mask to 0x3F for ingress direction */
    rc = prvTgfPclCopyReservedMaskSet(prvTgfDevNum, CPSS_PCL_DIRECTION_INGRESS_E,
            CPSS_PCL_LOOKUP_0_E, COPY_RESERVED_MASK_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclCopyReservedMaskSet: 0X%x, 0x%x",
            CPSS_PCL_LOOKUP_0_E, COPY_RESERVED_MASK_CNS);

    rc = prvTgfPclRuleSet(ruleFormat, PRV_TGF_IPCL_0_ID, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclRuleSet, prvTgfDevNum:%d,"
            "ruleFormat:%d, ruleIndex:%d", prvTgfDevNum, ruleFormat, PRV_TGF_IPCL_0_ID);
}


/**
* @internal prvTgfSgtEpclConfig function
* @endinternal
*
* @brief   Enable/disable EPCL on the port and setup src-trg access to
*          EPCL configuration table for Non tunnel start packets
* @param[in] portNum                  - physical port number
* @param[in] enable                   - GT_TRUE to enable, GT_FALSE to disable EPCL
*
* @retval GT_OK                    - on success
*/
static GT_VOID prvTgfSgtEpclConfig
(
    IN GT_U32                               egressPortNum,
    IN GT_BOOL                              enable,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT           lookupNum
)
{
    GT_STATUS                         rc;
    PRV_TGF_PCL_RULE_FORMAT_UNT     mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT     pattern;
    PRV_TGF_PCL_ACTION_STC          action;
    PRV_TGF_PCL_LOOKUP_CFG_STC      lookupCfg;
    CPSS_INTERFACE_INFO_STC         interfaceInfo;
    GT_U32                          epclIndex;
    GT_U8                           i;
    CPSS_PACKET_CMD_ENT             actionArr[3] = {CPSS_PACKET_CMD_FORWARD_E, CPSS_PACKET_CMD_FORWARD_E, CPSS_PACKET_CMD_DROP_HARD_E};
    GT_U32                          pclId = PRV_TGF_EPCL_PCL_ID_CNS;

    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));
    lookupCfg.enableLookup           = GT_TRUE;
    lookupCfg.pclId                  = pclId;
    lookupCfg.groupKeyTypes.nonIpKey = PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E;
    lookupCfg.groupKeyTypes.ipv4Key  = PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E;
    lookupCfg.groupKeyTypes.ipv6Key  = PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_E;

    /* AUTODOC: init PCL Engine for send port*/
    /* AUTODOC:   egress direction, lookup_0 */
    /* AUTODOC:   nonIpKey EGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key EGRESS_EXT_IPV4_L4 */
    /* AUTODOC:   ipv6Key EGRESS_EXT_IPv6_L4 */
    rc = prvTgfPclDefPortInitExt2(
            egressPortNum,
            CPSS_PCL_DIRECTION_EGRESS_E,
            lookupNum,
            &lookupCfg);
    UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "prvTgfPclDefPortInitExt2: %d, %d", prvTgfDevNum,
            egressPortNum);

    cpssOsMemSet(&action, 0, sizeof(action));
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    mask.ruleEgrExtIpv6L4.common.pclId      = 0x3FF;
    pattern.ruleEgrExtIpv6L4.common.pclId   = pclId;

    /* AUTODOC: EPCL action */
    action.egressPolicy = GT_TRUE;
    interfaceInfo.type  = CPSS_INTERFACE_INDEX_E;

    /* -----------+----------+----------+-------------+----------+--------
     * Pkt Number | DstIndex | srcIndex | CFG Tbl     | ruleIndex| Action
     * -----------+----------+----------+-------------+----------+--------
     * Pkt_4      | 54       | 0        | 3456(0xD80) | 10 (60)  | Forward
     * Pkt_1_2    | 52       | 62       | 3390(0xD3E) | 11 (66)  | Forward(With UDP filter)
     * Pkt_3      | 49       | 62       | 3198(0xC7E) | 12 (72)  | Hard Drop
     * -----------+----------+----------+-------------+----------+--------*/
    for (i=0; i<3;i++)
    {
        action.pktCmd   = actionArr[i];
        rc = prvTgfPclRuleSet(
                PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_E,
                PRV_TGF_EPCL_RULE_INDEX(i),
                &mask, &pattern, &action);
        UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, rc, "prvTgfPclRuleSet:");

        epclIndex       = PRV_TGF_EPCL_SRC_TRG_INDEX(prvTgfSgtSrcIndexArr[i], prvTgfSgtDstIndexArr[i]);
        interfaceInfo.index = epclIndex;
        PRV_UTF_LOG3_MAC("interface index = 0x%x, rule inedx 0x%x, action 0x%x\n",
                interfaceInfo.index, epclIndex, action.pktCmd);
        rc = prvTgfPclCfgTblSet(&interfaceInfo, CPSS_PCL_DIRECTION_EGRESS_E,
                lookupNum, &lookupCfg);
        UTF_VERIFY_EQUAL0_STRING_MAC( GT_OK, rc, "prvTgfPclCfgTblSet");
        pattern.ruleEgrExtIpv6L4.common.pclId++;
        lookupCfg.pclId++;
    }

    /* AUTODOC: Enables egress policy on port per packet type*/
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
            egressPortNum,
            PRV_TGF_PCL_EGRESS_PKT_NON_TS_E,
            enable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclEgressPclPacketTypesSet");

    /* AUTODOC: Configure src trg access mode for EPCL lookup */
    rc = prvTgfPclPortLookupCfgTabAccessModeGet(egressPortNum,
                           CPSS_PCL_DIRECTION_EGRESS_E,
                           lookupNum,             /* Lookup Number */
                           0,                               /* SubLookup Number */
                           &prvTgfSgtRestoreCfg.cfgTabAccMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeGet");

    /* AUTODOC: Configure src trg access mode for EPCL lookup */
    rc = prvTgfPclPortLookupCfgTabAccessModeSet(egressPortNum,
                           CPSS_PCL_DIRECTION_EGRESS_E,
                           lookupNum,             /* Lookup Number */
                           0,                               /* SubLookup Number */
                           PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_SRC_TRG_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeSet");
}

/**
 * @internal prvTgfSgtNetworkPort_test_trafficAndVerify function
 * @endinternal
*
* @brief  SGT Test cases for Network port - traffic and verify
* SEND_1    - Network Port
* SEND_2    - Not used
* RECEIVE_1 - Vxlan Port, Egress un-tagged
* RECEIVE_2 - User port, Egress un-tagged
*/
static GT_VOID prvTgfSgtNetworkPort_test_trafficAndVerify(GT_VOID)
{
    GT_STATUS                           rc;
    GT_U8                               portIter;
    GT_PORT_NUM                         ingressPort, egressPort;
    CPSS_DXCH_BRG_FDB_EPG_CONFIG_STC    fdbEpgConfig;
    PRV_TGF_TTI_ACTION_STC              ttiAction;
    PRV_TGF_TTI_RULE_UNT                ttiPattern;
    PRV_TGF_TTI_RULE_UNT                ttiMask;

    /* AUTODOC: Reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /*******************************************************************/
    /****** Network Port -> User Port related test cases ***************/
    /*******************************************************************/
    /* Configured:
     * pkt      dstIndex    srcindex  Action
     * ======== ========    ========  =======
     * pkt 1    52,         62        Forward
     * pkt 2    52,         62        Forward(TODO - UDB based filter to drop)
     * pkt 3    49,         62        Drop
     * pkt 4    54,         0         Forward
     */
    cpssOsMemCpy(prvTgfPacketIpv6Part.dstAddr, pkt_dip_arr[1], sizeof(prvTgfPacketIpv6Part.dstAddr));
    cpssOsMemCpy(prvTgfPacketL2Part.daMac, PORT_4_MAC, sizeof(prvTgfPacketL2Part.daMac));
    getFirst        = GT_TRUE;
    ingressPort     = prvTgfPortsArray[PRV_TGF_SGT_SEND_PORT_1_IDX_CNS];
    egressPort      = prvTgfPortsArray[PRV_TGF_SGT_RECEIVE_PORT_2_IDX_CNS];

    /* Packet - 3 */
    if(PRV_CPSS_SIP_6_30_CHECK_MAC(prvTgfDevNum))
    {
        prvTgfTtiRuleIndex = prvTgfTtiTcamAbsoluteIndexWithinTheLookupHitGet(PRV_TTI_LOOKUP_0, 3);

        rc = cpssDxChBrgFdbEpgConfigGet(prvTgfDevNum, &fdbEpgConfigRestore);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgFdbEpgConfigGet");

        /* AUTODOC: Configgure FDB to assign dst EPG */
        cpssOsMemSet(&fdbEpgConfig, 0, sizeof(fdbEpgConfig));
        fdbEpgConfig.dstEpgAssignEnable = GT_TRUE;
        rc = cpssDxChBrgFdbEpgConfigSet(prvTgfDevNum, &fdbEpgConfig);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgFdbEpgConfigSet");

        globalEpgNumber     = prvTgfSgtDstIndexArr[2];
        /* AUTODOC: Common bridge related config */
        prvTgfSgtBridgeConfig(
                PRV_TGF_BRG_VLAN_PORT_TAG1_CMD_E,
                PRV_TGF_BRG_VLAN_PORT_TAG1_CMD_E,
                PRV_TGF_VLANID_2_CNS,
                PRV_TGF_VLANID_2_CNS);

        rc = cpssDxChTtiGenericActionModeSet(prvTgfDevNum,
                CPSS_PCL_LOOKUP_0_E,
                CPSS_DXCH_TTI_GENERIC_ACTION_MODE_SRC_EPG_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChTtiGenericActionModeSet");

        /* AUTODOC: enable the TTI lookup for PRV_TGF_TTI_KEY_ETH_E at the port 1 */
        rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SGT_SEND_PORT_1_IDX_CNS],
                PRV_TGF_TTI_KEY_ETH_E,
                GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

        /* AUTODOC: set TTI Rule */
        cpssOsMemSet(&ttiPattern, 0, sizeof(ttiPattern));
        ttiPattern.eth.common.vid   = PRV_TGF_VLANID_2_CNS;

        /* AUTODOC: set TTI mask */
        cpssOsMemSet(&ttiMask, 0, sizeof(ttiMask));
        ttiMask.eth.common.vid      = 0xFFF;

        cpssOsMemSet(&ttiAction, 0, sizeof(ttiAction));
        ttiAction.command                                   = CPSS_PACKET_CMD_FORWARD_E;
        ttiAction.redirectCommand                           = PRV_TGF_TTI_ASSIGN_GENERIC_ACTION_E;
        ttiAction.interfaceInfo.type                        = CPSS_INTERFACE_PORT_E;
        ttiAction.interfaceInfo.devPort.hwDevNum            = prvTgfDevNum;
        ttiAction.interfaceInfo.devPort.portNum             = prvTgfPortsArray[PRV_TGF_SGT_SEND_PORT_1_IDX_CNS],
        ttiAction.genericAction                             = prvTgfSgtSrcIndexArr[2];

        rc = prvTgfTtiRuleSet(prvTgfTtiRuleIndex, PRV_TGF_TTI_KEY_ETH_E,
                &ttiPattern, &ttiMask, &ttiAction);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet");
    }
    else
    {
        rc = prvTgfIpRouterSourceIdSet(CPSS_IP_UNICAST_E,
                (prvTgfSgtDstIndexArr[2] << PRV_TGF_SRC_ID_LSB_CNS),
                (0x3F << PRV_TGF_SRC_ID_LSB_CNS));
    }
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterSourceIdSet: %d", prvTgfDevNum);

    prvTgfPacketUdpPart.dstPort = 91;
    prvTgfSgtPhaConfig(egressPort, THR0_DisablePha);
    dropExpected = GT_TRUE;
    prvTgfSgtSendAndCapturePkt(ingressPort, egressPort, &prvTgfPacketInfoSgtAndVlanTag);
    dropExpected = GT_FALSE;

    if(PRV_CPSS_SIP_6_30_CHECK_MAC(prvTgfDevNum))
    {
        globalEpgNumber = prvTgfSgtDstIndexArr[1];
        prvTgfSgtFdbEntryAdd(PRV_TGF_VLANID_2_CNS,
                PRV_TGF_SGT_RECEIVE_PORT_2_IDX_CNS,
                PORT_4_MAC);
    }
    else
    {
        /* Packet - 1 */
        rc = prvTgfIpRouterSourceIdSet(CPSS_IP_UNICAST_E,
                (prvTgfSgtDstIndexArr[1] << PRV_TGF_SRC_ID_LSB_CNS),
                (0x3F << PRV_TGF_SRC_ID_LSB_CNS));
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterSourceIdSet: %d", prvTgfDevNum);
    }

    prvTgfPacketUdpPart.dstPort = 91;
    if(PRV_CPSS_SIP_6_30_CHECK_MAC(prvTgfDevNum))
    {
        rc = prvTgfBrgVlanEgressEtherTypeSet(CPSS_VLAN_ETHERTYPE1_E,
                0x8100);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEgressEtherTypeSet");

        rc = prvTgfBrgVlanPortEgressTpidSet(prvTgfDevNum, egressPort,
                CPSS_VLAN_ETHERTYPE1_E, 1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortEgressTpidSet");

        rc = cpssDxChBrgVlanTpidTagTypeSet(prvTgfDevNum, CPSS_DIRECTION_EGRESS_E,
                1, CPSS_BRG_TPID_SIZE_TYPE_4_BYTES_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanTpidTagTypeSet");
    }
    else
    {
        prvTgfSgtPhaConfig(egressPort, THR3_SGT_NetRemove);
    }
    prvTgfSgtSendAndCapturePkt(ingressPort, egressPort, &prvTgfPacketInfoSgtAndVlanTag);
    prvTgfSgtVerification(PRV_TGF_SGT_VERIFY_NO_SGT_WITH_TAG0_E, 0);

    /* Packet - 2 */
    prvTgfPacketUdpPart.dstPort = 92;
    prvTgfSgtSendAndCapturePkt(ingressPort, egressPort, &prvTgfPacketInfoSgtAndVlanTag);
    prvTgfSgtVerification(PRV_TGF_SGT_VERIFY_NO_SGT_WITH_TAG0_E, 0);

    if(PRV_CPSS_SIP_6_30_CHECK_MAC(prvTgfDevNum))
    {
        globalEpgNumber = prvTgfSgtDstIndexArr[0];
        prvTgfSgtFdbEntryAdd(PRV_TGF_VLANID_2_CNS,
                PRV_TGF_SGT_RECEIVE_PORT_2_IDX_CNS,
                PORT_4_MAC);
    }
    else
    {
        /* Packet - 4 */
        rc = prvTgfIpRouterSourceIdSet(CPSS_IP_UNICAST_E,
                (prvTgfSgtDstIndexArr[0] << PRV_TGF_SRC_ID_LSB_CNS),
                (0x3F << PRV_TGF_SRC_ID_LSB_CNS));
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterSourceIdSet: %d", prvTgfDevNum);
    }

    prvTgfPacketUdpPart.dstPort = 91;
    prvTgfSgtPhaConfig(egressPort, THR0_DisablePha);
    prvTgfSgtSendAndCapturePkt(ingressPort, egressPort, &prvTgfPacketInfoSingleTag);
    prvTgfSgtVerification(PRV_TGF_SGT_VERIFY_NO_SGT_WITH_TAG0_E, 0);

    if(!PRV_CPSS_SIP_6_30_CHECK_MAC(prvTgfDevNum))
    {
        /*******************************************************************/
        /****** Network Port -> Tunnel Port related test cases *************/
        /*******************************************************************/
        /* Packet - 1 */
        cpssOsMemCpy(prvTgfPacketIpv6Part.dstAddr, pkt_dip_arr[0], sizeof(prvTgfPacketIpv6Part.dstAddr));
        cpssOsMemCpy(prvTgfPacketL2Part.daMac, PORT_2_MAC, sizeof(prvTgfPacketL2Part.daMac));
        getFirst        = GT_TRUE;
        ingressPort     = prvTgfPortsArray[PRV_TGF_SGT_SEND_PORT_1_IDX_CNS];
        egressPort      = prvTgfPortsArray[PRV_TGF_SGT_RECEIVE_PORT_1_IDX_CNS];

        /* Packet - 1 */
        prvTgfPacketUdpPart.dstPort = 91;
        prvTgfSgtPhaConfig(egressPort, THR9_SGT_GBPRemoveIPv6);
        prvTgfSgtSendAndCapturePkt(ingressPort, egressPort, &prvTgfPacketInfoSgtAndVlanTag);
        prvTgfSgtVerification(PRV_TGF_SGT_VERIFY_SGT_IN_TUNNEL_E, PRV_TGF_VXLAN_OFFSET_MAC(0));

        /* Packet - 2 */
        prvTgfPacketUdpPart.dstPort = 92;
        prvTgfSgtSendAndCapturePkt(ingressPort, egressPort, &prvTgfPacketInfoSgtAndVlanTag);
        prvTgfSgtVerification(PRV_TGF_SGT_VERIFY_SGT_IN_TUNNEL_E, PRV_TGF_VXLAN_OFFSET_MAC(0));

        /* Packet - 3 */
        prvTgfPacketUdpPart.dstPort = 91;
        prvTgfSgtSendAndCapturePkt(ingressPort, egressPort, &prvTgfPacketInfoSgtAndVlanTag);
        prvTgfSgtVerification(PRV_TGF_SGT_VERIFY_SGT_IN_TUNNEL_E, PRV_TGF_VXLAN_OFFSET_MAC(0));

        /* Packet - 4 */
        prvTgfPacketUdpPart.dstPort = 91;
        prvTgfSgtPhaConfig(egressPort, THR0_DisablePha);
        prvTgfSgtSendAndCapturePkt(ingressPort, egressPort, &prvTgfPacketInfoSingleTag);
        prvTgfSgtVerification(PRV_TGF_SGT_VERIFY_NO_SGT_IN_TUNNEL_E, PRV_TGF_VXLAN_OFFSET_MAC(0));
    }
}

/**
* @internal prvTgfSgtNetworkPort_test_config function
* @endinternal
*
* @brief  SGT Test cases for network port - config
*/
static GT_VOID prvTgfSgtNetworkPort_test_config(GT_VOID)
{
    if(!PRV_CPSS_SIP_6_30_CHECK_MAC(prvTgfDevNum))
    {
        /* Common bridge related config */
        prvTgfSgtBridgeConfig(PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E,
                PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E,
                PRV_TGF_VLANID_2_CNS,
                PRV_TGF_DEF_VLANID_CNS);

        /* FDB Entry to bridge the packet to egress port_2 */
        prvTgfSgtFdbEntryAdd(PRV_TGF_VLANID_2_CNS,
                PRV_TGF_SGT_RECEIVE_PORT_2_IDX_CNS,
                PORT_4_MAC);
    }

    /* Add SGT TPID to global table */
    prvTgfSgtTpidConfig(GT_TRUE, CPSS_DIRECTION_INGRESS_E, 1, 0/*egressPort*/);

    /* IPCL0 - To set tag1 and copy reserved (SGT Src Index)*/
    prvTgfSgtNetworkIpclConfig(prvTgfPortsArray[PRV_TGF_SGT_SEND_PORT_1_IDX_CNS]);

    /* Routing config - to assign source ID (SGT Dst Index)*/
    prvTgfSgtRoutingConfig();

    /* PCL UDB Config */
    prvTgfSgtPclUdbConfig();

    /* TTI Config - 1st port to add vxlan tunnel */
    prvTgfSgtTtiConfig(prvTgfPortsArray[PRV_TGF_SGT_RECEIVE_PORT_1_IDX_CNS], GT_TRUE/* is Tunnel IPV6 */);

    /* Epcl Config - Only for user port */
    prvTgfSgtEpclConfig(prvTgfPortsArray[PRV_TGF_SGT_RECEIVE_PORT_2_IDX_CNS], GT_TRUE, CPSS_PCL_LOOKUP_0_E);

}

/**
* @internal prvTgfSgtNetworkPort_test_restore function
* @endinternal
*
* @brief  SGT Test cases for Network port - Restore
*/
static GT_VOID prvTgfSgtNetworkPort_test_restore(GT_VOID)
{
    GT_STATUS                       rc;
    GT_U32                          i, dipIndex;
    GT_IPV6ADDR                     ipAddr;
    PRV_TGF_PCL_LOOKUP_CFG_STC      lookupCfg;
    CPSS_INTERFACE_INFO_STC         interfaceInfo;

    PRV_UTF_LOG0_MAC("======= Restore configurations =======\n");
    /* -------------------------------------------------------------------------
     * 1. Restore Bridge Configuration
     */
    /* AUTODOC: flush FDB include static entries */
    prvTgfSgtBridgeRestore();

    /* -------------------------------------------------------------------------
     * 2. Restore Tpid Configuration
     */
    prvTgfSgtTpidConfig(GT_FALSE, CPSS_DIRECTION_INGRESS_E, 1, 0/*egressPort*/);

    /* -------------------------------------------------------------------------
     * 3. Restore IPCL Configuration
     */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_60_BYTES_E, PRV_TGF_IPCL_0_ID, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfPclRuleValidStatusSet, index = %d", 0);

    /* -------------------------------------------------------------------------
     * 4. Restore TTI Configuration
     */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_SGT_RECEIVE_PORT_1_IDX_CNS],
            &(prvTgfSgtRestoreCfg.egressInfo));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "Failure in prvTgfBrgEportToPhysicalPortEgressPortInfoSet");

    rc = prvTgfTunnelStartEntrySet(
            PRV_TGF_TUNNEL_START_INDEX_CNS,
            prvTgfSgtRestoreCfg.tunnelType,
            &prvTgfSgtRestoreCfg.tunnelEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfTunnelStartEntrySet");

    /* -------------------------------------------------------------------------
     * 5. Restore Routing Configuration
     */
    /* AUTODOC: disable IPv6 Unicast Routing on sending port1 */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SGT_SEND_PORT_1_IDX_CNS, CPSS_IP_UNICAST_E,
                                    CPSS_IP_PROTOCOL_IPV6_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                    prvTgfDevNum, PRV_TGF_SGT_SEND_PORT_1_IDX_CNS);

    /* AUTODOC: disable IPv6 Unicast Routing on sending port2 */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SGT_SEND_PORT_2_IDX_CNS, CPSS_IP_UNICAST_E,
                                    CPSS_IP_PROTOCOL_IPV6_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                    prvTgfDevNum, PRV_TGF_SGT_SEND_PORT_2_IDX_CNS);

    /* AUTODOC: disable IPv6 Unicast Routing on Default Vlan */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_VLANID_2_CNS, CPSS_IP_UNICAST_E,
                                    CPSS_IP_PROTOCOL_IPV6_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                    prvTgfDevNum, PRV_TGF_VLANID_2_CNS);

    for(dipIndex=0; dipIndex<2; dipIndex++)
    {
        /* fill a destination IPv6 address for the prefix */
        for (i = 0; i < 8; i++)
        {
            ipAddr.arIP[i * 2]     = (GT_U8)(pkt_dip_arr[dipIndex][i] >> 8);
            ipAddr.arIP[i * 2 + 1] = (GT_U8) pkt_dip_arr[dipIndex][i];
        }
        /* call CPSS function */
        rc = prvTgfIpLpmIpv6UcPrefixDel(prvTgfLpmDBId, prvUtfVrfId, ipAddr, 128);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6UcPrefixAdd: %d", prvTgfDevNum);
    }

    /* -------------------------------------------------------------------------
     * 6. Restore PHA Configuration
     */
    prvTgfSgtPhaConfig(prvTgfPortsArray[PRV_TGF_SGT_RECEIVE_PORT_1_IDX_CNS], THR0_DisablePha);
    prvTgfSgtPhaConfig(prvTgfPortsArray[PRV_TGF_SGT_RECEIVE_PORT_2_IDX_CNS], THR0_DisablePha);

    /* -------------------------------------------------------------------------
     * 7. Restore EPCL Configuration
     */
    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));
    lookupCfg.groupKeyTypes.nonIpKey = PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E;
    lookupCfg.groupKeyTypes.ipv4Key  = PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E;
    lookupCfg.groupKeyTypes.ipv6Key  = PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_E;

    interfaceInfo.type  = CPSS_INTERFACE_INDEX_E;

    for (i=0; i<3;i++)
    {
        rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_60_BYTES_E,
                PRV_TGF_EPCL_RULE_INDEX(i),
                GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet:");

        interfaceInfo.index = PRV_TGF_EPCL_SRC_TRG_INDEX(prvTgfSgtSrcIndexArr[i], prvTgfSgtDstIndexArr[i]);
        rc = prvTgfPclCfgTblSet(&interfaceInfo, CPSS_PCL_DIRECTION_EGRESS_E,
                CPSS_PCL_LOOKUP_0_E, &lookupCfg);
        UTF_VERIFY_EQUAL0_STRING_MAC( GT_OK, rc, "prvTgfPclCfgTblSet");
    }

    /* AUTODOC: Restore access mode for EPCL lookup */
    rc = prvTgfPclPortLookupCfgTabAccessModeSet(
            prvTgfPortsArray[PRV_TGF_SGT_RECEIVE_PORT_2_IDX_CNS],
            CPSS_PCL_DIRECTION_EGRESS_E,
            CPSS_PCL_LOOKUP_0_E,             /* Lookup Number */
            0,                               /* SubLookup Number */
            prvTgfSgtRestoreCfg.cfgTabAccMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeSet");

    /* -------------------------------------------------------------------------
     * 8. Restore base Configuration
     */
    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: Disable RX capture */
    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in tgfTrafficTableRxStartCapture");

    /* AUTODOC: reset hw counters */
    rc = prvTgfCommonAllBridgeCntrsReset();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfCommonAllCntrsReset");

    if(PRV_CPSS_SIP_6_30_CHECK_MAC(prvTgfDevNum))
    {
        rc = cpssDxChBrgFdbEpgConfigSet(prvTgfDevNum, &fdbEpgConfigRestore);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgFdbEpgConfigSet");
    }
}

/**
* @internal prvTgfSgtTunnelIpcl1Config function
* @endinternal
*
* @brief  IPCL1 config To set:
*              set source ID = sgtDstIndex
*/
static GT_VOID prvTgfSgtTunnelIpcl1Config
(
    IN GT_PORT_NUM      ingressPort,
    IN GT_U32           sgtDstIndex
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT         mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT         pattern;
    PRV_TGF_PCL_ACTION_STC              action;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT    ruleFormat = PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_60_E;

    /* AUTODOC: init PCL Engine for Ingress PCL for ingress port */
    rc = prvTgfPclDefPortInit(
            ingressPort,
            CPSS_PCL_DIRECTION_INGRESS_E,
            CPSS_PCL_LOOKUP_NUMBER_1_E,                    /* Sub Lookup 1, lookup 0 */
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E /* nonIpKey */,
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E /* ipv4Key */,
            ruleFormat  /*ipv6Key*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclDefPortInit");

    /************************* mask & pattern (first rule) **********************************/
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    mask.ruleIngrUdbOnly.udb60FixedFld.pclId      = 0x3FF;
    pattern.ruleIngrUdbOnly.udb60FixedFld.pclId   =
        PRV_TGF_PCL_DEFAULT_ID_MAC(CPSS_PCL_DIRECTION_INGRESS_E, CPSS_PCL_LOOKUP_NUMBER_1_E, ingressPort);

    /*********************** PCL action **************************************/
    /* Redirect packet to target port, bypass bridge */
    cpssOsMemSet(&action, 0, sizeof(action));
    action.pktCmd                                   = CPSS_PACKET_CMD_FORWARD_E;
    if(PRV_CPSS_SIP_6_30_CHECK_MAC(prvTgfDevNum))
    {
        action.redirect.redirectCmd = PRV_TGF_PCL_ACTION_REDIRECT_CMD_GENERIC_ACTION_E;
        action.redirect.data.genericActionInfo.genericAction = sgtDstIndex;

        /* Configure TTI to assign src EPG */
        rc = cpssDxChPclGenericActionModeSet(prvTgfDevNum,
                CPSS_PCL_LOOKUP_NUMBER_1_E,
                0, /* hitNum */
                CPSS_DXCH_PCL_GENERIC_ACTION_MODE_DST_EPG_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPclGenericActionModeSet");
    }
    else
    {
        action.redirect.redirectCmd                     = PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
        action.redirect.data.outIf.outInterface.type    = CPSS_INTERFACE_PORT_E;
        action.redirect.data.outIf.outInterface.devPort.hwDevNum = prvTgfDevNum;
        action.redirect.data.outIf.outInterface.devPort.portNum  = prvTgfPortsArray[PRV_TGF_SGT_RECEIVE_PORT_1_IDX_CNS];

        /* Source ID assignment */
        action.sourceId.assignSourceId = GT_TRUE;
        action.sourceId.sourceIdValue  = (sgtDstIndex << PRV_TGF_SRC_ID_LSB_CNS);
    }

    rc = prvTgfPclRuleSet(ruleFormat, PRV_TGF_IPCL_1_ID, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclRuleSet, prvTgfDevNum:%d,"
            "ruleFormat:%d, ruleIndex:%d", prvTgfDevNum, ruleFormat, PRV_TGF_IPCL_1_ID);
}

/**
* @internal prvTgfSgtTunnelIpclConfig function
* @endinternal
*
* @brief  IPCL config To set:
*              copyReserveed, set assign Tag1 from UDB
*              set source ID[0] = 1
*/
static GT_VOID prvTgfSgtTunnelIpclConfig
(
    IN GT_PORT_NUM      ingressPort,
    IN GT_U32           sgtSrcIndex
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT         mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT         pattern;
    PRV_TGF_PCL_ACTION_STC              action;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT    ruleFormat;
    GT_U32                              udbIndex = 48;
    GT_U32                              index;
    CPSS_DXCH_PCL_EPG_CONFIG_STC        epgConfig;

    ruleFormat = PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_60_E;

    /*<UDB 49> and <UDB 48> are assigned to the Group Policy ID
      SKip the UDP header to reach VXLAN header(8B UDP header), 5th and 6th Byte from vxlan header
      UDB 49 - MSB bits (15:8) of Group Policy ID
      UDB 48 - LSB bits (7:0) of Group Policy ID */
    for (index=0; index<2; index++)
    {
        rc = prvTgfPclUserDefinedByteSet(
                ruleFormat,
                PRV_TGF_PCL_PACKET_TYPE_IPV6_UDP_E,
                CPSS_PCL_DIRECTION_INGRESS_E,
                (udbIndex + (1 - index)), /* 49..48 */
                PRV_TGF_PCL_OFFSET_L4_E,
                (8 + 5 + index));
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                "prvTgfPclUserDefinedByteSet: %d", prvTgfDevNum);
    }

    PRV_UTF_LOG0_MAC("======= Setting IPCL_0 configuration =======\n");

    /* AUTODOC: init PCL Engine for Ingress PCL for ingress port */
    rc = prvTgfPclDefPortInit(
            ingressPort,
            CPSS_PCL_DIRECTION_INGRESS_E,
            CPSS_PCL_LOOKUP_NUMBER_0_E,
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E /*nonIpKey*/,
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E /*ipv4Key*/,
            ruleFormat  /*ipv6Key*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclDefPortInit");

    /************************* mask & pattern (first rule) **********************************/
    /* Match Group policy ID & tagb bit inside tunnel */
    cpssOsMemSet(&action, 0, sizeof(action));
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    /* TODO - Need to make pattern as 'G bit' and policy ID
    mask.ruleIngrUdbOnly.udb[48] = 0xFF;
    pattern.ruleIngrUdbOnly.udb[48] = 0xA8;
    mask.ruleIngrUdbOnly.udb[49] = 0xFF;
    pattern.ruleIngrUdbOnly.udb[49] = 0xF1;*/
    mask.ruleIngrUdbOnly.udb60FixedFld.pclId      = 0x3FF;
    pattern.ruleIngrUdbOnly.udb60FixedFld.pclId   =
        PRV_TGF_PCL_DEFAULT_ID_MAC(CPSS_PCL_DIRECTION_INGRESS_E, CPSS_PCL_LOOKUP_NUMBER_0_E, ingressPort);

    /* Source ID assignment */
    action.sourceId.assignSourceId          = GT_TRUE;
    action.sourceId.sourceIdValue           = PRV_TGF_SOURCE_ID_CNS;

    /* Copy Reserved Assignment */
    action.copyReserved.assignEnable        = GT_TRUE;
    action.copyReserved.copyReserved        = PRV_CPSS_SIP_6_10_CHECK_MAC(prvTgfDevNum) ? sgtSrcIndex : sgtSrcIndex>>1;

    /* Assign tag1 from UDB 48,49 */
    action.vlan.updateTag1FromUdb           = GT_TRUE;

    if(PRV_CPSS_SIP_6_30_CHECK_MAC(prvTgfDevNum))
    {
        epgConfig.assignFromUdbsMode = CPSS_DXCH_PCL_ASSIGN_FROM_UDB_MODE_TAG1_SRC_EPG_E;
        epgConfig.useSrcTrgMode      = CPSS_DXCH_PCL_USE_SRC_TRG_MODE_COPY_RESERVED_SRC_ID_E;
        rc = cpssDxChPclEpgConfigSet(prvTgfDevNum, CPSS_PCL_LOOKUP_NUMBER_0_E, &epgConfig);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclEpgConfigSet");

        /* UDB48,49 willbe copied to src Index */
        prvTgfPacketTunnelInputVxlanGpePart.vni = sgtSrcIndex;
    }

    /* AUTODOC: Set copyReserved mask to 0x3F for ingress direction */
    rc = prvTgfPclCopyReservedMaskSet(prvTgfDevNum, CPSS_PCL_DIRECTION_INGRESS_E,
            CPSS_PCL_LOOKUP_0_E, COPY_RESERVED_MASK_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclCopyReservedMaskSet: 0X%x, 0x%x",
            CPSS_PCL_LOOKUP_0_E, COPY_RESERVED_MASK_CNS);

    /*********************** PCL action **************************************/
    /* Redirect packet to target port */
    rc = prvTgfPclRuleSet(ruleFormat, PRV_TGF_IPCL_0_ID, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclRuleSet, prvTgfDevNum:%d,"
            "ruleFormat:%d, ruleIndex:%d", prvTgfDevNum, ruleFormat, PRV_TGF_IPCL_0_ID);
}

/**
 * @internal prvTgfSgtTunnelPort_test_trafficAndVerify function
 * @endinternal
*
* @brief  SGT Test cases for tunnel port - traffic and verify
* SEND_1    - Tunnel Port
* SEND_2    - Not used
* RECEIVE_1 - Network Port, Egress tagged
* RECEIVE_2 - User port, Egress un-tagged
*/
static GT_VOID prvTgfSgtTunnelPort_test_trafficAndVerify(GT_VOID)
{
    GT_STATUS                   rc;
    GT_U8                       portIter;
    GT_U16                      restoreVni;
    GT_PORT_NUM                 ingressPort, egressPort;

    /* AUTODOC: Reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");
    /*******************************************************************/
    /******** Tunnel Port -> Network Port related test cases ************/
    /*******************************************************************/
    cpssOsMemCpy(prvTgfPacketTunnelL2Part.saMac, sa_mac[0], sizeof(prvTgfPacketTunnelL2Part.saMac));
    cpssOsMemCpy(prvTgfPacketTunnelL2Part.daMac, PORT_2_MAC, sizeof(prvTgfPacketTunnelL2Part.daMac));
    ingressPort = prvTgfPortsArray[PRV_TGF_SGT_SEND_PORT_1_IDX_CNS];
    egressPort  = prvTgfPortsArray[PRV_TGF_SGT_RECEIVE_PORT_1_IDX_CNS];
    getFirst = GT_TRUE;

    /* Packet - 3 */
    /* IPCL_0_1 - To Set Source ID - 54/52/49 */
    prvTgfSgtTunnelIpcl1Config(ingressPort, prvTgfSgtDstIndexArr[2]);

    prvTgfPacketTunnelL2Part.saMac[5] += 1;
    dropExpected = GT_TRUE;
    prvTgfPacketTunnelInputUdpPart.dstPort = 191;
    prvTgfSgtPhaConfig(egressPort, THR0_DisablePha);
    prvTgfSgtSendAndCapturePkt(ingressPort, egressPort, &prvTgfPacketInfoIpv6WithVxlan);
    dropExpected = GT_FALSE;
    prvTgfPacketTunnelInputVxlanGpePart.vni = PRV_TGF_SGT_VALUE_CNS;

    /* Packet - 1 */
    /* IPCL_0_1 - To Set Source ID - 54/52/49 */
    prvTgfSgtTunnelIpcl1Config(ingressPort, prvTgfSgtDstIndexArr[1]);

    prvTgfPacketTunnelL2Part.saMac[5] += 1;
    prvTgfPacketTunnelInputUdpPart.dstPort = 191;
    prvTgfSgtPhaConfig(egressPort, THR1_SGT_NetAddMSB);
    prvTgfSgtSendAndCapturePkt(ingressPort, egressPort, &prvTgfPacketInfoIpv6WithVxlan);
    prvTgfSgtVerification(PRV_TGF_SGT_VERIFY_SGT_WITH_TAG_0_E, 0);

    /* Packet - 2 */
    prvTgfPacketTunnelL2Part.saMac[5] += 1;
    /* TODO - EPCL should drop in case of Udp = 192 */
    prvTgfPacketTunnelInputUdpPart.dstPort = 192;
    prvTgfSgtSendAndCapturePkt(ingressPort, egressPort, &prvTgfPacketInfoIpv6WithVxlan);
    prvTgfSgtVerification(PRV_TGF_SGT_VERIFY_SGT_WITH_TAG_0_E, 0);

    /* Packet - 4 */
    prvTgfPacketTunnelL2Part.saMac[5] += 1;
    /* TODO - IPCL_0_0 - UDB pattern match not working so changing srcIndex this way
     * IPCL_0_0   - To Set copyReserved = srcIndex, sourceID = 0 */
    prvTgfSgtTunnelIpclConfig(ingressPort, prvTgfSgtSrcIndexArr[0]);

    /* IPCL_0_1 - To Set Source ID - 54/52/49 */
    prvTgfSgtTunnelIpcl1Config(ingressPort, prvTgfSgtDstIndexArr[0]);

    restoreVni = prvTgfPacketTunnelInputVxlanGpePart.vni;
    prvTgfPacketTunnelInputVxlanGpePart.vni = 0;

    prvTgfSgtPhaConfig(egressPort, THR3_SGT_NetRemove);
    prvTgfSgtSendAndCapturePkt(ingressPort, egressPort, &prvTgfPacketInfoIpv6WithVxlan);
    prvTgfSgtVerification(PRV_TGF_SGT_VERIFY_NO_SGT_WITH_TAG0_E, 0);

    /* Restore */
    prvTgfSgtTunnelIpclConfig(ingressPort, prvTgfSgtSrcIndexArr[1]);
    prvTgfPacketTunnelInputVxlanGpePart.vni = restoreVni;
    prvTgfPacketTunnelInputVxlanGpePart.vni = restoreVni;
    
    /*******************************************************************/
    /********** Tunnel Port -> User Port related test cases ************/
    /*******************************************************************/
    getFirst = GT_TRUE;
    cpssOsMemCpy(prvTgfPacketTunnelL2Part.daMac, PORT_4_MAC, sizeof(prvTgfPacketTunnelL2Part.daMac));
    ingressPort = prvTgfPortsArray[PRV_TGF_SGT_SEND_PORT_1_IDX_CNS];
    egressPort  = prvTgfPortsArray[PRV_TGF_SGT_RECEIVE_PORT_2_IDX_CNS];
    prvTgfSgtPhaConfig(egressPort, THR0_DisablePha);

    prvTgfSgtTunnelIpclConfig(ingressPort, prvTgfSgtSrcIndexArr[1]);
    /* Packet - 3 */
    /* IPCL_0_1 - To Set Source ID - 54/52/49 */
    prvTgfSgtTunnelIpcl1Config(ingressPort, prvTgfSgtDstIndexArr[2]);

    prvTgfPacketTunnelL2Part.saMac[5] += 1;
    dropExpected = GT_TRUE;
    prvTgfPacketTunnelInputUdpPart.dstPort = 191;
    prvTgfSgtSendAndCapturePkt(ingressPort, egressPort, &prvTgfPacketInfoIpv6WithVxlan);
    dropExpected = GT_FALSE;

    /* Packet - 1 */
    /* IPCL_0_1 - To Set Source ID - 54/52/49 */
    prvTgfSgtTunnelIpcl1Config(ingressPort, prvTgfSgtDstIndexArr[1]);

    prvTgfPacketTunnelL2Part.saMac[5] += 1;
    prvTgfPacketTunnelInputUdpPart.dstPort = 191;
    prvTgfSgtSendAndCapturePkt(ingressPort, egressPort, &prvTgfPacketInfoIpv6WithVxlan);
    prvTgfSgtVerification(PRV_TGF_SGT_VERIFY_NO_SGT_E, 0);

    /* Packet - 2 */
    prvTgfPacketTunnelL2Part.saMac[5] += 1;
    /* TODO - EPCL should drop in case of Udp = 192 */
    prvTgfPacketTunnelInputUdpPart.dstPort = 192;
    prvTgfSgtSendAndCapturePkt(ingressPort, egressPort, &prvTgfPacketInfoIpv6WithVxlan);
    prvTgfSgtVerification(PRV_TGF_SGT_VERIFY_NO_SGT_E, 0);

    /* Packet - 4 */
    prvTgfPacketTunnelL2Part.saMac[5] += 1;
    /* TODO - IPCL_0_0 - UDB pattern match not working so changing srcIndex this way
     * IPCL_0_0   - To Set copyReserved = srcIndex, sourceID = 0 */
    prvTgfSgtTunnelIpclConfig(ingressPort, prvTgfSgtSrcIndexArr[0]);

    /* IPCL_0_1 - To Set Source ID - 54/52/49 */
    prvTgfSgtTunnelIpcl1Config(ingressPort, prvTgfSgtDstIndexArr[0]);

    restoreVni = prvTgfPacketTunnelInputVxlanGpePart.vni;
    prvTgfPacketTunnelInputVxlanGpePart.vni = 0;
    prvTgfSgtSendAndCapturePkt(ingressPort, egressPort, &prvTgfPacketInfoIpv6WithVxlan);
    prvTgfSgtVerification(PRV_TGF_SGT_VERIFY_NO_SGT_E, 0);

    /* Restore */
    prvTgfSgtTunnelIpclConfig(ingressPort, prvTgfSgtSrcIndexArr[1]);
    prvTgfPacketTunnelInputVxlanGpePart.vni = restoreVni;
    prvTgfPacketTunnelInputVxlanGpePart.vni = restoreVni;
}

/**
* @internal prvTgfSgtTunnelPort_test_restore function
* @endinternal
*
* @brief  SGT Test cases for Network port - Restore
*/
static GT_VOID prvTgfSgtTunnelPort_test_restore(GT_VOID)
{
    GT_STATUS                       rc;
    GT_U32                          i;
    PRV_TGF_PCL_LOOKUP_CFG_STC      lookupCfg;
    CPSS_INTERFACE_INFO_STC         interfaceInfo;

    PRV_UTF_LOG0_MAC("======= Restore configurations =======\n");
    /* -------------------------------------------------------------------------
     * 1. Restore Bridge Configuration
     */
    /* AUTODOC: flush FDB include static entries */
    prvTgfSgtBridgeRestore();

    /* -------------------------------------------------------------------------
     * 2. Restore Tpid Configuration
     */
    prvTgfSgtTpidConfig(GT_FALSE, CPSS_DIRECTION_INGRESS_E, 1, 0/*egressPort*/);

    /* -------------------------------------------------------------------------
     * 3. Restore IPCL Configuration
     */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_60_BYTES_E, PRV_TGF_IPCL_0_ID, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfPclRuleValidStatusSet, index = %d", 0);

    /* -------------------------------------------------------------------------
     * 6. Restore PHA Configuration
     */
    prvTgfSgtPhaConfig(prvTgfPortsArray[PRV_TGF_SGT_RECEIVE_PORT_1_IDX_CNS], THR0_DisablePha);
    prvTgfSgtPhaConfig(prvTgfPortsArray[PRV_TGF_SGT_RECEIVE_PORT_2_IDX_CNS], THR0_DisablePha);

    /* -------------------------------------------------------------------------
     * 7. Restore EPCL Configuration
     */
    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));
    lookupCfg.groupKeyTypes.nonIpKey = PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E;
    lookupCfg.groupKeyTypes.ipv4Key  = PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E;
    lookupCfg.groupKeyTypes.ipv6Key  = PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_E;

    interfaceInfo.type  = CPSS_INTERFACE_INDEX_E;

    for (i=0; i<3;i++)
    {
        rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_60_BYTES_E,
                PRV_TGF_EPCL_RULE_INDEX(i),
                GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet:");

        interfaceInfo.index = PRV_TGF_EPCL_SRC_TRG_INDEX(prvTgfSgtSrcIndexArr[i], prvTgfSgtDstIndexArr[i]);
        rc = prvTgfPclCfgTblSet(&interfaceInfo, CPSS_PCL_DIRECTION_EGRESS_E,
                CPSS_PCL_LOOKUP_0_E, &lookupCfg);
        UTF_VERIFY_EQUAL0_STRING_MAC( GT_OK, rc, "prvTgfPclCfgTblSet");
    }

    /* AUTODOC: Restore access mode for EPCL lookup */
    rc = prvTgfPclPortLookupCfgTabAccessModeSet(
            prvTgfPortsArray[PRV_TGF_SGT_RECEIVE_PORT_1_IDX_CNS],
            CPSS_PCL_DIRECTION_EGRESS_E,
            CPSS_PCL_LOOKUP_0_E,             /* Lookup Number */
            0,                               /* SubLookup Number */
            prvTgfSgtRestoreCfg.cfgTabAccMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeSet");

    /* AUTODOC: Restore access mode for EPCL lookup */
    rc = prvTgfPclPortLookupCfgTabAccessModeSet(
            prvTgfPortsArray[PRV_TGF_SGT_RECEIVE_PORT_2_IDX_CNS],
            CPSS_PCL_DIRECTION_EGRESS_E,
            CPSS_PCL_LOOKUP_0_E,             /* Lookup Number */
            0,                               /* SubLookup Number */
            prvTgfSgtRestoreCfg.cfgTabAccMode_2ndPort);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeSet");

    /* -------------------------------------------------------------------------
     * 8. Restore base Configuration
     */
    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: Disable RX capture */
    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in tgfTrafficTableRxStartCapture");

    /* AUTODOC: reset hw counters */
    rc = prvTgfCommonAllBridgeCntrsReset();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfCommonAllCntrsReset");
}

/**
* @internal prvTgfSgtTunnelPort_test_config function
* @endinternal
*
* @brief  SGT Test cases for tunnel port - config
*/
GT_VOID prvTgfSgtTunnelPort_test_config()
{
    GT_STATUS rc;

    /* Common bridge related config */
    prvTgfSgtBridgeConfig(PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E,
            PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E,
            PRV_TGF_VLANID_2_CNS,
            PRV_TGF_DEF_VLANID_CNS);

    /* IPCL_0_0   - To Set copyReserved = srcIndex, sourceID = 0 */
    prvTgfSgtTunnelIpclConfig(prvTgfPortsArray[PRV_TGF_SGT_SEND_PORT_1_IDX_CNS], prvTgfSgtSrcIndexArr[1]);

    /* IPCL_0_1 - To Set Source ID - 54/52/49 */
    prvTgfSgtTunnelIpcl1Config(prvTgfPortsArray[PRV_TGF_SGT_SEND_PORT_1_IDX_CNS], prvTgfSgtDstIndexArr[1]);

    /* Epcl Config */
    prvTgfSgtEpclConfig(prvTgfPortsArray[PRV_TGF_SGT_RECEIVE_PORT_1_IDX_CNS],
            GT_TRUE,
            CPSS_PCL_LOOKUP_0_E);

    /* Backup src trg access mode for EPCL lookup */
    rc = prvTgfPclPortLookupCfgTabAccessModeGet(prvTgfPortsArray[PRV_TGF_SGT_RECEIVE_PORT_2_IDX_CNS],
                           CPSS_PCL_DIRECTION_EGRESS_E,
                           CPSS_PCL_LOOKUP_0_E,             /* Lookup Number */
                           0,                               /* SubLookup Number */
                           &prvTgfSgtRestoreCfg.cfgTabAccMode_2ndPort);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeGet");

    prvTgfSgtEpclConfig(prvTgfPortsArray[PRV_TGF_SGT_RECEIVE_PORT_2_IDX_CNS],
            GT_TRUE,
            CPSS_PCL_LOOKUP_0_E);

    /* PCL UDB Config */
    prvTgfSgtPclUdbConfig();
}

/**
* @internal prvTgfSgtCascadePort_test_restore function
* @endinternal
*
* @brief  SGT Test cases for Cascade port - Restore
*/
static GT_VOID prvTgfSgtCascadePort_test_restore(GT_VOID)
{
    GT_STATUS                               rc;

    PRV_UTF_LOG0_MAC("======= Restore configurations =======\n");
    /* -------------------------------------------------------------------------
     * 1. Restore Bridge Configuration
     */
    /* AUTODOC: flush FDB include static entries */
    prvTgfSgtBridgeRestore();

    /* -------------------------------------------------------------------------
     * 2. Restore PHA Configuration
     */
    prvTgfSgtPhaConfig(prvTgfPortsArray[PRV_TGF_SGT_RECEIVE_PORT_1_IDX_CNS], THR0_DisablePha);
    prvTgfSgtPhaConfig(prvTgfPortsArray[PRV_TGF_SGT_RECEIVE_PORT_2_IDX_CNS], THR0_DisablePha);

    /* -------------------------------------------------------------------------
     * 3. Restore base Configuration
     */

    rc = tgfTrafficGeneratorEgressCscdPortEnableSet(
            prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_SGT_RECEIVE_PORT_1_IDX_CNS],
            GT_FALSE, CPSS_CSCD_PORT_NETWORK_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorEgressCscdPortEnableSet");

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: Disable RX capture */
    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in tgfTrafficTableRxStartCapture");

    /* AUTODOC: reset hw counters */
    rc = prvTgfCommonAllBridgeCntrsReset();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfCommonAllCntrsReset");
}

/**
 * @internal prvTgfSgtCascadePort_test_trafficAndVerify function
 * @endinternal
*
* @brief  SGT Test cases for Cascade port - Traffic and verify
* SEND_1    - Network Port
* SEND_2    - Not used
* RECEIVE_1 - Cascade Port, Egress tagged(Outer_Tag0_Inner_Tag1)
* RECEIVE_2 - Not used
*/
static GT_VOID prvTgfSgtCascadePort_test_trafficAndVerify(GT_VOID)
{
    GT_STATUS                   rc;
    GT_U8                       portIter;
    GT_PORT_NUM                 ingressPort, egressPort;
    GT_U32                      offset;

    /* AUTODOC: Reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /*******************************************************************/
    /******** Network Port -> Cascade Port related test cases **********/
    /*******************************************************************/
    cpssOsMemCpy(prvTgfPacketL2Part.saMac, sa_mac[0], sizeof(prvTgfPacketL2Part.saMac));
    cpssOsMemCpy(prvTgfPacketL2Part.daMac, PORT_2_MAC, sizeof(prvTgfPacketL2Part.daMac));
    ingressPort = prvTgfPortsArray[PRV_TGF_SGT_SEND_PORT_1_IDX_CNS];
    egressPort  = prvTgfPortsArray[PRV_TGF_SGT_RECEIVE_PORT_1_IDX_CNS];
    getFirst = GT_TRUE;

    /* Packet - 1 - SGT_eDSAFix */
    prvTgfSgtPhaConfig(egressPort, THR4_SGT_eDSAFix);
    prvTgfSgtSendAndCapturePkt(ingressPort, egressPort, &prvTgfPacketInfoSgtAndVlanTag);

    /* Verification
     *     - eDSA word3[26] - Tag1 Src Tagged should be '0'
     *     - SGT TAG After eDSA(8 Bytes)
     */
    offset = TGF_L2_HEADER_SIZE_CNS + (4*3); /* L2 Header + eDSA 3rd word */
    UTF_VERIFY_EQUAL0_STRING_MAC(0,
            (packetBuf[offset] & (1 << 2)),
            "Packet compare failed : src tagged bit not clear");

    /* SGT Tag verify */
    offset = TGF_L2_HEADER_SIZE_CNS + TGF_eDSA_TAG_SIZE_CNS;
    rc = cpssOsMemCmp(&packetBuf[offset], &expectedSgtTag[0], 8) == 0 ? GT_OK: GT_FAIL;
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Expected SGT Tag mismatch");

    /* Packet - 2 - SGT_eDSARemove */
    prvTgfPacketL2Part.saMac[5] += 1;
    prvTgfSgtPhaConfig(egressPort, THR5_SGT_eDSARemove);
    prvTgfSgtSendAndCapturePkt(ingressPort, egressPort, &prvTgfPacketInfoSgtAndVlanTag);

    /* Verification
     *     - SGT TAG After eDSA(4 Bytes, 4 Bytes removed by PHA) */
    offset = TGF_L2_HEADER_SIZE_CNS + TGF_eDSA_TAG_SIZE_CNS;
    rc = cpssOsMemCmp(&packetBuf[offset], &expectedSgtTag[0], 4) == 0 ? GT_OK: GT_FAIL;
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Expected SGT Tag mismatch");
}

/**
* @internal prvTgfSgtCascadePort_test_config function
* @endinternal
*
* @brief  SGT Test cases for Cascade port - config
*/
static GT_VOID prvTgfSgtCascadePort_test_config(GT_VOID)
{
    GT_STATUS               rc;

    /* Common bridge related config */
    prvTgfSgtBridgeConfig(
            PRV_TGF_BRG_VLAN_PORT_TAG1_CMD_E,
            PRV_TGF_BRG_VLAN_PORT_TAG1_CMD_E,
            PRV_TGF_VLANID_2_CNS,
            PRV_TGF_DEF_VLANID_CNS);

    rc = tgfTrafficGeneratorEgressCscdPortEnableSet(
            prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_SGT_RECEIVE_PORT_1_IDX_CNS],
            GT_TRUE,
            CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorEgressCscdPortEnableSet");
}


/**
* @internal prvTgfSgtUserPort_test function
* @endinternal
*
* @brief  SGT Test cases for user port
*/
GT_VOID prvTgfSgtUserPort_test(GT_VOID)
{
    GT_U8       timeStampTagSize = 0;

    /******** Test without any timestamp TAG ********/
    prvTgfSgtUserPort_test_config();
    prvTgfSgtUserPort_test_trafficAndVerify(timeStampTagSize);

    /******** Test with timestamp TAG(16Bytes) ********/
    timeStampTagSize = 16;
    prvTgfSgtPtpConfig(
            PRV_TGF_PTP_TS_TAG_MODE_ALL_EXTENDED_E,
            GT_FALSE /* Not restore */);
    prvTgfSgtUserPort_test_trafficAndVerify(timeStampTagSize);
    prvTgfSgtPtpConfig(
            PRV_TGF_PTP_TS_TAG_MODE_ALL_EXTENDED_E,
            GT_TRUE /* Restore */);

    /******** Test with timestamp TAG(8Bytes) ********/
    timeStampTagSize = 8;
    prvTgfSgtPtpConfig(
            PRV_TGF_PTP_TS_TAG_MODE_ALL_NON_EXTENDED_E,
            GT_FALSE /* Not restore */);
    prvTgfSgtUserPort_test_trafficAndVerify(timeStampTagSize);
    prvTgfSgtPtpConfig(
            PRV_TGF_PTP_TS_TAG_MODE_ALL_NON_EXTENDED_E,
            GT_TRUE /* Restore */);

    /* Restore */
    prvTgfSgtUserPort_test_restore();
    return;
}

/**
* @internal prvTgfSgtNetworkPort_test function
* @endinternal
*
* @brief  SGT Test cases for Network port
*/
GT_VOID prvTgfSgtNetworkPort_test(GT_VOID)
{

    prvTgfSgtNetworkPort_test_config();
    prvTgfSgtNetworkPort_test_trafficAndVerify();
    prvTgfSgtNetworkPort_test_restore();

    return;
}

/**
* @internal prvTgfSgtTunnelPort_test function
* @endinternal
*
* @brief  SGT Test cases for Tunnel port
*/
GT_VOID prvTgfSgtTunnelPort_test(GT_VOID)
{
    prvTgfSgtTunnelPort_test_config();
    prvTgfSgtTunnelPort_test_trafficAndVerify();
    prvTgfSgtTunnelPort_test_restore();

    return;
}

/**
* @internal prvTgfSgtCascadePort_test function
* @endinternal
*
* @brief  SGT Test cases for cascade port
*/
GT_VOID prvTgfSgtCascadePort_test(GT_VOID)
{
    prvTgfSgtCascadePort_test_config();
    prvTgfSgtCascadePort_test_trafficAndVerify();
    prvTgfSgtCascadePort_test_restore();
}

/**
* @internal tgfSgtUseSrcTrgMode_test1_restore function
* @endinternal
*
* @brief  SGT Test case to verify "Use Src Trg mode" - Restore
*/
static GT_VOID tgfSgtUseSrcTrgMode_test1_restore(GT_VOID)
{
    GT_STATUS                               rc;

    PRV_UTF_LOG0_MAC("======= Restore configurations =======\n");

    /* Epcl Config */
    prvTgfSgtEpclConfig(prvTgfPortsArray[PRV_TGF_SGT_RECEIVE_PORT_1_IDX_CNS],
            GT_FALSE,
            CPSS_PCL_LOOKUP_1_E);

    /* -------------------------------------------------------------------------
     * Restore Bridge Configuration
     */
    /* AUTODOC: flush FDB include static entries */
    prvTgfSgtBridgeRestore();

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: Disable RX capture */
    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in tgfTrafficTableRxStartCapture");

    /* AUTODOC: reset hw counters */
    rc = prvTgfCommonAllBridgeCntrsReset();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfCommonAllCntrsReset");
}

/**
* @internal tgfSgtUseSrcTrgMode_test1_config function
* @endinternal
*
* @brief  SGT Test case to verify "Use Src Trg mode" - config
*/
static GT_VOID tgfSgtUseSrcTrgMode_test1_config
(
    IN GT_BOOL      srcEpgFdb,
    IN GT_U32       sgtIndex
)
{
    GT_STATUS                           rc;
    CPSS_DXCH_BRG_FDB_EPG_CONFIG_STC    fdbEpgConfig;
    PRV_TGF_TTI_ACTION_STC              ttiAction;
    PRV_TGF_TTI_RULE_UNT                ttiPattern;
    PRV_TGF_TTI_RULE_UNT                ttiMask;
    CPSS_DXCH_TTI_GENERIC_ACTION_MODE_ENT ttiGenericActionMode;
    GT_U32                              ttiGenericAction;

    prvTgfTtiRuleIndex = prvTgfTtiTcamAbsoluteIndexWithinTheLookupHitGet(PRV_TTI_LOOKUP_0, 3);

    rc = cpssDxChBrgFdbEpgConfigGet(prvTgfDevNum, &fdbEpgConfigRestore);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgFdbEpgConfigGet");

    /* AUTODOC: Configgure FDB to assign dst EPG */
	cpssOsMemSet(&fdbEpgConfig, 0, sizeof(fdbEpgConfig));
    if(srcEpgFdb)
    {
        fdbEpgConfig.srcEpgAssignEnable = GT_TRUE;
        globalEpgNumber     = prvTgfSgtSrcIndexArr[sgtIndex];
        ttiGenericActionMode= CPSS_DXCH_TTI_GENERIC_ACTION_MODE_DST_EPG_E;
        ttiGenericAction    = prvTgfSgtDstIndexArr[sgtIndex];
    }
    else
    {
        fdbEpgConfig.dstEpgAssignEnable = GT_TRUE;
        globalEpgNumber     = prvTgfSgtDstIndexArr[sgtIndex];
        ttiGenericActionMode= CPSS_DXCH_TTI_GENERIC_ACTION_MODE_SRC_EPG_E;
        ttiGenericAction    = prvTgfSgtSrcIndexArr[sgtIndex];
    }

    rc = cpssDxChBrgFdbEpgConfigSet(prvTgfDevNum, &fdbEpgConfig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgFdbEpgConfigSet");

    /* AUTODOC: Common bridge related config */
    prvTgfSgtBridgeConfig(
            PRV_TGF_BRG_VLAN_PORT_TAG1_CMD_E,
            PRV_TGF_BRG_VLAN_PORT_TAG1_CMD_E,
            PRV_TGF_VLANID_2_CNS,
            PRV_TGF_DEF_VLANID_CNS);

    /* Configure TTI to assign src EPG */
	rc = cpssDxChTtiGenericActionModeSet(prvTgfDevNum,
            CPSS_PCL_LOOKUP_0_E,
            ttiGenericActionMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChTtiGenericActionModeSet");

    /* AUTODOC: enable the TTI lookup for PRV_TGF_TTI_KEY_ETH_E at the port 1 */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SGT_SEND_PORT_1_IDX_CNS],
                                PRV_TGF_TTI_KEY_ETH_E,
                                GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

    /* AUTODOC: set TTI Rule */
    cpssOsMemSet(&ttiPattern, 0, sizeof(ttiPattern));
    ttiPattern.eth.common.vid   = PRV_TGF_VLANID_2_CNS;

    /* AUTODOC: set TTI mask */
    cpssOsMemSet(&ttiMask, 0, sizeof(ttiMask));
    ttiMask.eth.common.vid      = 0xFFF;

    cpssOsMemSet(&ttiAction, 0, sizeof(ttiAction));
    ttiAction.command                                   = CPSS_PACKET_CMD_FORWARD_E;
    ttiAction.redirectCommand                           = PRV_TGF_TTI_ASSIGN_GENERIC_ACTION_E;
    ttiAction.interfaceInfo.type                        = CPSS_INTERFACE_PORT_E;
    ttiAction.interfaceInfo.devPort.hwDevNum            = prvTgfDevNum;
    ttiAction.interfaceInfo.devPort.portNum             = prvTgfPortsArray[PRV_TGF_SGT_SEND_PORT_1_IDX_CNS],
    ttiAction.genericAction                             = ttiGenericAction;

    rc = prvTgfTtiRuleSet(prvTgfTtiRuleIndex, PRV_TGF_TTI_KEY_ETH_E,
                          &ttiPattern, &ttiMask, &ttiAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet");

    /* AUTODOC: EPCL Config */
    prvTgfSgtEpclConfig(prvTgfPortsArray[PRV_TGF_SGT_RECEIVE_PORT_1_IDX_CNS],
            GT_TRUE,
            CPSS_PCL_LOOKUP_1_E);
}



/**
* @internal tgfSgtUseSrcTrgMode_test1_trafficAndVerify function
* @endinternal
*
* @brief  SGT Test case to verify "Use Src Trg mode" - traffic and verify
*/
static GT_VOID tgfSgtUseSrcTrgMode_test1_trafficAndVerify()
{
    GT_STATUS                   rc;
    GT_U8                       portIter;
    GT_PORT_NUM                 ingressPort, egressPort;

    /* AUTODOC: Reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    cpssOsMemCpy(prvTgfPacketL2Part.daMac, PORT_2_MAC, sizeof(prvTgfPacketL2Part.daMac));
    ingressPort     = prvTgfPortsArray[PRV_TGF_SGT_SEND_PORT_1_IDX_CNS];
    egressPort      = prvTgfPortsArray[PRV_TGF_SGT_RECEIVE_PORT_1_IDX_CNS];
    getFirst        = GT_TRUE;

    prvTgfSgtSendAndCapturePkt(ingressPort, egressPort, &prvTgfPacketInfoSingleTag);
}




/**
* @internal tgfSgtUseSrcTrgMode_test1 function
* @endinternal
*
* @brief  SGT Test case to verify "Use Src Trg mode"
*/
GT_VOID tgfSgtUseSrcTrgMode_test1(GT_VOID)
{
    /* Case 1:
     *      FDB - SRC EPG
     *      TTI - DST EPG */
    tgfSgtUseSrcTrgMode_test1_config(
            GT_FALSE /* srcEpgFdb */,
            2 /* sgt Array index */);

    /* Traffic and Verify */
    dropExpected = GT_TRUE;     /* EPCL Rule - SGT array index 2 - Drop */
    tgfSgtUseSrcTrgMode_test1_trafficAndVerify();
    dropExpected = GT_FALSE;

    /* Restore */
    tgfSgtUseSrcTrgMode_test1_restore();

    /* Case 2:
     *      TTI - SRC EPG
     *      FDB - DST EPG */
    tgfSgtUseSrcTrgMode_test1_config(GT_TRUE, 0);

    /* Traffic and Verify
       EPCL Rule - SGT array index 0 - Forward */
    tgfSgtUseSrcTrgMode_test1_trafficAndVerify();

    /* Traffic and Verify */
    tgfSgtUseSrcTrgMode_test1_restore();
    return;
}

