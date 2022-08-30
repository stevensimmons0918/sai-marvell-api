/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* csRefVpnVxlanNvgre.c
*
* DESCRIPTION:
*  This files provide APIs to create VxLAN UNI and NNI interfaces.
*
*
* FILE REVISION NUMBER:
*       $Revision: 1.0 $
*
*******************************************************************************/

#include "csRefVpnVxlanNvgre.h"
#include "../../infrastructure/csRefServices/eArchElementsDb.h"


#include <gtOs/gtOsMem.h>
#include <gtOs/gtOsInet.h>

#include <cpss/generic/cpssTypes.h>

#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInit.h>

#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgVlan.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgMc.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgE2Phy.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgEgrFlt.h>

#include <cpss/dxCh/dxChxGen/l2mll/cpssDxChL2Mll.h>
#include <cpss/dxCh/dxChxGen/tti/cpssDxChTti.h>
#include <cpss/dxCh/dxChxGen/tti/cpssDxChTtiTypes.h>
#include <cpss/dxCh/dxChxGen/tunnel/cpssDxChTunnel.h>
#include <cpss/dxCh/dxChxGen/tunnel/cpssDxChTunnelTypes.h>
#include <cpss/dxCh/dxChxGen/pcl/cpssDxChPcl.h>

#include <cpss/dxCh/dxChxGen/ip/cpssDxChIpCtrl.h>


/***********************************************/
/*         Definitions for entry management purposes only */
/***********************************************/

#define VPN_TTI_LOOKUP_CNS    0

/***********************************************/
/*         Feature relate code                                          */
/***********************************************/
#define VPN_DEAFULT_TPID_PROFILE_CNS 6
#define VPN_EMPTY_TPID_PROFILE_CNS 7  /* Empty BMP. All packets considered untagged */

#define VPN_VXLAN_UDP_PORT_4789_CNS      4789


#define VPN_VXLAN_TS_GEN_PROFILE_ENTRY_CNS 0
#define VPN_NVGRE_TS_GEN_PROFILE_ENTRY_CNS 1


#define VPN_TTI_KEY_UDB_IPV4_UDP_PCL_ID_CNS       0x1
#define VPN_TTI_KEY_UDB_IPV6_UDP_PCL_ID_CNS       0x2
#define VPN_TTI_KEY_UDB_IPV4_OTHER_PCL_ID_CNS     0x3
#define VPN_TTI_KEY_UDB_ETHERNET_OTHER_PCL_ID_CNS 0x4

#define VPN_TTI_KEY_UDB_SIZE_CNS CPSS_DXCH_TTI_KEY_SIZE_30_B_E
#define VPN_TTI_ENTRY_SIZE 3

#define VPN_TTI_MAC2ME_ENTRY_INDEX_CNS 127
#define VPN_VXLAN_GLOBAL_MAC_SA_ENTRY_CNS 255


#define VPN_IPV6_PCL_CFG_TABLE_INDEX_CNS       0x0 /* Use the PCL configuration table entry 0 */
#define VPN_PCL_IPV6_VxLAN_PCL_ID_CNS          0x10 /* Use the PCL configuration table entry 0 */


#define VPN_IPV6_PCL_ipv6Key_CNS      CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E
#define VPN_IPV6_PCL_ipv6Key_size_CNS 3 /* rule size 30 Bytes */

#define VPN_IPV6_PCL_UDB_PACKET_TYPE_CNS CPSS_DXCH_PCL_PACKET_TYPE_UDE6_E

#define VPN_MESH_ID_BIT_IN_SOURCE_ID_CNS 0  /* Bit 0 of source id used for split horizon. */
#define VPN_MESH_ID_BIT_SIZE_CNS 1   /* Number of bits in source-id used for mesh-id. */


/****** Structure to store infromation for clean up **/

typedef struct{
  GT_PORT_NUM     portNum;
  GT_U32          pclRuleEntryIndex;
  GT_U32          ttiRuleEntryIndex;
  GT_U32          tunnelStartEntryIndex;
  CPSS_IP_PROTOCOL_STACK_ENT  ipProt;
}EPORT_INFOMATION;


/* PCL UDB configuration */
typedef struct{
    GT_U32                               udbIndex;
    CPSS_DXCH_PCL_OFFSET_TYPE_ENT        offsetType;
    GT_U8                                offset;
    GT_U8                                byteMask;
}PCL_UDB_STC;


/* TTI UDB configuration */
typedef struct{
    GT_U32                               udbIndex;
    CPSS_DXCH_TTI_OFFSET_TYPE_ENT        offsetType;
    GT_U8                                offset;
    GT_U8                                byteMask;
}TTI_UDB_STC;

/***************************** Internal variables  ****************************/





/****************************************************************************/
/***************************** Function implementation ****************************/
/****************************************************************************/

/* UDB configuration and mask table */
static TTI_UDB_STC tti_ETHERNET_OTHER_UdbInfo[] = {
     {0 , CPSS_DXCH_TTI_OFFSET_METADATA_E, 22, 0xBF} /* [0:4]:PCLI[0:4], [5] isTrunk, [7] MAC2ME */
    ,{1 , CPSS_DXCH_TTI_OFFSET_METADATA_E, 23 ,0x1F} /* [0:4]:PCLI[5:9]  */
    ,{2 , CPSS_DXCH_TTI_OFFSET_METADATA_E, 26 ,0xFF} /* [0:7]: Source Eport [0:7] */
    ,{3 , CPSS_DXCH_TTI_OFFSET_METADATA_E, 27 ,0x1F} /* [0:4]: Source Eport [8:12] */
    ,{9 , CPSS_DXCH_TTI_OFFSET_METADATA_E, 14 ,0xFF} /* [0:7]: tag1 vid [0:7] */
    ,{10, CPSS_DXCH_TTI_OFFSET_METADATA_E, 15 ,0x0F} /* [0:3]: tag1 vid  [8:11] */
    ,{11, CPSS_DXCH_TTI_OFFSET_L2_E ,6  ,0xFF} /* [0:7]: Source MAC byte 0 [0:7] */
    ,{12, CPSS_DXCH_TTI_OFFSET_L2_E ,7  ,0xFF} /* [0:7]: Source MAC byte 1 [0:7] */
    ,{13, CPSS_DXCH_TTI_OFFSET_L2_E ,8  ,0xFF} /* [0:7]: Source MAC byte 2 [0:7] */
    ,{14, CPSS_DXCH_TTI_OFFSET_L2_E ,9  ,0xFF} /* [0:7]: Source MAC byte 3 [0:7] */
    ,{15, CPSS_DXCH_TTI_OFFSET_L2_E ,10 ,0xFF} /* [0:7]: Source MAC byte 4 [0:7] */
    ,{16, CPSS_DXCH_TTI_OFFSET_L2_E ,11 ,0xFF} /* [0:7]: Source MAC byte 5 [0:7] */
    /* must be last */
    ,{0, CPSS_DXCH_TTI_OFFSET_INVALID_E , 0, 0xFF}
};

static GT_STATUS prvVpnUNITtiConfigurationSet
(
   IN GT_U8  devNum
)
{

  GT_STATUS rc = GT_OK;
  CPSS_DXCH_TTI_KEY_TYPE_ENT     keyType;
  GT_U32                         udbIndex, index;
  CPSS_DXCH_TTI_OFFSET_TYPE_ENT  offsetType;
  GT_U32                         offset;

  /****************************************************************************/
  /******************** Key definition to be used for ETH OTHER (UNI) *****************/
  /****************************************************************************/
  keyType = CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E;
  rc = cpssDxChTtiPclIdSet(devNum, keyType, VPN_TTI_KEY_UDB_ETHERNET_OTHER_PCL_ID_CNS);
  if(rc != GT_OK)
    return rc;

  rc = cpssDxChTtiPacketTypeKeySizeSet(devNum, keyType, VPN_TTI_KEY_UDB_SIZE_CNS);
  if(rc != GT_OK)
    return rc;

  /* Configure UDb's for Ethernet Other. */
  for(index = 0;  ; index++)
  {
    if(tti_ETHERNET_OTHER_UdbInfo[index].offsetType == CPSS_DXCH_TTI_OFFSET_INVALID_E)
        break;
    udbIndex = tti_ETHERNET_OTHER_UdbInfo[index].udbIndex;
    offsetType = tti_ETHERNET_OTHER_UdbInfo[index].offsetType;
    offset      = tti_ETHERNET_OTHER_UdbInfo[index].offset;
    rc = cpssDxChTtiUserDefinedByteSet(devNum, keyType, udbIndex, offsetType, offset);
    if(rc != GT_OK)
      return rc;
  }
  return GT_OK;
}


/* UDB configuration and mask table */
static TTI_UDB_STC tti_IPV4_UDP_UdbInfo[] = {
     {0 , CPSS_DXCH_TTI_OFFSET_METADATA_E, 22, 0xBF}   /* [0:4]:PCLI[0:4], [5] isTrunk, [7] MAC2ME */
    ,{1 , CPSS_DXCH_TTI_OFFSET_METADATA_E, 23 ,0x7F}   /* [0:4]:PCLI[5:9]  */
    ,{2 , CPSS_DXCH_TTI_OFFSET_METADATA_E, 26 ,0xFF}   /* [0:7]: Source Eport [0:7] */
    ,{3 , CPSS_DXCH_TTI_OFFSET_METADATA_E, 27 ,0xFF}   /* [0:4]: Source Eport [8:12] */
    ,{4 , CPSS_DXCH_TTI_OFFSET_L3_MINUS_2_E, 11 ,0xFF} /* [0:7]: IP header protcol [0:7] */
    ,{5 , CPSS_DXCH_TTI_OFFSET_L4_E, 2 ,0xFF} /* [0:7]: udp port byte [8:15] */
    ,{6 , CPSS_DXCH_TTI_OFFSET_L4_E, 3 ,0xFF} /* [0:7]: udp port byte [0:7]   */
    ,{7 , CPSS_DXCH_TTI_OFFSET_L4_E,12 ,0xFF} /* [0:7]: VXLAN Network Identifier (VNI) [16:23]   */
    ,{8 , CPSS_DXCH_TTI_OFFSET_L4_E,13 ,0xFF} /* [0:7]: VXLAN Network Identifier (VNI) [8:15]   */
    ,{9 , CPSS_DXCH_TTI_OFFSET_L4_E,14 ,0xFF} /* [0:7]: VXLAN Network Identifier (VNI) [0:7]   */
    ,{10, CPSS_DXCH_TTI_OFFSET_L4_E, 8 ,0xFF} /* [0:7]: VXLAN flags  [0:7]   */
    ,{11, CPSS_DXCH_TTI_OFFSET_METADATA_E ,4 ,0xFF} /* [0:7]: DIP [24:31] */
    ,{12, CPSS_DXCH_TTI_OFFSET_METADATA_E ,3 ,0xFF} /* [0:7]: DIP [16:23] */
    ,{13, CPSS_DXCH_TTI_OFFSET_METADATA_E ,2 ,0xFF} /* [0:7]: DIP [8:15]  */
    ,{14, CPSS_DXCH_TTI_OFFSET_METADATA_E, 1 ,0xFF} /* [0:7]: DIP [0:7]     */
    ,{15, CPSS_DXCH_TTI_OFFSET_METADATA_E ,8 ,0xFF} /* [0:7]: SIP [24:31]*/
    ,{16, CPSS_DXCH_TTI_OFFSET_METADATA_E ,7 ,0xFF} /* [0:7]: SIP [16:23] */
    ,{17, CPSS_DXCH_TTI_OFFSET_METADATA_E ,6 ,0xFF} /* [0:7]: SIP [8:15]    */
    ,{18, CPSS_DXCH_TTI_OFFSET_METADATA_E, 5 ,0xFF} /* [0:7]: SIP [0:7]     */
    /* must be last */
    ,{0, CPSS_DXCH_TTI_OFFSET_INVALID_E , 0, 0}
};

