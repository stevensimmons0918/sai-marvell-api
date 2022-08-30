// xpTypes.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#ifndef xp80_xpTypes_h  //in SWIG
#define xp80_xpTypes_h

#ifdef __cplusplus
#include <bitset>
#include <cstdlib>
#include <cstring>
#include <vector>
using std::vector;
#endif

#include <stdint.h>

/* Helper macros for printing variables */
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#include <inttypes.h>
#include <sys/time.h>
#include "errno.h"
#include <stdbool.h>
#include "xpEnums.h"
#include "openXpsTypes.h"

//#include "xpCommonDefines.h"

#define DEBUG_KPU 1        ///<  Debug KPU
#define DEBUG_TM 1        ///<  Debug TM

#if DEBUG_TM
#define debugPrint(x) printf x        ///<  Debug Print for TM
#else
#define debugPrint(x)        ///<  Debug Print
#endif

#define XPS_UNUSED(x)  (void(x))

#if DEBUG_KPU
#define printd(x) printf x        ///< Debug Print for KPU
#else
#define printd(x)        ///< Debug Print
#endif //DEBUG

#define XP_MAX_PKT_BFR_SIZE      1024

/* test device Id */
#define  XP_TEST_DEV_NUM    ((xpDevice_t)(0))        ///< Test Device Num

#define XP_ERR_INVALID_PORT_NUM                 -1        ///< Error Invalid Port Num
#define XP_MAX_40G_PORTS                        64        ///< Max 40G Ports
#define XP_MAX_100G_PORTS                       32        ///< Max 100G Ports
#define XP_NUM_SERVICE_PORTS                    8         ///< Number of Service Ports
#define XP_NUM_PHYS_PORTS                       260       ///< Number of Phys Ports
#define XP_SCOPE_DEFAULT                        0         ///< Scope Default
#define XP_MAX_SCOPES                           64        ///< Max Scopes
#define XP80_MAX_DEV_TYPES                      (2)       ///<  Xp80 Max Dev Types
#define XP_MAX_PHY_PORTS                        XP_NUM_PHYS_PORTS        ///< Max Phy Ports
//Field lengths in various input
#define SIZEOF_BYTE                             8        ///<  Sizeof Byte
#define XP_BITS_PER_UINT8                       ((int) (sizeof(uint8_t) * SIZEOF_BYTE))        ///< Bits Per Uint8
#define XP_BITS_PER_INT                         ((int) (sizeof(int) * SIZEOF_BYTE))        ///< Bits Per Int
#define XP_BITS_PER_UINT32                      ((int) (sizeof(uint32_t) * SIZEOF_BYTE))        ///< Bits Per Uint32
#define XP_BITS_TO_INTS(B)                      ((size_t) (((B) + XP_BITS_PER_INT - 1) / XP_BITS_PER_INT))        ///< Bits To Ints
#define XP_BITS_TO_BYTES(B)                     ((size_t) (((B) + SIZEOF_BYTE - 1) / SIZEOF_BYTE))        ///< Bits To Bytes
#define XP_NUM_BITS_IN_KILOBITS                 (1000)
#define XP_NUM_BYTES_IN_KILOBYTES               (1000)
#define XP_NUM_HZ_IN_MHZ                        (1000000)

#define XP_KPU_PKT_PTR_FIELD_LEN                1 //1 Byte        ///< Kpu Pkt Ptr Field Len
#define XP_KPU_DECPOINT_OFFSET_FIELD_LEN        1 //1 Byte        ///< Kpu Decpoint Offset Field Len
#define XP_KPU_NUM_DECPOINT                     3        ///< Kpu Num Decpoint
#define XP_KPU_LAYER_STACK_ENTRY_SIZE           2        ///< Kpu Layer Stack Entry Size
#define XP_LAYER_STACK_NUM_ENTRIES              8        ///< Layer Stack Num Entries

#define XP_MAX_PARSE_ENGINES                    4        ///< Max Parse Engines
#define XP_KPU_MAX_LOOK_AHEADS                  3        ///< Kpu Max Look Aheads
#define XP_SKPU_MAX_LOOK_AHEADS                 12        ///< Skpu Max Look Aheads
#define XP_MAX_KPUS                             5        ///< Max Kpus
#define XP_MAX_LAYERDATASIZE                    100        ///< Max Layerdatasize


#define XP_KPU_TCAM_WIDTH   57        ///< Kpu Tcam Width
#define XP_KPU_SRAM_WIDTH   120        ///< Kpu Sram Width
#define XP_KPU_TCAM_DEPTH   128        ///< Kpu Tcam Depth
#define XP_KPU_SRAM_DEPTH   128        ///< Kpu Sram Depth
#define XP_SKPU_TCAM_WIDTH  105        ///< Skpu Tcam Width
#define XP_SKPU_SRAM_WIDTH  163        ///< Skpu Sram Width
#define XP_SKPU_TCAM_DEPTH  256        ///< Skpu Tcam Depth
#define XP_SKPU_SRAM_DEPTH  256        ///< Skpu Sram Depth
#define XP_TEMPLATEID_TCAM_WIDTH 40        ///< Templateid Tcam Width
#define XP_MAC2ROUTER_TCAM_WIDTH 60        ///< Mac2Router Tcam Width
#define XP_MAC2ROUTER_SRAM_WIDTH 16        ///< Mac2Router Sram Width
#define XP_IKT_DIRECT_ACCESS_WIDTH 100        ///< Ikt Direct Access Width
#define XP_LAYERMAP_CANON_CMD_WIDTH 41        ///< Layermap Canon Cmd Width
#define XP_LAYERMAP_SPLIT_CMD_WIDTH 16        ///< Layermap Split Cmd Width
#define XP_LAYERMAP_TOKEN_CMD_WIDTH 34        ///< Layermap Token Cmd Width
#define XP_LAYERMAP_LAYER_CMD_WIDTH 73        ///< Layermap Layer Cmd Width
#define XP_LAYERMAP_QOS_WIDTH 19        ///< Layermap Qos Width
#define XP_LAYERMAP_NEXTENGINE_WIDTH 66        ///< Layermap Nextengine Width
#define XP_LAYERMAP_ETHERNETCNTRL_WIDTH 24        ///< Layermap Ethernetcntrl Width
#define XP_PARSER_CFG_CNTRL_WIDTH 26        ///< Parser Cfg Cntrl Width


#define XP_IKT_MEM_BITLEN                       125        ///< Ikt Mem Bitlen
#define XP_ITT_MEM_BITLEN                       105        ///< Itt Mem Bitlen
#define XP_KPU_MEM_BITLEN                       121        ///< Kpu Mem Bitlen
#define XP_SKPU_MEM_BITLEN                      165        ///< Skpu Mem Bitlen
#define XP_LAYERCMD_TEMPLATEID_MEM_BITLEN       48        ///< Layercmd Templateid Mem Bitlen
#define XP_CANONCMD_MEM_BITLEN                  42        ///< Canoncmd Mem Bitlen
#define XP_QOS_MEM_BITLEN                       19        ///< Qos Mem Bitlen
#define XP_TOKENCMD_MEM_BITLEN                  512            ///< Tokencmd Mem Bitlen
#define XP_KPU_HDR_DATA_LEN                     (512/SIZEOF_BYTE)        ///< Kpu Hdr Data Len
#define XP_KPU_IKT_LEN                          (1024/SIZEOF_BYTE)        ///< Kpu Ikt Len
#define XP_MAX_TEMPLATE_ID_ENTRIES              256        ///< Max Template Id Entries

#define LAYER_NUM_0                             0        ///<  Layer Num 0
#define LAYER_NUM_1                             1        ///<  Layer Num 1
#define LAYER_NUM_2                             2        ///<  Layer Num 2
#define LAYER_NUM_3                             3        ///<  Layer Num 3
#define LAYER_NUM_4                             4        ///<  Layer Num 4
#define LAYER_NUM_5                             5        ///<  Layer Num 5
#define LAYER_NUM_6                             6        ///<  Layer Num 6
#define LAYER_NUM_7                             7        ///<  Layer Num 7

#define XP_KPU_HDRDATA_BIT_LEN                  512        ///< Kpu Hdrdata Bit Len
#define XP_KPU_LAYER_STACK_BIT_LEN              128        ///< Kpu Layer Stack Bit Len
#define MAX_PORTS_INTO_PARSER_CHANNEL           17      //in SWIG        ///<  Max Ports Into Parser Channel
#define MAX_PHY_PORTS_IN_PARSER_CHANNEL         (MAX_PORTS_INTO_PARSER_CHANNEL - 1)        ///<  Max Phy Ports In Parser Channel
#define XP_LONGEST_KEY_LEN                      390        ///< Longest Key Len

#define XP_MAX_ACM_CLIENTS_PER_SDE    (49)        ///< Max Acm Clients Per Sde

#define MAX_PARSER_CHANNELS     8 ///< Maximum parser channels        ///<  Max Parser Channels
//#define SYSTEM_MAX_PORT         (MAX_PARSER_CHANNELS * MAX_PORTS_INTO_PARSER_CHANNEL)       ///< Maximum Ports        ///<  System Max Port
#define SYSTEM_MAX_PORT         264

#define XP_NXT_LDE_ADDR_TYPE                    0x80    // MSB 0=unicast, 1=multicast        ///< Nxt Lde Addressype
#define XP_NXT_LDE_ADDR_MULTI                   0x0F    // Multicast Group        ///< Nxt Lde Addr Multi
#define XP_NXT_LDE_ADDR_UNI                     0x1F    // Destination LDE Address        ///< Nxt Lde Addr Uni

#define XP_INET_ADDRSTRLEN          46        ///< Max ip addr len
#define XP_MAX_CONFIG_LINE_LEN      1800        ///< Max Config Line Len
#define XP_MAC_DST_ADDR_OFFSET      0        ///< Mac Dst Addr Offset
#define XP_MAC_SRC_ADDR_OFFSET      6        ///< Mac Src Addr Offset
#define XP_ETH_TYPE_OFFSET          12        ///< Eth Type Offset
#define XP_ETH_HDR_LEN              14        ///< Eth Hdr Len
#define XP_ETH_VLAN_TAG_LEN         4        ///< Eth Vlan Tag Len
#define XP_ETH_VLAN_TAG_OFFSET      (XP_ETH_HDR_LEN)        ///< Eth Vlan Tag Offset
#define XP_ETH_TAG_HDR_LEN          (XP_ETH_HDR_LEN + XP_ETH_VLAN_TAG_LEN)        ///< Eth Tag Hdr Len
#define XP_IPV4_ADDR_LEN            4        ///< Ipv4 Addr Len
#define XP_IPV4_HDR_LEN             20        ///< Ipv4 Hdr Len
#define XP_IPV4_SRC_ADDR_OFFSET     12        ///< Ipv4 Src Addr Offset
#define XP_IPV4_DST_ADDR_OFFSET     16        ///< Ipv4 Dst Addr Offset
#define XP_IPV6_ADDR_LEN            16        ///< Ipv6 Addr Len
#define XP_IP_HDR_LEN               (XP_IPV4_HDR_LEN)        ///< Ip Hdr Len
#define XP_TCP_HDR_LEN              20        ///< Tcp Hdr Len
#define XP_UDP_HDR_LEN              8        ///< Udp Hdr Len
#define XP_UDP_SRC_PORT_OFFSET      0        ///< Udp Src Port Offset
#define XP_UDP_DST_PORT_OFFSET      2        ///< Udp Dst Port Offset
#define XP_VLAN_ID_OFFSET           2        ///< Vlan Id Offset
#define XP_PBB_ISID_OFFSET          3        ///< Pbb Isid Offset
#define XP_PBB_ISID_LEN             3        ///< Pbb Isid Len
#define XP_TNL_VNI_OFFSET           4        ///< Tnl Vni Offset
#define XP_TNL_VNI_LEN              3        ///< Tnl Vni Len
#define XP_VXLAN_HDR_LEN            8        ///< Vxlan Hdr Len
#define XP_GRE_HDR_LEN              8        ///< Gre Hdr Len
#define XP_GENEVE_FIXED_HDR_LEN     8        ///< Geneve Fixed Hdr Len
#define XP_IP_HDR_SA_OFFSET         12        ///< Ip Hdr Sa Offset
#define XP_GENEVE_DEST_PORT         6081        ///< Geneve Dest Port

