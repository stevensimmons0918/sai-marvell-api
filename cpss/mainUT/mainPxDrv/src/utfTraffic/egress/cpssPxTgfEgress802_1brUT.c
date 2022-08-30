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
* @file cpssPxTgfEgress802_1brUT.c
*
* @brief Related to 802.1BR systems :
* Egress tests for tests with traffic for the Pipe devices.
*
* @version   1
********************************************************************************
*/
/* includes */
#include <cpss/px/cpssPxTypes.h>
#include <utfTraffic/basic/cpssPxTgfBasicUT.h>
#include <cpss/px/egress/cpssPxEgress.h>
#include <cpss/px/ingress/cpssPxIngress.h>
#include <cpss/px/ptp/cpssPxPtp.h>

static GT_BOOL  phaBypassEnable = GT_FALSE;
extern GT_BOOL gPtpNewParseMode;

#define INGRESS_PORT_NUM                    7
#define PRE_MAC_DA_PORT1                    10
#define PRE_MAC_DA_PORT2                    20
#define EXTENDED_PRE_MAC_DA_PORT1_NUM       8
#define EXTENDED_PRE_MAC_DA_PORT2_NUM       9

#define UPLINK_PORT_NUM             12
#define IPL_PORT_NUM                4
#define LAG_MC_SRC_PORT_NUM         12

#define ETAG_TPID                   0x893f
#define EVB_TAG_TPID                0x7940
#define EVB_QCN_TAG_TPID            0x8901
#define E_PCP_CNS                   0
#define E_DEI_CNS                   0
#define E_PCP1_CNS                  6
#define E_DEI1_CNS                  1
#define DUMMY_INGRESS_E_CID_BASE    0x111
#define ECID_INGRESS_PORT_CNS       0xabf
#define DUMMY_INGRESS_E_CID_EXT     0xab
#define DUMMY_E_CID_EXT             0xcd

#define EXTENDED_PORT_PVID_CNS      0x789
#define EXTENDED_PORT_TPID_CNS      TGF_ETHERTYPE_9100_TAG_CNS
#define EXTENDED_PORT_PTP_TPID_CNS  0x88F7
#define EXTENDED_PORT_MPLS_TPID_CNS 0x8847
#define QCN_ETHER_TYPE              0x8737
#define MPLS_LABEL                  0x3c2b1a
#define DUMMY_BYTE                  0xef

#define MULTICAST_TARGET_PORT       0x80000000
#define SRC_FILTERING_TEST_ECID    (0x10000)

#define TGF_PTP_MESSAGE_TYPE        0x5
#define TGF_PTP_VERSION             0x8
#define TGF_DOMAIN_NUMBER           0xA
#define TGF_UDP_DEST_PORT           0x1234
#define TGF_IPV4_IPV6_PROTOCOL      0x11
#define MAC_DA                      0x00, 0x00, 0x00, 0x00, 0x00, 0x01
#define MAC_SA                      0x00, 0x00, 0x00, 0x00, 0x23, 0x01

#define PTP_EGRES_DELAY_NANO_SEC    100

static GT_U8 frameVlanTagExtArr[] = {
/*byte 0*/    (GT_U8)(EXTENDED_PORT_TPID_CNS >> 8),
/*byte 1*/    (GT_U8)(EXTENDED_PORT_TPID_CNS & 0xFF),
/*byte 2*/    (GT_U8)((E_PCP1_CNS << 5) | (E_DEI1_CNS << 4) | (((EXTENDED_PORT_PVID_CNS) >> 8) & 0x0F)),
/*byte 3*/    (GT_U8)(EXTENDED_PORT_PVID_CNS & 0xFF)
};

static GT_U8 frameVlanTagArr[] = {
/*byte 0*/    (GT_U8)(EXTENDED_PORT_TPID_CNS >> 8),
/*byte 1*/    (GT_U8)(EXTENDED_PORT_TPID_CNS & 0xFF),
/*byte 2*/    (GT_U8)((EXTENDED_PORT_PVID_CNS >> 8) & 0x0F),
/*byte 3*/    (GT_U8)(EXTENDED_PORT_PVID_CNS & 0xFF)
};

static GT_U8 framePtpEthertype[] = {
/*byte 0*/    (GT_U8)(EXTENDED_PORT_PTP_TPID_CNS >> 8),
/*byte 1*/    (GT_U8)(EXTENDED_PORT_PTP_TPID_CNS & 0xFF),
};

static GT_U8 frameEvbVlanTagArr[] = {
/*byte 0*/    (GT_U8)(EVB_TAG_TPID >> 8),
/*byte 1*/    (GT_U8)(EVB_TAG_TPID & 0xFF),
/*byte 2*/    (GT_U8)((EXTENDED_PORT_PVID_CNS >> 8) & 0x0F),
/*byte 3*/    (GT_U8)(EXTENDED_PORT_PVID_CNS & 0xFF)
};

static GT_U8 framePtpL2MplsCwBytes[] = {
/*byte 0*/    (GT_U8)(EXTENDED_PORT_MPLS_TPID_CNS >> 8),
/*byte 1*/    (GT_U8)(EXTENDED_PORT_MPLS_TPID_CNS & 0xFF),
/*byte 2*/    (GT_U8)(MPLS_LABEL & 0xFF),
/*byte 3*/    (GT_U8)((MPLS_LABEL >> 8) & 0xFF),
/*byte 4*/    (GT_U8)((MPLS_LABEL >> 16) & 0xF),
/*byte 5*/    (GT_U8)(DUMMY_BYTE),   /* MPLS Label */
/*byte 6*/    (GT_U8)(DUMMY_BYTE),   /* MPLS CW */
/*byte 7*/    (GT_U8)(DUMMY_BYTE),
/*byte 8*/    (GT_U8)(DUMMY_BYTE),
/*byte 9*/    (GT_U8)(DUMMY_BYTE),
/*byte 10*/   MAC_DA,
/*byte 16*/   MAC_SA,
/*byte 22*/   (GT_U8)(EXTENDED_PORT_PTP_TPID_CNS >> 8),
/*byte 23*/   (GT_U8)(EXTENDED_PORT_PTP_TPID_CNS & 0xFF),
/*byte 24*/   (GT_U8)(TGF_PTP_MESSAGE_TYPE),
/*byte 25*/   (GT_U8)(TGF_PTP_VERSION),
/*byte 26*/   (GT_U8)(DUMMY_BYTE),  /* Message length */
/*byte 27*/   (GT_U8)(DUMMY_BYTE),  /* Message length */
/*byte 28*/   (GT_U8)(TGF_DOMAIN_NUMBER)
};

static GT_U8 frameQcnTag[] = {
/*byte 0*/    (GT_U8)(QCN_ETHER_TYPE >> 8),
/*byte 1*/    (GT_U8)(QCN_ETHER_TYPE & 0xFF),
/*byte 2*/    (GT_U8)((0 << 4)/* QCN version */ | (0 >> 2)), /* QCN Resreved */
/*byte 3*/    (GT_U8)((0 & 0x03) /* QCN Resreved */ | 0), /* QCN qfb */
/*byte 4*/    (GT_U8)(0), /* QCN CPID [63:56] */
/*byte 5*/    (GT_U8)(0), /* QCN CPID [55:48] */
/*byte 6*/    (GT_U8)(0), /* QCN CPID [47:40] */
/*byte 7*/    (GT_U8)(0), /* QCN CPID [39:32] */
/*byte 8*/    (GT_U8)(0), /* QCN CPID [31:24] */
/*byte 9*/    (GT_U8)(0), /* QCN CPID [23:16] */
/*byte 10*/   (GT_U8)(0), /* QCN CPID [15: 8] */
/*byte 11*/   (GT_U8)((0 << 3) /* QCN CPID [7:3] */ | (0 & 0x3)),/* QCN CPID [2:0] - congested queue number */
/*byte 12*/   (GT_U8)(0), /* Clear to zero the last 7 bytes of QCN tag*/
/*byte 13*/   (GT_U8)(0),
/*byte 14*/   (GT_U8)(0),
/*byte 15*/   (GT_U8)(0),
/*byte 16*/   (GT_U8)(0),
/*byte 17*/   (GT_U8)(0),
/*byte 18*/   (GT_U8)(0)

};

static GT_U8 frameEvbVlanTag[] = {
/*byte 0*/    (GT_U8)(EVB_QCN_TAG_TPID >> 8),
/*byte 1*/    (GT_U8)(EVB_QCN_TAG_TPID & 0xFF),
/*byte 2*/    (GT_U8)((E_PCP1_CNS << 5) | (E_DEI1_CNS << 4) | (((EXTENDED_PORT_PVID_CNS) >> 8) & 0x0F)),
/*byte 3*/    (GT_U8)(EXTENDED_PORT_PVID_CNS & 0xFF)
};

static GT_U8    egressEtag[8] = {
/*byte 0*/    (GT_U8)(ETAG_TPID >> 8),
/*byte 1*/    (GT_U8)(ETAG_TPID & 0xFF),
/*byte 2*/    (GT_U8)((E_PCP_CNS << 5) | (E_DEI_CNS << 4) | (DUMMY_INGRESS_E_CID_BASE >> 8)), /* ingress_E-CID_base (4 MSBits)*/
/*byte 3*/    (GT_U8)(DUMMY_INGRESS_E_CID_BASE & 0xFF),/* ingress_E-CID_base (8 LSBits)*/
/*byte 4*/    (GT_U8)((0/*Re-*/<<6) | (0/*GRP*/ << 4) |  (ECID_INGRESS_PORT_CNS >> 8)),
/*byte 5*/    (GT_U8)(ECID_INGRESS_PORT_CNS  & 0xFF),
/*byte 6*/    DUMMY_INGRESS_E_CID_EXT,
/*byte 7*/    DUMMY_E_CID_EXT
};
static CPSS_802_1BR_ETAG_STC   egressEtagInfo =
{
    0xF16A  /*TPID;   dummy --> overridden by  CPSS_PX_EGRESS_TARGET_PORT_COMMON_STC::tpid !!! */
   ,7       /*E_PCP;    dummy --> overridden by descriptor->up */
   ,1       /*E_DEI;    dummy --> overridden by descriptor->cfi_dei */
   ,DUMMY_INGRESS_E_CID_BASE       /*Ingress_E_CID_base;*/
   ,0       /*Direction;*/
   ,0       /*UpstreamSpecific*/
   ,0       /*GRP;     */
   ,0x987   /*E_CID_base;  dummy --> overridden by  CPSS_PX_EGRESS_SOURCE_PORT_802_1BR_STC::pcid    */
   ,DUMMY_INGRESS_E_CID_EXT     /*Ingress_E_CID_ext;*/
   ,DUMMY_E_CID_EXT             /*E_CID_ext;        */
};

static CPSS_802_1BR_ETAG_STC   egressEtag2CPUInfo =
{
    0x893F  /*TPID*/
   ,0       /*E_PCP*/
   ,0       /*E_DEI*/
   ,0       /*Ingress_E_CID_base*/
   ,0       /*Direction;*/
   ,0       /*UpstreamSpecific*/
   ,0       /*GRP*/
   ,0x987   /*E_CID_base;  dummy --> overridden by  CPSS_PX_EGRESS_SOURCE_PORT_802_1BR_STC::pcid    */
   ,DUMMY_INGRESS_E_CID_EXT     /*Ingress_E_CID_ext;*/
   ,DUMMY_E_CID_EXT             /*E_CID_ext;        */
};

static CPSS_PX_VLAN_TAG_STC evbQcnVlanTag =
{
    E_PCP1_CNS                       /* PCP */
   ,E_DEI1_CNS                       /* DEI */
   ,EXTENDED_PORT_PVID_CNS           /* VID */
};

static CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_MC_STC  egressLagMcInfo[] =
{
    {{3,4,5,6,7,8,9}}
   ,{{3,4,5,6,7,8,9}}
   ,{{3,4,5,6,7,8,9}}
   ,{{2,4,5,6,7,8,9}}
   ,{{2,3,5,6,7,8,9}}
   ,{{2,3,4,6,7,8,9}}
   ,{{2,3,4,5,7,8,9}}
   ,{{2,3,4,5,6,8,9}}
   ,{{2,3,4,5,6,7,9}}
};

static GT_U8 evbQcnL2Part[] = {MAC_DA, MAC_SA};

#define TGF_PRE_DA_TAG_LEN    2

static CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_QCN_STC evbQcnInfo =
{
    {
        {
            MAC_SA
        }
    }
    ,
    QCN_ETHER_TYPE
};

static CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_PRE_DA_PTP_DATA_STC preMacDaInfo =
{
    TGF_PTP_MESSAGE_TYPE,   /** PTP message type */
    TGF_PTP_VERSION,        /** PTP version */
    TGF_DOMAIN_NUMBER,      /** Domain number */
    TGF_UDP_DEST_PORT,      /** UDP destination port */
    TGF_IPV4_IPV6_PROTOCOL, /** IPv4 protocol/IPv6 next header */
};