static GT_STATUS prvVpnIpv4NNITtiConfigurationSet
(
  IN GT_U8  devNum
)
{
  GT_STATUS rc = GT_OK;
  CPSS_DXCH_TTI_KEY_TYPE_ENT     keyType;
  GT_U32                         udbIndex, index;
  CPSS_DXCH_TTI_OFFSET_TYPE_ENT  offsetType;
  GT_U32                         offset;

  /* TTI UDB configuration */



  /****************************************************************************/
  /*********************** Key definition to be used for VXLAN (NNI) ******************/
  /****************************************************************************/
  /* VXLAN:
  Match  src-physical-port   local-ip-addr remote-ip-addr dst-udp-port 4789 vsid {vsi-id} (4 bytes after UDP header) */
  keyType = CPSS_DXCH_TTI_KEY_UDB_IPV4_UDP_E;
  rc = cpssDxChTtiPclIdSet(devNum, keyType, VPN_TTI_KEY_UDB_IPV4_UDP_PCL_ID_CNS);
  if(rc != GT_OK)
    return rc;

  rc = cpssDxChTtiPacketTypeKeySizeSet(devNum, keyType, VPN_TTI_KEY_UDB_SIZE_CNS);
  if(rc != GT_OK)
    return rc;

  /* Configure UDb's for IPv4 UDP. */
  for(index = 0;  ; index++)
  {
    if(tti_IPV4_UDP_UdbInfo[index].offsetType == CPSS_DXCH_TTI_OFFSET_INVALID_E)
        break;
    udbIndex = tti_IPV4_UDP_UdbInfo[index].udbIndex;
    offsetType = tti_IPV4_UDP_UdbInfo[index].offsetType;
    offset      = tti_IPV4_UDP_UdbInfo[index].offset;
    rc = cpssDxChTtiUserDefinedByteSet(devNum, keyType, udbIndex, offsetType, offset);
    if(rc != GT_OK)
      return rc;
  }


  /****************************************************************************/
  /********************** Key definition to be used for NVGRE (NNI) ******************/
  /****************************************************************************/
  /* Set ethertype0 value 0x6558 to recognize GRE tunneling protocol. */
  rc = cpssDxChTtiEthernetTypeSet(devNum, CPSS_TUNNEL_ETHERTYPE_TYPE_IPV4_GRE0_E, 0x6558);
  if(rc != GT_OK)
    return rc;

  keyType = CPSS_DXCH_TTI_KEY_UDB_IPV4_OTHER_E;
  rc = cpssDxChTtiPclIdSet(devNum, keyType, VPN_TTI_KEY_UDB_IPV4_OTHER_PCL_ID_CNS);
  if(rc != GT_OK)
    return rc;

  rc = cpssDxChTtiPacketTypeKeySizeSet(devNum, keyType, VPN_TTI_KEY_UDB_SIZE_CNS);
  if(rc != GT_OK)
    return rc;

  /* pcl id, udbIndex 0-1 */
  udbIndex = 0;
  offsetType = CPSS_DXCH_TTI_OFFSET_METADATA_E;
  offset = 22; /* bits 0:4 of the byte are bits 0:4 of PCL id,
                               bits 5 of the byte is "Local Device Source is Trunk" indication */
  rc = cpssDxChTtiUserDefinedByteSet(devNum, keyType, udbIndex, offsetType, offset);
  if(rc != GT_OK)
    return rc;

  udbIndex = 1;
  offset = 23; /* bits 0:4 of the byte are bits 5:9 of PCL id */
  rc = cpssDxChTtiUserDefinedByteSet(devNum, keyType, udbIndex, offsetType, offset);
  if(rc != GT_OK)
    return rc;

  /* source physical eport, udbIndex  2-3*/
  udbIndex = 2;
  offsetType = CPSS_DXCH_TTI_OFFSET_METADATA_E;
  offset = 26; /* bits 0:7 of the byte are bits 0:7 of ingress source eport */
  rc = cpssDxChTtiUserDefinedByteSet(devNum, keyType, udbIndex, offsetType, offset);
  if(rc != GT_OK)
    return rc;

  udbIndex = 3;
  offset = 27; /* bits 0:4 of the byte are bits 8:12 of ingress source eport */
  rc = cpssDxChTtiUserDefinedByteSet(devNum, keyType, udbIndex, offsetType, offset);
  if(rc != GT_OK)
    return rc;

  /* 4 bytes destination ip address, udbIndex  4-7 */
  /* Offset 1-4 of metadata*/
  offsetType = CPSS_DXCH_TTI_OFFSET_METADATA_E;
  for(udbIndex = 4 , offset=4 ; udbIndex < 8 ; udbIndex++, offset--)
  {
    rc = cpssDxChTtiUserDefinedByteSet(devNum, keyType, udbIndex, offsetType, offset);
    if(rc != GT_OK)
      return rc;
  }

  /* 1 bytes to recognize IPv4 Tunneling protocol, udbIndex  8*/
  /* Offset 0 of metadata */
  udbIndex = 8;
  offsetType = CPSS_DXCH_TTI_OFFSET_METADATA_E;
  offset = 0;
  rc = cpssDxChTtiUserDefinedByteSet(devNum, keyType, udbIndex, offsetType, offset);
  if(rc != GT_OK)
    return rc;

  /* 3 bytes Virtual Subnet Id (TNI), udbIndex  9-11*/
  /* Offset 24-26 from start of IP header */
  offsetType = CPSS_DXCH_TTI_OFFSET_L3_MINUS_2_E;
  for(udbIndex = 9 , offset=26 ; udbIndex < 12 ; udbIndex++, offset++)
  {
    rc = cpssDxChTtiUserDefinedByteSet(devNum, keyType, udbIndex, offsetType, offset);
    if(rc != GT_OK)
      return rc;
  }

  /* 1 bytes NVGRE flags and version, udbIndex  12 */
  /* 8 flags bit, first byte of GRE header */
  offsetType = CPSS_DXCH_TTI_OFFSET_L3_MINUS_2_E;
  udbIndex = 12;
  offset = 22;
  rc = cpssDxChTtiUserDefinedByteSet(devNum, keyType, udbIndex, offsetType, offset);
  if(rc != GT_OK)
    return rc;

  return GT_OK;

}


static TTI_UDB_STC tti_IPV6_UDP_UdbInfo[] = {
     {0 , CPSS_DXCH_TTI_OFFSET_METADATA_E, 22, 0xBF}   /* [0:4]:PCLI[0:4], [5] isTrunk, [7] MAC2ME */
    ,{1 , CPSS_DXCH_TTI_OFFSET_METADATA_E, 23 ,0x7F}   /* [0:4]:PCLI[5:9]  */
    ,{2 , CPSS_DXCH_TTI_OFFSET_METADATA_E, 26 ,0xFF}   /* [0:7]: Source Eport [0:7] */
    ,{3 , CPSS_DXCH_TTI_OFFSET_METADATA_E, 27 ,0xFF}   /* [0:4]: Source Eport [8:12] */
    ,{4 , CPSS_DXCH_TTI_OFFSET_L3_MINUS_2_E, 8 ,0xFF}  /* [0:7]: IP header protcol [0:7] */
    ,{5 , CPSS_DXCH_TTI_OFFSET_L4_E, 2 ,0xFF} /* [0:7]: udp port byte [8:15] */
    ,{6 , CPSS_DXCH_TTI_OFFSET_L4_E, 3 ,0xFF} /* [0:7]: udp port byte [0:7]   */
    ,{10, CPSS_DXCH_TTI_OFFSET_L4_E, 8 ,0xFF} /* [0:7]: VXLAN flags  [0:7]    */
    ,{11, CPSS_DXCH_TTI_OFFSET_L3_MINUS_2_E ,26,0xFF} /* [0:7]: DIP [120:127] */
    ,{12, CPSS_DXCH_TTI_OFFSET_L3_MINUS_2_E ,27,0xFF} /* [0:7]: DIP [112:119] */
    ,{13, CPSS_DXCH_TTI_OFFSET_L3_MINUS_2_E ,28,0xFF} /* [0:7]: DIP [104:111]  */
    ,{14, CPSS_DXCH_TTI_OFFSET_L3_MINUS_2_E ,29,0xFF} /* [0:7]: DIP [96:103]     */
    ,{15, CPSS_DXCH_TTI_OFFSET_L3_MINUS_2_E ,30,0xFF} /* [0:7]: DIP [88:95]*/
    ,{16, CPSS_DXCH_TTI_OFFSET_L3_MINUS_2_E ,31,0xFF} /* [0:7]: DIP [80:87] */
    ,{17, CPSS_DXCH_TTI_OFFSET_L3_MINUS_2_E ,32,0xFF} /* [0:7]: DIP [72:79]    */
    ,{18, CPSS_DXCH_TTI_OFFSET_L3_MINUS_2_E, 33,0xFF} /* [0:7]: DIP [64:71]     */
    ,{19, CPSS_DXCH_TTI_OFFSET_L3_MINUS_2_E ,34,0xFF} /* [0:7]: DIP [56:63] */
    ,{20, CPSS_DXCH_TTI_OFFSET_L3_MINUS_2_E ,35,0xFF} /* [0:7]: DIP [48:55] */
    ,{21, CPSS_DXCH_TTI_OFFSET_L3_MINUS_2_E ,36,0xFF} /* [0:7]: DIP [40:47]  */
    ,{22, CPSS_DXCH_TTI_OFFSET_L3_MINUS_2_E, 37,0xFF} /* [0:7]: DIP [32:39]     */
    ,{23, CPSS_DXCH_TTI_OFFSET_L3_MINUS_2_E ,38,0xFF} /* [0:7]: DIP [24:31]*/
    ,{24, CPSS_DXCH_TTI_OFFSET_L3_MINUS_2_E ,39,0xFF} /* [0:7]: DIP [16:23] */
    ,{25, CPSS_DXCH_TTI_OFFSET_L3_MINUS_2_E ,40,0xFF} /* [0:7]: DIP [8:15]    */
    ,{26, CPSS_DXCH_TTI_OFFSET_L3_MINUS_2_E, 41,0xFF} /* [0:7]: DIP [0:7]     */
    /* must be last */
    ,{0, CPSS_DXCH_TTI_OFFSET_INVALID_E , 0, 0}
};