#define XP_MPLS_HEADER_LEN          4
#define XP_MPLS_LABEL_LEN           3
#define XP_MPLS_HEADER_OFFSET       (XP_ETH_HDR_LEN)

// Number of profiles to be stored for LDE or MME
#define XP_ENGINE_URW                           12

// Packet driver defines(needed for descrriptor chaining)
#define XP_TX_DESC_DEPTH            10
#define XP_RX_DESC_DEPTH            10
#define XP_NUM_TX_QUEUE             64
#define XP_NUM_RX_QUEUE             64

#define XP_NUM_PKT_BUFFERS          ((XP_NUM_RX_QUEUE * XP_RX_DESC_DEPTH) + (XP_NUM_TX_QUEUE * XP_TX_DESC_DEPTH) +  1)
#define XP_NUM_DEVICE_SUPPORT    1

/**
 * \brief Calculate the number of elements in the array
 */
#define ARRAY_SIZE(x)  (sizeof(x) / sizeof((x)[0]))

//802.1BR defines
/**
 * Extended ports are allocated from VIF range in groups.
 * Ids of Extended Port VIFs are groupId*MaxPerGroup+numOfPortInGroup
 * Each group reserves MaxPerGroup VIFs starting from groupId*MaxPerGroup VIF
 * groupId is in range from minGroupIdx to maxGroupIdx
 * In case when some VIFs are already allocated group with appropriate index cannot be created.
 */
#define XP_8021BR_MAX_PER_GROUP 1024        ///< 8021Br Max Per Group
#define XP_8021BR_MIN_GROUP_IDX 1        ///< 8021Br Min Group Idx
#define XP_8021BR_MAX_GROUP_IDX 16        ///< 8021Br Max Group Idx

/**
 * Please change "1" to XP_EACL_TOTAL_TYPE in the next chip release
 */
#define XP_ACL_TABLE_NUM   (XP_IACL_TOTAL_TYPE + 1)      /// 3 IACL + 1 EACL table.
#define XP_ACL_VIR_TABLE_NUM   256  ///< supporting 256 for Egress Table.
#define XP_PACL_ID_NUM         128  ///< supportig only 7 bits for ACL ID, MSB 1 bit is reserved for Switch Acl ID.
#define XP_BACL_ID_NUM         256  ///< supporting 256 ACl ID
#define XP_RACL_ID_NUM         256  ///< supporting 256 ACL ID
#define XP_ACL_COUNTER_MODE_C_ENTRY_NUM (4 * 1024)  // Please see: ACM-Design.doc        ///< Acl Counter Mode C Entry Num
#define XP_ACL_MAX_ENTRY_COUNT XP_ACL_COUNTER_MODE_C_ENTRY_NUM                           ///< Acl Max Entry Count
#define XP_ACL_COUNTER_ENTRY_NUM XP_ACL_COUNTER_MODE_C_ENTRY_NUM * XP_ACL_TABLE_NUM      ///< Acl Counter Entry Num
#define XP_ACL_TABLE_ID_SHIFT 24        ///< Acl Table Id Shift

#define XP_MAX_SEGMENTS            3   ///< Maximum number of segments supported for SR.

// XP specific global software types
typedef uint32_t xpLag_t;
typedef uint32_t xpTunnel_t;
typedef uint32_t XP_STATIC_TABLE_ID;

typedef uint32_t xpUnit_t;

typedef uint32_t xpEgressFilter_t;
typedef uint8_t  xpPeg_t;
typedef uint32_t xpAcm_t;

typedef uint8_t
compIpv6Addr_t[6];  ///< User define type for compressed Ipv6 Address
typedef uint32_t xpMcastDomainId_t;
typedef uint32_t xpVifPortBitMapPtrId_t;
typedef uint8_t xpLogModuleId; ///< Log module Id type

#define COPY_ARRAY(a,b,elementtype,numelements) (memcpy((a), (b), sizeof(elementtype) * numelements)) // We will never overrun the destination        ///<  Copy Array
#define COPY_MAC_ADDR_T(a,b) COPY_ARRAY((a), (b), uint8_t, 6)        ///<  Copy Mac Address
#define COPY_IPV4_ADDR_T(a,b) COPY_ARRAY((a), (b), uint8_t, 4)        ///<  Copy Ipv4 Address
#define COPY_IPV6_ADDR_T(a,b) COPY_ARRAY((a), (b), uint8_t, 16)        ///<  Copy Ipv6 Address
#define COPY_IPV6_ADDR_SIZE_T(a,b,n) COPY_ARRAY((a), (b), uint8_t, (n))        ///<  Copy portion of Ipv6 Address

#define COMPARE_MAC_ADDR_T(a,b) (memcmp((a), (b), sizeof(uint8_t)*6))        ///<  Compare Mac Address
#define COMPARE_IPV4_ADDR_T(a,b) (memcmp((a), (b), sizeof(uint8_t)*4))         ///<  Compare Ipv4 Address
#define COMPARE_IPV6_ADDR_T(a,b) (memcmp((a), (b), sizeof(uint8_t)*16))        ///<  Compare Ipv6 Address

typedef enum xpLpmSubTrieMode
{
    PrefixModeSparse=0x01,
    PrefixModeBitmap=0x02,
    PrefixModeLeafPush=0x03,
    PrefixModeInvalid=0x04
} xpLpmSubTrieMode;

#define XP_MAX_DEVICES_WM           1        ///< Max Devices Wm
#define XP_DMA_DESC_SIZE            32        ///< Dma Desc Size
#define XP_DEFAULT_RX_QUEUE_COUNT   64      ///< Rx queue count
#define XP_DEFAULT_TX_QUEUE_COUNT   64      ///< Tx queue count
#define XP_DEFAULT_RX_DESCRIPTOR_DEPTH  10      ///< Rx descriptor depth
#define XP_DEFAULT_TX_DESCRIPTOR_DEPTH  10      ///< Tx descriptor depth
#define XP_DEFAULT_PACKET_BUFFER_SIZE   1024        ///< Packet buffer size
#define XP_DEFAULT_DMA_COMPLETION_BOUNDARY  0       ///< Completion boundary
#define XP_DEFAULT_DMA_DESCRIPTOR_FETCH_RETRY   0   ///< Desc Fetch max Retries
#define XP_DEFAULT_DMA_PREFETCH_MODE            0   ///< Prefetch or Non-Prefetch mode 
#define XP_DEFAULT_DMA_DESCRIPTOR_RING          0   ///< DMA Descriptor Ring mode or Chain
#define MAX_TX_QUEUES                   64      ///<  Max Tx Queues
#define MAX_RX_QUEUES                   64      ///<  Max Rx Queues
#define MAX_PREFETCH_RX_QUEUES          16      ///< Max Rx Queues in Prefetch Mode
#define XP_MIN_PACKET_LENGTH            60      ///< Min Ethernet packet length excluding CRC

#define XP_SAI_CONFIG_PATH   "XP_SAI_CONFIG_PATH"   ///< macro for xdk root path to be set through SAI

/**
 * \struct _xphTxHdrMetaData__
 * \brief Fields of Tx metadata in Tx header. Max size is 56 bits
 */
typedef struct __attribute__((__packed__)) _xphTxHdrMetaData__
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    uint8_t reserved1: 8;                    // reserved
    uint8_t bridgeDomain[2];                /*  bridgeDomain[0]      bridgeDomain[1]
                                                    MSB                    LSB             */
    uint32_t privateWireEn          :1;
    uint32_t addPortBasedTag        :1;
    uint32_t reserved               :6;

    uint32_t portAclId              :8;

    uint32_t portState              :2;
    uint32_t setEgressPortFilter    :1;
    uint32_t macSAmissCmd           :2;
    uint32_t bypassBridgeRouter     :1;
    uint32_t portACLEn              :1;
    uint32_t portDebugEn            :1;

    uint32_t setIngressVif          :1;
    uint32_t setBridgeDomain        :1;
    uint32_t acceptedFrameType      :2;
    uint32_t bypassTunnelVif        :1;
    uint32_t bypassACLsPBR          :1;
    uint32_t samplerEn              :1;
    uint32_t policerEn              :1;
#else
    uint8_t reserved1:8;                    // reserved
    uint8_t bridgeDomain[2];                /*  bridgeDomain[0]      bridgeDomain[1]
                                                    MSB                    LSB            */
    uint32_t reserved               :6;
    uint32_t addPortBasedTag        :1;
    uint32_t privateWireEn          :1;

    uint32_t portAclId              :8;

    uint32_t portDebugEn            :1;
    uint32_t portACLEn              :1;
    uint32_t bypassBridgeRouter     :1;
    uint32_t macSAmissCmd           :2;
    uint32_t setEgressPortFilter    :1;
    uint32_t portState              :2;

    uint32_t policerEn              :1;
    uint32_t samplerEn              :1;
    uint32_t bypassACLsPBR          :1;
    uint32_t bypassTunnelVif        :1;
    uint32_t acceptedFrameType      :2;
    uint32_t setBridgeDomain        :1;
    uint32_t setIngressVif          :1;
#endif
} xphTxHdrMetaData;

/**
 * \struct _xphRxHdrMetaData__
 * \brief Fields of Rx metadata in Rx header. Max size is 64 bits
 */
typedef struct __attribute__((__packed__)) _xphRxHdrMetaData__
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    uint8_t rsvd0[2];
    uint8_t bdId[2];                   /*  bdId[0]      bdId[1]
                                            MSB           LSB      */
    uint8_t rsvd1                   :5;
    uint8_t tunnelTerminate         :1;
    uint8_t rsvd2                   :2;
    uint8_t rsvd3[2];
    uint8_t rsvd4                   :6;
    uint8_t fdbNewAddr              :1;
    uint8_t rsvd5                   :1;

#else

    uint8_t rsvd0[2];
    uint8_t bdId[2];                   /*  bdId[0]      bdId[1]
                                            MSB           LSB      */
    uint8_t rsvd2                   :2;
    uint8_t tunnelTerminate         :1;
    uint8_t rsvd1                   :5;
    uint8_t rsvd3[2];
    uint8_t rsvd5                   :1;
    uint8_t fdbNewAddr              :1;
    uint8_t rsvd4                   :6;

#endif

} xphRxHdrMetaData;

/**
 * \struct txHeader
 * \brief Fields of Tx Header, to be prefixed while Packet Tx when encap is enabled
 */