static GT_U32 ptpTimestampByteNumMaskList[] = {30, 31, 32, 33, 34, 35};

static GT_U8 framePreDaTagArr[] = {
/*byte 0*/    (GT_U8)0,
/*byte 1*/    (GT_U8)INGRESS_PORT_NUM
};

static TGF_BYTES_INFO_LIST_STC    egressTags_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_PRE_DA_EXTENDED_TO_UPSTREAM_PORT_E =
{
    1,/*pre MAC DA tag*/ /*numOfParts*/
    {
        {
            2,
            framePreDaTagArr,
            GT_TRUE /* Pre MAC DA tag */
        }
    }/*partsInfo*/
};

static GT_U8 framePreDa1TagArr[] = {
/*byte 0*/    (GT_U8)0,
/*byte 1*/    (GT_U8)PRE_MAC_DA_PORT1
};

static TGF_BYTES_INFO_LIST_STC    ingressTags_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_PRE_DA1_UPSTREAM_TO_EXTENDED_PORT_E =
{
    1,/*pre MAC DA tag*/ /*numOfParts*/
    {
        {
            2,
            framePreDa1TagArr,
            GT_TRUE /* Pre MAC DA tag */
        }
    }/*partsInfo*/
};

static GT_U8 framePreDa2TagArr[] = {
/*byte 0*/    (GT_U8)0,
/*byte 1*/    (GT_U8)PRE_MAC_DA_PORT2
};

static TGF_BYTES_INFO_LIST_STC    ingressTags_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_PRE_DA2_UPSTREAM_TO_EXTENDED_PORT_E =
{
    1,/*pre MAC DA tag*/ /*numOfParts*/
    {
        {
            2,
            framePreDa2TagArr,
            GT_TRUE /* Pre MAC DA tag */
        }
    }/*partsInfo*/
};

static TGF_BYTES_INFO_LIST_STC    ingressTags_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_E =
{
    1,/*vlan tag*/ /*numOfParts*/
    {
        {
            4,
            frameVlanTagArr,
            GT_FALSE
        }
    }/*partsInfo*/
};

static TGF_BYTES_INFO_LIST_STC    egressTags_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_E =
{
    2,/*vlan tag + the added ETag*/ /*numOfParts*/
    {
        {
            8,/*numOfBytes*/
            egressEtag,/*partBytesPtr*/
            GT_FALSE
        },
        {
            4,
            frameVlanTagArr,
            GT_FALSE
        }
    }/*partsInfo*/
};

static TGF_BYTES_INFO_LIST_STC    ingressL2PtpTags_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_E =
{
    1,/*vlan tag*/ /*numOfParts*/
    {
        {
            2,
            framePtpEthertype,
            GT_FALSE
        }
    }/*partsInfo*/
};

static TGF_BYTES_INFO_LIST_STC    egressL2PtpTags_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_E =
{
    2,/*vlan tag + the added ETag*/ /*numOfParts*/
    {
        {
            8,/*numOfBytes*/
            egressEtag,/*partBytesPtr*/
            GT_FALSE
        },
        {
            2,
            framePtpEthertype,
            GT_FALSE
        }
    }/*partsInfo*/
};

static TGF_BYTES_INFO_LIST_STC    ingressL2PtpTags_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_PRE_DA_EXTENDED_PORT_TO_UPSTREAM_PORT_E =
{
    1,/*PTP Ether Type + Message Type + Version PTP*/ /*numOfParts*/
    {
        {
            29,
            framePtpL2MplsCwBytes,
            GT_FALSE
        }
    }/*partsInfo*/
};

static TGF_BYTES_INFO_LIST_STC    egressL2PtpTags_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_PRE_DA_EXTENDED_TO_UPSTREAM_PORT_E =
{
    2,/*pre MAC DA tag + PTP Ether Type + Message Type + Version PTP */ /*numOfParts*/
    {
        {
            2,
            framePreDaTagArr,
            GT_TRUE /* Pre MAC DA tag */
        },
        {
            29,
            framePtpL2MplsCwBytes,
            GT_FALSE
        }

    }/*partsInfo*/
};

static TGF_BYTES_INFO_LIST_STC    egressPreDaTag_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_PRE_DA_EXTENDED_TO_UPSTREAM_PORT_E =
{
    1,/*pre MAC DA tag */ /*numOfParts*/
    {
        {
            2,
            framePreDaTagArr,
            GT_TRUE /* Pre MAC DA tag */
        }
    }/*partsInfo*/
};

static TGF_BYTES_INFO_LIST_STC    ingressL2PtpTags_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_PRE_DA1_UPSTREAM_TO_EXTENDED_PORT_E =
{
    2,/*pre MAC DA tag + PTP Ether Type + Message Type + Version PTP */ /*numOfParts*/
    {
        {
            2,
            framePreDa1TagArr,
            GT_TRUE /* Pre MAC DA tag */
        },
        {
            29,
            framePtpL2MplsCwBytes,
            GT_FALSE
        }

    }/*partsInfo*/
};

static TGF_BYTES_INFO_LIST_STC    egressL2PtpTags_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_PRE_DA1_UPSTREAM_TO_EXTENDED_PORT_E =
{
    1,/*PTP Ether Type + Message Type + Version PTP*/ /*numOfParts*/
    {
        {
            29,
            framePtpL2MplsCwBytes,
            GT_FALSE
        }
    }/*partsInfo*/
};

static TGF_BYTES_INFO_LIST_STC    egressTags_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_UNTAGGED_E =
{
    2,/*vlan tag + the added ETag*/ /*numOfParts*/
    {
        {
            8,/*numOfBytes*/
            egressEtag/*partBytesPtr*/,
            GT_FALSE
        },
        {
            4,
            frameVlanTagExtArr,
            GT_FALSE
        }
    }/*partsInfo*/
};

static TGF_BYTES_INFO_LIST_STC    egressTags_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_EXTENDED_PORT_TO_UPSTREAM_PORT_E =
{
    1,/*vlan tag*/ /*numOfParts*/
    {
        {
            4,
            frameVlanTagArr,
            GT_FALSE
        }
    }/*partsInfo*/
};

#define GRP_MC  2

static GT_U8    ingressEtagMc[8] = {
/*byte 0*/    (GT_U8)(ETAG_TPID >> 8),
/*byte 1*/    (GT_U8)(ETAG_TPID & 0xFF),
/*byte 2*/    (GT_U8)((E_PCP_CNS << 5) | (E_DEI_CNS << 4) | (DUMMY_INGRESS_E_CID_BASE >> 8)), /* ingress_E-CID_base (4 MSBits)*/
/*byte 3*/    (GT_U8)(DUMMY_INGRESS_E_CID_BASE & 0xFF),/* ingress_E-CID_base (8 LSBits)*/
/*byte 4*/    (GT_U8)((0/*Re-*/<<6) | (GRP_MC/*GRP*/ << 4) |  (ECID_INGRESS_PORT_CNS >> 8)),
/*byte 5*/    (GT_U8)(ECID_INGRESS_PORT_CNS  & 0xFF),
/*byte 6*/    DUMMY_INGRESS_E_CID_EXT,
/*byte 7*/    DUMMY_E_CID_EXT
};

static GT_U8    egressEtag2CPU[8] = {
/*byte 0*/    (GT_U8)(ETAG_TPID >> 8),
/*byte 1*/    (GT_U8)(ETAG_TPID & 0xFF),
/*byte 2*/    0, /* ingress_E-CID_base (4 MSBits)*/
/*byte 3*/    0,/* ingress_E-CID_base (8 LSBits)*/
/*byte 4*/    (GT_U8)((0/*Re-*/<<6) | (0/*GRP*/ << 4) |  (UPLINK_PORT_NUM >> 8)),
/*byte 5*/    (GT_U8)(UPLINK_PORT_NUM  & 0xFF),
/*byte 6*/    DUMMY_INGRESS_E_CID_EXT,
/*byte 7*/    DUMMY_E_CID_EXT
};

static GT_U8    egressEtag2IPL[8] = {
/*byte 0*/    (GT_U8)(ETAG_TPID >> 8),
/*byte 1*/    (GT_U8)(ETAG_TPID & 0xFF),
/*byte 2*/    0, /* ingress_E-CID_base (4 MSBits)*/
/*byte 3*/    0,/* ingress_E-CID_base (8 LSBits)*/
/*byte 4*/    (GT_U8)((1/*direction*/<<7) | (0 /*upsstreamPort */ << 6) | (0/*GRP*/ << 4) |  (UPLINK_PORT_NUM >> 8)),
/*byte 5*/    (GT_U8)(UPLINK_PORT_NUM  & 0xFF),
/*byte 6*/    DUMMY_INGRESS_E_CID_EXT,
/*byte 7*/    DUMMY_E_CID_EXT
};

static GT_U8    egressEtagIPL2IPL[8] = {
/*byte 0*/    (GT_U8)(ETAG_TPID >> 8),
/*byte 1*/    (GT_U8)(ETAG_TPID & 0xFF),
/*byte 2*/    (GT_U8)((E_PCP_CNS << 5) | (E_DEI_CNS << 4) | (DUMMY_INGRESS_E_CID_BASE >> 8)), /* ingress_E-CID_base (4 MSBits)*/
/*byte 3*/    (GT_U8)(DUMMY_INGRESS_E_CID_BASE & 0xFF),/* ingress_E-CID_base (8 LSBits)*/
/*byte 4*/    (GT_U8)((0/*Re-*/<<6) | (GRP_MC/*GRP*/ << 4) |  (ECID_INGRESS_PORT_CNS >> 8)),
/*byte 5*/    (GT_U8)(ECID_INGRESS_PORT_CNS  & 0xFF),
/*byte 6*/    DUMMY_INGRESS_E_CID_EXT,
/*byte 7*/    DUMMY_E_CID_EXT
};


#define E_PCP3_CNS                   3
#define E_DEI1_CNS                   1

static GT_U8    egressEtagCos2CPU[8] = {
/*byte 0*/    (GT_U8)(ETAG_TPID >> 8),
/*byte 1*/    (GT_U8)(ETAG_TPID & 0xFF),
/*byte 2*/    (GT_U8)((E_PCP3_CNS << 5) | (E_DEI1_CNS << 4)),
/*byte 3*/    0,/* ingress_E-CID_base (8 LSBits)*/
/*byte 4*/    (GT_U8)((0/*Re-*/<<6) | (0/*GRP*/ << 4) |  (UPLINK_PORT_NUM >> 8)),
/*byte 5*/    (GT_U8)(UPLINK_PORT_NUM  & 0xFF),
/*byte 6*/    DUMMY_INGRESS_E_CID_EXT,
/*byte 7*/    DUMMY_E_CID_EXT
};

static GT_U8    ingressEtagUc_IPL[8] = {
/*byte 0*/    (GT_U8)(ETAG_TPID >> 8),
/*byte 1*/    (GT_U8)(ETAG_TPID & 0xFF),
/*byte 2*/    0, /* ingress_E-CID_base (4 MSBits)*/
/*byte 3*/    0,/* ingress_E-CID_base (8 LSBits)*/
/*byte 4*/    (GT_U8)((0/*direction*/<<7) | (0 /*upsstreamPort */ << 6) | (0/*GRP*/ << 4) |  (UPLINK_PORT_NUM >> 8)),
/*byte 5*/    (GT_U8)(UPLINK_PORT_NUM  & 0xFF),
/*byte 6*/    DUMMY_INGRESS_E_CID_EXT,
/*byte 7*/    DUMMY_E_CID_EXT
};

static TGF_BYTES_INFO_LIST_STC    ingressTags_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_E =
{
    2,/*ETag + vlan tag */ /*numOfParts*/
    {
        {
            8,/*numOfBytes*/
            ingressEtagMc,/*partBytesPtr*/
            GT_FALSE
        },
        {
            4,
            frameVlanTagArr,
            GT_FALSE
        }
    }/*partsInfo*/
};

static TGF_BYTES_INFO_LIST_STC    egressTags_vlanTag_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_E =
{
    1,/* vlan tag */ /*numOfParts*/
    {
        {
            4,
            frameVlanTagArr,
            GT_FALSE
        }
    }/*partsInfo*/
};

static TGF_BYTES_INFO_LIST_STC    egressTags_eTag_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CPU_PORT_E =
{
    1,/* E-tag */ /*numOfParts*/
    {
        {
            8,
            egressEtag2CPU,/*partBytesPtr*/
            GT_FALSE
        }
    }/*partsInfo*/
};

static TGF_BYTES_INFO_LIST_STC    ingressTags_etag_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_IPL_PORT_E =
{
    1,/*ETag + vlan tag */ /*numOfParts*/
    {
        {
            8,/*numOfBytes*/
            ingressEtagUc_IPL,/*partBytesPtr*/
            GT_FALSE
        }
    }/*partsInfo*/
};
static TGF_BYTES_INFO_LIST_STC    ingressTags_etag_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_IPL_PORT_TO_IPL_PORT_E =
{
    1,/*ETag */ /*numOfParts*/
    {
        {
            8,/*numOfBytes*/
            ingressEtagMc,/*partBytesPtr*/
            GT_FALSE
        }
    }/*partsInfo*/
};