static PCL_UDB_STC pcl_IPV6_UDP_UdbInfo[] = {

    /* Bytes offset 0-1 in PCL Key are for PCL ID*/


    /* Bytes offset 2-4 in PCL Key/Pattern/Mask. */
    /* 3 bytes VXLAN Network Identifier (VNI), udbIndex  7 -9 */
    /* Offset 12-14 from start of UDP header */
    /* Offset TUNNEL_L3_MINUS_2, allow access tunnel L3 header, after tunnel termination. */
     {7 , CPSS_DXCH_PCL_OFFSET_TUNNEL_L3_MINUS_2_E, 54, 0xFF} /* [0:7]: VXLAN Network Identifier (VNI) [16:23]   */
    ,{8 , CPSS_DXCH_PCL_OFFSET_TUNNEL_L3_MINUS_2_E, 55, 0xFF} /* [0:7]: VXLAN Network Identifier (VNI) [8:15]   */
    ,{9 , CPSS_DXCH_PCL_OFFSET_TUNNEL_L3_MINUS_2_E, 56, 0xFF} /* [0:7]: VXLAN Network Identifier (VNI) [0:7]   */

    /* Bytes offset 5-20 in PCL Key/Pattern/Mask. */
    /* 16 bytes of Tunnel's Source IPv6 address, udbIndex 11-26. */
    /* offset 8-23 from L3 start, offsets 10-25 from L3-2 anchor */
    ,{11, CPSS_DXCH_PCL_OFFSET_TUNNEL_L3_MINUS_2_E, 10, 0xFF} /* [0:7]: SIP [120:127] */
    ,{12, CPSS_DXCH_PCL_OFFSET_TUNNEL_L3_MINUS_2_E, 11, 0xFF} /* [0:7]: SIP [112:119] */
    ,{13, CPSS_DXCH_PCL_OFFSET_TUNNEL_L3_MINUS_2_E, 12, 0xFF} /* [0:7]: SIP [104:111]  */
    ,{14, CPSS_DXCH_PCL_OFFSET_TUNNEL_L3_MINUS_2_E, 13, 0xFF} /* [0:7]: SIP [96:103]     */
    ,{15, CPSS_DXCH_PCL_OFFSET_TUNNEL_L3_MINUS_2_E, 14, 0xFF} /* [0:7]: SIP [88:95]*/
    ,{16, CPSS_DXCH_PCL_OFFSET_TUNNEL_L3_MINUS_2_E, 15, 0xFF} /* [0:7]: SIP [80:87] */
    ,{17, CPSS_DXCH_PCL_OFFSET_TUNNEL_L3_MINUS_2_E, 16, 0xFF} /* [0:7]: SIP [72:79]    */
    ,{18, CPSS_DXCH_PCL_OFFSET_TUNNEL_L3_MINUS_2_E, 17, 0xFF} /* [0:7]: SIP [64:71]     */
    ,{19, CPSS_DXCH_PCL_OFFSET_TUNNEL_L3_MINUS_2_E, 18, 0xFF} /* [0:7]: SIP [56:63] */
    ,{20, CPSS_DXCH_PCL_OFFSET_TUNNEL_L3_MINUS_2_E, 19, 0xFF} /* [0:7]: SIP [48:55] */
    ,{21, CPSS_DXCH_PCL_OFFSET_TUNNEL_L3_MINUS_2_E, 20, 0xFF} /* [0:7]: SIP [40:47]  */
    ,{22, CPSS_DXCH_PCL_OFFSET_TUNNEL_L3_MINUS_2_E, 21, 0xFF} /* [0:7]: SIP [32:39]     */
    ,{23, CPSS_DXCH_PCL_OFFSET_TUNNEL_L3_MINUS_2_E, 22, 0xFF} /* [0:7]: SIP [24:31]*/
    ,{24, CPSS_DXCH_PCL_OFFSET_TUNNEL_L3_MINUS_2_E, 23, 0xFF} /* [0:7]: SIP [16:23] */
    ,{25, CPSS_DXCH_PCL_OFFSET_TUNNEL_L3_MINUS_2_E, 24, 0xFF} /* [0:7]: SIP [8:15]    */
    ,{26, CPSS_DXCH_PCL_OFFSET_TUNNEL_L3_MINUS_2_E, 25, 0xFF} /* [0:7]: SIP [0:7]     */
    /* must be last */
    ,{0, CPSS_DXCH_PCL_OFFSET_INVALID_E , 0, 0}
};

static GT_STATUS prvVpnIpv6NNITtiPclConfigurationSet
(
  IN GT_U8  devNum
)
{
  GT_STATUS rc = GT_OK;
  CPSS_DXCH_TTI_KEY_TYPE_ENT     keyType;
  GT_U32                         udbIndex, index;
  CPSS_DXCH_TTI_OFFSET_TYPE_ENT  offsetType;
  GT_U32                         offset;

  CPSS_DXCH_PCL_LOOKUP_CFG_STC   lookupCfg; /* lookup configuration */
  CPSS_INTERFACE_INFO_STC        interfaceInfo; /* interface data: port, VLAN, or index */
  CPSS_PCL_LOOKUP_NUMBER_ENT     lookupNum = CPSS_PCL_LOOKUP_NUMBER_0_E;
  CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT  ruleFormat = VPN_IPV6_PCL_ipv6Key_CNS;
  CPSS_DXCH_PCL_PACKET_TYPE_ENT       packetType = VPN_IPV6_PCL_UDB_PACKET_TYPE_CNS;
  CPSS_PCL_DIRECTION_ENT              direction = CPSS_PCL_DIRECTION_INGRESS_E;
  CPSS_DXCH_PCL_UDB_SELECT_STC        udbSelect;
  GT_U32                              udbSelectidx;

  /****************************************************************************/
  /*********************** Key definition to be used for VXLAN (NNI) ******************/
  /****************************************************************************/
  /* VXLAN:
        Key = {Tunnel IPv6 DIP=egress node IP, IP Protocol (metadata) = UDP, UDP DPort=VXLAN}
    */
  keyType = CPSS_DXCH_TTI_KEY_UDB_IPV6_UDP_E;
  rc = cpssDxChTtiPclIdSet(devNum, keyType, VPN_TTI_KEY_UDB_IPV6_UDP_PCL_ID_CNS);
  if(rc != GT_OK)
    return rc;

  rc = cpssDxChTtiPacketTypeKeySizeSet(devNum, keyType, VPN_TTI_KEY_UDB_SIZE_CNS);
  if(rc != GT_OK)
    return rc;

  /* Configure UDb's for IPv4 UDP. */
  for(index = 0;  ; index++)
  {
    if(tti_IPV6_UDP_UdbInfo[index].offsetType == CPSS_DXCH_TTI_OFFSET_INVALID_E)
        break;
    udbIndex = tti_IPV6_UDP_UdbInfo[index].udbIndex;
    offsetType = tti_IPV6_UDP_UdbInfo[index].offsetType;
    offset      = tti_IPV6_UDP_UdbInfo[index].offset;
    rc = cpssDxChTtiUserDefinedByteSet(devNum, keyType, udbIndex, offsetType, offset);
    if(rc != GT_OK)
      return rc;
  }

  /* PCL Configuration to complete the lookup for VxLAN IPv6 tunnel header.
      Must use UDB for all 3 packet formats, because packet is tunnel terminated,
      but processing should continue on the tunnel header. */

   /* Enable PCL globally */
  rc = cpssDxChPclIngressPolicyEnable(devNum, GT_TRUE);
  if(rc != GT_OK)
    return rc;

  rc = cpssDxCh3PclTunnelTermForceVlanModeEnableSet(devNum, GT_FALSE);
  if(rc != GT_OK)
    return rc;

  /* set lookup 0_0 parameters
  IPv4 packets that will arrive to this port will use
  key of type CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E */
  osMemSet(&lookupCfg, 0, sizeof(CPSS_DXCH_PCL_LOOKUP_CFG_STC));
  osMemSet(&interfaceInfo, 0, sizeof(CPSS_INTERFACE_INFO_STC));

  lookupCfg.enableLookup = GT_TRUE;
  lookupCfg.dualLookup   = GT_FALSE;
  lookupCfg.pclId        = VPN_PCL_IPV6_VxLAN_PCL_ID_CNS;
  lookupCfg.groupKeyTypes.nonIpKey = ruleFormat;
  lookupCfg.groupKeyTypes.ipv4Key  = ruleFormat;
  lookupCfg.groupKeyTypes.ipv6Key  = ruleFormat;
  lookupCfg.udbKeyBitmapEnable     = GT_FALSE;
  interfaceInfo.type              = CPSS_INTERFACE_INDEX_E;
  interfaceInfo.index             = VPN_IPV6_PCL_CFG_TABLE_INDEX_CNS;

  rc = cpssDxChPclCfgTblSet(devNum,
                               &interfaceInfo,
                               direction,
                               lookupNum,
                               &lookupCfg);
  if(rc != GT_OK)
  {
    return rc;
  }

  osMemSet(&udbSelect, 0, sizeof(CPSS_DXCH_PCL_UDB_SELECT_STC));
  udbSelectidx = 2; /* Bytes offset 0-1 in PCL Key are for PCL ID*/

  /* VXLAN hedaer */
  /* 3 bytes VXLAN Network Identifier (VNI), udbIndex  7 -9 */
  /* Offset 12-14 from start of UDP header */
  /* Bytes offset 2-4 in PCL Key/Pattern/Mask. */
  while(1)
  {
    if(pcl_IPV6_UDP_UdbInfo[udbSelectidx-2].offsetType == CPSS_DXCH_PCL_OFFSET_INVALID_E)
        break;
    rc = cpssDxChPclUserDefinedByteSet(devNum, ruleFormat, packetType, direction,
                                           pcl_IPV6_UDP_UdbInfo[udbSelectidx-2].udbIndex,
                                           pcl_IPV6_UDP_UdbInfo[udbSelectidx-2].offsetType,
                                           pcl_IPV6_UDP_UdbInfo[udbSelectidx-2].offset);
    if(rc != GT_OK)
      return rc;
    udbSelect.udbSelectArr[udbSelectidx]=pcl_IPV6_UDP_UdbInfo[udbSelectidx-2].udbIndex;
    udbSelectidx++;
  }

  /* Configure PCL selectin for UDB only keys.
       Per packet type there are 70UDB that can be configured by the user.
       If rule format (configured by the cpssDxChPclCfgTblSet API), is one of
       CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E..CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_E,
       Than per key type user must configured which of the 60 bytes are selected.
       In this VxLan implemenetastion rule format is CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_E, and
       packet type (as will be configured in TTI action) defined by  VPN_IPV6_PCL_UDB_PACKET_TYPE_CNS */

  /* Select byte order in the key */
  udbSelect.ingrUdbReplaceArr[0] = GT_TRUE; /*  udbReplaceArr[0]  - UDB0  replaced with {PCL-ID[7:0]} */
  udbSelect.ingrUdbReplaceArr[1] = GT_TRUE; /*  udbReplaceArr[1]  - UDB1  replaced with {UDB Valid,reserved,PCL-ID[9:8]} */
  rc = cpssDxChPclUserDefinedBytesSelectSet(devNum, ruleFormat, packetType, lookupNum, &udbSelect);
  if(rc != GT_OK)
  {
    return rc;
  }

  return GT_OK;
}