typedef struct  __attribute__((__packed__)) _xphTxHdr__
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

    uint8_t pktRecvTimeStamp[8];          /*  pktRecvTimeStamp[0]      .......... pktRecvTimeStamp[7]
                                                    MSB                                LSB          */
    uint64_t headerSize               :4;
    uint64_t headerType               :4;

    uint64_t DP         :2;
    uint64_t TC         :4;
    uint64_t useXPHTimeStamp        :1;
    uint64_t r0         :1;

    uint64_t r1         :5;
    uint64_t txSampleID     :2;
    uint64_t txSample           :1;

    uint64_t ingressVifLsbByte2     :8;           // MSB of ingressVif
    uint64_t ingressVifLsbByte1     :8;           // Intermidiate byte of ingressVif

    uint64_t egressVifLsbByte2      :4;           // MSN (most significant nibble) of egressVif
    uint64_t ingressVifLsbByte0     :4;           // LSN (least significant nibble) of ingressVif

    uint64_t egressVifLsbByte1      :8;           // Intermidiate byte of egressVif
    uint64_t egressVifLsbByte0      :8;           // LSB of egressVif

    uint64_t nextEngine     :8;
    xphTxHdrMetaData  metadata     ;
#else
    uint8_t pktRecvTimeStamp[8];       /*  pktRecvTimeStamp[0]      .......... pktRecvTimeStamp[7]
                                                    MSB                                LSB          */
    uint64_t headerType               :4;
    uint64_t headerSize               :4;

    uint64_t r0         :1;
    uint64_t useXPHTimeStamp        :1;
    uint64_t TC         :4;
    uint64_t DP         :2;

    uint64_t txSample           :1;
    uint64_t txSampleID     :2;
    uint64_t r1         :5;

    uint64_t ingressVifLsbByte2     :8;           // MSB of ingressVif
    uint64_t ingressVifLsbByte1     :8;           // Intermidiate byte of ingressVif

    uint64_t ingressVifLsbByte0     :4;           // LSN (least significant nibble) of ingressVif
    uint64_t egressVifLsbByte2      :4;           // MSN (most significant nibble) of egressVif

    uint64_t egressVifLsbByte1      :8;           // Intermidiate byte of egressVif
    uint64_t egressVifLsbByte0      :8;           // LSB of egressVif

    uint64_t nextEngine     :8;
    xphTxHdrMetaData  metadata     ;
#endif
} xphTxHdr;

/**
 * \struct rxHeader
 * \brief Fields of Rx Header, to be parsed while Packet Rx
 */
typedef struct  __attribute__((__packed__)) _xphRxHdr__
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    uint8_t pktRecvTimeStamp[8];          /*  pktRecvTimeStamp[0]      .......... pktRecvTimeStamp[7]
                                                    MSB                                LSB          */
    uint64_t headerSize     :4;
    uint64_t headerType     :4;

    uint64_t ingressPortNum     :8;
    uint64_t ingressVifLsbByte2     :8;           // MSB of ingressVif
    uint64_t ingressVifLsbByte1     :8;           // Intermidiate byte of ingressVif

    uint64_t r0         :4;
    uint64_t ingressVifLsbByte0     :4;           // LSN (least significant nibble) of ingressVif

    uint64_t reasonCodeMSB     :8;            // MSB of reasonCode
    uint64_t r2         :3;
    uint64_t truncated      :1;
    uint64_t r1         :2;
    uint64_t reasonCodeLsbs    :2;            // Lsbs of reasonCode

    uint64_t r3         :8;

    xphRxHdrMetaData metadata          ;
#else
    uint8_t pktRecvTimeStamp[8];          /*  pktRecvTimeStamp[0]      .......... pktRecvTimeStamp[7]
                                                    MSB                                LSB          */
    uint64_t headerType     :4;
    uint64_t headerSize     :4;

    uint64_t ingressPortNum     :8;
    uint64_t ingressVifLsbByte2     :8;           // MSB of ingressVif
    uint64_t ingressVifLsbByte1     :8;           // Intermidiate byte of ingressVif

    uint64_t ingressVifLsbByte0     :4;           // LSN (least significant nibble) of ingressVif
    uint64_t r0         :4;

    uint64_t reasonCodeMSB     :8;            // MSB of reasonCode
    uint64_t reasonCodeLsbs    :2;            // Lsbs of reasonCode
    uint64_t r1         :2;
    uint64_t truncated      :1;
    uint64_t r2         :3;

    uint64_t r3         :8;

    xphRxHdrMetaData metadata          ;
#endif
} xphRxHdr;

/**
 * \struct lbRxHeader
 * \brief Fields of loopback Rx header, to be parsed while Packet Rx
 */
typedef struct  __attribute__((__packed__)) _xphLBRxHdr__
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    uint8_t pktRecvTimeStamp[8];          /*  pktRecvTimeStamp[0]      .......... pktRecvTimeStamp[7]
                                              MSB                                LSB          */
    uint64_t headerSize     :4;
    uint64_t headerType     :4;

    uint64_t DP         :2;
    uint64_t TC         :4;
    uint64_t useXPHTimeStamp        :1;
    uint64_t r0         :1;

    uint64_t r1:8;
    uint64_t ingressVifLsbByte2     :8;
    uint64_t ingressVifLsbByte1     :8;

    uint64_t egressEVifLsbByte2     :4;
    uint64_t ingressVifLsbByte0     :4;

    uint64_t egressEVifLsbByte1     :8;
    uint64_t egressEVifLsbByte0     :8;

    uint64_t  metadata     ;

#else
    uint8_t pktRecvTimeStamp[8];          /*  pktRecvTimeStamp[0]      .......... pktRecvTimeStamp[7]
                                                MSB                                LSB          */
    uint64_t headerType     :4;
    uint64_t headerSize     :4;

    uint64_t r0         :1;
    uint64_t useXPHTimeStamp:1;
    uint64_t TC:4;
    uint64_t DP:2;

    uint64_t r1:8;

    uint64_t ingressVifLsbByte2     :8;
    uint64_t ingressVifLsbByte1     :8;

    uint64_t ingressVifLsbByte0     :4;
    uint64_t egressEVifLsbByte2     :4;

    uint64_t egressEVifLsbByte1     :8;
    uint64_t egressEVifLsbByte0     :8;

    uint64_t metadata     ;
#endif
} xphLBRxHdr;

typedef struct  __attribute__((__packed__)) _xpEtagsStruct
{

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    uint8_t ingressECidBaseMSB      :4;
    uint8_t uDei        :1;
    uint8_t ePcp        :3;

    uint8_t ingressECidBaseLSB;

    uint8_t egressECidBaseMSB       :4;
    uint8_t grpBit  :2;
    uint8_t reservedBit     :2;

    uint8_t egressECidBaseLSB;

    uint8_t ingressECidExt;

    uint8_t egressECidExt;

#else

    uint8_t ePcp        :3;
    uint8_t uDei        :1;
    uint8_t ingressECidBaseMSB      :4;

    uint8_t ingressECidBaseLSB;

    uint8_t reservedBit     :2;
    uint8_t grpBit  :2;
    uint8_t egressECidBaseMSB       :4;

    uint8_t egressECidBaseLSB;

    uint8_t ingressECidExt;

    uint8_t egressECidExt;

#endif
} xpEtagsStruct;

#define TO_CPU_XPH_TYPE 0
#define FROM_CPU_XPH_TYPE 1
#define LOOPBACK_XPH_TYPE 2

#define FROM_CPU_XPH_SIZE 3 //From CPU_TO_XP header size = 24/8, where 24 is total bytes in Tx XPH
#define CPU_XPH_SIZE_BYTES 24 //From CPU_TO_XP header size = 24 Bytes

#define XP_ETAG_HEADER_LEN 8



/**
 * \struct xpQueueInfo
 * \brief Tx/Rx Queue specific information, provided by Customer OS when
 *    initializing Queues
 *
 */
typedef struct xpQueueInfo
{
    uint8_t queueNum;    /**< Queue number */
    uint16_t descDepth;   /**< Required depth of descriptors for the queue */
    bool isTxQueue;      /**< Type of queue, Tx/Rx */
} xpQueueInfo;

/**
 * \struct xpDmaConfigInfo
 * \brief DMA specific configuration information, provided by Customer OS when
 *    initializing DMA
 *
 */
typedef struct xpDmaConfigInfo
{
    uint16_t pktBufSize;                /**< Packet buffer size */
    uint16_t txDescDepth;               /**< Tx descriptor depth */
    uint16_t rxDescDepth;               /**< Rx descriptor depth */
    uint8_t numOfTxQueue;               /**<  Number of Tx queue */
    uint8_t numOfRxQueue;               /**< Number of Rx queue */
    uint32_t fetchRetryCount;           /**< Desc Fetch max Retries */
    bool enablePrefetch;                /**< Prefetch or Non-Prefetch mode */
    bool enableDescRing;                /**< DMA Descriptot Ring mode or Chain */
    uint16_t completionBoundary;        /**< DMA completion boundary (Zero means - to use the default one) */
    uint32_t numOffload;                /**< number of packets to process by offloading in different thread */
} xpDmaConfigInfo;

/**
 * \struct xpDmaDescDebugEntities
 * \brief DMA descriptor entities common for both Tx and Rx operations
 *
 */
typedef struct xpDmaDescDebugEntities
{
    uint8_t errorIndicationBit;     /**< Error indication bit */
    uint8_t interruptEnBit;         /**< Interrupt enable bit */
    uint8_t lastBufferBit;          /**< Last buffer identifier bit */
    uint8_t firstBufferBit;         /**< First buffer identifier bit */
    uint8_t ownershipBit;           /**< Ownership bit*/
    uint16_t bufferSize;            /**< Buffer size */
    uint64_t nextDescPtr;            /**< Next descriptor pointer */
} xpDmaDescDebugEntities;

/**
 * \struct rxQueueWeightInfo
 * \brief Provided by customer OS for sending the weight info
 *    for the queues when using weighted round robin scheduling policy
 *
 */
typedef struct rxQueueWeightInfo
{
    uint8_t queueNum;    /**< Rx queue number */
    uint32_t weight;     /**< Weight for the queue */
} rxQueueWeightInfo;


typedef uint32_t xpPortArr_t[SYSTEM_MAX_PORT];
typedef xpPortArr_t xpsPortArr_t;

/* Max Port Number */
#define MAX_PORTNUM                     XP_NUM_PHYS_PORTS

typedef struct
{
    uint32_t size;
    xpPortArr_t portList;
} xpPortsList_t;

typedef xpPortsList_t xpsPortList_t;


/**
 * \struct xpVlanData_t
 * \public
 * \brief This structure define vlan  encapsulation information.
 */
typedef struct xpVlanData_t
{
    xpVlan_t vlanId;
} xpVlanData_t;

/**
 * \struct xpVxlanData_t
 * \public
 * \brief This structure define Vxlan tunnel encapsulation information.
 */
typedef struct xpVxlanData_t
{
    uint32_t vni;
} xpVxlanData_t;

/**
 * \struct xpNvgreData_t
 * \public
 * \brief This structure define Nvgre tunnel encapsulation information.
 */
typedef struct xpNvgreData_t
{
    uint32_t tni;
} xpNvgreData_t;

/**
 * \struct xpGeneveData_t
 * \public
 * \brief This structure define Geneve tunnel encapsulation information.
 */
typedef struct xpGeneveData_t
{
    uint32_t vni;
} xpGeneveData_t;

/**
 * \struct xpPbbData_t
 * \public
 * \brief This structure define PBB tunnel encapsulation information.
 */
typedef struct xpPbbData_t
{
    uint32_t isid;
} xpPbbData_t;

/**
 * \struct xpL2Encapdata_t
 * \public
 * \brief This union speicifies encap information for various encapsulations.
 */
typedef union xpL2Encapdata_t
{
    xpVlanData_t vlanData;
    xpVxlanData_t vxlanData;
    xpNvgreData_t nvgreData;
    xpGeneveData_t geneveData;
    xpPbbData_t pbbData;
} xpL2Encapdata_t;