static TGF_BYTES_INFO_LIST_STC    ingressTags_etag_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_MC_PORT_E =
{
    1,/*ETag */ /*numOfParts*/
    {
        {
            8,/*numOfBytes*/
            ingressEtagMc,/*partBytesPtr*/
            GT_FALSE
        }
    }/*partsInfo*/
};

static TGF_BYTES_INFO_LIST_STC    egressTags_eTag_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_IPL_PORT_E =
{
    1,/* E-tag */ /*numOfParts*/
    {
        {
            8,
            egressEtag2IPL,/*partBytesPtr*/
            GT_FALSE
        }
    }/*partsInfo*/
};

static TGF_BYTES_INFO_LIST_STC    egressTags_eTag_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_IPL_PORT_TO_IPL_PORT_E =
{
    1,/* E-tag */ /*numOfParts*/
    {
        {
            8,
            egressEtagIPL2IPL,/*partBytesPtr*/
            GT_FALSE
        }
    }/*partsInfo*/
};

static TGF_BYTES_INFO_LIST_STC    egressTags_eTag_cos_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CPU_PORT_E =
{
    1,/* E-tag */ /*numOfParts*/
    {
        {
            8,
            egressEtagCos2CPU,/*partBytesPtr*/
            GT_FALSE
        }
    }/*partsInfo*/
};

static TGF_BYTES_INFO_LIST_STC    ingressTags_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_UPSTREAM_PORT_TO_EXTENDED_PORT_E =
{
    2,/* vlan tag + original VLAN tag */ /*numOfParts*/
    {
        {
            4,
            frameEvbVlanTagArr,
            GT_FALSE
        },
        {
            4,
            frameVlanTagArr,
            GT_FALSE
        }
    }/*partsInfo*/
};

static TGF_BYTES_INFO_LIST_STC    egressTags_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_UPSTREAM_PORT_TO_EXTENDED_PORT_E =
{
    1,/* original VLAN tag */ /*numOfParts*/
    {
        {
            4,
            frameVlanTagArr,
            GT_FALSE
        }
    }/*partsInfo*/
};

static TGF_BYTES_INFO_LIST_STC    ingressTags_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_QCN_E =
{
    1,/* vlan tag */ /*numOfParts*/
    {
        {
            4,
            frameEvbVlanTagArr,
            GT_FALSE
        },
    }/*partsInfo*/
};

static TGF_BYTES_INFO_LIST_STC    egressTags_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_QCN_E =
{
    2,/* EVB VLAN + QCN tag */ /*numOfParts*/
    {
        {
            4,
            frameEvbVlanTag,
            GT_FALSE
        },
        {
            19,
            frameQcnTag,
            GT_FALSE
        }
    }/*partsInfo*/
};

#define ECID_ON_OTHER_PE_PORT_CNS       0x567
#define ECID_ON_CPU_IN_NEXT_PE_CNS      0x987
#define ECID_ON_PORT_IN_NEXT_PE_CNS     0xabc

#define ECID_ON_CPU_IN_MY_PE_CNS        0x654

static GT_U8    ingressEtagUc_onOtherPe[8] = {
/*byte 0*/    (GT_U8)(ETAG_TPID >> 8),
/*byte 1*/    (GT_U8)(ETAG_TPID & 0xFF),
/*byte 2*/    (GT_U8)((E_PCP_CNS << 5) | (E_DEI_CNS << 4) | (DUMMY_INGRESS_E_CID_BASE >> 8)), /* ingress_E-CID_base (4 MSBits)*/
/*byte 3*/    (GT_U8)(DUMMY_INGRESS_E_CID_BASE & 0xFF),/* ingress_E-CID_base (8 LSBits)*/
/*byte 4*/    (GT_U8)((0/*Re-*/<<6) | (0/*GRP*/ << 4) |  (ECID_ON_OTHER_PE_PORT_CNS >> 8)),
/*byte 5*/    (GT_U8)(ECID_ON_OTHER_PE_PORT_CNS  & 0xFF),
/*byte 6*/    DUMMY_INGRESS_E_CID_EXT,
/*byte 7*/    DUMMY_E_CID_EXT
};
static GT_U8    ingressEtagUc_onCpuNextPe[8] = {
/*byte 0*/    (GT_U8)(ETAG_TPID >> 8),
/*byte 1*/    (GT_U8)(ETAG_TPID & 0xFF),
/*byte 2*/    (GT_U8)((E_PCP_CNS << 5) | (E_DEI_CNS << 4) | (DUMMY_INGRESS_E_CID_BASE >> 8)), /* ingress_E-CID_base (4 MSBits)*/
/*byte 3*/    (GT_U8)(DUMMY_INGRESS_E_CID_BASE & 0xFF),/* ingress_E-CID_base (8 LSBits)*/
/*byte 4*/    (GT_U8)((0/*Re-*/<<6) | (0/*GRP*/ << 4) |  (ECID_ON_CPU_IN_NEXT_PE_CNS >> 8)),
/*byte 5*/    (GT_U8)(ECID_ON_CPU_IN_NEXT_PE_CNS  & 0xFF),
/*byte 6*/    DUMMY_INGRESS_E_CID_EXT,
/*byte 7*/    DUMMY_E_CID_EXT
};



static TGF_BYTES_INFO_LIST_STC    ingressTags_etagEcidOnOtherPe_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CASCADE_PORT_E =
{
    2,/*ETag + vlan tag */ /*numOfParts*/
    {
        {
            8,/*numOfBytes*/
            ingressEtagUc_onOtherPe,/*partBytesPtr*/
            GT_FALSE
        },
        {
            4,
            frameVlanTagArr,
            GT_FALSE
        }
    }/*partsInfo*/
};

static TGF_BYTES_INFO_LIST_STC    ingressTags_etagEcidOnCpuNextPe_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CASCADE_PORT_E =
{
    2,/*ETag + vlan tag */ /*numOfParts*/
    {
        {
            8,/*numOfBytes*/
            ingressEtagUc_onCpuNextPe/*partBytesPtr*/,
            GT_FALSE
        },
        {
            4,
            frameVlanTagArr,
            GT_FALSE
        }
    }/*partsInfo*/
};

static TGF_BYTES_INFO_LIST_STC    egressTags_vlanTag_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CASCADE_PORT_E =
{
    1,/* vlan tag */ /*numOfParts*/
    {
        {
            4,
            frameVlanTagArr,
            GT_FALSE
        }
    }/*partsInfo*/
};

static GT_U8    ingressEtagFromCascade[8] = {
/*byte 0*/    (GT_U8)(ETAG_TPID >> 8),
/*byte 1*/    (GT_U8)(ETAG_TPID & 0xFF),
/*byte 2*/    (GT_U8)((E_PCP_CNS << 5) | (E_DEI_CNS << 4) | (DUMMY_INGRESS_E_CID_BASE >> 8)), /* ingress_E-CID_base (4 MSBits)*/
/*byte 3*/    (GT_U8)(DUMMY_INGRESS_E_CID_BASE & 0xFF),/* ingress_E-CID_base (8 LSBits)*/
/*byte 4*/    (GT_U8)((0/*Re-*/<<6) | (0/*GRP*/ << 4) |  (ECID_ON_PORT_IN_NEXT_PE_CNS >> 8)),
/*byte 5*/    (GT_U8)(ECID_ON_PORT_IN_NEXT_PE_CNS  & 0xFF),
/*byte 6*/    DUMMY_INGRESS_E_CID_EXT,
/*byte 7*/    DUMMY_E_CID_EXT
};

static TGF_BYTES_INFO_LIST_STC    ingressTags_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_CASCADE_PORT_TO_UPSTREAM_PORT_E =
{
    2,/*ETag + vlan tag */ /*numOfParts*/
    {
        {
            8,/*numOfBytes*/
            ingressEtagFromCascade,/*partBytesPtr*/
            GT_FALSE
        },
        {
            4,
            frameVlanTagArr,
            GT_FALSE
        }
    }/*partsInfo*/
};

static GT_U8    ingressEtagFromCPU[8] = {
/*byte 0*/    (GT_U8)(ETAG_TPID >> 8),
/*byte 1*/    (GT_U8)(ETAG_TPID & 0xFF),
/*byte 2*/    (GT_U8)((E_PCP_CNS << 5) | (E_DEI_CNS << 4) | (DUMMY_INGRESS_E_CID_BASE >> 8)), /* ingress_E-CID_base (4 MSBits)*/
/*byte 3*/    (GT_U8)(DUMMY_INGRESS_E_CID_BASE & 0xFF),/* ingress_E-CID_base (8 LSBits)*/
/*byte 4*/    (GT_U8)((0/*Re-*/<<6) | (0/*GRP*/ << 4) |  (ECID_ON_CPU_IN_MY_PE_CNS >> 8)),
/*byte 5*/    (GT_U8)(ECID_ON_CPU_IN_MY_PE_CNS  & 0xFF),
/*byte 6*/    DUMMY_INGRESS_E_CID_EXT,
/*byte 7*/    DUMMY_E_CID_EXT
};

static TGF_BYTES_INFO_LIST_STC    ingressTags_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_CPU_PORT_TO_UPSTREAM_PORT_E =
{
    2,/*ETag + vlan tag */ /*numOfParts*/
    {
        {
            8,/*numOfBytes*/
            ingressEtagFromCPU,/*partBytesPtr*/
            GT_FALSE
        },
        {
            4,
            frameVlanTagArr,
            GT_FALSE
        }
    }/*partsInfo*/
};



static CPSS_PX_EGRESS_SOURCE_PORT_802_1BR_STC      _802_1br_srcPortInfo =
{
    ECID_INGRESS_PORT_CNS, /* pcid */
    {0, {0,0,0}},          /* srcFilteringVector  relevant only for ipl2ipl */
    GT_FALSE,              /* Upstreram port set only by CPU */
    {GT_FALSE, 0}
};


static CPSS_PX_EGRESS_SOURCE_PORT_802_1BR_STC      _802_1br_srcPort2CpuInfo =
{
    UPLINK_PORT_NUM,    /* pcid */
    {0, {0,0,0}},       /* srcFilteringVector  relevant only for ipl2ipl */
    GT_FALSE,           /* Upstreram port set only by CPU */
    {GT_FALSE, 0}
};

static CPSS_PX_EGRESS_SOURCE_PORT_802_1BR_STC      _802_1br_srcPortIpl2IplInfo =
{
    UPLINK_PORT_NUM,    /* pcid */
    {0x80, {0,0,0}},    /* srcFilteringVector  relevant only for ipl2ipl */
    GT_FALSE,           /* Upstreram port set only by CPU */
    {GT_FALSE, 0}
};


static CPSS_PX_EGRESS_SOURCE_PORT_802_1BR_STC      _802_1br_srcPortPvidInfo = 
{
    0,                  /* pcid */
    {0, {0,0,0}},       /* srcFilteringVector  relevant only for ipl2ipl */
    GT_FALSE,           /* Upstreram port set only by CPU */
    {GT_FALSE, 0}
};

static CPSS_PX_EGRESS_SOURCE_PORT_802_1BR_STC      _802_1br_srcPort2LagMcInfo =
{
    LAG_MC_SRC_PORT_NUM,    /* pcid */
    {0, {0,0,0}},       /* srcFilteringVector  relevant only for ipl2ipl */
    GT_FALSE,           /* Upstreram port set only by CPU */
    {GT_FALSE, 0}
};


#define ECID_TRG_PORT_CNS       0x123
#define EGRESS_DELAY_CNS        0xFEDCB/*20 bits*/

static CPSS_PX_EGRESS_TARGET_PORT_COMMON_STC  info_common_802_1br_uplink_trgPortInfo =
{
    ETAG_TPID,         /*tpid*/
    ECID_ON_CPU_IN_MY_PE_CNS,  /* pcid -- the ECID represents the CPU port on 'my' PE */
    EGRESS_DELAY_CNS   /* egressDelay */
};

static CPSS_PX_EGRESS_TARGET_PORT_COMMON_STC  info_common_802_1br_extended_port_trgPortInfo =
{
    EXTENDED_PORT_TPID_CNS, /*tpid -- used to understand egress packet hold tag/not */
    ECID_TRG_PORT_CNS, /*pcid*/
    EGRESS_DELAY_CNS   /*egressDelay*/
};

static CPSS_PX_EGRESS_TARGET_PORT_COMMON_STC  info_common_802_1br_cascade_port_trgPortInfo =
{
    0xffff,  /*tpid : -- dummy -- not used by cascade port */
    ECID_ON_CPU_IN_NEXT_PE_CNS, /*pcid -- the ECID represents the CPU port on 'next' PE */
    EGRESS_DELAY_CNS   /*egressDelay*/
};

static CPSS_PX_EGRESS_TARGET_PORT_COMMON_STC  info_common_802_1br_CPU_trgPortInfo =
{
    ETAG_TPID,         /*tpid*/
    UPLINK_PORT_NUM,   /* pcid -- of source port */
    EGRESS_DELAY_CNS   /* egressDelay */
};