/**
* @internal prvVpnVxlanNvgreGenericInit function
* @endinternal
*
* @brief   The function creates a User Network Interface (UNI).
*            Both UNI direction are configured - ingress and egress.
*            Ingress: Classify packet. Assign ingress ePort and VNI
*            Egress:  forward packet (after tunnel termination),
*                     with its attributes - tagged/untagged etc.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - parameter value more then HW bit field
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note See comments to CPSS_DXCH_PCL_OFFSET_TYPE_ENT
*
*/
static GT_STATUS prvVpnVxlanNvgreGenericInit
(
  IN GT_U8         devNum
)
{
  GT_STATUS rc = GT_OK;
  static GT_BOOL initDone = GT_FALSE;
  CPSS_DXCH_TUNNEL_START_GEN_IP_PROFILE_STC tsGenericProfile;
  GT_U32   i, sourceIdTtiOverrideBmp;

  if(initDone == GT_TRUE)
    return GT_OK;

  /* configures Generic Tunnel-start Profile table entry for VXLAN */
  osMemSet(&tsGenericProfile, 0, sizeof(tsGenericProfile));
  tsGenericProfile.templateDataSize = CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_SIZE_8B_E;

  /* Set only bits that are NOT const 0 */
  tsGenericProfile.templateDataBitsCfg[3] = CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_CONST_1_E;
  for(i=32 ; i<=39 ; i++)
    tsGenericProfile.templateDataBitsCfg[i] = CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_EVLAN_I_SID_2_E;
  for(i=40 ; i<=47 ; i++)
    tsGenericProfile.templateDataBitsCfg[i] = CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_EVLAN_I_SID_1_E;
  for(i=48 ; i<=55 ; i++)
    tsGenericProfile.templateDataBitsCfg[i] = CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_EVLAN_I_SID_0_E;

  tsGenericProfile.udpSrcPortMode = CPSS_DXCH_TUNNEL_START_UDP_SRC_PORT_ASSIGN_FROM_PACKET_HASH_VALUE_E;

  rc = cpssDxChTunnelStartGenProfileTableEntrySet(devNum, VPN_VXLAN_TS_GEN_PROFILE_ENTRY_CNS, &tsGenericProfile);
  if(rc != GT_OK)
    return rc;

  /* configures Generic Tunnel-start Profile table entry for NVGRE */
  osMemSet(&tsGenericProfile, 0, sizeof(tsGenericProfile));
  tsGenericProfile.templateDataSize = CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_SIZE_4B_E;

  /* Set only bits that are NOT const 0 */
  /* First 3 bytes contains the VSID (TNI) */
  for(i=0 ; i<=7 ; i++)
    tsGenericProfile.templateDataBitsCfg[i] = CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_EVLAN_I_SID_2_E;
  for(i=8 ; i<=15 ; i++)
    tsGenericProfile.templateDataBitsCfg[i] = CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_EVLAN_I_SID_1_E;
  for(i=16 ; i<=23 ; i++)
    tsGenericProfile.templateDataBitsCfg[i] = CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_EVLAN_I_SID_0_E;
  /* 4th byte with constant vlaue 0x5A*/
  for(i=24 ; i<=27 ; i++)
    tsGenericProfile.templateDataBitsCfg[i] = (i%2)?
                                               CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_CONST_1_E:
                                               CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_CONST_0_E;
  for(i=28 ; i<=31 ; i++)
    tsGenericProfile.templateDataBitsCfg[i] = (i%2)?
                                               CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_CONST_0_E:
                                               CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_CONST_1_E;

  tsGenericProfile.udpSrcPortMode = CPSS_DXCH_TUNNEL_START_UDP_SRC_PORT_ASSIGN_FROM_PACKET_HASH_VALUE_E;

  rc = cpssDxChTunnelStartGenProfileTableEntrySet(devNum, VPN_NVGRE_TS_GEN_PROFILE_ENTRY_CNS, &tsGenericProfile);
  if(rc != GT_OK)
    return rc;

  /* Enable GRE extensions */
  rc = cpssDxChTtiGreExtensionsCheckEnableSet(devNum,GT_TRUE);
  if(rc != GT_OK)
    return rc;

  /* Split horizon configuration: use bit 11 of rource-id for split horizon */
  rc = cpssDxChTtiSourceIdBitsOverrideGet(devNum, VPN_TTI_LOOKUP_CNS, &sourceIdTtiOverrideBmp);
  if(rc != GT_OK)
  return rc;

  sourceIdTtiOverrideBmp |= (1<<VPN_MESH_ID_BIT_IN_SOURCE_ID_CNS);
  rc = cpssDxChTtiSourceIdBitsOverrideSet(devNum, VPN_TTI_LOOKUP_CNS, sourceIdTtiOverrideBmp);
  if(rc != GT_OK)
    return rc;

  rc = cpssDxChBrgEgrMeshIdConfigurationSet(devNum, GT_TRUE,
                                                  VPN_MESH_ID_BIT_IN_SOURCE_ID_CNS,
                                                  VPN_MESH_ID_BIT_SIZE_CNS);
  if(rc != GT_OK)
    return rc;


  /* eVidx general configuration */
  rc = cpssDxChL2MllLookupForAllEvidxEnableSet(devNum, GT_FALSE);
  if(rc != GT_OK)
    return rc;

  rc = prvVpnIpv4NNITtiConfigurationSet(devNum);
  if(rc != GT_OK)
    return rc;

  rc = prvVpnIpv6NNITtiPclConfigurationSet(devNum);
  if(rc != GT_OK)
    return rc;

  rc = prvVpnUNITtiConfigurationSet(devNum);
  if(rc != GT_OK)
    return rc;

  /* Configure TPID profile to recognize 0x8100 only as TAG1 */
  /* Set TPID entry 0 ingress and egress to 0x8100 */
  rc = cpssDxChBrgVlanTpidEntrySet(devNum, CPSS_DIRECTION_INGRESS_E,0 ,0x8100);
  if(rc != GT_OK)
    return rc;

  rc = cpssDxChBrgVlanTpidEntrySet(devNum, CPSS_DIRECTION_INGRESS_E,1 ,0x88A8);
  if(rc != GT_OK)
    return rc;

  rc = cpssDxChBrgVlanTpidEntrySet(devNum, CPSS_DIRECTION_EGRESS_E,0 ,0x8100);
  if(rc != GT_OK)
    return rc;

  rc = cpssDxChBrgVlanTpidEntrySet(devNum, CPSS_DIRECTION_EGRESS_E,1 ,0x88A8);
  if(rc != GT_OK)
    return rc;

  /* Set default profile bitmap to include 0x8100 only for ETH1 (tag1), ETH0 empty */
  rc = cpssDxChBrgVlanIngressTpidProfileSet(devNum, VPN_DEAFULT_TPID_PROFILE_CNS, CPSS_VLAN_ETHERTYPE0_E, 0);
  if(rc != GT_OK)
    return rc;
  rc = cpssDxChBrgVlanIngressTpidProfileSet(devNum, VPN_DEAFULT_TPID_PROFILE_CNS, CPSS_VLAN_ETHERTYPE1_E, 1);
  if(rc != GT_OK)
    return rc;

  /* Set empty profile bitmap to 0. Not include ant TPID. */
  rc = cpssDxChBrgVlanIngressTpidProfileSet(devNum, VPN_EMPTY_TPID_PROFILE_CNS, CPSS_VLAN_ETHERTYPE0_E, 0);
  if(rc != GT_OK)
    return rc;
  rc = cpssDxChBrgVlanIngressTpidProfileSet(devNum, VPN_EMPTY_TPID_PROFILE_CNS, CPSS_VLAN_ETHERTYPE1_E, 0);
  if(rc != GT_OK)
    return rc;

  initDone = GT_TRUE;
  return GT_OK;
}

/**
* @internal csRefVpnMac2MeSet function
* @endinternal
*
* @brief   Set mac2me address entry.
*          Mac2me is one of the condition to trigger VxLAN packet processing
*          ingress UNI or NNI. This MAC address is also the source MAC address
*          of packet that sent to core after encapsulation.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum           - device number
* @param[in] mac2me   - Mac2me to trigger VxLAN packet processing
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - parameter value more then HW bit field
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note
*
*/
GT_STATUS csRefVpnMac2MeSet
(
  IN GT_U8          devNum,
  IN GT_ETHERADDR  *mac2me
)
{
    GT_STATUS rc = GT_OK;
    CPSS_DXCH_TTI_MAC_VLAN_STC mac2MeVlanPattern,mac2MeVlanMask;
    CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC interfaceInfo;

    /* Mac to me configuration - used for UNI and NNI classification. */
    osMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));
    osMemSet(&mac2MeVlanPattern, 0, sizeof(mac2MeVlanPattern));
    osMemSet(&mac2MeVlanMask, 0, sizeof(mac2MeVlanMask));
    mac2MeVlanPattern.mac = *mac2me;
    osMemSet(&mac2MeVlanMask.mac, 0xFF, 6);
    rc = cpssDxChTtiMacToMeSet(devNum, VPN_TTI_MAC2ME_ENTRY_INDEX_CNS,
                                  &mac2MeVlanPattern, &mac2MeVlanMask, &interfaceInfo);
    if(rc != GT_OK)
      return rc;

    rc = cpssDxChIpRouterGlobalMacSaSet(devNum, VPN_VXLAN_GLOBAL_MAC_SA_ENTRY_CNS, mac2me);
    if(rc != GT_OK)
      return rc;

    return rc;
}

/**
* @internal csRefVpnMac2MeDelete function
* @endinternal
*
* @brief   Delete mac2me address entry.
*            Mac2me is one of the condition to trigger VxLAN packet processing ingress UNI or NNI.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum           - device number
* @param[in] assignEportNum   - ePort to assigned the NNI
*
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - parameter value more then HW bit field
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note
*
*/
GT_STATUS csRefVpnMac2MeDelete
(
  IN GT_U8          devNum
)
{
    GT_STATUS rc = GT_OK;
    CPSS_DXCH_TTI_MAC_VLAN_STC mac2MeVlanPattern,mac2MeVlanMask;
    CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC interfaceInfo;

    /* Mac to me configuration - used for UNI and NNI classification. */
    osMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));
    osMemSet(&mac2MeVlanPattern, 0, sizeof(mac2MeVlanPattern));
    osMemSet(&mac2MeVlanMask, 0, sizeof(mac2MeVlanMask));
    osMemSet(&mac2MeVlanMask.mac, 0xFF, 6);
    rc = cpssDxChTtiMacToMeSet(devNum, VPN_TTI_MAC2ME_ENTRY_INDEX_CNS,
                                  &mac2MeVlanPattern, &mac2MeVlanMask, &interfaceInfo);
    if(rc != GT_OK)
      return rc;

    rc = cpssDxChIpRouterGlobalMacSaSet(devNum, VPN_VXLAN_GLOBAL_MAC_SA_ENTRY_CNS, &(mac2MeVlanPattern.mac));
    if(rc != GT_OK)
      return rc;

    return rc;
}