typedef enum xpL2DomainType_t
{
    XP_L2_DOMAIN_TYPE_VLAN,   // Tradional VLAN Domain
    XP_L2_DOMAIN_TYPE_VPLS,   // Tradional VPLS Domain
    XP_L2_DOMAIN_TYPE_ROUTE_ONLY, // Routing Only Type L2 Domain // Plain L3 Interface (Tunnel Interfaces) // Port Base Routing Ethernet Interfaces
    XP_L2_DOMAIN_TYPE_VSI,    // Virtual service instance
    XP_L2_DOMAIN_TYPE_BD,    // Bridge Domain
    XP_L2_DOMIAN_TYPE_INVALID
} xpL2DomainType_t;

/**
 * \typedef xpL2EncapNode_t
 * \private
 * \brief This structure defines per-vlan per-encapsulation references
 *        of ID(s) and INDEX(es) allocated.
 * \ l2EncapData - Overloaded field for VLAN ID for egress encap, this can support normal VLAN flooding
 *        and also support VPLS instance which requires a different VLAN on the egress
 */
typedef struct xpL2EncapNode
{
    xpL2EncapType_e encapType;
    xpL2Encapdata_t l2EncapData;
    xpVif_t mVif;
    uint32_t mdtNodeIdx;
    uint32_t nextMdt;
} xpL2EncapNode_t;

typedef struct xpL2EncapEntry
{
    xpL2EncapNode_t encapNode;
    struct xpL2EncapEntry *next;
    struct xpL2EncapEntry *prev;
} xpL2EncapEntry_t;

typedef struct xpL2EncapHead
{
    xpL2EncapEntry_t *first;
    xpL2EncapEntry_t *last;
} xpL2EncapHead_t;

/**
 * \typedef xpL2DomainCtx_t
 * \public
 * \brief This structure defines a global (device independent) vlan
 *        reference by storing ID(s) and INDEX(es) allocated by various
 *        primitive entities, within their tables.
 *
 *    encapHead: Stores first and last pointers to linked list of encapNodes.
 *
 *    The linked list stores the floodVif and MDT nodes.
 *    first entry of list - stores a valid floodVIf as "mVif"
 *    and 0xFFFF as mdtNodeIdx, since there is no MDT associated
 *    rest all entries will have a valid mVif and valid mdtNode,
 */
typedef struct xpL2DomainCtx_t
{

    xpL2DomainType_t type;
    uint32_t bdId;
    xpL2EncapHead_t encapHead;
} xpL2DomainCtx_t;

typedef enum VLAN_TYPE_E
{
    XP_VLAN_TYPE_DOT1Q,
    XP_VLAN_TYPE_QINQ,
    XP_VLAN_TYPE_MAX
} XP_VLAN_TYPE;

typedef enum MULTICAST_TYPE_E
{
    XP_MULTICAST_TYPE_IPV4_BRIDGE,
    XP_MULTICAST_TYPE_IPV6_BRIDGE,
    XP_MULTICAST_TYPE_IPV4_ROUTE,
    XP_MULTICAST_TYPE_IPV6_ROUTE,
    XP_MULTICAST_TYPE_FDB,
    XP_MULTICAST_TYPE_VLAN,
    XP_MULTICAST_TYPE_MAX
} XP_MULTICAST_TYPE;

typedef enum xpTunnelInterfaceType
{
    XP_NO_TUNNEL_INTERFACE,
    XP_IPV4_TUNNEL_INTERFACE,
    XP_MPLS_TUNNEL_INTERFACE,
    XP_SRV6_TUNNEL_INTERFACE,
    XP_INVALID_TUNNEL_INTERFACE
} xpTunnelInterfaceType;


typedef struct xpEgressCosMapData_t
{
    uint32_t l2QosEn;
    uint32_t l3QosEn;
    uint32_t pcp;
    uint32_t dei;
    uint32_t dscp;
} xpEgressCosMapData_t;

/**
 * \struct xpsIpGreTunnelConfig_t
 *
 * This structure carries the configuration to be applied on a IpGre tunnel.
 */
typedef struct xpsIpGreTunnelConfig_t
{
    uint32_t  protocol;
    uint8_t   ttlHopLimit;
    macAddr_t dstMacAddr;
    uint16_t  vlanId;
    uint8_t   vlanPri;
    uint8_t   dscp;
} xpsIpGreTunnelConfig_t;

/**
 * \struct xpsIpInIpTunnelConfig_t
 *
 * This structure carries the configuration to be applied on a Ip in Ip tunnel.
 */
typedef struct xpsIpinIpTunnelConfig_t
{
    uint8_t baclEn;
    uint32_t baclId;
    uint8_t raclEn;
    uint32_t raclId;
} xpsIpinIpTunnelConfig_t;

/**
 *\type xpOpenFlowType_e
 *\public
 *\brief This type (enum) defines type of OpenFlow Table.
 *
 **/
typedef enum
{
    XP_OPENFLOW_SRAM0,
    XP_OPENFLOW_SRAM1,
    XP_OPENFLOW_SRAM2,
    XP_OPENFLOW_SRAM3,
    XP_OPENFLOW_SRAM4,
    XP_OPENFLOW_SRAM5,
    XP_OPENFLOW_SRAM6,
    XP_OPENFLOW_SRAM7,
    XP_OPENFLOW_SRAM8,
    XP_OPENFLOW_SRAM9,
    XP_OPENFLOW_SRAM10,
    XP_OPENFLOW_SRAM11,
    XP_OPENFLOW_MPLS_SRAM0,
    XP_OPENFLOW_MPLS_SRAM1,
    XP_OPENFLOW_MPLS_SRAM2,
    XP_OPENFLOW_MPLS_SRAM3,
    XP_OPENFLOW_MPLS_SRAM4,
    XP_OPENFLOW_MPLS_SRAM5,
    XP_OPENFLOW_MPLS_SRAM6,
    XP_OPENFLOW_MPLS_SRAM7,
    XP_OPENFLOW_MPLS_SRAM8,
    XP_OPENFLOW_MPLS_SRAM9,
    XP_OPENFLOW_MPLS_SRAM10,
    XP_OPENFLOW_MPLS_SRAM11,
    XP_OPENFLOW_TCAM0,
    XP_OPENFLOW_TCAM1,
    XP_OPENFLOW_TCAM2,
    XP_OPENFLOW_TCAM3,
    XP_OPENFLOW_TCAM4,
    XP_OPENFLOW_TCAM5,
    XP_OPENFLOW_TCAM6,
    XP_OPENFLOW_TCAM7,
    XP_OPENFLOW_TCAM8,
    XP_OPENFLOW_TCAM9,
    XP_OPENFLOW_TCAM10,
    XP_OPENFLOW_TCAM11,
    XP_OPENFLOW_MPLS_TCAM0,
    XP_OPENFLOW_MPLS_TCAM1,
    XP_OPENFLOW_MPLS_TCAM2,
    XP_OPENFLOW_MPLS_TCAM3,
    XP_OPENFLOW_MPLS_TCAM4,
    XP_OPENFLOW_MPLS_TCAM5,
    XP_OPENFLOW_MPLS_TCAM6,
    XP_OPENFLOW_MPLS_TCAM7,
    XP_OPENFLOW_MPLS_TCAM8,
    XP_OPENFLOW_MPLS_TCAM9,
    XP_OPENFLOW_MPLS_TCAM10,
    XP_OPENFLOW_MPLS_TCAM11,

    XP_OPENFLOW_TOTAL_TYPE
} xpOpenFlowType_e;

/*
 * \enum xpOfActionBucketNodeFormat_e
 * \brief Openflow Group Action Bucket Format Mapping t the Hardware
 *
 *
 */

typedef enum xpOfActionBucketNodeFormat_e
{
    XP_OF_ACTION_BKT_BRIDGE_NODE = 0, //Openflow Bridge MDT Node Type
    XP_OF_ACTION_BKT_ROUTE_NODE,      // Openflow Route MDT Node Type
    XP_OF_ACTION_BKT__MAX_NODE,
} xpOfActionBucketNodeFormat_e;

/*
 * \enum xpOfActionSetIpFormat_e
 * \brief Openflow Set IP Action Enum
 *
 *
 */

typedef enum xpOfActionSetIpFormat_e
{
    XP_OF_ACTION_SET_SOURCE_IP  = 0x01,     // Set Source Ip
    XP_OF_ACTION_SET_DEST_IP    = 0x02,     // Set Dest Ip
    XP_OF_ACTION_SET_MPLS       = 0x03,     // Set MPLS
} xpOfActionSetIpFormat_e;


/*
 * \enum xpOfActionSetPortFormat_e
 * \brief Openflow Set Port Action Enum
 *
 *
 */

typedef enum xpOfActionSetPortFormat_e
{
    XP_OF_ACTION_SET_SOURCE_PORT    = 0x01,     // Set Source Port
    XP_OF_ACTION_SET_DEST_PORT      = 0x02,     // Set Dest Port
} xpOfActionSetPortFormat_e;


/*
 * \struct xpOfActionBucketBridgeNode_t
 * \brief  Openflow Action Bucket Node which contains Bridge Actions
 *
 */

typedef struct _xpOfActionBridgeNode_
{

    uint32_t    isSetVlanPcp;  // Flag to check if we need to set the Vlan PCP
    uint32_t    isSetVlanId;   // Flag to check if we need to set the vlan Id
    uint32_t    isPopVlanId;   // Flag to check if we need to Pop the Vlan Id
    uint32_t
    isSetDestMac;  // Flag to check if we need to modify the Destination Mac

    uint32_t    vlanId;        // Field contains the Vlan Id to set
    uint32_t    pcpDei;        // Field Contains the PCP Dei value to set.
    macAddr_t   mac;           // Field contains the Mac that needs to be set.


} xpOfActionBucketBridgeNode_t;


/*
 * \struct xpOfActionBucketBridgeNode_t
 * \brief  Openflow Action Bucket Node which contains Route Actions
 *
 */

typedef struct _xpOfActionBucketRouteNode_
{
    uint32_t
    isSetIp;      // Flag to check if we need to set the the Source Ip or the Destination Ip
    uint32_t
    isSetPort;    // Flag to check if we need to set the Source Port of the Destination Port
    uint32_t    decrTtl;      // Flag to check if we need to decrement the ttl
    uint32_t    isSetTtl;     // Flag to check if we need to Ttl value.

    uint32_t
    portVal;      // Field holds the value to set the sourc port of the dest port
    uint32_t    ttlVal;       // Field holds the value to set the ttl value
    union
    {
        ipv4Addr_t  ip;       // Field holds the Source of the Destination Ip to be set.
        uint32_t    mpls;
    };


} xpOfActionBucketRouteNode_t;


struct xpOfGroupCtx;

/*
 * \struct xpOfActionBucketNode_t
 * \brief  Openflow Action Bucket Node which abstracts the Action Node type
 *
 */
typedef struct _xpOfActionBucketNode_
{
    xpOfActionBucketNodeFormat_e ofActionBucketType;
    uint32_t mdtIndex;          ///< This would be the Mdt Index
    xpVif_t mVif;               ///< Physical port(ports) VIF
    xpVif_t serviceVif;         ///< OF service port VIF (one of OFPP_ALL/OFPP_FLOOD/OFPP_CONTROLLER)
    uint8_t isServicePort;      ///< Use physical port either service port
    uint32_t pktCmd;
    struct xpOfGroupCtx *groupCtx;     ///< groupCtx for XP_OFPAT_GROUP in bucket
    union
    {
        xpOfActionBucketBridgeNode_t bridgeNode;
        xpOfActionBucketRouteNode_t  routeNode;
    } node;
    uint32_t nextMdt;

} xpOfActionBucketNode_t;