static CPSS_PX_EGRESS_TARGET_PORT_COMMON_STC  info_common_802_1br_IPL_trgPortInfo =
{
    ETAG_TPID,         /*tpid*/
    IPL_PORT_NUM,   /* pcid -- of source port */
    EGRESS_DELAY_CNS   /* egressDelay */
};

static CPSS_PX_EGRESS_TARGET_PORT_COMMON_STC  info_common_802_1br_LAG_MC_trgPortInfo =
{
    ETAG_TPID,                /*tpid*/
    LAG_MC_SRC_PORT_NUM,      /* pcid -- of target port */
    EGRESS_DELAY_CNS          /* egressDelay */
};



typedef struct{
    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT  haAction;
    GT_U32                                     extParam;
    TGF_BYTES_INFO_LIST_STC                      *ingressTagsPtr;
    TGF_BYTES_INFO_LIST_STC                      *egressTagsPtr;

    CPSS_PX_EGRESS_SOURCE_PORT_802_1BR_STC      *_802_1br_srcPortInfoPtr;
    CPSS_PX_EGRESS_TARGET_PORT_COMMON_STC       *info_common_trgPortInfoPtr;
}HA_ACTION_INFO;

static HA_ACTION_INFO ha_action_info[] =
{
    {CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_E
     ,EXT_PARAM_DEFAULT_CNS
     ,&ingressTags_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_E /*ingressTagsPtr*/
     ,&egressTags_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_E  /*egressTagsPtr*/
     ,&_802_1br_srcPortInfo /*_802_1br_srcPortInfoPtr*/
     ,&info_common_802_1br_uplink_trgPortInfo  /*info_common_trgPortInfoPtr*/
    }

    ,{CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_E
     ,EXT_PARAM_802_1BR_U2E_EGRESS_TAGGED_CNS |
      EXT_PARAM_802_1BR_U2E_EGRESS_UNTAGGED_TPID_NO_MATCH_CNS
     ,&ingressTags_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_E        /*ingressTagsPtr*/
     ,&egressTags_vlanTag_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_E /*egressTagsPtr*/
     ,NULL  /*_802_1br_srcPortInfoPtr*/
     ,&info_common_802_1br_extended_port_trgPortInfo  /*info_common_trgPortInfoPtr*/
     }

    ,{CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_E
     ,EXT_PARAM_802_1BR_U2E_EGRESS_UNTAGGED_CNS |
      EXT_PARAM_802_1BR_U2E_EGRESS_ALL_UNTAGGED_CNS
     ,&ingressTags_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_E /*ingressTagsPtr*/
     ,NULL  /*egressTagsPtr*/
     ,NULL  /*_802_1br_srcPortInfoPtr*/
     ,&info_common_802_1br_extended_port_trgPortInfo  /*info_common_trgPortInfoPtr*/
     }

    ,{CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CASCADE_PORT_E
     ,EXT_PARAM_802_1BR_U2C_EGRESS_KEEP_ETAG_CNS
     ,&ingressTags_etagEcidOnOtherPe_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CASCADE_PORT_E /*ingressTagsPtr*/
     ,&ingressTags_etagEcidOnOtherPe_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CASCADE_PORT_E /*egressTagsPtr*/ /* same as ingress !!! */
     ,NULL  /*_802_1br_srcPortInfoPtr*/
     ,&info_common_802_1br_cascade_port_trgPortInfo  /*info_common_trgPortInfoPtr*/
     }

    ,{CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CASCADE_PORT_E
     ,EXT_PARAM_802_1BR_U2C_EGRESS_POP_ETAG_CNS
     ,&ingressTags_etagEcidOnCpuNextPe_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CASCADE_PORT_E /*ingressTagsPtr*/
     ,&egressTags_vlanTag_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CASCADE_PORT_E /*egressTagsPtr*/ /* lose the ETag */
     ,NULL  /*_802_1br_srcPortInfoPtr*/
     ,&info_common_802_1br_cascade_port_trgPortInfo  /*info_common_trgPortInfoPtr*/
     }

    ,{CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_CASCADE_PORT_TO_UPSTREAM_PORT_E
     ,EXT_PARAM_DEFAULT_CNS
     ,&ingressTags_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_CASCADE_PORT_TO_UPSTREAM_PORT_E /*ingressTagsPtr*/
     ,&ingressTags_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_CASCADE_PORT_TO_UPSTREAM_PORT_E /*ingressTagsPtr*/ /* same as ingress !!! */
     ,NULL  /*_802_1br_srcPortInfoPtr*/
     ,&info_common_802_1br_uplink_trgPortInfo  /*info_common_trgPortInfoPtr*/
     }

    ,{CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CPU_PORT_E
     ,EXT_PARAM_DEFAULT_CNS
     ,NULL /*ingressTagsPtr*/
     ,&egressTags_eTag_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CPU_PORT_E  /*egressTagsPtr*/
     ,&_802_1br_srcPort2CpuInfo /*_802_1br_srcPortInfoPtr*/
     ,&info_common_802_1br_CPU_trgPortInfo /*info_common_trgPortInfoPtr*/
     }

    ,{CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_CPU_PORT_TO_UPSTREAM_PORT_E
     ,EXT_PARAM_802_1BR_CPU2U_EGRESS_NO_ETAG_CNS
     ,NULL /*ingressTagsPtr*/
     ,NULL /*ingressTagsPtr*/ /* same as ingress !!! */
     ,NULL  /*_802_1br_srcPortInfoPtr*/
     ,&info_common_802_1br_uplink_trgPortInfo  /*info_common_trgPortInfoPtr*/
     }

    ,{CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_CPU_PORT_TO_UPSTREAM_PORT_E
     ,EXT_PARAM_802_1BR_CPU2U_EGRESS_WITH_ETAG_CNS
     ,&ingressTags_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_CPU_PORT_TO_UPSTREAM_PORT_E /*ingressTagsPtr*/
     ,&ingressTags_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_CPU_PORT_TO_UPSTREAM_PORT_E /*ingressTagsPtr*/ /* same as ingress !!! */
     ,NULL  /*_802_1br_srcPortInfoPtr*/
     ,&info_common_802_1br_uplink_trgPortInfo  /*info_common_trgPortInfoPtr*/
     }

    ,{CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DO_NOT_MODIFY_E
     ,EXT_PARAM_DEFAULT_CNS
     ,NULL /*ingressTagsPtr*/
     ,NULL /*ingressTagsPtr*/ /* same as ingress !!! */
     ,NULL /*_802_1br_srcPortInfoPtr*/
     ,NULL /*info_common_trgPortInfoPtr*/
     }

    ,{CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CPU_PORT_E
     ,EXT_PARAM_802_1BR_U2C_EGRESS_MODIFIED_COS_ETAG_CNS
     ,NULL /*ingressTagsPtr*/
     ,&egressTags_eTag_cos_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CPU_PORT_E  /*egressTagsPtr*/
     ,&_802_1br_srcPort2CpuInfo /*_802_1br_srcPortInfoPtr*/
     ,&info_common_802_1br_CPU_trgPortInfo /*info_common_trgPortInfoPtr*/
     }
    ,{CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_IPL_PORT_E
     ,EXT_PARAM_DEFAULT_CNS
     ,&ingressTags_etag_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_IPL_PORT_E /*ingressTagsPtr*/
     ,&egressTags_eTag_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_IPL_PORT_E  /*egressTagsPtr*/
     ,NULL /*_802_1br_srcPortInfoPtr*/
     ,&info_common_802_1br_IPL_trgPortInfo /*info_common_trgPortInfoPtr*/
     }

    ,{CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_IPL_PORT_TO_IPL_PORT_E
     ,EXT_PARAM_DEFAULT_CNS
     ,&ingressTags_etag_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_IPL_PORT_TO_IPL_PORT_E /*ingressTagsPtr*/
     ,&egressTags_eTag_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_IPL_PORT_TO_IPL_PORT_E  /*egressTagsPtr*/
     ,&_802_1br_srcPortIpl2IplInfo/* srcPortInfoPtr*/
     ,&info_common_802_1br_IPL_trgPortInfo /*info_common_trgPortInfoPtr*/
     }
    ,{CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_M4_PORT_E
     ,0xFF
     ,&ingressTags_etag_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_MC_PORT_E /*ingressTagsPtr*/
     ,NULL /*egressTagsPtr*/
     ,&_802_1br_srcPort2LagMcInfo/* srcPortInfoPtr*/
     ,&info_common_802_1br_LAG_MC_trgPortInfo /*info_common_trgPortInfoPtr*/
     }
    ,{CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_M8_PORT_E
     ,0xFF
     ,&ingressTags_etag_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_MC_PORT_E /*ingressTagsPtr*/
     ,NULL /*egressTagsPtr*/
     ,&_802_1br_srcPort2LagMcInfo/* srcPortInfoPtr*/
     ,&info_common_802_1br_LAG_MC_trgPortInfo /*info_common_trgPortInfoPtr*/
     }

    ,{CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_UNTAGGED_E
     ,EXT_PARAM_DEFAULT_CNS
     ,NULL /*ingressTagsPtr*/
     ,&egressTags_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_E  /*egressTagsPtr*/
     ,&_802_1br_srcPortPvidInfo /*_802_1br_srcPortPvidInfoPtr*/
     ,&info_common_802_1br_uplink_trgPortInfo  /*info_common_trgPortInfoPtr*/
    }

    ,{CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_UNTAGGED_E
     ,EXT_PARAM_802_1BR_E2U_EGRESS_PVID_CNS
     ,NULL /*ingressTagsPtr*/
     ,&egressTags_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_UNTAGGED_E  /*egressTagsPtr*/
     ,&_802_1br_srcPortPvidInfo /*_802_1br_srcPortPvidInfoPtr*/
     ,&info_common_802_1br_uplink_trgPortInfo  /*info_common_trgPortInfoPtr*/
    }

    ,{CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DROP_PACKET_E
     ,EXT_PARAM_DEFAULT_CNS
     ,NULL /*ingressTagsPtr*/
     ,NULL /*ingressTagsPtr*/ /* same as ingress !!! */
     ,NULL /*_802_1br_srcPortInfoPtr*/
     ,NULL /*info_common_trgPortInfoPtr*/
     }

    ,{CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_E2U_PACKET_E
     ,EXT_PARAM_DEFAULT_CNS
     ,NULL /*ingressTagsPtr*/
     ,&egressTags_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_EXTENDED_PORT_TO_UPSTREAM_PORT_E  /*egressTagsPtr*/
     ,NULL /*_802_1br_srcPortInfoPtr*/
     ,NULL  /*info_common_trgPortInfoPtr*/
    }

    ,{CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_U2E_PACKET_E
     ,EXT_PARAM_DEFAULT_CNS
     ,&ingressTags_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_UPSTREAM_PORT_TO_EXTENDED_PORT_E   /*ingressTagsPtr*/
     ,&egressTags_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_UPSTREAM_PORT_TO_EXTENDED_PORT_E    /*egressTagsPtr*/
     ,NULL  /*_802_1br_srcPortInfoPtr*/
     ,NULL  /*info_common_trgPortInfoPtr*/
     }

    ,{CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_E
    ,EXT_PARAM_DEFAULT_PTP_HEADER_CNS | EXT_PARAM_DEFAULT_PTP_HEADER_NO_TS_CHECK_CNS
    ,&ingressL2PtpTags_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_E /*ingressTagsPtr*/
    ,&egressL2PtpTags_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_E  /*egressTagsPtr*/
    ,&_802_1br_srcPortInfo /*_802_1br_srcPortInfoPtr*/
    ,&info_common_802_1br_uplink_trgPortInfo  /*info_common_trgPortInfoPtr*/
    }

    ,{CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_QCN_E
     ,EXT_PARAM_DEFAULT_CNS
     ,&ingressTags_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_QCN_E                              /*ingressTagsPtr*/
     ,&egressTags_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_QCN_E                               /*egressTagsPtr*/
     ,NULL  /*_802_1br_srcPortInfoPtr*/
     ,NULL  /*info_common_trgPortInfoPtr*/
     }

    ,{CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_PRE_DA_EXTENDED_PORT_TO_UPSTREAM_PORT_E
     ,EXT_PARAM_DEFAULT_CNS
     ,NULL  /*ingressTagsPtr*/
     ,&egressTags_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_PRE_DA_EXTENDED_TO_UPSTREAM_PORT_E   /*egressTagsPtr*/
     ,NULL  /*_802_1br_srcPortInfoPtr*/
     ,NULL  /*info_common_trgPortInfoPtr*/
    }

    ,{CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_PRE_DA_UPSTREAM_PORT_TO_EXTENDED_PORT_E
     ,1 << EXTENDED_PRE_MAC_DA_PORT1_NUM
     ,&ingressTags_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_PRE_DA1_UPSTREAM_TO_EXTENDED_PORT_E  /*ingressTagsPtr*/
     ,NULL  /*egressTagsPtr*/
     ,NULL  /*_802_1br_srcPortInfoPtr*/
     ,NULL  /*info_common_trgPortInfoPtr*/
    }

    ,{CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_PRE_DA_UPSTREAM_PORT_TO_EXTENDED_PORT_E
     ,1 << EXTENDED_PRE_MAC_DA_PORT2_NUM
     ,&ingressTags_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_PRE_DA2_UPSTREAM_TO_EXTENDED_PORT_E /*ingressTagsPtr*/
     ,NULL  /*egressTagsPtr*/
     ,NULL  /*_802_1br_srcPortInfoPtr*/
     ,NULL  /*info_common_trgPortInfoPtr*/
    }

    ,{CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_PRE_DA_EXTENDED_PORT_TO_UPSTREAM_PORT_E
     ,EXT_PARAM_MPLS_PTP_HEADER_CNS
     ,&ingressL2PtpTags_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_PRE_DA_EXTENDED_PORT_TO_UPSTREAM_PORT_E /*ingressTagsPtr*/
     ,&egressL2PtpTags_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_PRE_DA_EXTENDED_TO_UPSTREAM_PORT_E   /*egressTagsPtr*/
     ,NULL  /*_802_1br_srcPortInfoPtr*/
     ,NULL  /*info_common_trgPortInfoPtr*/
    }

    ,{CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_PRE_DA_UPSTREAM_PORT_TO_EXTENDED_PORT_E
     ,EXT_PARAM_MPLS_PTP_HEADER_CNS
     ,&ingressL2PtpTags_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_PRE_DA1_UPSTREAM_TO_EXTENDED_PORT_E  /*ingressTagsPtr*/
     ,&egressL2PtpTags_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_PRE_DA1_UPSTREAM_TO_EXTENDED_PORT_E  /*egressTagsPtr*/
     ,NULL  /*_802_1br_srcPortInfoPtr*/
     ,NULL  /*info_common_trgPortInfoPtr*/
    }

    ,{CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_PRE_DA_EXTENDED_PORT_TO_UPSTREAM_PORT_E
     ,EXT_PARAM_MPLS_PTP_IPV6_HEADER_CNS
     ,NULL /*ingressTagsPtr*/
     ,&egressPreDaTag_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_PRE_DA_EXTENDED_TO_UPSTREAM_PORT_E   /*egressTagsPtr*/
     ,NULL  /*_802_1br_srcPortInfoPtr*/
     ,NULL  /*info_common_trgPortInfoPtr*/
    }

    ,{CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_CASCADE_PORT_TO_UPSTREAM_PORT_E
     ,EXT_PARAM_PTP_TS1_CF_CNS | EXT_PARAM_PTP_TS2_CF_CNS
     ,NULL /*ingressTagsPtr*/
     ,NULL /*ingressTagsPtr*/ /* same as ingress !!! */
     ,NULL /*_802_1br_srcPortInfoPtr*/
     ,&info_common_802_1br_uplink_trgPortInfo  /*info_common_trgPortInfoPtr*/
     }

    /*must be last*/
    ,{CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE___MUST_BE_LAST___E
     ,EXT_PARAM_DEFAULT_CNS
     ,NULL  /*ingressTagsPtr*/
     ,NULL  /*egressTagsPtr*/
     ,NULL  /*_802_1br_srcPortInfoPtr*/
     ,NULL  /*info_common_trgPortInfoPtr*/
    }
};