/**
* @internal csRefVpnUniInterfaceCreate function
* @endinternal
*
* @brief   The function creates a User Network Interface (UNI).
*            Both UNI direction are configured - ingress (to device)
*            and egress (from device) processing.
*            Ingress: Classify packet. Assign ingress ePort and VNI
*            Egress:  forward packet (after tunnel termination),
*                     with its attributes - tagged/untagged etc.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum          - device number
* @param[in] portNum         - port number
* @param[in] vlanId          - packet assigned vlan id.
* @param[in] macAddress      - Packet's source mac address.
*                              if macAddress is all 0, ignore it.
* @param[in] serviceId       - Segment id, VSI/VNI/TNI.
* @param[in] assignedEVlan   - eVlan to represent the VSI domain.
* @param[in] assignEportNum  - ePort to assigned the UNI
*
* @retval GT_OK                     - on success
* @retval GT_BAD_PARAM              - on wrong parameters
* @retval GT_OUT_OF_RANGE           - parameter value more then HW bit field
* @retval GT_NOT_APPLICABLE_DEVICE  - on not applicable device
* @retval GT_FAIL                   - otherwise
*
* @note
*
*/
GT_STATUS csRefVpnUniInterfaceCreate
(
  IN GT_U8         devNum,
  IN GT_PORT_NUM   portNum,
  IN GT_U16        vlanId,
  IN GT_ETHERADDR *macAddress,
  IN GT_U32        serviceId,
  IN GT_U16        assignedEVlan,
  IN GT_PORT_NUM   assignEportNum
)
{
  GT_STATUS rc = GT_OK;
  GT_U32                         entryIndex, udbIndex;
  CPSS_DXCH_TTI_KEY_TYPE_ENT     keyType   = CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E;
  GT_U32                         pclId = VPN_TTI_KEY_UDB_ETHERNET_OTHER_PCL_ID_CNS;
  CPSS_DXCH_TTI_RULE_TYPE_ENT    ruleType=CPSS_DXCH_TTI_RULE_UDB_30_E;
  CPSS_DXCH_TTI_RULE_UNT         ttiPattern, ttiMask;
  CPSS_DXCH_TTI_ACTION_STC       ttiAction;
  CPSS_INTERFACE_INFO_STC        physicalInfo;
  GT_HW_DEV_NUM                  hwDevNum;
  EPORT_INFOMATION               *userInfoPtr;
  CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC  egressEportInfo;

  GT_UNUSED_PARAM(serviceId);

  rc = prvVpnVxlanNvgreGenericInit(devNum);
  if(rc != GT_OK)
    return rc;

  userInfoPtr = osMalloc(sizeof(EPORT_INFOMATION));
  if(userInfoPtr  == NULL)
    return GT_OUT_OF_CPU_MEM;
  osMemSet(userInfoPtr, 0xFF, sizeof(EPORT_INFOMATION));


  /* enable ETH TTI lookup for portNum */
  rc = cpssDxChTtiPortLookupEnableSet(devNum, portNum, keyType, GT_TRUE);
  if(rc != GT_OK)
    return rc;

  osMemSet(&ttiPattern, 0, sizeof(ttiPattern));
  osMemSet(&ttiMask, 0, sizeof(ttiMask));
  osMemSet(&ttiAction, 0, sizeof(ttiAction));

  /* bits 0:4 of the byte are bits 0:4 of PCL id,
     bit 5 of the byte is "Local Device Source is Trunk" indication,
     bit 7 of the byte is DA-MAC2ME indication. */
  /* first 5 bits of PCL id, bit 5 isTrunk=False, bit 7 macToMe=True */
  ttiPattern.udbArray.udb[0] = (pclId&0x1F) + 0 +(1<<7);
  ttiMask.udbArray.udb[0] = 0xBF; /* bits 0-5,7 */

  /* bits 0:4 of the byte are bits 5:9 of PCL id */
  ttiPattern.udbArray.udb[1] = ((pclId>>5)&0x1F);
  ttiMask.udbArray.udb[1] = 0x1F; /* bits 0-4 */

  /* bits 0:7 of the byte are bits 0:7 of ingress source eport */
  ttiPattern.udbArray.udb[2] = (portNum&0xFF);
  ttiMask.udbArray.udb[2] = 0xFF; /* bits 0-5 */
  /* bits 0:4 of the byte are bits 8:12 of ingress source eport */
  ttiPattern.udbArray.udb[3] = ((portNum>>8)&0x1F);
  ttiMask.udbArray.udb[3] = 0x1F; /* bits 0-5 */

  /* 2 bytes of tag1 (c-tag) of packet, udbIndex  9-10 */
  ttiPattern.udbArray.udb[9] = (vlanId&0xFF); /* match bits 0:7 of c-tag */
  ttiMask.udbArray.udb[9] = 0xFF;
  /* bits 0:4 of the byte are bits 8:12 of ingress source eport */
  ttiPattern.udbArray.udb[10] = ((vlanId>>8)&0xF); /* bits 0:3 of udb match bits 8:11 of c-tag */
  ttiMask.udbArray.udb[10] = 0xF;

  /* 6 bytes source MAC address, udbIndex  11-16 */
  for(udbIndex = 0; udbIndex < 6 ; udbIndex++)
    if(macAddress->arEther[udbIndex] != 0)
       break;
  if(udbIndex < 6)
  {
    for(udbIndex = 11; udbIndex < 17 ; udbIndex++)
    {
      ttiPattern.udbArray.udb[udbIndex] = macAddress->arEther[udbIndex-11];
      ttiMask.udbArray.udb[udbIndex] = 0xFF;
    }
  }

  ttiAction.command                     = CPSS_PACKET_CMD_FORWARD_E;
  ttiAction.tag0VlanCmd                 = CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E;
  ttiAction.tag0VlanId                  = assignedEVlan;
  ttiAction.tag1VlanCmd                 = CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E;
  ttiAction.tag1VlanId                  = 0;
  ttiAction.sourceEPortAssignmentEnable = GT_TRUE;
  ttiAction.sourceEPort                 = assignEportNum;
  ttiAction.sourceIdSetEnable           = GT_TRUE;
  ttiAction.sourceId                    = (0<<VPN_MESH_ID_BIT_IN_SOURCE_ID_CNS);

  rc = csRefInfraIndexDbOp(VPN_EARCH_DB_TYPE_TTI_E, VPN_INDEX_DB_OPERATION_ALLOC_E , &entryIndex);
  if(rc != GT_OK)
    return rc;

  rc = cpssDxChTtiRuleSet(devNum, entryIndex, ruleType, &ttiPattern, &ttiMask, &ttiAction);
  if(rc != GT_OK)
    return rc;
  userInfoPtr->ttiRuleEntryIndex = entryIndex;


  /* Clean ePort configuration history.  */
  osMemSet(&egressEportInfo, 0, sizeof(egressEportInfo));
  rc = cpssDxChBrgEportToPhysicalPortEgressPortInfoSet(devNum, assignEportNum, &egressEportInfo);
  if(rc != GT_OK)
    return rc;

  /* Set mesh Id for eport (split horizon) to 0  */
  rc = cpssDxChBrgEgrPortMeshIdSet(devNum, assignEportNum, 0);
  if(rc != GT_OK)
    return rc;

  /* Set physical interace for the assigned eport */
  rc = cpssDxChCfgHwDevNumGet(devNum, &hwDevNum);
  if(rc != GT_OK)
    return rc;

  physicalInfo.type             = CPSS_INTERFACE_PORT_E;
  physicalInfo.devPort.hwDevNum = hwDevNum;
  physicalInfo.devPort.portNum  = portNum;
  rc = cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(devNum, assignEportNum, &physicalInfo);
  if(rc != GT_OK)
    return rc;

   /* Disable egress vlan filterring on eport */
   rc = cpssDxChBrgEgrFltVlanEPortFilteringEnableSet(devNum, assignEportNum, GT_FALSE);
   if((rc != GT_OK) && (rc != GT_NOT_APPLICABLE_DEVICE))
     return rc;

  /* Take egress tag state from eport and not from evlan */
  rc = cpssDxChBrgVlanEgressPortTagStateModeSet(devNum, assignEportNum, CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_EPORT_MODE_E);
  if(rc != GT_OK)
    return rc;

  /* Set eport egress tag state to TAG1 by default */
  rc = cpssDxChBrgVlanEgressPortTagStateSet(devNum, assignEportNum, CPSS_DXCH_BRG_VLAN_PORT_TAG1_CMD_E);
  if(rc != GT_OK)
    return rc;

  /* Send packet with Tag1 only if received with Tag1. */
  rc = cpssDxChBrgVlanPortRemoveTag1IfRxWithoutTag1EnableSet(devNum, assignEportNum, GT_TRUE);
  if(rc != GT_OK)
    return rc;

  /* Set TPID to profile that recognize 0x8100 only as TAG1 for physical port (no need for assigned eport) */
  rc = cpssDxChBrgVlanPortIngressTpidProfileSet(devNum, portNum, CPSS_VLAN_ETHERTYPE0_E, GT_TRUE, VPN_DEAFULT_TPID_PROFILE_CNS);
  if(rc != GT_OK)
    return rc;
  rc = cpssDxChBrgVlanPortIngressTpidProfileSet(devNum, portNum, CPSS_VLAN_ETHERTYPE1_E, GT_TRUE, VPN_DEAFULT_TPID_PROFILE_CNS);
  if(rc != GT_OK)
    return rc;

  /* To prevent eVlan egress filterring add physical port to the VSI domain assigned eVlan. */
  rc = cpssDxChBrgVlanMemberAdd(devNum, assignedEVlan, portNum, GT_FALSE, CPSS_DXCH_BRG_VLAN_PORT_DO_NOT_MODIFY_TAG_CMD_E);
  if(rc != GT_OK)
    return rc;

  /* Save user info for clean up */
  userInfoPtr->ipProt = CPSS_IP_PROTOCOL_ALL_E; /* Indicate ethernet key type */
  userInfoPtr->portNum = portNum;
  rc = csRefInfraIndexDbUserInfoSet(VPN_EARCH_DB_TYPE_EPORT_E, assignEportNum, (void *)userInfoPtr);
  if(rc != GT_OK)
    return rc;

  return GT_OK;
}