/*
 * \struct xpOfActionBucketEntry_t
 * \brief  Openflow Action Bucket Entry is a DLL Node which is used to maintian a Bucket List
 *
 */

typedef struct _xpOfActionBucketEntry_
{
    xpOfActionBucketNode_t ofActionBucketNode;
    struct _xpOfActionBucketEntry_ *next;
    struct _xpOfActionBucketEntry_ *prev;
} xpOfActionBucketEntry_t;

/*
 * \struct xpOfActionBucketEntry_t
 * \brief  Openflow Action Bucket Head whcih holds the first and the last entry
 *
 */


typedef struct _xpOfActionBucketHead_
{
    xpOfActionBucketEntry_t *first;
    xpOfActionBucketEntry_t *last;
} xpOfActionBucketHead_t;

/*
 * \struct xpOfGroupVifEntry_t
 * \brief  Group Vif Entry
 *
 */

typedef struct xpOfGroupVifEntry
{
    uint32_t mvifId;
    uint32_t mdtRootIdx;
} xpOfGroupVifEntry_t;

/*
 * \struct xpOfGroupCtx_t
 * \brief  Group Context which maps the Openflow Group Table on to the Hardware Tables.
 *
 */
typedef struct xpOfGroupCtx
{
    xpOfGroupVifEntry_t mVif;           ///< Group HW ID
    uint32_t bucketsNum;                ///< Number of buckets in this group
    xpOfActionBucketHead_t bucketHead;  ///< Group's buckets
    uint32_t validGroup;                ///< Group has been created in HW
} xpOfGroupCtx_t;

/**
 * \def XP_INVALIDATE_OF_GROUP_CTX
 * \brief Macro that will set the xpOfGroupCtx_t data to invalid values
 *
 * This Macro will be used to invalidate an xpOfGroupCtx_t structure. It
 * expects a reference to an xpOfGroupCtx_t
 *
 * \param [in] ofGroupCtx
 */
#define XP_INVALIDATE_OF_GROUP_CTX(ofGroupCtx) \
{                                         \
    ofGroupCtx.bucketHead.first = NULL; \
    ofGroupCtx.bucketHead.last = NULL; \
}

/* OpenFlow MPLS table number */
#define XP_OF_MPLS_TABLE_ID                    2

typedef struct xpPolicerResult_t
{

    uint8_t     dp;
    uint8_t     tc;
    uint8_t     pcp;
    uint8_t     dei;
    uint8_t     dscp;
    uint8_t     exp;
} xpPolicerResult_t;

typedef enum xpPolicingResultColor
{
    XP_POL_COLOR_RED,
    XP_POL_COLOR_YELLOW,
    XP_POL_COLOR_GREEN

} xpPolicingResultColor;

// Enum : operation type
typedef enum
{
    XP_READ_OP = 0,
    XP_WRITE_OP,
    XP_MAX_OP
} xpOperation_t;

typedef enum
{
    XP_BYTE_XFER_MODE,
    XP_BLOCK_XFER_MODE
} xpXferMode_t;

#ifdef BOOT_TIME_STAT
#define GET_TIME_STAMP(retVal) \
{                                \
    struct timeval currTime; \
    memset(&currTime, 0, sizeof(currTime)); \
    gettimeofday(&currTime, NULL); \
    retVal = currTime.tv_sec*(uint64_t)1000000+currTime.tv_usec; \
}
#else
#define GET_TIME_STAMP(retVal) \
{                                \
        retVal = 0;\
}
#endif

/** Structure to store bootime statistics*/
typedef struct xpTimeStamp
{
    unsigned long long startTime; //Start time
    unsigned long long endTime;   //End time
    unsigned long long executionTime; //Execution time
} xpTimeStamp_t;

/** Funtion names to get boot-time statistics*/
typedef enum
{
    XP_BOOTUP_ALL = 0,
    XP_APP_NEW_DEVICE_INIT,
    XP_INIT_ADD_DEVICE,
    XP_VLAN_ADD_DEVICE,
    XP_TUNNEL_ADD_DEVICE,
    XP_LINK_ADD_DEVICE,
    XP_DEVICE_INIT,
    XP_TXQMGR_INIT,
    XP_PORT_INIT,
    XP_SINGLE_PORT_INIT,
    XP_MAC_INIT,
    XP_SERDES_ILB,
    XP_SERDES_SETUP,
    XP_SERDES_FW_UPLOAD,
    XP_MAX_FUNCTION

} xpTimeStampFun_t;

typedef struct xpTxqQueuePathToPort
{
    uint32_t queueNum;
    uint32_t h1Num;
    uint32_t h2Num;
    uint32_t portNum;
} xpTxqQueuePathToPort_t;

/**
 * \typedef xpSeInsPtrs_t
 * \public
 * \brief This structure defines a insertion pointers
 *        for various insertion type
 */
typedef struct xpSeInsPtrs_t
{
    uint32_t insPtr0;
    uint32_t insPtr1;
    uint32_t insPtr2;
    uint32_t insPtr3;
} xpSeInsPtrs_t;

/// Global Egress/Ingress Enum
typedef enum XP_DIR_E
{
    XP_EGRESS = 0,
    XP_INGRESS,
    XP_DIR_TOTAL,
} XP_DIR_E;

///Mask Enum
typedef enum XP_MASK_E
{
    XP_UNMASK_BIT = 0,
    XP_MASK_BIT = 1,
} XP_MASK;


typedef enum XP_SPEED_E
{
    XP_PG_SPEED_1G,
    XP_PG_SPEED_10G,
    XP_PG_SPEED_25G,
    XP_PG_SPEED_40G,
    XP_PG_SPEED_50G,
    XP_PG_SPEED_100G,
    XP_PG_SPEED_200G,
    XP_PG_SPEED_MISC,
} XP_SPEED;

typedef enum XP_HDBF_CHANNEL_E
{
    XP_HDBF_CH0,
    XP_HDBF_CH1,
    XP_HDBF_CH2,
    XP_HDBF_CH3,
    XP_HDBF_MAX
} XP_HDBF_CHANNEL;

typedef enum XP_DATAPATH_ECN_ETHERTYPE_E
{
    XP_DP_ECN_ETHERTYPE_IPv4_0,
    XP_DP_ECN_ETHERTYPE_IPv4_1,
    XP_DP_ECN_ETHERTYPE_IPv6_0,
    XP_DP_ECN_ETHERTYPE_IPv6_1,
    XP_DP_ECN_ETHERTYPE_TAG_0,
    XP_DP_ECN_ETHERTYPE_TAG_1,
    XP_DP_ECN_ETHERTYPE_TAG_2,
    XP_DP_ECN_ETHERTYPE_TAG_3,
} XP_DATAPATH_ECN_ETHERTYPE;


typedef enum XP_PVLAN_TYPE_E
{
    VLANTYPE_PRIMARY = 0,
    VLANTYPE_COMMUNITY,
    VLANTYPE_ISOLATED,
    VLANTEYP_NONE
} XP_PVLAN_TYPE;

//Age Table Ids for SE Table That Needs Aging. Multiple
//Copies share same Age Table Id

#define XP_FDB_AGE_TABLE_ID                    0
#define XP_IPV4_HOST_AGE_TABLE_ID              1
#define XP_IPV6_HOST_AGE_TABLE_ID              2
#define XP_IPV4_BRIDGE_MC_AGE_TABLE_ID         3
#define XP_IPV6_BRIDGE_MC_AGE_TABLE_ID         4
#define XP_IPV4_ROUTE_MC_AGE_TABLE_ID          5
#define XP_IPV6_ROUTE_MC_AGE_TABLE_ID          6
#define XP_IP_ROUTE_NH_AGE_TABLE_ID           7
#define XP_FDB_DA_AGE_TABLE_ID                 8

// Table Ids for each Primitive table
//Table IDs for HASH tables

#define XP_PORTVLAN_TABLE_ID1                64
#define XP_TNL_IVIF_TABLE_ID1                81
#define XP_MY_SID_TABLE_ID1                  68
#define XP_MY_SID_TABLE_ID2                  79

#define XP_PORTVLAN_TABLE_ID2                70
#define XP_PORT_AC_TABLE_ID2                 89
#define XP_TNL_IVIF_TABLE_ID2                89

#define XP_PORTVLAN_TABLE_ID1_MIRROR         XP_PORTVLAN_TABLE_ID2
#define XP_TNL_IVIF_TABLE_ID1_MIRROR1        XP_TNL_IVIF_TABLE_ID2
#define XP_MY_SID_TABLE_ID1_MIRROR1        XP_MY_SID_TABLE_ID2


#define XP_FDB_TABLE_ID1                     72
#define XP_FDB_TABLE_ID2                     75
#define XP_FDB_TABLE_ID1_MIRROR              XP_FDB_TABLE_ID2

#define XP_IPV4_HOST_TABLE_ID1               65
#define XP_IPV4_HOST_TABLE_ID2               90
#define XP_IPV4_HOST_TABLE_ID1_MIRROR        XP_IPV4_HOST_TABLE_ID2

#define XP_IPV6_HOST_TABLE_ID1               66
#define XP_IPV6_HOST_TABLE_ID2               91
#define XP_IPV6_HOST_TABLE_ID1_MIRROR        XP_IPV6_HOST_TABLE_ID2

#define XP_MPLS_LABEL_TABLE_ID1              67
#define XP_MPLS_LABEL_TABLE_ID2              92
#define XP_MPLS_LABEL_TABLE_ID1_MIRROR       XP_MPLS_LABEL_TABLE_ID2

#define XP_IPV4_BRIDGE_MC_TABLE_ID1          73
#define XP_IPV4_BRIDGE_MC_TABLE_ID2          76
#define XP_IPV4_BRIDGE_MC_TABLE_ID1_MIRROR   XP_IPV4_BRIDGE_MC_TABLE_ID2

#define XP_IPV6_BRIDGE_MC_TABLE_ID1         71
#define XP_IPV6_BRIDGE_MC_TABLE_ID2         95
#define XP_IPV6_BRIDGE_MC_TABLE_ID1_MIRROR   XP_IPV6_BRIDGE_MC_TABLE_ID2

#define XP_IPV4_ROUTE_MC_TABLE_ID1           93
#define XP_IPV4_ROUTE_MC_TABLE_ID2           93
#define XP_VNI_TABLE_ID2                     82
#define XP_IPV4_ROUTE_TABLE_ID2              162


#define XP_IPV6_ROUTE_MC_TABLE_ID1          69
#define XP_IPV6_ROUTE_MC_TABLE_ID2          94
#define XP_IPV6_ROUTE_MC_TABLE_ID1_MIRROR    XP_IPV6_ROUTE_MC_TABLE_ID2


#define XP_IPV4_PIM_BIDIR_RPF_TABLE_ID1            68
#define XP_IPV4_PIM_BIDIR_RPF_TABLE_ID2            68
#define XP_IPV4_PIM_BIDIR_RPF_TABLE_ID1_MIRROR     XP_IPV4_PIM_BIDIR_RPF_TABLE_ID2

#define XP_VNI_TABLE_ID1             88

#define XP_LOCAL_VTEP_TABLE_ID1      83
#define XP_LOCAL_VTEP_TABLE_ID2      84

#define XP_OPENFLOW0_TABLE_ID1            78
#define XP_OPENFLOW1_TABLE_ID1            79
#define XP_OPENFLOW2_TABLE_ID1            86
#define XP_OPENFLOW3_TABLE_ID1            85