static GT_U8 frameArr[] = {
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,          /*mac da FF:FF:FF:FF:FF:FF */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x01,          /*mac sa 00:00:00:00:00:01 */
      0x55, 0x55,                                  /*ethertype                */
      0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, /*payload  */
      0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
      0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23,
      0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
      0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B,
      0x3C, 0x3D, 0x3E, 0x3F};

static CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT   orig_haPerationType;
static CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT  orig_haPerationInfo;
static CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT   orig_sourcePortType;
static CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT        orig_sourcePortInfo;
static CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT   orig_targetPortType;
static CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT        orig_targetPortInfo;
static GT_U32                                      orig_targetPortsBmpEnabled = 0;

/* save configuration to allow restore */
static void saveConfig(
    IN GT_SW_DEV_NUM     devNum,
    CPSS_PX_PACKET_TYPE  packetType,
    GT_PHYSICAL_PORT_NUM sourcePortNum,
    GT_PHYSICAL_PORT_NUM targetPortNum
)
{
    GT_STATUS   st;
    GT_U32      ii;
    GT_BOOL     enabled;

    st = cpssPxEgressHeaderAlterationEntryGet(devNum,targetPortNum,packetType,
            &orig_haPerationType,&orig_haPerationInfo);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    st = cpssPxEgressSourcePortEntryGet(devNum,sourcePortNum,
        &orig_sourcePortType,&orig_sourcePortInfo);
    if(st != GT_NOT_INITIALIZED)
    {
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
    }
    else
    {
        orig_sourcePortType = CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_NOT_USED_E;
        orig_sourcePortInfo.ptpPortInfo.ptpPortMode = CPSS_PX_EGRESS_SOURCE_PORT_PTP_MODE_TC_E;
        orig_sourcePortInfo.ptpPortInfo.ptpOverMplsEn = GT_FALSE;
    }

    st = cpssPxEgressTargetPortEntryGet(devNum,targetPortNum,
        &orig_targetPortType,&orig_targetPortInfo);
    if(st != GT_NOT_INITIALIZED)
    {
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
    }
    else
    {
        orig_targetPortType = CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_DO_NOT_MODIFY_E;
        orig_targetPortInfo.notNeeded = 0;
    }

    orig_targetPortsBmpEnabled = 0;
    for(ii = 0 ; ii < PRV_CPSS_PX_PORTS_NUM_CNS; ii++)
    {
        if(GT_OK != prvTgfPxSkipPortWithNoMac(devNum,ii))
        {
            continue;
        }
        st = cpssPxIngressPortTargetEnableGet(devNum,ii,&enabled);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
        if(enabled == GT_TRUE)
        {
            orig_targetPortsBmpEnabled |= 1 << ii;
        }
    }

}