/**
* @internal csRefVpnVxlanNniInterfaceCreate function
* @endinternal
*
* @brief   The function creates a VxLan Network Network Interface (NNI).
*            Both NNI direction are configured - ingress (to device)
*            and egress (from device) processing.
*            Ingress: Packet classification, Tunnel Termination, ingress ePort and eVlan assignment.
*            Egress:  Add tunnel encapsulation according to ePort, and forward packet to core network.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum            - device number
* @param[in] portNum           - port number
* @param[in] localIpAddress    - Array of local device IP address.
*                                It is the SIP in ingress packet
*                                It is the DIP in egress packet
* @param[in] remoteIpAddress   - Array of remote device IP address.
*                                It is the DIP in ingress packet
*                                It is the SIP in egress packet
* @param[in] ipProt            - VPN Ip protocol IPv4 or IPv6
* @param[in] serviceId         - Segment id, VSI/VNI/TNI.
* @param[in] assignedEVlan     - eVlan to represent the VSI domain.
* @param[in] assignEportNum    - ePort to assigned the NNI
* @param[in] innerPacketTagged - Encapsulated packet egress with tagged or not.
* @param[in] nhMacAddress      - destination mac address of egress traffic 
*                                through assigned assignEportNum
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - parameter value more then HW bit field
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note
*
*/
GT_STATUS csRefVpnVxlanNniInterfaceCreate
(
  IN GT_U8                       devNum,
  IN GT_PORT_NUM                 portNum,
  IN GT_U8                      *localIpAddress,
  IN GT_U8                      *remoteIpAddress,
  IN CPSS_IP_PROTOCOL_STACK_ENT  ipProt,
  IN GT_U32                      serviceId,
  IN GT_U16                      assignedEVlan,
  IN GT_PORT_NUM                 assignEportNum,
  IN GT_BOOL                     innerPacketTagged,
  IN GT_ETHERADDR               *nhMacAddress
)
{
  GT_STATUS                   rc = GT_OK;
  GT_U32                      entryIndex, udbIndex;
  CPSS_DXCH_TTI_KEY_TYPE_ENT  keyType   = CPSS_DXCH_TTI_KEY_UDB_IPV4_UDP_E;
  GT_U32                      pclId     = VPN_TTI_KEY_UDB_IPV4_UDP_PCL_ID_CNS;
  CPSS_DXCH_TTI_RULE_TYPE_ENT ruleType  = CPSS_DXCH_TTI_RULE_UDB_30_E;
  CPSS_DXCH_TTI_RULE_UNT      ttiPattern, ttiMask;
  CPSS_DXCH_TTI_ACTION_STC    ttiAction;
  CPSS_INTERFACE_INFO_STC     physicalInfo;
  GT_HW_DEV_NUM               hwDevNum;
  GT_U32                      routerArpTunnelStartLineIndex;

  CPSS_DXCH_TUNNEL_START_CONFIG_UNT   tsConfig;
  CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC  egressEportInfo;
  EPORT_INFOMATION                   *userInfoPtr;


  if((ipProt != CPSS_IP_PROTOCOL_IPV4_E) && (ipProt != CPSS_IP_PROTOCOL_IPV6_E))
    return GT_BAD_PARAM;

  rc = prvVpnVxlanNvgreGenericInit(devNum);
  if(rc != GT_OK)
    return rc;

  userInfoPtr = osMalloc(sizeof(EPORT_INFOMATION));
  if(userInfoPtr  == NULL)
   return GT_OUT_OF_CPU_MEM;
  osMemSet(userInfoPtr, 0xFF, sizeof(EPORT_INFOMATION));

  /***************** TTI Configuration ****************/

  /* depend on protocol type,
     enable UDB IPv4 UDP TTI lookup OR
     enable UDB IPv6 UDP TTI lookup on port
  */
  if(ipProt == CPSS_IP_PROTOCOL_IPV6_E)
  {
    keyType = CPSS_DXCH_TTI_KEY_UDB_IPV6_UDP_E;
    pclId     = VPN_TTI_KEY_UDB_IPV6_UDP_PCL_ID_CNS;
  }
  rc = cpssDxChTtiPortLookupEnableSet(devNum, portNum, keyType, GT_TRUE);
  if(rc != GT_OK)
    return rc;

  osMemSet(&ttiPattern, 0, sizeof(ttiPattern));
  osMemSet(&ttiMask, 0, sizeof(ttiMask));
  osMemSet(&ttiAction, 0, sizeof(ttiAction));

  /* bits 0:4 of the byte are bits 0:4 of PCL id,
     bit 5 of the byte is "Local Device Source is Trunk" indication,
     bit 7 of the byte is DA-MAC2ME indication. */
  /* first 5 bits of PCL id, bit 5 isTrunk=False, bit 7 macToMe=True */
  ttiPattern.udbArray.udb[0] = (pclId&0x1F) + 0 +(1<<7);
  ttiMask.udbArray.udb[0] = 0xBF; /* bits 0-5,7 */

  /* bits 0:4 of the byte are bits 5:9 of PCL id */
  ttiPattern.udbArray.udb[1] = ((pclId>>5)&0x1F);
  ttiMask.udbArray.udb[1] = 0x1F; /* bits 0-4 */

  /* bits 0:7 of the byte are bits 0:7 of ingress source eport */
  ttiPattern.udbArray.udb[2] = (portNum&0xFF);
  ttiMask.udbArray.udb[2] = 0xFF; /* bits 0-5 */
  /* bits 0:4 of the byte are bits 8:12 of ingress source eport */
  ttiPattern.udbArray.udb[3] = ((portNum>>8)&0x1F);
  ttiMask.udbArray.udb[3] = 0x1F; /* bits 0-5 */

  /* Protocol in IP header should be 17 (UDP). */
  /* IPv4: Offset 9 in IP header. Stored in UDB 4. */
  /* IPv6: Offset 7 in IP header. Stored in UDB 4.*/
  ttiPattern.udbArray.udb[4] = 17;
  ttiMask.udbArray.udb[4] = 0xFF;

  /* 2 bytes destination udp port should be 4789=0x12B5, udbIndex  5,6  */
  /* Offset 2-3 from start of UDP header */
  udbIndex = 5;
  ttiPattern.udbArray.udb[udbIndex] = (0x12B5>>8)&0xFF;
  ttiMask.udbArray.udb[udbIndex] = 0xFF;
  udbIndex = 6;
  ttiPattern.udbArray.udb[udbIndex] = 0x12B5&0xFF;
  ttiMask.udbArray.udb[udbIndex] = 0xFF;

  /* For IPv6, this part is done in the PCL. */
  if(ipProt == CPSS_IP_PROTOCOL_IPV4_E)
  {
    /* 3 bytes VXLAN Network Identifier (VNI), udbIndex  7-9*/
    /* Offset 12-14 from start of UDP header */
    udbIndex = 7;
    ttiPattern.udbArray.udb[udbIndex] = ((serviceId>>16)&0xFF);
    ttiMask.udbArray.udb[udbIndex] = 0xFF;
    udbIndex = 8;
    ttiPattern.udbArray.udb[udbIndex] = ((serviceId>>8)&0xFF);
    ttiMask.udbArray.udb[udbIndex] = 0xFF;
    udbIndex = 9;
    ttiPattern.udbArray.udb[udbIndex] = (serviceId&0xFF);
    ttiMask.udbArray.udb[udbIndex] = 0xFF;
  }

  /* 1 bytes VXLAN flags, udbIndex  10 */
  /* 8 flags bit, first byte after udp header */
  /* Vxlan flags: bit 3 the  i bit must be set, other must be 0 */
  udbIndex = 10;
  ttiPattern.udbArray.udb[udbIndex] = 0x08;
  ttiMask.udbArray.udb[udbIndex] = 0xFF;

  if(ipProt == CPSS_IP_PROTOCOL_IPV4_E)
  {
    /* 4 bytes destination ip address, udbIndex  11-14 */
    for(udbIndex = 11; udbIndex < 15; udbIndex++)
    {
      ttiPattern.udbArray.udb[udbIndex] = localIpAddress[udbIndex - 11];
      ttiMask.udbArray.udb[udbIndex] = 0xFF;
    }
    /* 4 bytes source ip address, udbIndex  15-18 */
    for(udbIndex = 15; udbIndex < 19 ; udbIndex++)
    {
      ttiPattern.udbArray.udb[udbIndex] = remoteIpAddress[udbIndex - 15];
      ttiMask.udbArray.udb[udbIndex] = 0xFF;
    }
  }
  else if(ipProt == CPSS_IP_PROTOCOL_IPV6_E)
  {
    /* 16 bytes destination ip address, udbIndex  11-26 */
    for(udbIndex = 11; udbIndex < 27 ; udbIndex++)
    {
      ttiPattern.udbArray.udb[udbIndex] = localIpAddress[udbIndex - 11];
      ttiMask.udbArray.udb[udbIndex] = 0xFF;
    }
  }

  ttiAction.command                     = CPSS_PACKET_CMD_FORWARD_E;
  ttiAction.ttPassengerPacketType       = CPSS_DXCH_TTI_PASSENGER_ETHERNET_NO_CRC_E;
  ttiAction.tunnelTerminate             = GT_TRUE;
  ttiAction.sourceIdSetEnable           = GT_TRUE;
  ttiAction.sourceId                    = (1<<VPN_MESH_ID_BIT_IN_SOURCE_ID_CNS);
  if(ipProt == CPSS_IP_PROTOCOL_IPV4_E)
  {
    /* 0x8100 if exists in packet, then it is calssified as Tag1.*/
    ttiAction.tag0VlanCmd                 = CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E;
    ttiAction.tag0VlanId                  = assignedEVlan;
    ttiAction.sourceEPortAssignmentEnable = GT_TRUE;
    ttiAction.sourceEPort                 = assignEportNum;
  }
  if(ipProt == CPSS_IP_PROTOCOL_IPV6_E)
  {
    ttiAction.tag0VlanCmd                 = CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E;
    ttiAction.tag0VlanId                  = 0xFFF; /* Should be overwritten */
    ttiAction.tag1VlanCmd                 = CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E;
    ttiAction.tag1VlanId                  = 0xFFF; /* Should be overwritten */

    /* IPCL will assign source ePort and eVlan */
    ttiAction.pcl0OverrideConfigIndex = CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E;
    ttiAction.iPclConfigIndex         = VPN_IPV6_PCL_CFG_TABLE_INDEX_CNS;
    ttiAction.iPclUdbConfigTableEnable = GT_TRUE;
    ttiAction.iPclUdbConfigTableIndex  = VPN_IPV6_PCL_UDB_PACKET_TYPE_CNS;
  }

  rc = csRefInfraIndexDbOp(VPN_EARCH_DB_TYPE_TTI_E, VPN_INDEX_DB_OPERATION_ALLOC_E , &entryIndex);
  if(rc != GT_OK)
    return rc;

  rc = cpssDxChTtiRuleSet(devNum, entryIndex, ruleType, &ttiPattern, &ttiMask, &ttiAction);
  if(rc != GT_OK)
    return rc;
  userInfoPtr->ttiRuleEntryIndex = entryIndex;


  /************* IPCL configuration for IPv6 only ****************/

  if(ipProt == CPSS_IP_PROTOCOL_IPV6_E)
  {
    CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT  pclRuleFormat = VPN_IPV6_PCL_ipv6Key_CNS;
    CPSS_DXCH_PCL_RULE_OPTION_ENT       ruleOptionsBmp = CPSS_DXCH_PCL_RULE_OPTION_WRITE_DEFAULT_E;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT       pclRuleMask;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT       pclRulePattern;
    CPSS_DXCH_PCL_ACTION_STC            pclAction;

    rc = cpssDxChPclPortIngressPolicyEnable(devNum, portNum, GT_TRUE);
    if(rc != GT_OK)
      return rc;

    osMemSet(&pclRuleMask    , 0, sizeof(pclRuleMask));
    osMemSet(&pclRulePattern , 0, sizeof(pclRulePattern));
    osMemSet(&pclAction      , 0, sizeof(pclAction));

    /* As assigned by PCL configuration entry, indexed from TTI action above. */
    pclId = VPN_PCL_IPV6_VxLAN_PCL_ID_CNS;

    /* bits 0:7 of the byte are bits 0:7 pclId (muxed with port list) */
    pclRulePattern.ruleIngrUdbOnly.udb[0] = (pclId&0xFF);
    pclRuleMask.ruleIngrUdbOnly.udb[0] = 0xFF;

    /* bits 0:1 of the byte are bits 8:9 pclId (muxed with port list) */
    pclRulePattern.ruleIngrUdbOnly.udb[1] = ((pclId>>8)&0x3);
    pclRuleMask.ruleIngrUdbOnly.udb[1] = 0x3;

    /* 3 bytes VXLAN Network Identifier (VNI), udbIndex  7-9*/
    /* Offset 12-14 from start of UDP header */
    /* Bytes offset 2-4 in PCL Key/Pattern/Mask. */
    udbIndex = 2;
    pclRulePattern.ruleIngrUdbOnly.udb[udbIndex] = ((serviceId>>16)&0xFF);
    pclRuleMask.ruleIngrUdbOnly.udb[udbIndex] = 0xFF;
    udbIndex = 3;
    pclRulePattern.ruleIngrUdbOnly.udb[udbIndex] = ((serviceId>>8)&0xFF);
    pclRuleMask.ruleIngrUdbOnly.udb[udbIndex] = 0xFF;
    udbIndex = 4;
    pclRulePattern.ruleIngrUdbOnly.udb[udbIndex] = (serviceId&0xFF);
    pclRuleMask.ruleIngrUdbOnly.udb[udbIndex] = 0xFF;

    /* 16 bytes of Tunnel's Source IPv6 address, udbIndex 11-26. */
    /* offset 8-23 from L3 start, offsets 10-25 from L3-2 anchor */
    /* Bytes offset 5-20 in PCL Key/Pattern/Mask. */
    for(udbIndex = 5; udbIndex < 21 ; udbIndex++)
    {
      pclRulePattern.ruleIngrUdbOnly.udb[udbIndex] = remoteIpAddress[udbIndex - 5];
      pclRuleMask.ruleIngrUdbOnly.udb[udbIndex] = 0xFF;
    }

    pclAction.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
    pclAction.sourcePort.assignSourcePortEnable = GT_TRUE;
    pclAction.sourcePort.sourcePortValue = assignEportNum;
    pclAction.vlan.ingress.modifyVlan = CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;
    pclAction.vlan.ingress.vlanId = assignedEVlan;
    pclAction.vlan.ingress.precedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;

    rc = csRefInfraIndexDbOp(VPN_EARCH_DB_TYPE_PCL_E, VPN_INDEX_DB_OPERATION_ALLOC_E , &entryIndex);
    if(rc != GT_OK)
      return rc;

    rc = cpssDxChPclRuleSet(devNum, 0/*tcamIndex*/, pclRuleFormat, entryIndex, ruleOptionsBmp,
                            &pclRuleMask, &pclRulePattern, &pclAction);
    if(rc != GT_OK)
      return rc;
    userInfoPtr->pclRuleEntryIndex = entryIndex;
  }


  /* Egress Processing. */
  rc = csRefInfraIndexDbOp(VPN_EARCH_DB_TYPE_TS_E, VPN_INDEX_DB_OPERATION_ALLOC_E , &routerArpTunnelStartLineIndex);
  if(rc != GT_OK)
    return rc;
  userInfoPtr->tunnelStartEntryIndex = routerArpTunnelStartLineIndex;


  osMemSet(&tsConfig, 0, sizeof(tsConfig));
/*  tsConfig.ipv4Cfg.tagEnable;
     tsConfig.ipv4Cfg.vlanId;
*/

  if(ipProt == CPSS_IP_PROTOCOL_IPV4_E)
  {
    tsConfig.ipv4Cfg.macDa = *nhMacAddress;
    tsConfig.ipv4Cfg.dontFragmentFlag = GT_FALSE;
    tsConfig.ipv4Cfg.ttl = 64;
    for(entryIndex = 0 ; entryIndex < 4 ; entryIndex++)
    {
      tsConfig.ipv4Cfg.destIp.arIP[entryIndex] = remoteIpAddress[entryIndex];
      tsConfig.ipv4Cfg.srcIp.arIP[entryIndex]  = localIpAddress[entryIndex];
    }
    tsConfig.ipv4Cfg.ipHeaderProtocol = CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E;
    tsConfig.ipv4Cfg.profileIndex = VPN_VXLAN_TS_GEN_PROFILE_ENTRY_CNS;
    tsConfig.ipv4Cfg.udpDstPort = 4789;
    rc = cpssDxChTunnelStartEntrySet(devNum, routerArpTunnelStartLineIndex, CPSS_TUNNEL_GENERIC_IPV4_E, &tsConfig);
    if(rc != GT_OK)
      return rc;
  }
  else if(ipProt == CPSS_IP_PROTOCOL_IPV6_E)
  {
    tsConfig.ipv6Cfg.macDa = *nhMacAddress;
    tsConfig.ipv6Cfg.ttl = 64;
    for(entryIndex = 0 ; entryIndex < 16 ; entryIndex++)
    {
      tsConfig.ipv6Cfg.destIp.arIP[entryIndex] = remoteIpAddress[entryIndex];
      tsConfig.ipv6Cfg.srcIp.arIP[entryIndex]  = localIpAddress[entryIndex];
    }
    tsConfig.ipv6Cfg.ipHeaderProtocol = CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E;
    tsConfig.ipv6Cfg.profileIndex = VPN_VXLAN_TS_GEN_PROFILE_ENTRY_CNS;
    tsConfig.ipv6Cfg.udpDstPort = 4789;
    rc = cpssDxChTunnelStartEntrySet(devNum, routerArpTunnelStartLineIndex, CPSS_TUNNEL_GENERIC_IPV6_E, &tsConfig);
    if(rc != GT_OK)
      return rc;
  }


  /* Set mesh Id for eport (split horizon) to 0  */
  rc = cpssDxChBrgEgrPortMeshIdSet(devNum, assignEportNum, 1);
  if(rc != GT_OK)
    return rc;

  /* Assign TS attributes to assigned eport */
  osMemSet(&egressEportInfo, 0, sizeof(egressEportInfo));
  egressEportInfo.tunnelStart  = GT_TRUE;
  egressEportInfo.tunnelStartPtr = routerArpTunnelStartLineIndex;
  egressEportInfo.tsPassengerPacketType = CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E;
  rc = cpssDxChBrgEportToPhysicalPortEgressPortInfoSet(devNum, assignEportNum, &egressEportInfo);
  if(rc != GT_OK)
    return rc;

  /* Set physical interace to assigned eport */
  rc = cpssDxChCfgHwDevNumGet(devNum, &hwDevNum);
  if(rc != GT_OK)
    return rc;
  physicalInfo.type             = CPSS_INTERFACE_PORT_E;
  physicalInfo.devPort.hwDevNum = hwDevNum;
  physicalInfo.devPort.portNum  = portNum;
  rc = cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(devNum, assignEportNum, &physicalInfo);
  if(rc != GT_OK)
    return rc;

  /* Set SA mode for VxLAN encasulated packet - per physical port asisgnment */
  rc = cpssDxChIpPortRouterMacSaLsbModeSet(devNum, portNum, CPSS_SA_LSB_FULL_48_BIT_GLOBAL);
  if(rc != GT_OK)
    return rc;

  /* Set Global-MAC-SA-table index for encapsulated VxLan packets - per ePort asisgnment. */
  rc = cpssDxChIpRouterPortGlobalMacSaIndexSet(devNum, assignEportNum, VPN_VXLAN_GLOBAL_MAC_SA_ENTRY_CNS);
  if(rc != GT_OK)
    return rc;

  /* Take egress tag state from eport and not from evlan */
  rc = cpssDxChBrgVlanEgressPortTagStateModeSet(devNum, assignEportNum, CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_EPORT_MODE_E);
  if(rc != GT_OK)
    return rc;

  /* Set eport egress tag state to DO_NOT_MODIFY by default */
  if(innerPacketTagged == GT_TRUE)
  {
    rc = cpssDxChBrgVlanEgressPortTagStateSet(devNum, assignEportNum, CPSS_DXCH_BRG_VLAN_PORT_TAG1_CMD_E);
    if(rc != GT_OK)
      return rc;

    /* Send packet with Tag1 only if received with Tag1. */
    rc = cpssDxChBrgVlanPortRemoveTag1IfRxWithoutTag1EnableSet(devNum, assignEportNum, GT_TRUE);
    if(rc != GT_OK)
      return rc;
  }
  else
  {
      rc = cpssDxChBrgVlanEgressPortTagStateSet(devNum, assignEportNum, CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E);
      if(rc != GT_OK)
        return rc;
  }

  /* Disable egress vlan filterring on eport */
  rc = cpssDxChBrgEgrFltVlanEPortFilteringEnableSet(devNum, assignEportNum, GT_FALSE);
  if((rc != GT_OK) && (rc != GT_NOT_APPLICABLE_DEVICE))
    return rc;

  /* Set TPID to profile to not recongnize any tag. */
  rc = cpssDxChBrgVlanPortIngressTpidProfileSet(devNum, assignEportNum, CPSS_VLAN_ETHERTYPE0_E, GT_FALSE, VPN_EMPTY_TPID_PROFILE_CNS);
  if(rc != GT_OK)
    return rc;
  rc = cpssDxChBrgVlanPortIngressTpidProfileSet(devNum, assignEportNum, CPSS_VLAN_ETHERTYPE1_E, GT_FALSE, VPN_EMPTY_TPID_PROFILE_CNS);
  if(rc != GT_OK)
    return rc;

  /* To prevent eVlan egress filterring add physical port to the VSI domain assigned eVlan. */
  rc = cpssDxChBrgVlanMemberAdd(devNum, assignedEVlan, portNum, GT_FALSE, CPSS_DXCH_BRG_VLAN_PORT_DO_NOT_MODIFY_TAG_CMD_E);
  if(rc != GT_OK)
    return rc;

  /* Save user info for clean up */
  userInfoPtr->ipProt = ipProt;
  userInfoPtr->portNum = portNum;
  rc = csRefInfraIndexDbUserInfoSet(VPN_EARCH_DB_TYPE_EPORT_E, assignEportNum, (void *)userInfoPtr);
  if(rc != GT_OK)
    return rc;

  return GT_OK;
}