#define XP_OPENFLOW0_0_TABLE_ID1           72
#define XP_OPENFLOW0_1_TABLE_ID1           73
#define XP_OPENFLOW0_2_TABLE_ID1           74
#define XP_OPENFLOW0_3_TABLE_ID1           75

#define XP_OPENFLOW1_0_TABLE_ID1           76
#define XP_OPENFLOW1_1_TABLE_ID1           77
#define XP_OPENFLOW1_2_TABLE_ID1           78
#define XP_OPENFLOW1_3_TABLE_ID1           79

#define XP_OPENFLOW0_TCAM_TABLE_ID1       106
#define XP_OPENFLOW1_TCAM_TABLE_ID1       108
#define XP_OPENFLOW2_TCAM_TABLE_ID1       110
#define XP_OPENFLOW3_TCAM_TABLE_ID1       102

#define XP_OPENFLOW0_TCAM_TABLE_ID2       107
#define XP_OPENFLOW1_TCAM_TABLE_ID2       109
#define XP_OPENFLOW2_TCAM_TABLE_ID2       111
#define XP_OPENFLOW3_TCAM_TABLE_ID2       103

#define XP_PURE_OPENFLOW0_TCAM_TABLE_ID1       96
#define XP_PURE_OPENFLOW1_TCAM_TABLE_ID1       98
#define XP_PURE_OPENFLOW2_TCAM_TABLE_ID1       100
#define XP_PURE_OPENFLOW3_TCAM_TABLE_ID1       102

#define XP_PURE_OPENFLOW0_TCAM_TABLE_ID2       97
#define XP_PURE_OPENFLOW1_TCAM_TABLE_ID2       99
#define XP_PURE_OPENFLOW2_TCAM_TABLE_ID2       101
#define XP_PURE_OPENFLOW3_TCAM_TABLE_ID2       103

#define XP_NULL_PORT_VIF                  4096

#define XP_INVALID_TCAM_TABLE_ID          -1

#define XP_EGRESS_QOS_MAP_TABLE_ID1       80
#define XP_EGRESS_QOS_MAP_TABLE_ID2       74

#define XP_COPP_TABLE_ID1                 87
#define XP_COPP_TABLE_ID2                 77

#define XP_VIF_INVALID_MDTPTR             0xFFFF ///< Value of invalid MDT pointer
#define XP_VIF_INVALID_INSPTR             0xFFFF ///< Value of invalid INS pointer
#define XP_VIF_INVALID_MODPTR             0xFF ///< Value of invalid MOD pointer
#define XP_VIF_INVALID_SINGLE_PORT        0xFF ///< Value of invalid port
#define XP_IACL_INVALID_FIELD             0x0  ///< Value of invalid ACL field
#define XP_EACL_INVALID_FIELD             0x0  ///< Value of invalid ACL field
#define XP_IACL_INVALID_DATA_FIELD        0xFFFF ///< Value of invalid iACL data field

#define MAX_PTRS_PER_MOD_PROFILE 6
#define MAX_PTRS_PER_INS_PROFILE 4

#define MOD_PTR_INVALID -1
#define INS_PTR_INVALID -1


//Table IDs for TCAM tables
#define XP_IACL0_TABLE_ID1                96
#define XP_IACL0_TABLE_ID2                97

#define XP_IACL1_TABLE_ID1                98
#define XP_IACL1_TABLE_ID2                99

#define XP_IACL2_TABLE_ID1                100
#define XP_IACL2_TABLE_ID2                101

#define XP_NAT_IPV4_TABLE_ID1                102
#define XP_NAT_IPV4_TABLE_ID2                103

#define XP_EACL0_TABLE_ID1                104
#define XP_EACL0_TABLE_ID2                105

//Table IDs for LPM tables

#define XP_IP_NH_TABLE_ID1      XP_SE_NH_LOGICAL_TABLE_ID_MIN
#define XP_IP_NH_TABLE_ID2      XP_SE_NH_LOGICAL_TABLE_ID_MAX

#define XP_PRE_LPM_TABLE_ID_MIN  XP_SE_NH_LOGICAL_TABLE_ID_MAX +  1
#define XP_PRE_LPM_TABLE_ID_MAX  XP_SE_NH_LOGICAL_TABLE_ID_MAX +  4

#define XP_PRE_AND_OVERFLOW_TCAM_MIN  XP_PRE_LPM_TABLE_ID_MAX + 1
#define XP_PRE_AND_OVERFLOW_TCAM_MAX  XP_PRE_LPM_TABLE_ID_MAX + 16


#define XP_IPV4_ROUTE_TABLE_ID1              166
#define XP_IPV4_ROUTE_TABLE_ID1_MIRROR       XP_IPV4_ROUTE_TABLE_ID2

#define XP_IPV6_ROUTE_TABLE_ID1              167
#define XP_IPV6_ROUTE_TABLE_ID2              163
#define XP_IPV6_ROUTE_TABLE_ID1_MIRROR       XP_IPV6_ROUTE_TABLE_ID2

//Table IDs for URW tables
#define XP_SE_VIF_EGR_32_TBL_ID1             130
#define XP_SE_VIF_EGR_32_TBL_ID2             -1
#define XP_SE_VIF_ING_32_TBL_ID1             153
#define XP_SE_VIF_ING_32_TBL_ID2             154
#define XP_SE_VIF_EGR_64_TBL_ID1             132
#define XP_SE_VIF_EGR_64_TBL_ID2             133
#define XP_SE_VIF_ING_64_TBL_ID1             128
#define XP_SE_VIF_ING_64_TBL_ID2             129
#define XP_SE_VIF_EGR_128_TBL_ID1            136
#define XP_SE_VIF_EGR_128_TBL_ID2            137
#define XP_SE_VIF_ING_128_TBL_ID1            -1
#define XP_SE_VIF_ING_128_TBL_ID2            -1
#define XP_SE_VIF_EGR_256_TBL_ID1            140
#define XP_SE_VIF_EGR_256_TBL_ID2            141
#define XP_SE_VIF_ING_256_TBL_ID1            144
#define XP_SE_VIF_ING_256_TBL_ID2            145
#define XP_URW_SE_IVIF_PORTBITMAP_PTR_TBL_ID1     140
#define XP_URW_SE_EVIF_PORTBITMAP_PTR_TBL_ID1     141
#define XP_SE_INS0_128_TBL_ID1               148
#define XP_SE_INS0_128_TBL_ID2               149
#define XP_SE_INS1_128_TBL_ID1               152
#define XP_SE_INS1_128_TBL_ID2               150
#define XP_SE_INS2_128_TBL_ID1               156
#define XP_SE_INS2_128_TBL_ID2               -1//157
#define XP_MDT_TABLE_ID1                     158
#define XP_MDT_TABLE_ID2                     -1

#define XP_EGRESS_BD_TABLE_ID1               1
#define XP_EGRESS_BD_TABLE_ID2               2

#define XP_MPLS_OUTER_LBL_TBL_ID1            XP_ISME0_LDE3_TBL_ID
#define XP_MPLS_OUTER_LBL_TBL_ID2            XP_ISME0_LDE0_TBL_ID
#define XP_MPLS_OUTER_LBL_TBL_ID3            XP_ISME3_LDE3_TBL_ID
#define XP_MPLS_OUTER_LBL_TBL_ID4            XP_ISME3_LDE0_TBL_ID

#define XP_BD_TBL_ID1                        XP_ISME1_LDE1_TBL_ID
#define XP_BD_TBL_ID2                        XP_ISME2_LDE0_TBL_ID
#define XP_BD_TBL_ID3                        XP_ISME4_LDE1_TBL_ID
#define XP_BD_TBL_ID4                        XP_ISME5_LDE0_TBL_ID


/// XP_OK is the same as XP_NO_ERR
#define XP_OK                   XP_NO_ERR

#define XP_SE_MAX_NUM_TABLE_ID                  168
#define XP_SRAM_TYPE0_POOL_MIN                  0
#define XP_SRAM_TYPE0_POOL_MAX                  3
#define XP_SRAM_TYPE1_POOL_MIN                  4
#define XP_SRAM_TYPE1_POOL_MAX                  7
#define XP_SE_NUM_OF_SRAM_ROW_BLK_PER_TILE      8 //number of SRAM Row Block Per Tile
#define XP_SRAM_WIDTH                           128 //SDE TCAM width in bits
//#define XP_SRAM_DEPTH               4096 //SDE TCAM width in bits
#define XP_TCAM_WIDTH                           65 //SDE TCAM width in bits
#define XP_KPU_TABLES_NUM                       48
#define XP_CANONICAL_CMD_NUM                    128
#define XP_CANONICAL_HD_KEY_LEN                 17
#define XP_CANONICAL_HD_HASH_LEN                7
#define XP_CANONICAL_HD_LEFT_POLY               0xFF
#define XP_CANONICAL_HD_LEFT_HASH_INDX          0
#define XP_CANONICAL_HD_RIGHT_HASH_INDX         1
#define XP_CANONICAL_HD_RIGHT_POLY              0xC9
#define XP_CANONICAL_CMD_WIDTH                  98
#define XP_KPU_NUM                              24
#define XP_MAX_LAYERMAPS                        8
#define XP_CANONICAL_HD_CMD_NUM                 4
#define XP_NUM_UTRT_PER_ISME                    6
#define XP_SE_HASH_PER_WAY_NUM                  4
#define XP_INDEX_LIST_SIZE_FOR_SINGLE_LEVEL_REHASH      2
#define XP_INDEX_LIST_SIZE_FOR_NO_REHASH        1
#define XP_SINGLE_REHASH_LEVEL         1


typedef uint16_t HASH_POLY_TYPE;


/*All forwarding egressFilterId*/
#define XP_ALL_FORWARDING_EGRESS_FILTER_ID 254
#define XP_CONTROL_PKT_ERRATA_EGRESS_FLT_ID 255

/**
 * Mirroring Sessions:
 */
#define XP_MIRROR_MIN_USER_SESSION          0
#define XP_MIRROR_MAX_USER_SESSION          6
#define XP_MIRROR_ILLEGAL_SESSION           0xFF
#define XP_MIRROR_IS_USER_SESSION(SSN)     (XP_MIRROR_MIN_USER_SESSION     <= (SSN) && (SSN) <= XP_MIRROR_MAX_USER_SESSION)

#define XP_MHZ_TO_HZ 1000*1000
#define XP_KBPS_TO_GBPS_DIVIDER 1000*1000
#define XP_KBPS_TO_BITS_FACTOR 1000
#define XP_BITS_IN_BYTE 8

// *** WARNING: THE FOLLOWING MUST BE REPLACED BY A CAL OPERATION ***
#define XP_MDT_NULL_PTR(DEV_ID)             0   // Should look up device type based on device ID and return correct value.

/*
 * List of LDE engines
 */
typedef enum
{
    XP_LDE_ENGINE_OF0,
    XP_LDE_ENGINE_OF1,
    XP_LDE_ENGINE_OF2,
    XP_LDE_ENGINE_OF3,
    XP_LDE_ENGINE_OF4,
    XP_LDE_ENGINE_OF5,
    XP_LDE_ENGINE_OF6,
    XP_LDE_ENGINE_OF7,
    XP_LDE_ENGINE_URW,
} xpEngineType_t;

// Covers both SDE 0 and SDE 1:
#define XP_ISME_IS_1X1(N)       (((N) == 1) || ((N) == 2) || ((N) == 4) || ((N) == 5))
#define XP_ISME_IS_2X1(N)       (((N) == 0) || ((N) == 3))