/* restore configuration */
static void restoreConfig(
    IN GT_SW_DEV_NUM     devNum,
    CPSS_PX_PACKET_TYPE  packetType,
    GT_PHYSICAL_PORT_NUM sourcePortNum,
    GT_PHYSICAL_PORT_NUM targetPortNum
)
{
    GT_STATUS   st;
    GT_BOOL     enable;
    GT_U32      ii;

    st = cpssPxEgressHeaderAlterationEntrySet(devNum,targetPortNum,packetType,
            orig_haPerationType,&orig_haPerationInfo);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    st = cpssPxEgressSourcePortEntrySet(devNum,sourcePortNum,
        orig_sourcePortType,&orig_sourcePortInfo);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    st = cpssPxEgressTargetPortEntrySet(devNum,targetPortNum,
        orig_targetPortType,&orig_targetPortInfo);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    for(ii = 0 ; ii < PRV_CPSS_PX_PORTS_NUM_CNS; ii++)
    {
        if(GT_OK != prvTgfPxSkipPortWithNoMac(devNum,ii))
        {
            continue;
        }

        enable = BIT2BOOL_MAC(((orig_targetPortsBmpEnabled>>ii)&1));

        st = cpssPxIngressPortTargetEnableSet(devNum,ii,enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
    }

        /* Set default VLAN tag state entry for target ports */
    st = cpssPxEgressVlanTagStateEntrySet(devNum, EXTENDED_PORT_PVID_CNS, 0);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

    return;
}

/* disable all ports. (we skip CPU port)
    those that need traffic will be enabled by the engine.
    main purpose : to reduce replications in the simulation LOG.

    anyway we test only single egress port at a test.
*/
static void disabledAllPorts(
    IN GT_SW_DEV_NUM     devNum
)
{
    GT_STATUS   st;
    GT_U32      ii;

    for(ii = 0 ; ii < PRV_CPSS_PX_PORTS_NUM_CNS; ii++)
    {
        if(GT_OK != prvTgfPxSkipPortWithNoMac(devNum,ii))
        {
            continue;
        }

        st = cpssPxIngressPortTargetEnableSet(devNum,ii,GT_FALSE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
    }

    return;
}

/* Generic egress processing function based on HA action.
   Prepare all needed information for egress processing:
       - lookup in database
       - build ingress and egress packet information
       - send packet to destination port
       - check the egress results                           */
GT_STATUS prvTgfPxEgressProcess
(
    IN GT_SW_DEV_NUM        devNum,
    IN CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT  haAction,
    IN GT_U32               extParam,
    IN CPSS_PX_PACKET_TYPE  packetType,
    IN GT_PHYSICAL_PORT_NUM sourcePortNum,
    IN GT_PHYSICAL_PORT_NUM targetPortNum,
    IN GT_U8                *frameArrPtr,
    IN GT_U32               frameLength
)
{
    GT_STATUS           rc;
    GT_U32              ii, j;
    GT_U32              numOfEgressPortsInfo;
    TGF_EXPECTED_EGRESS_INFO_SIMPLE_LIST_STC simpleList_egressPortsArr[16];
    TGF_BYTES_INFO_LIST_STC ingressFrame;
    TGF_BYTES_INFO_LIST_STC *egressFramePtr;
    HA_ACTION_INFO      *haInfoPtr;
    CPSS_PX_PTP_TSU_CONTROL_STC                 control = {GT_TRUE};
    CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT  haPerationInfo;
    CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT   sourceInfoType;
    CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT        sourcePortInfo;
    CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT   targetInfoType;
    CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT        targetPortInfo;
    TGF_BYTES_INFO_LIST_STC                     *temp_egressTagsPtr;
    CPSS_PX_PORTS_BMP                           portsTagging;
    GT_U32 sourceFilter;
    GT_U32                                      nonCompareEvbQcndBytesArr[3] = {19, 27, 0};
    GT_U32                                      partIdx;
    GT_BOOL                                     ptpTsCheck = GT_FALSE;
    #define COMPARE_EVB_QCN_PACKET_LEN          34

    cpssOsMemSet(simpleList_egressPortsArr, 0, sizeof(simpleList_egressPortsArr));
    cpssOsMemSet(&ingressFrame, 0, sizeof(ingressFrame));
    cpssOsMemSet(&sourcePortInfo, 0, sizeof(sourcePortInfo));

    if (frameArrPtr == NULL)
    {
        frameLength = sizeof(frameArr);
        frameArrPtr = frameArr;
    }

    /* look for our case in the table */
    for(ii = 0 ;
        ha_action_info[ii].haAction != CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE___MUST_BE_LAST___E ;
        ii++)
    {
        if((ha_action_info[ii].haAction == haAction) &&
           (ha_action_info[ii].extParam & extParam))
        {
            break;
        }
    }

    if(ha_action_info[ii].haAction == CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE___MUST_BE_LAST___E)
    {
        /*case not found*/
        return GT_NOT_SUPPORTED;
    }

    haInfoPtr = &ha_action_info[ii];

    partIdx = 0;
    if(haInfoPtr->ingressTagsPtr)
    {
        if (haInfoPtr->ingressTagsPtr->partsInfo[partIdx].numOfBytes &&
            haInfoPtr->ingressTagsPtr->partsInfo[partIdx].preMacDaPart == GT_TRUE)
        {
            ingressFrame.partsInfo[partIdx] = haInfoPtr->ingressTagsPtr->partsInfo[partIdx];
            partIdx++;
        }
    }
    /**************************/
    /* prepare ingress packet */
    /**************************/
    ingressFrame.partsInfo[partIdx].numOfBytes   = TGF_L2_HEADER_SIZE_CNS;/*mac DA,SA*/
    ingressFrame.partsInfo[partIdx++].partBytesPtr = frameArrPtr;

    if(haInfoPtr->ingressTagsPtr)
    {
        if(haInfoPtr->ingressTagsPtr->numOfParts >= 1)
        {
            if (haInfoPtr->ingressTagsPtr->partsInfo[partIdx - 1].numOfBytes)
            {
                if (haInfoPtr->ingressTagsPtr->partsInfo[partIdx - 1].preMacDaPart == GT_TRUE)
                {
                    return GT_BAD_STATE;
                }
                ingressFrame.partsInfo[partIdx] = haInfoPtr->ingressTagsPtr->partsInfo[partIdx - 1];
                partIdx++;
            }
        }

        if(haInfoPtr->ingressTagsPtr->numOfParts == 2)
        {
            if (haInfoPtr->ingressTagsPtr->partsInfo[partIdx - 1].numOfBytes)
            {
                if (haInfoPtr->ingressTagsPtr->partsInfo[partIdx - 1].preMacDaPart == GT_TRUE)
                {
                    return GT_BAD_STATE;
                }
                ingressFrame.partsInfo[partIdx] = haInfoPtr->ingressTagsPtr->partsInfo[partIdx - 1];
                partIdx++;
            }
        }

        if(haInfoPtr->ingressTagsPtr->numOfParts > 2)
        {
            /*case not supported*/
            return GT_NOT_SUPPORTED;
        }
    }

    ingressFrame.partsInfo[partIdx].numOfBytes   = frameLength - TGF_L2_HEADER_SIZE_CNS;
    ingressFrame.partsInfo[partIdx++].partBytesPtr = &frameArrPtr[TGF_L2_HEADER_SIZE_CNS];

    ingressFrame.numOfParts = partIdx;

    /*************************/
    /* prepare egress packet */
    /*************************/
    if (targetPortNum & MULTICAST_TARGET_PORT)
    {
        numOfEgressPortsInfo = (targetPortNum & MULTICAST_TARGET_PORT) >> 16;
        j = 0;
        for (ii= 0; ii < 16; ii++)
        {
            if ((targetPortNum >> ii ) & 1)
            {
                simpleList_egressPortsArr[j].portNum = ii;
                j++;
            }
        }
        numOfEgressPortsInfo = j;

    }
    else
    {
        numOfEgressPortsInfo = 1;
        simpleList_egressPortsArr[0].portNum = targetPortNum;
    }


    for (j= 0; j < numOfEgressPortsInfo; j++)
    {
        egressFramePtr = &simpleList_egressPortsArr[j].egressFrame;
        partIdx = 0;
        if(haInfoPtr->egressTagsPtr)
        {
            if (haInfoPtr->egressTagsPtr->partsInfo[partIdx].numOfBytes &&
                haInfoPtr->egressTagsPtr->partsInfo[partIdx].preMacDaPart == GT_TRUE)
            {
                egressFramePtr->partsInfo[partIdx] = haInfoPtr->egressTagsPtr->partsInfo[partIdx];
                partIdx++;
            }
        }

        egressFramePtr->partsInfo[partIdx].numOfBytes   = TGF_L2_HEADER_SIZE_CNS;/*mac DA,SA*/
        egressFramePtr->partsInfo[partIdx++].partBytesPtr = frameArrPtr;

        if(phaBypassEnable == GT_TRUE)
        {
            /* The PHA is bypassed .. so no packet modification expected ! */
            /* so use the same info as for the ingress packet !            */
            temp_egressTagsPtr = haInfoPtr->ingressTagsPtr;
        }
        else
        {
            temp_egressTagsPtr = haInfoPtr->egressTagsPtr;
        }

        if(temp_egressTagsPtr)
        {
            if(temp_egressTagsPtr->numOfParts >= 1)
            {
                if (temp_egressTagsPtr->partsInfo[partIdx - 1].numOfBytes)
                {
                    if (temp_egressTagsPtr->partsInfo[partIdx - 1].preMacDaPart == GT_TRUE)
                    {
                        return GT_BAD_STATE;
                    }
                    egressFramePtr->partsInfo[partIdx] = temp_egressTagsPtr->partsInfo[partIdx - 1];
                    partIdx++;
                }
            }

            if(temp_egressTagsPtr->numOfParts == 2)
            {
                if (temp_egressTagsPtr->partsInfo[partIdx - 1].numOfBytes)
                {
                    if (temp_egressTagsPtr->partsInfo[partIdx - 1].preMacDaPart == GT_TRUE)
                    {
                        return GT_BAD_STATE;
                    }
                    egressFramePtr->partsInfo[partIdx] = temp_egressTagsPtr->partsInfo[partIdx - 1];
                    partIdx++;
                }
            }

            if(temp_egressTagsPtr->numOfParts > 2)
            {
                /*case not supported*/
                return GT_NOT_SUPPORTED;
            }
        }

        /* the ingress packet hold 'hidden CRC bytes' */
        egressFramePtr->partsInfo[partIdx].numOfBytes   = frameLength - TGF_L2_HEADER_SIZE_CNS + TGF_CRC_LEN_CNS;
        egressFramePtr->partsInfo[partIdx++].partBytesPtr = &frameArrPtr[TGF_L2_HEADER_SIZE_CNS];

        egressFramePtr->numOfParts = partIdx;
    }


    /*************************/
    /* prepare egress tables */
    /*************************/
    haPerationInfo.notNeeded = 0;
    sourceInfoType = CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_NOT_USED_E;
    targetInfoType = CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_COMMON_E;
    sourcePortInfo.ptpPortInfo.ptpPortMode = CPSS_PX_EGRESS_SOURCE_PORT_PTP_MODE_TC_E;
    sourcePortInfo.ptpPortInfo.ptpOverMplsEn = GT_FALSE;
    targetPortInfo.notNeeded = 0;


    if(haInfoPtr->_802_1br_srcPortInfoPtr)
    {
        sourceInfoType = CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_802_1BR_E;
        sourcePortInfo.info_802_1br = *haInfoPtr->_802_1br_srcPortInfoPtr;

        sourceFilter = sourcePortInfo.info_802_1br.srcPortInfo.srcFilteringVector;
        /* check filters*/
        for (j= 0; j < numOfEgressPortsInfo; j++)
        {
            if((sourceFilter & (1 << simpleList_egressPortsArr[j].portNum)) ||
               ((((haAction == CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_M4_PORT_E) ||
                (haAction == CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_M8_PORT_E)) &&
                (extParam & SRC_FILTERING_TEST_ECID) && (j==0))))
            {
                egressFramePtr = &simpleList_egressPortsArr[j].egressFrame;
                egressFramePtr->numOfParts = 1;
                egressFramePtr->partsInfo[0].partBytesPtr = NULL;
                egressFramePtr->partsInfo[0].numOfBytes = 0;
            }

        }
    }

    if(haInfoPtr->info_common_trgPortInfoPtr)
    {
        targetPortInfo.info_common = *haInfoPtr->info_common_trgPortInfoPtr;
    }

    switch(haAction)
    {
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_E:
            PRV_TGF_LOG2_MAC("Ingress packet from 'Extended Port' [%d] --> expected to : egress at 'Uplink Port' [%d] with added 8 bytes ETAG \n",
                sourcePortNum,
                targetPortNum);
            haPerationInfo.info_802_1br_E2U.eTag = egressEtagInfo;
            sourceInfoType = CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_802_1BR_E;

            if (extParam & EXT_PARAM_DEFAULT_PTP_HEADER_NO_TS_CHECK_CNS)
            {
                PRV_TGF_LOG2_MAC("Ingress packet from 'Extended Port' [%d] --> expected to : egress at 'Uplink Port' [%d] with updated timestamp - no TS check\n",
                    sourcePortNum,
                    targetPortNum);

                simpleList_egressPortsArr[0].byteNumMaskListPtr = ptpTimestampByteNumMaskList;
                simpleList_egressPortsArr[0].byteNumMaskSize = sizeof(ptpTimestampByteNumMaskList)/sizeof(ptpTimestampByteNumMaskList[0]);
                ptpTsCheck = GT_TRUE;
            }
            else if (extParam & EXT_PARAM_DEFAULT_PTP_HEADER_CNS)
            {
                PRV_TGF_LOG2_MAC("Ingress packet from 'Extended Port' [%d] --> expected to : egress at 'Uplink Port' [%d] with updated timestamp - TS check\n",
                    sourcePortNum,
                    targetPortNum);

                ptpTsCheck = GT_TRUE;
            }

            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_E:


            if(extParam == EXT_PARAM_802_1BR_U2E_EGRESS_UNTAGGED_CNS)
            {
                PRV_TGF_LOG2_MAC("Ingress packet from 'Uplink Port' [%d] --> expected to : egress at 'Extended Port' [%d] with removed 8 bytes ETAG and removed 4 bytes VLAN TAG \n",
                    sourcePortNum,
                    targetPortNum);

                portsTagging = 0;
            }
            else
            if(extParam == EXT_PARAM_802_1BR_U2E_EGRESS_TAGGED_CNS)
            {
                PRV_TGF_LOG2_MAC("Ingress packet from 'Uplink Port' [%d] --> expected to : egress at 'Extended Port' [%d] with removed 8 bytes ETAG \n",
                    sourcePortNum,
                    targetPortNum);

                portsTagging = 1 << targetPortNum;
            }
            else
            if(extParam == EXT_PARAM_802_1BR_U2E_EGRESS_ALL_UNTAGGED_CNS)
            {
                PRV_TGF_LOG2_MAC("Ingress packet from 'Uplink Port' [%d] --> expected to : (ALL that match TPID) egress at 'Extended Port' [%d] with removed 8 bytes ETAG and removed 4 bytes VLAN TAG \n",
                    sourcePortNum,
                    targetPortNum);

                targetPortInfo.info_common.tpid = EXTENDED_PORT_TPID_CNS;/* TPID to cause match */
                portsTagging = 0;
            }
            else
            if(extParam == EXT_PARAM_802_1BR_U2E_EGRESS_UNTAGGED_TPID_NO_MATCH_CNS)
            {
                PRV_TGF_LOG2_MAC("Ingress packet from 'Uplink Port' [%d] --> expected to : egress at 'Extended Port' [%d] with removed 8 bytes ETAG (keep 4 bytes VLAN TAG because not recognized as vlan tag) \n",
                    sourcePortNum,
                    targetPortNum);

                targetPortInfo.info_common.tpid = EXTENDED_PORT_TPID_CNS + 1;/* TPID to cause no match */
                portsTagging = 0;
            }
            else
            {
                /*case not supported*/
                return GT_NOT_SUPPORTED;
            }
            /* Set VLAN tag state entry for target ports */
            rc = cpssPxEgressVlanTagStateEntrySet(devNum, EXTENDED_PORT_PVID_CNS, portsTagging);
            if(rc != GT_OK)
            {
                return rc;
            }

            break;

        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CASCADE_PORT_E:
            if(extParam == EXT_PARAM_802_1BR_U2C_EGRESS_KEEP_ETAG_CNS)
            {
                PRV_TGF_LOG2_MAC("Ingress packet from 'Uplink Port' [%d] --> expected to : egress at 'Cascade Port' [%d] (as ingress) --> not pop ETAG \n",
                    sourcePortNum,
                    targetPortNum);
            }
            else
            if(extParam == EXT_PARAM_802_1BR_U2C_EGRESS_POP_ETAG_CNS)
            {
                PRV_TGF_LOG2_MAC("Ingress packet from 'Uplink Port' [%d] --> expected to : egress at 'Cascade Port' [%d] with removed 8 bytes ETAG (target to CPU on next PE) \n",
                    sourcePortNum,
                    targetPortNum);
            }
            else
            {
                /*case not supported*/
                return GT_NOT_SUPPORTED;
            }

            break;

        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_CASCADE_PORT_TO_UPSTREAM_PORT_E:
            PRV_TGF_LOG2_MAC("Ingress packet from 'Cascade Port' [%d] --> expected to : egress at 'Uplink Port' [%d] (as ingress) --> not add/pop ETAG \n",
                sourcePortNum,
                targetPortNum);

            if (extParam & EXT_PARAM_PTP_TS1_CF_CNS)
            {
                PRV_TGF_LOG3_MAC("Ingress packet from 'Extended Port' [%d] --> expected to : egress at 'Uplink Port' [%d], PTP parsing mode [%d] with NOT updated timestamp 1 in CF - TS check\n",
                    sourcePortNum,
                    targetPortNum,
                    gPtpNewParseMode);

                ptpTsCheck = GT_TRUE;
                targetPortInfo.info_common.egressDelay = 0;
                sourceInfoType = CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_PRE_DA_E;
                sourcePortInfo.info_pre_da.ptpPortInfo.ptpOverMplsEn = gPtpNewParseMode;
            }
            else if (extParam & EXT_PARAM_PTP_TS2_CF_CNS)
            {
                PRV_TGF_LOG3_MAC("Ingress packet from 'Extended Port' [%d] --> expected to : egress at 'Uplink Port' [%d], PTP parsing mode [%d] with updated timestamp 2 in CF - TS check\n",
                    sourcePortNum,
                    targetPortNum,
                    gPtpNewParseMode);

                ptpTsCheck = GT_TRUE;
                targetPortInfo.info_common.egressDelay = PTP_EGRES_DELAY_NANO_SEC;
                sourceInfoType = CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_PRE_DA_E;
                sourcePortInfo.info_pre_da.ptpPortInfo.ptpOverMplsEn = gPtpNewParseMode;
            }

            break;

        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CPU_PORT_E:
            PRV_TGF_LOG2_MAC("Ingress packet from 'Uplink Port' [%d] --> expected to : egress at 'CPU Port' [%d] with added 8 bytes ETAG \n",
                sourcePortNum,
                targetPortNum);
            haPerationInfo.info_802_1br_E2U.eTag = egressEtag2CPUInfo;
            sourceInfoType = CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_802_1BR_E;

            break;

        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_CPU_PORT_TO_UPSTREAM_PORT_E:
            if(extParam == EXT_PARAM_802_1BR_CPU2U_EGRESS_NO_ETAG_CNS)
            {
                PRV_TGF_LOG2_MAC("Ingress packet from 'CPU Port' [%d] --> expected to : egress at 'Uplink Port' [%d] (as ingress) --> without ETAG \n",
                    sourcePortNum,
                    targetPortNum);
            }
            else
            if(extParam == EXT_PARAM_802_1BR_CPU2U_EGRESS_WITH_ETAG_CNS)
            {
                PRV_TGF_LOG2_MAC("Ingress packet from 'CPU Port' [%d] --> expected to : egress at 'Uplink Port' [%d] (as ingress) --> with ETAG \n",
                    sourcePortNum,
                    targetPortNum);
            }
            else
            {
                /*case not supported*/
                return GT_NOT_SUPPORTED;
            }
            GT_ATTR_FALLTHROUGH;

        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DO_NOT_MODIFY_E:
            PRV_TGF_LOG2_MAC("Ingress packet from port [%d] --> expected to : egress at port [%d] (as ingress) --> without modification \n",
                sourcePortNum,
                targetPortNum);
            targetInfoType = CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_DO_NOT_MODIFY_E;
            break;

        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_IPL_PORT_TO_IPL_PORT_E:
            PRV_TGF_LOG2_MAC("Ingress packet from 'IPL Port' [%d] --> expected to : egress at ' Port' [%x] without modification\n",
                             sourcePortNum,
                             targetPortNum);

            sourceInfoType = CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_802_1BR_E;
            break;

        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_UNTAGGED_E:
            PRV_TGF_LOG2_MAC("Ingress untagged packet from 'Extended Port' [%d] --> expected to : egress at 'Uplink Port' [%d] with VLAN tag \n",
                sourcePortNum,
                targetPortNum);

            /* Assign expected ETAG and default VLAN tag attributes for output packet */
            haPerationInfo.info_802_1br_E2U.eTag = egressEtagInfo;
            haPerationInfo.info_802_1br_E2U.vlanTagTpid = EXTENDED_PORT_TPID_CNS;
            sourceInfoType = CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_802_1BR_UNTAGGED_E;
            sourcePortInfo.info_802_1br.pcid = ECID_INGRESS_PORT_CNS;
            sourcePortInfo.info_802_1br.ptpPortInfo.ptpPortMode = CPSS_PX_EGRESS_SOURCE_PORT_PTP_MODE_TC_E;
            sourcePortInfo.info_802_1br.srcPortInfo.vlanTag.vid = EXTENDED_PORT_PVID_CNS;

            if (extParam == EXT_PARAM_802_1BR_E2U_EGRESS_PVID_CNS)
            {
                /* Assign expected VLAN tag attributes for output packet */
                sourcePortInfo.info_802_1br.srcPortInfo.vlanTag.pcp = E_PCP1_CNS;
                sourcePortInfo.info_802_1br.srcPortInfo.vlanTag.dei = E_DEI1_CNS;
            }
            break;

        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DROP_PACKET_E:
            PRV_TGF_LOG2_MAC("Ingress packet from port [%d] --> expected to : dropped on egress port [%d] \n",
                sourcePortNum,
                targetPortNum);
            targetInfoType = CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_DO_NOT_MODIFY_E;

            break;

        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_M4_PORT_E:
            PRV_TGF_LOG2_MAC("Ingress packet from 'Uplink Port' [%d] --> expected to : egress at LAG M4 ' Port' [%x] without Etag \n",
                             sourcePortNum,targetPortNum);
            haPerationInfo.info_802_1br_U2E_MC = egressLagMcInfo[sourcePortNum];
            if (extParam & SRC_FILTERING_TEST_ECID)
            {
                ingressFrame.partsInfo[1].partBytesPtr[2] = 0;
                ingressFrame.partsInfo[1].partBytesPtr[3] = (GT_U8)simpleList_egressPortsArr[0].portNum;

            }
            else
            {
                ingressFrame.partsInfo[1].partBytesPtr[2] = 0;
                ingressFrame.partsInfo[1].partBytesPtr[3] = (GT_U8)sourcePortNum;
            }
            break;

        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_M8_PORT_E:
            PRV_TGF_LOG2_MAC("Ingress packet from 'Uplink Port' [%d] --> expected to : egress at LAG M8 ' Port' [%x] without Etag \n",
                             sourcePortNum,targetPortNum);
            haPerationInfo.info_802_1br_U2E_MC = egressLagMcInfo[sourcePortNum];
            if (extParam & SRC_FILTERING_TEST_ECID)
            {
                ingressFrame.partsInfo[1].partBytesPtr[2] = 0;
                ingressFrame.partsInfo[1].partBytesPtr[3] = (GT_U8)simpleList_egressPortsArr[0].portNum;
            }
            else
            {
                ingressFrame.partsInfo[1].partBytesPtr[2] = 0;
                ingressFrame.partsInfo[1].partBytesPtr[3] = (GT_U8)sourcePortNum;
            }
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_E2U_PACKET_E:
            PRV_TGF_LOG2_MAC("EVB Ingress packet from 'Extended Port' [%d] --> expected to : egress at 'Uplink Port' [%d] with added 4 bytes VLAN tag \n",
                sourcePortNum,
                targetPortNum);
            sourceInfoType = CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_EVB_E;
            targetInfoType = CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_EVB_E;

            /* Assign expected VLAN tag attributes for output packet */
            sourcePortInfo.info_evb.vid = EXTENDED_PORT_PVID_CNS;
            sourcePortInfo.info_evb.ptpPortInfo.ptpPortMode = CPSS_PX_EGRESS_SOURCE_PORT_PTP_MODE_TC_E;
            targetPortInfo.info_common.tpid = EXTENDED_PORT_TPID_CNS;   /* TPID to cause match */

            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_U2E_PACKET_E:
            PRV_TGF_LOG2_MAC("EVB Ingress VLAN tagged packet from 'Upstream Port' [%d] --> expected to : egress at 'Extended Port' [%d] with removed 4 bytes VLAN tag \n",
                sourcePortNum,
                targetPortNum);
            targetInfoType = CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_DO_NOT_MODIFY_E;

            break;

        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_QCN_E:
            PRV_TGF_LOG2_MAC("EVB Ingress VLAN tagged packet from 'Upstream Port' [%d] --> expected to : egress at 'Extended Port' [%d] with added EVB QCN tag \n",
                sourcePortNum,
                targetPortNum);

            haPerationInfo.info_evb_QCN = evbQcnInfo;

            targetInfoType = CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_EVB_E;
            targetPortInfo.info_common.tpid = EVB_QCN_TAG_TPID;             /* TPID to cause match */

            rc = cpssPxEgressQcnVlanTagEntrySet(devNum, simpleList_egressPortsArr[0].portNum, &evbQcnVlanTag);
            if(rc != GT_OK)
            {
                return rc;
            }

            nonCompareEvbQcndBytesArr[2] = ((PRV_TGF_TRAFFIC_GEN_SKIP_PKT_LEN_CHECK_CNS << 16) | COMPARE_EVB_QCN_PACKET_LEN);

            simpleList_egressPortsArr[0].egressFrame.partsInfo[0].partBytesPtr = evbQcnL2Part;
            simpleList_egressPortsArr[0].byteNumMaskListPtr = nonCompareEvbQcndBytesArr;
            simpleList_egressPortsArr[0].byteNumMaskSize = sizeof(nonCompareEvbQcndBytesArr)/sizeof(nonCompareEvbQcndBytesArr[0]);

            break;

        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_PRE_DA_EXTENDED_PORT_TO_UPSTREAM_PORT_E:
            PRV_TGF_LOG2_MAC("Ingress packet from 'Extended Port' [%d] --> expected to : egress at 'Uplink Port' [%d] with added 2 bytes pre-DA tag \n",
                sourcePortNum,
                targetPortNum);
            haPerationInfo.info_pre_da_PTP = preMacDaInfo;
            sourceInfoType = CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_PRE_DA_E;
            sourcePortInfo.info_pre_da.port = INGRESS_PORT_NUM;
            sourcePortInfo.info_pre_da.ptpPortInfo.ptpPortMode = sourcePortInfo.ptpPortInfo.ptpPortMode;
            sourcePortInfo.info_pre_da.ptpPortInfo.ptpOverMplsEn = GT_FALSE;

            targetInfoType = CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_DO_NOT_MODIFY_E;

            if (extParam & EXT_PARAM_MPLS_PTP_HEADER_CNS  ||
                extParam & EXT_PARAM_MPLS_PTP_IPV6_HEADER_CNS)
            {
                PRV_TGF_LOG2_MAC("Ingress packet from 'Extended Port' [%d] --> expected to : egress at 'Uplink Port' [%d] with updated timestamp - TS check\n",
                    sourcePortNum,
                    targetPortNum);
                sourcePortInfo.info_pre_da.ptpPortInfo.ptpOverMplsEn = GT_TRUE;
                ptpTsCheck = GT_TRUE;
            }

            break;

        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_PRE_DA_UPSTREAM_PORT_TO_EXTENDED_PORT_E:
            PRV_TGF_LOG2_MAC("Ingress packet from 'Upstream Port' [%d] --> expected to : egress at 'Extended Port' [%d] acording to port in 2 bytes pre-DA tag without 2 bytes pre-DA tag \n",
                sourcePortNum,
                targetPortNum);
            haPerationInfo.info_pre_da_PTP = preMacDaInfo;
            targetInfoType = CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_DO_NOT_MODIFY_E;
            if (extParam & EXT_PARAM_MPLS_PTP_HEADER_CNS  ||
                extParam & EXT_PARAM_MPLS_PTP_IPV6_HEADER_CNS)
            {
                PRV_TGF_LOG2_MAC("Ingress packet from 'Upstream Port' [%d] --> expected to : egress at 'Extended Port' [%d] with updated timestamp - TS check\n",
                    sourcePortNum,
                    targetPortNum);
                sourceInfoType = CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_PRE_DA_E;
                sourcePortInfo.info_pre_da.ptpPortInfo.ptpOverMplsEn = GT_TRUE;
                ptpTsCheck = GT_TRUE;
            }

            break;


        default :
            /* not implemented or not needed */
            break;
    }

    if(phaBypassEnable == GT_TRUE)
    {
        PRV_TGF_LOG2_MAC("phaBypassEnable = GT_TRUE --> Ingress packet from port [%d] --> expected to : egress 'AS ingress' at port [%d] (as ingress) --> without modification \n",
            sourcePortNum,
            targetPortNum);

        rc = cpssPxEgressBypassModeSet(devNum,GT_TRUE,GT_TRUE);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    for (j= 0; j < numOfEgressPortsInfo; j++)
    {
        rc = cpssPxEgressHeaderAlterationEntrySet(devNum, simpleList_egressPortsArr[j].portNum, packetType,
                haAction,&haPerationInfo);
    }
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = cpssPxEgressSourcePortEntrySet(devNum,sourcePortNum,
        sourceInfoType,&sourcePortInfo);
    if(rc != GT_OK)
    {
        return rc;
    }

    for (j= 0; j < numOfEgressPortsInfo; j++)
    {
        if ((haAction == CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_M4_PORT_E) ||
            (haAction == CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_M8_PORT_E))
        {
            targetPortInfo.info_common.pcid = simpleList_egressPortsArr[j].portNum;
        }
        rc = cpssPxEgressTargetPortEntrySet(devNum,simpleList_egressPortsArr[j].portNum,
                                            targetInfoType,&targetPortInfo);
        if(rc != GT_OK)
        {
            return rc;
        }

        if (ptpTsCheck)
        {
            rc = cpssPxPtpTsuControlSet(devNum, simpleList_egressPortsArr[j].portNum, &control);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    rc = prvTgfPxInjectToIngressPortAndCheckEgressSimpleList(devNum,sourcePortNum,
        &ingressFrame,numOfEgressPortsInfo,simpleList_egressPortsArr);

    if (ptpTsCheck)
    {
        if(rc == GT_BAD_VALUE)
        {
            /* Packet TS is changed */
            rc = GT_OK;
        }
        else
        {
            if (!((extParam & EXT_PARAM_DEFAULT_PTP_HEADER_NO_TS_CHECK_CNS) ||
                  (extParam & EXT_PARAM_PTP_TS1_CF_CNS)))
            {
                /* Packet TS is not changed */
                rc = GT_FAIL;
            }
        }
    }

    if(rc != GT_OK)
    {
        PRV_TGF_LOG1_MAC("prvTgfPxEgressBasic[%d] : Test FAILED \n",haAction);
    }
    else
    {
        PRV_TGF_LOG1_MAC("prvTgfPxEgressBasic[%d] : Test PASSED \n",haAction);
    }

    if(phaBypassEnable == GT_TRUE)
    {
        /* restore config to 'default'*/
        (void)cpssPxEgressBypassModeSet(devNum,GT_FALSE,GT_TRUE);
    }

    return rc;
}

/* Common egress processing function used in tests */
static GT_STATUS prvTgfPxEgressBasic(
    IN GT_SW_DEV_NUM     devNum,
    IN CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT  haAction,
    IN GT_U32           extParam
)
{
    GT_PHYSICAL_PORT_NUM sourcePortNum = INGRESS_PORT_NUM;
    GT_U32               egressPorts[1]={UPLINK_PORT_NUM};/* set at runtime */
    GT_PHYSICAL_PORT_NUM targetPortNum;
    CPSS_PX_PACKET_TYPE  packetType;
    GT_STATUS            rc;
    GT_U32               frameLength;

    packetType = 7;/* as the ingress processing assigned */
    targetPortNum = egressPorts[0];

    /* save configuration to allow restore */
    saveConfig(devNum,packetType,sourcePortNum,targetPortNum);
    /* disable all ports. (we skip CPU port)
        those that need traffic will be enabled by the engine.
        main purpose : to reduce replications in the simulation LOG.

        anyway we test only single egress port at a test.
    */
    disabledAllPorts(devNum);

    frameLength = sizeof(frameArr);

    /* call main egress process */
    rc = prvTgfPxEgressProcess(devNum, haAction, extParam, packetType, sourcePortNum, targetPortNum, frameArr, frameLength);

    restoreConfig(devNum,packetType,sourcePortNum,targetPortNum);

    return rc;
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: basic test for CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_E
 *    check for 802.1br : Extended Port to Upstream Port (802.1br-E2U)
 *    check that egress packet added with proper ETag.
 */
UTF_TEST_CASE_MAC(prvTgfPxEgress_802_1br_E2U)
{
    GT_STATUS   st;
    GT_U8       dev = 0;

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvTgfPxEgressBasic(dev,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_E,
            EXT_PARAM_DEFAULT_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: basic test for CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_E
 *    check for 802.1br : Upstream Port to Extended Port (802.1br-U2E)
 *    check that egress packet Popped the ETAG + vlan tag
 *    mode:The VLAN tag is removed only if the packet�s VLAN-ID equal to the
 *    Target Port Table[27:16] - Port VLAN-ID (PVID).
 *
 *   NOTE: PVID of egress port match vlanId on the tag after the ETag --> pop vlan tag
 */
UTF_TEST_CASE_MAC(prvTgfPxEgress_802_1br_U2E_untagged_pvid_match)
{
    GT_STATUS   st;
    GT_U8       dev = 0;

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvTgfPxEgressBasic(dev,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_E,
            EXT_PARAM_802_1BR_U2E_EGRESS_UNTAGGED_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: basic test for CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_E
 *    check for 802.1br : Upstream Port to Extended Port (802.1br-U2E)
 *    check that egress packet Popped the ETAG , keep vlan tag.(egress with vlan tag)
 *    mode:The VLAN tag is removed only if the packet�s VLAN-ID equal to the
 *    Target Port Table[27:16] - Port VLAN-ID (PVID).
 *
 *   NOTE: PVID of egress port NOT match vlanId on the tag after the ETag -->
 *   NOT pop vlan tag (keep vlan tag) --> egress with vlan tag
 */
UTF_TEST_CASE_MAC(prvTgfPxEgress_802_1br_U2E_tagged_pvid_no_match)
{
    GT_STATUS   st;
    GT_U8       dev = 0;

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvTgfPxEgressBasic(dev,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_E,
            EXT_PARAM_802_1BR_U2E_EGRESS_TAGGED_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: basic test for CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_E
 *    check for 802.1br : Upstream Port to Extended Port (802.1br-U2E)
 *    check that egress packet Popped the ETAG + vlan tag
 *    mode:The VLAN tag is removed for all the packets egressing the port.
 *
 *   NOTE: all packets egress without the vlan tag
 */
UTF_TEST_CASE_MAC(prvTgfPxEgress_802_1br_U2E_untagged_all)
{
    GT_STATUS   st;
    GT_U8       dev = 0;

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvTgfPxEgressBasic(dev,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_E,
            EXT_PARAM_802_1BR_U2E_EGRESS_ALL_UNTAGGED_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: basic test for CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_E
 *    check for 802.1br : Upstream Port to Extended Port (802.1br-U2E)
 *    check that egress packet Popped the ETAG (vlan tag not recognized on egress port)
 *    vlan tag was not recognized on egress port - due to no match with target port TPID
 *
 */
UTF_TEST_CASE_MAC(prvTgfPxEgress_802_1br_U2E_tpid_no_match)
{
    GT_STATUS   st;
    GT_U8       dev = 0;

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvTgfPxEgressBasic(dev,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_E,
            EXT_PARAM_802_1BR_U2E_EGRESS_UNTAGGED_TPID_NO_MATCH_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: basic test for CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CASCADE_PORT_E
 *    check for 802.1br : Upstream Port to Cascade Port  (802.1br-U2C)
 *    check that egress packet Keep the ETAG as ingress (packet egress unchanged)
 *
 *    NOTE: the 'target ECID' on the ETag is not the one of the cascade port
 *
 */
UTF_TEST_CASE_MAC(prvTgfPxEgress_802_1br_U2C_keep_ETag)
{
    GT_STATUS   st;
    GT_U8       dev = 0;

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvTgfPxEgressBasic(dev,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CASCADE_PORT_E,
            EXT_PARAM_802_1BR_U2C_EGRESS_KEEP_ETAG_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: basic test for CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CASCADE_PORT_E
 *    check for 802.1br : Upstream Port to Cascade Port  (802.1br-U2C)
 *    check that egress packet pop the ETAG (ETAG represents 'target ECID' is CPU of the 'next PE')
 *
 *    NOTE: the 'target ECID' on the ETag is the one of the cascade port.
 *
 */
UTF_TEST_CASE_MAC(prvTgfPxEgress_802_1br_U2C_pop_ETag)
{
    GT_STATUS   st;
    GT_U8       dev = 0;

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvTgfPxEgressBasic(dev,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CASCADE_PORT_E,
            EXT_PARAM_802_1BR_U2C_EGRESS_POP_ETAG_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: run the test of prvTgfPxEgress_802_1br_U2C_pop_ETag ,
 *  but when 'phaBypassEnable = GT_TRUE'
 *
 */
UTF_TEST_CASE_MAC(prvTgfPxEgress_802_1br_U2C_pop_ETag__PHA_bypassed)
{
    phaBypassEnable = GT_TRUE;

    /* call test of prvTgfPxEgress_802_1br_U2C_pop_ETag */
    UTF_TEST_CALL_MAC(prvTgfPxEgress_802_1br_U2C_pop_ETag);

    /* restore the defaults */
    phaBypassEnable = GT_FALSE;
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: basic test for CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_CASCADE_PORT_TO_UPSTREAM_PORT_E
 *    check for 802.1br : Cascade Port to Upstream Port  (802.1br-C2U)
 *    check that egress packet Keep the ETAG as ingress (packet egress unchanged)
 */
UTF_TEST_CASE_MAC(prvTgfPxEgress_802_1br_C2U)
{
    GT_STATUS   st;
    GT_U8       dev = 0;

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvTgfPxEgressBasic(dev,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_CASCADE_PORT_TO_UPSTREAM_PORT_E,
            EXT_PARAM_DEFAULT_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: basic test for CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CPU_PORT_E
 *    check for 802.1br : Upstream Port to CPU Port  (802.1br-U2CPU)
 *    CPU expected to get packet without ETAG (as ingress from the cascade port)
 *    check that egress packet as ingress (packet egress unchanged - ingress without ETAG)
 */
UTF_TEST_CASE_MAC(prvTgfPxEgress_802_1br_U2CPU)
{
    GT_STATUS   st;
    GT_U8       dev = 0;

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvTgfPxEgressBasic(dev,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CPU_PORT_E,
            EXT_PARAM_DEFAULT_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: basic test for CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_CPU_PORT_TO_UPSTREAM_PORT_E
 *    check for 802.1br : CPU Port to Upstream Port (802.1br-CPU2U)
 *    uplink expected to get packet with/without ETAG from the CPU
 *    check case : uplink get packet without ETAG from the CPU. (egress as ingress)
 */
UTF_TEST_CASE_MAC(prvTgfPxEgress_802_1br_CPU2U_no_etag)
{
    GT_STATUS   st;
    GT_U8       dev = 0;

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvTgfPxEgressBasic(dev,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_CPU_PORT_TO_UPSTREAM_PORT_E,
            EXT_PARAM_802_1BR_CPU2U_EGRESS_NO_ETAG_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: basic test for CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_CPU_PORT_TO_UPSTREAM_PORT_E
 *    check for 802.1br : CPU Port to Upstream Port (802.1br-CPU2U)
 *    uplink expected to get packet with/without ETAG from the CPU
 *    check case : uplink get packet with ETAG from the CPU. (egress as ingress)
 */
UTF_TEST_CASE_MAC(prvTgfPxEgress_802_1br_CPU2U_with_etag)
{
    GT_STATUS   st;
    GT_U8       dev = 0;

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvTgfPxEgressBasic(dev,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_CPU_PORT_TO_UPSTREAM_PORT_E,
            EXT_PARAM_802_1BR_CPU2U_EGRESS_WITH_ETAG_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: basic test for CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DO_NOT_MODIFY_E
 *    check case : egress as ingress
 */
UTF_TEST_CASE_MAC(prvTgfPxEgress_doNotModify)
{
    GT_STATUS   st;
    GT_U8       dev = 0;

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvTgfPxEgressBasic(dev,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DO_NOT_MODIFY_E,
            EXT_PARAM_DEFAULT_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: basic test for CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_UNTAGGED_E
 *    check for 802.1br : Extended Port to Upstream Port for Untagged packets (802.1br-U2E-Untagged)
 *    check that in egress packet VLAN tag is added after ETAG.
 */
UTF_TEST_CASE_MAC(prvTgfPxEgress_802_1br_E2U_Untagged)
{
    GT_STATUS   st;
    GT_U8       dev = 0;

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvTgfPxEgressBasic(dev, CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_UNTAGGED_E,
                                 EXT_PARAM_DEFAULT_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = prvTgfPxEgressBasic(dev,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_UNTAGGED_E,
            EXT_PARAM_802_1BR_E2U_EGRESS_PVID_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: basic test for CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DROP_PACKET_E
 *    check for 802.1br : Discard packet (802.1br-Discard)
 *    - send packet with HA type - CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DROP_PACKET_E
 *    the packet is dropped on egress port.
 *    - send the same packet with HA type - CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DO_NOT_MODIFY_E
 *    thr packet is received on egress port unmodified
 */
UTF_TEST_CASE_MAC(prvTgfPxEgress_802_1br_Discard_packet)
{
    GT_STATUS   st;
    GT_U8       dev = 0;

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvTgfPxEgressBasic(dev,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DROP_PACKET_E,
            EXT_PARAM_DEFAULT_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_GET_ERROR, st, dev);

        st = prvTgfPxEgressBasic(dev,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DO_NOT_MODIFY_E,
            EXT_PARAM_DEFAULT_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: basic test for CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_IPL_PORT_E
 *    check for 802.1br : Upstream Port to IPL Port (802.1br-U2IPL)
 *    check that in egress packet ETag direction bit is set.
 */
UTF_TEST_CASE_MAC(prvTgfPxEgress_802_1br_U2IPL)
{
    GT_STATUS   st;
    GT_U8       dev = 0;

    /* Prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvTgfPxEgressBasic(dev,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_IPL_PORT_E,
            EXT_PARAM_DEFAULT_CNS);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssPxTgfEgress suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssPxTgfEgress802_1br)
    /*****/
    /*CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_E*/
    /*****/
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxEgress_802_1br_E2U)
    /*****/
    /*CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_E*/
    /*****/
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxEgress_802_1br_U2E_untagged_pvid_match)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxEgress_802_1br_U2E_tagged_pvid_no_match)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxEgress_802_1br_U2E_untagged_all)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxEgress_802_1br_U2E_tpid_no_match)
    /*****/
    /*CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CASCADE_PORT_E*/
    /*****/
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxEgress_802_1br_U2C_keep_ETag)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxEgress_802_1br_U2C_pop_ETag)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxEgress_802_1br_U2C_pop_ETag__PHA_bypassed)
    /*****/
    /*CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_CASCADE_PORT_TO_UPSTREAM_PORT_E*/
    /*****/
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxEgress_802_1br_C2U)
    /*****/
    /*CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CPU_PORT_E*/
    /*****/
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxEgress_802_1br_U2CPU)
    /*****/
    /*CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_CPU_PORT_TO_UPSTREAM_PORT_E*/
    /*****/
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxEgress_802_1br_CPU2U_no_etag)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxEgress_802_1br_CPU2U_with_etag)
    /*****/
    /*CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_IPL_PORT_E*/
    /*****/
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxEgress_802_1br_U2IPL)
    /*****/
    /*CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_UNTAGGED_E*/
    /*****/
    UTF_SUIT_DECLARE_TEST_MAC (prvTgfPxEgress_802_1br_E2U_Untagged)

    /*****/
    /*CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DROP_PACKET_E*/
    /*****/
    UTF_SUIT_DECLARE_TEST_MAC (prvTgfPxEgress_802_1br_Discard_packet)

    /*****/
    /*CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DO_NOT_MODIFY_E*/
    /*****/
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfPxEgress_doNotModify)


UTF_SUIT_END_TESTS_MAC(cpssPxTgfEgress802_1br)