/**
* @internal csRefVpnNvgreNniInterfaceCreate function
* @endinternal
*
* @brief   The function creates an NVGRE Network Network Interface (NNI).
*            Both UNI direction are configured - ingress (to device)
*            Ingress: Classify packet. Tunnel Terminate packet, Assign ingress ePort and VNI
*            Egress:  Add tunnel encapsulation according to ePort, forward packet to core network.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum              - device number
* @param[in] portNum             - port number
* @param[in] localIpAddressU32   - IPv4 address of local device.
*                                  It is the SIP in ingress packet
*                                  It is the DIP in egress packet
*
* @param[in] remoteIpAddressU32  - IPv4 address of remote device.
*                                  It is the DIP in ingress packet
*                                  It is the SIP in egress packet
* @param[in] ipProt              - VPN Ip protocol IPv4 or IPv6
* @param[in] serviceId           - Segment id, VSI/VNI/TNI.
* @param[in] assignedEVlan       - eVlan to represent the VSI domain.
* @param[in] assignEportNum      - ePort to assigned the UNI
* @param[in] macAddress          - destination mac address of egress traffic through assigned assignEportNum
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - parameter value more then HW bit field
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - parameter value more then HW bit field
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note
*
*/
GT_STATUS csRefVpnNvgreNniInterfaceCreate
(
  IN GT_U8                        devNum,
  IN GT_PORT_NUM                  portNum,
  IN GT_U32                       localIpaddrU32,
  IN GT_U32                       remoteIpaddrU32,
  IN CPSS_IP_PROTOCOL_STACK_ENT   ipProt,
  IN GT_U32                       serviceId,
  IN GT_U16                       assignedEVlan,
  IN GT_PORT_NUM                  assignEportNum,
  IN GT_ETHERADDR                *macAddress
)
{
  GT_STATUS rc = GT_OK;
  GT_U32                       entryIndex, udbIndex;
  CPSS_DXCH_TTI_KEY_TYPE_ENT   keyType   = CPSS_DXCH_TTI_KEY_UDB_IPV4_OTHER_E;
  GT_U32                       pclId     = VPN_TTI_KEY_UDB_IPV4_OTHER_PCL_ID_CNS;
  CPSS_DXCH_TTI_RULE_TYPE_ENT  ruleType  = CPSS_DXCH_TTI_RULE_UDB_30_E;
  CPSS_DXCH_TTI_RULE_UNT       ttiPattern, ttiMask;
  CPSS_DXCH_TTI_ACTION_STC     ttiAction;
  CPSS_INTERFACE_INFO_STC      physicalInfo;
  GT_HW_DEV_NUM                hwDevNum;

  GT_U32 routerArpTunnelStartLineIndex;
/*  CPSS_TUNNEL_TYPE_ENT tunnelType; */
  CPSS_DXCH_TUNNEL_START_CONFIG_UNT tsConfig;
  CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC  egressEportInfo;
  GT_IPADDR     localIpaddr;
  GT_IPADDR     remoteIpaddr;
  EPORT_INFOMATION               *userInfoPtr;

  rc = prvVpnVxlanNvgreGenericInit(devNum);
  if(rc != GT_OK)
    return rc;

   userInfoPtr = osMalloc(sizeof(EPORT_INFOMATION));
   if(userInfoPtr  == NULL)
     return GT_OUT_OF_CPU_MEM;
   osMemSet(userInfoPtr, 0xFF, sizeof(EPORT_INFOMATION));


  /* enable UDB IPv4 UDP TTI lookup on port */
  rc = cpssDxChTtiPortLookupEnableSet(devNum, portNum, keyType, GT_TRUE);
  if(rc != GT_OK)
    return rc;

  localIpaddr.u32Ip  = osNtohl(localIpaddrU32);
  remoteIpaddr.u32Ip = osNtohl(remoteIpaddrU32);

  osMemSet(&ttiPattern, 0, sizeof(ttiPattern));
  osMemSet(&ttiMask, 0, sizeof(ttiMask));
  osMemSet(&ttiAction, 0, sizeof(ttiAction));

  /* bits 0:4 of the byte are bits 0:4 of PCL id,
     bit 5 of the byte is "Local Device Source is Trunk" indication,
     bit 7 of the byte is DA-MAC2ME indication. */
  /* first 5 bits of PCL id, bit 5 isTrunk=False, bit 7 macToMe=True */
  ttiPattern.udbArray.udb[0] = (pclId&0x1F) + 0 +(1<<7);
  ttiMask.udbArray.udb[0] = 0xBF; /* bits 0-5,7 */

  /* bits 0:4 of the byte are bits 5:9 of PCL id */
  ttiPattern.udbArray.udb[1] = ((pclId>>5)&0x1F);
  ttiMask.udbArray.udb[1] = 0x1F; /* bits 0-4 */

  /* bits 0:7 of the byte are bits 0:7 of ingress source eport */
  ttiPattern.udbArray.udb[2] = (portNum&0xFF);
  ttiMask.udbArray.udb[2] = 0xFF; /* bits 0-5 */
  /* bits 0:4 of the byte are bits 8:12 of ingress source eport */
  ttiPattern.udbArray.udb[3] = ((portNum>>8)&0x1F);
  ttiMask.udbArray.udb[3] = 0x1F; /* bits 0-5 */

  /* 4 bytes destination ip address, udbIndex  4-7 */
  for(udbIndex = 4; udbIndex < 8 ; udbIndex++)
  {
    ttiPattern.udbArray.udb[udbIndex] = localIpaddr.arIP[udbIndex - 4];
    ttiMask.udbArray.udb[udbIndex] = 0xFF;
  }

  /* 1 bytes to recognize IPv4 Tunneling protocol, udbIndex  8*/
  udbIndex = 8;
  ttiPattern.udbArray.udb[udbIndex] = 4<<1; /* IPv4 Tunneling Protocol: match bits 1-3 only */
  ttiMask.udbArray.udb[udbIndex] = 0xE;

  /* 3 bytes NVGRE Tenant Network Identifier (TNI), udbIndex  14-16*/
  /* Offset 12-14 from start of UDP header */
  udbIndex = 9;
  ttiPattern.udbArray.udb[udbIndex] = ((serviceId>>16)&0xFF);
  ttiMask.udbArray.udb[udbIndex] = 0xFF;
  udbIndex = 10;
  ttiPattern.udbArray.udb[udbIndex] = ((serviceId>>8)&0xFF);
  ttiMask.udbArray.udb[udbIndex] = 0xFF;
  udbIndex = 11;
  ttiPattern.udbArray.udb[udbIndex] = (serviceId&0xFF);
  ttiMask.udbArray.udb[udbIndex] = 0xFF;
  udbIndex = 12;
  ttiPattern.udbArray.udb[udbIndex] = 0x20; /* Only K bit is set. */
  ttiMask.udbArray.udb[udbIndex] = 0xFF;

  ttiAction.command                     = CPSS_PACKET_CMD_FORWARD_E;
  ttiAction.tag0VlanCmd                 = CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E;
  ttiAction.tag0VlanId                  = assignedEVlan;
  ttiAction.tag1VlanCmd                 = CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E;
  ttiAction.tag1VlanId                  = 0;
  ttiAction.ttPassengerPacketType       = CPSS_DXCH_TTI_PASSENGER_ETHERNET_NO_CRC_E;
  ttiAction.tag0VlanCmd                 = CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E;
  ttiAction.tunnelTerminate             = GT_TRUE;
  ttiAction.sourceEPortAssignmentEnable = GT_TRUE;
  ttiAction.sourceEPort                 = assignEportNum;
  ttiAction.sourceIdSetEnable           = GT_TRUE;
  ttiAction.sourceId                    = (1<<VPN_MESH_ID_BIT_IN_SOURCE_ID_CNS);

  rc = csRefInfraIndexDbOp(VPN_EARCH_DB_TYPE_TTI_E, VPN_INDEX_DB_OPERATION_ALLOC_E , &entryIndex);
  if(rc != GT_OK)
    return rc;

  rc = cpssDxChTtiRuleSet(devNum, entryIndex, ruleType, &ttiPattern, &ttiMask, &ttiAction);
  if(rc != GT_OK)
    return rc;

  rc = csRefInfraIndexDbOp(VPN_EARCH_DB_TYPE_TS_E, VPN_INDEX_DB_OPERATION_ALLOC_E , &routerArpTunnelStartLineIndex);
  if(rc != GT_OK)
    return rc;

  osMemSet(&tsConfig, 0, sizeof(tsConfig));
  tsConfig.ipv4Cfg.macDa = *macAddress;
  tsConfig.ipv4Cfg.dontFragmentFlag = GT_FALSE;
  tsConfig.ipv4Cfg.ttl = 64;
  tsConfig.ipv4Cfg.destIp = remoteIpaddr;
  tsConfig.ipv4Cfg.srcIp  = localIpaddr;
  tsConfig.ipv4Cfg.ipHeaderProtocol = CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E;
  tsConfig.ipv4Cfg.profileIndex = VPN_NVGRE_TS_GEN_PROFILE_ENTRY_CNS;
  tsConfig.ipv4Cfg.greProtocolForEthernet= 0x6558;
  tsConfig.ipv4Cfg.greFlagsAndVersion = 0x2000; /* Set K bit only, bit 5 of high byte. */
  rc = cpssDxChTunnelStartEntrySet(devNum, routerArpTunnelStartLineIndex, CPSS_TUNNEL_GENERIC_IPV4_E, &tsConfig);
  if(rc != GT_OK)
    return rc;


  /* Set mesh Id for eport (split horizon) to 0  */
  rc = cpssDxChBrgEgrPortMeshIdSet(devNum, assignEportNum, 1);
  if(rc != GT_OK)
    return rc;

  /* Assign TS attributes to assigned eport */
  osMemSet(&egressEportInfo, 0, sizeof(egressEportInfo));
  egressEportInfo.tunnelStart  = GT_TRUE;
  egressEportInfo.tunnelStartPtr = routerArpTunnelStartLineIndex;
  egressEportInfo.tsPassengerPacketType = CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E;
  rc = cpssDxChBrgEportToPhysicalPortEgressPortInfoSet(devNum, assignEportNum, &egressEportInfo);
  if(rc != GT_OK)
    return rc;

  /* Set physical interace to assigned eport */
  rc = cpssDxChCfgHwDevNumGet(devNum, &hwDevNum);
  if(rc != GT_OK)
    return rc;

  physicalInfo.type             = CPSS_INTERFACE_PORT_E;
  physicalInfo.devPort.hwDevNum = hwDevNum;
  physicalInfo.devPort.portNum  = portNum;
  rc = cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(devNum, assignEportNum, &physicalInfo);
  if(rc != GT_OK)
    return rc;

  /* Take egress tag state from eport and not from evlan */
  rc = cpssDxChBrgVlanEgressPortTagStateModeSet(devNum, assignEportNum, CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_EPORT_MODE_E);
  if(rc != GT_OK)
    return rc;

  /* Set eport egress tag state to DO_NOT_MODIFY by default */
  rc = cpssDxChBrgVlanEgressPortTagStateSet(devNum, assignEportNum, CPSS_DXCH_BRG_VLAN_PORT_DO_NOT_MODIFY_TAG_CMD_E);
  if(rc != GT_OK)
    return rc;

  /* Send packet with Tag1 only if received with Tag1. */
  rc = cpssDxChBrgVlanPortRemoveTag1IfRxWithoutTag1EnableSet(devNum, assignEportNum, GT_TRUE);
  if(rc != GT_OK)
    return rc;

  /* Disable egress vlan filterring on eport */
  rc = cpssDxChBrgEgrFltVlanEPortFilteringEnableSet(devNum, assignEportNum, GT_FALSE);
  if((rc != GT_OK) && (rc != GT_NOT_APPLICABLE_DEVICE))
    return rc;

  /* Set TPID to profile that recognize 0x8100 only as TAG1 for assigned eport (no need for physical eport) */
  rc = cpssDxChBrgVlanPortIngressTpidProfileSet(devNum, assignEportNum, CPSS_VLAN_ETHERTYPE0_E, GT_FALSE, VPN_EMPTY_TPID_PROFILE_CNS);
  if(rc != GT_OK)
    return rc;
  rc = cpssDxChBrgVlanPortIngressTpidProfileSet(devNum, assignEportNum, CPSS_VLAN_ETHERTYPE1_E, GT_FALSE, VPN_EMPTY_TPID_PROFILE_CNS);
  if(rc != GT_OK)
    return rc;

  /* To prevent eVlan egress filterring add physical port to the VSI domain assigned eVlan. */
  rc = cpssDxChBrgVlanMemberAdd(devNum, assignedEVlan, portNum, GT_FALSE, CPSS_DXCH_BRG_VLAN_PORT_DO_NOT_MODIFY_TAG_CMD_E);
  if(rc != GT_OK)
    return rc;

  rc = csRefInfraIndexDbUserInfoSet(VPN_EARCH_DB_TYPE_EPORT_E, assignEportNum, (void *)userInfoPtr);
  if(rc != GT_OK)
    return rc;

  /* Save user info for clean up */
  userInfoPtr->ipProt = ipProt;
  userInfoPtr->portNum = portNum;
  rc = csRefInfraIndexDbUserInfoSet(VPN_EARCH_DB_TYPE_EPORT_E, assignEportNum, (void *)userInfoPtr);
  if(rc != GT_OK)
    return rc;

  return GT_OK;
}