typedef enum //XP supported key lengths for SDE TCAM
{
    KEY_LEN_64 = 64,
    KEY_LEN_128 = 128,
    KEY_LEN_192 = 192,
    KEY_LEN_384 = 384
} XP_TCAM_KEY_LEN_T;

typedef enum  //SE block number where a table is located. SE_GLOBAL is for a table that should serve both SDEs
{
    SE_0,
    SE_1,
    SE_GLOBAL
} XP_SE_BLOCK_T;

typedef enum   //XP table types
{
    DIRECT_ACCESS,
    HASH,
    LPM,
    MATCH,
    SRAM,
    URW,
    POLICER_CONFIG,
    ISME,
    NH,
    URW_DIRECT_ACCESS,
    WCM,
    PRE_LPM,
    INVALID
} XP_TABLE_TYPE_T;

typedef enum
{
    TABLE_INIT_WITH_DYNAMIC_INFO,   // Create Table at Init Time with Sram/Db Pool Info Dynamically getting Allocated
    TABLE_INIT_WITH_FIXED_INFO,     // Create Table at Init Time with Sram/Db Pool Info being Fixed
    TABLE_RECREATE_FROM_HW_INFO, // Recreate Table once the Init is done in Muli Process Env with Sram/Db Pool Info Being read from HW
    TABLE_RECREATE_FROM_SW_INFO,  // Recreate Table once the Init is done in Muli Process Env with Sram/Db Pool Info Being Passed
    TABLE_INVALID_INFO,
} XP_TABLE_MODE_T;

typedef enum    ///XTCAM supported width. Width has 2 parts - data and control data (valid bits)
{
    NOT_APPLICABLE = 0,
    WIDTH_64 = 64 + 1,
    WIDTH_128 = 128 + 2,
    WIDTH_192 = 192 + 3,
    WIDTH_384 = 384 + 6

} XP_TCAM_WIDTH_T;

typedef enum
{
    TCAM_DATA_32 = 32,
    TCAM_DATA_64 = 64,
    TCAM_DATA_128 = 128,
    TCAM_DATA_256 = 256,
} XP_TCAM_DATA_TYPE;

typedef uint16_t XP_TABLE_WIDTH_TYPE;

typedef enum    //XP SDE HASH supported key sizes
{
    KEY_64 = 64,
    KEY_128 = 128,
    KEY_256 = 256,

} XP_HASH_KEY_SIZE_T;

typedef enum    //XP SDE HASH supported action entry sizes
{
    HASH_ENTRY_64 = 64,
    HASH_ENTRY_112 = 112,
    HASH_ENTRY_128 = 128,
    HASH_ENTRY_240 = 240,
    HASH_ENTRY_256 = 256,
    HASH_ENTRY_512 = 512,

} XP_SE_HASH_ENTRY_SIZE_T;

typedef enum    //XP SDE HASH supported action entry sizes
{
    HASH_KEYSIZE_32 = 32,
    HASH_KEYSIZE_64 = 64,
    HASH_KEYSIZE_72 = 72,
    HASH_KEYSIZE_80 = 80,
    HASH_KEYSIZE_96 = 96,
    HASH_KEYSIZE_128 = 128,
    HASH_KEYSIZE_144 = 144,
    HASH_KEYSIZE_176 = 176,
    HASH_KEYSIZE_192 = 192,
    HASH_KEYSIZE_208 = 208,
    HASH_KEYSIZE_256 = 256,
    HASH_KEYSIZE_288 = 288

} XP_SE_HASH_KEY_SIZE_T;

typedef enum
{
    HASH_TABLE_NO_SHARE_ENTRY = 0,
    HASH_TABLE_SHARE_ONLY_64_ENTRY,
    HASH_TABLE_SHARE_ONLY_128_ENTRY,
    HASH_TABLE_SHARE_ONLY_256_ENTRY,
    HASH_TABLE_SHARE_ONLY_512_ENTRY,
    HASH_TABLE_SHARE_MIX_OF_64_AND_ALL_HIGHER_ENTRY_INCL_512,
    HASH_TABLE_SHARE_MIX_OF_64_AND_ALL_HIGHER_ENTRY_EXCL_512,
    HASH_TABLE_SHARE_MIX_OF_128_AND_ALL_HIGHER_ENTRY_INCL_512,
    HASH_TABLE_SHARE_MIX_OF_128_AND_ALL_HIGHER_ENTRY_EXCL_512,
    HASH_TABLE_SHARE_MIX_OF_256_AND_ALL_HIGHER_ENTRY_INCL_512,
    HASH_TABLE_SHARE_MIX_OF_256_AND_ALL_HIGHER_ENTRY_EXCL_512,
    HASH_TABLE_SHARE_INVALID_ENTRY
} HASH_TABLE_SHARE_GROUP_TYPE;

typedef enum
{
    XP70_TCAM_TYPE_WCM = 1,
    XP70_TCAM_TYPE_REGULAR = 2,
    XP70_TCAM_TYPE_SUBNET_SIP = 3,
    XP70_TCAM_TYPE_SUBNET_DIP = 4,
    XP70_TCAM_TYPE_INVALID
} xp70TcamType_t;

#if defined (DXP100) || defined (ALL_XPDEV)
typedef enum
{
    XP100_TCAM_TYPE_WCM = 1,
    XP100_TCAM_TYPE_REGULAR = 2,
    XP100_TCAM_TYPE_SUBNET_SIP = 3,
    XP100_TCAM_TYPE_SUBNET_DIP = 4,
    XP100_TCAM_TYPE_INVALID
} xp100TcamType_t;
#endif // DXP100

typedef enum
{
    WCM_ENTRY_TYPE0 = 0,
    WCM_ENTRY_TYPE1,
    WCM_ENTRY_TYPE2,
    WCM_ENTRY_TYPE3,
    WCM_ENTRY_MAX
} xpWcmEntryType;


typedef struct xpSePoolTProfile_t
{
    uint32_t dbBmp;
    uint32_t tcamProfId;
    uint32_t tcamDataProfId;
    xp70TcamType_t tcamType;
    uint32_t tblType;
    xpWcmEntryType entryType;
} xpSePoolTProfile;

typedef struct xpSePoolTSramProfile_t
{
    uint32_t tcamDataProfId;
    uint32_t tileIdList;
    uint32_t valueSize;
    xp70TcamType_t tcamType;
    uint32_t ageProfileId;
} xpSePoolTSramProfile;


typedef struct xpSePoolAProfile_t
{
    uint32_t usePathB;
    uint32_t pathB;
    uint32_t entryPtSel;
    uint32_t sramStart;
    uint32_t sramEnd;
} xpSePoolAProfile;

typedef struct xpSeXlatorSelProfile_t
{
    uint32_t selEn;
    uint32_t targetSel;
    uint32_t xlatorProfileId;
} xpSeXlatorSelProfile;

typedef struct xpSeXlatorProfile_t
{
    uint32_t rangeMatch;
    uint32_t ipv4OrV6;
    uint32_t poolAProfileId;
    uint32_t sipEn;
    uint32_t dipEn;
    uint32_t sipByteStart;
    uint32_t dipByteStart;
    uint32_t sportByteStart;
    uint32_t dportByteStart;
    uint32_t sipTileChainId;
    uint32_t sipTileBmp;
    uint32_t dipTileChainId;
    uint32_t dipTileBmp;
    uint32_t stateTblByteStart;
    uint32_t pathTblByteStart;
    uint64_t keyByteBmp;
    uint32_t byteShift;
    uint32_t fieldSub;
} xpSeXlatorProfile;

typedef struct xpSeMgHashProfile_t
{
    uint32_t lookupMode;
    uint32_t directAcEn;
    uint32_t directAcNibbleStart;
    uint32_t keyType;
    uint32_t keyGran;
    uint64_t keyBmp;
    uint32_t keyMaskByteOff3;
    uint32_t keyMaskBitLen3;
    uint32_t keyMaskByteOff2;
    uint32_t keyMaskBitLen2;
    uint32_t keyMaskByteOff1;
    uint32_t keyMaskBitLen1;
    uint32_t keyMaskByteOff0;
    uint32_t keyMaskBitLen0;
    uint32_t keySize;
    uint32_t rsltType;
} xpSeMgHashProfile;

typedef struct xpSePoolAEntryExit_t
{
    uint32_t exit_point34_set1_en;
    uint32_t exit_point34_set0_en;
    uint32_t exit_point22_set1_en;
    uint32_t exit_point22_set0_en;
    uint32_t entry_point34_rslt_lane_set_id;
    uint32_t entry_point22_rslt_lane_set_id;
    uint32_t entry_point12_rslt_lane_set_id;
    uint32_t entry_point34_en;
    uint32_t entry_point22_en;
    uint32_t entry_point12_en;
    uint32_t entry_point0_en;
} xpSePoolAEntryExit;

/* struct for reg field info to get position and length */
typedef struct xpRegFieldInfo_t
{
    uint32_t regId;
    int fieldPos;
    int fieldLen;
    int valid;
} XP_REG_FIELD_INFO_T;

/*
   typedef enum    //XP SDE TCAM supported width
   {
   WIDTH_64 = 64,
   WIDTH_128 = 128,
   WIDTH_192 = 192,
   WIDTH_384 = 384

   }XP_TABLE_WIDTH_TYPE;
   */

typedef enum    // Add names for all the blocking thread macro's here.
{
    XP_LOG_PORT_TUNE_AND_AN_THREAD_BLOCK = 0,
    XP_LOG_THREAD_BLOCK_MAX     // Must be smaller than XP_LOG_MAX_SUBMODS.

} XP_LOG_PORT_TUNE_AND_AN_THREAD_BLOCK_T;

typedef enum
{
    DA_SHORTEST_ENTRY_SIZE = 32,
    DA_ENTRY_SIZE_1 = 64,
    DA_ENTRY_SIZE_2 = 128,
    DA_LONGEST_ENTRY_SIZE = 256

} XP_SE_DIRECTACCESS_TABLE_ENTSIZE_T;

typedef enum XP_PRINT_MEM_TYPE_E
{
    XP_NO_PRINT,
    XP_REG_ONLY,
    XP_TABLE_ONLY,
    XP_REG_AND_TABLE,
    XP_PRINT_MEM_TYPE_TOTAL
} XP_PRINT_MEM_TYPE_T;

typedef enum
{
    XP_DMA_TABLE_FROM_HW_TO_SW,
    XP_DMA_TABLE_FROM_SW_TO_HW,
    XP_DMA_TABLE_INVALID_OPTION
} xpTableDmaDir;

typedef enum
{
    XP_CORE_CLK_FREQ_400_MHZ,
    XP_CORE_CLK_FREQ_500_MHZ,
    XP_CORE_CLK_FREQ_550_MHZ,
    XP_CORE_CLK_FREQ_600_MHZ,
    XP_CORE_CLK_FREQ_633_MHZ,
    XP_CORE_CLK_FREQ_650_MHZ,
    XP_CORE_CLK_FREQ_683_MHZ,
    XP_CORE_CLK_FREQ_700_MHZ,
    XP_CORE_CLK_FREQ_733_MHZ,
    XP_CORE_CLK_FREQ_750_MHZ,
    XP_CORE_CLK_FREQ_783_MHZ,
    XP_CORE_CLK_FREQ_800_MHZ,
    XP_CORE_CLK_FREQ_833_MHZ,
    XP_CORE_CLK_FREQ_850_MHZ,
    XP_CORE_CLK_FREQ_900_MHZ,
    XP_CORE_CLK_FREQ_950_MHZ,
    XP_CORE_CLK_FREQ_1000_MHZ,
    XP_CORE_CLK_FREQ_MAX
} xpCoreClkFreq_t;

typedef enum    //XP SDE WCM supported action entry sizes
{
    WCM_VALUESIZE_32 = 32,
    WCM_VALUESIZE_64 = 64,
    WCM_VALUESIZE_128 = 128,
    WCM_VALUESIZE_256 = 256,

} XP_SE_WCM_VALUE_SIZE_T;

typedef enum    //XP SDE WCM supported key sizes
{
    WCM_KEYSIZE_256 = 256,

} XP_SE_WCM_KEY_SIZE_T;

/**
 * \typedef xpRangeType_t
 * \public
 * \brief This type (enum) defines type of range table
 */
typedef enum xpRangeType
{
    RANGE_TABLE_A,
    RANGE_TABLE_B
} xpRangeType_t;

typedef enum xpTranslationType
{
    XP_TRANS_NONE = 0,
    XP_RANGE_A_TRANS,
    XP_RANGE_B_TRANS,
    XP_PREFIX_A_TRANS,
    XP_PREFIX_B_TRANS,
    XP_DIR_TRANS,
} xpTranslationType_t;

typedef enum
{
    XP_WCM_INSTR_NONE = 0,
    XP_WCM_INSTR_NOT_MATCH,
    XP_WCM_INSTR_MATCH,
    XP_WCM_INSTR_IN_RANGE,
    XP_WCM_INSTR_OUT_RANGE,
    XP_WCM_MAX_INSTR
} xpWcmInstrType_t;

typedef struct xpWcmMatchInstr
{
    uint32_t nibbleEndIdx;
    uint32_t bitLength;
} xpWcmMatchInstr_t;

typedef struct xpWcmRangeInstr
{
    uint32_t byteStartOffset;
    uint32_t upperBound;
} xpWcmRangeInstr_t;

typedef union xpWcmInstr
{
    xpWcmMatchInstr_t matchInstr;
    xpWcmRangeInstr_t rangeInstr;
} xpWcmInstr_t;

typedef struct xpWcmInstrs
{
    uint32_t instrCnt;
    xpWcmInstrType_t instrType[XP_WCM_MAX_INSTR];
    xpWcmInstr_t instr[XP_WCM_MAX_INSTR];
} xpWcmInstrs_t;

typedef enum
{
    XP_MAC_CLK_FREQ_800_MHZ,
    XP_MAC_CLK_FREQ_833_MHZ,
    XP_MAC_CLK_FREQ_883_MHZ,
    XP_MAC_CLK_FREQ_MAX
} xpMacClkFreq_t;


typedef struct xpPrefix
{
    xpIpPrefixType_t type;       ///< IP Prefix type
    ipv4Addr_t ipv4Addr;         ///< IPv4
    ipv6Addr_t ipv6Addr;         ///< IPv6
    uint32_t   ipMaskLen;        ///< IP Mask
} xpPrefix_t;

/**
 * \struct xpIdRangeInfo
 * \public
 * \brief This struct represents depth and start offset of the ID range at an index.
 *
 **/
typedef struct xpIdRangeInfo
{
    uint8_t index;
    uint32_t start;
    uint32_t depth;
} xpIdRangeInfo;

/**
 * \brief This structure defines IPv4 rehashed route entry
 */
typedef struct xpIpv4RouteRehashEntry_t
{
    uint32_t vrfId;              ///< VRF ID
    ipv4Addr_t ipv4Addr;         ///< IPv4
    uint32_t ipMaskLen;          ///< IP Mask
    uint32_t bucketIdx;          ///< LPM bucket index
    uint8_t isNewBucket;         ///< New bucket
} xpIpv4RouteRehashEntry_t;

/**
 * \brief This structure defines IPv4 rehashed route entry
 */
typedef struct xpIpv6RouteRehashEntry_t
{
    uint32_t vrfId;              ///< VRF ID
    ipv6Addr_t ipv6Addr;         ///< IPv6
    uint32_t ipMaskLen;          ///< IP Mask
    uint32_t bucketIdx;          ///< LPM bucket index
    uint8_t isNewBucket;         ///< New bucket
} xpIpv6RouteRehashEntry_t;

#define XP_AGE_DISABLE_TABLE_ID   -2

typedef XP_ACM_RANGE_TYPE xpAcmRangeType_t;

#ifndef CPU_RX_TX_OVER_IPC
struct xpDmaSharedMemInfo
{
    uint64_t dmaSharedMemPtr;
    int dmaSharedMemId;
};
#endif
typedef enum _XP_URW_VIF_ENTRY_MODE
{
    XP_URW_SE_VIF_ENTRY_MODE_STD        = 0,
    XP_URW_SE_VIF_ENTRY_MODE_INS        = 1,
    XP_URW_SE_VIF_ENTRY_MODE_MRE        = 2,
    XP_URW_SE_VIF_ENTRY_MODE_EVIF       = 3,
    XP_URW_SE_VIF_INVALID_ENTRY_MODE    = 4,
} XP_URW_VIF_ENTRY_MODE;

typedef XP_MCAST_DOMAIN_ID_TYPE xpMcastDomainIdType_t;
typedef XP_VIF_PORTBITMAPPTR_TABLE_TYPE  xpVifPortBitMapPtrIdType_t;

/* If user's platform has repeaters available on board which needs to be initialized when
 * port is getting initialized then this function needs to be implemented by
 * board specific code and user application should register that function in this function pointer
 * In current implmenetation of XDK, when xp(s)App is run with "-t svb" option this
 * function pointer is registered.
 * */
typedef XP_STATUS(*platformSerdesInitFunc)(xpDevice_t devId, uint8_t serdesId,
                                           uint32_t txWidth, uint32_t rxWidth, uint32_t divider,
                                           uint8_t* isTxPolaritySwapped, uint8_t* isRxPolaritySwapped);

/* To get the seredes lane swap information, within the MAC/PTG the serdes lanes can be based on the board layout.
 * This function needs to be implemented by board specific code and user application should register that function in this function pointer
 * In current implmenetation of XDK, when xp(s)App is run with "-t svb" option this
 * function pointer is registered.
 * */
typedef XP_STATUS(*platformGetSerdesLaneSwapFunc)(xpDevice_t devId,
                                                  uint8_t macNum, uint8_t channelNum, uint8_t* channelSwappedTo);

/* This function pointer needs to be filled with a board specific function which provides
 * the mapping between Chassis port vs XP port# and the serdes polarity swapping info on board.
 * When user runs xp(s)App with -t option xp(s)App will fill this function pointer by calling
 * xpsMacRegisterGetChassisPortInfoFunc(board_specific_function)
 * When user don't run xp(s)App the xpsMacRegisterGetChassisPortInfoFunc() or
 * xpLinkManagerRegisterGetChassisPortInfoFunc() API needs to be called by user
 * application to register board specific function to get this mapping.
 * */
typedef XP_STATUS(*getChassisPortInfoFunc)(xpDevice_t devId, uint8_t xpPortNum,
                                           uint8_t serdesIdx, xpMacConfigMode macConfigMode, char* chassisPortName,
                                           uint8_t* isTxPolaritySwapped, uint8_t* isRxPolaritySwapped,
                                           void *structSerdesTxEq_t);

/* This function is use to register interrupt event handler
 * */
typedef void* (*xpEventSignalHandler)(void *);

typedef int (*i2cDevOpen)();
typedef int (*i2cDevClose)(int i2cDevFd);
typedef int (*i2cDevRw)(int i2cDevFd, void* xfer);
typedef void* (*i2cDongleOpen)(void *dev);
typedef int (*i2cDongleClose)(void *hndl);
typedef int (*i2cDongleRead)(void * hndl, int sa, int32_t ma, int ma_sz,
                             char* buf, int sz);
typedef int (*i2cDongleWrite)(void * hndl, int sa, int32_t ma, int ma_sz,
                              char* buf, int sz);
typedef int (*i2cDongleHsRw)(void * hndl, int mcode, int sa, int r_w, int sz,
                             char *data, int *status, int *act_sz);

/*
* This callback is used to compute port bit map index for ingress/egress direction
*/
typedef XP_STATUS(*xpAllocPortBitMapIndex)(xpDevice_t deviceId, XP_DIR_E dir,
                                           xpVifPortBitMapPtrId_t* portbitMapIndex);

/*
* This callback is used to release port bit map index for ingress/egress direction
*/
typedef XP_STATUS(*xpReleasePortBitMapIndex)(xpDevice_t deviceId, XP_DIR_E dir,
                                             xpVifPortBitMapPtrId_t portbitMapIndex);

/*Print formats, and value for printing mac, v4 addr and v6 addr*/

#define FMT_MAC " %02x:%02x:%02x:%02x:%02x:%02x "
#define FMT_IP4 " %u.%u.%u.%u "
#define FMT_IP6 " %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x "

#define PRI_MAC(x) x[0],x[1],x[2],x[3],x[4],x[5]
#define PRI_IP4(x)    x[0],\
                      x[1],\
                      x[2],\
                      x[3]
//use this if ipv4 is integer type
//#if little endian
#define PRI_IP4I(x) \
        *((unsigned char *)&x + 3), \
        *((unsigned char *)&x + 2), \
        *((unsigned char *)&x + 1), \
        *((unsigned char *)&x + 0)

#define PRI_IP6(x)    x[0],\
                        x[1],\
                        x[2],\
                        x[3],\
                        x[4],\
                        x[5],\
                        x[6],\
                        x[7],\
                        x[8],\
                        x[9],\
                        x[10],\
                        x[11],\
                        x[12],\
                        x[13],\
                        x[14],\
                        x[15]

#ifdef __cplusplus
/***************************************************************************************************/
//typedef std::bitset<XP_PARSER_INPUT_BITLEN> PARSER_INPUT_BITSET_TYPE;

typedef std::bitset<XP_IKT_MEM_BITLEN> IKT_MEM_BITSET_TYPE;
typedef std::bitset<XP_ITT_MEM_BITLEN> ITT_MEM_BITSET_TYPE;
typedef std::bitset<XP_KPU_MEM_BITLEN> KPU_MEM_BITSET_TYPE;
typedef std::bitset<XP_SKPU_MEM_BITLEN> SKPU_MEM_BITSET_TYPE ;
typedef std::bitset<XP_LAYERCMD_TEMPLATEID_MEM_BITLEN>
LAYERCMD_TEMPLATEID_MEM_BITSET_TYPE;
typedef std::bitset<XP_CANONCMD_MEM_BITLEN> CANONCMD_MEM_BITSET_TYPE;
typedef std::bitset<XP_TOKENCMD_MEM_BITLEN> TOKENCMD_MEM_BITSET_TYPE;
typedef std::bitset<XP_QOS_MEM_BITLEN> QOS_MEM_BITSET_TYPE;


typedef std::bitset<WIDTH_64> DATA_64_BITSET_TYPE;
typedef std::bitset<WIDTH_64>  DATA_64_BITSET_TYPE;
typedef std::bitset<WIDTH_128> DATA_128_BITSET_TYPE;
typedef std::bitset<WIDTH_192> DATA_192_BITSET_TYPE;
typedef std::bitset<WIDTH_384> DATA_384_BITSET_TYPE;

typedef std::vector<xpPort_t> xpPortList_t;
typedef std::vector<xpIndex_t> xpIndexList_t;
typedef std::vector<xpLag_t> xpLagList_t;
typedef std::vector<xpTunnel_t> xpTunnelList_t;

typedef std::vector<xpPort_t>::iterator xpPortListIter_t;

typedef std::vector<xpVif_t> xpVifList_t;

#endif

#endif  //in SWIG