/**
* @internal csRefVpnInterfaceDelete function
* @endinternal
*
* @brief   The function clean all VPN related configuration that done by
*          the UNI and NNI create APIs, and remove it from DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum           - device number
* @param[in] assignEportNum   - ePort to assigned the NNI or UNI
*
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - parameter value more then HW bit field
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note
*
*/
GT_STATUS csRefVpnInterfaceDelete
(
  IN GT_U8         devNum,
  IN GT_PORT_NUM   assignEportNum
)
{
   GT_STATUS                          rc = GT_OK;
   EPORT_INFOMATION                  *userInfoPtr=NULL;
   CPSS_DXCH_TUNNEL_START_CONFIG_UNT  tsConfig;
   CPSS_INTERFACE_INFO_STC            physicalInfo;
   CPSS_DXCH_TTI_KEY_TYPE_ENT         keyType;
   CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC egressEportInfo;
   GT_PORT_NUM   portNum;  /* physical ingress eport */

   rc = csRefInfraIndexDbUserInfoGet(VPN_EARCH_DB_TYPE_EPORT_E, assignEportNum, (void **)(&userInfoPtr));
   if((rc != GT_OK) || (userInfoPtr == NULL))
     return GT_NO_SUCH;

   /* Disable TTI lookup */
   if(userInfoPtr->ipProt == CPSS_IP_PROTOCOL_IPV6_E)
   {
     keyType = CPSS_DXCH_TTI_KEY_UDB_IPV6_UDP_E;
   }
   else if(userInfoPtr->ipProt == CPSS_IP_PROTOCOL_IPV4_E)
   {
      keyType = CPSS_DXCH_TTI_KEY_UDB_IPV4_UDP_E;
   }
   else if(userInfoPtr->ipProt == CPSS_IP_PROTOCOL_ALL_E) /* Non valid value to indicate ethernet key type (for UNI) */
   {
      keyType = CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E;
   }
   else
     return GT_BAD_PARAM;

   portNum = userInfoPtr->portNum;
   rc = cpssDxChTtiPortLookupEnableSet(devNum, portNum, keyType, GT_FALSE);
   if(rc != GT_OK)
     return rc;

   /* Disable TTI lookup */
   rc = cpssDxChPclPortIngressPolicyEnable(devNum, portNum, GT_FALSE);
   if(rc != GT_OK)
     return rc;


   if(userInfoPtr->ttiRuleEntryIndex != 0xFFFFFFFF)
   {
     rc = cpssDxChTtiRuleValidStatusSet(devNum, userInfoPtr->ttiRuleEntryIndex, GT_FALSE);
     if(rc != GT_OK)
       return rc;

     rc = csRefInfraIndexDbOp(VPN_EARCH_DB_TYPE_TTI_E, VPN_INDEX_DB_OPERATION_FREE_E, &(userInfoPtr->ttiRuleEntryIndex));
     if(rc != GT_OK)
       return rc;
   }

   if(userInfoPtr->pclRuleEntryIndex != 0xFFFFFFFF)
   {
     rc = cpssDxChPclRuleInvalidate(devNum, 0/*tcamIndex*/, CPSS_PCL_RULE_SIZE_30_BYTES_E, userInfoPtr->pclRuleEntryIndex);
     if(rc != GT_OK)
       return rc;

     rc = csRefInfraIndexDbOp(VPN_EARCH_DB_TYPE_PCL_E, VPN_INDEX_DB_OPERATION_FREE_E, &(userInfoPtr->pclRuleEntryIndex));
     if(rc != GT_OK)
       return rc;
   }

   if(userInfoPtr->tunnelStartEntryIndex != 0xFFFFFFFF)
   {
     osMemSet(&tsConfig, 0, sizeof(tsConfig));
     rc = cpssDxChTunnelStartEntrySet(devNum, userInfoPtr->tunnelStartEntryIndex, CPSS_TUNNEL_GENERIC_IPV6_E, &tsConfig);
     if(rc != GT_OK)
       return rc;

     rc = csRefInfraIndexDbOp(VPN_EARCH_DB_TYPE_TS_E, VPN_INDEX_DB_OPERATION_FREE_E, &(userInfoPtr->tunnelStartEntryIndex));
     if(rc != GT_OK)
       return rc;
   }

   /* Egress processing undo */
   osMemSet(&physicalInfo,0,sizeof(physicalInfo));
   rc = cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(devNum, assignEportNum, &physicalInfo);
   if(rc != GT_OK)
     return rc;

   /* Clean ePort configuration history.  */
   osMemSet(&egressEportInfo, 0, sizeof(egressEportInfo));
   rc = cpssDxChBrgEportToPhysicalPortEgressPortInfoSet(devNum, assignEportNum, &egressEportInfo);
   if(rc != GT_OK)
     return rc;

   /* Enable egress vlan filterring on eport */
   rc = cpssDxChBrgEgrFltVlanEPortFilteringEnableSet(devNum, assignEportNum, GT_TRUE);
   if((rc != GT_OK) && (rc != GT_NOT_APPLICABLE_DEVICE))
     return rc;

   /* Take egress tag state from eVlan */
   rc = cpssDxChBrgVlanEgressPortTagStateModeSet(devNum, assignEportNum, CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_EVLAN_MODE_E);
   if(rc != GT_OK)
     return rc;

   /* Send packet with Tag1 only if received with Tag1. */
   rc = cpssDxChBrgVlanPortRemoveTag1IfRxWithoutTag1EnableSet(devNum, assignEportNum, GT_FALSE);
   if(rc != GT_OK)
     return rc;

   /* Set TPID to profile that recognize 0x8100 only as TAG1 for physical port (no need for assigned eport) */
   rc = cpssDxChBrgVlanPortIngressTpidProfileSet(devNum, portNum, CPSS_VLAN_ETHERTYPE0_E, GT_TRUE, VPN_DEAFULT_TPID_PROFILE_CNS);
   if(rc != GT_OK)
     return rc;
   rc = cpssDxChBrgVlanPortIngressTpidProfileSet(devNum, portNum, CPSS_VLAN_ETHERTYPE1_E, GT_TRUE, VPN_DEAFULT_TPID_PROFILE_CNS);
   if(rc != GT_OK)
     return rc;

   /* Free user info. */
   osFree(userInfoPtr);

   return GT_OK;
}

