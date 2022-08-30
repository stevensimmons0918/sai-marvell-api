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
* @file prvCpssDxChHwTables.c
*
* @brief Private API implementation for tables access.
*
* @version   178
********************************************************************************
*/
#include <cpss/generic/cpssHwInit/cpssHwInit.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwTables.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/private/routerTunnelTermTcam/prvCpssDxChRouterTunnelTermTcam.h>
#include <cpssDriver/pp/config/prvCpssDrvPpCfgInit.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChTables.h>
#include <cpss/dxCh/dxChxGen/diag/private/prvCpssDxChDiag.h>
#include <cpss/dxCh/dxChxGen/diag/private/prvCpssDxChDiagDataIntegrityMainMappingDb.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


/*global variables macros*/
#define PRV_SHARED_HW_INIT_TABLES_DB_VAR_SET(_var,_value)\
      PRV_SHARED_GLOBAL_VAR_SET(mainPpDrvMod.dxChCpssHwInit.tablesSrc._var,_value)

#define PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(_var)\
      PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.dxChCpssHwInit.tablesSrc._var)

#define PRV_SHAREG_GLOBAL_TABLE_ACCESS_OBJECT \
      PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.dxChCpssHwInit.tablesSrc.prvDxChTableCpssHwAccessObj)

/**
* @internal prvCpssFalconRavenMemoryAddressSkipCheck function
* @endinternal
*
* @brief   Check skipping of non-valid Raven memory addresses in Falcon device.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum         - the device number
* @param[in] address        - memory address in Raven device to be sampled
* @param[in] skipUnitPtr    - pointer to) memory address status
*                               GT_TRUE     - the address should be skipped
*                               GT_FALSE    - the address should NOT be skipped
*/
extern GT_VOID prvCpssFalconRavenMemoryAddressSkipCheck
(
    IN GT_U8                   devNum,
    IN GT_U32                  address,
    IN GT_BOOL                 *skipUnitPtr
);

/* "not modify" value for unsigned integer fields  */
#define NOT_MODIFIED_VALUE_CNS  0xFFFFFFFF

/* update the address according to new unit base address */
#define UPDATE_ADDR_WITH_NEW_UNIT_MAC(oldAddr,newUnitBaseAddr)  \
                    (oldAddr) = ((oldAddr) & 0x00FFFFFF) + (newUnitBaseAddr)

/* max num of vlans entries in Mapping table -- range 0..4095 */
#define PRV_CPSS_DXCH_MAX_NUM_VLANS_CNS (4096)

/* max num of adjacency entries in TRILL Adjacency Check table -- range 0..127 */
#define PRV_CPSS_DXCH_MAX_NUM_ADJACENCY_CNS (128)

/* number of bits in the Bobcat2; Caelum; Bobcat3 - TTI action entry */
#define SIP5_TTI_ACTION_BITS_SIZE_CNS  246
/* number of bits in the Bobcat2; Caelum; Bobcat3 - TTI physical port entry */
#define SIP5_TTI_PHYSICAL_PORT_BITS_SIZE_CNS  56
/* number of bits in the Hawk - TTI physical port entry */
#define SIP6_10_TTI_PHYSICAL_PORT_BITS_SIZE_CNS  92
/* number of bits in the Bobcat2; Caelum; Bobcat3 - TTI default port entry */
#define SIP5_TTI_DEFAULT_PORT_BITS_SIZE_CNS  126
/* number of bits in the Bobcat2; Caelum; Bobcat3 - TTI PCL UDB entry */
#define SIP5_TTI_PCL_UDB_BITS_SIZE_CNS  550
/* number of bits in the Bobcat2; Caelum; Bobcat3 - L2I ingress vlan entry */
#define SIP5_L2I_INGRESS_VLAN_BITS_SIZE_CNS  113
/* number of bits in the Bobcat2; Caelum; Bobcat3 - L2I Ingress Bridge Port Membership Table */
#define SIP5_L2I_INGRESS_BRIDGE_PORT_MEMBERS_BITS_SIZE_CNS  256
/* number of bits in the Bobcat2; Caelum; Bobcat3 - L2I Ingress Span State Group Index Table */
#define SIP5_L2I_INGRESS_SPAN_STATE_GROUP_INDEX_BITS_SIZE_CNS  12
/* number of bits in the Bobcat2; Caelum; Bobcat3 - L2I ingress eport entry */
#define SIP5_L2I_EPORT_BITS_SIZE_CNS  79
/* number of bits in the Bobcat2; Caelum; Bobcat3 - L2I ingress STP entry */
#define SIP5_L2I_INGRESS_STP_BITS_SIZE_CNS  512
/* number of bits in the Bobcat2; Caelum; Bobcat3 - L2I ingress eport learn prio entry */
#define SIP5_L2I_EPORT_LEARN_PRIO_BITS_SIZE_CNS  40
/* number of bits in the Bobcat2; Caelum; Bobcat3 - l2i source trunk attribute entry */
#define SIP5_L2I_SOURCE_TRUNK_ATTRIBUTE_BITS_SIZE_CNS 40
/* number of bits in the Bobcat2; Caelum; Bobcat3 - L2I ingress physical port entry */
#define SIP5_L2I_PHYSICAL_PORT_BITS_SIZE_CNS   29
/* number of bits in the Bobcat2; Caelum; Bobcat3 - L2I ingress physical port entry */
#define SIP5_L2I_PHYSICAL_PORT_RATE_LIMIT_COUNTERS_BITS_SIZE_CNS   22
/* number of bits in the Bobcat2; Caelum; Bobcat3 - EGF_QAG egress eport entry */
#define SIP5_EGF_QAG_EGRESS_EPORT_BITS_SIZE_CNS    47
/* number of bits in the Bobcat2; Caelum; Bobcat3 - EGF_QAG egress eport entry */
#define SIP5_EGF_QAG_EGRESS_EVLAN_BITS_SIZE_CNS    768
/* number of bits in the Bobcat2; Caelum; Bobcat3 - EGF_QAG Target Port Mapper entry */
#define SIP5_EGF_QAG_TARGET_PORT_MAPPER_BITS_SIZE_CNS    8
/* number of bits in the Bobcat2; Caelum; Bobcat3 - EGF_QAG Code To Loopback Mapper entry */
#define SIP5_EGF_QAG_CPU_CODE_TO_LOOPBACK_MAPPER_BITS_SIZE_CNS    17
/* number of bits in the Bobcat2; Caelum; Bobcat3 - EGF_QAG CPU To Loopback Mapper entry */
#define SIP5_EGF_QAG_FWD_FROM_CPU_TO_LOOPBACK_MAPPER_BITS_SIZE_CNS    16
/* number of bits in the Bobcat2; Caelum; Bobcat3 - EGF_SHT egress eport entry */
#define SIP5_EGF_SHT_EGRESS_EPORT_BITS_SIZE_CNS    15
/* number of bits in the Bobcat2; Caelum; Bobcat3 - EGF_SHT egress vlan entry */
#define SIP5_EGF_SHT_EGRESS_VLAN_BITS_SIZE_CNS    257
/* number of bits in the Bobcat2; Caelum; Bobcat3 - EGF_SHT egress vlan attributes entry */
#define SIP5_EGF_SHT_EGRESS_VLAN_ATTRIBUTES_BITS_SIZE_CNS 6
/* number of bits in the Bobcat2; Caelum; Bobcat3 - EGF_SHT egress vlan spanning entry */
#define SIP5_EGF_SHT_EGRESS_VLAN_SPANNING_BITS_SIZE_CNS 12
/* number of bits in the Bobcat2; Caelum; Bobcat3 - EGF_SHT egress vlan mapper entry */
#define SIP5_EGF_SHT_EGRESS_VLAN_MAPPER_BITS_SIZE_CNS 12

/* number of bits in the Falcon - TTI UDB Config entry*/
#define SIP6_TTI_UDB_CONFIG_BITS_SIZE_CNS               360

/* number of bits in the Hawk - TTI UDB Config entry*/
#define SIP6_10_TTI_UDB_CONFIG_BITS_SIZE_CNS            384

/* number of bits in the Ironman - TTI default port entry */
#define SIP6_30_TTI_DEFAULT_PORT_BITS_SIZE_CNS          155
/* number of bits in the Ironman - HA ePort 1 entry */
#define SIP6_30_HA_EGRESS_EPORT_1_BITS_SIZE_CNS         120
/* number of bits in the Ironman - HA physical Port 1 entry */
#define SIP6_30_HA_EGRESS_PHYSICAL_PORT_1_BITS_SIZE_CNS 81
/* number of bits in the Ironman - HA physical Port 2 entry */
#define SIP6_30_HA_EGRESS_PHYSICAL_PORT_2_BITS_SIZE_CNS 101

/* bmp of 64 ports - 1 bit per port */
#define BMP_64_PORTS_CNS           64

/* bmp of 128 ports - 1 bit per port */
#define BMP_128_PORTS_CNS           128
/* bmp of 256 ports - 1 bit per port */
#define BMP_256_PORTS_CNS           256
/* bmp of 512 ports - 1 bit per port */
#define BMP_512_PORTS_CNS           512
/* bmp of 1024 ports - 1 bit per port */
#define BMP_1K_PORTS_CNS           _1K

/* number of bits in the Bobcat2; Caelum; Bobcat3 - HA ePort 1 entry */
#define SIP5_HA_EGRESS_EPORT_1_BITS_SIZE_CNS                   87
/* number of bits in the Hawk - HA ePort 1 entry */
#define SIP6_10_HA_EGRESS_EPORT_1_BITS_SIZE_CNS                97
#define BOBCAT_B0_HA_EGRESS_EPORT_1_BITS_SIZE_CNS               85
/* number of bits in the Bobcat2; Caelum; Bobcat3 - HA ePort 2 entry */
#define SIP5_HA_EGRESS_EPORT_2_BITS_SIZE_CNS                   24
#define BOBCAT_B0_HA_EGRESS_EPORT_2_BITS_SIZE_CNS               25
/* number of bits in the Bobcat2; Caelum; Bobcat3 - HA physical Port 1 entry */
#define SIP5_HA_EGRESS_PHYSICAL_PORT_1_BITS_SIZE_CNS           27
/* number of bits in the Hawk - HA physical Port 1 entry */
#define SIP6_10_HA_EGRESS_PHYSICAL_PORT_1_BITS_SIZE_CNS        58
/* number of bits in the Bobcat2; Caelum; Bobcat3 - HA physical Port 2 entry */
#define SIP5_HA_EGRESS_PHYSICAL_PORT_2_BITS_SIZE_CNS           77
/* number of bits in the Hawk - HA physical Port 2 entry */
#define SIP6_10_HA_EGRESS_PHYSICAL_PORT_2_BITS_SIZE_CNS        95
/* number of bits in the Bobcat2; Caelum; Bobcat3 - HA tunnel start / ARP entry */
#define SIP5_HA_TUNNEL_START_BITS_SIZE_CNS                     384
/* number of bits in the Bobcat2; Caelum; Bobcat3 - HA qos profile to exp entry */
#define SIP5_HA_QOS_PROFILE_TO_EXP_BITS_SIZE_CNS                3
/* number of bits in the Bobcat2; Caelum; Bobcat3 - HA EPCL UDB configuration entry */
#define SIP5_HA_EPCL_UDB_CONFIG_BITS_SIZE_CNS                  600
/* number of bits in the Hawk - HA EPCL UDB configuration entry */
#define SIP6_10_HA_EPCL_UDB_CONFIG_BITS_SIZE_CNS               720
/* number of bits in the Bobcat2; Caelum; Bobcat3 - HA Domain Table entry */
#define SIP5_HA_PTP_DOMAIN_BITS_SIZE_CNS                       35
/* number of bits in the Bobcat2; Caelum; Bobcat3 - HA Generic Tunnel Start Table entry */
#define SIP5_HA_GEN_TS_PROFILE_BITS_SIZE_CNS                   534

/* number of bits in the Bobcat2; Caelum; Bobcat3 - HA Global MAC SA Table entry */
#define SIP5_HA_GLOBAL_MAC_SA_BITS_SIZE_CNS                    48

/* number of bits in the Bobcat2; Caelum; Bobcat3 - HA egress vlan entry */
#define SIP5_HA_EGRESS_VLAN_BITS_SIZE_CNS                              36
/* number of bits in the Bobcat2; Caelum; Bobcat3 - HA egress vlan MAC_SA entry */
#define SIP5_HA_EGRESS_VLAN_MAC_SA_SIZE_CNS                     8
/* number of bits in the Falcon - HA egress vlan MAC_SA entry */
#define SIP6_HA_EGRESS_VLAN_MAC_SA_SIZE_CNS                     12
/* number of bits in the Bobcat2; Caelum; Bobcat3 - EQ ingress STC for physical port entry */
#define SIP5_EQ_INGRESS_STC_PHYSICAL_PORT_SIZE_CNS   80
/* number of bits in the Bobcat2; Caelum; Bobcat3 - MLL entry */
#define SIP5_MLL_ENTRY_BITS_SIZE_CNS       158
/* number of bits in the Bobcat2; Caelum; Bobcat3 - L2 MLL LTT entry */
#define SIP5_L2_MLL_LTT_ENTRY_BITS_SIZE_CNS       20
/* number of bits in the Bobcat2; Caelum; Bobcat3 - FDB unit FDB entry */
#define SIP5_FDB_FDB_BITS_SIZE_CNS     138
/* number of bits in the Falcon - EM unit Exact Match entry */
#define SIP6_EM_EXACT_MATCH_BITS_SIZE_CNS     115
/* number of bits in the Bobcat2; Caelum; Bobcat3 - PCL unit trunk hash mask CRC entry */
#define SIP5_PCL_TRUNK_HASH_MASK_CRC_BITS_SIZE_CNS   74
/* number of bits in the Hawk - PCL unit trunk hash mask CRC entry */
#define SIP6_10_PCL_TRUNK_HASH_MASK_CRC_BITS_SIZE_CNS   148/*(2*74)*/
/* number of bits in the Bobcat2; Caelum; Bobcat3 - PCL unit PCL configuration entry */
#define SIP5_PCL_PCL_CONFIG_BITS_SIZE_CNS   27
/* number of bits in the Hawk - PCL unit PCL configuration entry */
#define SIP6_10_PCL_PCL_CONFIG_BITS_SIZE_CNS   33
/* number of bits in the Bobcat2; Caelum; Bobcat3 - PCL unit PCL UDB select entry*/
#define SIP5_IPCL_UDB_SELECT_BITS_SIZE_CNS 316
/* number of bits in the Bobcat2; Caelum; Bobcat3 - TCAM action entry (for TTI,IPCL(0,1,2),EPCL) */
#define SIP5_TCAM_PCL_TTI_ACTION_BITS_SIZE_CNS 240
/* number of bits in the Bobcat2; Caelum; Bobcat3 - ERMRK Timestamp Configuration Table entry*/
#define SIP5_ERMRK_TIMESTAMP_CFG_BITS_SIZE_CNS 26
#define BOBCAT2_B0_ERMRK_TIMESTAMP_CFG_BITS_SIZE_CNS 27
/* number of bits in the Bobcat2; Caelum; Bobcat3 - ERMRK Local Action Table entry*/
#define SIP5_ERMRK_PTP_LOCAL_ACTION_BITS_SIZE_CNS 6
/* number of bits in the Bobcat2; Caelum; Bobcat3 - ERMRK Target Port Table entry*/
#define SIP5_ERMRK_PTP_TARGET_PORT_BITS_SIZE_CNS 321
/* number of bits in the Bobcat2; Caelum; Bobcat3 - ERMRK Source Port Table entry*/
#define SIP5_ERMRK_PTP_SOURCE_PORT_BITS_SIZE_CNS 321
/* number of bits in the Bobcat2; Caelum; Bobcat3 - ERMRK QoS Map Dscp Table entry*/
#define SIP5_ERMRK_QOS_DSCP_MAP_BITS_SIZE_CNS 24
/* number of bits in the Bobcat2; Caelum; Bobcat3 - ERMRK QoS Map TC_DP Table entry*/
#define SIP5_ERMRK_QOS_TC_DP_MAP_BITS_SIZE_CNS 24

/* number of bits in the Bobcat2; Caelum; Bobcat3 - EPCL unit EPCL configuration entry */
#define SIP5_EPCL_PCL_CONFIG_BITS_SIZE_CNS   27
/* number of bits in the Bobcat2; Caelum; Bobcat3 - EPCL unit UDB Select table entry */
#define SIP5_EPCL_UDB_SELECT_BITS_SIZE_CNS   306
/* number of bits in the Hawk - EPCL unit UDB Select table entry */
#define SIP6_10_EPCL_UDB_SELECT_BITS_SIZE_CNS   366

/* number of bits in the Bobcat2; Caelum; Bobcat3 - BMA port maping table entry */
#define SIP5_BMA_PORT_MAPPING_BIT_SIZE_CNS   8
/* number of bits in the Bobcat2; Caelum; Bobcat3 - BMA multicast counters table entry */
#define SIP5_BMA_MULTICAST_COUNTERS_BIT_SIZE_CNS   11

/* number of bits in the Bobcat2; Caelum; Bobcat3 - TTI PTP Command Table entry */
#define SIP5_TTI_PTP_COMMAND_BITS_SIZE_CNS   256
/* number of bits in the Bobcat2; Caelum; Bobcat3 - IPvX unit Router eVlan entry */
#define SIP5_IPVX_ROUTER_EVLAN_BITS_SIZE_CNS   66
/* number of bits in the Bobcat2; Caelum; Bobcat3 - IPvX unit Router ePort entry */
#define SIP5_IPVX_ROUTER_EPORT_BITS_SIZE_CNS   128
/* number of bits in the Bobcat2; Caelum; Bobcat3 - IPvX unit Router Nexthop entry */
#define SIP5_IPVX_ROUTER_NEXTHOP_BITS_SIZE_CNS 104
/* number of bits in the Bobcat2; Caelum; Bobcat3 - IPvX unit Router Nexthop Age Bits entry */
#define SIP5_IPVX_ROUTER_NEXTHOP_AGE_BITS_BITS_SIZE_CNS 32
/* number of bits in the Bobcat2; Caelum; Bobcat3 - IPvX unit Router Access Matrix entry */
#define SIP5_IPVX_ROUTER_ACCESS_MATRIX_BITS_SIZE_CNS 128
/* number of bits in the Bobcat2; Caelum; Bobcat3 - IPvX unit Router Qos Profile Offsets entry */
#define SIP5_IPVX_ROUTER_QOS_PROFILE_OFFSETS_BITS_SIZE_CNS 12
/* number of bits in the Bobcat2; Caelum; Bobcat3 - TCAM entry */
#define SIP5_TCAM_BITS_SIZE_CNS 84
/* number of bits in the Bobcat2; Caelum; Bobcat3 - Policer Management Counters entry */
#define SIP5_IPVX_POLICER_MANAGEMENT_COUNTERS_BITS_SIZE_CNS 74
/* number of bits in the Bobcat2; Caelum; Bobcat3 - e Attributes entry */
#define SIP5_IPVX_POLICER_E_ATTRIBUTES_BITS_SIZE_CNS 36
/* number of bits in the Falcon - e Attributes entry */
#define SIP6_IPVX_POLICER_E_ATTRIBUTES_BITS_SIZE_CNS 32
/* number of bits in the Bobcat2; Caelum; Bobcat3 - Re-Marking entry */
#define SIP5_IPVX_POLICER_REMARKING_BITS_SIZE_CNS 51
/* number of bits in the Bobcat2; Caelum; Bobcat3 - Hierarchical Policing entry */
#define SIP5_IPVX_POLICER_HIERARCHICAL_BITS_SIZE_CNS 18
/* number of bits in the Bobcat2; Caelum; Bobcat3 - Metering entry */
#define SIP5_IPVX_POLICER_METERING_BITS_SIZE_CNS 241
/* number of bits in the Bobcat2; Caelum; Bobcat3 - Counting (max) entry */
#define SIP5_IPVX_POLICER_COUNTING_BITS_SIZE_CNS 241

/* number of bits in the Bobcat2; Caelum; Bobcat3 - tail drop max queue limit table entry */
#define SIP5_TAIL_DROP_MAX_QUEUE_LIMITS_BITS_SIZE_CNS  34
/* number of bits in the Bobcat2; Caelum; Bobcat3 - tail drop eq queue limits dp0 table entry */
#define SIP5_TAIL_DROP_EQ_QUEUE_LIMITS_DP0_BITS_SIZE_CNS  48
/* number of bits in the Bobcat2; Caelum; Bobcat3 - tail drop eq queue limits dp0 table entry */
#define SIP5_TAIL_DROP_DQ_QUEUE_BUF_BITS_SIZE_CNS  34
/* number of bits in the Bobcat2; Caelum; Bobcat3 - tail drop eq queue limits dp0 table entry */
#define SIP5_TAIL_DROP_DQ_QUEUE_DESC_LIMITS_BITS_SIZE_CNS  14
/* number of bits in the Bobcat2; Caelum; Bobcat3 - tail drop eq queue limits dp0 table entry */
#define SIP5_TAIL_DROP_EQ_QUEUE_LIMITS_DP1_BITS_SIZE_CNS  48
/* number of bits in the Bobcat2; Caelum; Bobcat3 - tail drop eq queue limits dp0 table entry */
#define SIP5_TAIL_DROP_EQ_QUEUE_LIMITS_DP2_BITS_SIZE_CNS  48

/* number of bits in the Bobcat2; Caelum; Bobcat3 - CN sample interval table entry */
#define SIP5_CN_SAMPLE_INTERVALS_BITS_SIZE_CNS  36

/* number of bits in the Bobcat2; Caelum; Bobcat3 - tail drop counters - Buffers Queue Maintenance counters table entry */
#define SIP5_TAIL_DROP_COUNTERS_MAIN_BUFF_BITS_SIZE_CNS  14
/* number of bits in the Bobcat2; Caelum; Bobcat3 - tail drop counters - Multicast Buffers Queue Maintenance counters table entry */
#define SIP5_TAIL_DROP_COUNTERS_MAIN_MC_BUFF_BITS_SIZE_CNS  20

/* number of bits in the Bobcat2; Caelum; Bobcat3 - TXQ LL - Descriptors Queue Maintenance counters table entry */
#define SIP5_TXQ_LINK_LIST_COUNTERS_MAIN_DESC_BITS_SIZE_CNS  14

/* number of bits in the Bobcat2; Caelum; Bobcat3 - FC Mode Profile TC XOFF Thresholds table entry */
#define SIP5_PFC_MODE_PROFILE_TC_XOFF_THRESHOLDS_BITS_SIZE_CNS  29
/* number of bits in the Bobcat2; Caelum; Bobcat3 - FC Mode Profile TC XON Thresholds table entry */
#define SIP5_PFC_MODE_PROFILE_TC_XON_THRESHOLDS_BITS_SIZE_CNS  29
/* number of bits in the Bobcat2; Caelum; Bobcat3 - PFC/LLFC Counters table entry */
#define SIP5_PFC_LLFC_COUNTERS_BITS_SIZE_CNS  29

/* number of bits in the Bobcat2 - TM DROP Drop Masking table entry */
#define SIP5_TM_DROP_DROP_MASKING_BITS_SIZE_CNS               12
/* number of bits in the Bobcat2 - TM DROP Queue Profile Id table entry */
#define SIP5_TM_DROP_QUEUE_PROFILE_ID_BITS_SIZE_CNS           48

/* number of bits in the Bobcat2 - TM QMAP CPU Code to TC table entry */
#define SIP5_TM_QMAP_CPU_CODE_TO_TC_BITS_SIZE_CNS             4
/* number of bits in the Bobcat2 - TM QMAP Target Device To Seletor Index table entry */
#define SIP5_TM_QMAP_TARGET_DEV_TO_INDEX_BITS_SIZE_CNS        8
/* number of bits in the Bobcat2 - TM QMAP Policing Enable per Queue table entry */
#define SIP5_TM_QMAP_TM_QMAP_POLICING_ENABLE_BITS_SIZE_CNS    32
/* number of bits in the Bobcat2 - TM QMAP Queue Id Selector table entry */
#define SIP5_TM_QMAP_QUEUE_ID_SELECTOR_BITS_SIZE_CNS          126

/* number of bits in the Bobcat2 - TM Ingress Glue Packet Length Offset table entry */
#define SIP5_TM_INGR_GLUE_L1_PKT_LEN_OFFSET_SIZE_CNS          32

/* number of bits in the Bobcat2 - TM Egress Glue Aging Queue Profile table entry */
#define SIP5_TM_EGR_GLUE_AGING_QUEUE_PROFILE_SIZE_CNS         64
/* number of bits in the Bobcat2 - TM Egress Glue Aging Profile Thresholds table entry */
#define SIP5_TM_EGR_GLUE_AGING_PROFILE_THESHOLDS_SIZE_CNS     60
/* number of bits in the Bobcat2 - TM Egress Glue Packet Length Offset table entry */
#define SIP5_TM_EGR_GLUE_TARGET_INTERFACE_SIZE_CNS            32

/* number of bits in the Bobcat2 - TM FCU Ethernet DMA To Port Mapping table entry */
#define SIP5_TM_FCU_ETH_DMA_TO_TM_PORT_MAPPING_SIZE_CNS       32
/* number of bits in the Bobcat2 - TM FCU Interlaken DMA To Port Mapping table entry table entry */
#define SIP5_TM_FCU_ILK_DMA_TO_TM_PORT_MAPPING_SIZE_CNS       32
/* number of bits in the Bobcat2 - TM FCU PFC Port To CNode Port Mapping table entry */
#define SIP5_TM_FCU_TC_PORT_TO_CNODE_PORT_MAPPING_SIZE_CNS    72
/* number of bits in the Bobcat2 - TM FCU Port Ingress Timer Configuration table entry */
#define SIP5_TM_FCU_PORT_INGRESS_TIMERS_CONFIG_SIZE_CNS       32
/* number of bits in the Bobcat2 - TM FCU Ingress Timers table entry */
#define SIP5_TM_FCU_INGRESS_TIMERS_SIZE_CNS                   256
/* number of bits in the Bobcat2 - TM FCU PFC Port To Pysical Port Mapping table entry */
#define SIP5_TM_FCU_PORT_TO_PHYSICAL_PORT_MAPPING_SIZE_CNS 32


/* number of bits in the Bobcat2; Caelum; Bobcat3 - TXQ DQ - Priority Token Bucket Configuration table entry */
#define SIP5_PRIO_TOKEN_BUCKET_CONFIGURATION_BITS_SIZE_CNS  488

/* number of bits in the Bobcat2; Caelum; Bobcat3 - TXQ DQ - Port Token Bucket Configuration table entry */
#define SIP5_PORT_TOKEN_BUCKET_CONFIGURATION_BITS_SIZE_CNS  61

/* number of bits in the Bobcat2; Caelum; Bobcat3 - TXQ DQ - Egress STC Table */
#define SIP5_EGRESS_STC_TABLE_BITS_SIZE_CNS  80

/* number of table lines in the Bobcat2; Caelum; Bobcat3 - e Attributes table */
#define SIP5_IPVX_POLICER_E_ATTRIBUTES_MAX_ENTRY_CNS   5120

/* SIP5 TXQ DQ Unit ports numbers for single port group devices  */
#define SIP5_SINGLE_PORT_GROUP_TXQ_DQ_PORTS_NUM_CNS 72

/* SIP5 TXQ DQ Unit ports numbers for multi port group devices  */
#define SIP5_MULTI_PORT_GROUP_TXQ_DQ_PORTS_NUM_CNS 18

/* number of bits in the BobK - IPvX unit Router ePort entry */
#define BOBK_IPVX_ROUTER_EPORT_BITS_SIZE_CNS   112

/* number of bits in the Bobk - Metering configuration entry */
#define BOBK_POLICER_METERING_CONFIG_BITS_SIZE_CNS 51
/* number of bits in the Bobk - Metering token bucket entry */
#define BOBK_POLICER_METERING_TOKEN_BUCKET_BITS_SIZE_CNS 211
/* number of bits in the Bobk - Metering Conformance Sign entry */
#define BOBK_POLICER_METERING_CONFORM_SIGN_BITS_SIZE_CNS 2

/* number of bits in bobcat3 - "TTI Port Mapping Table" */
#define SIP5_20_TTI_PORT_MAPPING_BITS_SIZE_CNS 7

/* number of bits in the sip5_20 - EGF_QAG egress eport entry */
#define SIP5_20_EGF_QAG_PORT_TARGET_BITS_SIZE_CNS    17
/* number of bits in the sip5_20 - EGF_QAG egress eport entry */
#define SIP5_20_EGF_QAG_EGRESS_EVLAN_BITS_SIZE_CNS   (16*3)


/* number of bits in the sip5_20 - TTI physical port entry 2 */
#define SIP5_20_TTI_PHYSICAL_PORT_2_BITS_SIZE_CNS       251
/* number of bits in the sip5_20 - TTI PCL UDB entry */
#define SIP5_20_TTI_PCL_UDB_BITS_SIZE_CNS               770

/* number of bits in the sip5_20 - PCL unit PCL UDB select entry*/
#define SIP5_20_IPCL_UDB_SELECT_BITS_SIZE_CNS       506

/* number of bits in the sip6_10 - PCL unit PCL UDB select entry*/
#define SIP6_10_IPCL_UDB_SELECT_BITS_SIZE_CNS       513

/* number of bits in the sip6_30 - PCL unit PCL UDB select entry */
/* table was split to addition table of : IPCL_UDB_REPLACEMENT   */
#define SIP6_30_IPCL_UDB_SELECT_BITS_SIZE_CNS       490 /* 490 = 70UDBs * 7bits */

/* number of bits in the sip6_30 - (new tables) PCL unit PCL UDB replacement entry */
#define SIP6_30_IPCL_UDB_REPLACEMENT_BITS_SIZE_CNS  25

/* number of bits in the sip6_30 - (new tables) SMU SNG IRF entry */
#define SIP6_30_SMU_SNG_IRF_BITS_SIZE_CNS   36

/* number of bits in the sip6_30 - (new tables) SMU IRF counters entry */
#define SIP6_30_SMU_IRF_COUNTERS_BITS_SIZE_CNS   146

/* number of bits in the sip6_30 - (new tables) PREQ SRF mapping entry */
#define SIP6_30_PREQ_SRF_MAPPING_BITS_SIZE_CNS   76

/* number of bits in the sip6_30 - (new tables) PREQ SRF config entry */
#define SIP6_30_PREQ_SRF_CONFIG_BITS_SIZE_CNS   91
/* number of bits in the sip6_30 - (new tables) PREQ SRF counting entry */
#define SIP6_30_PREQ_SRF_COUNTERS_BITS_SIZE_CNS 160
/* number of bits in the sip6_30 - (new tables) PREQ SRF daemon entry */
#define SIP6_30_PREQ_SRF_DAEMON_BITS_SIZE_CNS   79
/* number of bits in the sip6_30 - (new tables) PREQ SRF history buffer entry */
#define SIP6_30_PREQ_SRF_HISTORY_BUFFER_BITS_SIZE_CNS   128
/* number of bits in the sip6_30 - (new tables) PREQ SRF bit vector entry */
#define SIP6_30_PREQ_SRF_ZERO_BIT_VECTOR_BITS_SIZE_CNS  16


/* number of bits in the sip5_20 - IPvX unit Router Nexthop entry */
#define SIP5_20_IPVX_ROUTER_NEXTHOP_BITS_SIZE_CNS   106

/* number of bits in the sip5_20 - MLL entry */
#define SIP5_20_MLL_ENTRY_BITS_SIZE_CNS   162

/* number of bits in the sip6_10 - PFCC calendar entry size*/
#define SIP6_10_PFCC_CFG__ENTRY_BITS_SIZE_CNS   18

/* number of bits in the sip6_10 - PFCC calendar  size*/
#define SIP6_10_PFCC_CFG__TABLE_SIZE_CNS   148


/* the number of flows that the PCL classifies */
#define PCL_NUM_FLOWS_CNS   12
/* Bobcat2; Caelum; Bobcat3 the number of flows that the PCL classifies */
#define SIP5_PCL_NUM_FLOWS_CNS   16

/* macro to get the alignment as number of words from the number of bits in the entry */
#define BITS_TO_BYTES_ALIGNMENT_MAC(bits) \
    (((bits) > _1K) ? 256 :      \
     ((bits) > 512) ? 128 :      \
     ((bits) > 256) ?  64 :      \
     ((bits) > 128) ?  32 :      \
     ((bits) >  64) ?  16 :      \
     ((bits) >  32) ?   8 :   4)

#define TABLE_MODE_TO_DIVISION_NUMBER_MAC(_devNum)                                              \
    (!PRV_CPSS_SIP_6_CHECK_MAC(_devNum)) ?  PRV_CPSS_DXCH_TABLES_SIZE_MODE_GET_MAC(_devNum) :  \
    (PRV_CPSS_DXCH_TABLES_SIZE_MODE_GET_MAC(_devNum) == 0) ? /*64*/  1 :                        \
    (PRV_CPSS_DXCH_TABLES_SIZE_MODE_GET_MAC(_devNum) == 1) ? /*128*/ 1 :                        \
    (PRV_CPSS_DXCH_TABLES_SIZE_MODE_GET_MAC(_devNum) == 2) ? /*256*/ 2 :                        \
    (PRV_CPSS_DXCH_TABLES_SIZE_MODE_GET_MAC(_devNum) == 3) ? /*512*/ 4 : /*1024*/ 8

/*calc number of words form number of bits */
#define NUM_WORDS_FROM_BITS_MAC(x)   (((x) + 31)>>5)

/* access to info of the DxCh device tables */
#define PRV_CPSS_DXCH_DEV_TBLS_MAC(devNum)  \
    (PRV_CPSS_DXCH_PP_MAC(devNum)->accessTableInfoPtr)

/* for indirect information table */
typedef enum {
    PRV_CPSS_DXCH2_TABLE_STP_E       = CPSS_DXCH_TABLE_POLICER_COUNTERS_E + 1,
    PRV_CPSS_DXCH2_TABLE_VLAN_E,
    PRV_CPSS_DXCH2_TABLE_MULTICAST_E
}INDIRECT_INDEX_ENT;

/* for SIP_5, SIP_6 indirect information tables */
typedef enum {
    PRV_CPSS_SIP5_FDB_TABLE_INDIRECT_E = 0,
    PRV_CPSS_SIP5_IOAM_TABLE_INDIRECT_E,
    PRV_CPSS_SIP5_EOAM_TABLE_INDIRECT_E,
    PRV_CPSS_SIP6_EXACT_MATCH_TABLE_INDIRECT_E
}INDIRECT_INDEX_SIP5_ENT;


/* This value is used when control registor relates only to one table.
   In this case there is no need to update table index in control register.
   The action that is done for all tables is: table index << table index offset
   0 << 0 doesn't have any impact */
#define PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS           0

/* number of words in the cheetah QoS profile entry */
#define PRV_CPSS_DXCH_QOS_PROFILE_ENTRY_WORDS_COUNT_CNS       1


/* number of entries in cheetah vlan table */
#define PRV_CPSS_DXCH_MAX_VLANS_NUM_CNS                       4096
/* number of words in the cheetah vlan entry */
#define PRV_CPSS_DXCH2_VLAN_ENTRY_WORDS_COUNT_CNS             4
#define PRV_CPSS_DXCHXCAT_VLAN_ENTRY_WORDS_COUNT_CNS          6

/* number of words in the cheetah multicast entry */
#define PRV_CPSS_DXCH_MULTICAST_ENTRY_WORDS_COUNT_CNS         1

/* number of words in the cheetah mac sa entry */
#define PRV_CPSS_DXCH_ROUTE_HA_MAC_SA_ENTRY_WORDS_COUNT_CNS   1


#define PRV_CPSS_DXCH_PORT_TBLS_MAX_INDEX_CNS 64

#define PRV_CPSS_DXCH_FDB_MAC_ENTRY_WORDS_SIZE_CNS       4

/* 32K */
#define PRV_CPSS_DXCH3_FDB_MAX_ENTRIES_CNS                0x8000

#define PRV_CPSS_DXCHXCAT_PRTCL_BASED_VLAN_ENTRY_WORDS_COUNT_CNS   1

/* 128 trunks * 8 members in trunk = 1024 */
#define PRV_CPSS_DXCH_TRUNK_MEMBERS_TABLE_MAX_ENTRIES_CNS     1024

/* number of words in the cheetah trunk entry */
#define PRV_CPSS_DXCH_TRUNK_ENTRY_WORDS_COUNT_CNS             1

/* number of entries in cheetah rate limit table */
#define PRV_CPSS_DXCH_RATE_LIMIT_TABLE_MAX_ENTRIES_CNS        32

/* number of words in the cheetah rate limit entry */
#define PRV_CPSS_DXCH_RATE_LIMIT_ENTRY_WORDS_COUNT_CNS        1

/* number of entries in cheetah arp da table */
#define PRV_CPSS_DXCH_ROUTE_HA_ARP_DA_TABLE_MAX_ENTRIES_CNS   1024

/* number of words in the cheetah arp da entry */
#define PRV_CPSS_DXCH2_ROUTE_HA_ARP_DA_ENTRY_WORDS_COUNT_CNS  6

/* number of words in the cheetah2 QoSProfile-to-Route-Block-Offset entry */
#define PRV_CPSS_DXCH2_QOS_TO_ROUTE_BLOCK_WORDS_COUNT_CNS   1
/* number of entries in the cheetah2 QoSProfile-to-Route-Block-Offset table */
#define PRV_CPSS_DXCH2_QOS_TO_ROUTE_BLOCK_TABLE_MAX_ENTRIES_CNS    16

/* number of words in the cheetah2 Router Access Matrix Line entry */
#define PRV_CPSS_DXCH2_ROUTE_ACCESS_MATRIX_WORDS_COUNT_CNS   1
/* number of entries in the cheetah2 Router Access Matrix Line table */
#define PRV_CPSS_DXCH2_ROUTE_ACCESS_MATRIX_TABLE_MAX_ENTRIES_CNS    8

/* number of words in LTT and TT Action Table entry */
#define PRV_CPSS_DXCH2_LTT_TT_ACTION_WORDS_COUNT_CNS   4
/* number of entries in the cheetah2 LTT and TT Action table */
#define PRV_CPSS_DXCH2_LTT_TT_ACTION_TABLE_MAX_ENTRIES_CNS    1024

/* number of words in Unicast/Multicast Router Next Hop  entry */
#define PRV_CPSS_DXCH2_UC_MC_ROUTE_NEXT_HOP_WORDS_COUNT_CNS   4
/* number of entries in the cheetah2 Unicast/Multicast Router Next Hop table */
#define PRV_CPSS_DXCH2_UC_MC_ROUTE_NEXT_HOP_TABLE_MAX_ENTRIES_CNS    4096

/* number of words in yhe cheetah2 Router Next Hop Table Age Bits entry */
#define PRV_CPSS_DXCH2_ROUTE_NEXT_HOP_AGE_COUNT_CNS   1
/* number of entries in the cheetah2 Router Next Hop Table Age Bits Entry table */
#define PRV_CPSS_DXCH2_ROUTE_NEXT_HOP_AGE_TABLE_MAX_ENTRIES_CNS    128
/* number of words in the cheetah3 mac2me table */
#define PRV_CPSS_DXCH3_MAC2ME_WORDS_COUNT_CNS   4
/* number of entries in the cheetah3 mac2me table */
#define PRV_CPSS_DXCH3_MAC2ME_TABLE_MAX_ENTRIES_CNS    8
/* number of words in the chhetah3 vlan translation table (ingress/egress) */
#define PRV_CPSS_DXCH3_VLAN_TRANSLATION_WORDS_COUNT_CNS     1
/* number of entries in the cheetah3 vlan translation tables (ingress/egress) */
#define PRV_CPSS_DXCH3_VLAN_TRANSLATION_TABLE_MAX_ENTRIES_CNS       4096
/* number of words in the chhetah3 VRF ID table */
#define PRV_CPSS_DXCH3_VRF_ID_WORDS_COUNT_CNS     1
/* number of entries in the cheetah3 VRF ID table */
#define PRV_CPSS_DXCH3_VRF_ID_TABLE_MAX_ENTRIES_CNS       4096
/* number of words in LTT and TT Action Table entry (Ch3) */
#define PRV_CPSS_DXCH3_LTT_TT_ACTION_WORDS_COUNT_CNS    4
/* number of entries in the cheetah3 LTT and TT Action table */
#define PRV_CPSS_DXCH3_LTT_TT_ACTION_TABLE_MAX_ENTRIES_CNS    1024
/* number of words in LTT and TT Action Table entry (xCat) */
#define PRV_CPSS_XCAT_LTT_TT_ACTION_WORDS_COUNT_CNS    5

/* number of entries in cheetah cpu code table */
#define PRV_CPSS_DXCH_CPU_CODE_TABLE_MAX_ENTRIES_CNS          256

/* number of words in the cheetah cpu code entry */
#define PRV_CPSS_DXCH_CPU_CODE_ENTRY_WORDS_COUNT_CNS          1

/* STG entry size -- in words */
#define PRV_CPSS_DXCH_STG_ENTRY_WORDS_SIZE_CNS           2

/* cheetah default value for next word offset for table direct access */
#define PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS           4

typedef enum {
    SPECIAL_ADDRESS_NOT_MODIFIED_E = 0x0EEEEEEE,/* unchanged address indication */
    SPECIAL_ADDRESS_NOT_EXISTS_E   = 0x0FFFFFFF /* non exists address */
}SPECIAL_ADDRESS_ENT;

#define TABLES_INFO_DIRECT_NOT_EXISTS_CNS   \
    SPECIAL_ADDRESS_NOT_EXISTS_E, 0 , 0

#define TABLES_INFO_DIRECT_NOT_MODIFIED_CNS   \
    SPECIAL_ADDRESS_NOT_MODIFIED_E, 0 , 0

#define PRV_CPSS_DXCH_UNIT_IN_TILE_0_BASE_E 0



#define PRV_DECLARE_CENTRAL_DP_TABLE(_name,_addr,_unitName,_entrySize) \
{PRV_DXCH_SIP6_TABLE_TXQ_##_name##_E,\
{CPSS_DXCH_SIP6_TXQ_##_name##_E,PRV_CPSS_DXCH_UNIT_TXQ_##_unitName##_E,\
      {_addr,BITS_TO_BYTES_ALIGNMENT_MAC(_entrySize), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS,},GT_FALSE}}\
,{PRV_DXCH_SIP6_TABLE_TXQ_##_name##_E+1,\
{CPSS_DXCH_SIP6_TXQ_##_name##_E+1,PRV_CPSS_DXCH_UNIT_TXQ_##_unitName##_E+PRV_CPSS_DXCH_UNIT_IN_TILE_1_BASE_E,\
      {_addr,BITS_TO_BYTES_ALIGNMENT_MAC(_entrySize), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS,},GT_FALSE}}\
,{PRV_DXCH_SIP6_TABLE_TXQ_##_name##_E+2,\
{CPSS_DXCH_SIP6_TXQ_##_name##_E+2,PRV_CPSS_DXCH_UNIT_TXQ_##_unitName##_E+PRV_CPSS_DXCH_UNIT_IN_TILE_2_BASE_E,\
      {_addr,BITS_TO_BYTES_ALIGNMENT_MAC(_entrySize), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS,},GT_FALSE}}\
,{PRV_DXCH_SIP6_TABLE_TXQ_##_name##_E+3,\
{CPSS_DXCH_SIP6_TXQ_##_name##_E+3,PRV_CPSS_DXCH_UNIT_TXQ_##_unitName##_E+PRV_CPSS_DXCH_UNIT_IN_TILE_3_BASE_E,\
      {_addr,BITS_TO_BYTES_ALIGNMENT_MAC(_entrySize), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS,},GT_FALSE}}


#define PRV_DECLARE_SINGLE_PIPE_DP_TABLE(_name,_addr,_unitName,_pipeInd,_entrySize,_tile) \
{PRV_DXCH_SIP6_TABLE_TXQ_##_name##_E+_pipeInd*4+_tile*8,\
{CPSS_DXCH_SIP6_TXQ_##_name##_E+_pipeInd*4+_tile*8,PRV_CPSS_DXCH_UNIT_TXQ_PIPE##_pipeInd##_##_unitName##0_E + PRV_CPSS_DXCH_UNIT_IN_TILE_##_tile##_BASE_E,\
      {_addr,BITS_TO_BYTES_ALIGNMENT_MAC(_entrySize), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS,},GT_FALSE}}\
,{PRV_DXCH_SIP6_TABLE_TXQ_##_name##_E+1+_pipeInd*4+_tile*8,\
{CPSS_DXCH_SIP6_TXQ_##_name##_E+1+_pipeInd*4+_tile*8,PRV_CPSS_DXCH_UNIT_TXQ_PIPE##_pipeInd##_##_unitName##1_E + PRV_CPSS_DXCH_UNIT_IN_TILE_##_tile##_BASE_E,\
      {_addr,BITS_TO_BYTES_ALIGNMENT_MAC(_entrySize), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS,},GT_FALSE}}\
,{PRV_DXCH_SIP6_TABLE_TXQ_##_name##_E+2+_pipeInd*4+_tile*8,\
{CPSS_DXCH_SIP6_TXQ_##_name##_E+2+_pipeInd*4+_tile*8,PRV_CPSS_DXCH_UNIT_TXQ_PIPE##_pipeInd##_##_unitName##2_E + PRV_CPSS_DXCH_UNIT_IN_TILE_##_tile##_BASE_E,\
      {_addr,BITS_TO_BYTES_ALIGNMENT_MAC(_entrySize), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS,},GT_FALSE}}\
,{PRV_DXCH_SIP6_TABLE_TXQ_##_name##_E+3+_pipeInd*4+_tile*8,\
{CPSS_DXCH_SIP6_TXQ_##_name##_E+3+_pipeInd*4+_tile*8,PRV_CPSS_DXCH_UNIT_TXQ_PIPE##_pipeInd##_##_unitName##3_E + PRV_CPSS_DXCH_UNIT_IN_TILE_##_tile##_BASE_E,\
      {_addr,BITS_TO_BYTES_ALIGNMENT_MAC(_entrySize), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS,},GT_FALSE}}


#define PRV_DECLARE_DUAL_PIPE_DP_TABLE(_name,_addr,_unitName,_entrySize,_tile) \
    PRV_DECLARE_SINGLE_PIPE_DP_TABLE(_name,_addr,_unitName,0,_entrySize,_tile)\
   ,PRV_DECLARE_SINGLE_PIPE_DP_TABLE(_name,_addr,_unitName,1,_entrySize,_tile)


#define PRV_DECLARE_MULTI_TILE_DP_TABLE(_name,_addr,_unitName,_entrySize)\
     PRV_DECLARE_DUAL_PIPE_DP_TABLE(_name,_addr,_unitName,_entrySize,0)\
    ,PRV_DECLARE_DUAL_PIPE_DP_TABLE(_name,_addr,_unitName,_entrySize,1)\
    ,PRV_DECLARE_DUAL_PIPE_DP_TABLE(_name,_addr,_unitName,_entrySize,2)\
    ,PRV_DECLARE_DUAL_PIPE_DP_TABLE(_name,_addr,_unitName,_entrySize,3)

#define PRV_DECLARE_SINGLE_INSTANCE_DP_TABLE(_name,_addr,_unitName,_entrySize) \
{PRV_DXCH_SIP6_TABLE_TXQ_##_name##_E,\
{CPSS_DXCH_SIP6_TXQ_##_name##_E,PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_##_unitName##0_E,\
      {_addr,BITS_TO_BYTES_ALIGNMENT_MAC(_entrySize), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS,},GT_FALSE}}

#define PRV_DECLARE_SINGLE_INSTANCE_CENTRAL_DP_TABLE(_name,_addr,_unitName,_entrySize) \
{PRV_DXCH_SIP6_TABLE_TXQ_##_name##_E,\
{CPSS_DXCH_SIP6_TXQ_##_name##_E,PRV_CPSS_DXCH_UNIT_TXQ_##_unitName##_E,\
      {_addr,BITS_TO_BYTES_ALIGNMENT_MAC(_entrySize), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS,},GT_FALSE}}



#define PRV_SET_SIZE_SINGLE_PIPE_DP_TABLE_WITH_DIVIDER(_name,_tableType,_divider,_size,_pipe,_tile) \
 {CPSS_DXCH_SIP6_TXQ_##_name##_E + _pipe*4 + _tile*8,\
  _tableType,_divider,_size}\
,{CPSS_DXCH_SIP6_TXQ_##_name##_E+1+ _pipe*4 + _tile*8,\
  _tableType,_divider,_size}\
,{CPSS_DXCH_SIP6_TXQ_##_name##_E+2+ _pipe*4 + _tile*8,\
  _tableType,_divider,_size}\
,{CPSS_DXCH_SIP6_TXQ_##_name##_E+3+ _pipe*4 + _tile*8,\
  _tableType,_divider,_size}

#define PRV_SET_SIZE_SINGLE_PIPE_DP_TABLE(_name,_tableType,_size,_pipe,_tile) \
    PRV_SET_SIZE_SINGLE_PIPE_DP_TABLE_WITH_DIVIDER(_name,_tableType,0,_size,_pipe,_tile)

#define PRV_SET_SIZE_DUAL_PIPE_DP_TABLE(_name,_tableType,_size,_tile)\
    PRV_SET_SIZE_SINGLE_PIPE_DP_TABLE(_name,_tableType,_size,0,_tile)\
   ,PRV_SET_SIZE_SINGLE_PIPE_DP_TABLE(_name,_tableType,_size,1,_tile)

#define PRV_SET_SIZE_MULTI_TILE_DP_TABLE(_name,_tableType,_size)\
    PRV_SET_SIZE_DUAL_PIPE_DP_TABLE(_name,_tableType,_size,0)\
   ,PRV_SET_SIZE_DUAL_PIPE_DP_TABLE(_name,_tableType,_size,1)\
   ,PRV_SET_SIZE_DUAL_PIPE_DP_TABLE(_name,_tableType,_size,2)\
   ,PRV_SET_SIZE_DUAL_PIPE_DP_TABLE(_name,_tableType,_size,3)

#define PRV_SET_SIZE_CENTRAL_DP_TABLE(_name,_tableType,_size) \
 PRV_SET_SIZE_SINGLE_PIPE_DP_TABLE(_name,_tableType,_size,0,0)

static const PRV_CPSS_DXCH_TABLES_INFO_INDIRECT_STC indirectAccessTableInfo[] =
{
    /* CPSS_DXCH_TABLE_VLAN_PORT_PROTOCOL_E */
    {0x0B800328,  0x0B800300,   0,  2,  2,  10,  1 },

    /* CPSS_DXCH_TABLE_PORT_VLAN_QOS_E */
    {0x0B800328,  0x0B800320,   0,  2,  1,  10,  1},

    /* CPSS_DXCH_TABLE_TRUNK_MEMBERS_E */
    {0x0B000028,  0x0B001000,   0,  2, PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS,
          PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS, 1},

    /* CPSS_DXCH_TABLE_STATISTICAL_RATE_LIMIT_E */
    {0x0B000034,  0x0B004000,   0,  2, PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS,
          PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS, 1},

    /* CPSS_DXCH_TABLE_CPU_CODE_E */
    {0x0B000030,  0x0B003000,   0,  2, PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS,
          PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS, 1},

    /* CPSS_DXCH_TABLE_PCL_CONFIG_E */
    {0x0B800204,  0x0B800200,   0,  2, PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS,
          PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS, 1},

    /* CPSS_DXCH_TABLE_QOS_PROFILE_E */
    {0x0B00002C,  0x0B002000,   0,  2, PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS,
          PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS, 1},

    /* CPSS_DXCH_TABLE_REMARKING_E */
    {0x0C000024,  0x0C000020,   0 , 2, PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS,
          PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS, 1},

    /* CPSS_DXCH_TABLE_STG_E */
    {0x0A00000C,  0x0A000004,   15, 0,  2,  13,  12},

    /* CPSS_DXCH_TABLE_VLAN_E */
    {0x0A00000C,  0x0A000000,   15, 0,  0,  13,  12},

    /* CPSS_DXCH_TABLE_MULTICAST_E */
    {0x0A00000C,  0x0A000008,   15, 0,  1,  13,  12},

    /* CPSS_DXCH_TABLE_ROUTE_HA_MAC_SA_E */
    {0x07800208,  0x07800204,   0,  3,  1,  2,   1},

    /* CPSS_DXCH_TABLE_ROUTE_HA_ARP_DA_E */
    {0x07800208,  0x07800200,   0,  3,  0,  2,   1},

    /* CPSS_DXCH_TABLE_FDB_E */
    {0x06000064,  0x06000054,   0,  2, PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS,
          PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS, 1},

    /* CPSS_DXCH_TABLE_POLICER_E */
    {0x0C000014,  0x0C00000C,   0,  2, PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS,
          PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS, 1},

    /* CPSS_DXCH_TABLE_POLICER_COUNTERS_E */
    {0x0C000038,  0x0C000030,   0,  2, PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS,
          PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS, 1},

    /* PRV_CPSS_DXCH2_TABLE_STP_E */
    {0x0A000010,  0x0A000008,   15, 0,  2,  13,  12},

    /* PRV_CPSS_DXCH2_TABLE_VLAN_E */
    {0x0A000010,  0x0A000000,   15, 0,  0,  13,  12},

    /* PRV_CPSS_DXCH2_TABLE_MULTICAST_E */
    {0x0A000010,  0x0A00000C,   15, 0,  1,  13,  12}
};

/* xCat3 indirect table information */
static const PRV_CPSS_DXCH_TABLES_INFO_INDIRECT_STC indirectAccessXcat3TableInfo[] =
{
    /* PRV_DXCH_XCAT3_TABLE_TRUNK_MEMBERS_E */
    {0x14000028,  0x14001000,   0,  2,  0,  0,  1},

    /* PRV_DXCH_XCAT3_TABLE_STATISTICAL_RATE_LIMIT_E */
    {0x14000034,  0x14004000,   0,  2,  0,  0,  1},

    /* PRV_DXCH_XCAT3_TABLE_CPU_CODE_E */
    {0x14000030,  0x14003000,   0,  2,  0,  0,  1},

    /* PRV_DXCH_XCAT3_TABLE_QOS_PROFILE_E */
    {0x1400002C,  0x14002000,   0,  2,  0,  0,  1},

    /* PRV_DXCH_XCAT3_TABLE_FDB_E */
    {0x0B000064,  0x0B000054,   0,  2,  0,  0,  1},

    /* PRV_DXCH_XCAT3_TABLE_LOGICAL_TARGET_MAPPING_E */
    {0x140000A0,  0x1400A000,   0,  2,  0,  0,  1},

    /* PRV_DXCH_XCAT3_TABLE_VLAN_MAPPING_E */
    {0x140000A4,  0x140000A8,   0,  2,  0,  0,  1},

    /* PRV_DXCH_XCAT3_TABLE_LP_EGRESS_VLAN_MEMBER_E */
    {0x140000AC,  0x140000B0,   0,  2,  0,  0,  1},

    /* PRV_DXCH_XCAT3_TABLE_L2MLL_VIDX_ENABLE_E */
    {0x19000464,  0x19000460,   0,  2,  0,  0,  1},

    /* PRV_DXCH_XCAT3_TABLE_L2MLL_POINTER_MAPPING_E */
    {0x19000474,  0x19000470,   0,  2,  0,  0,  1}
};

/* indirect table info for Bobcat2, Caelum, Aldrin, AC3X devices */
static const PRV_CPSS_DXCH_TABLES_INFO_INDIRECT_STC indirectAccessTableInfo_sip_5[] =
{
    /* PRV_CPSS_SIP5_FDB_TABLE_INDIRECT_E  */
    {0x04000130,  0x04000134,   0,  2, PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS,
          PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS, 1,},

    /* PRV_CPSS_SIP5_IOAM_TABLE_INDIRECT_E */
    {0x1C0000D0, 0x1C0000D4,    0, 16, PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS,
          PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS, 1},

    /* PRV_CPSS_SIP5_EOAM_TABLE_INDIRECT_E */
    {0x1E0000D0, 0x1E0000D4,    0, 16, PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS,
          PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS, 1}
};

/* indirect table info for Bobcat3, Aldrin2 */
static const PRV_CPSS_DXCH_TABLES_INFO_INDIRECT_STC indirectAccessTableInfo_sip_5_20[] =
{
    /* PRV_CPSS_SIP5_FDB_TABLE_INDIRECT_E  */
    {0x43000130,  0x43000134,   0,  2, PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS,
          PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS, 1},

    /* PRV_CPSS_SIP5_IOAM_TABLE_INDIRECT_E */
    {0x070000D0, 0x070000D4,    0, 16, PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS,
          PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS, 1},

    /* PRV_CPSS_SIP5_EOAM_TABLE_INDIRECT_E */
    {0x180000D0, 0x180000D4,    0, 16, PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS,
          PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS, 1}
};

/* indirect table info for Falcon */
static const PRV_CPSS_DXCH_TABLES_INFO_INDIRECT_STC indirectAccessTableInfo_falcon[] =
{
    /* PRV_CPSS_SIP5_FDB_TABLE_INDIRECT_E  */
    {0x188B0130, 0x188B0134,   0,  2, PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS,
          PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS, 1},

    /* PRV_CPSS_SIP5_IOAM_TABLE_INDIRECT_E */
    {0x0AB000D0, 0x0AB000D4,    0, 16, PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS,
          PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS, 1},

    /* PRV_CPSS_SIP5_EOAM_TABLE_INDIRECT_E */
    {0x0A2000D0, 0x0A2000D4,    0, 16, PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS,
          PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS, 1},

   /* PRV_CPSS_SIP6_EXACT_MATCH_TABLE_INDIRECT_E */
    {0x188A0130, 0x188A0134,   0,  2, PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS,
          PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS, 1}
};

/* indirect table info for AC5P (Hawk) */
static const PRV_CPSS_DXCH_TABLES_INFO_INDIRECT_STC indirectAccessTableInfo_ac5p[] =
{
    /* PRV_CPSS_SIP5_FDB_TABLE_INDIRECT_E  */
    {0x12800130, 0x12800134,   0,  2, PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS,
          PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS, 1},

    /* PRV_CPSS_SIP5_IOAM_TABLE_INDIRECT_E */
    {0x170000D0, 0x170000D4,    0, 16, PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS,
          PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS, 1},

    /* PRV_CPSS_SIP5_EOAM_TABLE_INDIRECT_E */
    {0x0F6000D0, 0x0F6000D4,    0, 16, PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS,
          PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS, 1},

   /* PRV_CPSS_SIP6_EXACT_MATCH_TABLE_INDIRECT_E */
    {0x12402000, 0x12402004,   0,  2, PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS,
          PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS, 1}
};

/* indirect table info for AC5X (Phoenix) */
static const PRV_CPSS_DXCH_TABLES_INFO_INDIRECT_STC indirectAccessTableInfo_ac5x[] =
{
    /* PRV_CPSS_SIP5_FDB_TABLE_INDIRECT_E  */
    {0x8D800130, 0x8D800134,   0,  2, PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS,
          PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS, 1},

    /* PRV_CPSS_SIP5_IOAM_TABLE_INDIRECT_E */
    {0x888000D0, 0x888000D4,    0, 16, PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS,
          PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS, 1},

    /* PRV_CPSS_SIP5_EOAM_TABLE_INDIRECT_E */
    {0x8F8000D0, 0x8F8000D4,    0, 16, PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS,
          PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS, 1},

   /* PRV_CPSS_SIP6_EXACT_MATCH_TABLE_INDIRECT_E */
    {0x8D402000, 0x8D402004,   0,  2, PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS,
          PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS, 1}
};

/* indirect table info for Harrier  */
static const PRV_CPSS_DXCH_TABLES_INFO_INDIRECT_STC indirectAccessTableInfo_harrier[] =
{
    /* PRV_CPSS_SIP5_FDB_TABLE_INDIRECT_E  */
    {0x04000130, 0x04000134,   0,  2, PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS,
          PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS, 1},

    /* PRV_CPSS_SIP5_IOAM_TABLE_INDIRECT_E */
    {0x058000D0, 0x058000D4,    0, 16, PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS,
          PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS, 1},

    /* PRV_CPSS_SIP5_EOAM_TABLE_INDIRECT_E */
    {0x0A0000D0, 0x0A0000D4,    0, 16, PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS,
          PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS, 1},

   /* PRV_CPSS_SIP6_EXACT_MATCH_TABLE_INDIRECT_E */
    {0x05002000, 0x05002004,   0,  2, PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS,
          PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS, 1}
};

/* indirect table info for IronMan  */
static const PRV_CPSS_DXCH_TABLES_INFO_INDIRECT_STC indirectAccessTableInfo_ironMan[] =
{
    /* PRV_CPSS_SIP5_FDB_TABLE_INDIRECT_E  */
    {PRV_CPSS_IRONMAN_FDB_BASE_ADDRESS_CNS + 0x00000130,
     PRV_CPSS_IRONMAN_FDB_BASE_ADDRESS_CNS + 0x00000134,
     0,  2, PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS,
          PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS, 1},

    /* PRV_CPSS_SIP5_IOAM_TABLE_INDIRECT_E */
    {PRV_CPSS_IRONMAN_IOAM_BASE_ADDRESS_CNS + 0x000000D0,
     PRV_CPSS_IRONMAN_IOAM_BASE_ADDRESS_CNS + 0x000000D4,
     0, 16, PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS,
          PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS, 1}
};


/* Lion2 direct table information */
static const PRV_CPSS_DXCH_TABLES_INFO_DIRECT_STC directAccessTableInfo[] =
{
    /* CPSS_DXCH_TABLE_VLAN_PORT_PROTOCOL_E */
    {0x0B810800, 0x20, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /* CPSS_DXCH_TABLE_PORT_VLAN_QOS_E */
    {0x0B810000, 0x20, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /* CPSS_DXCH_TABLE_TRUNK_MEMBERS_E */
    {0x0B400000, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /* CPSS_DXCH_TABLE_STATISTICAL_RATE_LIMIT_E */
    {0x0B100000, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /* CPSS_DXCH_TABLE_CPU_CODE_E */
    {0x0B200000, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /* CPSS_DXCH_TABLE_PCL_CONFIG_E */
    {0x0B840000, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /* CPSS_DXCH_TABLE_QOS_PROFILE_E */
    {0x0B300000, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /* used for CH3 only, CH2 uses indirect access, the base address is for CH3 only */
    /* CPSS_DXCH_TABLE_REMARKING_E */
    {0x0c080000, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /* CPSS_DXCH_TABLE_STG_E */
    {0x0A100000, 0x10, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /* CPSS_DXCH_TABLE_VLAN_E */
    {0x0A400000, 0x10, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /* CPSS_DXCH_TABLE_MULTICAST_E */
    {0x0A200000, 0x10, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /* CPSS_DXCH_TABLE_ROUTE_HA_MAC_SA_E */
    {0x07F80000, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /* CPSS_DXCH_TABLE_ROUTE_HA_ARP_DA_E */
    {0x07E80000, 0x20, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /* CPSS_DXCH_TABLE_FDB_E */
    {0x06400000, 0x10, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /* CPSS_DXCH_TABLE_POLICER_E */
    {0x0C100000, 0x10, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /* CPSS_DXCH_TABLE_POLICER_COUNTERS_E */
    {0x0C300000, 0x10, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /* CPSS_DXCH2_TABLE_EGRESS_PCL_CONFIG_E */
    {0x07700000, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /* CPSS_DXCH2_TABLE_TUNNEL_START_CONFIG_E */
    {0x07E80000, 0x20, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /* CPSS_DXCH2_TABLE_QOS_PROFILE_TO_ROUTE_BLOCK_E */
    {0x02800300, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /* CPSS_DXCH2_TABLE_ROUTE_ACCESS_MATRIX_E */
    {0x02800440, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /* CPSS_DXCH2_LTT_TT_ACTION_E */
    {0x02900000, 0x10, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /* CPSS_DXCH2_UC_MC_ROUTE_NEXT_HOP_E */
    {0x02B00000, 0x10, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /* CPSS_DXCH2_ROUTE_NEXT_HOP_AGE_E */
    {0x02801000, 0x10, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /* CPSS_DXCH3_TABLE_MAC2ME_E */
    {0xB800700, 0x10, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /* CPSS_DXCH3_TABLE_INGRESS_VLAN_TRANSLATION_E */
    {0x0B804000, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /* CPSS_DXCH3_TABLE_EGRESS_VLAN_TRANSLATION_E */
    {0x07FC0000, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /* CPSS_DXCH3_TABLE_VRF_ID_E */
    {0x0A300000, 0x10, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /* CPSS_DXCH3_LTT_TT_ACTION_E */
    {0x02900000, 0x4, 0x5000}
};

/**
* @enum PRV_DXCH_XCAT_TABLES_E
 *
 * @brief Index for directAccessXcatTableInfo table
*/
typedef enum{

    /** VLAN table index */
    PRV_DXCH_XCAT_TABLE_VLAN_E = 0,

    /** VRD-ID table index */
    PRV_DXCH_XCAT_TABLE_VRF_ID_E,

    /** Port VLAN QoS table index */
    PRV_DXCH_XCAT_TABLE_PORT_VLAN_QOS_E,

    /** STG table index */
    PRV_DXCH_XCAT_TABLE_STG_E,

    /** LTT action table index */
    PRV_DXCH_XCAT_LTT_TT_ACTION_E,

    /** Multicast table index */
    PRV_DXCH_XCAT_TABLE_MULTICAST_E,

    /** Router HA MAC SA table index */
    PRV_CPSS_XCAT_TABLE_ROUTE_HA_MAC_SA_E,

    /** Router HA ARP DA table index */
    PRV_CPSS_XCAT_TABLE_ROUTE_HA_ARP_DA_E,

    /** Ingress VLAN translation table index */
    PRV_DXCH_XCAT_TABLE_INGRESS_TRANSLATION_E,

    /** Egress VLAN translation table index */
    PRV_DXCH_XCAT_TABLE_EGRESS_TRANSLATION_E,

    /** Ingress PCL configuration table */
    PRV_DXCH_XCAT_TABLE_IPCL_CFG_E,

    /** @brief Ingress PCL Lookup1
     *  configuration table
     */
    PRV_DXCH_XCAT_TABLE_IPCL_LOOKUP1_CFG_E,

    /** Egress PCL configuration table */
    PRV_DXCH_XCAT_TABLE_EPCL_CFG_E,

    /** Ingress PCL UDB Cfg table */
    PRV_DXCH_XCAT_TABLE_IPCL_UDB_CFG_E,

    /** Tunnel Start table */
    PRV_DXCH_XCAT_TABLE_TUNNEL_START_CFG_E,

    /** @brief Port Protocol VID and QoS
     *  Configuration Table
     */
    PRV_DXCH_XCAT_TABLE_VLAN_PORT_PROTOCOL_E,

    /** TTI MAC2ME table */
    PRV_CPSS_XCAT_TABLE_MAC2ME_E,

    /** BCN Profiles Table */
    PRV_DXCH_XCAT_TABLE_BCN_PROFILE_E,

    /** Egress Policer Remarking table */
    PRV_DXCH_XCAT_TABLE_EGRESS_POLICER_REMARKING_E,

    /** L2MLL VIDX enable table */
    PRV_DXCH_XCAT_TABLE_L2MLL_VIDX_ENABLE_E,

    /** L2MLL Pointer Mapping table */
    PRV_DXCH_XCAT_TABLE_L2MLL_POINTER_MAPPING_E

} PRV_DXCH_XCAT_TABLES_E;

/**
* @enum PRV_DXCH_XCAT3_DIRECT_ACCESS_TABLES_E
 *
 * @brief Index for directAccessXcatTableInfo table
*/
typedef enum{

    /** @brief Port Protocol VID and QoS
     *  Configuration Table
     */
    PRV_DXCH_XCAT3_TABLE_VLAN_PORT_PROTOCOL_E = 0,

    /** Port VLAN QoS table */
    PRV_DXCH_XCAT3_TABLE_PORT_VLAN_QOS_E,

    /** Ingress PCL configuration table */
    PRV_DXCH_XCAT3_TABLE_IPCL_CFG_E,

    /** STG table */
    PRV_DXCH_XCAT3_TABLE_STG_E,

    /** VLAN table */
    PRV_DXCH_XCAT3_TABLE_VLAN_E,

    /** Multicast table */
    PRV_DXCH_XCAT3_TABLE_MULTICAST_E,

    /** Router HA MAC SA table */
    PRV_DXCH_XCAT3_TABLE_ROUTE_HA_MAC_SA_E,

    /** Router HA ARP DA table */
    PRV_DXCH_XCAT3_TABLE_ROUTE_HA_ARP_DA_E,

    /** Egress PCL configuration table */
    PRV_DXCH_XCAT3_TABLE_EPCL_CFG_E,

    /** Tunnel Start table */
    PRV_DXCH_XCAT3_TABLE_TUNNEL_START_CFG_E,

    /** @brief QoS Profile To Route
     *  Block Offset table
     */
    PRV_DXCH_XCAT3_TABLE_QOS_PROFILE_TO_ROUTE_BLOCK_E,

    /** Router Access Matrix table */
    PRV_DXCH_XCAT3_TABLE_ROUTE_ACCESS_MATRIX_E,

    /** Router Next Hop table */
    PRV_DXCH_XCAT3_UC_MC_ROUTE_NEXT_HOP_E,

    /** Router Next Hop Aging Bits table */
    PRV_DXCH_XCAT3_ROUTE_NEXT_HOP_AGE_E,

    /** TTI MAC2ME table */
    PRV_DXCH_XCAT3_TABLE_MAC2ME_E,

    /** Ingress VLAN translation table */
    PRV_DXCH_XCAT3_TABLE_INGRESS_TRANSLATION_E,

    /** Egress VLAN translation table */
    PRV_DXCH_XCAT3_TABLE_EGRESS_TRANSLATION_E,

    /** VRD-ID table */
    PRV_DXCH_XCAT3_TABLE_VRF_ID_E,

    /** LTT action table */
    PRV_DXCH_XCAT3_LTT_TT_ACTION_E,

    /** @brief Ingress PCL Lookup1
     *  configuration table
     */
    PRV_DXCH_XCAT3_TABLE_IPCL_LOOKUP1_CFG_E,

    /** Ingress PCL UDB Cfg table */
    PRV_DXCH_XCAT3_TABLE_IPCL_UDB_CFG_E,

    /** BCN Profiles Table */
    PRV_DXCH_XCAT3_TABLE_BCN_PROFILE_E,

    /** Egress Policer Remarking table */
    PRV_DXCH_XCAT3_TABLE_EGRESS_POLICER_REMARKING_E,

    /** L2MLL VIDX enable table */
    PRV_DXCH_XCAT3_TABLE_MLL_L2MLL_VIDX_ENABLE_E,

    /** L2MLL Pointer Mapping table */
    PRV_DXCH_XCAT3_TABLE_MLL_L2MLL_POINTER_MAPPING_E,

    /** Ingress ECID Port filtration table */
    PRV_DXCH_XCAT3_TABLE_INGRESS_ECID_E,

    /** MLL table index */
    PRV_DXCH_XCAT3_TABLE_MLL_E,

    /** L2 port isolation table index */
    PRV_DXCH_XCAT3_TABLE_PORT_ISOLATION_L2_E,

    /** L3 port isolation table index */
    PRV_DXCH_XCAT3_TABLE_PORT_ISOLATION_L3_E,

    /** Ingress policer 0 */
    PRV_DXCH_XCAT3_TABLE_INGRESS_POLICER_METERING_0_E,

    /** Ingress policer 1 */
    PRV_DXCH_XCAT3_TABLE_INGRESS_POLICER_METERING_1_E,

    /** Egress policer */
    PRV_DXCH_XCAT3_TABLE_EGRESS_POLICER_METERING_E,

    /** Ingress counting 0 */
    PRV_DXCH_XCAT3_TABLE_INGRESS_POLICER_COUNTING_0_E,

    /** Ingress counting 1 */
    PRV_DXCH_XCAT3_TABLE_INGRESS_POLICER_COUNTING_1_E,

    /** Egress counting */
    PRV_DXCH_XCAT3_TABLE_EGRESS_POLICER_COUNTING_E

} PRV_DXCH_XCAT3_DIRECT_ACCESS_TABLES_E;

/**
* @enum PRV_DXCH_XCAT3_INDIRECT_ACCESS_TABLES_E
 *
 * @brief Index for indirectAccessXcatTableInfo table
*/
typedef enum{

    /** Trunk Members table. */
    PRV_DXCH_XCAT3_TABLE_TRUNK_MEMBERS_E = 0,

    /** Statistical Rate Linit table */
    PRV_DXCH_XCAT3_TABLE_STATISTICAL_RATE_LIMIT_E,

    /** CPU Code table */
    PRV_DXCH_XCAT3_TABLE_CPU_CODE_E,

    /** Qos Profile To Qos table */
    PRV_DXCH_XCAT3_TABLE_QOS_PROFILE_E,

    /** FDB table */
    PRV_DXCH_XCAT3_TABLE_FDB_E,

    /** Logical Target Mapping table */
    PRV_DXCH_XCAT3_TABLE_LOGICAL_TARGET_MAPPING_E,

    /** Egress Filter Vlan Mapping table */
    PRV_DXCH_XCAT3_TABLE_VLAN_MAPPING_E,

    /** Egress Filter Vlan Member table */
    PRV_DXCH_XCAT3_TABLE_LP_EGRESS_VLAN_MEMBER_E,

    /** L2MLL VIDX enable table */
    PRV_DXCH_XCAT3_TABLE_L2MLL_VIDX_ENABLE_E,

    /** L2MLL Pointer Mapping table */
    PRV_DXCH_XCAT3_TABLE_L2MLL_POINTER_MAPPING_E

} PRV_DXCH_XCAT3_INDIRECT_ACCESS_TABLES_E;

/**
* @enum PRV_DXCH_LION_TABLES_E
 *
 * @brief Index for directAccessLion2TableInfoExt table
*/
typedef enum{

    /** index of last xCat table */
    PRV_DXCH_LION_TABLE_START_E = PRV_DXCH_XCAT_TABLE_EGRESS_POLICER_REMARKING_E,

    /** Ingress VLAN table index */
    PRV_DXCH_LION_TABLE_VLAN_INGRESS_E,

    /** Egress VLAN table index */
    PRV_DXCH_LION_TABLE_VLAN_EGRESS_E,

    /** Ingress STG table index */
    PRV_DXCH_LION_TABLE_STG_INGRESS_E,

    /** Egress STG table index */
    PRV_DXCH_LION_TABLE_STG_EGRESS_E,

    /** L2 port isolation table index */
    PRV_CPSS_LION_TABLE_PORT_ISOLATION_L2_E,

    /** L3 port isolation table index */
    PRV_CPSS_LION_TABLE_PORT_ISOLATION_L3_E,

    /** @brief shaper per port per priority
     *  table index
     *  PRV_CPSS_LION_TABLE_TXQ_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG_E-
     *  shaper per port table index
     */
    PRV_CPSS_LION_TABLE_TXQ_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG_E,

    PRV_CPSS_LION_TABLE_TXQ_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG_E,

    /** Source ID table index */
    PRV_CPSS_LION_TABLE_TXQ_SOURCE_ID_MEMBERS_E,

    /** Non trunk table index */
    PRV_CPSS_LION_TABLE_TXQ_NON_TRUNK_MEMBERS_E,

    /** Trunk designated table index */
    PRV_CPSS_LION_TABLE_TXQ_DESIGNATED_PORT_E,

    /** Egress STC table index */
    PRV_CPSS_LION_TABLE_TXQ_EGRESS_STC_E,

    /** VLAN URPF table index */
    PRV_CPSS_LION_TABLE_ROUTER_VLAN_URPF_STC_E,

    /** Trunk Hash mask table index */
    PRV_CPSS_LION_TABLE_TRUNK_HASH_MASK_CRC_E

} PRV_DXCH_LION_TABLES_E;

/**
* @enum PRV_DXCH_XCAT2_TABLES_E
 *
 * @brief Index for directAccessXCAT2TableInfo table
*/
typedef enum{

    /** @brief IPCL Lookup 0_1
     *  Configuration table index
     */
    PRV_DXCH_XCAT2_TABLE_IPCL_LOOKUP01_CFG_E = 0,

    /** @brief IPCL Lookup 1
     *  Configuration table index
     */
    PRV_DXCH_XCAT2_TABLE_IPCL_LOOKUP1_CFG_E,

    /** MAC-2-Me table index */
    PRV_DXCH_XCAT2_TABLE_MAC2ME_E

} PRV_DXCH_XCAT2_TABLES_E;

static const PRV_CPSS_ENTRY_FORMAT_TABLE_STC prvCpssDxChSip5TtiDefaultEportTableFieldsFormat[SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS___LAST_VALUE___E] =
{
    PRV_CPSS_FIELD_MAC(   0,   1, SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_PROT_BASED_QOS_EN_E),
    PRV_CPSS_FIELD_MAC(   1,   1, SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_PROT_BASED_VLAN_EN_E),
    PRV_CPSS_FIELD_MAC(   2,  12, SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_DEF_TAG1_VLAN_ID_E),
    PRV_CPSS_FIELD_MAC(  14,   3, SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_PORT_UP0_E),
    PRV_CPSS_FIELD_MAC(  17,   3, SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_INGRESS_TAG0_TPID_PROFILE_E),
    PRV_CPSS_FIELD_MAC(  20,   3, SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_INGRESS_TAG1_TPID_PROFILE_E),
    PRV_CPSS_FIELD_MAC(  23,   1, SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_VLAN_TRANSLATION_E),
    PRV_CPSS_FIELD_MAC(  24,   1, SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_P_VID_PRECEDENCE_E),
    PRV_CPSS_FIELD_MAC(  25,   1, SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_P_EVLAN_MODE_E),
    PRV_CPSS_FIELD_MAC(  26,  13, SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_P_EVLAN_E),
    PRV_CPSS_FIELD_MAC(  39,   1, SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_TTI_802_1AH_MAC_TO_ME_EN_E),
    PRV_CPSS_FIELD_MAC(  40,   1, SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_EN_802_1AH_TTI_LOOKUP_E),
    PRV_CPSS_FIELD_MAC(  41,   1, SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_ETHERNET_CLASSIFIER_EN_E),
    PRV_CPSS_FIELD_MAC(  42,   1, SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_MPLS_TUNNEL_TERMINATION_EN_E),
    PRV_CPSS_FIELD_MAC(  43,   1, SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_TTI_MPLS_MAC_TO_ME_EN_E),
    PRV_CPSS_FIELD_MAC(  44,   1, SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_TTI_IPV4_MAC_TO_ME_EN_E),
    PRV_CPSS_FIELD_MAC(  45,   1, SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_IPV4_TTI_FOR_TT_ONLY_E),
    PRV_CPSS_FIELD_MAC(  46,   1, SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_IPV4_TUNNEL_TERMINATION_EN_E),
    PRV_CPSS_FIELD_MAC(  47,   1, SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_EXTENDED_DSA_BYPASS_BRIDGE_E),
    PRV_CPSS_FIELD_MAC(  48,  10, SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_PORT_QOS_PROFILE_E),
    PRV_CPSS_FIELD_MAC(  58,   1, SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_TRUST_DSA_TAG_QOS_E),
    PRV_CPSS_FIELD_MAC(  59,   1, SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_TRUST_EXP_E),
    PRV_CPSS_FIELD_MAC(  60,   1, SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_TRUST_DSCP_E),
    PRV_CPSS_FIELD_MAC(  61,   1, SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_TRUST_UP_E),
    PRV_CPSS_FIELD_MAC(  62,   1, SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_MAP_DSCP_TO_DSCP_E),
    PRV_CPSS_FIELD_MAC(  63,   4, SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_TRUST_QOS_MAPPING_TABLE_INDEX_E),
    PRV_CPSS_FIELD_MAC(  67,   1, SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_PORT_MODIFY_DSCP_E),
    PRV_CPSS_FIELD_MAC(  68,   1, SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_PORT_MODIFY_UP_E),
    PRV_CPSS_FIELD_MAC(  69,   1, SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_PORT_QOS_PRECEDENCE_E),
    PRV_CPSS_FIELD_MAC(  70,   1, SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_NESTED_VLAN_ACCESS_PORT_E),
    PRV_CPSS_FIELD_MAC(  71,   1, SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_OVERSIZE_UNTAGGED_PKTS_FILTER_EN_E),
    PRV_CPSS_FIELD_MAC(  72,   1, SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_TRILL_ENGINE_EN_E),
    PRV_CPSS_FIELD_MAC(  73,  12, SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_TRILL_INGRESS_OUTER_VID0_E),
    PRV_CPSS_FIELD_MAC(  85,   1, SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_TTI_802_1AH_PASSENGER_STAG_IS_TAG0_1_E),
    PRV_CPSS_FIELD_MAC(  86,   1, SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_IPV4_6_TOTAL_LENGTH_DEDUCTION_EN_E),
    PRV_CPSS_FIELD_MAC(  87,   1, SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_OAM_LINK_LAYER_PDU_TRAP_EN_E),
    PRV_CPSS_FIELD_MAC(  88,   1, SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_OAM_LINK_LAYER_LOOPBACK_EN_E),
    PRV_CPSS_FIELD_MAC(  89,   1, SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_UP_CFI_TO_QOS_TABLE_SELECT_MODE_E),
    PRV_CPSS_FIELD_MAC(  90,   2, SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_LOOKUP0_PCL_CFG_MODE_E),
    PRV_CPSS_FIELD_MAC(  92,   2, SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_LOOKUP1_PCL_CFG_MODE_E),
    PRV_CPSS_FIELD_MAC(  94,   2, SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_LOOKUP2_PCL_CFG_MODE_E),
    PRV_CPSS_FIELD_MAC(  96,   1, SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_ASSIGN_VF_ID_EN_E),
    PRV_CPSS_FIELD_MAC(  97,   1, SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_TRUST_L2_QOS_TAG0_OR_TAG1_E),
    PRV_CPSS_FIELD_MAC(  98,  16, SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_TTI_PKT_TYPE_UDB_KEY_I_ENABLE_E),
    PRV_CPSS_FIELD_MAC( 114,   3, SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_IPV4_MC_DUPLICATION_MODE_E),
    PRV_CPSS_FIELD_MAC( 117,   3, SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_IPV6_MC_DUPLICATION_MODE_E),
    PRV_CPSS_FIELD_MAC( 120,   1, SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_MPLS_MC_DUPLICATION_ENABLE_E),
    PRV_CPSS_FIELD_MAC( 121,   1, SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_TRILL_MC_DUPLICATION_ENABLE_E),
    PRV_CPSS_FIELD_MAC( 122,   1, SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_PBB_MC_DUPLICATION_ENABLE_E),
    PRV_CPSS_FIELD_MAC( 123,   3, SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_MRU_INDEX_E),
    PRV_CPSS_FIELD_MAC( 126,  12, SIP6_30_TTI_DEFAULT_EPORT_TABLE_FIELDS_SOURCE_EPG_E),
    PRV_CPSS_FIELD_MAC( 138,   1, SIP6_30_TTI_DEFAULT_EPORT_TABLE_FIELDS_COPY_TAG1_VID_TO_SRC_EPG_E),
    PRV_CPSS_FIELD_MAC( 139,   3, SIP6_30_TTI_DEFAULT_EPORT_TABLE_FIELDS_EPORT_UP1_E),
    PRV_CPSS_FIELD_MAC( 142,   1, SIP6_30_TTI_DEFAULT_EPORT_TABLE_FIELDS_EPORT_DEI1_E),
    PRV_CPSS_FIELD_MAC( 143,   1, SIP6_30_TTI_DEFAULT_EPORT_TABLE_FIELDS_EPORT_DEI0_E),
    PRV_CPSS_FIELD_MAC( 144,   1, SIP6_30_TTI_DEFAULT_EPORT_TABLE_FIELDS_PRP_ENABLED_E),
    PRV_CPSS_FIELD_MAC( 145,   4, SIP6_30_TTI_DEFAULT_EPORT_TABLE_FIELDS_PRT_PORT_LAN_ID_E),
    PRV_CPSS_FIELD_MAC( 149,   1, SIP6_30_TTI_DEFAULT_EPORT_TABLE_FIELDS_PRT_TREAT_WRONG_LAN_ID_AS_RCT_EXISTS_E),
    PRV_CPSS_FIELD_MAC( 150,   5, SIP6_30_TTI_DEFAULT_EPORT_TABLE_FIELDS_LSDU_CONSTANT_E)
};

static const PRV_CPSS_ENTRY_FORMAT_TABLE_STC prvCpssDxChSip5TtiPhysicalPortTableFieldsFormat[SIP5_TTI_PHYSICAL_PORT_TABLE_FIELDS___LAST_VALUE___E] =
{
    PRV_CPSS_FIELD_MAC(   0,  13, SIP5_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_DEFAULT_SOURCE_EPORT_NUMBER_E),
    PRV_CPSS_FIELD_MAC(  13,   1, SIP5_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_IS_RING_CORE_PORT_E),
    PRV_CPSS_FIELD_MAC(  14,   1, SIP5_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_IS_LOOPED_E),
    PRV_CPSS_FIELD_MAC(  15,   1, SIP5_TTI_PHYSICAL_PORT_TABLE_FIELDS_ENABLE_MRU_CHECK_ON_CASCADE_PORT_E),
    PRV_CPSS_FIELD_MAC(  16,  13, SIP5_TTI_PHYSICAL_PORT_TABLE_FIELDS_CC_LINE_CARD_PORTS_DEFAULT_EPORT_BASE_E),
    PRV_CPSS_FIELD_MAC(  29,  13, SIP5_TTI_PHYSICAL_PORT_TABLE_FIELDS_CC_LINE_CARD_TRUNK_DEFAULT_EPORT_BASE_E),
    PRV_CPSS_FIELD_MAC(  42,   1, SIP5_TTI_PHYSICAL_PORT_TABLE_FIELDS_CENTRALIZED_CHASSIS_PORT_ENABLE_E),
    PRV_CPSS_FIELD_MAC(  43,  12, SIP5_TTI_PHYSICAL_PORT_TABLE_FIELDS_TRUNK_ID_E),
    PRV_CPSS_FIELD_MAC(  55,   1, SIP5_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_QOS_MODE_E)
};

static const PRV_CPSS_ENTRY_FORMAT_TABLE_STC prvCpssDxChSip5TtiEportAttributesTableFieldsFormat[SIP5_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS___LAST_VALUE___E] =
{
    PRV_CPSS_FIELD_MAC(   0,   3, SIP5_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_ING_TAG0_TPID_PROFILE_E),
    PRV_CPSS_FIELD_MAC(   3,   3, SIP5_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_ING_TAG1_TPID_PROFILE_E),
    PRV_CPSS_FIELD_MAC(   6,   2, SIP5_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_NUM_OF_TAGS_TO_POP_E),
    PRV_CPSS_FIELD_MAC(   8,   1, SIP5_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_ING_POLICY_EN_E),
    PRV_CPSS_FIELD_MAC(   9,   1, SIP5_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_DIS_IPCL0_FOR_ROUTED_E),
    PRV_CPSS_FIELD_MAC(  10,   1, SIP5_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_TRUST_L2_QOS_TAG0_OR_TAG1_E),
    PRV_CPSS_FIELD_MAC(  11,   1, SIP5_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_OVERRIDE_MASK_HASH_EN_E),
    PRV_CPSS_FIELD_MAC(  12,   4, SIP5_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_HASH_MASK_INDEX_E),
    PRV_CPSS_FIELD_MAC(  16,   1, SIP5_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_PHY_SRC_MC_FILTERING_EN_E)
};

static const PRV_CPSS_ENTRY_FORMAT_TABLE_STC prvCpssDxChSip5TtiDefaultPortProtocolEvlanAndQosConfigTableFieldsFormat[SIP5_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS___LAST_VALUE___E] =
{
    PRV_CPSS_FIELD_MAC(   0,  13, SIP5_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_PROTOCOL_EVLAN_E),
    PRV_CPSS_FIELD_MAC(  13,   2, SIP5_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_EVLAN_COMMAND_E),
    PRV_CPSS_FIELD_MAC(  15,   1, SIP5_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_EVLAN_PRECEDENCE_E),
    PRV_CPSS_FIELD_MAC(  16,   1, SIP5_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_VALID_E),
    PRV_CPSS_FIELD_MAC(  17,   1, SIP5_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_PROTOCOL_MODIFY_UP_E),
    PRV_CPSS_FIELD_MAC(  18,   1, SIP5_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_PROTOCOL_MODIFY_DSCP_E),
    PRV_CPSS_FIELD_MAC(  19,   2, SIP5_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_PROTOCOL_MODIFY_QOS_PROFILE_E),
    PRV_CPSS_FIELD_MAC(  21,  10, SIP5_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_QOS_PROFILE_E),
    PRV_CPSS_FIELD_MAC(  31,   1, SIP5_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_PROTOCOL_QOS_PRECEDENCE_E)
};

static const PRV_CPSS_ENTRY_FORMAT_TABLE_STC prvCpssDxChSip5TtiActionTableFieldsFormat[SIP5_TTI_ACTION_TABLE_FIELDS___LAST_VALUE___E] =
{
    PRV_CPSS_FIELD_MAC(   0,   3, SIP5_TTI_ACTION_TABLE_FIELDS_COMMAND_E),
    PRV_CPSS_FIELD_MAC(   3,   8, SIP5_TTI_ACTION_TABLE_FIELDS_CPU_CODE_E),
    PRV_CPSS_FIELD_MAC(  11,   3, SIP5_TTI_ACTION_TABLE_FIELDS_MIRROR_TO_ANALYZER_PORT_E),
    PRV_CPSS_FIELD_MAC(  14,   3, SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_CMD_E),
    PRV_CPSS_FIELD_MAC(  17,  12, SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_FLOW_ID_E),
    PRV_CPSS_FIELD_MAC(  29,   3, SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_IPCL_UDB_CONFIGURATION_TABLE_UDE_INDEX_E),
    PRV_CPSS_FIELD_MAC(  32,   1, SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_RESERVED_E),
    PRV_CPSS_FIELD_MAC(  33,   1, SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_TO_EGRESS_INTERFACE_TTI_RESERVED_ENABLE_E),
    PRV_CPSS_FIELD_MAC(  34,  13, SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_TO_EGRESS_INTERFACE_TTI_RESERVED_VALUE_E),
    PRV_CPSS_FIELD_MAC(  47,   1, SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_TO_EGRESS_INTERFACE_POLICY2_LOOKUP_MODE_E),
    PRV_CPSS_FIELD_MAC(  48,   1, SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_TO_EGRESS_INTERFACE_POLICY1_LOOKUP_MODE_E),
    PRV_CPSS_FIELD_MAC(  49,   1, SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_TO_EGRESS_INTERFACE_POLICY0_LOOKUP_MODE_E),
    PRV_CPSS_FIELD_MAC(  50,  13, SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_TO_EGRESS_INTERFACE_IPCL_PROFILE_INDEX_E),
    PRV_CPSS_FIELD_MAC(  17,   1, SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_IS_TRUNK_E),
    PRV_CPSS_FIELD_MAC(  18,  12, SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_TRUNK_NUMBER_E),
    PRV_CPSS_FIELD_MAC(  18,  13, SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_EPORT_NUMBER_E),
    PRV_CPSS_FIELD_MAC(  31,  10, SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_DEVICE_NUMBER_E),
    PRV_CPSS_FIELD_MAC(  17,  16, SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_EVIDX_E),
    PRV_CPSS_FIELD_MAC(  33,   8, SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_RESERVED_E),
    PRV_CPSS_FIELD_MAC(  41,   1, SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_USE_EVIDX_E),
    PRV_CPSS_FIELD_MAC(  42,   1, SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_VNT_L2_ECHO_E),
    PRV_CPSS_FIELD_MAC(  43,   1, SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_TUNNEL_START_E),
    PRV_CPSS_FIELD_MAC(  44,  15, SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_TUNNEL_INDEX_E),
    PRV_CPSS_FIELD_MAC(  59,   1, SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_TUNNEL_START_PASSENGER_TYPE_E),
    PRV_CPSS_FIELD_MAC(  44,  17, SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_ARP_POINTER_E),
    PRV_CPSS_FIELD_MAC(  61,   1, SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_MODIFY_MAC_DA_E),
    PRV_CPSS_FIELD_MAC(  62,   1, SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_MODIFY_MAC_SA_E),
    PRV_CPSS_FIELD_MAC(  17,  16, SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_ROUTER_ROUTER_LTT_INDEX_E),
    PRV_CPSS_FIELD_MAC(  17,  12, SIP5_TTI_ACTION_TABLE_FIELDS_ASSIGN_VRF_ID_VRF_ID_E),
    PRV_CPSS_FIELD_MAC(  63,   1, SIP5_TTI_ACTION_TABLE_FIELDS_BIND_TO_CNC_COUNTER_E),
    PRV_CPSS_FIELD_MAC(  64,  14, SIP5_TTI_ACTION_TABLE_FIELDS_CNC_COUNTER_INDEX_E),
    PRV_CPSS_FIELD_MAC(  78,   1, SIP5_TTI_ACTION_TABLE_FIELDS_BIND_TO_POLICER_METER_E),
    PRV_CPSS_FIELD_MAC(  79,   1, SIP5_TTI_ACTION_TABLE_FIELDS_BIND_TO_POLICER_COUNTER_E),
    PRV_CPSS_FIELD_MAC(  80,  16, SIP5_TTI_ACTION_TABLE_FIELDS_POLICER_INDEX_E),
    PRV_CPSS_FIELD_MAC(  96,   1, SIP5_TTI_ACTION_TABLE_FIELDS_SOURCE_ID_SET_ENABLE_E),
    PRV_CPSS_FIELD_MAC(  97,  12, SIP5_TTI_ACTION_TABLE_FIELDS_SOURCE_ID_E),
    PRV_CPSS_FIELD_MAC( 109,   1, SIP5_TTI_ACTION_TABLE_FIELDS_ACTION_STOP_E),
    PRV_CPSS_FIELD_MAC( 110,   1, SIP5_TTI_ACTION_TABLE_FIELDS_BYPASS_BRIDGE_E),
    PRV_CPSS_FIELD_MAC( 111,   1, SIP5_TTI_ACTION_TABLE_FIELDS_BYPASS_INGRESS_PIPE_E),
    PRV_CPSS_FIELD_MAC( 112,   1, SIP5_TTI_ACTION_TABLE_FIELDS_EVLAN_PRECEDENCE_E),
    PRV_CPSS_FIELD_MAC( 113,   1, SIP5_TTI_ACTION_TABLE_FIELDS_ENABLE_NESTED_VLAN_E),
    PRV_CPSS_FIELD_MAC( 114,   3, SIP5_TTI_ACTION_TABLE_FIELDS_EVID_CMD_E),
    PRV_CPSS_FIELD_MAC( 117,  13, SIP5_TTI_ACTION_TABLE_FIELDS_EVLAN_E),
    PRV_CPSS_FIELD_MAC( 130,   1, SIP5_TTI_ACTION_TABLE_FIELDS_VID1_CMD_E),
    PRV_CPSS_FIELD_MAC( 131,  12, SIP5_TTI_ACTION_TABLE_FIELDS_VID1_E),
    PRV_CPSS_FIELD_MAC( 143,   1, SIP5_TTI_ACTION_TABLE_FIELDS_QOS_PRECEDENCE_E),
    PRV_CPSS_FIELD_MAC( 144,   7, SIP5_TTI_ACTION_TABLE_FIELDS_QOS_PROFILE_E),
    PRV_CPSS_FIELD_MAC( 151,   2, SIP5_TTI_ACTION_TABLE_FIELDS_MODIFY_DSCP_E),
    PRV_CPSS_FIELD_MAC( 153,   2, SIP5_TTI_ACTION_TABLE_FIELDS_MODIFY_UP_E),
    PRV_CPSS_FIELD_MAC( 155,   1, SIP5_TTI_ACTION_TABLE_FIELDS_KEEP_PREVIOUS_QOS_E),
    PRV_CPSS_FIELD_MAC( 156,   1, SIP5_TTI_ACTION_TABLE_FIELDS_TRUST_UP_E),
    PRV_CPSS_FIELD_MAC( 157,   1, SIP5_TTI_ACTION_TABLE_FIELDS_TRUST_DSCP_E),
    PRV_CPSS_FIELD_MAC( 158,   1, SIP5_TTI_ACTION_TABLE_FIELDS_TRUST_EXP_E),
    PRV_CPSS_FIELD_MAC( 159,   1, SIP5_TTI_ACTION_TABLE_FIELDS_MAP_DSCP_E),
    PRV_CPSS_FIELD_MAC( 160,   3, SIP5_TTI_ACTION_TABLE_FIELDS_UP0_E),
    PRV_CPSS_FIELD_MAC( 163,   2, SIP5_TTI_ACTION_TABLE_FIELDS_UP1_COMMAND_E),
    PRV_CPSS_FIELD_MAC( 165,   3, SIP5_TTI_ACTION_TABLE_FIELDS_UP1_E),
    PRV_CPSS_FIELD_MAC( 168,   2, SIP5_TTI_ACTION_TABLE_FIELDS_PASSENGER_PACKET_TYPE_E),
    PRV_CPSS_FIELD_MAC( 170,   1, SIP5_TTI_ACTION_TABLE_FIELDS_COPY_TTL_E),
    PRV_CPSS_FIELD_MAC( 171,   1, SIP5_TTI_ACTION_TABLE_FIELDS_TUNNEL_TERMINATION_E),
    PRV_CPSS_FIELD_MAC( 172,   3, SIP5_TTI_ACTION_TABLE_FIELDS_MPLS_COMMAND_E),
    PRV_CPSS_FIELD_MAC( 175,   4, SIP5_TTI_ACTION_TABLE_FIELDS_HASH_MASK_INDEX_E),
    PRV_CPSS_FIELD_MAC( 179,   4, SIP5_TTI_ACTION_TABLE_FIELDS_TRUST_QOS_MAPPING_TABLE_INDEX_E),
    PRV_CPSS_FIELD_MAC( 183,   1, SIP5_TTI_ACTION_TABLE_FIELDS_L_LSP_QOS_PROFILE_ENABLE_E),
    PRV_CPSS_FIELD_MAC( 184,   8, SIP5_TTI_ACTION_TABLE_FIELDS_TTL_E),
    PRV_CPSS_FIELD_MAC( 192,   1, SIP5_TTI_ACTION_TABLE_FIELDS_ENABLE_DEC_TTL_E),
    PRV_CPSS_FIELD_MAC( 193,   1, SIP5_TTI_ACTION_TABLE_FIELDS_SOURCE_EPORT_ASSIGNMENT_ENABLE_E),
    PRV_CPSS_FIELD_MAC( 194,  13, SIP5_TTI_ACTION_TABLE_FIELDS_SOURCE_EPORT_E),
    PRV_CPSS_FIELD_MAC( 207,   5, SIP5_TTI_ACTION_TABLE_FIELDS_TT_HEADER_LENGTH_E),
    PRV_CPSS_FIELD_MAC( 212,   1, SIP5_TTI_ACTION_TABLE_FIELDS_IS_PTP_PACKET_E),
    PRV_CPSS_FIELD_MAC( 213,   1, SIP5_TTI_ACTION_TABLE_FIELDS_NON_PTP_PACKET_TIMESTAMP_ENABLE_E),
    PRV_CPSS_FIELD_MAC( 214,   7, SIP5_TTI_ACTION_TABLE_FIELDS_NON_PTP_PACKET_OFFSET_INDEX_E),
    PRV_CPSS_FIELD_MAC( 213,   2, SIP5_TTI_ACTION_TABLE_FIELDS_PTP_PACKET_PTP_TRIGGER_TYPE_E),
    PRV_CPSS_FIELD_MAC( 215,   7, SIP5_TTI_ACTION_TABLE_FIELDS_PTP_PACKET_PTP_OFFSET_E),
    PRV_CPSS_FIELD_MAC( 222,   0, SIP5_TTI_ACTION_TABLE_FIELDS_ENABLE_OAM_PROCESSING_WHEN_GAL_OR_OAL_EXISTS_E),
    PRV_CPSS_FIELD_MAC( 222,   1, SIP5_TTI_ACTION_TABLE_FIELDS_OAM_PROCESSING_ENABLE_E),
    PRV_CPSS_FIELD_MAC( 223,   1, SIP5_TTI_ACTION_TABLE_FIELDS_CW_BASED_PSEUDO_WIRE_E),
    PRV_CPSS_FIELD_MAC( 224,   1, SIP5_TTI_ACTION_TABLE_FIELDS_TTL_EXPIRY_VCCV_ENABLE_E),
    PRV_CPSS_FIELD_MAC( 225,   1, SIP5_TTI_ACTION_TABLE_FIELDS_PWE3_FLOW_LABEL_EXIST_E),
    PRV_CPSS_FIELD_MAC( 226,   1, SIP5_TTI_ACTION_TABLE_FIELDS_PW_CW_BASED_E_TREE_ENABLE_E),
    PRV_CPSS_FIELD_MAC( 227,   1, SIP5_TTI_ACTION_TABLE_FIELDS_UP_CFI_TO_QOS_TABLE_SELECT_MODE_E),
    PRV_CPSS_FIELD_MAC( 228,   1, SIP5_TTI_ACTION_TABLE_FIELDS_RX_IS_PROTECTION_PATH_E),
    PRV_CPSS_FIELD_MAC( 229,   1, SIP5_TTI_ACTION_TABLE_FIELDS_RX_ENABLE_PROTECTION_SWITCHING_E),
    PRV_CPSS_FIELD_MAC( 230,   1, SIP5_TTI_ACTION_TABLE_FIELDS_SET_MAC2ME_E),
    PRV_CPSS_FIELD_MAC( 231,   1, SIP5_TTI_ACTION_TABLE_FIELDS_OAM_PROFILE_E),
    PRV_CPSS_FIELD_MAC( 232,   1, SIP5_TTI_ACTION_TABLE_FIELDS_APPLY_NON_DATA_CW_COMMAND_E),
    PRV_CPSS_FIELD_MAC( 233,   2, SIP5_TTI_ACTION_TABLE_FIELDS_PW_TAG_MODE_E),
    PRV_CPSS_FIELD_MAC( 235,   1, SIP5_TTI_ACTION_TABLE_FIELDS_ENABLE_TTI_1_LOOKUP_E),
    PRV_CPSS_FIELD_MAC( 236,   1, SIP5_TTI_ACTION_TABLE_FIELDS_CHANNEL_TYPE_TO_OPCODE_MAPPING_EN_E),
    PRV_CPSS_FIELD_MAC( 237,   2, SIP5_TTI_ACTION_TABLE_FIELDS_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_ENABLE_E),
    PRV_CPSS_FIELD_MAC( 239,   1, SIP5_TTI_ACTION_TABLE_FIELDS_PASSENGER_PARSING_OF_NON_MPLS_TRANSIT_TUNNEL_ENABLE_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_TTI_ACTION_TABLE_FIELDS_SKIP_FDB_SA_LOOKUP_ENABLE_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_TTI_ACTION_TABLE_FIELDS_IPV6_SEGMENT_ROUTING_END_NODE_ENABLE_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_TTI_ACTION_TABLE_FIELDS_TUNNEL_HEADER_START_L4_ENABLE_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_TTI_ACTION_TABLE_FIELDS_RESERVED_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_TTI_ACTION_TABLE_FIELDS_TTI_OVER_EXACT_MATCH_ENABLE_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_10_TTI_ACTION_TABLE_FIELDS_TRIGER_CNC_HASH_CLIENT_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_10_TTI_ACTION_TABLE_FIELDS_IPFIX_ENABLE_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_10_TTI_ACTION_TABLE_FIELDS_PPU_PROFILE_E)
};


static const PRV_CPSS_ENTRY_FORMAT_TABLE_STC prvCpssDxChSip5_10TtiActionTableFieldsFormat[SIP5_TTI_ACTION_TABLE_FIELDS___LAST_VALUE___E] =
{
    PRV_CPSS_FIELD_MAC(   0,   3, SIP5_TTI_ACTION_TABLE_FIELDS_COMMAND_E),
    PRV_CPSS_FIELD_MAC(   3,   8, SIP5_TTI_ACTION_TABLE_FIELDS_CPU_CODE_E),
    PRV_CPSS_FIELD_MAC(  11,   3, SIP5_TTI_ACTION_TABLE_FIELDS_MIRROR_TO_ANALYZER_PORT_E),
    PRV_CPSS_FIELD_MAC(  14,   3, SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_CMD_E),
    PRV_CPSS_FIELD_MAC(  17,  12, SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_FLOW_ID_E),
    PRV_CPSS_FIELD_MAC(  29,   3, SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_IPCL_UDB_CONFIGURATION_TABLE_UDE_INDEX_E),
    PRV_CPSS_FIELD_MAC(  32,   1, SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_RESERVED_E),
    PRV_CPSS_FIELD_MAC(  33,   1, SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_TO_EGRESS_INTERFACE_TTI_RESERVED_ENABLE_E),
    PRV_CPSS_FIELD_MAC(  34,  13, SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_TO_EGRESS_INTERFACE_TTI_RESERVED_VALUE_E),
    PRV_CPSS_FIELD_MAC(  47,   1, SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_TO_EGRESS_INTERFACE_POLICY2_LOOKUP_MODE_E),
    PRV_CPSS_FIELD_MAC(  48,   1, SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_TO_EGRESS_INTERFACE_POLICY1_LOOKUP_MODE_E),
    PRV_CPSS_FIELD_MAC(  49,   1, SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_TO_EGRESS_INTERFACE_POLICY0_LOOKUP_MODE_E),
    PRV_CPSS_FIELD_MAC(  50,  13, SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_TO_EGRESS_INTERFACE_IPCL_PROFILE_INDEX_E),
    PRV_CPSS_FIELD_MAC(  17,   1, SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_IS_TRUNK_E),
    PRV_CPSS_FIELD_MAC(  18,  12, SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_TRUNK_NUMBER_E),
    PRV_CPSS_FIELD_MAC(  18,  13, SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_EPORT_NUMBER_E),
    PRV_CPSS_FIELD_MAC(  31,  10, SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_DEVICE_NUMBER_E),
    PRV_CPSS_FIELD_MAC(  17,  16, SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_EVIDX_E),
    PRV_CPSS_FIELD_MAC(  33,   8, SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_RESERVED_E),
    PRV_CPSS_FIELD_MAC(  41,   1, SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_USE_EVIDX_E),
    PRV_CPSS_FIELD_MAC(  42,   1, SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_VNT_L2_ECHO_E),
    PRV_CPSS_FIELD_MAC(  43,   1, SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_TUNNEL_START_E),
    PRV_CPSS_FIELD_MAC(  44,  15, SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_TUNNEL_INDEX_E),
    PRV_CPSS_FIELD_MAC(  59,   1, SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_TUNNEL_START_PASSENGER_TYPE_E),
    PRV_CPSS_FIELD_MAC(  44,  17, SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_ARP_POINTER_E),
    PRV_CPSS_FIELD_MAC(  61,   1, SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_MODIFY_MAC_DA_E),
    PRV_CPSS_FIELD_MAC(  62,   1, SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_MODIFY_MAC_SA_E),
    PRV_CPSS_FIELD_MAC(  17,  16, SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_ROUTER_ROUTER_LTT_INDEX_E),
    PRV_CPSS_FIELD_MAC(  17,  12, SIP5_TTI_ACTION_TABLE_FIELDS_ASSIGN_VRF_ID_VRF_ID_E),
    PRV_CPSS_FIELD_MAC(  63,   1, SIP5_TTI_ACTION_TABLE_FIELDS_BIND_TO_CNC_COUNTER_E),
    PRV_CPSS_FIELD_MAC(  64,  14, SIP5_TTI_ACTION_TABLE_FIELDS_CNC_COUNTER_INDEX_E),
    PRV_CPSS_FIELD_MAC(  78,   1, SIP5_TTI_ACTION_TABLE_FIELDS_BIND_TO_POLICER_METER_E),
    PRV_CPSS_FIELD_MAC(  79,   1, SIP5_TTI_ACTION_TABLE_FIELDS_BIND_TO_POLICER_COUNTER_E),
    PRV_CPSS_FIELD_MAC(  80,  16, SIP5_TTI_ACTION_TABLE_FIELDS_POLICER_INDEX_E),
    PRV_CPSS_FIELD_MAC(  96,   1, SIP5_TTI_ACTION_TABLE_FIELDS_SOURCE_ID_SET_ENABLE_E),
    PRV_CPSS_FIELD_MAC(  97,  12, SIP5_TTI_ACTION_TABLE_FIELDS_SOURCE_ID_E),
    PRV_CPSS_FIELD_MAC( 109,   1, SIP5_TTI_ACTION_TABLE_FIELDS_ACTION_STOP_E),
    PRV_CPSS_FIELD_MAC( 110,   1, SIP5_TTI_ACTION_TABLE_FIELDS_BYPASS_BRIDGE_E),
    PRV_CPSS_FIELD_MAC( 111,   1, SIP5_TTI_ACTION_TABLE_FIELDS_BYPASS_INGRESS_PIPE_E),
    PRV_CPSS_FIELD_MAC( 112,   1, SIP5_TTI_ACTION_TABLE_FIELDS_EVLAN_PRECEDENCE_E),
    PRV_CPSS_FIELD_MAC( 113,   1, SIP5_TTI_ACTION_TABLE_FIELDS_ENABLE_NESTED_VLAN_E),
    PRV_CPSS_FIELD_MAC( 114,   3, SIP5_TTI_ACTION_TABLE_FIELDS_EVID_CMD_E),
    PRV_CPSS_FIELD_MAC( 117,  13, SIP5_TTI_ACTION_TABLE_FIELDS_EVLAN_E),
    PRV_CPSS_FIELD_MAC( 130,   1, SIP5_TTI_ACTION_TABLE_FIELDS_VID1_CMD_E),
    PRV_CPSS_FIELD_MAC( 131,  12, SIP5_TTI_ACTION_TABLE_FIELDS_VID1_E),
    PRV_CPSS_FIELD_MAC( 143,   1, SIP5_TTI_ACTION_TABLE_FIELDS_QOS_PRECEDENCE_E),
    PRV_CPSS_FIELD_MAC( 144,   7, SIP5_TTI_ACTION_TABLE_FIELDS_QOS_PROFILE_E),
    PRV_CPSS_FIELD_MAC( 151,   2, SIP5_TTI_ACTION_TABLE_FIELDS_MODIFY_DSCP_E),
    PRV_CPSS_FIELD_MAC( 153,   2, SIP5_TTI_ACTION_TABLE_FIELDS_MODIFY_UP_E),
    PRV_CPSS_FIELD_MAC( 155,   1, SIP5_TTI_ACTION_TABLE_FIELDS_KEEP_PREVIOUS_QOS_E),
    PRV_CPSS_FIELD_MAC( 156,   1, SIP5_TTI_ACTION_TABLE_FIELDS_TRUST_UP_E),
    PRV_CPSS_FIELD_MAC( 157,   1, SIP5_TTI_ACTION_TABLE_FIELDS_TRUST_DSCP_E),
    PRV_CPSS_FIELD_MAC( 158,   1, SIP5_TTI_ACTION_TABLE_FIELDS_TRUST_EXP_E),
    PRV_CPSS_FIELD_MAC( 159,   1, SIP5_TTI_ACTION_TABLE_FIELDS_MAP_DSCP_E),
    PRV_CPSS_FIELD_MAC( 160,   3, SIP5_TTI_ACTION_TABLE_FIELDS_UP0_E),
    PRV_CPSS_FIELD_MAC( 163,   2, SIP5_TTI_ACTION_TABLE_FIELDS_UP1_COMMAND_E),
    PRV_CPSS_FIELD_MAC( 165,   3, SIP5_TTI_ACTION_TABLE_FIELDS_UP1_E),
    PRV_CPSS_FIELD_MAC( 168,   2, SIP5_TTI_ACTION_TABLE_FIELDS_PASSENGER_PACKET_TYPE_E),
    PRV_CPSS_FIELD_MAC( 170,   1, SIP5_TTI_ACTION_TABLE_FIELDS_COPY_TTL_E),
    PRV_CPSS_FIELD_MAC( 171,   1, SIP5_TTI_ACTION_TABLE_FIELDS_TUNNEL_TERMINATION_E),
    PRV_CPSS_FIELD_MAC( 172,   3, SIP5_TTI_ACTION_TABLE_FIELDS_MPLS_COMMAND_E),
    PRV_CPSS_FIELD_MAC( 175,   4, SIP5_TTI_ACTION_TABLE_FIELDS_HASH_MASK_INDEX_E),
    PRV_CPSS_FIELD_MAC( 179,   4, SIP5_TTI_ACTION_TABLE_FIELDS_TRUST_QOS_MAPPING_TABLE_INDEX_E),
    PRV_CPSS_FIELD_MAC( 183,   1, SIP5_TTI_ACTION_TABLE_FIELDS_L_LSP_QOS_PROFILE_ENABLE_E),
    PRV_CPSS_FIELD_MAC( 184,   8, SIP5_TTI_ACTION_TABLE_FIELDS_TTL_E),
    PRV_CPSS_FIELD_MAC( 192,   1, SIP5_TTI_ACTION_TABLE_FIELDS_ENABLE_DEC_TTL_E),
    PRV_CPSS_FIELD_MAC( 193,   1, SIP5_TTI_ACTION_TABLE_FIELDS_SOURCE_EPORT_ASSIGNMENT_ENABLE_E),
    PRV_CPSS_FIELD_MAC( 194,  13, SIP5_TTI_ACTION_TABLE_FIELDS_SOURCE_EPORT_E),
    PRV_CPSS_FIELD_MAC( 207,   5, SIP5_TTI_ACTION_TABLE_FIELDS_TT_HEADER_LENGTH_E),
    PRV_CPSS_FIELD_MAC( 212,   1, SIP5_TTI_ACTION_TABLE_FIELDS_IS_PTP_PACKET_E),
    PRV_CPSS_FIELD_MAC( 213,   1, SIP5_TTI_ACTION_TABLE_FIELDS_NON_PTP_PACKET_TIMESTAMP_ENABLE_E),
    PRV_CPSS_FIELD_MAC( 214,   7, SIP5_TTI_ACTION_TABLE_FIELDS_NON_PTP_PACKET_OFFSET_INDEX_E),
    PRV_CPSS_FIELD_MAC( 213,   2, SIP5_TTI_ACTION_TABLE_FIELDS_PTP_PACKET_PTP_TRIGGER_TYPE_E),
    PRV_CPSS_FIELD_MAC( 215,   6, SIP5_TTI_ACTION_TABLE_FIELDS_PTP_PACKET_PTP_OFFSET_E),
    PRV_CPSS_FIELD_MAC( 221,   1, SIP5_TTI_ACTION_TABLE_FIELDS_ENABLE_OAM_PROCESSING_WHEN_GAL_OR_OAL_EXISTS_E),
    PRV_CPSS_FIELD_MAC( 222,   1, SIP5_TTI_ACTION_TABLE_FIELDS_OAM_PROCESSING_ENABLE_E),
    PRV_CPSS_FIELD_MAC( 223,   1, SIP5_TTI_ACTION_TABLE_FIELDS_CW_BASED_PSEUDO_WIRE_E),
    PRV_CPSS_FIELD_MAC( 224,   1, SIP5_TTI_ACTION_TABLE_FIELDS_TTL_EXPIRY_VCCV_ENABLE_E),
    PRV_CPSS_FIELD_MAC( 225,   1, SIP5_TTI_ACTION_TABLE_FIELDS_PWE3_FLOW_LABEL_EXIST_E),
    PRV_CPSS_FIELD_MAC( 226,   1, SIP5_TTI_ACTION_TABLE_FIELDS_PW_CW_BASED_E_TREE_ENABLE_E),
    PRV_CPSS_FIELD_MAC( 227,   1, SIP5_TTI_ACTION_TABLE_FIELDS_UP_CFI_TO_QOS_TABLE_SELECT_MODE_E),
    PRV_CPSS_FIELD_MAC( 228,   1, SIP5_TTI_ACTION_TABLE_FIELDS_RX_IS_PROTECTION_PATH_E),
    PRV_CPSS_FIELD_MAC( 229,   1, SIP5_TTI_ACTION_TABLE_FIELDS_RX_ENABLE_PROTECTION_SWITCHING_E),
    PRV_CPSS_FIELD_MAC( 230,   1, SIP5_TTI_ACTION_TABLE_FIELDS_SET_MAC2ME_E),
    PRV_CPSS_FIELD_MAC( 231,   1, SIP5_TTI_ACTION_TABLE_FIELDS_OAM_PROFILE_E),
    PRV_CPSS_FIELD_MAC( 232,   1, SIP5_TTI_ACTION_TABLE_FIELDS_APPLY_NON_DATA_CW_COMMAND_E),
    PRV_CPSS_FIELD_MAC( 233,   2, SIP5_TTI_ACTION_TABLE_FIELDS_PW_TAG_MODE_E),
    PRV_CPSS_FIELD_MAC( 235,   1, SIP5_TTI_ACTION_TABLE_FIELDS_ENABLE_TTI_1_LOOKUP_E),
    PRV_CPSS_FIELD_MAC( 236,   1, SIP5_TTI_ACTION_TABLE_FIELDS_CHANNEL_TYPE_TO_OPCODE_MAPPING_EN_E),
    PRV_CPSS_FIELD_MAC( 237,   2, SIP5_TTI_ACTION_TABLE_FIELDS_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_ENABLE_E),
    PRV_CPSS_FIELD_MAC( 239,   1, SIP5_TTI_ACTION_TABLE_FIELDS_PASSENGER_PARSING_OF_NON_MPLS_TRANSIT_TUNNEL_ENABLE_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_TTI_ACTION_TABLE_FIELDS_SKIP_FDB_SA_LOOKUP_ENABLE_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_TTI_ACTION_TABLE_FIELDS_IPV6_SEGMENT_ROUTING_END_NODE_ENABLE_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_TTI_ACTION_TABLE_FIELDS_TUNNEL_HEADER_START_L4_ENABLE_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_TTI_ACTION_TABLE_FIELDS_RESERVED_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_TTI_ACTION_TABLE_FIELDS_TTI_OVER_EXACT_MATCH_ENABLE_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_10_TTI_ACTION_TABLE_FIELDS_TRIGER_CNC_HASH_CLIENT_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_10_TTI_ACTION_TABLE_FIELDS_IPFIX_ENABLE_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_10_TTI_ACTION_TABLE_FIELDS_PPU_PROFILE_E)
};

static const PRV_CPSS_ENTRY_FORMAT_TABLE_STC prvCpssDxChSip5_20TtiActionTableFieldsFormat[SIP5_TTI_ACTION_TABLE_FIELDS___LAST_VALUE___E] =
{
    PRV_CPSS_FIELD_MAC(   0,   3, SIP5_TTI_ACTION_TABLE_FIELDS_COMMAND_E),
    PRV_CPSS_FIELD_MAC(   3,   8, SIP5_TTI_ACTION_TABLE_FIELDS_CPU_CODE_E),
    PRV_CPSS_FIELD_MAC(  11,   3, SIP5_TTI_ACTION_TABLE_FIELDS_MIRROR_TO_ANALYZER_PORT_E),
    PRV_CPSS_FIELD_MAC(  14,   3, SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_CMD_E),
    PRV_CPSS_FIELD_MAC(  17,  13, SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_FLOW_ID_E),
    PRV_CPSS_FIELD_MAC(  30,   3, SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_IPCL_UDB_CONFIGURATION_TABLE_UDE_INDEX_E),
    PRV_CPSS_FIELD_MAC(  33,   2, SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_RESERVED_E),
    PRV_CPSS_FIELD_MAC(  35,   1, SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_TO_EGRESS_INTERFACE_TTI_RESERVED_ENABLE_E),
    PRV_CPSS_FIELD_MAC(  36,  13, SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_TO_EGRESS_INTERFACE_TTI_RESERVED_VALUE_E),
    PRV_CPSS_FIELD_MAC(  49,   1, SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_TO_EGRESS_INTERFACE_POLICY2_LOOKUP_MODE_E),
    PRV_CPSS_FIELD_MAC(  50,   1, SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_TO_EGRESS_INTERFACE_POLICY1_LOOKUP_MODE_E),
    PRV_CPSS_FIELD_MAC(  51,   1, SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_TO_EGRESS_INTERFACE_POLICY0_LOOKUP_MODE_E),
    PRV_CPSS_FIELD_MAC(  52,  13, SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_TO_EGRESS_INTERFACE_IPCL_PROFILE_INDEX_E),
    PRV_CPSS_FIELD_MAC(  17,   1, SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_IS_TRUNK_E),
    PRV_CPSS_FIELD_MAC(  18,  12, SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_TRUNK_NUMBER_E),
    PRV_CPSS_FIELD_MAC(  18,  14, SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_EPORT_NUMBER_E),
    PRV_CPSS_FIELD_MAC(  32,  10, SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_DEVICE_NUMBER_E),
    PRV_CPSS_FIELD_MAC(  17,  16, SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_EVIDX_E),
    PRV_CPSS_FIELD_MAC(  33,   9, SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_RESERVED_E),
    PRV_CPSS_FIELD_MAC(  42,   1, SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_USE_EVIDX_E),
    PRV_CPSS_FIELD_MAC(  43,   1, SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_VNT_L2_ECHO_E),
    PRV_CPSS_FIELD_MAC(  44,   1, SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_TUNNEL_START_E),
    PRV_CPSS_FIELD_MAC(  45,  16, SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_TUNNEL_INDEX_E),
    PRV_CPSS_FIELD_MAC(  61,   1, SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_TUNNEL_START_PASSENGER_TYPE_E),
    PRV_CPSS_FIELD_MAC(  45,  18, SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_ARP_POINTER_E),
    PRV_CPSS_FIELD_MAC(  63,   1, SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_MODIFY_MAC_DA_E),
    PRV_CPSS_FIELD_MAC(  64,   1, SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_MODIFY_MAC_SA_E),
    PRV_CPSS_FIELD_MAC(  17,  18, SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_ROUTER_ROUTER_LTT_INDEX_E),
    PRV_CPSS_FIELD_MAC(  17,  12, SIP5_TTI_ACTION_TABLE_FIELDS_ASSIGN_VRF_ID_VRF_ID_E),
    PRV_CPSS_FIELD_MAC(  65,   1, SIP5_TTI_ACTION_TABLE_FIELDS_BIND_TO_CNC_COUNTER_E),
    PRV_CPSS_FIELD_MAC(  66,  14, SIP5_TTI_ACTION_TABLE_FIELDS_CNC_COUNTER_INDEX_E),
    PRV_CPSS_FIELD_MAC(  80,   1, SIP5_TTI_ACTION_TABLE_FIELDS_BIND_TO_POLICER_METER_E),
    PRV_CPSS_FIELD_MAC(  81,   1, SIP5_TTI_ACTION_TABLE_FIELDS_BIND_TO_POLICER_COUNTER_E),
    PRV_CPSS_FIELD_MAC(  82,  14, SIP5_TTI_ACTION_TABLE_FIELDS_POLICER_INDEX_E),
    PRV_CPSS_FIELD_MAC(  96,   1, SIP5_TTI_ACTION_TABLE_FIELDS_SOURCE_ID_SET_ENABLE_E),
    PRV_CPSS_FIELD_MAC(  97,  12, SIP5_TTI_ACTION_TABLE_FIELDS_SOURCE_ID_E),
    PRV_CPSS_FIELD_MAC( 109,   1, SIP5_TTI_ACTION_TABLE_FIELDS_ACTION_STOP_E),
    PRV_CPSS_FIELD_MAC( 110,   1, SIP5_TTI_ACTION_TABLE_FIELDS_BYPASS_BRIDGE_E),
    PRV_CPSS_FIELD_MAC( 111,   1, SIP5_TTI_ACTION_TABLE_FIELDS_BYPASS_INGRESS_PIPE_E),
    PRV_CPSS_FIELD_MAC( 112,   1, SIP5_TTI_ACTION_TABLE_FIELDS_EVLAN_PRECEDENCE_E),
    PRV_CPSS_FIELD_MAC( 113,   1, SIP5_TTI_ACTION_TABLE_FIELDS_ENABLE_NESTED_VLAN_E),
    PRV_CPSS_FIELD_MAC( 114,   3, SIP5_TTI_ACTION_TABLE_FIELDS_EVID_CMD_E),
    PRV_CPSS_FIELD_MAC( 117,  13, SIP5_TTI_ACTION_TABLE_FIELDS_EVLAN_E),
    PRV_CPSS_FIELD_MAC( 130,   1, SIP5_TTI_ACTION_TABLE_FIELDS_VID1_CMD_E),
    PRV_CPSS_FIELD_MAC( 131,  12, SIP5_TTI_ACTION_TABLE_FIELDS_VID1_E),
    PRV_CPSS_FIELD_MAC( 143,   1, SIP5_TTI_ACTION_TABLE_FIELDS_QOS_PRECEDENCE_E),
    PRV_CPSS_FIELD_MAC( 144,   7, SIP5_TTI_ACTION_TABLE_FIELDS_QOS_PROFILE_E),
    PRV_CPSS_FIELD_MAC( 151,   2, SIP5_TTI_ACTION_TABLE_FIELDS_MODIFY_DSCP_E),
    PRV_CPSS_FIELD_MAC( 153,   2, SIP5_TTI_ACTION_TABLE_FIELDS_MODIFY_UP_E),
    PRV_CPSS_FIELD_MAC( 155,   1, SIP5_TTI_ACTION_TABLE_FIELDS_KEEP_PREVIOUS_QOS_E),
    PRV_CPSS_FIELD_MAC( 156,   1, SIP5_TTI_ACTION_TABLE_FIELDS_TRUST_UP_E),
    PRV_CPSS_FIELD_MAC( 157,   1, SIP5_TTI_ACTION_TABLE_FIELDS_TRUST_DSCP_E),
    PRV_CPSS_FIELD_MAC( 158,   1, SIP5_TTI_ACTION_TABLE_FIELDS_TRUST_EXP_E),
    PRV_CPSS_FIELD_MAC( 159,   1, SIP5_TTI_ACTION_TABLE_FIELDS_MAP_DSCP_E),
    PRV_CPSS_FIELD_MAC( 160,   3, SIP5_TTI_ACTION_TABLE_FIELDS_UP0_E),
    PRV_CPSS_FIELD_MAC( 163,   2, SIP5_TTI_ACTION_TABLE_FIELDS_UP1_COMMAND_E),
    PRV_CPSS_FIELD_MAC( 165,   3, SIP5_TTI_ACTION_TABLE_FIELDS_UP1_E),
    PRV_CPSS_FIELD_MAC( 168,   2, SIP5_TTI_ACTION_TABLE_FIELDS_PASSENGER_PACKET_TYPE_E),
    PRV_CPSS_FIELD_MAC( 170,   1, SIP5_TTI_ACTION_TABLE_FIELDS_COPY_TTL_E),
    PRV_CPSS_FIELD_MAC( 171,   1, SIP5_TTI_ACTION_TABLE_FIELDS_TUNNEL_TERMINATION_E),
    PRV_CPSS_FIELD_MAC( 172,   3, SIP5_TTI_ACTION_TABLE_FIELDS_MPLS_COMMAND_E),
    PRV_CPSS_FIELD_MAC( 175,   4, SIP5_TTI_ACTION_TABLE_FIELDS_HASH_MASK_INDEX_E),
    PRV_CPSS_FIELD_MAC( 179,   4, SIP5_TTI_ACTION_TABLE_FIELDS_TRUST_QOS_MAPPING_TABLE_INDEX_E),
    PRV_CPSS_FIELD_MAC( 183,   1, SIP5_TTI_ACTION_TABLE_FIELDS_L_LSP_QOS_PROFILE_ENABLE_E),
    PRV_CPSS_FIELD_MAC( 184,   8, SIP5_TTI_ACTION_TABLE_FIELDS_TTL_E),
    PRV_CPSS_FIELD_MAC( 192,   1, SIP5_TTI_ACTION_TABLE_FIELDS_ENABLE_DEC_TTL_E),
    PRV_CPSS_FIELD_MAC( 193,   1, SIP5_TTI_ACTION_TABLE_FIELDS_SOURCE_EPORT_ASSIGNMENT_ENABLE_E),
    PRV_CPSS_FIELD_MAC( 194,  14, SIP5_TTI_ACTION_TABLE_FIELDS_SOURCE_EPORT_E),
    PRV_CPSS_FIELD_MAC( 208,   6, SIP5_TTI_ACTION_TABLE_FIELDS_TT_HEADER_LENGTH_E),
    PRV_CPSS_FIELD_MAC( 214,   1, SIP5_TTI_ACTION_TABLE_FIELDS_IS_PTP_PACKET_E),
    PRV_CPSS_FIELD_MAC( 215,   1, SIP5_TTI_ACTION_TABLE_FIELDS_NON_PTP_PACKET_TIMESTAMP_ENABLE_E),
    PRV_CPSS_FIELD_MAC( 216,   7, SIP5_TTI_ACTION_TABLE_FIELDS_NON_PTP_PACKET_OFFSET_INDEX_E),
    PRV_CPSS_FIELD_MAC( 215,   2, SIP5_TTI_ACTION_TABLE_FIELDS_PTP_PACKET_PTP_TRIGGER_TYPE_E),
    PRV_CPSS_FIELD_MAC( 217,   6, SIP5_TTI_ACTION_TABLE_FIELDS_PTP_PACKET_PTP_OFFSET_E),
    PRV_CPSS_FIELD_MAC( 223,   1, SIP5_TTI_ACTION_TABLE_FIELDS_ENABLE_OAM_PROCESSING_WHEN_GAL_OR_OAL_EXISTS_E),
    PRV_CPSS_FIELD_MAC( 224,   1, SIP5_TTI_ACTION_TABLE_FIELDS_OAM_PROCESSING_ENABLE_E),
    PRV_CPSS_FIELD_MAC( 225,   1, SIP5_TTI_ACTION_TABLE_FIELDS_CW_BASED_PSEUDO_WIRE_E),
    PRV_CPSS_FIELD_MAC( 226,   1, SIP5_TTI_ACTION_TABLE_FIELDS_TTL_EXPIRY_VCCV_ENABLE_E),
    PRV_CPSS_FIELD_MAC( 227,   1, SIP5_TTI_ACTION_TABLE_FIELDS_PWE3_FLOW_LABEL_EXIST_E),
    PRV_CPSS_FIELD_MAC( 228,   1, SIP5_TTI_ACTION_TABLE_FIELDS_PW_CW_BASED_E_TREE_ENABLE_E),
    PRV_CPSS_FIELD_MAC( 229,   1, SIP5_TTI_ACTION_TABLE_FIELDS_UP_CFI_TO_QOS_TABLE_SELECT_MODE_E),
    PRV_CPSS_FIELD_MAC( 230,   1, SIP5_TTI_ACTION_TABLE_FIELDS_RX_IS_PROTECTION_PATH_E),
    PRV_CPSS_FIELD_MAC( 231,   1, SIP5_TTI_ACTION_TABLE_FIELDS_RX_ENABLE_PROTECTION_SWITCHING_E),
    PRV_CPSS_FIELD_MAC( 232,   1, SIP5_TTI_ACTION_TABLE_FIELDS_SET_MAC2ME_E),
    PRV_CPSS_FIELD_MAC( 233,   1, SIP5_TTI_ACTION_TABLE_FIELDS_OAM_PROFILE_E),
    PRV_CPSS_FIELD_MAC( 234,   1, SIP5_TTI_ACTION_TABLE_FIELDS_APPLY_NON_DATA_CW_COMMAND_E),
    PRV_CPSS_FIELD_MAC( 235,   2, SIP5_TTI_ACTION_TABLE_FIELDS_PW_TAG_MODE_E),
    PRV_CPSS_FIELD_MAC( 237,   1, SIP5_TTI_ACTION_TABLE_FIELDS_ENABLE_TTI_1_LOOKUP_E),
    PRV_CPSS_FIELD_MAC( 238,   1, SIP5_TTI_ACTION_TABLE_FIELDS_CHANNEL_TYPE_TO_OPCODE_MAPPING_EN_E),
    PRV_CPSS_FIELD_MAC( 239,   2, SIP5_TTI_ACTION_TABLE_FIELDS_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_ENABLE_E),
    PRV_CPSS_FIELD_MAC( 241,   1, SIP5_TTI_ACTION_TABLE_FIELDS_PASSENGER_PARSING_OF_NON_MPLS_TRANSIT_TUNNEL_ENABLE_E),
    PRV_CPSS_FIELD_MAC( 242,   1, SIP6_TTI_ACTION_TABLE_FIELDS_SKIP_FDB_SA_LOOKUP_ENABLE_E),
    PRV_CPSS_FIELD_MAC( 243,   1, SIP6_TTI_ACTION_TABLE_FIELDS_IPV6_SEGMENT_ROUTING_END_NODE_ENABLE_E),
    PRV_CPSS_FIELD_MAC( 244,   1, SIP6_TTI_ACTION_TABLE_FIELDS_TUNNEL_HEADER_START_L4_ENABLE_E),
    PRV_CPSS_FIELD_MAC( 245,   1, SIP6_TTI_ACTION_TABLE_FIELDS_RESERVED_E),
    PRV_CPSS_FIELD_MAC( 246,   1, SIP6_TTI_ACTION_TABLE_FIELDS_TTI_OVER_EXACT_MATCH_ENABLE_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_10_TTI_ACTION_TABLE_FIELDS_TRIGER_CNC_HASH_CLIENT_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_10_TTI_ACTION_TABLE_FIELDS_IPFIX_ENABLE_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_10_TTI_ACTION_TABLE_FIELDS_PPU_PROFILE_E)
};

static const PRV_CPSS_ENTRY_FORMAT_TABLE_STC prvCpssDxChSip5L2iIngressVlanTableFieldsFormat[SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS___LAST_VALUE___E] =
{
    PRV_CPSS_FIELD_MAC(   0,   1, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_VALID_E),
    PRV_CPSS_FIELD_MAC(   1,   1, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_NEW_SRC_ADDR_IS_NOT_SECURITY_BREACH_E),
    PRV_CPSS_FIELD_MAC(   2,   3, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_UNREGISTERED_NON_IP_MULTICAST_CMD_E),
    PRV_CPSS_FIELD_MAC(   5,   3, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_UNREGISTERED_IPV4_MULTICAST_CMD_E),
    PRV_CPSS_FIELD_MAC(   8,   3, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_UNREGISTERED_IPV6_MULTICAST_CMD_E),
    PRV_CPSS_FIELD_MAC(  11,   3, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_UNKNOWN_UNICAST_CMD_E),
    PRV_CPSS_FIELD_MAC(  14,   1, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV4_IGMP_TO_CPU_EN_E),
    PRV_CPSS_FIELD_MAC(  15,   1, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV4_IPM_BRIDGING_MODE_E),
    PRV_CPSS_FIELD_MAC(  16,   1, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV6_IPM_BRIDGING_MODE_E),
    PRV_CPSS_FIELD_MAC(  17,   3, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_MIRROR_TO_INGRESS_ANALYZER_E),
    PRV_CPSS_FIELD_MAC(  20,   1, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV6_ICMP_TO_CPU_EN_E),
    PRV_CPSS_FIELD_MAC(  21,   1, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV4_CONTROL_TO_CPU_EN_E),
    PRV_CPSS_FIELD_MAC(  22,   1, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV4_IPM_BRIDGING_EN_E),
    PRV_CPSS_FIELD_MAC(  23,   1, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV6_IPM_BRIDGING_EN_E),
    PRV_CPSS_FIELD_MAC(  24,   3, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_UNREGISTERED_IPV4_BC_CMD_E),
    PRV_CPSS_FIELD_MAC(  27,   3, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_UNREGISTERED_NON_IPV4_BC_CMD_E),
    PRV_CPSS_FIELD_MAC(  30,   1, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV4_UNICAST_ROUTE_EN_E),
    PRV_CPSS_FIELD_MAC(  31,   1, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV4_MULTICAST_ROUTE_EN_E),
    PRV_CPSS_FIELD_MAC(  32,   1, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV6_UNICAST_ROUTE_EN_E),
    PRV_CPSS_FIELD_MAC(  33,   1, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV6_MULTICAST_ROUTE_EN_E),
    PRV_CPSS_FIELD_MAC(  34,   1, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV6_SITEID_E),
    PRV_CPSS_FIELD_MAC(  35,   1, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_AUTO_LEARN_DIS_E),
    PRV_CPSS_FIELD_MAC(  36,   1, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_NA_MSG_TO_CPU_EN_E),
    PRV_CPSS_FIELD_MAC(  37,   3, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_MRU_INDEX_E),
    PRV_CPSS_FIELD_MAC(  40,   1, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_BC_UDP_TRAP_MIRROR_EN_E),
    PRV_CPSS_FIELD_MAC(  41,   1, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV6_CONTROL_TO_CPU_EN_E),
    PRV_CPSS_FIELD_MAC(  42,  16, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_FLOOD_EVIDX_E),
    PRV_CPSS_FIELD_MAC(  58,  12, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_VRF_ID_E),
    PRV_CPSS_FIELD_MAC(  70,   1, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_UC_LOCAL_EN_E),
    PRV_CPSS_FIELD_MAC(  71,   1, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_FLOOD_VIDX_MODE_E),
    PRV_CPSS_FIELD_MAC(  72,   3, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV4_MC_BC_TO_MIRROR_ANLYZER_IDX_E),
    PRV_CPSS_FIELD_MAC(  75,   3, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV6_MC_TO_MIRROR_ANALYZER_IDX_E),
    PRV_CPSS_FIELD_MAC(  78,  13, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_FID_E),
    PRV_CPSS_FIELD_MAC(  91,   3, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_UNKOWN_MAC_SA_CMD_E),
    PRV_CPSS_FIELD_MAC(  94,   1, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_FCOE_FORWARDING_EN_E),
    PRV_CPSS_FIELD_MAC(  95,   2, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_UNREG_IPM_EVIDX_MODE_E),
    PRV_CPSS_FIELD_MAC(  97,  16, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_UNREG_IPM_EVIDX_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_FDB_LOOKUP_KEY_MODE_E)
};

static const PRV_CPSS_ENTRY_FORMAT_TABLE_STC prvCpssDxChSip5L2iEPortTableFieldsFormat[SIP5_L2I_EPORT_TABLE_FIELDS___LAST_VALUE___E] =
{
    PRV_CPSS_FIELD_MAC(   0,   1, SIP5_L2I_EPORT_TABLE_FIELDS_NA_MSG_TO_CPU_EN_E),
    PRV_CPSS_FIELD_MAC(   1,   1, SIP5_L2I_EPORT_TABLE_FIELDS_AUTO_LEARN_DIS_E),
    PRV_CPSS_FIELD_MAC(   2,   1, SIP5_L2I_EPORT_TABLE_FIELDS_NA_STORM_PREV_EN_E),
    PRV_CPSS_FIELD_MAC(   3,   1, SIP5_L2I_EPORT_TABLE_FIELDS_NEW_SRC_ADDR_SECURITY_BREACH_E),
    PRV_CPSS_FIELD_MAC(   4,   1, SIP5_L2I_EPORT_TABLE_FIELDS_VLAN_INGRESS_FILTERING_E),
    PRV_CPSS_FIELD_MAC(   5,   2, SIP5_L2I_EPORT_TABLE_FIELDS_ACCEPT_FRAME_TYPE_E),
    PRV_CPSS_FIELD_MAC(   7,   1, SIP5_L2I_EPORT_TABLE_FIELDS_UC_LOCAL_CMD_E),
    PRV_CPSS_FIELD_MAC(   8,   3, SIP5_L2I_EPORT_TABLE_FIELDS_UNKNOWN_SRC_ADDR_CMD_E),
    PRV_CPSS_FIELD_MAC(  11,   1, SIP5_L2I_EPORT_TABLE_FIELDS_PORT_PVLAN_EN_E),
    PRV_CPSS_FIELD_MAC(  12,   1, SIP5_L2I_EPORT_TABLE_FIELDS_PORT_VLAN_IS_TRUNK_E),
    PRV_CPSS_FIELD_MAC(  13,  13, SIP5_L2I_EPORT_TABLE_FIELDS_PORT_PVLAN_TRG_EPORT_TRUNK_NUM_E),
    PRV_CPSS_FIELD_MAC(  26,  10, SIP5_L2I_EPORT_TABLE_FIELDS_PORT_PVLAN_TRG_DEV_E),
    PRV_CPSS_FIELD_MAC(  36,   1, SIP5_L2I_EPORT_TABLE_FIELDS_ALL_PKT_TO_PVLAN_UPLINK_EN_E),
    PRV_CPSS_FIELD_MAC(  37,   1, SIP5_L2I_EPORT_TABLE_FIELDS_IGMP_TRAP_EN_E),
    PRV_CPSS_FIELD_MAC(  38,   1, SIP5_L2I_EPORT_TABLE_FIELDS_ARP_BC_TRAP_EN_E),
    PRV_CPSS_FIELD_MAC(  39,   1, SIP5_L2I_EPORT_TABLE_FIELDS_EN_LEARN_ON_TRAP_IEEE_RSRV_MC_E),
    PRV_CPSS_FIELD_MAC(  40,   3, SIP5_L2I_EPORT_TABLE_FIELDS_IEEE_RSVD_MC_TABLE_SEL_E),
    PRV_CPSS_FIELD_MAC(  43,   1, SIP5_L2I_EPORT_TABLE_FIELDS_PORT_SOURCE_ID_FORCE_MODE_E),
    PRV_CPSS_FIELD_MAC(  44,  12, SIP5_L2I_EPORT_TABLE_FIELDS_SRC_ID_E),
    PRV_CPSS_FIELD_MAC(  56,   1, SIP5_L2I_EPORT_TABLE_FIELDS_ARP_MAC_SA_MIS_DROP_EN_E),
    PRV_CPSS_FIELD_MAC(  57,   3, SIP5_L2I_EPORT_TABLE_FIELDS_INGRESS_PORT_UNKNOWN_UC_FILTER_CMD_E),
    PRV_CPSS_FIELD_MAC(  60,   3, SIP5_L2I_EPORT_TABLE_FIELDS_INGRESS_PORT_UNREG_MC_FILTER_CMD_E),
    PRV_CPSS_FIELD_MAC(  63,   3, SIP5_L2I_EPORT_TABLE_FIELDS_INGRESS_PORT_BC_FILTER_CMD_E),
    PRV_CPSS_FIELD_MAC(  66,   1, SIP5_L2I_EPORT_TABLE_FIELDS_INGRESS_EPORT_STP_STATE_MODE_E),
    PRV_CPSS_FIELD_MAC(  67,   2, SIP5_L2I_EPORT_TABLE_FIELDS_INGRESS_EPORT_SPANNING_TREE_STATE_E),
    PRV_CPSS_FIELD_MAC(  69,   1, SIP5_L2I_EPORT_TABLE_FIELDS_IPV4_CONTROL_TRAP_EN_E),
    PRV_CPSS_FIELD_MAC(  70,   1, SIP5_L2I_EPORT_TABLE_FIELDS_IPV6_CONTROL_TRAP_EN_E),
    PRV_CPSS_FIELD_MAC(  71,   1, SIP5_L2I_EPORT_TABLE_FIELDS_BC_UDP_TRAP_OR_MIRROR_EN_E),
    PRV_CPSS_FIELD_MAC(  72,   1, SIP5_L2I_EPORT_TABLE_FIELDS_EN_LEARN_ORIG_TAG1_VID_E),
    PRV_CPSS_FIELD_MAC(  73,   1, SIP5_L2I_EPORT_TABLE_FIELDS_FDB_UC_IPV4_ROUTING_EN_E),
    PRV_CPSS_FIELD_MAC(  74,   1, SIP5_L2I_EPORT_TABLE_FIELDS_FDB_UC_IPV6_ROUTING_EN_E),
    PRV_CPSS_FIELD_MAC(  75,   1, SIP5_L2I_EPORT_TABLE_FIELDS_FDB_FCOE_ROUTING_EN_E),
    PRV_CPSS_FIELD_MAC(  76,   3, SIP5_L2I_EPORT_TABLE_FIELDS_MOVED_MAC_SA_CMD_E)
};

/*static*/ const PRV_CPSS_ENTRY_FORMAT_TABLE_STC prvCpssDxChSip5FdbFdbTableFieldsFormat[SIP5_10_FDB_FDB_TABLE_FIELDS___LAST_VALUE___E] =
{
    PRV_CPSS_FIELD_MAC(   0,   1, SIP5_FDB_FDB_TABLE_FIELDS_VALID_E),
    PRV_CPSS_FIELD_MAC(   1,   1, SIP5_FDB_FDB_TABLE_FIELDS_SKIP_E),
    PRV_CPSS_FIELD_MAC(   2,   1, SIP5_FDB_FDB_TABLE_FIELDS_AGE_E),
    PRV_CPSS_FIELD_MAC(   3,   3, SIP5_FDB_FDB_TABLE_FIELDS_FDB_ENTRY_TYPE_E),
    PRV_CPSS_FIELD_MAC(   6,  13, SIP5_FDB_FDB_TABLE_FIELDS_FID_E),
    PRV_CPSS_FIELD_MAC(  19,  48, SIP5_FDB_FDB_TABLE_FIELDS_MAC_ADDR_E),
    PRV_CPSS_FIELD_MAC(  67,  10, SIP5_FDB_FDB_TABLE_FIELDS_DEV_ID_E),
    PRV_CPSS_FIELD_MAC(  77,   6, SIP5_FDB_FDB_TABLE_FIELDS_SOURCE_ID_5_0_E),
    PRV_CPSS_FIELD_MAC(  83,   1, SIP5_FDB_FDB_TABLE_FIELDS_IS_TRUNK_E),
    PRV_CPSS_FIELD_MAC(  19,  32, SIP5_FDB_FDB_TABLE_FIELDS_DIP_E),
    PRV_CPSS_FIELD_MAC(  51,  32, SIP5_FDB_FDB_TABLE_FIELDS_SIP_E),
    PRV_CPSS_FIELD_MAC(  83,  16, SIP5_FDB_FDB_TABLE_FIELDS_VIDX_E),
    PRV_CPSS_FIELD_MAC(  84,  12, SIP5_FDB_FDB_TABLE_FIELDS_TRUNK_NUM_E),
    PRV_CPSS_FIELD_MAC(  84,  13, SIP5_FDB_FDB_TABLE_FIELDS_EPORT_NUM_E),
    PRV_CPSS_FIELD_MAC(  99,   8, SIP5_FDB_FDB_TABLE_FIELDS_USER_DEFINED_E),
    PRV_CPSS_FIELD_MAC(  99,   5, SIP5_FDB_FDB_TABLE_FIELDS_RESERVED_99_103_E),
    PRV_CPSS_FIELD_MAC( 104,   3, SIP5_FDB_FDB_TABLE_FIELDS_SOURCE_ID_11_9_E),
    PRV_CPSS_FIELD_MAC( 107,   3, SIP5_FDB_FDB_TABLE_FIELDS_DA_ACCESS_LEVEL_E),
    PRV_CPSS_FIELD_MAC( 110,   3, SIP5_FDB_FDB_TABLE_FIELDS_SA_ACCESS_LEVEL_E),
    PRV_CPSS_FIELD_MAC( 113,   3, SIP5_FDB_FDB_TABLE_FIELDS_SOURCE_ID_8_6_E),
    PRV_CPSS_FIELD_MAC( 116,   3, SIP5_FDB_FDB_TABLE_FIELDS_RESERVED_116_118_E),
    PRV_CPSS_FIELD_MAC( 107,  12, SIP5_FDB_FDB_TABLE_FIELDS_ORIG_VID1_E),
    PRV_CPSS_FIELD_MAC( 119,   1, SIP5_FDB_FDB_TABLE_FIELDS_IS_STATIC_E),
    PRV_CPSS_FIELD_MAC( 120,   1, SIP5_FDB_FDB_TABLE_FIELDS_MULTIPLE_E),
    PRV_CPSS_FIELD_MAC( 121,   3, SIP5_FDB_FDB_TABLE_FIELDS_DA_CMD_E),
    PRV_CPSS_FIELD_MAC( 124,   3, SIP5_FDB_FDB_TABLE_FIELDS_SA_CMD_E),
    PRV_CPSS_FIELD_MAC( 127,   1, SIP5_FDB_FDB_TABLE_FIELDS_DA_ROUTE_E),
    PRV_CPSS_FIELD_MAC( 128,   1, SIP5_FDB_FDB_TABLE_FIELDS_SP_UNKNOWN_E),
    PRV_CPSS_FIELD_MAC( 129,   3, SIP5_FDB_FDB_TABLE_FIELDS_SA_QOS_PARAM_SET_IDX_E),
    PRV_CPSS_FIELD_MAC( 132,   3, SIP5_FDB_FDB_TABLE_FIELDS_DA_QOS_PARAM_SET_IDX_E),
    PRV_CPSS_FIELD_MAC( 135,   1, SIP5_FDB_FDB_TABLE_FIELDS_APP_SPECIFIC_CPU_CODE_E),
    PRV_CPSS_FIELD_MAC( 136,   1, SIP5_FDB_FDB_TABLE_FIELDS_SA_LOOKUP_INGRESS_MIRROR_TO_ANALYZER_E),
    PRV_CPSS_FIELD_MAC( 137,   1, SIP5_FDB_FDB_TABLE_FIELDS_DA_LOOKUP_INGRESS_MIRROR_TO_ANALYZER_E),
    PRV_CPSS_FIELD_MAC(   6,  12, SIP5_FDB_FDB_TABLE_FIELDS_VRF_ID_E),
    PRV_CPSS_FIELD_MAC(  18,   1, SIP5_FDB_FDB_TABLE_FIELDS_IPV6_SCOPE_CHECK_E),
    PRV_CPSS_FIELD_MAC(  18,  24, SIP5_FDB_FDB_TABLE_FIELDS_FCOE_D_ID_E),
    PRV_CPSS_FIELD_MAC(  18,  32, SIP5_FDB_FDB_TABLE_FIELDS_IPV4_DIP_E),
    PRV_CPSS_FIELD_MAC(  19,   1, SIP5_FDB_FDB_TABLE_FIELDS_IPV6_DST_SITE_ID_E),
    PRV_CPSS_FIELD_MAC(  50,   1, SIP5_FDB_FDB_TABLE_FIELDS_DEC_TTL_OR_HOP_COUNT_E),
    PRV_CPSS_FIELD_MAC(  51,   1, SIP5_FDB_FDB_TABLE_FIELDS_BYPASS_TTL_OPTIONS_OR_HOP_EXTENSION_E),
    PRV_CPSS_FIELD_MAC(  52,   3, SIP5_FDB_FDB_TABLE_FIELDS_INGRESS_MIRROR_TO_ANALYZER_INDEX_E),
    PRV_CPSS_FIELD_MAC(  55,   1, SIP5_FDB_FDB_TABLE_FIELDS_QOS_PROFILE_MARKING_EN_E),
    PRV_CPSS_FIELD_MAC(  56,   7, SIP5_FDB_FDB_TABLE_FIELDS_QOS_PROFILE_INDEX_E),
    PRV_CPSS_FIELD_MAC(  63,   1, SIP5_FDB_FDB_TABLE_FIELDS_QOS_PROFILE_PRECEDENCE_E),
    PRV_CPSS_FIELD_MAC(  64,   2, SIP5_FDB_FDB_TABLE_FIELDS_MODIFY_UP_E),
    PRV_CPSS_FIELD_MAC(  66,   2, SIP5_FDB_FDB_TABLE_FIELDS_MODIFY_DSCP_E),
    PRV_CPSS_FIELD_MAC(  68,   3, SIP5_FDB_FDB_TABLE_FIELDS_COUNTER_SET_INDEX_E),
    PRV_CPSS_FIELD_MAC(  71,   1, SIP5_FDB_FDB_TABLE_FIELDS_ARP_BC_TRAP_MIRROR_EN_E),
    PRV_CPSS_FIELD_MAC(  72,   6, SIP5_FDB_FDB_TABLE_FIELDS_DIP_ACCESS_LEVEL_E),
    PRV_CPSS_FIELD_MAC(  78,   1, SIP5_FDB_FDB_TABLE_FIELDS_ICMP_REDIRECT_EXCEP_MIRROR_EN_E),
    PRV_CPSS_FIELD_MAC(  79,   3, SIP5_FDB_FDB_TABLE_FIELDS_MTU_INDEX_E),
    PRV_CPSS_FIELD_MAC(  82,   1, SIP5_FDB_FDB_TABLE_FIELDS_USE_VIDX_E),
    PRV_CPSS_FIELD_MAC(  97,  10, SIP5_FDB_FDB_TABLE_FIELDS_TARGET_DEVICE_E),
    PRV_CPSS_FIELD_MAC( 107,  13, SIP5_FDB_FDB_TABLE_FIELDS_NEXT_HOP_EVLAN_E),
    PRV_CPSS_FIELD_MAC( 120,   1, SIP5_FDB_FDB_TABLE_FIELDS_TUNNEL_START_E),
    PRV_CPSS_FIELD_MAC( 121,   1, SIP5_FDB_FDB_TABLE_FIELDS_TUNNEL_TYPE_E),
    PRV_CPSS_FIELD_MAC( 121,  17, SIP5_FDB_FDB_TABLE_FIELDS_ARP_PTR_E),
    PRV_CPSS_FIELD_MAC( 122,  15, SIP5_FDB_FDB_TABLE_FIELDS_TUNNEL_PTR_E),
    PRV_CPSS_FIELD_MAC(   6,  32, SIP5_FDB_FDB_TABLE_FIELDS_IPV6_DIP_0_E),
    PRV_CPSS_FIELD_MAC(  38,  32, SIP5_FDB_FDB_TABLE_FIELDS_IPV6_DIP_1_E),
    PRV_CPSS_FIELD_MAC(  70,  32, SIP5_FDB_FDB_TABLE_FIELDS_IPV6_DIP_2_E),
    PRV_CPSS_FIELD_MAC( 102,  32, SIP5_FDB_FDB_TABLE_FIELDS_IPV6_DIP_3_E),
    PRV_CPSS_FIELD_MAC( 134,   4, SIP5_FDB_FDB_TABLE_FIELDS_NH_DATA_BANK_NUM_E),
    PRV_CPSS_FIELD_MAC(  31,   1, SIP5_10_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TS_IS_NAT_E)
};

static const PRV_CPSS_ENTRY_FORMAT_TABLE_STC prvCpssDxChSip5FdbAuMsgTableFieldsFormat[SIP5_10_FDB_AU_MSG_TABLE_FIELDS___LAST_VALUE___E] =
{
    PRV_CPSS_FIELD_MAC(   0,   1, SIP5_FDB_AU_MSG_TABLE_FIELDS_MESSAGE_ID_E),
    PRV_CPSS_FIELD_MAC(   1,   3, SIP5_FDB_AU_MSG_TABLE_FIELDS_MSG_TYPE_E),
    PRV_CPSS_FIELD_MAC(   4,   3, SIP5_FDB_AU_MSG_TABLE_FIELDS_FDB_ENTRY_TYPE_E),
    PRV_CPSS_FIELD_MAC(   7,   1, SIP5_FDB_AU_MSG_TABLE_FIELDS_VALID_E),
    PRV_CPSS_FIELD_MAC(   8,   1, SIP5_FDB_AU_MSG_TABLE_FIELDS_SKIP_E),
    PRV_CPSS_FIELD_MAC(   9,   1, SIP5_FDB_AU_MSG_TABLE_FIELDS_AGE_E),
    PRV_CPSS_FIELD_MAC(  10,   9, SIP5_FDB_AU_MSG_TABLE_FIELDS_MAC_ADDR_INDEX_8_0_E),
    PRV_CPSS_FIELD_MAC(  10,   1, SIP5_FDB_AU_MSG_TABLE_FIELDS_CHAIN_TOO_LONG_E),
    PRV_CPSS_FIELD_MAC(  11,   5, SIP5_FDB_AU_MSG_TABLE_FIELDS_MAC_ADDR_OFFSET_E),
    PRV_CPSS_FIELD_MAC(  16,   1, SIP5_FDB_AU_MSG_TABLE_FIELDS_ENTRY_FOUND_E),
    PRV_CPSS_FIELD_MAC(  19,  48, SIP5_FDB_AU_MSG_TABLE_FIELDS_MAC_ADDR_E),
    PRV_CPSS_FIELD_MAC(  67,  10, SIP5_FDB_AU_MSG_TABLE_FIELDS_DEV_ID_E),
    PRV_CPSS_FIELD_MAC(  77,   6, SIP5_FDB_AU_MSG_TABLE_FIELDS_SOURCE_ID_5_0_E),
    PRV_CPSS_FIELD_MAC(  19,  32, SIP5_FDB_AU_MSG_TABLE_FIELDS_DIP_E),
    PRV_CPSS_FIELD_MAC(  51,  32, SIP5_FDB_AU_MSG_TABLE_FIELDS_SIP_E),
    PRV_CPSS_FIELD_MAC(  83,  13, SIP5_FDB_AU_MSG_TABLE_FIELDS_FID_E),
    PRV_CPSS_FIELD_MAC(  96,  16, SIP5_FDB_AU_MSG_TABLE_FIELDS_VIDX_E),
    PRV_CPSS_FIELD_MAC(  96,   1, SIP5_FDB_AU_MSG_TABLE_FIELDS_IS_TRUNK_E),
    PRV_CPSS_FIELD_MAC(  97,  13, SIP5_FDB_AU_MSG_TABLE_FIELDS_EPORT_NUM_E),
    PRV_CPSS_FIELD_MAC(  97,  12, SIP5_FDB_AU_MSG_TABLE_FIELDS_TRUNK_NUM_E),
    PRV_CPSS_FIELD_MAC( 112,   8, SIP5_FDB_AU_MSG_TABLE_FIELDS_USER_DEFINED_E),
    PRV_CPSS_FIELD_MAC( 112,   5, SIP5_FDB_AU_MSG_TABLE_FIELDS_RESERVED_109_113_E),
    PRV_CPSS_FIELD_MAC( 117,   3, SIP5_FDB_AU_MSG_TABLE_FIELDS_SOURCE_ID_11_9_E),
    PRV_CPSS_FIELD_MAC( 120,  12, SIP5_FDB_AU_MSG_TABLE_FIELDS_ORIG_VID1_E),
    PRV_CPSS_FIELD_MAC( 120,   3, SIP5_FDB_AU_MSG_TABLE_FIELDS_DA_ACCESS_LEVEL_E),
    PRV_CPSS_FIELD_MAC( 123,   3, SIP5_FDB_AU_MSG_TABLE_FIELDS_SA_ACCESS_LEVEL_E),
    PRV_CPSS_FIELD_MAC( 126,   3, SIP5_FDB_AU_MSG_TABLE_FIELDS_SOURCE_ID_8_6_E),
    PRV_CPSS_FIELD_MAC( 132,   1, SIP5_FDB_AU_MSG_TABLE_FIELDS_APP_SPECIFIC_CPU_CODE_E),
    PRV_CPSS_FIELD_MAC( 133,   1, SIP5_FDB_AU_MSG_TABLE_FIELDS_SP_UNKNOWN_E),
    PRV_CPSS_FIELD_MAC( 134,   1, SIP5_FDB_AU_MSG_TABLE_FIELDS_SEARCH_TYPE_E),
    PRV_CPSS_FIELD_MAC( 135,  12, SIP5_FDB_AU_MSG_TABLE_FIELDS_MAC_ADDR_INDEX_20_9_E),
    PRV_CPSS_FIELD_MAC( 147,   1, SIP5_FDB_AU_MSG_TABLE_FIELDS_MULTIPLE_E),
    PRV_CPSS_FIELD_MAC( 148,   1, SIP5_FDB_AU_MSG_TABLE_FIELDS_DA_ROUTE_E),
    PRV_CPSS_FIELD_MAC( 149,   3, SIP5_FDB_AU_MSG_TABLE_FIELDS_SA_QOS_PARAM_SET_IDX_E),
    PRV_CPSS_FIELD_MAC( 152,   3, SIP5_FDB_AU_MSG_TABLE_FIELDS_DA_QOS_PARAM_SET_IDX_E),
    PRV_CPSS_FIELD_MAC( 155,   1, SIP5_FDB_AU_MSG_TABLE_FIELDS_IS_STATIC_E),
    PRV_CPSS_FIELD_MAC( 156,   3, SIP5_FDB_AU_MSG_TABLE_FIELDS_DA_CMD_E),
    PRV_CPSS_FIELD_MAC( 159,   3, SIP5_FDB_AU_MSG_TABLE_FIELDS_SA_CMD_E),
    PRV_CPSS_FIELD_MAC( 162,   1, SIP5_FDB_AU_MSG_TABLE_FIELDS_SA_LOOKUP_INGRESS_MIRROR_TO_ANALYZER_E),
    PRV_CPSS_FIELD_MAC( 163,   1, SIP5_FDB_AU_MSG_TABLE_FIELDS_DA_LOOKUP_INGRESS_MIRROR_TO_ANALYZER_E),
    PRV_CPSS_FIELD_MAC( 164,   1, SIP5_FDB_AU_MSG_TABLE_FIELDS_FDB_LOOKUP_KEY_MODE_E),
    PRV_CPSS_FIELD_MAC(  16,   3, SIP5_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_UP0_E),
    PRV_CPSS_FIELD_MAC( 135,   1, SIP5_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_IS_MOVED_E),
    PRV_CPSS_FIELD_MAC( 136,   1, SIP5_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_OLD_IS_TRUNK_E),
    PRV_CPSS_FIELD_MAC( 137,  13, SIP5_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_OLD_EPORT_E),
    PRV_CPSS_FIELD_MAC( 137,  12, SIP5_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_OLD_TRUNK_NUM_E),
    PRV_CPSS_FIELD_MAC( 152,  10, SIP5_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_OLD_DEVICE_E),
    PRV_CPSS_FIELD_MAC( 162,  12, SIP5_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_OLD_SRC_ID_E),
    PRV_CPSS_FIELD_MAC(  10,  21, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_MAC_ADDR_INDEX_E),
    PRV_CPSS_FIELD_MAC(  31,  12, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_VRF_ID_E),
    PRV_CPSS_FIELD_MAC(  43,   1, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IPV6_SCOPE_CHECK_E),
    PRV_CPSS_FIELD_MAC(  43,  24, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_FCOE_D_ID_E),
    PRV_CPSS_FIELD_MAC(  43,  32, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IPV4_DIP_E),
    PRV_CPSS_FIELD_MAC(  44,   1, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IPV6_DST_SITE_ID_E),
    PRV_CPSS_FIELD_MAC(  75,   1, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_DEC_TTL_OR_HOP_COUNT_E),
    PRV_CPSS_FIELD_MAC(  76,   1, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_BYPASS_TTL_OPTIONS_OR_HOP_EXTENSION_E),
    PRV_CPSS_FIELD_MAC(  77,   3, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_INGRESS_MIRROR_TO_ANALYZER_INDEX_E),
    PRV_CPSS_FIELD_MAC(  80,   1, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_QOS_PROFILE_MARKING_EN_E),
    PRV_CPSS_FIELD_MAC(  81,   7, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_QOS_PROFILE_INDEX_E),
    PRV_CPSS_FIELD_MAC(  88,   1, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_QOS_PROFILE_PRECEDENCE_E),
    PRV_CPSS_FIELD_MAC(  89,   2, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_MODIFY_UP_E),
    PRV_CPSS_FIELD_MAC(  91,   2, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_MODIFY_DSCP_E),
    PRV_CPSS_FIELD_MAC(  93,   3, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_COUNTER_SET_INDEX_E),
    PRV_CPSS_FIELD_MAC(  96,   1, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_ARP_BC_TRAP_MIRROR_EN_E),
    PRV_CPSS_FIELD_MAC(  97,   6, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_DIP_ACCESS_LEVEL_E),
    PRV_CPSS_FIELD_MAC( 103,   1, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_ICMP_REDIRECT_EXCEP_MIRROR_EN_E),
    PRV_CPSS_FIELD_MAC( 104,   3, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_MTU_INDEX_E),
    PRV_CPSS_FIELD_MAC( 107,   1, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_USE_VIDX_E),
    PRV_CPSS_FIELD_MAC( 108,   1, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IS_TRUNK_E),
    PRV_CPSS_FIELD_MAC( 108,  16, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_EVIDX_E),
    PRV_CPSS_FIELD_MAC( 109,  10, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_TARGET_DEVICE_E),
    PRV_CPSS_FIELD_MAC( 109,  12, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_TRUNK_NUM_E),
    PRV_CPSS_FIELD_MAC( 119,  13, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_EPORT_NUM_E),
    PRV_CPSS_FIELD_MAC( 132,  13, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_NEXT_HOP_EVLAN_E),
    PRV_CPSS_FIELD_MAC( 145,   1, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_TUNNEL_START_E),
    PRV_CPSS_FIELD_MAC( 146,   1, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_TUNNEL_TYPE_E),
    PRV_CPSS_FIELD_MAC( 146,  17, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_ARP_PTR_E),
    PRV_CPSS_FIELD_MAC( 147,  15, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_TUNNEL_PTR_E),
    PRV_CPSS_FIELD_MAC(  31,  32, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_0_E),
    PRV_CPSS_FIELD_MAC(  63,  32, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_1_E),
    PRV_CPSS_FIELD_MAC(  95,  32, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_2_E),
    PRV_CPSS_FIELD_MAC( 127,  32, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_3_E),
    PRV_CPSS_FIELD_MAC( 159,   4, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_NH_DATA_BANK_NUM_E),
    PRV_CPSS_FIELD_MAC( 162,   1, SIP5_10_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_TS_IS_NAT_E)
};

static const PRV_CPSS_ENTRY_FORMAT_TABLE_STC prvCpssDxChSip5EgfQagEgressEPortTableFieldsFormat[SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS___LAST_VALUE___E] =
{
    PRV_CPSS_FIELD_MAC(   0,   1, SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_DP_TO_CFI_REMAP_ENABLE_E),
    PRV_CPSS_FIELD_MAC(   1,   3, SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_EGRESS_MIRROR_TO_ANALYZER_INDEX_E),
    PRV_CPSS_FIELD_MAC(   4,   1, SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_EPORT_TAG_STATE_MODE_E),
    PRV_CPSS_FIELD_MAC(   5,   3, SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_EPORT_TAG_STATE_E),
    PRV_CPSS_FIELD_MAC(   8,   1, SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_EVLAN_COMMAND_E),
    PRV_CPSS_FIELD_MAC(   9,   1, SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_VID1_COMMAND_E),
    PRV_CPSS_FIELD_MAC(  10,   1, SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_UP0_COMMAND_E),
    PRV_CPSS_FIELD_MAC(  11,   1, SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_UP1_COMMAND_E),
    PRV_CPSS_FIELD_MAC(  12,  16, SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_EVLAN_E),
    PRV_CPSS_FIELD_MAC(  28,  12, SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_VID1_E),
    PRV_CPSS_FIELD_MAC(  40,   3, SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_UP0_E),
    PRV_CPSS_FIELD_MAC(  43,   3, SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_UP1_E),
    PRV_CPSS_FIELD_MAC(  46,   1, SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_REMOVE_VLAN_TAG_1_IF_RX_WITHOUT_TAG_1_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_EGRESS_MIRRORING_MODE_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_30_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_REMOVE_VLAN_TAG_0_IF_RX_WITHOUT_TAG_0_E)
};

static const PRV_CPSS_ENTRY_FORMAT_TABLE_STC prvCpssDxChSip5EgfQagTargetPortMapperTableFieldsFormat[SIP5_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_TABLE_FIELDS___LAST_VALUE___E] =
{
    PRV_CPSS_FIELD_MAC(   0,  10, SIP5_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_TABLE_FIELDS_PORT_BASE_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_TABLE_FIELDS_PORT_SPEED_E)
};

static const PRV_CPSS_ENTRY_FORMAT_TABLE_STC prvCpssDxChSip5EgfShtEgressEVlanTableFieldsFormat[SIP5_EGRESS_EGF_SHT_EGRESS_EVLAN_TABLE_FIELDS___LAST_VALUE___E] =
{
    PRV_CPSS_FIELD_MAC(   0,   1, SIP5_EGRESS_EGF_SHT_EGRESS_EVLAN_TABLE_FIELDS_VALID_E),
    PRV_CPSS_FIELD_MAC(   1, 256, SIP5_EGRESS_EGF_SHT_EGRESS_EVLAN_TABLE_FIELDS_PORT_X_MEMBER_E)
};

static const PRV_CPSS_ENTRY_FORMAT_TABLE_STC prvCpssDxChSip5_20EgfShtEgressEVlanTableFieldsFormat[SIP5_EGRESS_EGF_SHT_EGRESS_EVLAN_TABLE_FIELDS___LAST_VALUE___E] =
{
    PRV_CPSS_FIELD_MAC(   0,   0, SIP5_EGRESS_EGF_SHT_EGRESS_EVLAN_TABLE_FIELDS_VALID_E),
    PRV_CPSS_FIELD_MAC(   0, 256, SIP5_EGRESS_EGF_SHT_EGRESS_EVLAN_TABLE_FIELDS_PORT_X_MEMBER_E)
};

static const PRV_CPSS_ENTRY_FORMAT_TABLE_STC prvCpssDxChSip5_20EgfShtEgressEVlanTableFieldsFormat_512Ports_mode[
    SIP5_EGRESS_EGF_SHT_EGRESS_EVLAN_TABLE_FIELDS___LAST_VALUE___E]=
{
    PRV_CPSS_FIELD_MAC(   0,   0, SIP5_EGRESS_EGF_SHT_EGRESS_EVLAN_TABLE_FIELDS_VALID_E),
    PRV_CPSS_FIELD_MAC(   0, 512, SIP5_EGRESS_EGF_SHT_EGRESS_EVLAN_TABLE_FIELDS_PORT_X_MEMBER_E)
};

static const PRV_CPSS_ENTRY_FORMAT_TABLE_STC prvCpssDxChSip5EgfShtEgressEVlanAttributesTableFieldsFormat[
    SIP5_EGRESS_EGF_SHT_EGRESS_EVLAN_ATTRIBUTES_TABLE_FIELDS___LAST_VALUE___E]=
{
    PRV_CPSS_FIELD_MAC(   0,   1, SIP5_EGRESS_EGF_SHT_EGRESS_EVLAN_ATTRIBUTES_TABLE_FIELDS_LOCAL_SWITHING_EN_E),
    PRV_CPSS_FIELD_MAC(   1,   2, SIP5_EGRESS_EGF_SHT_EGRESS_EVLAN_ATTRIBUTES_TABLE_FIELDS_PORT_ISOLATION_VLAN_CMD_E),
    PRV_CPSS_FIELD_MAC(   3,   3, SIP5_EGRESS_EGF_SHT_EGRESS_EVLAN_ATTRIBUTES_TABLE_FIELDS_EGRESS_MIRROR_TO_ANALYZER_INDEX_E),
    PRV_CPSS_FIELD_MAC(   6,   0, SIP6_EGRESS_EGF_SHT_EGRESS_EVLAN_ATTRIBUTES_TABLE_FIELDS_EGRESS_MIRRORING_MODE_E)
};

static const PRV_CPSS_ENTRY_FORMAT_TABLE_STC prvCpssDxChSip5EgfShtEgressEPortTableFieldsFormat[SIP5_EGRESS_EGF_SHT_EGRESS_EPORT_TABLE_FIELDS___LAST_VALUE___E] =
{
    PRV_CPSS_FIELD_MAC(   0,   1, SIP5_EGRESS_EGF_SHT_EGRESS_EPORT_TABLE_FIELDS_FROM_CPU_FWD_RESTRICT_E),
    PRV_CPSS_FIELD_MAC(   1,   1, SIP5_EGRESS_EGF_SHT_EGRESS_EPORT_TABLE_FIELDS_BRIDGED_FWD_RESTRICT_E),
    PRV_CPSS_FIELD_MAC(   2,   1, SIP5_EGRESS_EGF_SHT_EGRESS_EPORT_TABLE_FIELDS_ROUTED_FWD_RESTRICT_E),
    PRV_CPSS_FIELD_MAC(   3,   1, SIP5_EGRESS_EGF_SHT_EGRESS_EPORT_TABLE_FIELDS_IPMC_ROUTED_FILTER_EN_E),
    PRV_CPSS_FIELD_MAC(   4,   1, SIP5_EGRESS_EGF_SHT_EGRESS_EPORT_TABLE_FIELDS_EGRESS_EVLAN_FILTER_EN_E),
    PRV_CPSS_FIELD_MAC(   5,   2, SIP5_EGRESS_EGF_SHT_EGRESS_EPORT_TABLE_FIELDS_PORT_ISOLATION_MODE_E),
    PRV_CPSS_FIELD_MAC(   7,   1, SIP5_EGRESS_EGF_SHT_EGRESS_EPORT_TABLE_FIELDS_EPORT_STP_STATE_E),
    PRV_CPSS_FIELD_MAC(   8,   1, SIP5_EGRESS_EGF_SHT_EGRESS_EPORT_TABLE_FIELDS_STP_STATE_MODE_E),
    PRV_CPSS_FIELD_MAC(   9,   1, SIP5_EGRESS_EGF_SHT_EGRESS_EPORT_TABLE_FIELDS_ODD_ONLY_SRC_ID_FILTER_EN_E),
    PRV_CPSS_FIELD_MAC(  10,   1, SIP5_EGRESS_EGF_SHT_EGRESS_EPORT_TABLE_FIELDS_EGRESS_EPORT_VLAN_FILTER_EN_E),
    PRV_CPSS_FIELD_MAC(  11,   4, SIP5_EGRESS_EGF_SHT_EGRESS_EPORT_TABLE_FIELDS_MESH_ID_E)
};

static const PRV_CPSS_ENTRY_FORMAT_TABLE_STC prvCpssDxChSip5L2MllTableFieldsFormat[SIP5_L2_MLL_TABLE_FIELDS___LAST_VALUE___E] =
{
    PRV_CPSS_FIELD_MAC(   0,   1, SIP5_L2_MLL_TABLE_FIELDS_LAST_0_E),
    PRV_CPSS_FIELD_MAC(   1,   2, SIP5_L2_MLL_TABLE_FIELDS_BIND_TO_MLL_COUNTER_0_E),
    PRV_CPSS_FIELD_MAC(   3,  15, SIP5_L2_MLL_TABLE_FIELDS_MASK_BITMAP_0_E),
    PRV_CPSS_FIELD_MAC(  18,   1, SIP5_L2_MLL_TABLE_FIELDS_USE_VIDX_0_E),
    PRV_CPSS_FIELD_MAC(  19,   1, SIP5_L2_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_0_E),
    PRV_CPSS_FIELD_MAC(  20,  13, SIP5_L2_MLL_TABLE_FIELDS_TRG_EPORT_0_E),
    PRV_CPSS_FIELD_MAC(  20,  12, SIP5_L2_MLL_TABLE_FIELDS_TRG_TRUNK_ID_0_E),
    PRV_CPSS_FIELD_MAC(  20,  12, SIP5_L2_MLL_TABLE_FIELDS_VIDX_0_E),
    PRV_CPSS_FIELD_MAC(  33,   1, SIP5_L2_MLL_TABLE_FIELDS_UNKNOWN_UC_FILTER_ENABLE_0_E),
    PRV_CPSS_FIELD_MAC(  34,   1, SIP5_L2_MLL_TABLE_FIELDS_UNREGISTERED_MC_FILTER_ENABLE_0_E),
    PRV_CPSS_FIELD_MAC(  35,   1, SIP5_L2_MLL_TABLE_FIELDS_BC_FILTER_ENABLE_0_E),
    PRV_CPSS_FIELD_MAC(  36,  10, SIP5_L2_MLL_TABLE_FIELDS_TRG_DEV_0_E),
    PRV_CPSS_FIELD_MAC(  46,   1, SIP5_L2_MLL_TABLE_FIELDS_ONE_PLUS_ONE_FILTERING_ENABLE_0_E),
    PRV_CPSS_FIELD_MAC(  47,   8, SIP5_L2_MLL_TABLE_FIELDS_TTL_THRESHOLD_0_E),
    PRV_CPSS_FIELD_MAC(  55,   8, SIP5_L2_MLL_TABLE_FIELDS_MESH_ID_0_E),
    PRV_CPSS_FIELD_MAC(  63,   1, SIP5_L2_MLL_TABLE_FIELDS_MC_LOCAL_SWITCHING_ENABLE_0_E),
    PRV_CPSS_FIELD_MAC(  64,   1, SIP5_L2_MLL_TABLE_FIELDS_MAX_HOP_COUNT_ENABLE_0_E),
    PRV_CPSS_FIELD_MAC(  65,   6, SIP5_L2_MLL_TABLE_FIELDS_MAX_OUTGOING_HOP_COUNT_0_E),
    PRV_CPSS_FIELD_MAC(  71,   0, SIP5_L2_MLL_TABLE_FIELDS_RESERVED_1_E),
    PRV_CPSS_FIELD_MAC(  71,   1, SIP5_L2_MLL_TABLE_FIELDS_LAST_1_E),
    PRV_CPSS_FIELD_MAC(  72,   2, SIP5_L2_MLL_TABLE_FIELDS_BIND_TO_MLL_COUNTER_1_E),
    PRV_CPSS_FIELD_MAC(  74,  15, SIP5_L2_MLL_TABLE_FIELDS_MASK_BITMAP_1_E),
    PRV_CPSS_FIELD_MAC(  89,   1, SIP5_L2_MLL_TABLE_FIELDS_USE_VIDX_1_E),
    PRV_CPSS_FIELD_MAC(  90,   1, SIP5_L2_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_1_E),
    PRV_CPSS_FIELD_MAC(  91,  13, SIP5_L2_MLL_TABLE_FIELDS_TRG_EPORT_1_E),
    PRV_CPSS_FIELD_MAC(  91,  12, SIP5_L2_MLL_TABLE_FIELDS_TRG_TRUNK_ID_1_E),
    PRV_CPSS_FIELD_MAC(  91,  12, SIP5_L2_MLL_TABLE_FIELDS_VIDX_1_E),
    PRV_CPSS_FIELD_MAC( 104,   1, SIP5_L2_MLL_TABLE_FIELDS_UNKNOWN_UC_FILTER_ENABLE_1_E),
    PRV_CPSS_FIELD_MAC( 105,   1, SIP5_L2_MLL_TABLE_FIELDS_UNREGISTERED_MC_FILTER_ENABLE_1_E),
    PRV_CPSS_FIELD_MAC( 106,   1, SIP5_L2_MLL_TABLE_FIELDS_BC_FILTER_ENABLE_1_E),
    PRV_CPSS_FIELD_MAC( 107,  10, SIP5_L2_MLL_TABLE_FIELDS_TRG_DEV_1_E),
    PRV_CPSS_FIELD_MAC( 117,   1, SIP5_L2_MLL_TABLE_FIELDS_ONE_PLUS_ONE_FILTERING_ENABLE_1_E),
    PRV_CPSS_FIELD_MAC( 118,   8, SIP5_L2_MLL_TABLE_FIELDS_TTL_THRESHOLD_1_E),
    PRV_CPSS_FIELD_MAC( 126,   8, SIP5_L2_MLL_TABLE_FIELDS_MESH_ID_1_E),
    PRV_CPSS_FIELD_MAC( 134,   1, SIP5_L2_MLL_TABLE_FIELDS_MC_LOCAL_SWITCHING_ENABLE_1_E),
    PRV_CPSS_FIELD_MAC( 135,   1, SIP5_L2_MLL_TABLE_FIELDS_MAX_HOP_COUNT_ENABLE_1_E),
    PRV_CPSS_FIELD_MAC( 136,   6, SIP5_L2_MLL_TABLE_FIELDS_MAX_OUTGOING_HOP_COUNT_1_E),
    PRV_CPSS_FIELD_MAC( 142,   0, SIP5_L2_MLL_TABLE_FIELDS_RESERVED_2_E),
    PRV_CPSS_FIELD_MAC( 142,   1, SIP5_L2_MLL_TABLE_FIELDS_ENTRY_SELECTOR_E),
    PRV_CPSS_FIELD_MAC( 143,  15, SIP5_L2_MLL_TABLE_FIELDS_L2_NEXT_MLL_PTR_E)
};

static const PRV_CPSS_ENTRY_FORMAT_TABLE_STC prvCpssDxChSip5IpMllTableFieldsFormat[SIP5_IP_MLL_TABLE_FIELDS___LAST_VALUE___E] =
{
    PRV_CPSS_FIELD_MAC(   0,   1, SIP5_IP_MLL_TABLE_FIELDS_LAST_0_E),
    PRV_CPSS_FIELD_MAC(   1,   1, SIP5_IP_MLL_TABLE_FIELDS_START_OF_TUNNEL_0_E),
    PRV_CPSS_FIELD_MAC(   2,   3, SIP5_IP_MLL_TABLE_FIELDS_MLL_RPF_FAIL_CMD_0_E),
    PRV_CPSS_FIELD_MAC(   5,  13, SIP5_IP_MLL_TABLE_FIELDS_MLL_EVID_0_E),
    PRV_CPSS_FIELD_MAC(  18,   1, SIP5_IP_MLL_TABLE_FIELDS_USE_VIDX_0_E),
    PRV_CPSS_FIELD_MAC(  19,   1, SIP5_IP_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_0_E),
    PRV_CPSS_FIELD_MAC(  20,  16, SIP5_IP_MLL_TABLE_FIELDS_EVIDX_0_E),
    PRV_CPSS_FIELD_MAC(  20,  13, SIP5_IP_MLL_TABLE_FIELDS_TRG_EPORT_0_E),
    PRV_CPSS_FIELD_MAC(  20,  12, SIP5_IP_MLL_TABLE_FIELDS_TRG_TRUNK_ID_0_E),
    PRV_CPSS_FIELD_MAC(  32,   0, SIP5_IP_MLL_TABLE_FIELDS_RESEREVED_2_E),
    PRV_CPSS_FIELD_MAC(  36,  10, SIP5_IP_MLL_TABLE_FIELDS_TRG_DEV_0_E),
    PRV_CPSS_FIELD_MAC(  46,   1, SIP5_IP_MLL_TABLE_FIELDS_EXCLUDE_SRC_VLAN_0_E),
    PRV_CPSS_FIELD_MAC(  47,   8, SIP5_IP_MLL_TABLE_FIELDS_TTL_THRESHOLD_0_OR_HOP_LIMIT_THRESHOLD_0_E),
    PRV_CPSS_FIELD_MAC(  55,  15, SIP5_IP_MLL_TABLE_FIELDS_TUNNEL_PTR_0_E),
    PRV_CPSS_FIELD_MAC(  70,   1, SIP5_IP_MLL_TABLE_FIELDS_TUNNEL_TYPE_0_E),
    PRV_CPSS_FIELD_MAC(  71,   1, SIP5_IP_MLL_TABLE_FIELDS_LAST_1_E),
    PRV_CPSS_FIELD_MAC(  72,   1, SIP5_IP_MLL_TABLE_FIELDS_START_OF_TUNNEL_1_E),
    PRV_CPSS_FIELD_MAC(  73,   3, SIP5_IP_MLL_TABLE_FIELDS_MLL_RPF_FAIL_CMD_1_E),
    PRV_CPSS_FIELD_MAC(  76,  13, SIP5_IP_MLL_TABLE_FIELDS_MLL_EVID_1_E),
    PRV_CPSS_FIELD_MAC(  89,   1, SIP5_IP_MLL_TABLE_FIELDS_USE_VIDX_1_E),
    PRV_CPSS_FIELD_MAC(  90,   1, SIP5_IP_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_1_E),
    PRV_CPSS_FIELD_MAC(  91,  16, SIP5_IP_MLL_TABLE_FIELDS_EVIDX_1_E),
    PRV_CPSS_FIELD_MAC(  91,  13, SIP5_IP_MLL_TABLE_FIELDS_TRG_EPORT_1_E),
    PRV_CPSS_FIELD_MAC(  91,  12, SIP5_IP_MLL_TABLE_FIELDS_TRG_TRUNK_ID_1_E),
    PRV_CPSS_FIELD_MAC( 103,   0, SIP5_IP_MLL_TABLE_FIELDS_RESEREVED_3_E),
    PRV_CPSS_FIELD_MAC( 107,  10, SIP5_IP_MLL_TABLE_FIELDS_TRG_DEV_1_E),
    PRV_CPSS_FIELD_MAC( 117,   1, SIP5_IP_MLL_TABLE_FIELDS_EXCLUDE_SRC_VLAN_1_E),
    PRV_CPSS_FIELD_MAC( 118,   8, SIP5_IP_MLL_TABLE_FIELDS_TTL_THRESHOLD_1_OR_HOP_LIMIT_THRESHOLD_1_E),
    PRV_CPSS_FIELD_MAC( 126,  15, SIP5_IP_MLL_TABLE_FIELDS_TUNNEL_PTR_1_E),
    PRV_CPSS_FIELD_MAC( 141,   1, SIP5_IP_MLL_TABLE_FIELDS_TUNNEL_TYPE_1_E),
    PRV_CPSS_FIELD_MAC( 142,  16, SIP5_IP_MLL_TABLE_FIELDS_NEXT_MLL_PTR_E)
};

static const PRV_CPSS_ENTRY_FORMAT_TABLE_STC prvCpssDxChSip5HaPhysicalPort1TableFieldsFormat[SIP5_HA_PHYSICAL_PORT_TABLE_1_FIELDS____LAST_VALUE___E] =
{
    PRV_CPSS_FIELD_MAC(   0,   1, SIP5_HA_PHYSICAL_PORT_TABLE_1_FIELDS_ROUTED_MAC_SA_MOD_EN_E),
    PRV_CPSS_FIELD_MAC(   1,   2, SIP5_HA_PHYSICAL_PORT_TABLE_1_FIELDS_ROUTER_MAC_SA_ASSIGNMENT_MODE_E),
    PRV_CPSS_FIELD_MAC(   3,   8, SIP5_HA_PHYSICAL_PORT_TABLE_1_FIELDS_PER_UP0_KEEP_VLAN1_ENABLE_E),
    PRV_CPSS_FIELD_MAC(  11,   1, SIP5_HA_PHYSICAL_PORT_TABLE_1_FIELDS_IP_TUNNEL_LENGTH_OFFSET_ENABLE_E),
    PRV_CPSS_FIELD_MAC(  12,   2, SIP5_HA_PHYSICAL_PORT_TABLE_1_FIELDS_EPCL_LOOKUP_CONFIGURATION_MODE_E),
    PRV_CPSS_FIELD_MAC(  14,   1, SIP5_HA_PHYSICAL_PORT_TABLE_1_FIELDS_EPCL_NON_TS_DATA_PORT_ENABLE_E),
    PRV_CPSS_FIELD_MAC(  15,   1, SIP5_HA_PHYSICAL_PORT_TABLE_1_FIELDS_EPCL_TS_DATA_PORT_ENABLE_E),
    PRV_CPSS_FIELD_MAC(  16,   1, SIP5_HA_PHYSICAL_PORT_TABLE_1_FIELDS_EPCL_FROM_CPU_DATA_PACKETS_PORT_ENABLE_E),
    PRV_CPSS_FIELD_MAC(  17,   1, SIP5_HA_PHYSICAL_PORT_TABLE_1_FIELDS_EPCL_FROM_CPU_CONTROL_PACKETS_PORT_ENABLE_E),
    PRV_CPSS_FIELD_MAC(  18,   4, SIP5_HA_PHYSICAL_PORT_TABLE_1_FIELDS_PORT_GROUP_E),
    PRV_CPSS_FIELD_MAC(  22,   5, SIP5_HA_PHYSICAL_PORT_TABLE_1_FIELDS_PORT_LIST_BIT_VECTOR_OFFSET_E),
    PRV_CPSS_FIELD_MAC(  27,  24, SIP6_10_HA_PHYSICAL_PORT_TABLE_1_FIELDS_PCL_ID2_E),
    PRV_CPSS_FIELD_MAC(  51,   1, SIP6_10_HA_PHYSICAL_PORT_TABLE_1_FIELDS_SET_SRC_ID_E),
    PRV_CPSS_FIELD_MAC(  52,   6, SIP6_10_HA_PHYSICAL_PORT_TABLE_1_FIELDS_SRC_ID_E),
    PRV_CPSS_FIELD_MAC(  58,   1, SIP6_30_HA_PHYSICAL_PORT_TABLE_1_FIELDS_SET_DESTINATION_EPG_E),
    PRV_CPSS_FIELD_MAC(  59,  12, SIP6_30_HA_PHYSICAL_PORT_TABLE_1_FIELDS_DESTINATION_EPG_E),
    PRV_CPSS_FIELD_MAC(  71,   2, SIP6_30_HA_PHYSICAL_PORT_TABLE_1_FIELDS_PRP_TRAILER_ACTION_E),
    PRV_CPSS_FIELD_MAC(  73,   2, SIP6_30_HA_PHYSICAL_PORT_TABLE_1_FIELDS_PRP_PADDING_SIZE_E),
    PRV_CPSS_FIELD_MAC(  75,   6, SIP6_30_HA_PHYSICAL_PORT_TABLE_1_FIELDS_PRP_BASE_LSDU_OFFSET_E)
};

static const PRV_CPSS_ENTRY_FORMAT_TABLE_STC prvCpssDxChSip5HaPhysicalPort2TableFieldsFormat[SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS____LAST_VALUE___E] =
{
    PRV_CPSS_FIELD_MAC(   0,   1, SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_CASCADE_PORT_ENABLE_E),
    PRV_CPSS_FIELD_MAC(   1,   2, SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_EGRESS_DSA_TAG_TYPE_E),
    PRV_CPSS_FIELD_MAC(   3,   1, SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_SER_CHECK_ENABLE_E),
    PRV_CPSS_FIELD_MAC(   4,   1, SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_DISABLE_CRC_ADDING_E),
    PRV_CPSS_FIELD_MAC(   5,   1, SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_RETAIN_EXTERNAL_CRC_E),
    PRV_CPSS_FIELD_MAC(   6,   1, SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_PACKET_ID_ENABLE_E),
    PRV_CPSS_FIELD_MAC(   7,   1, SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_ROUTED_SRC_DEVICE_ID_PORT_MODE_ENABLE_E),
    PRV_CPSS_FIELD_MAC(   8,   1, SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_SET_SOURCE_TO_LOCAL_E),
    PRV_CPSS_FIELD_MAC(   9,  12, SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_LOCAL_DEVICE_NUMBER_E),
    PRV_CPSS_FIELD_MAC(  21,  12, SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_LOCAL_SOURCE_ID_E),
    PRV_CPSS_FIELD_MAC(  33,   1, SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_EPCL_TO_ANALYZER_PACKETS_PORT_ENABLE_E),
    PRV_CPSS_FIELD_MAC(  34,   1, SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_EPCL_TO_CPU_PACKETS_PORT_ENABLE_E),
    PRV_CPSS_FIELD_MAC(  35,   3, SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_PTP_TIMESTAMP_TAG_MODE_E),
    PRV_CPSS_FIELD_MAC(  38,   1, SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_INVALID_CRC_MODE_E),
    PRV_CPSS_FIELD_MAC(  39,   1, SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_TIMESTAMP_RECEPTION_EN_E),
    PRV_CPSS_FIELD_MAC(  40,  12, SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_TRUNK_ID_E),
    PRV_CPSS_FIELD_MAC(  52,   1, SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_FORCE_NEW_DSA_TO_CPU_E),
    PRV_CPSS_FIELD_MAC(  53,   1, SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_FORCE_NEW_DSA_FORWARD_OR_FROM_CPU_E),
    PRV_CPSS_FIELD_MAC(  54,   1, SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_PRE_PEND_TWO_BYTES_HEADER_EN_E),
    PRV_CPSS_FIELD_MAC(  55,   1, SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_DSA_QOS_MODE_E),
    PRV_CPSS_FIELD_MAC(  56,   1, SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_REMOTE_PHYSICAL_PORT_MAP_ENABLE_E),
    PRV_CPSS_FIELD_MAC(  57,   8, SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_REMOTE_PHYSICAL_PORT_NUMBER_E),
    PRV_CPSS_FIELD_MAC(  65,  12, SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_REMOTE_PHYSICAL_PORT_DEVICE_ID_E),
    PRV_CPSS_FIELD_MAC(  77,   1, SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_TRANS_FWD_TO_4B_FROM_CPU_DSA_E),
    PRV_CPSS_FIELD_MAC(  78,   1, SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_TRANS_TO_ANALYZER_TO_4B_FROM_CPU_DSA_E),
    PRV_CPSS_FIELD_MAC(  79,   1, SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_TRANS_FROM_CPU_TO_4B_FROM_CPU_DSA_E),
    PRV_CPSS_FIELD_MAC(  80,   1, SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_INSERT_HASH_INTO_FORWARD_DSA_ENABLE_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_PUSHED_TAG_VLAN_ID_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_USE_PHYSICAL_PORT_PUSH_TAG_VID_EN_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_30_HA_PHYSICAL_PORT_TABLE_2_FIELDS_HSR_BASE_LSDU_OFFSET_E)
};

static const PRV_CPSS_ENTRY_FORMAT_TABLE_STC prvCpssDxChSip5HaEPort1TableFieldsFormat[SIP5_HA_EPORT_TABLE_1_FIELDS____LAST_VALUE___E] =
{
    PRV_CPSS_FIELD_MAC(   0,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_VLAN_TRANS_EN_E),
    PRV_CPSS_FIELD_MAC(   1,   3, SIP5_HA_EPORT_TABLE_1_FIELDS_EGRESS_TAG0_TPID_INDEX_E),
    PRV_CPSS_FIELD_MAC(   4,   3, SIP5_HA_EPORT_TABLE_1_FIELDS_EGRESS_TAG1_TPID_INDEX_E),
    PRV_CPSS_FIELD_MAC(   7,   3, SIP5_HA_EPORT_TABLE_1_FIELDS_TS_HEADER_TPID_INDEX_E),
    PRV_CPSS_FIELD_MAC(  10,  32, SIP5_HA_EPORT_TABLE_1_FIELDS_TS_EXT_E),
    PRV_CPSS_FIELD_MAC(  10,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_MPLS_PW_LABEL_PUSH_ENABLE_E),
    PRV_CPSS_FIELD_MAC(  11,  20, SIP5_HA_EPORT_TABLE_1_FIELDS_MPLS_PW_LABEL_E),
    PRV_CPSS_FIELD_MAC(  31,   3, SIP5_HA_EPORT_TABLE_1_FIELDS_MPLS_PW_LABEL_EXP_E),
    PRV_CPSS_FIELD_MAC(  34,   8, SIP5_HA_EPORT_TABLE_1_FIELDS_MPLS_PW_LABEL_TTL_E),
    PRV_CPSS_FIELD_MAC(  42,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_PUSH_MPLS_FLOW_LABEL_ENABLE_E),
    PRV_CPSS_FIELD_MAC(  43,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_FORCE_ARP_TS_EPORT_DECISION_E),
    PRV_CPSS_FIELD_MAC(  44,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_TUNNEL_START_E),
    PRV_CPSS_FIELD_MAC(  45,  18, SIP5_HA_EPORT_TABLE_1_FIELDS_TS_POINTER_E),
    PRV_CPSS_FIELD_MAC(  45,  20, SIP5_HA_EPORT_TABLE_1_FIELDS_ARP_POINTER_E),
    PRV_CPSS_FIELD_MAC(  63,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_TS_PASSENGER_TYPE_E),
    PRV_CPSS_FIELD_MAC(  65,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_MODIFY_MAC_SA_E),
    PRV_CPSS_FIELD_MAC(  66,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_MODIFY_MAC_DA_E),
    PRV_CPSS_FIELD_MAC(  67,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_TRILL_INTERFACE_ENABLE_E),
    PRV_CPSS_FIELD_MAC(  68,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_KEEP_ORIGINAL_TAG0_TPID_E),
    PRV_CPSS_FIELD_MAC(  69,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_KEEP_ORIGINAL_TAG1_TPID_E),
    PRV_CPSS_FIELD_MAC(  70,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_ENABLE_EGRESS_UP_MAPPING_E),
    PRV_CPSS_FIELD_MAC(  71,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_ENABLE_EGRESS_DSCP_MAPPING_E),
    PRV_CPSS_FIELD_MAC(  72,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_ENABLE_EGRESS_EXP_MAPPING_E),
    PRV_CPSS_FIELD_MAC(  73,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_ENABLE_EGRESS_TC_DP_MAPPING_E),
    PRV_CPSS_FIELD_MAC(  74,   4, SIP5_HA_EPORT_TABLE_1_FIELDS_QOS_MAPPING_TABLE_INDEX_E),
    PRV_CPSS_FIELD_MAC(  78,   8, SIP5_HA_EPORT_TABLE_1_FIELDS_ROUTER_MAC_SA_INDEX_E),
    PRV_CPSS_FIELD_MAC(  86,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_DP_TO_CFI_MAP_ENABLE_E),
    PRV_CPSS_FIELD_MAC(  87,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_PUSH_SOURCE_BASED_MPLS_LABEL_E),
    PRV_CPSS_FIELD_MAC(  88,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_PUSH_EVLAN_BASED_MPLS_LABEL_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_HA_EPORT_TABLE_1_FIELDS_PHA_THREAD_NUMBER_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_10_HA_EPORT_TABLE_1_FIELDS_NESTED_VLAN_EN_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_30_HA_EPORT_TABLE_1_FIELDS_NESTED_VLAN_MODE_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_30_HA_EPORT_TABLE_1_FIELDS_HSR_PRP_LAN_ID_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_30_HA_EPORT_TABLE_1_FIELDS_L2NAT_ENABLE_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_30_HA_EPORT_TABLE_1_FIELDS_NAT_PTR_E)
};

static const PRV_CPSS_ENTRY_FORMAT_TABLE_STC prvCpssDxChSip5HaEPort2TableFieldsFormat[SIP5_HA_EPORT_TABLE_2_FIELDS____LAST_VALUE___E] =
{
    PRV_CPSS_FIELD_MAC(   0,   1, SIP5_HA_EPORT_TABLE_2_MIRROR_TO_ANALYZER_KEEP_TAGS_E),
    PRV_CPSS_FIELD_MAC(   1,   1, SIP5_HA_EPORT_TABLE_2_TO_ANALYZER_VLAN_TAG_ADD_EN_E),
    PRV_CPSS_FIELD_MAC(   2,   1, SIP5_HA_EPORT_TABLE_2_PUSH_VLAN_COMMAND_E),
    PRV_CPSS_FIELD_MAC(   3,   3, SIP5_HA_EPORT_TABLE_2_PUSHED_TAG_TPID_SELECT_E),
    PRV_CPSS_FIELD_MAC(   6,  12, SIP5_HA_EPORT_TABLE_2_PUSHED_TAG_VALUE_E),
    PRV_CPSS_FIELD_MAC(  18,   1, SIP5_HA_EPORT_TABLE_2_UP_CFI_ASSIGNMENT_COMMAND_E),
    PRV_CPSS_FIELD_MAC(  19,   3, SIP5_HA_EPORT_TABLE_2_UP_E),
    PRV_CPSS_FIELD_MAC(  22,   1, SIP5_HA_EPORT_TABLE_2_CFI_E),
    PRV_CPSS_FIELD_MAC(  23,   1, SIP5_HA_EPORT_TABLE_2_FORCE_E_TAG_IE_PID_TO_DEFAULT_E)
};

static const PRV_CPSS_ENTRY_FORMAT_TABLE_STC prvCpssDxChSip5HaTunnelStartTableFieldsFormat[SIP5_HA_TUNNEL_START_TABLE_FIELDS____LAST_VALUE___E] =
{
    PRV_CPSS_FIELD_MAC(   0,   3, SIP5_HA_TUNNEL_START_TABLE_FIELDS_TUNNEL_TYPE_E),
    PRV_CPSS_FIELD_MAC(   3,   1, SIP5_HA_TUNNEL_START_TABLE_FIELDS_UP_MARKING_MODE_E),
    PRV_CPSS_FIELD_MAC(   4,   3, SIP5_HA_TUNNEL_START_TABLE_FIELDS_UP_E),
    PRV_CPSS_FIELD_MAC(   7,   1, SIP5_HA_TUNNEL_START_TABLE_FIELDS_TAG_ENABLE_E),
    PRV_CPSS_FIELD_MAC(   8,  12, SIP5_HA_TUNNEL_START_TABLE_FIELDS_VID_E),
    PRV_CPSS_FIELD_MAC(  20,   8, SIP5_HA_TUNNEL_START_TABLE_FIELDS_TTL_E),
    PRV_CPSS_FIELD_MAC(  28,   1, SIP5_HA_TUNNEL_START_TABLE_FIELDS_RETAIN_INNER_CRC_E),
    PRV_CPSS_FIELD_MAC(  29,   1, SIP5_HA_TUNNEL_START_TABLE_FIELDS_RESERVED_E),
    PRV_CPSS_FIELD_MAC(  30,   1, SIP5_HA_TUNNEL_START_TABLE_FIELDS_B_SA_ASSIGN_MODE_E),
    PRV_CPSS_FIELD_MAC(  30,   2, SIP5_HA_TUNNEL_START_TABLE_FIELDS_GRE_KEY_MODE_E),
    PRV_CPSS_FIELD_MAC(  30,   2, SIP5_HA_TUNNEL_START_TABLE_FIELDS_PROTOCOL_E),
    PRV_CPSS_FIELD_MAC(  32,  48, SIP5_HA_TUNNEL_START_TABLE_FIELDS_NEXT_HOP_MAC_DA_E),
    PRV_CPSS_FIELD_MAC(  80,  16, SIP5_HA_TUNNEL_START_TABLE_FIELDS_GRE_PROTOCOL_E),
    PRV_CPSS_FIELD_MAC(  80,  16, SIP5_HA_TUNNEL_START_TABLE_FIELDS_RESERVED_1_E),
    PRV_CPSS_FIELD_MAC(  80,  16, SIP5_HA_TUNNEL_START_TABLE_FIELDS_IPV4_GRE_KEY_1_E),
    PRV_CPSS_FIELD_MAC(  80,   8, SIP5_HA_TUNNEL_START_TABLE_FIELDS_IP_PROTOCOL_E),
    PRV_CPSS_FIELD_MAC(  80,  16, SIP5_HA_TUNNEL_START_TABLE_FIELDS_UDP_DST_PORT_E),
    PRV_CPSS_FIELD_MAC(  96,   6, SIP5_HA_TUNNEL_START_TABLE_FIELDS_DSCP_E),
    PRV_CPSS_FIELD_MAC( 102,   1, SIP5_HA_TUNNEL_START_TABLE_FIELDS_DSCP_MARKING_MODE_E),
    PRV_CPSS_FIELD_MAC( 103,   1, SIP5_HA_TUNNEL_START_TABLE_FIELDS_DF_FLAG_E),
    PRV_CPSS_FIELD_MAC( 104,  24, SIP5_HA_TUNNEL_START_TABLE_FIELDS_RESERVED_2_E),
    PRV_CPSS_FIELD_MAC( 104,   5, SIP5_HA_TUNNEL_START_TABLE_FIELDS_RID_E),
    PRV_CPSS_FIELD_MAC( 109,   3, SIP5_HA_TUNNEL_START_TABLE_FIELDS_CAPWAP_FLAGS_E),
    PRV_CPSS_FIELD_MAC( 104,   4, SIP5_HA_TUNNEL_START_TABLE_FIELDS_AUTO_TUNNEL_OFFSET_E),
    PRV_CPSS_FIELD_MAC( 108,   1, SIP5_HA_TUNNEL_START_TABLE_FIELDS_AUTO_TUNNEL_FLAG_E),
    PRV_CPSS_FIELD_MAC( 109,   1, SIP5_HA_TUNNEL_START_TABLE_FIELDS_IPV4_GRE_ENABLE_E),
    PRV_CPSS_FIELD_MAC( 110,  16, SIP5_HA_TUNNEL_START_TABLE_FIELDS_IPV4_GRE_KEY_0_E),
    PRV_CPSS_FIELD_MAC( 126,   2, SIP5_HA_TUNNEL_START_TABLE_FIELDS_RESERVED_4_E),
    PRV_CPSS_FIELD_MAC(  96,  48, SIP5_HA_TUNNEL_START_TABLE_FIELDS_TRILL_HEADER_E),
    PRV_CPSS_FIELD_MAC(  96,  20, SIP5_HA_TUNNEL_START_TABLE_FIELDS_LABEL1_E),
    PRV_CPSS_FIELD_MAC( 116,   3, SIP5_HA_TUNNEL_START_TABLE_FIELDS_EXP1_E),
    PRV_CPSS_FIELD_MAC( 119,   1, SIP5_HA_TUNNEL_START_TABLE_FIELDS_EXP1_MARKING_MODE_E),
    PRV_CPSS_FIELD_MAC( 120,   2, SIP5_HA_TUNNEL_START_TABLE_FIELDS_NUMBER_OF_MPLS_LABELS_E),
    PRV_CPSS_FIELD_MAC( 122,   1, SIP5_HA_TUNNEL_START_TABLE_FIELDS_RESERVED_3_E),
    PRV_CPSS_FIELD_MAC( 123,   1, SIP5_HA_TUNNEL_START_TABLE_FIELDS_SET_S_BIT_E),
    PRV_CPSS_FIELD_MAC( 124,   2, SIP5_HA_TUNNEL_START_TABLE_FIELDS_SWAP_TTL_MODE_E),
    PRV_CPSS_FIELD_MAC( 126,   1, SIP5_HA_TUNNEL_START_TABLE_FIELDS_MPLS_MC_UPSTREAM_ASSIGNED_LABEL_E),
    PRV_CPSS_FIELD_MAC( 127,   1, SIP5_HA_TUNNEL_START_TABLE_FIELDS_RESERVED_7_E),
    PRV_CPSS_FIELD_MAC( 128,  20, SIP5_HA_TUNNEL_START_TABLE_FIELDS_LABEL2_E),
    PRV_CPSS_FIELD_MAC( 148,   3, SIP5_HA_TUNNEL_START_TABLE_FIELDS_EXP2_E),
    PRV_CPSS_FIELD_MAC( 151,   1, SIP5_HA_TUNNEL_START_TABLE_FIELDS_EXP2_MARKING_MODE_E),
    PRV_CPSS_FIELD_MAC( 152,   4, SIP5_HA_TUNNEL_START_TABLE_FIELDS_PW_CONTROL_INDEX_E),
    PRV_CPSS_FIELD_MAC( 156,   4, SIP5_HA_TUNNEL_START_TABLE_FIELDS_RESERVED_5_E),
    PRV_CPSS_FIELD_MAC( 160,  20, SIP5_HA_TUNNEL_START_TABLE_FIELDS_LABEL3_E),
    PRV_CPSS_FIELD_MAC( 180,   3, SIP5_HA_TUNNEL_START_TABLE_FIELDS_EXP3_E),
    PRV_CPSS_FIELD_MAC( 183,   1, SIP5_HA_TUNNEL_START_TABLE_FIELDS_EXP3_MARKING_MODE_E),
    PRV_CPSS_FIELD_MAC(  96,  24, SIP5_HA_TUNNEL_START_TABLE_FIELDS_I_SID_E),
    PRV_CPSS_FIELD_MAC( 120,   1, SIP5_HA_TUNNEL_START_TABLE_FIELDS_I_UP_MARKING_MODE_E),
    PRV_CPSS_FIELD_MAC( 121,   1, SIP5_HA_TUNNEL_START_TABLE_FIELDS_RESERVED_6_E),
    PRV_CPSS_FIELD_MAC( 122,   3, SIP5_HA_TUNNEL_START_TABLE_FIELDS_I_UP_E),
    PRV_CPSS_FIELD_MAC( 125,   1, SIP5_HA_TUNNEL_START_TABLE_FIELDS_I_DEI_E),
    PRV_CPSS_FIELD_MAC( 126,   1, SIP5_HA_TUNNEL_START_TABLE_FIELDS_I_DEI_MARKING_MODE_E),
    PRV_CPSS_FIELD_MAC( 127,   1, SIP5_HA_TUNNEL_START_TABLE_FIELDS_I_SID_ASSIGN_MODE_E),
    PRV_CPSS_FIELD_MAC( 128,   2, SIP5_HA_TUNNEL_START_TABLE_FIELDS_I_TAG_RES2_E),
    PRV_CPSS_FIELD_MAC( 130,   1, SIP5_HA_TUNNEL_START_TABLE_FIELDS_I_TAG_RES1_E),
    PRV_CPSS_FIELD_MAC( 131,   1, SIP5_HA_TUNNEL_START_TABLE_FIELDS_I_TAG_NCA_E),
    PRV_CPSS_FIELD_MAC( 132,   1, SIP5_HA_TUNNEL_START_TABLE_FIELDS_B_DA_ASSIGN_MODE_E),
    PRV_CPSS_FIELD_MAC( 109,   3, SIP5_HA_TUNNEL_START_TABLE_FIELDS_PROFILE_NUMBER_E),
    PRV_CPSS_FIELD_MAC( 112,  16, SIP5_HA_TUNNEL_START_TABLE_FIELDS_UDP_SRC_PORT_E),
    PRV_CPSS_FIELD_MAC( 112,  16, SIP5_HA_TUNNEL_START_TABLE_FIELDS_GRE_FLAGS_AND_VER_E),
    PRV_CPSS_FIELD_MAC( 128,  32, SIP5_HA_TUNNEL_START_TABLE_FIELDS_DIP_E),
    PRV_CPSS_FIELD_MAC( 160,  32, SIP5_HA_TUNNEL_START_TABLE_FIELDS_SIP_E),
    PRV_CPSS_FIELD_MAC( 103,   1, SIP5_HA_TUNNEL_START_TABLE_FIELDS_FLOW_LABEL_MODE_E),
    PRV_CPSS_FIELD_MAC(  80,   1, SIP5_HA_TUNNEL_START_TABLE_FIELDS_MPLS_PW_EXP_MARKING_MODE_E),
    PRV_CPSS_FIELD_MAC(  81,   1, SIP5_HA_TUNNEL_START_TABLE_FIELDS_PUSH_ELI_AND_EL_AFTER_LABEL_1_E),
    PRV_CPSS_FIELD_MAC(  82,   1, SIP5_HA_TUNNEL_START_TABLE_FIELDS_PUSH_ELI_AND_EL_AFTER_LABEL_2_E),
    PRV_CPSS_FIELD_MAC(  83,   1, SIP5_HA_TUNNEL_START_TABLE_FIELDS_PUSH_ELI_AND_EL_AFTER_LABEL_3_E)
};

static const PRV_CPSS_ENTRY_FORMAT_TABLE_STC prvCpssDxChSip6HaTunnelStartTableFieldsFormat[SIP6_HA_TUNNEL_START_TABLE_GENERIC_FIELDS____LAST_VALUE___E] =
{
    PRV_CPSS_FIELD_MAC(   0,   3, SIP6_HA_TUNNEL_START_TABLE_GENERIC_FIELDS_TUNNEL_TYPE_E),
    PRV_CPSS_FIELD_MAC(   3,   1, SIP6_HA_TUNNEL_START_TABLE_GENERIC_FIELDS_UP_MARKING_MODE_E),
    PRV_CPSS_FIELD_MAC(   4,   3, SIP6_HA_TUNNEL_START_TABLE_GENERIC_FIELDS_UP_E),
    PRV_CPSS_FIELD_MAC(   7,   1, SIP6_HA_TUNNEL_START_TABLE_GENERIC_FIELDS_TAG_ENABLE_E),
    PRV_CPSS_FIELD_MAC(   8,  12, SIP6_HA_TUNNEL_START_TABLE_GENERIC_FIELDS_VID_E),
    PRV_CPSS_FIELD_MAC(  20,   3, SIP6_HA_TUNNEL_START_TABLE_GENERIC_FIELDS_GENERIC_TUNNEL_START_TYPE_E),
    PRV_CPSS_FIELD_MAC(  23,   5, SIP6_HA_TUNNEL_START_TABLE_GENERIC_FIELDS_RESERVED_1_E),
    PRV_CPSS_FIELD_MAC(  28,   1, SIP6_HA_TUNNEL_START_TABLE_GENERIC_FIELDS_RETAIN_INNER_CRC_E),
    PRV_CPSS_FIELD_MAC(  29,   3, SIP6_HA_TUNNEL_START_TABLE_GENERIC_FIELDS_RESERVED_2_E),
    PRV_CPSS_FIELD_MAC(  32,  48, SIP6_HA_TUNNEL_START_TABLE_GENERIC_FILEDS_MAC_DA_E),
    PRV_CPSS_FIELD_MAC(  80,  16, SIP6_HA_TUNNEL_START_TABLE_GENERIC_FIELDS_ETHER_TYPE_E)
};

static const PRV_CPSS_ENTRY_FORMAT_TABLE_STC prvCpssDxChSip5HaGenTunnelStartProfileTableFieldsFormat[SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS____LAST_VALUE___E] =
{
    PRV_CPSS_FIELD_MAC(   0,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE0_BIT0_E),
    PRV_CPSS_FIELD_MAC(   4,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE0_BIT1_E),
    PRV_CPSS_FIELD_MAC(   8,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE0_BIT2_E),
    PRV_CPSS_FIELD_MAC(  12,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE0_BIT3_E),
    PRV_CPSS_FIELD_MAC(  16,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE0_BIT4_E),
    PRV_CPSS_FIELD_MAC(  20,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE0_BIT5_E),
    PRV_CPSS_FIELD_MAC(  24,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE0_BIT6_E),
    PRV_CPSS_FIELD_MAC(  28,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE0_BIT7_E),
    PRV_CPSS_FIELD_MAC(  32,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE1_BIT0_E),
    PRV_CPSS_FIELD_MAC(  36,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE1_BIT1_E),
    PRV_CPSS_FIELD_MAC(  40,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE1_BIT2_E),
    PRV_CPSS_FIELD_MAC(  44,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE1_BIT3_E),
    PRV_CPSS_FIELD_MAC(  48,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE1_BIT4_E),
    PRV_CPSS_FIELD_MAC(  52,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE1_BIT5_E),
    PRV_CPSS_FIELD_MAC(  56,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE1_BIT6_E),
    PRV_CPSS_FIELD_MAC(  60,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE1_BIT7_E),
    PRV_CPSS_FIELD_MAC(  64,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE2_BIT0_E),
    PRV_CPSS_FIELD_MAC(  68,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE2_BIT1_E),
    PRV_CPSS_FIELD_MAC(  72,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE2_BIT2_E),
    PRV_CPSS_FIELD_MAC(  76,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE2_BIT3_E),
    PRV_CPSS_FIELD_MAC(  80,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE2_BIT4_E),
    PRV_CPSS_FIELD_MAC(  84,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE2_BIT5_E),
    PRV_CPSS_FIELD_MAC(  88,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE2_BIT6_E),
    PRV_CPSS_FIELD_MAC(  92,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE2_BIT7_E),
    PRV_CPSS_FIELD_MAC(  96,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE3_BIT0_E),
    PRV_CPSS_FIELD_MAC( 100,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE3_BIT1_E),
    PRV_CPSS_FIELD_MAC( 104,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE3_BIT2_E),
    PRV_CPSS_FIELD_MAC( 108,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE3_BIT3_E),
    PRV_CPSS_FIELD_MAC( 112,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE3_BIT4_E),
    PRV_CPSS_FIELD_MAC( 116,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE3_BIT5_E),
    PRV_CPSS_FIELD_MAC( 120,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE3_BIT6_E),
    PRV_CPSS_FIELD_MAC( 124,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE3_BIT7_E),
    PRV_CPSS_FIELD_MAC( 128,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE4_BIT0_E),
    PRV_CPSS_FIELD_MAC( 132,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE4_BIT1_E),
    PRV_CPSS_FIELD_MAC( 136,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE4_BIT2_E),
    PRV_CPSS_FIELD_MAC( 140,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE4_BIT3_E),
    PRV_CPSS_FIELD_MAC( 144,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE4_BIT4_E),
    PRV_CPSS_FIELD_MAC( 148,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE4_BIT5_E),
    PRV_CPSS_FIELD_MAC( 152,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE4_BIT6_E),
    PRV_CPSS_FIELD_MAC( 156,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE4_BIT7_E),
    PRV_CPSS_FIELD_MAC( 160,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE5_BIT0_E),
    PRV_CPSS_FIELD_MAC( 164,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE5_BIT1_E),
    PRV_CPSS_FIELD_MAC( 168,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE5_BIT2_E),
    PRV_CPSS_FIELD_MAC( 172,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE5_BIT3_E),
    PRV_CPSS_FIELD_MAC( 176,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE5_BIT4_E),
    PRV_CPSS_FIELD_MAC( 180,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE5_BIT5_E),
    PRV_CPSS_FIELD_MAC( 184,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE5_BIT6_E),
    PRV_CPSS_FIELD_MAC( 188,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE5_BIT7_E),
    PRV_CPSS_FIELD_MAC( 192,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE6_BIT0_E),
    PRV_CPSS_FIELD_MAC( 196,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE6_BIT1_E),
    PRV_CPSS_FIELD_MAC( 200,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE6_BIT2_E),
    PRV_CPSS_FIELD_MAC( 204,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE6_BIT3_E),
    PRV_CPSS_FIELD_MAC( 208,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE6_BIT4_E),
    PRV_CPSS_FIELD_MAC( 212,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE6_BIT5_E),
    PRV_CPSS_FIELD_MAC( 216,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE6_BIT6_E),
    PRV_CPSS_FIELD_MAC( 220,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE6_BIT7_E),
    PRV_CPSS_FIELD_MAC( 224,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE7_BIT0_E),
    PRV_CPSS_FIELD_MAC( 228,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE7_BIT1_E),
    PRV_CPSS_FIELD_MAC( 232,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE7_BIT2_E),
    PRV_CPSS_FIELD_MAC( 236,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE7_BIT3_E),
    PRV_CPSS_FIELD_MAC( 240,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE7_BIT4_E),
    PRV_CPSS_FIELD_MAC( 244,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE7_BIT5_E),
    PRV_CPSS_FIELD_MAC( 248,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE7_BIT6_E),
    PRV_CPSS_FIELD_MAC( 252,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE7_BIT7_E),
    PRV_CPSS_FIELD_MAC( 256,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE8_BIT0_E),
    PRV_CPSS_FIELD_MAC( 260,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE8_BIT1_E),
    PRV_CPSS_FIELD_MAC( 264,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE8_BIT2_E),
    PRV_CPSS_FIELD_MAC( 268,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE8_BIT3_E),
    PRV_CPSS_FIELD_MAC( 272,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE8_BIT4_E),
    PRV_CPSS_FIELD_MAC( 276,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE8_BIT5_E),
    PRV_CPSS_FIELD_MAC( 280,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE8_BIT6_E),
    PRV_CPSS_FIELD_MAC( 284,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE8_BIT7_E),
    PRV_CPSS_FIELD_MAC( 288,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE9_BIT0_E),
    PRV_CPSS_FIELD_MAC( 292,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE9_BIT1_E),
    PRV_CPSS_FIELD_MAC( 296,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE9_BIT2_E),
    PRV_CPSS_FIELD_MAC( 300,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE9_BIT3_E),
    PRV_CPSS_FIELD_MAC( 304,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE9_BIT4_E),
    PRV_CPSS_FIELD_MAC( 308,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE9_BIT5_E),
    PRV_CPSS_FIELD_MAC( 312,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE9_BIT6_E),
    PRV_CPSS_FIELD_MAC( 316,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE9_BIT7_E),
    PRV_CPSS_FIELD_MAC( 320,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE10_BIT0_E),
    PRV_CPSS_FIELD_MAC( 324,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE10_BIT1_E),
    PRV_CPSS_FIELD_MAC( 328,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE10_BIT2_E),
    PRV_CPSS_FIELD_MAC( 332,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE10_BIT3_E),
    PRV_CPSS_FIELD_MAC( 336,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE10_BIT4_E),
    PRV_CPSS_FIELD_MAC( 340,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE10_BIT5_E),
    PRV_CPSS_FIELD_MAC( 344,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE10_BIT6_E),
    PRV_CPSS_FIELD_MAC( 348,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE10_BIT7_E),
    PRV_CPSS_FIELD_MAC( 352,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE11_BIT0_E),
    PRV_CPSS_FIELD_MAC( 356,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE11_BIT1_E),
    PRV_CPSS_FIELD_MAC( 360,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE11_BIT2_E),
    PRV_CPSS_FIELD_MAC( 364,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE11_BIT3_E),
    PRV_CPSS_FIELD_MAC( 368,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE11_BIT4_E),
    PRV_CPSS_FIELD_MAC( 372,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE11_BIT5_E),
    PRV_CPSS_FIELD_MAC( 376,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE11_BIT6_E),
    PRV_CPSS_FIELD_MAC( 380,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE11_BIT7_E),
    PRV_CPSS_FIELD_MAC( 384,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE12_BIT0_E),
    PRV_CPSS_FIELD_MAC( 388,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE12_BIT1_E),
    PRV_CPSS_FIELD_MAC( 392,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE12_BIT2_E),
    PRV_CPSS_FIELD_MAC( 396,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE12_BIT3_E),
    PRV_CPSS_FIELD_MAC( 400,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE12_BIT4_E),
    PRV_CPSS_FIELD_MAC( 404,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE12_BIT5_E),
    PRV_CPSS_FIELD_MAC( 408,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE12_BIT6_E),
    PRV_CPSS_FIELD_MAC( 412,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE12_BIT7_E),
    PRV_CPSS_FIELD_MAC( 416,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE13_BIT0_E),
    PRV_CPSS_FIELD_MAC( 420,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE13_BIT1_E),
    PRV_CPSS_FIELD_MAC( 424,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE13_BIT2_E),
    PRV_CPSS_FIELD_MAC( 428,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE13_BIT3_E),
    PRV_CPSS_FIELD_MAC( 432,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE13_BIT4_E),
    PRV_CPSS_FIELD_MAC( 436,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE13_BIT5_E),
    PRV_CPSS_FIELD_MAC( 440,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE13_BIT6_E),
    PRV_CPSS_FIELD_MAC( 444,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE13_BIT7_E),
    PRV_CPSS_FIELD_MAC( 448,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE14_BIT0_E),
    PRV_CPSS_FIELD_MAC( 452,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE14_BIT1_E),
    PRV_CPSS_FIELD_MAC( 456,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE14_BIT2_E),
    PRV_CPSS_FIELD_MAC( 460,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE14_BIT3_E),
    PRV_CPSS_FIELD_MAC( 464,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE14_BIT4_E),
    PRV_CPSS_FIELD_MAC( 468,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE14_BIT5_E),
    PRV_CPSS_FIELD_MAC( 472,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE14_BIT6_E),
    PRV_CPSS_FIELD_MAC( 476,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE14_BIT7_E),
    PRV_CPSS_FIELD_MAC( 480,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE15_BIT0_E),
    PRV_CPSS_FIELD_MAC( 484,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE15_BIT1_E),
    PRV_CPSS_FIELD_MAC( 488,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE15_BIT2_E),
    PRV_CPSS_FIELD_MAC( 492,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE15_BIT3_E),
    PRV_CPSS_FIELD_MAC( 496,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE15_BIT4_E),
    PRV_CPSS_FIELD_MAC( 500,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE15_BIT5_E),
    PRV_CPSS_FIELD_MAC( 504,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE15_BIT6_E),
    PRV_CPSS_FIELD_MAC( 508,   4, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE15_BIT7_E),
    PRV_CPSS_FIELD_MAC( 512,   3, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_HASH_CIRCULAR_SHIFT_LEFT_E),
    PRV_CPSS_FIELD_MAC( 515,   3, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_SIZE_E),
    PRV_CPSS_FIELD_MAC( 518,   1, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_UDP_SRC_PORT_MODE_E),
    PRV_CPSS_FIELD_MAC( 519,   5, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_SERVICE_ID_E),
    PRV_CPSS_FIELD_MAC( 524,   5, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_MAC_DA_MODE_E),
    PRV_CPSS_FIELD_MAC( 529,   5, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_PROFILE_DIP_MODE_E),
    PRV_CPSS_FIELD_MAC( 534,   3, SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_CONTROL_WORD_INDEX_E)
};

static const PRV_CPSS_ENTRY_FORMAT_TABLE_STC prvCpssDxChSip5HaNat44TableFieldsFormat[SIP5_HA_NAT44_TABLE_FIELDS____LAST_VALUE___E] =
{
    PRV_CPSS_FIELD_MAC(   0,  48, SIP5_HA_NAT44_TABLE_FIELDS_MAC_DA_E),
    PRV_CPSS_FIELD_MAC(  48,   1, SIP5_HA_NAT44_TABLE_FIELDS_MODIFY_DIP_E),
    PRV_CPSS_FIELD_MAC(  49,  32, SIP5_HA_NAT44_TABLE_FIELDS_NEW_DIP_E),
    PRV_CPSS_FIELD_MAC(  81,   1, SIP5_HA_NAT44_TABLE_FIELDS_MODIFY_SIP_E),
    PRV_CPSS_FIELD_MAC(  82,  32, SIP5_HA_NAT44_TABLE_FIELDS_NEW_SIP_E),
    PRV_CPSS_FIELD_MAC( 114,   1, SIP5_HA_NAT44_TABLE_FIELDS_MODIFY_TCP_UDP_DST_PORT_E),
    PRV_CPSS_FIELD_MAC( 115,  16, SIP5_HA_NAT44_TABLE_FIELDS_NEW_TCP_UDP_DST_PORT_E),
    PRV_CPSS_FIELD_MAC( 131,   1, SIP5_HA_NAT44_TABLE_FIELDS_MODIFY_TCP_UDP_SRC_PORT_E),
    PRV_CPSS_FIELD_MAC( 132,  16, SIP5_HA_NAT44_TABLE_FIELDS_NEW_TCP_UDP_SRC_PORT_E),
    PRV_CPSS_FIELD_MAC( 148,   5, SIP6_30_HA_NAT44_TABLE_FIELDS_NAT_SIP_PREFIX_LENGTH_E),
    PRV_CPSS_FIELD_MAC( 153,   5, SIP6_30_HA_NAT44_TABLE_FIELDS_NAT_DIP_PREFIX_LENGTH_E),
    PRV_CPSS_FIELD_MAC( 190,   2, SIP5_HA_NAT44_TABLE_FIELDS_NAT_ENTRY_TYPE_E)
};

static const PRV_CPSS_ENTRY_FORMAT_TABLE_STC prvCpssDxChSip5HaNat66TableFieldsFormat[SIP5_HA_NAT66_TABLE_FIELDS____LAST_VALUE___E] =
{
    PRV_CPSS_FIELD_MAC(   0,  48, SIP5_HA_NAT66_TABLE_FIELDS_MAC_DA_E),
    PRV_CPSS_FIELD_MAC(  48,   2, SIP5_HA_NAT66_TABLE_FIELDS_MODIFY_COMMAND_E),
    PRV_CPSS_FIELD_MAC(  50, 128, SIP5_HA_NAT66_TABLE_FIELDS_ADDRESS_E),
    PRV_CPSS_FIELD_MAC( 178,   6, SIP5_HA_NAT66_TABLE_FIELDS_PREFIX_SIZE_E),
    PRV_CPSS_FIELD_MAC( 190,   2, SIP5_HA_NAT66_TABLE_FIELDS_NAT_ENTRY_TYPE_E)
};

static const PRV_CPSS_ENTRY_FORMAT_TABLE_STC prvCpssDxChSip6HaNat66TableFieldsFormat[SIP5_HA_NAT66_TABLE_FIELDS____LAST_VALUE___E] =
{
    PRV_CPSS_FIELD_MAC(   0,  48, SIP5_HA_NAT66_TABLE_FIELDS_MAC_DA_E),
    PRV_CPSS_FIELD_MAC(  48,   3, SIP5_HA_NAT66_TABLE_FIELDS_MODIFY_COMMAND_E),
    PRV_CPSS_FIELD_MAC(  51, 128, SIP5_HA_NAT66_TABLE_FIELDS_ADDRESS_E),
    PRV_CPSS_FIELD_MAC( 179,   6, SIP5_HA_NAT66_TABLE_FIELDS_PREFIX_SIZE_E),
    PRV_CPSS_FIELD_MAC( 190,   2, SIP5_HA_NAT66_TABLE_FIELDS_NAT_ENTRY_TYPE_E)
};

static const PRV_CPSS_ENTRY_FORMAT_TABLE_STC prvCpssDxChSip5IpclActionTableFieldsFormat[SIP5_IPCL_ACTION_TABLE_FIELDS___LAST_VALUE___E] =
{
    PRV_CPSS_FIELD_MAC(   0,   3, SIP5_IPCL_ACTION_TABLE_FIELDS_COMMAND_E),
    PRV_CPSS_FIELD_MAC(   3,   8, SIP5_IPCL_ACTION_TABLE_FIELDS_CPU_CODE_E),
    PRV_CPSS_FIELD_MAC(  11,   1, SIP5_IPCL_ACTION_TABLE_FIELDS_MIRROR_TO_ANALYZER_PORT_0_E),
    PRV_CPSS_FIELD_MAC(  12,   3, SIP5_IPCL_ACTION_TABLE_FIELDS_REDIRECT_COMMAND_E),
    PRV_CPSS_FIELD_MAC(  15,  32, SIP5_IPCL_ACTION_TABLE_FIELDS_RESERVED_46_15_E),
    PRV_CPSS_FIELD_MAC(  47,   1, SIP5_IPCL_ACTION_TABLE_FIELDS_POLICY1_LOOKUP_MODE_E),
    PRV_CPSS_FIELD_MAC(  48,   1, SIP5_IPCL_ACTION_TABLE_FIELDS_POLICY2_LOOKUP_MODE_E),
    PRV_CPSS_FIELD_MAC(  49,  13, SIP5_IPCL_ACTION_TABLE_FIELDS_IPCL_PROFILE_INDEX_E),
    PRV_CPSS_FIELD_MAC(  15,   3, SIP5_IPCL_ACTION_TABLE_FIELDS_RESERVED_17_15_E),
    PRV_CPSS_FIELD_MAC(  18,  16, SIP5_IPCL_ACTION_TABLE_FIELDS_VIDX_E),
    PRV_CPSS_FIELD_MAC(  18,   1, SIP5_IPCL_ACTION_TABLE_FIELDS_IS_TRUNK_E),
    PRV_CPSS_FIELD_MAC(  19,  13, SIP5_IPCL_ACTION_TABLE_FIELDS_TRG_PORT_E),
    PRV_CPSS_FIELD_MAC(  32,  10, SIP5_IPCL_ACTION_TABLE_FIELDS_TARGET_DEVICE_E),
    PRV_CPSS_FIELD_MAC(  19,  12, SIP5_IPCL_ACTION_TABLE_FIELDS_TRUNK_ID_E),
    PRV_CPSS_FIELD_MAC(  42,   1, SIP5_IPCL_ACTION_TABLE_FIELDS_USE_VIDX_E),
    PRV_CPSS_FIELD_MAC(  43,   1, SIP5_IPCL_ACTION_TABLE_FIELDS_VNT_L2_ECHO_E),
    PRV_CPSS_FIELD_MAC(  44,   1, SIP5_IPCL_ACTION_TABLE_FIELDS_TUNNEL_START_E),
    PRV_CPSS_FIELD_MAC(  45,  17, SIP5_IPCL_ACTION_TABLE_FIELDS_ARP_POINTER_E),
    PRV_CPSS_FIELD_MAC(  45,  15, SIP5_IPCL_ACTION_TABLE_FIELDS_TUNNEL_POINTER_E),
    PRV_CPSS_FIELD_MAC(  60,   1, SIP5_IPCL_ACTION_TABLE_FIELDS_TUNNEL_START_PASSENGER_TYPE_E),
    PRV_CPSS_FIELD_MAC(  15,  16, SIP5_IPCL_ACTION_TABLE_FIELDS_IP_NEXT_HOP_ENTRY_INDEX_E),
    PRV_CPSS_FIELD_MAC(  15,  12, SIP5_IPCL_ACTION_TABLE_FIELDS_VRF_ID_E),
    PRV_CPSS_FIELD_MAC(  15,  30, SIP5_IPCL_ACTION_TABLE_FIELDS_MAC_SA_29_0_E),
    PRV_CPSS_FIELD_MAC(  62,   1, SIP5_IPCL_ACTION_TABLE_FIELDS_BIND_TO_CNC_COUNTER_E),
    PRV_CPSS_FIELD_MAC(  63,  14, SIP5_IPCL_ACTION_TABLE_FIELDS_CNC_COUNTER_INDEX_E),
    PRV_CPSS_FIELD_MAC(  77,  18, SIP5_IPCL_ACTION_TABLE_FIELDS_MAC_SA_47_30_E),
    PRV_CPSS_FIELD_MAC(  77,   1, SIP5_IPCL_ACTION_TABLE_FIELDS_ACTIVATE_METER_E),
    PRV_CPSS_FIELD_MAC(  78,   1, SIP5_IPCL_ACTION_TABLE_FIELDS_BIND_TO_POLICER_COUNTER_E),
    PRV_CPSS_FIELD_MAC(  79,  16, SIP5_IPCL_ACTION_TABLE_FIELDS_POLICER_PTR_E),
    PRV_CPSS_FIELD_MAC(  95,   1, SIP5_IPCL_ACTION_TABLE_FIELDS_SOURCE_ID_SET_ENABLE_E),
    PRV_CPSS_FIELD_MAC(  96,  12, SIP5_IPCL_ACTION_TABLE_FIELDS_SOURCE_ID_E),
    PRV_CPSS_FIELD_MAC( 108,   1, SIP5_IPCL_ACTION_TABLE_FIELDS_ACTION_STOP_E),
    PRV_CPSS_FIELD_MAC( 109,   1, SIP5_IPCL_ACTION_TABLE_FIELDS_BRIDGE_BYPASS_E),
    PRV_CPSS_FIELD_MAC( 110,   1, SIP5_IPCL_ACTION_TABLE_FIELDS_BYPASS_INGRESS_PIPE_E),
    PRV_CPSS_FIELD_MAC( 111,   1, SIP5_IPCL_ACTION_TABLE_FIELDS_VLAN_PRECEDENCE_E),
    PRV_CPSS_FIELD_MAC( 112,   1, SIP5_IPCL_ACTION_TABLE_FIELDS_EN_NESTED_VLAN_E),
    PRV_CPSS_FIELD_MAC( 113,   2, SIP5_IPCL_ACTION_TABLE_FIELDS_VLAN_COMMAND_E),
    PRV_CPSS_FIELD_MAC( 115,  13, SIP5_IPCL_ACTION_TABLE_FIELDS_VID0_E),
    PRV_CPSS_FIELD_MAC( 128,   1, SIP5_IPCL_ACTION_TABLE_FIELDS_QOS_PROFILE_MARKING_ENABLE_E),
    PRV_CPSS_FIELD_MAC( 129,   1, SIP5_IPCL_ACTION_TABLE_FIELDS_QOS_PRECEDENCE_E),
    PRV_CPSS_FIELD_MAC( 130,  10, SIP5_IPCL_ACTION_TABLE_FIELDS_QOS_PROFILE_E),
    PRV_CPSS_FIELD_MAC( 140,   2, SIP5_IPCL_ACTION_TABLE_FIELDS_MODIFY_DSCP_E),
    PRV_CPSS_FIELD_MAC( 142,   2, SIP5_IPCL_ACTION_TABLE_FIELDS_MODIFY_UP_E),
    PRV_CPSS_FIELD_MAC( 144,   1, SIP5_IPCL_ACTION_TABLE_FIELDS_ENABLE_MIRROR_TCP_RST_OR_FIN_E),
    PRV_CPSS_FIELD_MAC( 145,   2, SIP5_IPCL_ACTION_TABLE_FIELDS_MIRROR_TO_ANALYZER_PORT_2_1_E),
    PRV_CPSS_FIELD_MAC( 147,   1, SIP5_IPCL_ACTION_TABLE_FIELDS_MODIFY_MAC_DA_E),
    PRV_CPSS_FIELD_MAC( 148,   1, SIP5_IPCL_ACTION_TABLE_FIELDS_MODIFY_MAC_SA_E),
    PRV_CPSS_FIELD_MAC( 148,   1, SIP5_IPCL_ACTION_TABLE_FIELDS_SET_EGRESS_FILTER_REGISTERED_E),
    PRV_CPSS_FIELD_MAC( 149,   2, SIP5_IPCL_ACTION_TABLE_FIELDS_VLAN1_CMD_E),
    PRV_CPSS_FIELD_MAC( 151,   2, SIP5_IPCL_ACTION_TABLE_FIELDS_UP1_CMD_E),
    PRV_CPSS_FIELD_MAC( 153,  12, SIP5_IPCL_ACTION_TABLE_FIELDS_VID1_E),
    PRV_CPSS_FIELD_MAC( 165,   3, SIP5_IPCL_ACTION_TABLE_FIELDS_UP1_E),
    PRV_CPSS_FIELD_MAC( 168,  12, SIP5_IPCL_ACTION_TABLE_FIELDS_FLOW_ID_E),
    PRV_CPSS_FIELD_MAC( 180,   1, SIP5_IPCL_ACTION_TABLE_FIELDS_IPCL_RESERVED_EN_E),
    PRV_CPSS_FIELD_MAC( 181,  20, SIP5_IPCL_ACTION_TABLE_FIELDS_IPCL_TO_TXQ_RESERVED_E),
    PRV_CPSS_FIELD_MAC( 201,   1, SIP5_IPCL_ACTION_TABLE_FIELDS_SET_MAC2ME_E),
    PRV_CPSS_FIELD_MAC( 202,   1, SIP5_IPCL_ACTION_TABLE_FIELDS_TIMESTAMP_EN_E),
    PRV_CPSS_FIELD_MAC( 203,   7, SIP5_IPCL_ACTION_TABLE_FIELDS_OFFSET_INDEX_E),
    PRV_CPSS_FIELD_MAC( 210,   1, SIP5_IPCL_ACTION_TABLE_FIELDS_OAM_PROCESSING_EN_E),
    PRV_CPSS_FIELD_MAC( 211,   1, SIP5_IPCL_ACTION_TABLE_FIELDS_OAM_PROFILE_E),
    PRV_CPSS_FIELD_MAC( 212,   1, SIP5_IPCL_ACTION_TABLE_FIELDS_ASSIGN_SRC_EPORT_EN_E),
    PRV_CPSS_FIELD_MAC( 213,  13, SIP5_IPCL_ACTION_TABLE_FIELDS_SRC_EPORT_E),
    PRV_CPSS_FIELD_MAC( 226,   1, SIP5_IPCL_ACTION_TABLE_FIELDS_USER_AC_ENABLE_E)
};

static const PRV_CPSS_ENTRY_FORMAT_TABLE_STC prvCpssDxChSip5EpclActionTableFieldsFormat[SIP5_EPCL_ACTION_TABLE_FIELDS___LAST_VALUE___E] =
{
    PRV_CPSS_FIELD_MAC(   0,   3, SIP5_EPCL_ACTION_TABLE_FIELDS_COMMAND_E),
    PRV_CPSS_FIELD_MAC(   3,   6, SIP5_EPCL_ACTION_TABLE_FIELDS_DSCP_EXP_E),
    PRV_CPSS_FIELD_MAC(   9,   3, SIP5_EPCL_ACTION_TABLE_FIELDS_UP0_E),
    PRV_CPSS_FIELD_MAC(  12,   2, SIP5_EPCL_ACTION_TABLE_FIELDS_MODIFY_DSCP_EXP_E),
    PRV_CPSS_FIELD_MAC(  14,   2, SIP5_EPCL_ACTION_TABLE_FIELDS_ENABLE_MODIFY_UP0_E),
    PRV_CPSS_FIELD_MAC(  16,   1, SIP5_EPCL_ACTION_TABLE_FIELDS_BIND_TO_CNC_COUNTER_E),
    PRV_CPSS_FIELD_MAC(  17,  14, SIP5_EPCL_ACTION_TABLE_FIELDS_CNC_COUNTER_INDEX_E),
    PRV_CPSS_FIELD_MAC(  31,   1, SIP5_EPCL_ACTION_TABLE_FIELDS_TAG1_VID_CMD_E),
    PRV_CPSS_FIELD_MAC(  32,   1, SIP5_EPCL_ACTION_TABLE_FIELDS_ENABLE_MODIFY_UP1_E),
    PRV_CPSS_FIELD_MAC(  33,  12, SIP5_EPCL_ACTION_TABLE_FIELDS_TAG1_VID_E),
    PRV_CPSS_FIELD_MAC(  45,   3, SIP5_EPCL_ACTION_TABLE_FIELDS_UP1_E),
    PRV_CPSS_FIELD_MAC(  48,   2, SIP5_EPCL_ACTION_TABLE_FIELDS_TAG0_VLAN_CMD_E),
    PRV_CPSS_FIELD_MAC(  50,  12, SIP5_EPCL_ACTION_TABLE_FIELDS_TAG0_VID_E),
    PRV_CPSS_FIELD_MAC(  62,   1, SIP5_EPCL_ACTION_TABLE_FIELDS_BIND_TO_POLICER_COUNTER_E),
    PRV_CPSS_FIELD_MAC(  63,   1, SIP5_EPCL_ACTION_TABLE_FIELDS_BIND_TO_POLICER_METER_E),
    PRV_CPSS_FIELD_MAC(  64,  13, SIP5_EPCL_ACTION_TABLE_FIELDS_POLICER_INDEX_E),
    PRV_CPSS_FIELD_MAC(  77,   3, SIP5_EPCL_ACTION_TABLE_FIELDS_RESERVED_E),
    PRV_CPSS_FIELD_MAC(  80,  12, SIP5_EPCL_ACTION_TABLE_FIELDS_FLOW_ID_E),
    PRV_CPSS_FIELD_MAC(  92,   1, SIP5_EPCL_ACTION_TABLE_FIELDS_OAM_PROCESSING_ENABLE_E),
    PRV_CPSS_FIELD_MAC(  93,   1, SIP5_EPCL_ACTION_TABLE_FIELDS_OAM_PROFILE_E),
    PRV_CPSS_FIELD_MAC(  94,   1, SIP5_EPCL_ACTION_TABLE_FIELDS_TIME_STAMP_ENABLE_E),
    PRV_CPSS_FIELD_MAC(  95,   7, SIP5_EPCL_ACTION_TABLE_FIELDS_TIME_STAMP_OFFSET_INDEX_E),
    PRV_CPSS_FIELD_MAC( 102,   1, SIP5_EPCL_ACTION_TABLE_FIELDS_CHANNEL_TYPE_TO_OPCODE_MAPPING_EN_E),
    PRV_CPSS_FIELD_MAC( 103,   1, SIP5_EPCL_ACTION_TABLE_FIELDS_EPCL_RESERVED_ENABLE_E),
    PRV_CPSS_FIELD_MAC( 104,  20, SIP5_EPCL_ACTION_TABLE_FIELDS_EPCL_RESERVED_E),
    PRV_CPSS_FIELD_MAC( 124,  14, SIP5_EPCL_ACTION_TABLE_FIELDS_TM_QUEUE_ID_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_EPCL_ACTION_TABLE_FIELDS_PHA_METADATA_ASSIGN_ENABLE_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_EPCL_ACTION_TABLE_FIELDS_PHA_METADATA_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_EPCL_ACTION_TABLE_FIELDS_PHA_THREAD_NUMBER_ASSIGN_ENABLE_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_EPCL_ACTION_TABLE_FIELDS_PHA_THREAD_NUMBER_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_EPCL_ACTION_TABLE_FIELDS_EGRESS_MIRRORING_MODE_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_EPCL_ACTION_TABLE_FIELDS_EGRESS_ANALYZER_INDEX_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_EPCL_ACTION_TABLE_FIELDS_CPU_CODE_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_EPCL_ACTION_TABLE_FIELDS_LATENCY_PROFILE_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_EPCL_ACTION_TABLE_FIELDS_LATENCY_MONITORING_ENABLE_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_EPCL_ACTION_TABLE_FIELDS_CUT_THROUGH_TERMINATE_ID_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_EPCL_ACTION_TABLE_FIELDS_RESERVED_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_EPCL_ACTION_TABLE_FIELDS_PCL_OVER_EXACT_MATCH_ENABLE_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_10_EPCL_ACTION_TABLE_FIELDS_IPFIX_ENABLE_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_10_EPCL_ACTION_TABLE_FIELDS_PHA_METADATA_PART2_E)
};

static const PRV_CPSS_ENTRY_FORMAT_TABLE_STC prvCpssDxChSip5IpvxRouterNextHopTableFieldsFormat[SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS___LAST_VALUE___E] =
{
    PRV_CPSS_FIELD_MAC(   0,   3, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_COMMAND_E),
    PRV_CPSS_FIELD_MAC(   3,   1, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_BYPASS_TTL_OPTIONS_OR_HOP_EXTENSION_E),
    PRV_CPSS_FIELD_MAC(   4,   2, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_CPU_CODE_INDEX_E),
    PRV_CPSS_FIELD_MAC(   6,   3, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_INGRESS_MIRROR_TO_ANALYZER_INDEX_E),
    PRV_CPSS_FIELD_MAC(   9,   1, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_QOS_PROFILE_PRECEDENCE_E),
    PRV_CPSS_FIELD_MAC(  10,   1, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_QOS_PROFILE_MARKING_EN_E),
    PRV_CPSS_FIELD_MAC(  11,   7, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_QOS_PROFILE_INDEX_E),
    PRV_CPSS_FIELD_MAC(  18,   2, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_MODIFY_DSCP_E),
    PRV_CPSS_FIELD_MAC(  20,   2, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_MODIFY_UP_E),
    PRV_CPSS_FIELD_MAC(  22,  13, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_MC_RPF_EVLAN_E),
    PRV_CPSS_FIELD_MAC(  22,   6, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_MC_RPF_EVLAN_MRST_INDEX_E),
    PRV_CPSS_FIELD_MAC(  28,   7, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_RESERVED_28_34_E),
    PRV_CPSS_FIELD_MAC(  35,  13, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_RESERVED_35_47_E),
    PRV_CPSS_FIELD_MAC(  48,   3, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_MULTICAST_INGRESS_VLAN_CHECK_FAIL_CMD_E),
    PRV_CPSS_FIELD_MAC(  51,   1, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_MULTICAST_RPF_FAIL_CMD_MODE_E),
    PRV_CPSS_FIELD_MAC(  52,   1, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_RESERVED_52_E),
    PRV_CPSS_FIELD_MAC(  53,  16, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_INTERNAL_MLL_PTR_E),
    PRV_CPSS_FIELD_MAC(  69,   3, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_EXTERNAL_MLL_PTR_MSB_BITS_15_13_E),
    PRV_CPSS_FIELD_MAC(  72,   3, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_RESERVED_72_74_E),
    PRV_CPSS_FIELD_MAC(  75,   2, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_MULTICAST_INGRESS_VLAN_CHECK_E),
    PRV_CPSS_FIELD_MAC(  77,  13, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_EXTERNAL_MLL_PTR_E),
    PRV_CPSS_FIELD_MAC(  90,   1, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_IPV6_MULTICAST_RESERVED_90_E),
    PRV_CPSS_FIELD_MAC(  91,   1, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_IPV6_MULTICAST_DEST_SITE_ID_E),
    PRV_CPSS_FIELD_MAC(  92,   1, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_IPV6_MULTICAST_SCOPE_CHECK_EN_E),
    PRV_CPSS_FIELD_MAC(   9,  12, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_NEXT_HOP_VID1_E),
    PRV_CPSS_FIELD_MAC(  21,   1, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_RESERVED_21_E),
    PRV_CPSS_FIELD_MAC(  22,  13, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_NEXT_HOP_EVLAN_E),
    PRV_CPSS_FIELD_MAC(  35,   1, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_USE_VIDX_E),
    PRV_CPSS_FIELD_MAC(  36,  16, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_EVIDX_E),
    PRV_CPSS_FIELD_MAC(  52,   8, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_RESERVED_52_59_E),
    PRV_CPSS_FIELD_MAC(  36,   1, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_TARGET_IS_TRUNK_E),
    PRV_CPSS_FIELD_MAC(  37,  13, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_TRG_EPORT_E),
    PRV_CPSS_FIELD_MAC(  50,  10, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_TRG_DEV_E),
    PRV_CPSS_FIELD_MAC(  37,  12, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_TRG_TRUNK_ID_E),
    PRV_CPSS_FIELD_MAC(  49,  11, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_RESERVED_49_59_E),
    PRV_CPSS_FIELD_MAC(  60,   6, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_DIP_ACCESS_LEVEL_E),
    PRV_CPSS_FIELD_MAC(  66,   6, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_SIP_ACCESS_LEVEL_E),
    PRV_CPSS_FIELD_MAC(  72,   1, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_START_OF_TUNNEL_E),
    PRV_CPSS_FIELD_MAC(  73,  17, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_ARP_PTR_E),
    PRV_CPSS_FIELD_MAC(  73,  15, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_TUNNEL_PTR_E),
    PRV_CPSS_FIELD_MAC(  88,   1, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_RESERVED_88_E),
    PRV_CPSS_FIELD_MAC(  89,   1, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_UNICAST_TS_IS_NAT_E),
    PRV_CPSS_FIELD_MAC(  90,   1, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_UNICAST_TUNNEL_TYPE_E),
    PRV_CPSS_FIELD_MAC(  91,   1, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_UNICAST_DEST_SITE_ID_E),
    PRV_CPSS_FIELD_MAC(  92,   1, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_UNICAST_SCOPE_CHECK_EN_E),
    PRV_CPSS_FIELD_MAC(  93,   3, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_COUNTER_SET_INDEX_E),
    PRV_CPSS_FIELD_MAC(  96,   3, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_MTU_INDEX_E),
    PRV_CPSS_FIELD_MAC(  99,   1, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_ARP_BC_TRAP_MIRROR_EN_E),
    PRV_CPSS_FIELD_MAC( 100,   1, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_APP_SPECIFIC_CPU_CODE_EN_E),
    PRV_CPSS_FIELD_MAC( 101,   1, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_RESERVED_101_E),
    PRV_CPSS_FIELD_MAC( 101,   1, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_SIP_FILTER_EN_E),
    PRV_CPSS_FIELD_MAC( 102,   1, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_DEC_TTL_OR_HOP_COUNT_E),
    PRV_CPSS_FIELD_MAC( 103,   1, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_ICMP_REDIRECT_EXCEP_MIRROR_E)
};

static const PRV_CPSS_ENTRY_FORMAT_TABLE_STC prvCpssDxChSip5IpvxRouterEportTableFieldsFormat[SIP5_IPVX_ROUTER_EPORT_TABLE_FIELDS___LAST_VALUE___E] =
{
    PRV_CPSS_FIELD_MAC(   0,   1, SIP5_IPVX_ROUTER_EPORT_TABLE_FIELDS_ROUTER_PER_EPORT_SIP_SA_CHECK_ENABLE_E),
    PRV_CPSS_FIELD_MAC(   1,   1, SIP5_IPVX_ROUTER_EPORT_TABLE_FIELDS_FCOE_FORWARDING_ENABLE_E),
    PRV_CPSS_FIELD_MAC(   2,   1, SIP5_IPVX_ROUTER_EPORT_TABLE_FIELDS_IPV4_UC_ROUTING_ENABLE_E),
    PRV_CPSS_FIELD_MAC(   3,   1, SIP5_IPVX_ROUTER_EPORT_TABLE_FIELDS_IPV4_MC_ROUTING_ENABLE_E),
    PRV_CPSS_FIELD_MAC(   4,   1, SIP5_IPVX_ROUTER_EPORT_TABLE_FIELDS_IPV6_UC_ROUTING_ENABLE_E),
    PRV_CPSS_FIELD_MAC(   5,   1, SIP5_IPVX_ROUTER_EPORT_TABLE_FIELDS_IPV6_MC_ROUTING_ENABLE_E),
    PRV_CPSS_FIELD_MAC(   6,   1, SIP5_IPVX_ROUTER_EPORT_TABLE_FIELDS_URPF_ENABLE_E),
    PRV_CPSS_FIELD_MAC(   7,   1, SIP5_IPVX_ROUTER_EPORT_TABLE_FIELDS_DISABLE_SIP_LOOKUP)
};

static const PRV_CPSS_ENTRY_FORMAT_TABLE_STC prvCpssDxChSip5PlrMeteringTableFieldsFormat[SIP5_PLR_METERING_TABLE_FIELDS___LAST_VALUE___E] =
{
    PRV_CPSS_FIELD_MAC(   0,  28, SIP5_PLR_METERING_TABLE_FIELDS_LAST_UPDATE_TIME0_E),
    PRV_CPSS_FIELD_MAC(  28,  28, SIP5_PLR_METERING_TABLE_FIELDS_LAST_UPDATE_TIME1_E),
    PRV_CPSS_FIELD_MAC(  56,   1, SIP5_PLR_METERING_TABLE_FIELDS_WRAP_AROUND_INDICATOR0_E),
    PRV_CPSS_FIELD_MAC(  57,   1, SIP5_PLR_METERING_TABLE_FIELDS_WRAP_AROUND_INDICATOR1_E),
    PRV_CPSS_FIELD_MAC(  58,  32, SIP5_PLR_METERING_TABLE_FIELDS_BUCKET_SIZE0_E),
    PRV_CPSS_FIELD_MAC(  90,  32, SIP5_PLR_METERING_TABLE_FIELDS_BUCKET_SIZE1_E),
    PRV_CPSS_FIELD_MAC( 122,   3, SIP5_PLR_METERING_TABLE_FIELDS_RATE_TYPE0_E),
    PRV_CPSS_FIELD_MAC( 125,   3, SIP5_PLR_METERING_TABLE_FIELDS_RATE_TYPE1_E),
    PRV_CPSS_FIELD_MAC( 128,  17, SIP5_PLR_METERING_TABLE_FIELDS_RATE0_E),
    PRV_CPSS_FIELD_MAC( 145,  17, SIP5_PLR_METERING_TABLE_FIELDS_RATE1_E),
    PRV_CPSS_FIELD_MAC( 162,  16, SIP5_PLR_METERING_TABLE_FIELDS_MAX_BURST_SIZE0_E),
    PRV_CPSS_FIELD_MAC( 178,  16, SIP5_PLR_METERING_TABLE_FIELDS_MAX_BURST_SIZE1_E),
    PRV_CPSS_FIELD_MAC( 194,   1, SIP5_PLR_METERING_TABLE_FIELDS_COLOR_MODE_E),
    PRV_CPSS_FIELD_MAC( 195,   2, SIP5_PLR_METERING_TABLE_FIELDS_POLICER_MODE_E),
    PRV_CPSS_FIELD_MAC( 197,   2, SIP5_PLR_METERING_TABLE_FIELDS_MG_COUNTERS_SET_EN_E),
    PRV_CPSS_FIELD_MAC( 199,  16, SIP5_PLR_METERING_TABLE_FIELDS_BILLING_PTR_E),
    PRV_CPSS_FIELD_MAC( 215,   1, SIP5_PLR_METERING_TABLE_FIELDS_BYTE_OR_PACKET_COUNTING_MODE_E),
    PRV_CPSS_FIELD_MAC( 216,   1, SIP5_PLR_METERING_TABLE_FIELDS_PACKET_SIZE_MODE_E),
    PRV_CPSS_FIELD_MAC( 217,   1, SIP5_PLR_METERING_TABLE_FIELDS_TUNNEL_TERMINATION_PACKET_SIZE_MODE_E),
    PRV_CPSS_FIELD_MAC( 218,   1, SIP5_PLR_METERING_TABLE_FIELDS_INCLUDE_LAYER1_OVERHEAD_E),
    PRV_CPSS_FIELD_MAC( 219,   1, SIP5_PLR_METERING_TABLE_FIELDS_DSA_TAG_COUNTING_MODE_E),
    PRV_CPSS_FIELD_MAC( 220,   1, SIP5_PLR_METERING_TABLE_FIELDS_TIMESTAMP_TAG_COUNTING_MODE_E),
    PRV_CPSS_FIELD_MAC( 221,   2, SIP5_PLR_METERING_TABLE_FIELDS_INGRESS_RED_CMD_E),
    PRV_CPSS_FIELD_MAC( 223,   2, SIP5_PLR_METERING_TABLE_FIELDS_INGRESS_YELLOW_CMD_E),
    PRV_CPSS_FIELD_MAC( 225,   2, SIP5_PLR_METERING_TABLE_FIELDS_INGRESS_GREEN_CMD_E),
    PRV_CPSS_FIELD_MAC( 227,   2, SIP5_PLR_METERING_TABLE_FIELDS_INGRESS_MODIFY_DSCP_E),
    PRV_CPSS_FIELD_MAC( 229,   2, SIP5_PLR_METERING_TABLE_FIELDS_INGRESS_MODIFY_UP_E),
    PRV_CPSS_FIELD_MAC( 231,  10, SIP5_PLR_METERING_TABLE_FIELDS_INGRESS_QOS_PROFILE_E),
    PRV_CPSS_FIELD_MAC( 221,   1, SIP5_PLR_METERING_TABLE_FIELDS_EGRESS_REMARK_MODE_E),
    PRV_CPSS_FIELD_MAC( 222,   1, SIP5_PLR_METERING_TABLE_FIELDS_EGRESS_DROP_RED_E),
    PRV_CPSS_FIELD_MAC( 223,   1, SIP5_PLR_METERING_TABLE_FIELDS_EGRESS_EN_MODIFY_EXP_E),
    PRV_CPSS_FIELD_MAC( 224,   2, SIP5_PLR_METERING_TABLE_FIELDS_EGRESS_EN_MODIFY_DSCP_E),
    PRV_CPSS_FIELD_MAC( 226,   1, SIP5_PLR_METERING_TABLE_FIELDS_EGRESS_EN_MODIFY_TC_E),
    PRV_CPSS_FIELD_MAC( 227,   2, SIP5_PLR_METERING_TABLE_FIELDS_EGRESS_EN_MODIFY_UP_E),
    PRV_CPSS_FIELD_MAC( 229,   1, SIP5_PLR_METERING_TABLE_FIELDS_EGRESS_EN_MODIFY_DP_E),
    PRV_CPSS_FIELD_MAC( 230,   1, SIP5_PLR_METERING_TABLE_FIELDS_EGRESS_EN_YELLOW_ECN_MARKING_E),
    PRV_CPSS_FIELD_MAC( 194,   3, SIP5_15_PLR_METERING_TABLE_FIELDS_BUCKET_0_PRIORITY_E),
    PRV_CPSS_FIELD_MAC( 197,   3, SIP5_15_PLR_METERING_TABLE_FIELDS_BUCKET_1_PRIORITY_E),
    PRV_CPSS_FIELD_MAC( 200,   1, SIP5_15_PLR_METERING_TABLE_FIELDS_BUCKET_0_COLOR_E),
    PRV_CPSS_FIELD_MAC( 201,   1, SIP5_15_PLR_METERING_TABLE_FIELDS_BUCKET_1_COLOR_E),
    PRV_CPSS_FIELD_MAC( 202,   1, SIP5_15_PLR_METERING_TABLE_FIELDS_COUPLING_FLAG_E),
    PRV_CPSS_FIELD_MAC( 203,   1, SIP5_15_PLR_METERING_TABLE_FIELDS_COUPLING_FLAG_0_E),
    PRV_CPSS_FIELD_MAC( 204,   7, SIP5_15_PLR_METERING_TABLE_FIELDS_EIR_AND_CIR_MAX_INDEX_E),
    PRV_CPSS_FIELD_MAC( 211,   0, SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_INGRESS_MEF_10_3_ENV_SIZE_E___EMULATION_ONLY_E),
    PRV_CPSS_FIELD_MAC( 211,   0, SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_EGRESS_MEF_10_3_ENV_SIZE_E___EMULATION_ONLY_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP_6_PLR_METERING_CONFIG_TABLE_FIELDS_INGRESS_GREEN_MIRROR_EN___EMULATION_ONLY_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP_6_PLR_METERING_CONFIG_TABLE_FIELDS_INGRESS_YELLOW_MIRROR_EN___EMULATION_ONLY_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP_6_PLR_METERING_CONFIG_TABLE_FIELDS_INGRESS_RED_MIRROR_EN___EMULATION_ONLY_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP_6_PLR_METERING_CONFIG_TABLE_FIELDS_EGRESS_GREEN_MIRROR_EN___EMULATION_ONLY_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP_6_PLR_METERING_CONFIG_TABLE_FIELDS_EGRESS_YELLOW_MIRROR_EN___EMULATION_ONLY_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP_6_PLR_METERING_CONFIG_TABLE_FIELDS_EGRESS_RED_MIRROR_EN___EMULATION_ONLY_E)
};

static const PRV_CPSS_ENTRY_FORMAT_TABLE_STC prvCpssDxChSip5_15PlrMeteringConfigurationTableFieldsFormat[SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS___LAST_VALUE___E] =
{
    PRV_CPSS_FIELD_MAC(   0,   1, SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_COLOR_MODE_E),
    PRV_CPSS_FIELD_MAC(   1,   3, SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_POLICER_MODE_E),
    PRV_CPSS_FIELD_MAC(   4,   2, SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_MG_COUNTERS_SET_EN_E),
    PRV_CPSS_FIELD_MAC(   6,  16, SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_BILLING_PTR_E),
    PRV_CPSS_FIELD_MAC(  22,   1, SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_BYTE_OR_PACKET_COUNTING_MODE_E),
    PRV_CPSS_FIELD_MAC(  23,   1, SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_PACKET_SIZE_MODE_E),
    PRV_CPSS_FIELD_MAC(  24,   1, SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_TUNNEL_TERMINATION_PACKET_SIZE_MODE_E),
    PRV_CPSS_FIELD_MAC(  25,   1, SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_INCLUDE_LAYER1_OVERHEAD_E),
    PRV_CPSS_FIELD_MAC(  26,   1, SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_DSA_TAG_COUNTING_MODE_E),
    PRV_CPSS_FIELD_MAC(  27,   1, SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_TIMESTAMP_TAG_COUNTING_MODE_E),
    PRV_CPSS_FIELD_MAC(  28,   2, SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_INGRESS_RED_CMD_E),
    PRV_CPSS_FIELD_MAC(  30,   2, SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_INGRESS_YELLOW_CMD_E),
    PRV_CPSS_FIELD_MAC(  32,   2, SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_INGRESS_GREEN_CMD_E),
    PRV_CPSS_FIELD_MAC(  34,   2, SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_INGRESS_MODIFY_DSCP_E),
    PRV_CPSS_FIELD_MAC(  36,   2, SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_INGRESS_MODIFY_UP_E),
    PRV_CPSS_FIELD_MAC(  38,  10, SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_INGRESS_QOS_PROFILE_E),
    PRV_CPSS_FIELD_MAC(  28,   1, SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_EGRESS_REMARK_MODE_E),
    PRV_CPSS_FIELD_MAC(  29,   1, SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_EGRESS_DROP_RED_E),
    PRV_CPSS_FIELD_MAC(  30,   1, SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_EGRESS_EN_MODIFY_EXP_E),
    PRV_CPSS_FIELD_MAC(  31,   2, SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_EGRESS_EN_MODIFY_DSCP_E),
    PRV_CPSS_FIELD_MAC(  33,   1, SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_EGRESS_EN_MODIFY_TC_E),
    PRV_CPSS_FIELD_MAC(  34,   2, SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_EGRESS_EN_MODIFY_UP_E),
    PRV_CPSS_FIELD_MAC(  36,   1, SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_EGRESS_EN_MODIFY_DP_E),
    PRV_CPSS_FIELD_MAC(  37,   1, SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_EGRESS_EN_YELLOW_ECN_MARKING_E),
    PRV_CPSS_FIELD_MAC(  48,   3, SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_INGRESS_MEF_10_3_ENV_SIZE_E),
    PRV_CPSS_FIELD_MAC(  48,   3, SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_EGRESS_MEF_10_3_ENV_SIZE_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP_6_PLR_METERING_CONFIG_TABLE_FIELDS_INGRESS_GREEN_MIRROR_EN_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP_6_PLR_METERING_CONFIG_TABLE_FIELDS_INGRESS_YELLOW_MIRROR_EN_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP_6_PLR_METERING_CONFIG_TABLE_FIELDS_INGRESS_RED_MIRROR_EN_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP_6_PLR_METERING_CONFIG_TABLE_FIELDS_EGRESS_GREEN_MIRROR_EN_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP_6_PLR_METERING_CONFIG_TABLE_FIELDS_EGRESS_YELLOW_MIRROR_EN_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP_6_PLR_METERING_CONFIG_TABLE_FIELDS_EGRESS_RED_MIRROR_EN_E)
};

static const PRV_CPSS_ENTRY_FORMAT_TABLE_STC prvCpssDxChPlrMeteringConformanceSignTableFieldsFormat[PRV_CPSS_DXCH_PLR_METERING_CONFORM_SIGN_TABLE_FIELDS___LAST_VALUE___E] =
{
    PRV_CPSS_FIELD_MAC(   0,   1, PRV_CPSS_DXCH_PLR_METERING_CONFORM_SIGN_TABLE_FIELDS_BUCKET0_SIGN_E),
    PRV_CPSS_FIELD_MAC(   1,   1, PRV_CPSS_DXCH_PLR_METERING_CONFORM_SIGN_TABLE_FIELDS_BUCKET1_SIGN_E)
};


/* Number of bits in table pattern entry */
#define BOBCAT2_IPVX_ROUTER_EPORT_TABLE_PATTERN_BITS_CNS         8

/* Number of ports (patterns) per table entry */
#define BOBCAT2_IPVX_ROUTER_EPORT_TABLE_PATTERN_NUM_CNS         16

/* BOBCAT2 B0 TABLES - Start */
static const PRV_CPSS_ENTRY_FORMAT_TABLE_STC prvCpssDxChBobcat2B0TtiPhysicalPortTableFieldsFormat[SIP6_TTI_PHYSICAL_PORT_TABLE_FIELDS_FLOW_TRACK_ENABLE_E] =
{
    PRV_CPSS_FIELD_MAC(   0,  13, SIP5_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_DEFAULT_SOURCE_EPORT_NUMBER_E),
    PRV_CPSS_FIELD_MAC(  13,   0, SIP5_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_IS_RING_CORE_PORT_E),
    PRV_CPSS_FIELD_MAC(  13,   1, SIP5_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_IS_LOOPED_E),
    PRV_CPSS_FIELD_MAC(  14,   1, SIP5_TTI_PHYSICAL_PORT_TABLE_FIELDS_ENABLE_MRU_CHECK_ON_CASCADE_PORT_E),
    PRV_CPSS_FIELD_MAC(  15,  13, SIP5_TTI_PHYSICAL_PORT_TABLE_FIELDS_CC_LINE_CARD_PORTS_DEFAULT_EPORT_BASE_E),
    PRV_CPSS_FIELD_MAC(  28,  13, SIP5_TTI_PHYSICAL_PORT_TABLE_FIELDS_CC_LINE_CARD_TRUNK_DEFAULT_EPORT_BASE_E),
    PRV_CPSS_FIELD_MAC(  41,   1, SIP5_TTI_PHYSICAL_PORT_TABLE_FIELDS_CENTRALIZED_CHASSIS_PORT_ENABLE_E),
    PRV_CPSS_FIELD_MAC(  42,  12, SIP5_TTI_PHYSICAL_PORT_TABLE_FIELDS_TRUNK_ID_E),
    PRV_CPSS_FIELD_MAC(  54,   1, SIP5_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_QOS_MODE_E)
};

static const PRV_CPSS_ENTRY_FORMAT_TABLE_STC prvCpssDxChBobcat2B0L2iIngressVlanTableFieldsFormat[SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS___LAST_VALUE___E] =
{
    PRV_CPSS_FIELD_MAC(   0,   1, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_VALID_E),
    PRV_CPSS_FIELD_MAC(   1,   1, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_NEW_SRC_ADDR_IS_NOT_SECURITY_BREACH_E),
    PRV_CPSS_FIELD_MAC(   2,   3, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_UNREGISTERED_NON_IP_MULTICAST_CMD_E),
    PRV_CPSS_FIELD_MAC(   5,   3, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_UNREGISTERED_IPV4_MULTICAST_CMD_E),
    PRV_CPSS_FIELD_MAC(   8,   3, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_UNREGISTERED_IPV6_MULTICAST_CMD_E),
    PRV_CPSS_FIELD_MAC(  11,   3, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_UNKNOWN_UNICAST_CMD_E),
    PRV_CPSS_FIELD_MAC(  14,   1, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV4_IGMP_TO_CPU_EN_E),
    PRV_CPSS_FIELD_MAC(  15,   1, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV4_IPM_BRIDGING_MODE_E),
    PRV_CPSS_FIELD_MAC(  16,   1, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV6_IPM_BRIDGING_MODE_E),
    PRV_CPSS_FIELD_MAC(  17,   3, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_MIRROR_TO_INGRESS_ANALYZER_E),
    PRV_CPSS_FIELD_MAC(  20,   1, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV6_ICMP_TO_CPU_EN_E),
    PRV_CPSS_FIELD_MAC(  21,   1, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV4_CONTROL_TO_CPU_EN_E),
    PRV_CPSS_FIELD_MAC(  22,   1, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV4_IPM_BRIDGING_EN_E),
    PRV_CPSS_FIELD_MAC(  23,   1, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV6_IPM_BRIDGING_EN_E),
    PRV_CPSS_FIELD_MAC(  24,   3, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_UNREGISTERED_IPV4_BC_CMD_E),
    PRV_CPSS_FIELD_MAC(  27,   3, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_UNREGISTERED_NON_IPV4_BC_CMD_E),
    PRV_CPSS_FIELD_MAC(  30,   1, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV4_UNICAST_ROUTE_EN_E),
    PRV_CPSS_FIELD_MAC(  31,   1, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV4_MULTICAST_ROUTE_EN_E),
    PRV_CPSS_FIELD_MAC(  32,   1, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV6_UNICAST_ROUTE_EN_E),
    PRV_CPSS_FIELD_MAC(  33,   1, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV6_MULTICAST_ROUTE_EN_E),
    PRV_CPSS_FIELD_MAC(  34,   1, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV6_SITEID_E),
    PRV_CPSS_FIELD_MAC(  35,   1, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_AUTO_LEARN_DIS_E),
    PRV_CPSS_FIELD_MAC(  36,   1, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_NA_MSG_TO_CPU_EN_E),
    PRV_CPSS_FIELD_MAC(  37,   3, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_MRU_INDEX_E),
    PRV_CPSS_FIELD_MAC(  40,   1, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_BC_UDP_TRAP_MIRROR_EN_E),
    PRV_CPSS_FIELD_MAC(  41,   1, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV6_CONTROL_TO_CPU_EN_E),
    PRV_CPSS_FIELD_MAC(  42,  16, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_FLOOD_EVIDX_E),
    PRV_CPSS_FIELD_MAC(  58,  12, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_VRF_ID_E),
    PRV_CPSS_FIELD_MAC(  70,   1, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_UC_LOCAL_EN_E),
    PRV_CPSS_FIELD_MAC(  71,   1, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_FLOOD_VIDX_MODE_E),
    PRV_CPSS_FIELD_MAC(  72,   3, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV4_MC_BC_TO_MIRROR_ANLYZER_IDX_E),
    PRV_CPSS_FIELD_MAC(  75,   3, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV6_MC_TO_MIRROR_ANALYZER_IDX_E),
    PRV_CPSS_FIELD_MAC(  78,  13, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_FID_E),
    PRV_CPSS_FIELD_MAC(  91,   3, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_UNKOWN_MAC_SA_CMD_E),
    PRV_CPSS_FIELD_MAC(  94,   1, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_FCOE_FORWARDING_EN_E),
    PRV_CPSS_FIELD_MAC(  95,   2, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_UNREG_IPM_EVIDX_MODE_E),
    PRV_CPSS_FIELD_MAC(  97,  16, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_UNREG_IPM_EVIDX_E),
    PRV_CPSS_FIELD_MAC( 113,   1, SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_FDB_LOOKUP_KEY_MODE_E)
};

static const PRV_CPSS_ENTRY_FORMAT_TABLE_STC prvCpssDxChBobcat2B0EgfQagEgressEPortTableFieldsFormat[SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS___LAST_VALUE___E] =
{
    PRV_CPSS_FIELD_MAC(   0,   1, SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_DP_TO_CFI_REMAP_ENABLE_E),
    PRV_CPSS_FIELD_MAC(   1,   3, SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_EGRESS_MIRROR_TO_ANALYZER_INDEX_E),
    PRV_CPSS_FIELD_MAC(   4,   1, SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_EPORT_TAG_STATE_MODE_E),
    PRV_CPSS_FIELD_MAC(   5,   3, SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_EPORT_TAG_STATE_E),
    PRV_CPSS_FIELD_MAC(   8,   1, SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_EVLAN_COMMAND_E),
    PRV_CPSS_FIELD_MAC(   9,   1, SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_VID1_COMMAND_E),
    PRV_CPSS_FIELD_MAC(  10,   1, SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_UP0_COMMAND_E),
    PRV_CPSS_FIELD_MAC(  11,   1, SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_UP1_COMMAND_E),
    PRV_CPSS_FIELD_MAC(  12,  13, SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_EVLAN_E),
    PRV_CPSS_FIELD_MAC(  25,  12, SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_VID1_E),
    PRV_CPSS_FIELD_MAC(  37,   3, SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_UP0_E),
    PRV_CPSS_FIELD_MAC(  40,   3, SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_UP1_E),
    PRV_CPSS_FIELD_MAC(  43,   1, SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_REMOVE_VLAN_TAG_1_IF_RX_WITHOUT_TAG_1_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_EGRESS_MIRRORING_MODE_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_30_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_REMOVE_VLAN_TAG_0_IF_RX_WITHOUT_TAG_0_E)
};

static const PRV_CPSS_ENTRY_FORMAT_TABLE_STC prvCpssDxChBobcat2B0HaEPort1TableFieldsFormat[SIP5_HA_EPORT_TABLE_1_FIELDS____LAST_VALUE___E] =
{
    PRV_CPSS_FIELD_MAC(   0,   2, SIP5_HA_EPORT_TABLE_1_FIELDS_VLAN_TRANS_EN_E),
    PRV_CPSS_FIELD_MAC(   2,   3, SIP5_HA_EPORT_TABLE_1_FIELDS_EGRESS_TAG0_TPID_INDEX_E),
    PRV_CPSS_FIELD_MAC(   5,   3, SIP5_HA_EPORT_TABLE_1_FIELDS_EGRESS_TAG1_TPID_INDEX_E),
    PRV_CPSS_FIELD_MAC(   8,   3, SIP5_HA_EPORT_TABLE_1_FIELDS_TS_HEADER_TPID_INDEX_E),
    PRV_CPSS_FIELD_MAC(  11,  32, SIP5_HA_EPORT_TABLE_1_FIELDS_TS_EXT_E),
    PRV_CPSS_FIELD_MAC(  11,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_MPLS_PW_LABEL_PUSH_ENABLE_E),
    PRV_CPSS_FIELD_MAC(  12,  20, SIP5_HA_EPORT_TABLE_1_FIELDS_MPLS_PW_LABEL_E),
    PRV_CPSS_FIELD_MAC(  32,   3, SIP5_HA_EPORT_TABLE_1_FIELDS_MPLS_PW_LABEL_EXP_E),
    PRV_CPSS_FIELD_MAC(  35,   8, SIP5_HA_EPORT_TABLE_1_FIELDS_MPLS_PW_LABEL_TTL_E),
    PRV_CPSS_FIELD_MAC(  43,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_PUSH_MPLS_FLOW_LABEL_ENABLE_E),
    PRV_CPSS_FIELD_MAC(  44,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_FORCE_ARP_TS_EPORT_DECISION_E),
    PRV_CPSS_FIELD_MAC(  45,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_TUNNEL_START_E),
    PRV_CPSS_FIELD_MAC(  46,  15, SIP5_HA_EPORT_TABLE_1_FIELDS_TS_POINTER_E),
    PRV_CPSS_FIELD_MAC(  46,  17, SIP5_HA_EPORT_TABLE_1_FIELDS_ARP_POINTER_E),
    PRV_CPSS_FIELD_MAC(  61,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_TS_PASSENGER_TYPE_E),
    PRV_CPSS_FIELD_MAC(  63,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_MODIFY_MAC_SA_E),
    PRV_CPSS_FIELD_MAC(  64,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_MODIFY_MAC_DA_E),
    PRV_CPSS_FIELD_MAC(  65,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_TRILL_INTERFACE_ENABLE_E),
    PRV_CPSS_FIELD_MAC(  66,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_KEEP_ORIGINAL_TAG0_TPID_E),
    PRV_CPSS_FIELD_MAC(  67,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_KEEP_ORIGINAL_TAG1_TPID_E),
    PRV_CPSS_FIELD_MAC(  68,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_ENABLE_EGRESS_UP_MAPPING_E),
    PRV_CPSS_FIELD_MAC(  69,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_ENABLE_EGRESS_DSCP_MAPPING_E),
    PRV_CPSS_FIELD_MAC(  70,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_ENABLE_EGRESS_EXP_MAPPING_E),
    PRV_CPSS_FIELD_MAC(  71,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_ENABLE_EGRESS_TC_DP_MAPPING_E),
    PRV_CPSS_FIELD_MAC(  72,   4, SIP5_HA_EPORT_TABLE_1_FIELDS_QOS_MAPPING_TABLE_INDEX_E),
    PRV_CPSS_FIELD_MAC(  76,   8, SIP5_HA_EPORT_TABLE_1_FIELDS_ROUTER_MAC_SA_INDEX_E),
    PRV_CPSS_FIELD_MAC(  84,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_DP_TO_CFI_MAP_ENABLE_E),
    PRV_CPSS_FIELD_MAC(  85,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_PUSH_SOURCE_BASED_MPLS_LABEL_E),
    PRV_CPSS_FIELD_MAC(  86,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_PUSH_EVLAN_BASED_MPLS_LABEL_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_HA_EPORT_TABLE_1_FIELDS_PHA_THREAD_NUMBER_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_10_HA_EPORT_TABLE_1_FIELDS_NESTED_VLAN_EN_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_30_HA_EPORT_TABLE_1_FIELDS_NESTED_VLAN_MODE_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_30_HA_EPORT_TABLE_1_FIELDS_HSR_PRP_LAN_ID_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_30_HA_EPORT_TABLE_1_FIELDS_L2NAT_ENABLE_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_30_HA_EPORT_TABLE_1_FIELDS_NAT_PTR_E)
};

static const PRV_CPSS_ENTRY_FORMAT_TABLE_STC prvCpssDxChBobcat2B0HaEPort2TableFieldsFormat[SIP5_HA_EPORT_TABLE_2_FIELDS____LAST_VALUE___E] =
{
    PRV_CPSS_FIELD_MAC(   0,   1, SIP5_HA_EPORT_TABLE_2_MIRROR_TO_ANALYZER_KEEP_TAGS_E),
    PRV_CPSS_FIELD_MAC(   1,   1, SIP5_HA_EPORT_TABLE_2_TO_ANALYZER_VLAN_TAG_ADD_EN_E),
    PRV_CPSS_FIELD_MAC(   2,   2, SIP5_HA_EPORT_TABLE_2_PUSH_VLAN_COMMAND_E),
    PRV_CPSS_FIELD_MAC(   4,   3, SIP5_HA_EPORT_TABLE_2_PUSHED_TAG_TPID_SELECT_E),
    PRV_CPSS_FIELD_MAC(   7,  12, SIP5_HA_EPORT_TABLE_2_PUSHED_TAG_VALUE_E),
    PRV_CPSS_FIELD_MAC(  19,   1, SIP5_HA_EPORT_TABLE_2_UP_CFI_ASSIGNMENT_COMMAND_E),
    PRV_CPSS_FIELD_MAC(  20,   3, SIP5_HA_EPORT_TABLE_2_UP_E),
    PRV_CPSS_FIELD_MAC(  23,   1, SIP5_HA_EPORT_TABLE_2_CFI_E),
    PRV_CPSS_FIELD_MAC(  24,   1, SIP5_HA_EPORT_TABLE_2_FORCE_E_TAG_IE_PID_TO_DEFAULT_E)
};

static const PRV_CPSS_ENTRY_FORMAT_TABLE_STC prvCpssDxChBobKIpvxRouterEportTableFieldsFormat[SIP5_IPVX_ROUTER_EPORT_TABLE_FIELDS___LAST_VALUE___E] =
{
    PRV_CPSS_FIELD_MAC(   0,   1, SIP5_IPVX_ROUTER_EPORT_TABLE_FIELDS_ROUTER_PER_EPORT_SIP_SA_CHECK_ENABLE_E),
    PRV_CPSS_FIELD_MAC(   1,   1, SIP5_IPVX_ROUTER_EPORT_TABLE_FIELDS_FCOE_FORWARDING_ENABLE_E),
    PRV_CPSS_FIELD_MAC(   2,   1, SIP5_IPVX_ROUTER_EPORT_TABLE_FIELDS_IPV4_UC_ROUTING_ENABLE_E),
    PRV_CPSS_FIELD_MAC(   3,   1, SIP5_IPVX_ROUTER_EPORT_TABLE_FIELDS_IPV4_MC_ROUTING_ENABLE_E),
    PRV_CPSS_FIELD_MAC(   4,   1, SIP5_IPVX_ROUTER_EPORT_TABLE_FIELDS_IPV6_UC_ROUTING_ENABLE_E),
    PRV_CPSS_FIELD_MAC(   5,   1, SIP5_IPVX_ROUTER_EPORT_TABLE_FIELDS_IPV6_MC_ROUTING_ENABLE_E),
    PRV_CPSS_FIELD_MAC(   6,   0, SIP5_IPVX_ROUTER_EPORT_TABLE_FIELDS_URPF_ENABLE_E),
    PRV_CPSS_FIELD_MAC(   6,   1, SIP5_IPVX_ROUTER_EPORT_TABLE_FIELDS_DISABLE_SIP_LOOKUP)
};

/* Number of bits in table pattern entry */
#define BOBK_IPVX_ROUTER_EPORT_TABLE_PATTERN_BITS_CNS         7


/* BOBCAT2 B0 TABLES - End */

/* bobk tables - start */
static const PRV_CPSS_ENTRY_FORMAT_TABLE_STC prvCpssDxChBobkHaEPort1TableFieldsFormat[SIP5_HA_EPORT_TABLE_1_FIELDS____LAST_VALUE___E] =
{
    PRV_CPSS_FIELD_MAC(   0,   2, SIP5_HA_EPORT_TABLE_1_FIELDS_VLAN_TRANS_EN_E),
    PRV_CPSS_FIELD_MAC(   2,   3, SIP5_HA_EPORT_TABLE_1_FIELDS_EGRESS_TAG0_TPID_INDEX_E),
    PRV_CPSS_FIELD_MAC(   5,   3, SIP5_HA_EPORT_TABLE_1_FIELDS_EGRESS_TAG1_TPID_INDEX_E),
    PRV_CPSS_FIELD_MAC(   8,   3, SIP5_HA_EPORT_TABLE_1_FIELDS_TS_HEADER_TPID_INDEX_E),
    PRV_CPSS_FIELD_MAC(  11,  32, SIP5_HA_EPORT_TABLE_1_FIELDS_TS_EXT_E),
    PRV_CPSS_FIELD_MAC(  11,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_MPLS_PW_LABEL_PUSH_ENABLE_E),
    PRV_CPSS_FIELD_MAC(  12,  20, SIP5_HA_EPORT_TABLE_1_FIELDS_MPLS_PW_LABEL_E),
    PRV_CPSS_FIELD_MAC(  32,   3, SIP5_HA_EPORT_TABLE_1_FIELDS_MPLS_PW_LABEL_EXP_E),
    PRV_CPSS_FIELD_MAC(  35,   8, SIP5_HA_EPORT_TABLE_1_FIELDS_MPLS_PW_LABEL_TTL_E),
    PRV_CPSS_FIELD_MAC(  43,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_PUSH_MPLS_FLOW_LABEL_ENABLE_E),
    PRV_CPSS_FIELD_MAC(  44,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_FORCE_ARP_TS_EPORT_DECISION_E),
    PRV_CPSS_FIELD_MAC(  45,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_TUNNEL_START_E),
    PRV_CPSS_FIELD_MAC(  46,  14, SIP5_HA_EPORT_TABLE_1_FIELDS_TS_POINTER_E),
    PRV_CPSS_FIELD_MAC(  46,  16, SIP5_HA_EPORT_TABLE_1_FIELDS_ARP_POINTER_E),
    PRV_CPSS_FIELD_MAC(  60,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_TS_PASSENGER_TYPE_E),
    PRV_CPSS_FIELD_MAC(  62,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_MODIFY_MAC_SA_E),
    PRV_CPSS_FIELD_MAC(  63,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_MODIFY_MAC_DA_E),
    PRV_CPSS_FIELD_MAC(  64,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_TRILL_INTERFACE_ENABLE_E),
    PRV_CPSS_FIELD_MAC(  65,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_KEEP_ORIGINAL_TAG0_TPID_E),
    PRV_CPSS_FIELD_MAC(  66,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_KEEP_ORIGINAL_TAG1_TPID_E),
    PRV_CPSS_FIELD_MAC(  67,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_ENABLE_EGRESS_UP_MAPPING_E),
    PRV_CPSS_FIELD_MAC(  68,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_ENABLE_EGRESS_DSCP_MAPPING_E),
    PRV_CPSS_FIELD_MAC(  69,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_ENABLE_EGRESS_EXP_MAPPING_E),
    PRV_CPSS_FIELD_MAC(  70,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_ENABLE_EGRESS_TC_DP_MAPPING_E),
    PRV_CPSS_FIELD_MAC(  71,   4, SIP5_HA_EPORT_TABLE_1_FIELDS_QOS_MAPPING_TABLE_INDEX_E),
    PRV_CPSS_FIELD_MAC(  75,   8, SIP5_HA_EPORT_TABLE_1_FIELDS_ROUTER_MAC_SA_INDEX_E),
    PRV_CPSS_FIELD_MAC(  83,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_DP_TO_CFI_MAP_ENABLE_E),
    PRV_CPSS_FIELD_MAC(  84,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_PUSH_SOURCE_BASED_MPLS_LABEL_E),
    PRV_CPSS_FIELD_MAC(  85,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_PUSH_EVLAN_BASED_MPLS_LABEL_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_HA_EPORT_TABLE_1_FIELDS_PHA_THREAD_NUMBER_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_10_HA_EPORT_TABLE_1_FIELDS_NESTED_VLAN_EN_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_30_HA_EPORT_TABLE_1_FIELDS_NESTED_VLAN_MODE_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_30_HA_EPORT_TABLE_1_FIELDS_HSR_PRP_LAN_ID_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_30_HA_EPORT_TABLE_1_FIELDS_L2NAT_ENABLE_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_30_HA_EPORT_TABLE_1_FIELDS_NAT_PTR_E)
};
/* bobk tables - end */

/* bobcat3 tables - start */
static const PRV_CPSS_ENTRY_FORMAT_TABLE_STC prvCpssDxChSip5_20TtiPhysicalPortTableFieldsFormat[SIP6_TTI_PHYSICAL_PORT_TABLE_FIELDS_FLOW_TRACK_ENABLE_E] =
{
    PRV_CPSS_FIELD_MAC(   0,  14, SIP5_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_DEFAULT_SOURCE_EPORT_NUMBER_E),
    PRV_CPSS_FIELD_MAC(  14,   0, SIP5_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_IS_RING_CORE_PORT_E),
    PRV_CPSS_FIELD_MAC(  14,   1, SIP5_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_IS_LOOPED_E),
    PRV_CPSS_FIELD_MAC(  15,   1, SIP5_TTI_PHYSICAL_PORT_TABLE_FIELDS_ENABLE_MRU_CHECK_ON_CASCADE_PORT_E),
    PRV_CPSS_FIELD_MAC(  16,  14, SIP5_TTI_PHYSICAL_PORT_TABLE_FIELDS_CC_LINE_CARD_PORTS_DEFAULT_EPORT_BASE_E),
    PRV_CPSS_FIELD_MAC(  30,  14, SIP5_TTI_PHYSICAL_PORT_TABLE_FIELDS_CC_LINE_CARD_TRUNK_DEFAULT_EPORT_BASE_E),
    PRV_CPSS_FIELD_MAC(  44,   1, SIP5_TTI_PHYSICAL_PORT_TABLE_FIELDS_CENTRALIZED_CHASSIS_PORT_ENABLE_E),
    PRV_CPSS_FIELD_MAC(  45,  12, SIP5_TTI_PHYSICAL_PORT_TABLE_FIELDS_TRUNK_ID_E),
    PRV_CPSS_FIELD_MAC(  57,   1, SIP5_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_QOS_MODE_E)
};

static const PRV_CPSS_ENTRY_FORMAT_TABLE_STC prvCpssDxChSip5_20L2iEPortTableFieldsFormat[SIP5_L2I_EPORT_TABLE_FIELDS___LAST_VALUE___E] =
{
    PRV_CPSS_FIELD_MAC(   0,   1, SIP5_L2I_EPORT_TABLE_FIELDS_NA_MSG_TO_CPU_EN_E),
    PRV_CPSS_FIELD_MAC(   1,   1, SIP5_L2I_EPORT_TABLE_FIELDS_AUTO_LEARN_DIS_E),
    PRV_CPSS_FIELD_MAC(   2,   1, SIP5_L2I_EPORT_TABLE_FIELDS_NA_STORM_PREV_EN_E),
    PRV_CPSS_FIELD_MAC(   3,   1, SIP5_L2I_EPORT_TABLE_FIELDS_NEW_SRC_ADDR_SECURITY_BREACH_E),
    PRV_CPSS_FIELD_MAC(   4,   1, SIP5_L2I_EPORT_TABLE_FIELDS_VLAN_INGRESS_FILTERING_E),
    PRV_CPSS_FIELD_MAC(   5,   2, SIP5_L2I_EPORT_TABLE_FIELDS_ACCEPT_FRAME_TYPE_E),
    PRV_CPSS_FIELD_MAC(   7,   1, SIP5_L2I_EPORT_TABLE_FIELDS_UC_LOCAL_CMD_E),
    PRV_CPSS_FIELD_MAC(   8,   3, SIP5_L2I_EPORT_TABLE_FIELDS_UNKNOWN_SRC_ADDR_CMD_E),
    PRV_CPSS_FIELD_MAC(  11,   1, SIP5_L2I_EPORT_TABLE_FIELDS_PORT_PVLAN_EN_E),
    PRV_CPSS_FIELD_MAC(  12,   1, SIP5_L2I_EPORT_TABLE_FIELDS_PORT_VLAN_IS_TRUNK_E),
    PRV_CPSS_FIELD_MAC(  13,  14, SIP5_L2I_EPORT_TABLE_FIELDS_PORT_PVLAN_TRG_EPORT_TRUNK_NUM_E),
    PRV_CPSS_FIELD_MAC(  27,  10, SIP5_L2I_EPORT_TABLE_FIELDS_PORT_PVLAN_TRG_DEV_E),
    PRV_CPSS_FIELD_MAC(  37,   1, SIP5_L2I_EPORT_TABLE_FIELDS_ALL_PKT_TO_PVLAN_UPLINK_EN_E),
    PRV_CPSS_FIELD_MAC(  38,   1, SIP5_L2I_EPORT_TABLE_FIELDS_IGMP_TRAP_EN_E),
    PRV_CPSS_FIELD_MAC(  39,   1, SIP5_L2I_EPORT_TABLE_FIELDS_ARP_BC_TRAP_EN_E),
    PRV_CPSS_FIELD_MAC(  40,   1, SIP5_L2I_EPORT_TABLE_FIELDS_EN_LEARN_ON_TRAP_IEEE_RSRV_MC_E),
    PRV_CPSS_FIELD_MAC(  41,   3, SIP5_L2I_EPORT_TABLE_FIELDS_IEEE_RSVD_MC_TABLE_SEL_E),
    PRV_CPSS_FIELD_MAC(  44,   1, SIP5_L2I_EPORT_TABLE_FIELDS_PORT_SOURCE_ID_FORCE_MODE_E),
    PRV_CPSS_FIELD_MAC(  45,  12, SIP5_L2I_EPORT_TABLE_FIELDS_SRC_ID_E),
    PRV_CPSS_FIELD_MAC(  57,   1, SIP5_L2I_EPORT_TABLE_FIELDS_ARP_MAC_SA_MIS_DROP_EN_E),
    PRV_CPSS_FIELD_MAC(  58,   3, SIP5_L2I_EPORT_TABLE_FIELDS_INGRESS_PORT_UNKNOWN_UC_FILTER_CMD_E),
    PRV_CPSS_FIELD_MAC(  61,   3, SIP5_L2I_EPORT_TABLE_FIELDS_INGRESS_PORT_UNREG_MC_FILTER_CMD_E),
    PRV_CPSS_FIELD_MAC(  64,   3, SIP5_L2I_EPORT_TABLE_FIELDS_INGRESS_PORT_BC_FILTER_CMD_E),
    PRV_CPSS_FIELD_MAC(  67,   1, SIP5_L2I_EPORT_TABLE_FIELDS_INGRESS_EPORT_STP_STATE_MODE_E),
    PRV_CPSS_FIELD_MAC(  68,   2, SIP5_L2I_EPORT_TABLE_FIELDS_INGRESS_EPORT_SPANNING_TREE_STATE_E),
    PRV_CPSS_FIELD_MAC(  70,   1, SIP5_L2I_EPORT_TABLE_FIELDS_IPV4_CONTROL_TRAP_EN_E),
    PRV_CPSS_FIELD_MAC(  71,   1, SIP5_L2I_EPORT_TABLE_FIELDS_IPV6_CONTROL_TRAP_EN_E),
    PRV_CPSS_FIELD_MAC(  72,   1, SIP5_L2I_EPORT_TABLE_FIELDS_BC_UDP_TRAP_OR_MIRROR_EN_E),
    PRV_CPSS_FIELD_MAC(  73,   1, SIP5_L2I_EPORT_TABLE_FIELDS_EN_LEARN_ORIG_TAG1_VID_E),
    PRV_CPSS_FIELD_MAC(  74,   1, SIP5_L2I_EPORT_TABLE_FIELDS_FDB_UC_IPV4_ROUTING_EN_E),
    PRV_CPSS_FIELD_MAC(  75,   1, SIP5_L2I_EPORT_TABLE_FIELDS_FDB_UC_IPV6_ROUTING_EN_E),
    PRV_CPSS_FIELD_MAC(  76,   1, SIP5_L2I_EPORT_TABLE_FIELDS_FDB_FCOE_ROUTING_EN_E),
    PRV_CPSS_FIELD_MAC(  77,   3, SIP5_L2I_EPORT_TABLE_FIELDS_MOVED_MAC_SA_CMD_E)
};

static const PRV_CPSS_ENTRY_FORMAT_TABLE_STC prvCpssDxChSip5_20HaEPort1TableFieldsFormat[SIP5_HA_EPORT_TABLE_1_FIELDS____LAST_VALUE___E] =
{
    PRV_CPSS_FIELD_MAC(   0,   2, SIP5_HA_EPORT_TABLE_1_FIELDS_VLAN_TRANS_EN_E),
    PRV_CPSS_FIELD_MAC(   2,   3, SIP5_HA_EPORT_TABLE_1_FIELDS_EGRESS_TAG0_TPID_INDEX_E),
    PRV_CPSS_FIELD_MAC(   5,   3, SIP5_HA_EPORT_TABLE_1_FIELDS_EGRESS_TAG1_TPID_INDEX_E),
    PRV_CPSS_FIELD_MAC(   8,   3, SIP5_HA_EPORT_TABLE_1_FIELDS_TS_HEADER_TPID_INDEX_E),
    PRV_CPSS_FIELD_MAC(  11,  32, SIP5_HA_EPORT_TABLE_1_FIELDS_TS_EXT_E),
    PRV_CPSS_FIELD_MAC(  11,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_MPLS_PW_LABEL_PUSH_ENABLE_E),
    PRV_CPSS_FIELD_MAC(  12,  20, SIP5_HA_EPORT_TABLE_1_FIELDS_MPLS_PW_LABEL_E),
    PRV_CPSS_FIELD_MAC(  32,   3, SIP5_HA_EPORT_TABLE_1_FIELDS_MPLS_PW_LABEL_EXP_E),
    PRV_CPSS_FIELD_MAC(  35,   8, SIP5_HA_EPORT_TABLE_1_FIELDS_MPLS_PW_LABEL_TTL_E),
    PRV_CPSS_FIELD_MAC(  43,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_PUSH_MPLS_FLOW_LABEL_ENABLE_E),
    PRV_CPSS_FIELD_MAC(  44,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_FORCE_ARP_TS_EPORT_DECISION_E),
    PRV_CPSS_FIELD_MAC(  45,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_TUNNEL_START_E),
    PRV_CPSS_FIELD_MAC(  46,  16, SIP5_HA_EPORT_TABLE_1_FIELDS_TS_POINTER_E),
    PRV_CPSS_FIELD_MAC(  46,  18, SIP5_HA_EPORT_TABLE_1_FIELDS_ARP_POINTER_E),
    PRV_CPSS_FIELD_MAC(  62,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_TS_PASSENGER_TYPE_E),
    PRV_CPSS_FIELD_MAC(  64,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_MODIFY_MAC_SA_E),
    PRV_CPSS_FIELD_MAC(  65,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_MODIFY_MAC_DA_E),
    PRV_CPSS_FIELD_MAC(  66,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_TRILL_INTERFACE_ENABLE_E),
    PRV_CPSS_FIELD_MAC(  67,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_KEEP_ORIGINAL_TAG0_TPID_E),
    PRV_CPSS_FIELD_MAC(  68,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_KEEP_ORIGINAL_TAG1_TPID_E),
    PRV_CPSS_FIELD_MAC(  69,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_ENABLE_EGRESS_UP_MAPPING_E),
    PRV_CPSS_FIELD_MAC(  70,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_ENABLE_EGRESS_DSCP_MAPPING_E),
    PRV_CPSS_FIELD_MAC(  71,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_ENABLE_EGRESS_EXP_MAPPING_E),
    PRV_CPSS_FIELD_MAC(  72,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_ENABLE_EGRESS_TC_DP_MAPPING_E),
    PRV_CPSS_FIELD_MAC(  73,   4, SIP5_HA_EPORT_TABLE_1_FIELDS_QOS_MAPPING_TABLE_INDEX_E),
    PRV_CPSS_FIELD_MAC(  77,   8, SIP5_HA_EPORT_TABLE_1_FIELDS_ROUTER_MAC_SA_INDEX_E),
    PRV_CPSS_FIELD_MAC(  85,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_DP_TO_CFI_MAP_ENABLE_E),
    PRV_CPSS_FIELD_MAC(  86,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_PUSH_SOURCE_BASED_MPLS_LABEL_E),
    PRV_CPSS_FIELD_MAC(  87,   1, SIP5_HA_EPORT_TABLE_1_FIELDS_PUSH_EVLAN_BASED_MPLS_LABEL_E),
    PRV_CPSS_FIELD_MAC(  88,   8, SIP6_HA_EPORT_TABLE_1_FIELDS_PHA_THREAD_NUMBER_E),
    PRV_CPSS_FIELD_MAC(  96,   1, SIP6_10_HA_EPORT_TABLE_1_FIELDS_NESTED_VLAN_EN_E),
    PRV_CPSS_FIELD_MAC(  97,   2, SIP6_30_HA_EPORT_TABLE_1_FIELDS_NESTED_VLAN_MODE_E),
    PRV_CPSS_FIELD_MAC(  99,   4, SIP6_30_HA_EPORT_TABLE_1_FIELDS_HSR_PRP_LAN_ID_E),
    PRV_CPSS_FIELD_MAC( 103,   1, SIP6_30_HA_EPORT_TABLE_1_FIELDS_L2NAT_ENABLE_E),
    PRV_CPSS_FIELD_MAC( 104,  16, SIP6_30_HA_EPORT_TABLE_1_FIELDS_NAT_PTR_E)
};

static const PRV_CPSS_ENTRY_FORMAT_TABLE_STC prvCpssDxChSip5_20EpclActionTableFieldsFormat[SIP5_EPCL_ACTION_TABLE_FIELDS___LAST_VALUE___E] =
{
    PRV_CPSS_FIELD_MAC(   0,   3, SIP5_EPCL_ACTION_TABLE_FIELDS_COMMAND_E),
    PRV_CPSS_FIELD_MAC(   3,   6, SIP5_EPCL_ACTION_TABLE_FIELDS_DSCP_EXP_E),
    PRV_CPSS_FIELD_MAC(   9,   3, SIP5_EPCL_ACTION_TABLE_FIELDS_UP0_E),
    PRV_CPSS_FIELD_MAC(  12,   2, SIP5_EPCL_ACTION_TABLE_FIELDS_MODIFY_DSCP_EXP_E),
    PRV_CPSS_FIELD_MAC(  14,   2, SIP5_EPCL_ACTION_TABLE_FIELDS_ENABLE_MODIFY_UP0_E),
    PRV_CPSS_FIELD_MAC(  16,   1, SIP5_EPCL_ACTION_TABLE_FIELDS_BIND_TO_CNC_COUNTER_E),
    PRV_CPSS_FIELD_MAC(  17,  14, SIP5_EPCL_ACTION_TABLE_FIELDS_CNC_COUNTER_INDEX_E),
    PRV_CPSS_FIELD_MAC(  31,   1, SIP5_EPCL_ACTION_TABLE_FIELDS_TAG1_VID_CMD_E),
    PRV_CPSS_FIELD_MAC(  32,   1, SIP5_EPCL_ACTION_TABLE_FIELDS_ENABLE_MODIFY_UP1_E),
    PRV_CPSS_FIELD_MAC(  33,  12, SIP5_EPCL_ACTION_TABLE_FIELDS_TAG1_VID_E),
    PRV_CPSS_FIELD_MAC(  45,   3, SIP5_EPCL_ACTION_TABLE_FIELDS_UP1_E),
    PRV_CPSS_FIELD_MAC(  48,   2, SIP5_EPCL_ACTION_TABLE_FIELDS_TAG0_VLAN_CMD_E),
    PRV_CPSS_FIELD_MAC(  50,  12, SIP5_EPCL_ACTION_TABLE_FIELDS_TAG0_VID_E),
    PRV_CPSS_FIELD_MAC(  62,   1, SIP5_EPCL_ACTION_TABLE_FIELDS_BIND_TO_POLICER_COUNTER_E),
    PRV_CPSS_FIELD_MAC(  63,   1, SIP5_EPCL_ACTION_TABLE_FIELDS_BIND_TO_POLICER_METER_E),
    PRV_CPSS_FIELD_MAC(  64,  14, SIP5_EPCL_ACTION_TABLE_FIELDS_POLICER_INDEX_E),
    PRV_CPSS_FIELD_MAC(  78,   0, SIP5_EPCL_ACTION_TABLE_FIELDS_RESERVED_E),
    PRV_CPSS_FIELD_MAC(  78,  13, SIP5_EPCL_ACTION_TABLE_FIELDS_FLOW_ID_E),
    PRV_CPSS_FIELD_MAC(  91,   1, SIP5_EPCL_ACTION_TABLE_FIELDS_OAM_PROCESSING_ENABLE_E),
    PRV_CPSS_FIELD_MAC(  92,   1, SIP5_EPCL_ACTION_TABLE_FIELDS_OAM_PROFILE_E),
    PRV_CPSS_FIELD_MAC(  93,   1, SIP5_EPCL_ACTION_TABLE_FIELDS_TIME_STAMP_ENABLE_E),
    PRV_CPSS_FIELD_MAC(  94,   7, SIP5_EPCL_ACTION_TABLE_FIELDS_TIME_STAMP_OFFSET_INDEX_E),
    PRV_CPSS_FIELD_MAC( 101,   1, SIP5_EPCL_ACTION_TABLE_FIELDS_CHANNEL_TYPE_TO_OPCODE_MAPPING_EN_E),
    PRV_CPSS_FIELD_MAC( 102,   1, SIP5_EPCL_ACTION_TABLE_FIELDS_EPCL_RESERVED_ENABLE_E),
    PRV_CPSS_FIELD_MAC( 103,  20, SIP5_EPCL_ACTION_TABLE_FIELDS_EPCL_RESERVED_E),
    PRV_CPSS_FIELD_MAC( 123,   0, SIP5_EPCL_ACTION_TABLE_FIELDS_TM_QUEUE_ID_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_EPCL_ACTION_TABLE_FIELDS_PHA_METADATA_ASSIGN_ENABLE_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_EPCL_ACTION_TABLE_FIELDS_PHA_METADATA_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_EPCL_ACTION_TABLE_FIELDS_PHA_THREAD_NUMBER_ASSIGN_ENABLE_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_EPCL_ACTION_TABLE_FIELDS_PHA_THREAD_NUMBER_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_EPCL_ACTION_TABLE_FIELDS_EGRESS_MIRRORING_MODE_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_EPCL_ACTION_TABLE_FIELDS_EGRESS_ANALYZER_INDEX_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_EPCL_ACTION_TABLE_FIELDS_CPU_CODE_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_EPCL_ACTION_TABLE_FIELDS_LATENCY_PROFILE_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_EPCL_ACTION_TABLE_FIELDS_LATENCY_MONITORING_ENABLE_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_EPCL_ACTION_TABLE_FIELDS_CUT_THROUGH_TERMINATE_ID_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_EPCL_ACTION_TABLE_FIELDS_RESERVED_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_EPCL_ACTION_TABLE_FIELDS_PCL_OVER_EXACT_MATCH_ENABLE_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_10_EPCL_ACTION_TABLE_FIELDS_IPFIX_ENABLE_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_10_EPCL_ACTION_TABLE_FIELDS_PHA_METADATA_PART2_E)
};

static const PRV_CPSS_ENTRY_FORMAT_TABLE_STC prvCpssDxChSip5_20EgfQagPortTargetAttributesTableFieldsFormat[SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS___LAST_VALUE___E] =
{
    PRV_CPSS_FIELD_MAC(   0,   2, SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS_ETARGET_PHYSICAL_PORT_LOOPBACK_PROFILE_E),
    PRV_CPSS_FIELD_MAC(   2,   2, SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS_ETARGET_PHYSICAL_PORT_TC_PROFILE_E),
    PRV_CPSS_FIELD_MAC(   4,   1, SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS_ETARGET_PHYSICAL_PORT_EVLAN_MIRRORING_ENABLE_E),
    PRV_CPSS_FIELD_MAC(   5,   1, SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS_EUSE_VLAN_TAG_1_FOR_TAG_STATE_E),
    PRV_CPSS_FIELD_MAC(   6,   1, SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS_ETARGET_PHYSICAL_PORT_ENQ_PROFILE_E),
    PRV_CPSS_FIELD_MAC(   7,   1, SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS_ELOOPBACK_ENQ_PROFILE_E),
    PRV_CPSS_FIELD_MAC(   8,   9, SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS_ELOOPBACK_PORT_E)
};

static const PRV_CPSS_ENTRY_FORMAT_TABLE_STC prvCpssDxChSip5_20EgfQagPortSourceAttributesTableFieldsFormat[SIP5_20_EGF_QAG_PORT_SOURCE_ATTRIBUTES_TABLE_FIELDS___LAST_VALUE___E] =
{
    PRV_CPSS_FIELD_MAC(   0,   2, SIP5_20_EGF_QAG_PORT_SOURCE_ATTRIBUTES_TABLE_FIELDS_SOURCE_PHYSICAL_PORT_LOOPBACK_PROFILE_E),
    PRV_CPSS_FIELD_MAC(   2,   2, SIP5_20_EGF_QAG_PORT_SOURCE_ATTRIBUTES_TABLE_FIELDS_SOURCE_PHYSICAL_PORT_TC_PROFILE_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP6_EGF_QAG_PORT_SOURCE_ATTRIBUTES_TABLE_FIELDS_SOURCE_PHYSICAL_PORT_SPEED_E)
};

static const PRV_CPSS_ENTRY_FORMAT_TABLE_STC prvCpssDxChSip5_20IpvxRouterNextHopTableFieldsFormat[SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS___LAST_VALUE___E] =
{
    PRV_CPSS_FIELD_MAC(   0,   3, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_COMMAND_E),
    PRV_CPSS_FIELD_MAC(   3,   1, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_BYPASS_TTL_OPTIONS_OR_HOP_EXTENSION_E),
    PRV_CPSS_FIELD_MAC(   4,   2, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_CPU_CODE_INDEX_E),
    PRV_CPSS_FIELD_MAC(   6,   3, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_INGRESS_MIRROR_TO_ANALYZER_INDEX_E),
    PRV_CPSS_FIELD_MAC(   9,   1, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_QOS_PROFILE_PRECEDENCE_E),
    PRV_CPSS_FIELD_MAC(  10,   1, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_QOS_PROFILE_MARKING_EN_E),
    PRV_CPSS_FIELD_MAC(  11,   7, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_QOS_PROFILE_INDEX_E),
    PRV_CPSS_FIELD_MAC(  18,   2, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_MODIFY_DSCP_E),
    PRV_CPSS_FIELD_MAC(  20,   2, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_MODIFY_UP_E),
    PRV_CPSS_FIELD_MAC(  22,  13, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_MC_RPF_EVLAN_E),
    PRV_CPSS_FIELD_MAC(  22,   6, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_MC_RPF_EVLAN_MRST_INDEX_E),
    PRV_CPSS_FIELD_MAC(  28,   7, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_RESERVED_28_34_E),
    PRV_CPSS_FIELD_MAC(  35,  13, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_RESERVED_35_47_E),
    PRV_CPSS_FIELD_MAC(  48,   3, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_MULTICAST_INGRESS_VLAN_CHECK_FAIL_CMD_E),
    PRV_CPSS_FIELD_MAC(  51,   1, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_MULTICAST_RPF_FAIL_CMD_MODE_E),
    PRV_CPSS_FIELD_MAC(  52,   1, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_RESERVED_52_E),
    PRV_CPSS_FIELD_MAC(  53,  16, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_INTERNAL_MLL_PTR_E),
    PRV_CPSS_FIELD_MAC(  69,   3, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_EXTERNAL_MLL_PTR_MSB_BITS_15_13_E),
    PRV_CPSS_FIELD_MAC(  72,   3, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_RESERVED_72_74_E),
    PRV_CPSS_FIELD_MAC(  75,   2, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_MULTICAST_INGRESS_VLAN_CHECK_E),
    PRV_CPSS_FIELD_MAC(  77,  13, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_EXTERNAL_MLL_PTR_E),
    PRV_CPSS_FIELD_MAC(  90,   3, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_IPV6_MULTICAST_RESERVED_90_E),
    PRV_CPSS_FIELD_MAC(  93,   1, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_IPV6_MULTICAST_DEST_SITE_ID_E),
    PRV_CPSS_FIELD_MAC(  94,   1, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_IPV6_MULTICAST_SCOPE_CHECK_EN_E),
    PRV_CPSS_FIELD_MAC(   9,  12, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_NEXT_HOP_VID1_E),
    PRV_CPSS_FIELD_MAC(  21,   1, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_RESERVED_21_E),
    PRV_CPSS_FIELD_MAC(  22,  13, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_NEXT_HOP_EVLAN_E),
    PRV_CPSS_FIELD_MAC(  35,   1, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_USE_VIDX_E),
    PRV_CPSS_FIELD_MAC(  36,  16, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_EVIDX_E),
    PRV_CPSS_FIELD_MAC(  52,   8, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_RESERVED_52_59_E),
    PRV_CPSS_FIELD_MAC(  36,   1, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_TARGET_IS_TRUNK_E),
    PRV_CPSS_FIELD_MAC(  37,  14, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_TRG_EPORT_E),
    PRV_CPSS_FIELD_MAC(  51,  10, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_TRG_DEV_E),
    PRV_CPSS_FIELD_MAC(  37,  12, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_TRG_TRUNK_ID_E),
    PRV_CPSS_FIELD_MAC(  49,  12, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_RESERVED_49_59_E),
    PRV_CPSS_FIELD_MAC(  61,   6, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_DIP_ACCESS_LEVEL_E),
    PRV_CPSS_FIELD_MAC(  67,   6, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_SIP_ACCESS_LEVEL_E),
    PRV_CPSS_FIELD_MAC(  73,   1, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_START_OF_TUNNEL_E),
    PRV_CPSS_FIELD_MAC(  74,  18, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_ARP_PTR_E),
    PRV_CPSS_FIELD_MAC(  74,  16, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_TUNNEL_PTR_E),
    PRV_CPSS_FIELD_MAC(  90,   1, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_RESERVED_88_E),
    PRV_CPSS_FIELD_MAC(  91,   1, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_UNICAST_TS_IS_NAT_E),
    PRV_CPSS_FIELD_MAC(  92,   1, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_UNICAST_TUNNEL_TYPE_E),
    PRV_CPSS_FIELD_MAC(  93,   1, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_UNICAST_DEST_SITE_ID_E),
    PRV_CPSS_FIELD_MAC(  94,   1, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_UNICAST_SCOPE_CHECK_EN_E),
    PRV_CPSS_FIELD_MAC(  95,   3, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_COUNTER_SET_INDEX_E),
    PRV_CPSS_FIELD_MAC(  98,   3, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_MTU_INDEX_E),
    PRV_CPSS_FIELD_MAC( 101,   1, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_ARP_BC_TRAP_MIRROR_EN_E),
    PRV_CPSS_FIELD_MAC( 102,   1, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_APP_SPECIFIC_CPU_CODE_EN_E),
    PRV_CPSS_FIELD_MAC( 103,   1, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_RESERVED_101_E),
    PRV_CPSS_FIELD_MAC( 103,   1, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_SIP_FILTER_EN_E),
    PRV_CPSS_FIELD_MAC( 104,   1, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_DEC_TTL_OR_HOP_COUNT_E),
    PRV_CPSS_FIELD_MAC( 105,   1, SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_ICMP_REDIRECT_EXCEP_MIRROR_E),
    PRV_CPSS_FIELD_MAC(   9,   9, SIP6_10_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_TARGET_EPG_E)
};

static const PRV_CPSS_ENTRY_FORMAT_TABLE_STC prvCpssDxChSip5_20L2MllTableFieldsFormat[SIP5_L2_MLL_TABLE_FIELDS___LAST_VALUE___E] =
{
    PRV_CPSS_FIELD_MAC(   0,   1, SIP5_L2_MLL_TABLE_FIELDS_LAST_0_E),
    PRV_CPSS_FIELD_MAC(   1,   2, SIP5_L2_MLL_TABLE_FIELDS_BIND_TO_MLL_COUNTER_0_E),
    PRV_CPSS_FIELD_MAC(   3,  15, SIP5_L2_MLL_TABLE_FIELDS_MASK_BITMAP_0_E),
    PRV_CPSS_FIELD_MAC(  18,   1, SIP5_L2_MLL_TABLE_FIELDS_USE_VIDX_0_E),
    PRV_CPSS_FIELD_MAC(  19,   1, SIP5_L2_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_0_E),
    PRV_CPSS_FIELD_MAC(  20,  14, SIP5_L2_MLL_TABLE_FIELDS_TRG_EPORT_0_E),
    PRV_CPSS_FIELD_MAC(  20,  12, SIP5_L2_MLL_TABLE_FIELDS_TRG_TRUNK_ID_0_E),
    PRV_CPSS_FIELD_MAC(  20,  12, SIP5_L2_MLL_TABLE_FIELDS_VIDX_0_E),
    PRV_CPSS_FIELD_MAC(  34,   1, SIP5_L2_MLL_TABLE_FIELDS_UNKNOWN_UC_FILTER_ENABLE_0_E),
    PRV_CPSS_FIELD_MAC(  35,   1, SIP5_L2_MLL_TABLE_FIELDS_UNREGISTERED_MC_FILTER_ENABLE_0_E),
    PRV_CPSS_FIELD_MAC(  36,   1, SIP5_L2_MLL_TABLE_FIELDS_BC_FILTER_ENABLE_0_E),
    PRV_CPSS_FIELD_MAC(  37,  10, SIP5_L2_MLL_TABLE_FIELDS_TRG_DEV_0_E),
    PRV_CPSS_FIELD_MAC(  47,   1, SIP5_L2_MLL_TABLE_FIELDS_ONE_PLUS_ONE_FILTERING_ENABLE_0_E),
    PRV_CPSS_FIELD_MAC(  48,   8, SIP5_L2_MLL_TABLE_FIELDS_TTL_THRESHOLD_0_E),
    PRV_CPSS_FIELD_MAC(  56,   8, SIP5_L2_MLL_TABLE_FIELDS_MESH_ID_0_E),
    PRV_CPSS_FIELD_MAC(  64,   1, SIP5_L2_MLL_TABLE_FIELDS_MC_LOCAL_SWITCHING_ENABLE_0_E),
    PRV_CPSS_FIELD_MAC(  65,   1, SIP5_L2_MLL_TABLE_FIELDS_MAX_HOP_COUNT_ENABLE_0_E),
    PRV_CPSS_FIELD_MAC(  66,   6, SIP5_L2_MLL_TABLE_FIELDS_MAX_OUTGOING_HOP_COUNT_0_E),
    PRV_CPSS_FIELD_MAC(  72,   1, SIP5_L2_MLL_TABLE_FIELDS_RESERVED_1_E),
    PRV_CPSS_FIELD_MAC(  73,   1, SIP5_L2_MLL_TABLE_FIELDS_LAST_1_E),
    PRV_CPSS_FIELD_MAC(  74,   2, SIP5_L2_MLL_TABLE_FIELDS_BIND_TO_MLL_COUNTER_1_E),
    PRV_CPSS_FIELD_MAC(  76,  15, SIP5_L2_MLL_TABLE_FIELDS_MASK_BITMAP_1_E),
    PRV_CPSS_FIELD_MAC(  91,   1, SIP5_L2_MLL_TABLE_FIELDS_USE_VIDX_1_E),
    PRV_CPSS_FIELD_MAC(  92,   1, SIP5_L2_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_1_E),
    PRV_CPSS_FIELD_MAC(  93,  14, SIP5_L2_MLL_TABLE_FIELDS_TRG_EPORT_1_E),
    PRV_CPSS_FIELD_MAC(  93,  12, SIP5_L2_MLL_TABLE_FIELDS_TRG_TRUNK_ID_1_E),
    PRV_CPSS_FIELD_MAC(  93,  12, SIP5_L2_MLL_TABLE_FIELDS_VIDX_1_E),
    PRV_CPSS_FIELD_MAC( 107,   1, SIP5_L2_MLL_TABLE_FIELDS_UNKNOWN_UC_FILTER_ENABLE_1_E),
    PRV_CPSS_FIELD_MAC( 108,   1, SIP5_L2_MLL_TABLE_FIELDS_UNREGISTERED_MC_FILTER_ENABLE_1_E),
    PRV_CPSS_FIELD_MAC( 109,   1, SIP5_L2_MLL_TABLE_FIELDS_BC_FILTER_ENABLE_1_E),
    PRV_CPSS_FIELD_MAC( 110,  10, SIP5_L2_MLL_TABLE_FIELDS_TRG_DEV_1_E),
    PRV_CPSS_FIELD_MAC( 120,   1, SIP5_L2_MLL_TABLE_FIELDS_ONE_PLUS_ONE_FILTERING_ENABLE_1_E),
    PRV_CPSS_FIELD_MAC( 121,   8, SIP5_L2_MLL_TABLE_FIELDS_TTL_THRESHOLD_1_E),
    PRV_CPSS_FIELD_MAC( 129,   8, SIP5_L2_MLL_TABLE_FIELDS_MESH_ID_1_E),
    PRV_CPSS_FIELD_MAC( 137,   1, SIP5_L2_MLL_TABLE_FIELDS_MC_LOCAL_SWITCHING_ENABLE_1_E),
    PRV_CPSS_FIELD_MAC( 138,   1, SIP5_L2_MLL_TABLE_FIELDS_MAX_HOP_COUNT_ENABLE_1_E),
    PRV_CPSS_FIELD_MAC( 139,   6, SIP5_L2_MLL_TABLE_FIELDS_MAX_OUTGOING_HOP_COUNT_1_E),
    PRV_CPSS_FIELD_MAC( 145,   1, SIP5_L2_MLL_TABLE_FIELDS_RESERVED_2_E),
    PRV_CPSS_FIELD_MAC( 146,   1, SIP5_L2_MLL_TABLE_FIELDS_ENTRY_SELECTOR_E),
    PRV_CPSS_FIELD_MAC( 147,  15, SIP5_L2_MLL_TABLE_FIELDS_L2_NEXT_MLL_PTR_E)
};

static const PRV_CPSS_ENTRY_FORMAT_TABLE_STC prvCpssDxChSip5_20IpMllTableFieldsFormat[SIP5_IP_MLL_TABLE_FIELDS___LAST_VALUE___E] =
{
    PRV_CPSS_FIELD_MAC(   0,   1, SIP5_IP_MLL_TABLE_FIELDS_LAST_0_E),
    PRV_CPSS_FIELD_MAC(   1,   1, SIP5_IP_MLL_TABLE_FIELDS_START_OF_TUNNEL_0_E),
    PRV_CPSS_FIELD_MAC(   2,   3, SIP5_IP_MLL_TABLE_FIELDS_MLL_RPF_FAIL_CMD_0_E),
    PRV_CPSS_FIELD_MAC(   5,  13, SIP5_IP_MLL_TABLE_FIELDS_MLL_EVID_0_E),
    PRV_CPSS_FIELD_MAC(  18,   1, SIP5_IP_MLL_TABLE_FIELDS_USE_VIDX_0_E),
    PRV_CPSS_FIELD_MAC(  19,   1, SIP5_IP_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_0_E),
    PRV_CPSS_FIELD_MAC(  20,  16, SIP5_IP_MLL_TABLE_FIELDS_EVIDX_0_E),
    PRV_CPSS_FIELD_MAC(  20,  14, SIP5_IP_MLL_TABLE_FIELDS_TRG_EPORT_0_E),
    PRV_CPSS_FIELD_MAC(  20,  12, SIP5_IP_MLL_TABLE_FIELDS_TRG_TRUNK_ID_0_E),
    PRV_CPSS_FIELD_MAC(  36,   1, SIP5_IP_MLL_TABLE_FIELDS_RESEREVED_2_E),
    PRV_CPSS_FIELD_MAC(  37,  10, SIP5_IP_MLL_TABLE_FIELDS_TRG_DEV_0_E),
    PRV_CPSS_FIELD_MAC(  47,   1, SIP5_IP_MLL_TABLE_FIELDS_EXCLUDE_SRC_VLAN_0_E),
    PRV_CPSS_FIELD_MAC(  48,   8, SIP5_IP_MLL_TABLE_FIELDS_TTL_THRESHOLD_0_OR_HOP_LIMIT_THRESHOLD_0_E),
    PRV_CPSS_FIELD_MAC(  56,  16, SIP5_IP_MLL_TABLE_FIELDS_TUNNEL_PTR_0_E),
    PRV_CPSS_FIELD_MAC(  72,   1, SIP5_IP_MLL_TABLE_FIELDS_TUNNEL_TYPE_0_E),
    PRV_CPSS_FIELD_MAC(  73,   1, SIP5_IP_MLL_TABLE_FIELDS_LAST_1_E),
    PRV_CPSS_FIELD_MAC(  74,   1, SIP5_IP_MLL_TABLE_FIELDS_START_OF_TUNNEL_1_E),
    PRV_CPSS_FIELD_MAC(  75,   3, SIP5_IP_MLL_TABLE_FIELDS_MLL_RPF_FAIL_CMD_1_E),
    PRV_CPSS_FIELD_MAC(  78,  13, SIP5_IP_MLL_TABLE_FIELDS_MLL_EVID_1_E),
    PRV_CPSS_FIELD_MAC(  91,   1, SIP5_IP_MLL_TABLE_FIELDS_USE_VIDX_1_E),
    PRV_CPSS_FIELD_MAC(  92,   1, SIP5_IP_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_1_E),
    PRV_CPSS_FIELD_MAC(  93,  16, SIP5_IP_MLL_TABLE_FIELDS_EVIDX_1_E),
    PRV_CPSS_FIELD_MAC(  93,  14, SIP5_IP_MLL_TABLE_FIELDS_TRG_EPORT_1_E),
    PRV_CPSS_FIELD_MAC(  93,  12, SIP5_IP_MLL_TABLE_FIELDS_TRG_TRUNK_ID_1_E),
    PRV_CPSS_FIELD_MAC( 109,   1, SIP5_IP_MLL_TABLE_FIELDS_RESEREVED_3_E),
    PRV_CPSS_FIELD_MAC( 110,  10, SIP5_IP_MLL_TABLE_FIELDS_TRG_DEV_1_E),
    PRV_CPSS_FIELD_MAC( 120,   1, SIP5_IP_MLL_TABLE_FIELDS_EXCLUDE_SRC_VLAN_1_E),
    PRV_CPSS_FIELD_MAC( 121,   8, SIP5_IP_MLL_TABLE_FIELDS_TTL_THRESHOLD_1_OR_HOP_LIMIT_THRESHOLD_1_E),
    PRV_CPSS_FIELD_MAC( 129,  16, SIP5_IP_MLL_TABLE_FIELDS_TUNNEL_PTR_1_E),
    PRV_CPSS_FIELD_MAC( 145,   1, SIP5_IP_MLL_TABLE_FIELDS_TUNNEL_TYPE_1_E),
    PRV_CPSS_FIELD_MAC( 146,  16, SIP5_IP_MLL_TABLE_FIELDS_NEXT_MLL_PTR_E)
};

static const PRV_CPSS_ENTRY_FORMAT_TABLE_STC prvCpssDxChSip5_20PlrMeteringConfigurationTableFieldsFormat[SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS___LAST_VALUE___E] =
{
    PRV_CPSS_FIELD_MAC(   0,   1, SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_COLOR_MODE_E),
    PRV_CPSS_FIELD_MAC(   1,   3, SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_POLICER_MODE_E),
    PRV_CPSS_FIELD_MAC(   4,   2, SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_MG_COUNTERS_SET_EN_E),
    PRV_CPSS_FIELD_MAC(   6,  17, SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_BILLING_PTR_E),
    PRV_CPSS_FIELD_MAC(  23,   1, SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_BYTE_OR_PACKET_COUNTING_MODE_E),
    PRV_CPSS_FIELD_MAC(  24,   1, SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_PACKET_SIZE_MODE_E),
    PRV_CPSS_FIELD_MAC(  25,   1, SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_TUNNEL_TERMINATION_PACKET_SIZE_MODE_E),
    PRV_CPSS_FIELD_MAC(  26,   1, SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_INCLUDE_LAYER1_OVERHEAD_E),
    PRV_CPSS_FIELD_MAC(  27,   1, SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_DSA_TAG_COUNTING_MODE_E),
    PRV_CPSS_FIELD_MAC(  28,   1, SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_TIMESTAMP_TAG_COUNTING_MODE_E),
    PRV_CPSS_FIELD_MAC(  29,   2, SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_INGRESS_RED_CMD_E),
    PRV_CPSS_FIELD_MAC(  31,   2, SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_INGRESS_YELLOW_CMD_E),
    PRV_CPSS_FIELD_MAC(  33,   2, SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_INGRESS_GREEN_CMD_E),
    PRV_CPSS_FIELD_MAC(  35,   2, SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_INGRESS_MODIFY_DSCP_E),
    PRV_CPSS_FIELD_MAC(  37,   2, SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_INGRESS_MODIFY_UP_E),
    PRV_CPSS_FIELD_MAC(  39,  10, SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_INGRESS_QOS_PROFILE_E),
    PRV_CPSS_FIELD_MAC(  29,   1, SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_EGRESS_REMARK_MODE_E),
    PRV_CPSS_FIELD_MAC(  30,   1, SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_EGRESS_DROP_RED_E),
    PRV_CPSS_FIELD_MAC(  31,   1, SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_EGRESS_EN_MODIFY_EXP_E),
    PRV_CPSS_FIELD_MAC(  32,   2, SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_EGRESS_EN_MODIFY_DSCP_E),
    PRV_CPSS_FIELD_MAC(  34,   1, SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_EGRESS_EN_MODIFY_TC_E),
    PRV_CPSS_FIELD_MAC(  35,   2, SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_EGRESS_EN_MODIFY_UP_E),
    PRV_CPSS_FIELD_MAC(  37,   1, SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_EGRESS_EN_MODIFY_DP_E),
    PRV_CPSS_FIELD_MAC(  38,   1, SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_EGRESS_EN_YELLOW_ECN_MARKING_E),
    PRV_CPSS_FIELD_MAC(  49,   3, SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_INGRESS_MEF_10_3_ENV_SIZE_E),
    PRV_CPSS_FIELD_MAC(  49,   3, SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_EGRESS_MEF_10_3_ENV_SIZE_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP_6_PLR_METERING_CONFIG_TABLE_FIELDS_INGRESS_GREEN_MIRROR_EN_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP_6_PLR_METERING_CONFIG_TABLE_FIELDS_INGRESS_YELLOW_MIRROR_EN_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP_6_PLR_METERING_CONFIG_TABLE_FIELDS_INGRESS_RED_MIRROR_EN_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP_6_PLR_METERING_CONFIG_TABLE_FIELDS_EGRESS_GREEN_MIRROR_EN_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP_6_PLR_METERING_CONFIG_TABLE_FIELDS_EGRESS_YELLOW_MIRROR_EN_E),
    PRV_CPSS_FIELD_MAC(   0,   0, SIP_6_PLR_METERING_CONFIG_TABLE_FIELDS_EGRESS_RED_MIRROR_EN_E)
};

/* static */ const PRV_CPSS_ENTRY_FORMAT_TABLE_STC prvCpssDxChSip5_20FdbFdbTableFieldsFormat[SIP5_10_FDB_FDB_TABLE_FIELDS___LAST_VALUE___E] =
{
    PRV_CPSS_FIELD_MAC(   0,   1, SIP5_FDB_FDB_TABLE_FIELDS_VALID_E),
    PRV_CPSS_FIELD_MAC(   1,   1, SIP5_FDB_FDB_TABLE_FIELDS_SKIP_E),
    PRV_CPSS_FIELD_MAC(   2,   1, SIP5_FDB_FDB_TABLE_FIELDS_AGE_E),
    PRV_CPSS_FIELD_MAC(   3,   3, SIP5_FDB_FDB_TABLE_FIELDS_FDB_ENTRY_TYPE_E),
    PRV_CPSS_FIELD_MAC(   6,  13, SIP5_FDB_FDB_TABLE_FIELDS_FID_E),
    PRV_CPSS_FIELD_MAC(  19,  48, SIP5_FDB_FDB_TABLE_FIELDS_MAC_ADDR_E),
    PRV_CPSS_FIELD_MAC(  67,  10, SIP5_FDB_FDB_TABLE_FIELDS_DEV_ID_E),
    PRV_CPSS_FIELD_MAC(  77,   6, SIP5_FDB_FDB_TABLE_FIELDS_SOURCE_ID_5_0_E),
    PRV_CPSS_FIELD_MAC(  83,   1, SIP5_FDB_FDB_TABLE_FIELDS_IS_TRUNK_E),
    PRV_CPSS_FIELD_MAC(  19,  32, SIP5_FDB_FDB_TABLE_FIELDS_DIP_E),
    PRV_CPSS_FIELD_MAC(  51,  32, SIP5_FDB_FDB_TABLE_FIELDS_SIP_E),
    PRV_CPSS_FIELD_MAC(  83,  16, SIP5_FDB_FDB_TABLE_FIELDS_VIDX_E),
    PRV_CPSS_FIELD_MAC(  84,  12, SIP5_FDB_FDB_TABLE_FIELDS_TRUNK_NUM_E),
    PRV_CPSS_FIELD_MAC(  84,  14, SIP5_FDB_FDB_TABLE_FIELDS_EPORT_NUM_E),
    PRV_CPSS_FIELD_MAC(  99,   8, SIP5_FDB_FDB_TABLE_FIELDS_USER_DEFINED_E),
    PRV_CPSS_FIELD_MAC(  99,   5, SIP5_FDB_FDB_TABLE_FIELDS_RESERVED_99_103_E),
    PRV_CPSS_FIELD_MAC( 104,   3, SIP5_FDB_FDB_TABLE_FIELDS_SOURCE_ID_11_9_E),
    PRV_CPSS_FIELD_MAC( 107,   3, SIP5_FDB_FDB_TABLE_FIELDS_DA_ACCESS_LEVEL_E),
    PRV_CPSS_FIELD_MAC( 110,   3, SIP5_FDB_FDB_TABLE_FIELDS_SA_ACCESS_LEVEL_E),
    PRV_CPSS_FIELD_MAC( 113,   3, SIP5_FDB_FDB_TABLE_FIELDS_SOURCE_ID_8_6_E),
    PRV_CPSS_FIELD_MAC( 116,   3, SIP5_FDB_FDB_TABLE_FIELDS_RESERVED_116_118_E),
    PRV_CPSS_FIELD_MAC( 107,  12, SIP5_FDB_FDB_TABLE_FIELDS_ORIG_VID1_E),
    PRV_CPSS_FIELD_MAC( 119,   1, SIP5_FDB_FDB_TABLE_FIELDS_IS_STATIC_E),
    PRV_CPSS_FIELD_MAC( 120,   1, SIP5_FDB_FDB_TABLE_FIELDS_MULTIPLE_E),
    PRV_CPSS_FIELD_MAC( 121,   3, SIP5_FDB_FDB_TABLE_FIELDS_DA_CMD_E),
    PRV_CPSS_FIELD_MAC( 124,   3, SIP5_FDB_FDB_TABLE_FIELDS_SA_CMD_E),
    PRV_CPSS_FIELD_MAC( 127,   1, SIP5_FDB_FDB_TABLE_FIELDS_DA_ROUTE_E),
    PRV_CPSS_FIELD_MAC( 128,   1, SIP5_FDB_FDB_TABLE_FIELDS_SP_UNKNOWN_E),
    PRV_CPSS_FIELD_MAC( 129,   3, SIP5_FDB_FDB_TABLE_FIELDS_SA_QOS_PARAM_SET_IDX_E),
    PRV_CPSS_FIELD_MAC( 132,   3, SIP5_FDB_FDB_TABLE_FIELDS_DA_QOS_PARAM_SET_IDX_E),
    PRV_CPSS_FIELD_MAC( 135,   1, SIP5_FDB_FDB_TABLE_FIELDS_APP_SPECIFIC_CPU_CODE_E),
    PRV_CPSS_FIELD_MAC( 136,   1, SIP5_FDB_FDB_TABLE_FIELDS_SA_LOOKUP_INGRESS_MIRROR_TO_ANALYZER_E),
    PRV_CPSS_FIELD_MAC( 137,   1, SIP5_FDB_FDB_TABLE_FIELDS_DA_LOOKUP_INGRESS_MIRROR_TO_ANALYZER_E),
    PRV_CPSS_FIELD_MAC(   6,  12, SIP5_FDB_FDB_TABLE_FIELDS_VRF_ID_E),
    PRV_CPSS_FIELD_MAC(  18,   1, SIP5_FDB_FDB_TABLE_FIELDS_IPV6_SCOPE_CHECK_E),
    PRV_CPSS_FIELD_MAC(  18,  24, SIP5_FDB_FDB_TABLE_FIELDS_FCOE_D_ID_E),
    PRV_CPSS_FIELD_MAC(  18,  32, SIP5_FDB_FDB_TABLE_FIELDS_IPV4_DIP_E),
    PRV_CPSS_FIELD_MAC(  19,   1, SIP5_FDB_FDB_TABLE_FIELDS_IPV6_DST_SITE_ID_E),
    PRV_CPSS_FIELD_MAC(  50,   1, SIP5_FDB_FDB_TABLE_FIELDS_DEC_TTL_OR_HOP_COUNT_E),
    PRV_CPSS_FIELD_MAC(  51,   1, SIP5_FDB_FDB_TABLE_FIELDS_BYPASS_TTL_OPTIONS_OR_HOP_EXTENSION_E),
    PRV_CPSS_FIELD_MAC(  52,   3, SIP5_FDB_FDB_TABLE_FIELDS_INGRESS_MIRROR_TO_ANALYZER_INDEX_E),
    PRV_CPSS_FIELD_MAC(  55,   1, SIP5_FDB_FDB_TABLE_FIELDS_QOS_PROFILE_MARKING_EN_E),
    PRV_CPSS_FIELD_MAC(  56,   7, SIP5_FDB_FDB_TABLE_FIELDS_QOS_PROFILE_INDEX_E),
    PRV_CPSS_FIELD_MAC(  63,   1, SIP5_FDB_FDB_TABLE_FIELDS_QOS_PROFILE_PRECEDENCE_E),
    PRV_CPSS_FIELD_MAC(  64,   2, SIP5_FDB_FDB_TABLE_FIELDS_MODIFY_UP_E),
    PRV_CPSS_FIELD_MAC(  66,   2, SIP5_FDB_FDB_TABLE_FIELDS_MODIFY_DSCP_E),
    PRV_CPSS_FIELD_MAC(  68,   3, SIP5_FDB_FDB_TABLE_FIELDS_COUNTER_SET_INDEX_E),
    PRV_CPSS_FIELD_MAC(  71,   1, SIP5_FDB_FDB_TABLE_FIELDS_ARP_BC_TRAP_MIRROR_EN_E),
    PRV_CPSS_FIELD_MAC(  72,   6, SIP5_FDB_FDB_TABLE_FIELDS_DIP_ACCESS_LEVEL_E),
    PRV_CPSS_FIELD_MAC(  78,   1, SIP5_FDB_FDB_TABLE_FIELDS_ICMP_REDIRECT_EXCEP_MIRROR_EN_E),
    PRV_CPSS_FIELD_MAC(  79,   3, SIP5_FDB_FDB_TABLE_FIELDS_MTU_INDEX_E),
    PRV_CPSS_FIELD_MAC(  82,   1, SIP5_FDB_FDB_TABLE_FIELDS_USE_VIDX_E),
    PRV_CPSS_FIELD_MAC(  98,  10, SIP5_FDB_FDB_TABLE_FIELDS_TARGET_DEVICE_E),
    PRV_CPSS_FIELD_MAC( 108,  13, SIP5_FDB_FDB_TABLE_FIELDS_NEXT_HOP_EVLAN_E),
    PRV_CPSS_FIELD_MAC( 121,   1, SIP5_FDB_FDB_TABLE_FIELDS_TUNNEL_START_E),
    PRV_CPSS_FIELD_MAC( 122,   1, SIP5_FDB_FDB_TABLE_FIELDS_TUNNEL_TYPE_E),
    PRV_CPSS_FIELD_MAC( 122,  18, SIP5_FDB_FDB_TABLE_FIELDS_ARP_PTR_E),
    PRV_CPSS_FIELD_MAC( 123,  16, SIP5_FDB_FDB_TABLE_FIELDS_TUNNEL_PTR_E),
    PRV_CPSS_FIELD_MAC(   6,  32, SIP5_FDB_FDB_TABLE_FIELDS_IPV6_DIP_0_E),
    PRV_CPSS_FIELD_MAC(  38,  32, SIP5_FDB_FDB_TABLE_FIELDS_IPV6_DIP_1_E),
    PRV_CPSS_FIELD_MAC(  70,  32, SIP5_FDB_FDB_TABLE_FIELDS_IPV6_DIP_2_E),
    PRV_CPSS_FIELD_MAC( 102,  32, SIP5_FDB_FDB_TABLE_FIELDS_IPV6_DIP_3_E),
    PRV_CPSS_FIELD_MAC( 134,   4, SIP5_FDB_FDB_TABLE_FIELDS_NH_DATA_BANK_NUM_E),
    PRV_CPSS_FIELD_MAC(  31,   1, SIP5_10_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TS_IS_NAT_E)
};

static const PRV_CPSS_ENTRY_FORMAT_TABLE_STC prvCpssDxChSip5_20FdbAuMsgTableFieldsFormat[SIP5_10_FDB_AU_MSG_TABLE_FIELDS___LAST_VALUE___E] =
{
    PRV_CPSS_FIELD_MAC(   0,   1, SIP5_FDB_AU_MSG_TABLE_FIELDS_MESSAGE_ID_E),
    PRV_CPSS_FIELD_MAC(   1,   3, SIP5_FDB_AU_MSG_TABLE_FIELDS_MSG_TYPE_E),
    PRV_CPSS_FIELD_MAC(   4,   3, SIP5_FDB_AU_MSG_TABLE_FIELDS_FDB_ENTRY_TYPE_E),
    PRV_CPSS_FIELD_MAC(   7,   1, SIP5_FDB_AU_MSG_TABLE_FIELDS_VALID_E),
    PRV_CPSS_FIELD_MAC(   8,   1, SIP5_FDB_AU_MSG_TABLE_FIELDS_SKIP_E),
    PRV_CPSS_FIELD_MAC(   9,   1, SIP5_FDB_AU_MSG_TABLE_FIELDS_AGE_E),
    PRV_CPSS_FIELD_MAC(  10,   9, SIP5_FDB_AU_MSG_TABLE_FIELDS_MAC_ADDR_INDEX_8_0_E),
    PRV_CPSS_FIELD_MAC(  10,   1, SIP5_FDB_AU_MSG_TABLE_FIELDS_CHAIN_TOO_LONG_E),
    PRV_CPSS_FIELD_MAC(  11,   5, SIP5_FDB_AU_MSG_TABLE_FIELDS_MAC_ADDR_OFFSET_E),
    PRV_CPSS_FIELD_MAC(  16,   1, SIP5_FDB_AU_MSG_TABLE_FIELDS_ENTRY_FOUND_E),
    PRV_CPSS_FIELD_MAC(  19,  48, SIP5_FDB_AU_MSG_TABLE_FIELDS_MAC_ADDR_E),
    PRV_CPSS_FIELD_MAC(  67,  10, SIP5_FDB_AU_MSG_TABLE_FIELDS_DEV_ID_E),
    PRV_CPSS_FIELD_MAC(  77,   6, SIP5_FDB_AU_MSG_TABLE_FIELDS_SOURCE_ID_5_0_E),
    PRV_CPSS_FIELD_MAC(  19,  32, SIP5_FDB_AU_MSG_TABLE_FIELDS_DIP_E),
    PRV_CPSS_FIELD_MAC(  51,  32, SIP5_FDB_AU_MSG_TABLE_FIELDS_SIP_E),
    PRV_CPSS_FIELD_MAC(  83,  13, SIP5_FDB_AU_MSG_TABLE_FIELDS_FID_E),
    PRV_CPSS_FIELD_MAC(  96,  16, SIP5_FDB_AU_MSG_TABLE_FIELDS_VIDX_E),
    PRV_CPSS_FIELD_MAC(  96,   1, SIP5_FDB_AU_MSG_TABLE_FIELDS_IS_TRUNK_E),
    PRV_CPSS_FIELD_MAC(  97,  14, SIP5_FDB_AU_MSG_TABLE_FIELDS_EPORT_NUM_E),
    PRV_CPSS_FIELD_MAC(  97,  12, SIP5_FDB_AU_MSG_TABLE_FIELDS_TRUNK_NUM_E),
    PRV_CPSS_FIELD_MAC( 112,   8, SIP5_FDB_AU_MSG_TABLE_FIELDS_USER_DEFINED_E),
    PRV_CPSS_FIELD_MAC( 112,   5, SIP5_FDB_AU_MSG_TABLE_FIELDS_RESERVED_109_113_E),
    PRV_CPSS_FIELD_MAC( 117,   3, SIP5_FDB_AU_MSG_TABLE_FIELDS_SOURCE_ID_11_9_E),
    PRV_CPSS_FIELD_MAC( 120,  12, SIP5_FDB_AU_MSG_TABLE_FIELDS_ORIG_VID1_E),
    PRV_CPSS_FIELD_MAC( 120,   3, SIP5_FDB_AU_MSG_TABLE_FIELDS_DA_ACCESS_LEVEL_E),
    PRV_CPSS_FIELD_MAC( 123,   3, SIP5_FDB_AU_MSG_TABLE_FIELDS_SA_ACCESS_LEVEL_E),
    PRV_CPSS_FIELD_MAC( 126,   3, SIP5_FDB_AU_MSG_TABLE_FIELDS_SOURCE_ID_8_6_E),
    PRV_CPSS_FIELD_MAC( 132,   1, SIP5_FDB_AU_MSG_TABLE_FIELDS_APP_SPECIFIC_CPU_CODE_E),
    PRV_CPSS_FIELD_MAC( 133,   1, SIP5_FDB_AU_MSG_TABLE_FIELDS_SP_UNKNOWN_E),
    PRV_CPSS_FIELD_MAC( 134,   1, SIP5_FDB_AU_MSG_TABLE_FIELDS_SEARCH_TYPE_E),
    PRV_CPSS_FIELD_MAC( 135,  12, SIP5_FDB_AU_MSG_TABLE_FIELDS_MAC_ADDR_INDEX_20_9_E),
    PRV_CPSS_FIELD_MAC( 147,   1, SIP5_FDB_AU_MSG_TABLE_FIELDS_MULTIPLE_E),
    PRV_CPSS_FIELD_MAC( 148,   1, SIP5_FDB_AU_MSG_TABLE_FIELDS_DA_ROUTE_E),
    PRV_CPSS_FIELD_MAC( 149,   3, SIP5_FDB_AU_MSG_TABLE_FIELDS_SA_QOS_PARAM_SET_IDX_E),
    PRV_CPSS_FIELD_MAC( 152,   3, SIP5_FDB_AU_MSG_TABLE_FIELDS_DA_QOS_PARAM_SET_IDX_E),
    PRV_CPSS_FIELD_MAC( 155,   1, SIP5_FDB_AU_MSG_TABLE_FIELDS_IS_STATIC_E),
    PRV_CPSS_FIELD_MAC( 156,   3, SIP5_FDB_AU_MSG_TABLE_FIELDS_DA_CMD_E),
    PRV_CPSS_FIELD_MAC( 159,   3, SIP5_FDB_AU_MSG_TABLE_FIELDS_SA_CMD_E),
    PRV_CPSS_FIELD_MAC( 162,   1, SIP5_FDB_AU_MSG_TABLE_FIELDS_SA_LOOKUP_INGRESS_MIRROR_TO_ANALYZER_E),
    PRV_CPSS_FIELD_MAC( 163,   1, SIP5_FDB_AU_MSG_TABLE_FIELDS_DA_LOOKUP_INGRESS_MIRROR_TO_ANALYZER_E),
    PRV_CPSS_FIELD_MAC( 164,   1, SIP5_FDB_AU_MSG_TABLE_FIELDS_FDB_LOOKUP_KEY_MODE_E),
    PRV_CPSS_FIELD_MAC(  16,   3, SIP5_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_UP0_E),
    PRV_CPSS_FIELD_MAC( 135,   1, SIP5_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_IS_MOVED_E),
    PRV_CPSS_FIELD_MAC( 136,   1, SIP5_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_OLD_IS_TRUNK_E),
    PRV_CPSS_FIELD_MAC( 137,  14, SIP5_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_OLD_EPORT_E),
    PRV_CPSS_FIELD_MAC( 137,  12, SIP5_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_OLD_TRUNK_NUM_E),
    PRV_CPSS_FIELD_MAC( 152,  10, SIP5_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_OLD_DEVICE_E),
    PRV_CPSS_FIELD_MAC( 162,  12, SIP5_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_OLD_SRC_ID_E),
    PRV_CPSS_FIELD_MAC(  10,  21, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_MAC_ADDR_INDEX_E),
    PRV_CPSS_FIELD_MAC(  31,  12, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_VRF_ID_E),
    PRV_CPSS_FIELD_MAC(  43,   1, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IPV6_SCOPE_CHECK_E),
    PRV_CPSS_FIELD_MAC(  43,  24, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_FCOE_D_ID_E),
    PRV_CPSS_FIELD_MAC(  43,  32, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IPV4_DIP_E),
    PRV_CPSS_FIELD_MAC(  44,   1, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IPV6_DST_SITE_ID_E),
    PRV_CPSS_FIELD_MAC(  75,   1, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_DEC_TTL_OR_HOP_COUNT_E),
    PRV_CPSS_FIELD_MAC(  76,   1, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_BYPASS_TTL_OPTIONS_OR_HOP_EXTENSION_E),
    PRV_CPSS_FIELD_MAC(  77,   3, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_INGRESS_MIRROR_TO_ANALYZER_INDEX_E),
    PRV_CPSS_FIELD_MAC(  80,   1, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_QOS_PROFILE_MARKING_EN_E),
    PRV_CPSS_FIELD_MAC(  81,   7, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_QOS_PROFILE_INDEX_E),
    PRV_CPSS_FIELD_MAC(  88,   1, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_QOS_PROFILE_PRECEDENCE_E),
    PRV_CPSS_FIELD_MAC(  89,   2, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_MODIFY_UP_E),
    PRV_CPSS_FIELD_MAC(  91,   2, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_MODIFY_DSCP_E),
    PRV_CPSS_FIELD_MAC(  93,   3, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_COUNTER_SET_INDEX_E),
    PRV_CPSS_FIELD_MAC(  96,   1, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_ARP_BC_TRAP_MIRROR_EN_E),
    PRV_CPSS_FIELD_MAC(  97,   6, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_DIP_ACCESS_LEVEL_E),
    PRV_CPSS_FIELD_MAC( 103,   1, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_ICMP_REDIRECT_EXCEP_MIRROR_EN_E),
    PRV_CPSS_FIELD_MAC( 104,   3, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_MTU_INDEX_E),
    PRV_CPSS_FIELD_MAC( 107,   1, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_USE_VIDX_E),
    PRV_CPSS_FIELD_MAC( 108,   1, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IS_TRUNK_E),
    PRV_CPSS_FIELD_MAC( 108,  16, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_EVIDX_E),
    PRV_CPSS_FIELD_MAC( 109,  10, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_TARGET_DEVICE_E),
    PRV_CPSS_FIELD_MAC( 109,  12, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_TRUNK_NUM_E),
    PRV_CPSS_FIELD_MAC( 119,  14, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_EPORT_NUM_E),
    PRV_CPSS_FIELD_MAC( 133,  13, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_NEXT_HOP_EVLAN_E),
    PRV_CPSS_FIELD_MAC( 146,   1, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_TUNNEL_START_E),
    PRV_CPSS_FIELD_MAC( 147,   1, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_TUNNEL_TYPE_E),
    PRV_CPSS_FIELD_MAC( 147,  18, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_ARP_PTR_E),
    PRV_CPSS_FIELD_MAC( 148,  16, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_TUNNEL_PTR_E),
    PRV_CPSS_FIELD_MAC(  31,  32, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_0_E),
    PRV_CPSS_FIELD_MAC(  63,  32, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_1_E),
    PRV_CPSS_FIELD_MAC(  95,  32, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_2_E),
    PRV_CPSS_FIELD_MAC( 127,  32, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_3_E),
    PRV_CPSS_FIELD_MAC( 159,   4, SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_NH_DATA_BANK_NUM_E),
    PRV_CPSS_FIELD_MAC( 164,   1, SIP5_10_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_TS_IS_NAT_E)
};

static const PRV_CPSS_ENTRY_FORMAT_TABLE_STC prvCpssDxChSip5_20IpclActionTableFieldsFormat[SIP5_20_IPCL_ACTION_TABLE_FIELDS___LAST_VALUE___E] =
{
    PRV_CPSS_FIELD_MAC(   0,   8, SIP5_20_IPCL_ACTION_TABLE_FIELDS_CPU_CODE_E),
    PRV_CPSS_FIELD_MAC(   8,   3, SIP5_20_IPCL_ACTION_TABLE_FIELDS_COMMAND_E),
    PRV_CPSS_FIELD_MAC(  11,   3, SIP5_20_IPCL_ACTION_TABLE_FIELDS_REDIRECT_COMMAND_E),
    PRV_CPSS_FIELD_MAC(  14,   1, SIP5_20_IPCL_ACTION_TABLE_FIELDS_ACTION_STOP_E),
    PRV_CPSS_FIELD_MAC(  15,   1, SIP5_20_IPCL_ACTION_TABLE_FIELDS_SET_MAC2ME_E),
    PRV_CPSS_FIELD_MAC(  16,   1, SIP5_20_IPCL_ACTION_TABLE_FIELDS_POLICY1_LOOKUP_MODE_E),
    PRV_CPSS_FIELD_MAC(  17,   1, SIP5_20_IPCL_ACTION_TABLE_FIELDS_POLICY2_LOOKUP_MODE_E),
    PRV_CPSS_FIELD_MAC(  18,  13, SIP5_20_IPCL_ACTION_TABLE_FIELDS_IPCL_PROFILE_INDEX_E),
    PRV_CPSS_FIELD_MAC(  16,   1, SIP5_20_IPCL_ACTION_TABLE_FIELDS_USE_VIDX_E),
    PRV_CPSS_FIELD_MAC(  17,  16, SIP5_20_IPCL_ACTION_TABLE_FIELDS_VIDX_E),
    PRV_CPSS_FIELD_MAC(  17,   1, SIP5_20_IPCL_ACTION_TABLE_FIELDS_IS_TRUNK_E),
    PRV_CPSS_FIELD_MAC(  18,  14, SIP5_20_IPCL_ACTION_TABLE_FIELDS_TRG_PORT_E),
    PRV_CPSS_FIELD_MAC(  32,  10, SIP5_20_IPCL_ACTION_TABLE_FIELDS_TARGET_DEVICE_E),
    PRV_CPSS_FIELD_MAC(  18,  14, SIP5_20_IPCL_ACTION_TABLE_FIELDS_TRUNK_ID_E),
    PRV_CPSS_FIELD_MAC(  42,   1, SIP5_20_IPCL_ACTION_TABLE_FIELDS_VNT_L2_ECHO_E),
    PRV_CPSS_FIELD_MAC(  43,   1, SIP5_20_IPCL_ACTION_TABLE_FIELDS_TUNNEL_START_E),
    PRV_CPSS_FIELD_MAC(  44,  18, SIP5_20_IPCL_ACTION_TABLE_FIELDS_ARP_POINTER_E),
    PRV_CPSS_FIELD_MAC(  44,  16, SIP5_20_IPCL_ACTION_TABLE_FIELDS_TUNNEL_POINTER_E),
    PRV_CPSS_FIELD_MAC(  60,   1, SIP5_20_IPCL_ACTION_TABLE_FIELDS_TUNNEL_START_PASSENGER_TYPE_E),
    PRV_CPSS_FIELD_MAC(  32,   1, SIP5_20_IPCL_ACTION_TABLE_FIELDS_PBR_MODE_E),
    PRV_CPSS_FIELD_MAC(  33,  18, SIP5_20_IPCL_ACTION_TABLE_FIELDS_IP_NEXT_HOP_ENTRY_INDEX_E),
    PRV_CPSS_FIELD_MAC(  52,  12, SIP5_20_IPCL_ACTION_TABLE_FIELDS_VRF_ID_E),
    PRV_CPSS_FIELD_MAC(  16,  28, SIP5_20_IPCL_ACTION_TABLE_FIELDS_MAC_SA_27_0_E),
    PRV_CPSS_FIELD_MAC(  62,  20, SIP5_20_IPCL_ACTION_TABLE_FIELDS_MAC_SA_47_28_E),
    PRV_CPSS_FIELD_MAC(  64,   1, SIP5_20_IPCL_ACTION_TABLE_FIELDS_ACTIVATE_METER_E),
    PRV_CPSS_FIELD_MAC(  65,   1, SIP5_20_IPCL_ACTION_TABLE_FIELDS_BIND_TO_POLICER_COUNTER_E),
    PRV_CPSS_FIELD_MAC(  66,  14, SIP5_20_IPCL_ACTION_TABLE_FIELDS_POLICER_PTR_E),
    PRV_CPSS_FIELD_MAC(  80,   2, SIP5_20_IPCL_ACTION_TABLE_FIELDS_RESERVED_E),
    PRV_CPSS_FIELD_MAC(  82,   1, SIP5_20_IPCL_ACTION_TABLE_FIELDS_ENABLE_MIRROR_TCP_RST_OR_FIN_E),
    PRV_CPSS_FIELD_MAC(  83,   3, SIP5_20_IPCL_ACTION_TABLE_FIELDS_MIRROR_TO_ANALYZER_PORT_E),
    PRV_CPSS_FIELD_MAC(  86,   1, SIP5_20_IPCL_ACTION_TABLE_FIELDS_MODIFY_MAC_DA_E),
    PRV_CPSS_FIELD_MAC(  87,   1, SIP5_20_IPCL_ACTION_TABLE_FIELDS_MODIFY_MAC_SA_E),
    PRV_CPSS_FIELD_MAC(  87,   1, SIP5_20_IPCL_ACTION_TABLE_FIELDS_SET_EGRESS_FILTER_REGISTERED_E),
    PRV_CPSS_FIELD_MAC(  88,   1, SIP5_20_IPCL_ACTION_TABLE_FIELDS_BRIDGE_BYPASS_E),
    PRV_CPSS_FIELD_MAC(  89,   1, SIP5_20_IPCL_ACTION_TABLE_FIELDS_BYPASS_INGRESS_PIPE_E),
    PRV_CPSS_FIELD_MAC(  90,   1, SIP5_20_IPCL_ACTION_TABLE_FIELDS_EN_NESTED_VLAN_E),
    PRV_CPSS_FIELD_MAC(  91,   1, SIP5_20_IPCL_ACTION_TABLE_FIELDS_SET_SST_ID_E),
    PRV_CPSS_FIELD_MAC(  92,  12, SIP5_20_IPCL_ACTION_TABLE_FIELDS_SST_ID_E),
    PRV_CPSS_FIELD_MAC( 104,   1, SIP5_20_IPCL_ACTION_TABLE_FIELDS_USER_AC_ENABLE_E),
    PRV_CPSS_FIELD_MAC( 105,   1, SIP5_20_IPCL_ACTION_TABLE_FIELDS_ACTIVATE_COUNTER_E),
    PRV_CPSS_FIELD_MAC( 106,  14, SIP5_20_IPCL_ACTION_TABLE_FIELDS_COUNTER_INDEX_E),
    PRV_CPSS_FIELD_MAC( 120,   1, SIP5_20_IPCL_ACTION_TABLE_FIELDS_VLAN_PRECEDENCE_E),
    PRV_CPSS_FIELD_MAC( 121,   2, SIP5_20_IPCL_ACTION_TABLE_FIELDS_VLAN_COMMAND_E),
    PRV_CPSS_FIELD_MAC( 123,  13, SIP5_20_IPCL_ACTION_TABLE_FIELDS_VID0_E),
    PRV_CPSS_FIELD_MAC( 136,   1, SIP5_20_IPCL_ACTION_TABLE_FIELDS_QOS_PROFILE_MARKING_ENABLE_E),
    PRV_CPSS_FIELD_MAC( 137,   1, SIP5_20_IPCL_ACTION_TABLE_FIELDS_QOS_PRECEDENCE_E),
    PRV_CPSS_FIELD_MAC( 138,  10, SIP5_20_IPCL_ACTION_TABLE_FIELDS_QOS_PROFILE_E),
    PRV_CPSS_FIELD_MAC( 148,   2, SIP5_20_IPCL_ACTION_TABLE_FIELDS_MODIFY_DSCP_E),
    PRV_CPSS_FIELD_MAC( 150,   2, SIP5_20_IPCL_ACTION_TABLE_FIELDS_MODIFY_UP_E),
    PRV_CPSS_FIELD_MAC( 152,   2, SIP5_20_IPCL_ACTION_TABLE_FIELDS_VLAN1_CMD_E),
    PRV_CPSS_FIELD_MAC( 154,   2, SIP5_20_IPCL_ACTION_TABLE_FIELDS_UP1_CMD_E),
    PRV_CPSS_FIELD_MAC( 156,  12, SIP5_20_IPCL_ACTION_TABLE_FIELDS_VID1_E),
    PRV_CPSS_FIELD_MAC( 168,   3, SIP5_20_IPCL_ACTION_TABLE_FIELDS_UP1_E),
    PRV_CPSS_FIELD_MAC( 171,   1, SIP5_20_IPCL_ACTION_TABLE_FIELDS_IPCL_RESERVED_EN_E),
    PRV_CPSS_FIELD_MAC( 172,  20, SIP5_20_IPCL_ACTION_TABLE_FIELDS_IPCL_TO_TXQ_RESERVED_E),
    PRV_CPSS_FIELD_MAC( 192,   1, SIP5_20_IPCL_ACTION_TABLE_FIELDS_TIMESTAMP_EN_E),
    PRV_CPSS_FIELD_MAC( 193,   7, SIP5_20_IPCL_ACTION_TABLE_FIELDS_OFFSET_INDEX_E),
    PRV_CPSS_FIELD_MAC( 200,   1, SIP5_20_IPCL_ACTION_TABLE_FIELDS_OAM_PROCESSING_EN_E),
    PRV_CPSS_FIELD_MAC( 201,   1, SIP5_20_IPCL_ACTION_TABLE_FIELDS_OAM_PROFILE_E),
    PRV_CPSS_FIELD_MAC( 202,  13, SIP5_20_IPCL_ACTION_TABLE_FIELDS_FLOW_ID_E),
    PRV_CPSS_FIELD_MAC( 215,   1, SIP5_20_IPCL_ACTION_TABLE_FIELDS_RESERVED_1_E),
    PRV_CPSS_FIELD_MAC( 216,   1, SIP5_20_IPCL_ACTION_TABLE_FIELDS_ASSIGN_SRC_EPORT_EN_E),
    PRV_CPSS_FIELD_MAC( 217,  14, SIP5_20_IPCL_ACTION_TABLE_FIELDS_SRC_EPORT_E)
};
/* bobcat3 tables - end */

/* xCat3, AC5 direct table information */
static const PRV_CPSS_DXCH_TABLES_INFO_DIRECT_STC directAccessXcat3TableInfo[] =
{
    /* PRV_DXCH_XCAT3_TABLE_VLAN_PORT_PROTOCOL_E */
    {0x16004000, 0x8, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /* PRV_DXCH_XCAT3_TABLE_PORT_VLAN_QOS_E */
    {0x16001000, 0x10, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /* PRV_DXCH_XCAT3_TABLE_IPCL_CFG_E */
    {0x15010000, 0x8, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /* PRV_DXCH_XCAT3_TABLE_STG_E */
    {0x03880000, 0x10, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /* PRV_DXCH_XCAT3_TABLE_VLAN_E */
    {0x03A00000, 0x20, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /* PRV_DXCH_XCAT3_TABLE_MULTICAST_E */
    {0x03900000, 0x10, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /* PRV_DXCH_XCAT3_TABLE_ROUTE_HA_MAC_SA_E */
    {0x1C008000, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /* PRV_DXCH_XCAT3_TABLE_ROUTE_HA_ARP_DA_E */
    {0x1C040000, 0x20, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /* PRV_DXCH_XCAT3_TABLE_EPCL_CFG_E */
    {0x1D008000, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /* PRV_DXCH_XCAT3_TABLE_TUNNEL_START_CFG_E */
    {0x1C040000, 0x20, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /* PRV_DXCH_XCAT3_TABLE_QOS_PROFILE_TO_ROUTE_BLOCK_E */
    {0x04000300, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /* PRV_DXCH_XCAT3_TABLE_ROUTE_ACCESS_MATRIX_E */
    {0x04000440, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /* PRV_DXCH_XCAT3_UC_MC_ROUTE_NEXT_HOP_E */
    {0x04300000, 0x10, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /* PRV_DXCH_XCAT3_ROUTE_NEXT_HOP_AGE_E */
    {0x04001000, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /* PRV_DXCH_XCAT3_TABLE_MAC2ME_E */
    {0x16001600, 0x10, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /* PRV_DXCH_XCAT3_TABLE_INGRESS_TRANSLATION_E */
    {0x1600C000, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /* PRV_DXCH_XCAT3_TABLE_EGRESS_TRANSLATION_E */
    {0x1C010000, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /* PRV_DXCH_XCAT3_TABLE_VRF_ID_E */
    {0x03980000, 0x10, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /* PRV_DXCH_XCAT3_LTT_TT_ACTION_E */
    {0x1B0C0000, 0x20, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /* PRV_DXCH_XCAT3_TABLE_IPCL_LOOKUP1_CFG_E */
    {0x15020000, 0x8, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /* PRV_DXCH_XCAT3_TABLE_IPCL_UDB_CFG_E */
    {0x16010000, 0x20, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /*PRV_DXCH_XCAT3_TABLE_BCN_PROFILE_E */
    {0x03400100, 0x10, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /*PRV_DXCH_XCAT3_TABLE_EGRESS_POLICER_REMARKING_E*/
    {0x06080000, 0x8, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /*PRV_DXCH_XCAT3_TABLE_MLL_L2MLL_VIDX_ENABLE_E*/
    {0x19008000, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /*PRV_DXCH_XCAT_TABLE_MLL_L2MLL_POINTER_MAPPING_E*/
    {0x19009000, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /*PRV_DXCH_XCAT3_TABLE_INGRESS_ECID_E*/
    {0x3A80000, 0x10, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /*PRV_DXCH_XCAT3_TABLE_MLL_E*/
    {0x19080000, 0x10, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /*PRV_DXCH_LION_TABLE_L2_PORT_ISOLATION_E*/
    {0x02E40004, 0x10, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /*PRV_DXCH_LION_TABLE_L3_PORT_ISOLATION_E*/
    {0x02E40008, 0x10, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /*PRV_DXCH_XCAT3_TABLE_INGRESS_POLICER_METERING_0_E*/
    {0x17040000, 0x20, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /*PRV_DXCH_XCAT3_TABLE_INGRESS_POLICER_METERING_1_E*/
    {0x18040000, 0x20, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /*PRV_DXCH_XCAT3_TABLE_EGRESS_POLICER_METERING_E*/
    {0x6040000, 0x20, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /*PRV_DXCH_XCAT3_TABLE_INGRESS_POLICER_COUNTING_0_E*/
    {0x17060000, 0x20, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /*PRV_DXCH_XCAT3_TABLE_INGRESS_POLICER_COUNTING_1_E*/
    {0x18060000, 0x20, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /*PRV_DXCH_XCAT3_TABLE_EGRESS_POLICER_COUNTING_E*/
    {0x6060000, 0x20, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}
};

typedef struct{
    CPSS_DXCH_TABLE_ENT                 globalIndex;
    PRV_CPSS_DXCH_TABLES_INFO_DIRECT_STC    directAccessInfo;
}PRV_CPSS_DXCH_TABLES_INFO_DIRECT_EXT_STC;

/* Lion2 direct table information */
static const PRV_CPSS_DXCH_TABLES_INFO_DIRECT_EXT_STC directAccessLion2TableInfoExt[] =
{
    {CPSS_DXCH_TABLE_VLAN_E,
    /* PRV_DXCH_XCAT_TABLE_VLAN_E */
    {  TABLES_INFO_DIRECT_NOT_EXISTS_CNS }}, /* see ingress/egress tables instead */

    {CPSS_DXCH3_TABLE_VRF_ID_E,
    /* PRV_DXCH_XCAT_TABLE_VRF_ID_E */
    {  TABLES_INFO_DIRECT_NOT_EXISTS_CNS }}, /* the field is in ingress vlan entry */

    {CPSS_DXCH_TABLE_PORT_VLAN_QOS_E,
    /* PRV_DXCH_XCAT_TABLE_PORT_VLAN_QOS_E */
    {0x01001000, 0x10, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

    {CPSS_DXCH_TABLE_STG_E,
    /* CPSS_DXCH_TABLE_STG_E */
    {  TABLES_INFO_DIRECT_NOT_EXISTS_CNS }}, /* see ingress/egress tables instead */

    {CPSS_DXCH3_LTT_TT_ACTION_E,
    /* PRV_DXCH_XCAT_LTT_TT_ACTION_E */
    {0x0D8C0000, 0x20, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

    {CPSS_DXCH_TABLE_MULTICAST_E,
    /* PRV_DXCH_XCAT_TABLE_MULTICAST_E */
    {0x11860000, 0x10, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

    {CPSS_DXCH_TABLE_ROUTE_HA_MAC_SA_E,
    /* PRV_CPSS_XCAT_TABLE_ROUTE_HA_MAC_SA_E         */
    {0x0E808000, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

    {CPSS_DXCH_TABLE_ROUTE_HA_ARP_DA_E,
    /* PRV_CPSS_XCAT_TABLE_ROUTE_HA_ARP_DA_E         */
    {0x0E840000, 0x20, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

    {CPSS_DXCH3_TABLE_INGRESS_VLAN_TRANSLATION_E,
    /* PRV_DXCH_XCAT_TABLE_INGRESS_TRANSLATION_E */
    {0x0100C000, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

    {CPSS_DXCH3_TABLE_EGRESS_VLAN_TRANSLATION_E,
    /* PRV_DXCH_XCAT_TABLE_EGRESS_TRANSLATION_E      */
    {0x0E810000, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

    {CPSS_DXCH_TABLE_PCL_CONFIG_E,
    /* PRV_DXCH_XCAT_TABLE_IPCL_CFG_E                */
    {0x0B810000, 0x8, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

    {CPSS_DXCH_XCAT_TABLE_INGRESS_PCL_LOOKUP1_CONFIG_E,
    /* PRV_DXCH_XCAT_TABLE_IPCL_LOOKUP1_CFG_E        */
    {0x0B820000, 0x8, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

    {CPSS_DXCH2_TABLE_EGRESS_PCL_CONFIG_E,
    /* PRV_DXCH_XCAT_TABLE_EPCL_CFG_E                */
    {0x0E008000, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

    {CPSS_DXCH_XCAT_TABLE_INGRESS_PCL_UDB_CONFIG_E,
    /* PRV_DXCH_XCAT_TABLE_IPCL_UDB_CFG_E */
    {0x01010000, 0x20, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

    {CPSS_DXCH2_TABLE_TUNNEL_START_CONFIG_E,
    /* PRV_DXCH_XCAT_TABLE_TUNNEL_START_CFG_E        */
    {0x0E840000, 0x20, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

    {CPSS_DXCH_TABLE_VLAN_PORT_PROTOCOL_E,
    /* PRV_DXCH_XCAT_TABLE_VLAN_PORT_PROTOCOL_E */
    {0x01004000, 0x8, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

    {CPSS_DXCH3_TABLE_MAC2ME_E,
    /* PRV_CPSS_XCAT_TABLE_MAC2ME_E */
    {0x01001600, 0x10, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

    {CPSS_DXCH_XCAT_TABLE_BCN_PROFILES_E,
    /* PRV_DXCH_XCAT_TABLE_BCN_PROFILE_E             */
    {TABLES_INFO_DIRECT_NOT_EXISTS_CNS}},

    {CPSS_DXCH_XCAT_TABLE_EGRESS_POLICER_REMARKING_E,
    /*PRV_DXCH_XCAT_TABLE_EGRESS_POLICER_REMARKING_E */
    {0x07880000, 0x8, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

    {CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
    /* PRV_DXCH_LION_TABLE_VLAN_INGRESS_E            */
    {0x118A0000, 0x20, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

    {CPSS_DXCH_LION_TABLE_VLAN_EGRESS_E,
    /* PRV_DXCH_LION_TABLE_VLAN_EGRESS_E             */
    {0x11800000, 0x40, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

    {CPSS_DXCH_LION_TABLE_STG_INGRESS_E,
    /* PRV_DXCH_LION_TABLE_STG_INGRESS_E             */
    {0x118D0000, 0x10, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

    {CPSS_DXCH_LION_TABLE_STG_EGRESS_E,
    /* PRV_DXCH_LION_TABLE_STG_EGRESS_E              */
    {0x11840000, 0x10, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

    {CPSS_DXCH_LION_TABLE_PORT_ISOLATION_L2_E,
    /*PRV_CPSS_LION_TABLE_PORT_ISOLATION_L2_E*/
    {0x11880000, 0x10, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

    {CPSS_DXCH_LION_TABLE_PORT_ISOLATION_L3_E,
    /*PRV_CPSS_LION_TABLE_PORT_ISOLATION_L3_E*/
    {0x11890000, 0x10, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

    {CPSS_DXCH_LION_TABLE_TXQ_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG_E,
    /*PRV_CPSS_LION_TABLE_TXQ_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG_E*/
    {0x11002400, 0x40, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

    {CPSS_DXCH_LION_TABLE_TXQ_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG_E,
    /*PRV_CPSS_LION_TABLE_TXQ_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG_E*/
    {0x11002800, 0x8, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

    {CPSS_DXCH_LION_TABLE_TXQ_SOURCE_ID_MEMBERS_E,
    /*PRV_CPSS_LION_TABLE_TXQ_SOURCE_ID_MEMBERS_E*/
    {0x12010500, 0x8, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

    {CPSS_DXCH_LION_TABLE_TXQ_NON_TRUNK_MEMBERS_E,
    /*PRV_CPSS_LION_TABLE_TXQ_NON_TRUNK_MEMBERS_E*/
    {0x12010800, 0x8, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

    {CPSS_DXCH_LION_TABLE_TXQ_DESIGNATED_PORT_E,
    /*PRV_CPSS_LION_TABLE_TXQ_DESIGNATED_PORT_E*/
    {0x12010200, 0x8, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

    {CPSS_DXCH_LION_TABLE_TXQ_EGRESS_STC_E,
    /*PRV_CPSS_LION_TABLE_TXQ_EGRESS_STC_E*/
    {0x11004400, 0x10, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

    {CPSS_DXCH_LION_TABLE_ROUTER_VLAN_URPF_STC_E,
    /*CPSS_DXCH_LION_TABLE_ROUTER_VLAN_URPF_STC_E*/
    {0x02801400, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

    {CPSS_DXCH_LION_TABLE_TRUNK_HASH_MASK_CRC_E,
    /*PRV_CPSS_LION_TABLE_TRUNK_HASH_MASK_CRC_E*/
    {0x0b800400 , 0x10, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},
};

static const GT_U32   directAccessLion2TableInfoExtNumEntries =
    NUM_ELEMENTS_IN_ARR_MAC(directAccessLion2TableInfoExt);

/* xCat2 direct table information */
static const PRV_CPSS_DXCH_TABLES_INFO_DIRECT_STC directAccessXCAT2TableInfo[] =
{
    /*PRV_DXCH_XCAT2_TABLE_IPCL_LOOKUP01_CFG_E*/
    {0x0B820000 , 0x08, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /*PRV_DXCH_XCAT2_TABLE_IPCL_LOOKUP1_CFG_E*/
    {0x0B830000 , 0x08, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},

    /* PRV_DXCH_XCAT2_TABLE_MAC2ME_E */
    {0x0C001600, 0x10, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}
};

#define NO_DIRECT_ACCESS_FROM_CPU_TO_ADDRESS_CNS    0xFFFFFFFF

/**
* @enum PRV_DXCH_BOBCAT2_TABLES_ENT
 *
 * @brief enumeration fields of the directAccessBobcat2TableInfo table.
*/
typedef enum {
    PRV_DXCH_BOBCAT2_TABLE_EQ_STATISTICAL_RATE_LIMIT_E = 0,
    PRV_DXCH_BOBCAT2_TABLE_EQ_CPU_CODE_E,
    PRV_DXCH_BOBCAT2_TABLE_EQ_QOS_PROFILE_E,
    PRV_DXCH_BOBCAT2_TABLE_TTI_INGRESS_VLAN_TRANSLATION_E,
    PRV_DXCH_BOBCAT2_TABLE_TTI_INGRESS_PCL_UDB_CONFIG_E,
    PRV_DXCH_BOBCAT2_TABLE_TTI_VLAN_PORT_PROTOCOL_E,
    PRV_DXCH_BOBCAT2_TABLE_TTI_MAC2ME_E,
    PRV_DXCH_BOBCAT2_TABLE_L2I_VLAN_INGRESS_E,
    PRV_DXCH_BOBCAT2_TABLE_L2I_INGRESS_BRIDGE_PORT_MEMBERS_E,
    PRV_DXCH_BOBCAT2_TABLE_L2I_INGRESS_SPAN_STATE_GROUP_INDEX_E,
    PRV_DXCH_BOBCAT2_TABLE_L2I_STG_INGRESS_E,
    PRV_DXCH_BOBCAT2_TABLE_TXQ_DQ_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG_E,
    PRV_DXCH_BOBCAT2_TABLE_TXQ_DQ_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG_E,
    PRV_DXCH_BOBCAT2_TABLE_TXQ_DQ_EGRESS_STC_E,
    PRV_DXCH_BOBCAT2_TABLE_TTI_PHYSICAL_PORT_ATTRIBUTE_E,
    PRV_DXCH_BOBCAT2_TABLE_TTI_PRE_LOOKUP_INGRESS_EPORT_E,
    PRV_DXCH_BOBCAT2_TABLE_TTI_POST_LOOKUP_INGRESS_EPORT_E,
    PRV_DXCH_BOBCAT2_TABLE_L2I_BRIDGE_INGRESS_EPORT_E,
    PRV_DXCH_BOBCAT2_TABLE_L2I_BRIDGE_INGRESS_EPORT_LEARN_PRIO_E,
    PRV_DXCH_BOBCAT2_TABLE_L2I_BRIDGE_SOURCE_TRUNK_ATTRIBUTE_E,
    PRV_DXCH_BOBCAT2_TABLE_L2I_BRIDGE_PHYSICAL_PORT_E,
    PRV_DXCH_BOBCAT2_TABLE_L2I_BRIDGE_PHYSICAL_PORT_RATE_LIMIT_COUNTERS_E,
    PRV_DXCH_BOBCAT2_TABLE_EQ_INGRESS_EPORT_E,
    PRV_DXCH_BOBCAT2_TABLE_EQ_INGRESS_STC_PHYSICAL_PORT_E,
    PRV_DXCH_BOBCAT2_TABLE_EQ_INGRESS_MIRROR_PHYSICAL_PORT_E,
    PRV_DXCH_BOBCAT2_TABLE_EQ_TRUNK_LTT_E,
    PRV_DXCH_BOBCAT2_TABLE_EQ_L2_ECMP_LTT_E,
    PRV_DXCH_BOBCAT2_TABLE_EQ_L2_ECMP_E,
    PRV_DXCH_BOBCAT2_TABLE_EQ_EPORT_TO_PHYSICAL_PORT_TARGET_MAPPING_E,
    PRV_DXCH_BOBCAT2_TABLE_EQ_TO_CPU_RATE_LIMITER_CONFIG_E,
    PRV_DXCH_BOBCAT2_TABLE_EQ_TX_PROTECTION_E,
    PRV_DXCH_BOBCAT2_TABLE_EQ_EPORT_TO_LOC_MAPPING_E,
    PRV_DXCH_BOBCAT2_TABLE_EQ_PROTECTION_LOC_E,
    PRV_DXCH_BOBCAT2_TABLE_LPM_MEM_E,
    PRV_DXCH_BOBCAT2_TABLE_LPM_IPV4_VRF_E,
    PRV_DXCH_BOBCAT2_TABLE_LPM_IPV6_VRF_E,
    PRV_DXCH_BOBCAT2_TABLE_LPM_FCOE_VRF_E,
    PRV_DXCH_BOBCAT2_TABLE_LPM_ECMP_E,
    PRV_DXCH_BOBCAT2_TABLE_EGF_EFT_EGRESS_FAST_STACK_FAILOVER_SECONDARY_TARGET_PORT_E,
    PRV_DXCH_BOBCAT2_TABLE_EGF_QAG_EGRESS_EGRESS_EPORT_E,
    PRV_DXCH_BOBCAT2_TABLE_EGF_QAG_EGRESS_EGRESS_VLAN_E,
    PRV_DXCH_BOBCAT2_TABLE_EGF_QAG_EGRESS_TARGET_PORT_MAPPER_E,
    PRV_DXCH_BOBCAT2_TABLE_EGF_QAG_EGRESS_CPU_CODE_TO_LOOPBACK_MAPPER_E,
    PRV_DXCH_BOBCAT2_TABLE_EGF_QAG_EGRESS_FWD_FROM_CPU_TO_LOOPBACK_MAPPER_E,
    PRV_DXCH_BOBCAT2_TABLE_EGF_SHT_EGRESS_EGRESS_EPORT_E,
    PRV_DXCH_BOBCAT2_TABLE_EGF_SHT_EGRESS_DEVICE_MAP_TABLE_E,
    PRV_DXCH_BOBCAT2_TABLE_EGF_SHT_VLAN_EGRESS_E,
    PRV_DXCH_BOBCAT2_TABLE_EGF_SHT_EGRESS_VLAN_ATTRIBUTES_E,
    PRV_DXCH_BOBCAT2_TABLE_EGF_SHT_EGRESS_VLAN_SPANNING_E,
    PRV_DXCH_BOBCAT2_TABLE_EGF_SHT_EGRESS_VLAN_MAPPER_E,
    PRV_DXCH_BOBCAT2_TABLE_EGF_SHT_MULTICAST_E,
    PRV_DXCH_BOBCAT2_TABLE_EGF_SHT_STG_EGRESS_E,
    PRV_DXCH_BOBCAT2_TABLE_EGF_SHT_PORT_ISOLATION_L2_E,
    PRV_DXCH_BOBCAT2_TABLE_EGF_SHT_PORT_ISOLATION_L3_E,
    PRV_DXCH_BOBCAT2_TABLE_EGF_SHT_TXQ_SOURCE_ID_MEMBERS_E,
    PRV_DXCH_BOBCAT2_TABLE_EGF_SHT_TXQ_NON_TRUNK_MEMBERS_E,
    PRV_DXCH_BOBCAT2_TABLE_EGF_SHT_EGRESS_NON_TRUNK_MEMBERS2_E,
    PRV_DXCH_BOBCAT2_TABLE_EGF_SHT_EGRESS_EPORT_FILTER_E,
    PRV_DXCH_BOBCAT2_TABLE_EGF_SHT_TXQ_DESIGNATED_PORT_E,
    PRV_DXCH_BOBCAT2_TABLE_HA_EGRESS_EPORT_1_E,
    PRV_DXCH_BOBCAT2_TABLE_HA_EGRESS_EPORT_2_E,
    PRV_DXCH_BOBCAT2_TABLE_HA_PHYSICAL_PORT_1_E,
    PRV_DXCH_BOBCAT2_TABLE_HA_PHYSICAL_PORT_2_E,
    PRV_DXCH_BOBCAT2_TABLE_HA_QOS_PROFILE_TO_EXP_E,
    PRV_DXCH_BOBCAT2_TABLE_HA_EPCL_UDB_CONFIG_E,
    PRV_DXCH_BOBCAT2_TABLE_HA_PTP_DOMAIN_E,
    PRV_DXCH_BOBCAT2_TABLE_HA_GENERIC_TS_PROFILE_E,
    PRV_DXCH_BOBCAT2_TABLE_HA_GLOBAL_MAC_SA_E,
    PRV_DXCH_BOBCAT2_TABLE_HA_TUNNEL_START_CONFIG_E,
    PRV_DXCH_BOBCAT2_TABLE_HA_EGRESS_VLAN_TRANSLATION_E,
    PRV_DXCH_BOBCAT2_TABLE_HA_ROUTE_MAC_SA_E,
    PRV_DXCH_BOBCAT2_TABLE_MLL_IP_E,
    PRV_DXCH_BOBCAT2_TABLE_MLL_L2_E,
    PRV_DXCH_BOBCAT2_TABLE_MLL_L2_LTT_E,
    PRV_DXCH_BOBCAT2_TABLE_PCL_CONFIG_E,
    PRV_DXCH_BOBCAT2_TABLE_PCL_INGRESS_LOOKUP01_CONFIG_E,
    PRV_DXCH_BOBCAT2_TABLE_PCL_INGRESS_LOOKUP1_CONFIG_E,
    PRV_DXCH_BOBCAT2_TABLE_PCL_TRUNK_HASH_MASK_CRC_E,
    PRV_DXCH_BOBCAT2_TABLE_PCL_IPCL0_UDB_SELECT_E,
    PRV_DXCH_BOBCAT2_TABLE_PCL_IPCL1_UDB_SELECT_E,
    PRV_DXCH_BOBCAT2_TABLE_PCL_IPCL2_UDB_SELECT_E,
    PRV_DXCH_BOBCAT2_TABLE_TCAM_PCL_TTI_ACTION_E,
    PRV_DXCH_BOBCAT2_TABLE_ERMRK_TIMESTAMP_CFG_E,
    PRV_DXCH_BOBCAT2_TABLE_ERMRK_PTP_LOCAL_ACTION_E,
    PRV_DXCH_BOBCAT2_TABLE_ERMRK_PTP_TARGET_PORT_E,
    PRV_DXCH_BOBCAT2_TABLE_ERMRK_PTP_SOURCE_PORT_E,
    PRV_DXCH_BOBCAT2_TABLE_ERMRK_QOS_DSCP_MAP_E,
    PRV_DXCH_BOBCAT2_TABLE_ERMRK_QOS_TC_DP_MAP_E,
    PRV_DXCH_BOBCAT2_TABLE_TCAM_E,
    PRV_DXCH_BOBCAT2_TABLE_TTI_ADJACENCY_E,
    PRV_DXCH_BOBCAT2_TABLE_IPLR_INGRESS_POLICER_0_E_ATTRIBUTES_E,
    PRV_DXCH_BOBCAT2_TABLE_IPLR_INGRESS_POLICER_0_REMARKING_E,
    PRV_DXCH_BOBCAT2_TABLE_IPLR_INGRESS_POLICER_0_HIERARCHICAL_E,
    PRV_DXCH_BOBCAT2_TABLE_IPLR_INGRESS_POLICER_0_METERING_E,
    PRV_DXCH_BOBCAT2_TABLE_IPLR_1_INGRESS_POLICER_1_E_ATTRIBUTES_E,
    PRV_DXCH_BOBCAT2_TABLE_IPLR_1_INGRESS_POLICER_1_REMARKING_E,
    PRV_DXCH_BOBCAT2_TABLE_IPLR_1_INGRESS_POLICER_1_METERING_E,
    PRV_DXCH_BOBCAT2_TABLE_EPLR_EGRESS_POLICER_E_ATTRIBUTES_E,
    PRV_DXCH_BOBCAT2_TABLE_EPLR_EGRESS_POLICER_REMARKING_E,
    PRV_DXCH_BOBCAT2_TABLE_EPLR_EGRESS_POLICER_METERING_E,
    PRV_DXCH_BOBCAT2_TABLE_IPVX_INGRESS_EPORT_E,
    PRV_DXCH_BOBCAT2_TABLE_IPVX_EVLAN_E,
    PRV_DXCH_BOBCAT2_TABLE_IPVX_ROUTER_NEXTHOP_E,
    PRV_DXCH_BOBCAT2_TABLE_IPVX_ROUTER_NEXTHOP_AGE_BITS_E,
    PRV_DXCH_BOBCAT2_TABLE_IPVX_ROUTER_ACCESS_MATRIX_E,
    PRV_DXCH_BOBCAT2_TABLE_IPVX_ROUTER_QOS_PROFILE_OFFSETS_E,
    PRV_DXCH_BOBCAT2_TABLE_IOAM_INGRESS_OAM_E,
    PRV_DXCH_BOBCAT2_TABLE_IOAM_INGRESS_OAM_KEEPALIVE_AGING_E,
    PRV_DXCH_BOBCAT2_TABLE_IOAM_INGRESS_OAM_MEG_EXCEPTION_E,
    PRV_DXCH_BOBCAT2_TABLE_IOAM_INGRESS_OAM_SOURCE_INTERFACE_EXCEPTION_E,
    PRV_DXCH_BOBCAT2_TABLE_IOAM_INGRESS_OAM_INVALID_KEEPALIVE_HASH_E,
    PRV_DXCH_BOBCAT2_TABLE_IOAM_INGRESS_OAM_KEEPALIVE_EXCESS_E,
    PRV_DXCH_BOBCAT2_TABLE_IOAM_INGRESS_OAM_EXCEPTION_SUMMARY_E,
    PRV_DXCH_BOBCAT2_TABLE_IOAM_INGRESS_OAM_RDI_STATUS_CHANGE_EXCEPTION_E,
    PRV_DXCH_BOBCAT2_TABLE_IOAM_INGRESS_OAM_TX_PERIOD_EXCEPTION_E,
    PRV_DXCH_BOBCAT2_TABLE_IOAM_INGRESS_OAM_OPCODE_PACKET_COMMAND_E,
    PRV_DXCH_BOBCAT2_TABLE_EOAM_EGRESS_OAM_E,
    PRV_DXCH_BOBCAT2_TABLE_EOAM_EGRESS_OAM_KEEPALIVE_AGING_E,
    PRV_DXCH_BOBCAT2_TABLE_EOAM_EGRESS_OAM_MEG_EXCEPTION_E,
    PRV_DXCH_BOBCAT2_TABLE_EOAM_EGRESS_OAM_SOURCE_INTERFACE_EXCEPTION_E,
    PRV_DXCH_BOBCAT2_TABLE_EOAM_EGRESS_OAM_INVALID_KEEPALIVE_HASH_E,
    PRV_DXCH_BOBCAT2_TABLE_EOAM_EGRESS_OAM_KEEPALIVE_EXCESS_E,
    PRV_DXCH_BOBCAT2_TABLE_EOAM_EGRESS_OAM_EXCEPTION_SUMMARY_E,
    PRV_DXCH_BOBCAT2_TABLE_EOAM_EGRESS_OAM_RDI_STATUS_CHANGE_EXCEPTION_E,
    PRV_DXCH_BOBCAT2_TABLE_EOAM_EGRESS_OAM_TX_PERIOD_EXCEPTION_E,
    PRV_DXCH_BOBCAT2_TABLE_EOAM_EGRESS_OAM_OPCODE_PACKET_COMMAND_E,
    PRV_DXCH_BOBCAT2_TABLE_ERMRK_OAM_LM_OFFSET_E,
    PRV_DXCH_BOBCAT2_TABLE_TTI_QOS_MAP_DSCP_TO_QOS_PROFILE_E,
    PRV_DXCH_BOBCAT2_TABLE_TTI_QOS_MAP_UP_CFI_TO_QOS_PROFILE_E,
    PRV_DXCH_BOBCAT2_TABLE_TTI_QOS_MAP_EXP_TO_QOS_PROFILE_E,
    PRV_DXCH_BOBCAT2_TABLE_TTI_QOS_MAP_DSCP_TO_DSCP_E,
    PRV_DXCH_BOBCAT2_TABLE_TTI_UDB_CONFIG_E,
    PRV_DXCH_BOBCAT2_TABLE_TTI_PTP_COMMAND_E,
    PRV_DXCH_BOBCAT2_TABLE_EPCL_EGRESS_PCL_CONFIG_E,
    PRV_DXCH_BOBCAT2_TABLE_EPCL_UDB_SELECT_E,
    PRV_DXCH_BOBCAT2_TABLE_BMA_PORT_MAPPING_E,
    PRV_DXCH_BOBCAT2_TABLE_BMA_MULTICAST_COUNTERS_E,
    PRV_DXCH_BOBCAT2_TABLE_TXQ_QUEUE_TAIL_DROP_MAX_QUEUE_LIMITS_E,
    PRV_DXCH_BOBCAT2_TABLE_TXQ_QUEUE_TAIL_DROP_EQ_QUEUE_LIMITS_DP0_E,
    PRV_DXCH_BOBCAT2_TABLE_TXQ_QUEUE_TAIL_DROP_DQ_QUEUE_BUF_LIMITS_E,
    PRV_DXCH_BOBCAT2_TABLE_TXQ_QUEUE_TAIL_DROP_DQ_QUEUE_DESC_LIMITS_E,
    PRV_DXCH_BOBCAT2_TABLE_TXQ_QUEUE_TAIL_DROP_EQ_QUEUE_LIMITS_DP1_E,
    PRV_DXCH_BOBCAT2_TABLE_TXQ_QUEUE_TAIL_DROP_EQ_QUEUE_LIMITS_DP2_E,
    PRV_DXCH_BOBCAT2_TABLE_TXQ_QUEUE_TAIL_DROP_COUNTERS_Q_MAIN_BUFF_E,
    PRV_DXCH_BOBCAT2_TABLE_TXQ_QUEUE_TAIL_DROP_COUNTERS_Q_MAIN_MC_BUFF_E,
    PRV_DXCH_BOBCAT2_TABLE_TXQ_LL_TXQ_LINK_LIST_COUNTERS_Q_MAIN_DESC_E,
    PRV_DXCH_BOBCAT2_TABLE_TXQ_PFC_MODE_PROFILE_TC_XOFF_THRESHOLDS_E,
    PRV_DXCH_BOBCAT2_TABLE_TXQ_PFC_MODE_PROFILE_TC_XON_THRESHOLDS_E,
    PRV_DXCH_BOBCAT2_TABLE_TXQ_PFC_LLFC_COUNTERS_E,
    PRV_DXCH_BOBCAT2_TABLE_TM_DROP_DROP_MASKING_E,
    PRV_DXCH_BOBCAT2_TABLE_TM_DROP_QUEUE_PROFILE_ID_E,
    PRV_DXCH_BOBCAT2_TABLE_TM_QMAP_CPU_CODE_TO_TC_E,
    PRV_DXCH_BOBCAT2_TABLE_TM_QMAP_TARGET_DEV_TO_INDEX_E,
    PRV_DXCH_BOBCAT2_TABLE_TM_QMAP_POLICING_ENABLE_E,
    PRV_DXCH_BOBCAT2_TABLE_TM_QMAP_QUEUE_ID_SELECTOR_E,
    PRV_DXCH_BOBCAT2_TABLE_TM_INGRESS_GLUE_L1_PKT_LEN_OFFSET_E,
    PRV_DXCH_BOBCAT2_TABLE_TM_EGRESS_GLUE_AGING_QUEUE_PROFILE_E,
    PRV_DXCH_BOBCAT2_TABLE_TM_EGRESS_GLUE_AGING_PROFILE_THESHOLDS_E,
    PRV_DXCH_BOBCAT2_TABLE_TM_EGRESS_GLUE_TARGET_INTERFACE_E,
    PRV_DXCH_BOBCAT2_TABLE_TM_FCU_ETH_DMA_TO_TM_PORT_MAPPING_E,
    PRV_DXCH_BOBCAT2_TABLE_TM_FCU_ILK_DMA_TO_TM_PORT_MAPPING_E,
    PRV_DXCH_BOBCAT2_TABLE_TM_FCU_TC_PORT_TO_CNODE_PORT_MAPPING_E,
    PRV_DXCH_BOBCAT2_TABLE_TM_FCU_PORT_INGRESS_TIMERS_CONFIG_E,
    PRV_DXCH_BOBCAT2_TABLE_TM_FCU_INGRESS_TIMERS_E,
    PRV_DXCH_BOBCAT2_TABLE_PORT_TO_PHYSICAL_PORT_MAPPING_E,
    PRV_DXCH_BOBCAT2_TABLE_CN_SAMPLE_INTERVALS_E,

    PRV_CPSS_BOBCAT2_TABLE_INGRESS_POLICER_0_COUNTING_E,
    PRV_CPSS_BOBCAT2_TABLE_INGRESS_POLICER_1_COUNTING_E,
    PRV_CPSS_BOBCAT2_TABLE_EGRESS_POLICER_COUNTING_E,

    PRV_CPSS_BOBCAT2_TABLE_INGRESS_POLICER_0_IPFIX_ALERT_WRAPAROUND_E,
    PRV_CPSS_BOBCAT2_TABLE_INGRESS_POLICER_1_IPFIX_ALERT_WRAPAROUND_E,
    PRV_CPSS_BOBCAT2_TABLE_EGRESS_POLICER_IPFIX_ALERT_WRAPAROUND_E,

    PRV_CPSS_BOBCAT2_TABLE_INGRESS_POLICER_0_IPFIX_ALERT_AGING_E,
    PRV_CPSS_BOBCAT2_TABLE_INGRESS_POLICER_1_IPFIX_ALERT_AGING_E,
    PRV_CPSS_BOBCAT2_TABLE_EGRESS_POLICER_IPFIX_ALERT_AGING_E,

    /* CNC counters for blocks 0..31 */
    PRV_CPSS_BOBCAT2_TABLE_CNC_0_COUNTERS_E ,
    PRV_CPSS_BOBCAT2_TABLE_CNC_31_COUNTERS_E = PRV_CPSS_BOBCAT2_TABLE_CNC_0_COUNTERS_E + 31,


    PRV_DXCH_BOBCAT2_TABLE___LAST_E, /* last bobcat 2 direct tables */
    /* add here additional direct access tables for bobk */

    PRV_DXCH_BOBK_TABLE_PLR_INGRESS_POLICER_0_METERING_CONFIG_E = PRV_DXCH_BOBCAT2_TABLE___LAST_E,
    PRV_DXCH_BOBK_TABLE_PLR_INGRESS_POLICER_1_METERING_CONFIG_E,
    PRV_DXCH_BOBK_TABLE_PLR_EGRESS_POLICER_METERING_CONFIG_E,

    PRV_DXCH_BOBK_TABLE_PLR_INGRESS_POLICER_0_METERING_CONFORM_SIGN_E,
    PRV_DXCH_BOBK_TABLE_PLR_INGRESS_POLICER_1_METERING_CONFORM_SIGN_E,
    PRV_DXCH_BOBK_TABLE_PLR_EGRESS_POLICER_METERING_CONFORM_SIGN_E,

    PRV_DXCH_BOBK_TABLE___LAST_E, /* last bobk direct tables */

    /* add here additional direct access tables for bobcat3 */
    PRV_CPSS_BOBCAT3_TABLE_EGF_QAG_PORT_TARGET_ATTRIBUTES_E = PRV_DXCH_BOBK_TABLE___LAST_E,
    PRV_CPSS_BOBCAT3_TABLE_TTI_PHYSICAL_PORT_ATTRIBUTE_2_E,
    PRV_CPSS_BOBCAT3_TABLE_TTI_QCN_TO_TIMER_PAUSE_MAP_E,
    PRV_CPSS_BOBCAT3_TABLE_EQ_TX_PROTECTION_LOC_E,
    PRV_CPSS_BOBCAT3_TABLE_EGF_QAG_TC_DP_MAPPER_E,
    PRV_CPSS_BOBCAT3_TABLE_EGF_QAG_PORT_SOURCE_ATTRIBUTES_E,

    /* MULTI-instance support */
    PRV_CPSS_BOBCAT3_MULTI_INSTANCE_TABLE_TXQ_EGRESS_STC__DQ_1_E,        /* DQ[1] */
    PRV_CPSS_BOBCAT3_MULTI_INSTANCE_TABLE_TXQ_EGRESS_STC__DQ_LAST_E =    /* DQ[5] */
        PRV_CPSS_BOBCAT3_MULTI_INSTANCE_TABLE_TXQ_EGRESS_STC__DQ_1_E + 4,

    PRV_CPSS_BOBCAT3_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG__DQ_1_E,         /* DQ[1] */
    PRV_CPSS_BOBCAT3_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG__DQ_LAST_E =     /* DQ[5] */
        PRV_CPSS_BOBCAT3_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG__DQ_1_E+4,

    PRV_CPSS_BOBCAT3_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG__DQ_1_E,      /* DQ[1] */
    PRV_CPSS_BOBCAT3_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG__DQ_LAST_E=   /* DQ[5] */
        PRV_CPSS_BOBCAT3_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG__DQ_1_E+4,

    PRV_CPSS_BOBCAT3_MULTI_INSTANCE_TABLE_EQ_INGRESS_STC_PHYSICAL_PORT_PIPE_1_E , /*pipe [1]*/
    PRV_CPSS_BOBCAT3_LPM_MEM_PIPE_1_E , /* LPM for pipe 1 */

    PRV_CPSS_BOBCAT3_MULTI_INSTANCE_TABLE_TTI_PORT_MAPPING__DQ_0_E,     /* DQ[0] */
    PRV_CPSS_BOBCAT3_MULTI_INSTANCE_TABLE_TTI_PORT_MAPPING__DQ_1_E,     /* DQ[1] */
    PRV_CPSS_BOBCAT3_MULTI_INSTANCE_TABLE_TTI_PORT_MAPPING__DQ_LAST_E = /* DQ[5] */
        PRV_CPSS_BOBCAT3_MULTI_INSTANCE_TABLE_TTI_PORT_MAPPING__DQ_0_E + 5,

    PRV_DXCH_BOBCAT3_TABLE___LAST_E, /* last BC3 direct tables */

    PRV_DXCH_ALDRIN2_TABLE_IPVX_ROUTER_ECMP_POINTER_E,
    PRV_DXCH_ALDRIN2_TABLE_TAIL_DROP_PORT_BUFFER_FILL_LEVEL_E,
    PRV_DXCH_ALDRIN2_TABLE_TAIL_DROP_QUEUE_BUFFER_FILL_LEVEL_E,

    PRV_DXCH_ALDRIN2_TABLE___LAST_E, /* last ALDRIN2 direct tables */

    PRV_DXCH_SIP6_TABLE_INGRESS_POLICER_0_QOS_ATTRIBUTE_TABLE_E,
    PRV_DXCH_SIP6_TABLE_INGRESS_POLICER_1_QOS_ATTRIBUTE_TABLE_E,
    PRV_DXCH_SIP6_TABLE_EGRESS_POLICER_QOS_ATTRIBUTE_TABLE_E,

    PRV_DXCH_SIP6_TABLE_INGRESS_POLICER_0_PORT_ATTRIBUTE_TABLE_E,
    PRV_DXCH_SIP6_TABLE_INGRESS_POLICER_1_PORT_ATTRIBUTE_TABLE_E,
    PRV_DXCH_SIP6_TABLE_EGRESS_POLICER_PORT_ATTRIBUTE_TABLE_E,

    PRV_DXCH_SIP6_TABLE_INGRESS_PCL_HASH_MODE_CRC_TABLE_E,

    PRV_DXCH_SIP6_TABLE_EGRESS_PCL_EXACT_MATCH_PROFILE_ID_MAPPING_E,

    /*TXQ -PDX*/

    PRV_DXCH_SIP6_TABLE_TXQ_PDX_DX_QGRPMAP_E,

    /*TXQ -PDS*/

    PRV_DXCH_SIP6_TABLE_TXQ_PDS_PER_QUEUE_COUNTERS_E,
    PRV_DXCH_SIP6_TABLE_TXQ_PDS_PER_QUEUE_COUNTERS_LAST_E = PRV_DXCH_SIP6_TABLE_TXQ_PDS_PER_QUEUE_COUNTERS_E+CPSS_DXCH_SIP6_MAX_PDS_UNITS_PER_DEVICE_MAC-1
     /*debug*/
    ,PRV_CPSS_DXCH_TXQ_MULTIPLE_TABLE(PRV_DXCH_SIP6_TABLE_TXQ_PDS_DATA_STORAGE,CPSS_DXCH_SIP6_MAX_PDS_UNITS_PER_DEVICE_MAC)
    ,PRV_CPSS_DXCH_TXQ_MULTIPLE_TABLE(PRV_DXCH_SIP6_TABLE_TXQ_PDS_NXT_TABLE,CPSS_DXCH_SIP6_MAX_PDS_UNITS_PER_DEVICE_MAC)
    ,PRV_CPSS_DXCH_TXQ_MULTIPLE_TABLE(PRV_DXCH_SIP6_TABLE_TXQ_PDS_PID_TABLE,CPSS_DXCH_SIP6_MAX_PDS_UNITS_PER_DEVICE_MAC)
    ,PRV_CPSS_DXCH_TXQ_MULTIPLE_TABLE(PRV_DXCH_SIP6_TABLE_TXQ_PDS_HEAD_HEAD,CPSS_DXCH_SIP6_MAX_PDS_UNITS_PER_DEVICE_MAC)
    ,PRV_CPSS_DXCH_TXQ_MULTIPLE_TABLE(PRV_DXCH_SIP6_TABLE_TXQ_PDS_HEAD_TAIL,CPSS_DXCH_SIP6_MAX_PDS_UNITS_PER_DEVICE_MAC)
    ,PRV_CPSS_DXCH_TXQ_MULTIPLE_TABLE(PRV_DXCH_SIP6_TABLE_TXQ_PDS_TAIL_TAIL,CPSS_DXCH_SIP6_MAX_PDS_UNITS_PER_DEVICE_MAC)
    ,PRV_CPSS_DXCH_TXQ_MULTIPLE_TABLE(PRV_DXCH_SIP6_TABLE_TXQ_PDS_TAIL_HEAD,CPSS_DXCH_SIP6_MAX_PDS_UNITS_PER_DEVICE_MAC)
    ,PRV_CPSS_DXCH_TXQ_MULTIPLE_TABLE(PRV_DXCH_SIP6_TABLE_TXQ_PDS_FRAG_HEAD,CPSS_DXCH_SIP6_MAX_PDS_UNITS_PER_DEVICE_MAC)
    ,PRV_CPSS_DXCH_TXQ_MULTIPLE_TABLE(PRV_DXCH_SIP6_TABLE_TXQ_PDS_FRAG_TAIL,CPSS_DXCH_SIP6_MAX_PDS_UNITS_PER_DEVICE_MAC)

    /*TXQ -SDQ*/

    ,PRV_CPSS_DXCH_TXQ_MULTIPLE_TABLE(PRV_DXCH_SIP6_TABLE_TXQ_SDQ_QUEUE_CFG,CPSS_DXCH_SIP6_MAX_SDQ_UNITS_PER_DEVICE_MAC)
    ,PRV_CPSS_DXCH_TXQ_MULTIPLE_TABLE(PRV_DXCH_SIP6_TABLE_TXQ_SDQ_QUEUE_CRDT_BLNC,CPSS_DXCH_SIP6_MAX_SDQ_UNITS_PER_DEVICE_MAC)
    ,PRV_CPSS_DXCH_TXQ_MULTIPLE_TABLE(PRV_DXCH_SIP6_TABLE_TXQ_SDQ_QUEUE_ELIG_STATE,CPSS_DXCH_SIP6_MAX_SDQ_UNITS_PER_DEVICE_MAC)
    ,PRV_CPSS_DXCH_TXQ_MULTIPLE_TABLE(PRV_DXCH_SIP6_TABLE_TXQ_SDQ_PORT_CRDT_BLNC,CPSS_DXCH_SIP6_MAX_SDQ_UNITS_PER_DEVICE_MAC)
    ,PRV_CPSS_DXCH_TXQ_MULTIPLE_TABLE(PRV_DXCH_SIP6_TABLE_TXQ_SDQ_SEL_LIST_PTRS,CPSS_DXCH_SIP6_MAX_SDQ_UNITS_PER_DEVICE_MAC)

     /*Txq -PFCC*/
    ,PRV_CPSS_DXCH_TXQ_MULTIPLE_TABLE(PRV_DXCH_SIP6_TABLE_TXQ_PFCC_PFCC_CFG,CPSS_DXCH_SIP6_MAX_PFCC_UNITS_PER_DEVICE_MAC),

    /* TTI */
    PRV_DXCH_SIP6_TABLE_TTI_PORT_TO_QUEUE_TRANSLATION_TABLE_E,
    PRV_CPSS_DXCH_SIP6_TABLE_TTI_VLAN_TO_VRF_ID_TABLE_E,

    /* PREQ */
    PRV_DXCH_SIP6_TABLE_PREQ_CNC_PORT_MAPPING_E,
    PRV_DXCH_SIP6_TABLE_PREQ_QUEUE_DP_CONFIGURATIONS_E,
    PRV_DXCH_SIP6_TABLE_PREQ_QUEUE_CONFIGURATIONS_E,
    PRV_DXCH_SIP6_TABLE_PREQ_PORT_PROFILE_E,
    PRV_DXCH_SIP6_TABLE_PREQ_TARGET_PHYSICAL_PORT_E,

    /* EQ */
    PRV_DXCH_SIP6_TABLE_EQ_L2ECMP_PATH_UTILIZATION_TABLE0_E,
    PRV_DXCH_SIP6_TABLE_EQ_L2ECMP_PATH_UTILIZATION_TABLE1_E,
    PRV_DXCH_SIP6_TABLE_EQ_L2ECMP_PATH_UTILIZATION_TABLE2_E,
    PRV_DXCH_SIP6_TABLE_EQ_L2ECMP_PATH_UTILIZATION_TABLE3_E,

    /* IPVX */
    PRV_DXCH_SIP6_TABLE_IPVX_ECMP_E,

    /* LPM */
    PRV_DXCH_SIP6_TABLE_PBR_E,
    PRV_DXCH_SIP6_TABLE_LPM_AGING_E,

    /* LMU */
    PRV_DXCH_SIP6_TABLE_LMU_STATISTICS_TABLE_E,
    PRV_DXCH_SIP6_TABLE_LMU_STATISTICS_TABLE_LAST_E = PRV_DXCH_SIP6_TABLE_LMU_STATISTICS_TABLE_E + CPSS_DXCH_SIP6_MAX_GOP_LMU_STATISTICS_MAC - 1,
    PRV_DXCH_SIP6_TABLE_LMU_CONFIGURATION_TABLE_E,
    PRV_DXCH_SIP6_TABLE_LMU_CONFIGURATION_TABLE_LAST_E = PRV_DXCH_SIP6_TABLE_LMU_CONFIGURATION_TABLE_E + CPSS_DXCH_SIP6_MAX_GOP_LMU_CONFIGURATION_MAC - 1,

    /* EPCL */
    PRV_DXCH_SIP6_TABLE_EGRESS_PCL_PORT_LATENCY_MONITORING_E,
    PRV_DXCH_SIP6_TABLE_EGRESS_PCL_SOURCE_PHYSICAL_PORT_MAPPING_E,
    PRV_DXCH_SIP6_TABLE_EGRESS_PCL_TARGET_PHYSICAL_PORT_MAPPING_E,

    /* PHA */
    PRV_DXCH_SIP6_TABLE_PHA_PPA_THREADS_CONF_1_E,
    PRV_DXCH_SIP6_TABLE_PHA_PPA_THREADS_CONF_2_E,
    PRV_DXCH_SIP6_TABLE_PHA_SOURCE_PHYSICAL_PORT_E,
    PRV_DXCH_SIP6_TABLE_PHA_TARGET_PHYSICAL_PORT_E,
    PRV_DXCH_SIP6_TABLE_PHA_FW_IMAGE_E,
    PRV_DXCH_SIP6_TABLE_PHA_SHARED_DMEM_E,

    /* EXACT MATCH */
    PRV_DXCH_SIP6_TABLE_EXACT_MATCH_E,

    PRV_DXCH_SIP6_TABLE_MTIP_MAC_STAT_E,

    PRV_DXCH_FALCON_TABLE___LAST_E, /* last FALCON direct tables */

    /* SIP 6.10 only - PPU */
    PRV_DXCH_SIP6_10_TABLE_PPU_ACTION_TABLE_0_E,
    PRV_DXCH_SIP6_10_TABLE_PPU_ACTION_TABLE_1_E,
    PRV_DXCH_SIP6_10_TABLE_PPU_ACTION_TABLE_2_E,
    PRV_DXCH_SIP6_10_TABLE_PPU_DAU_PROFILE_TABLE_E,

    PRV_DXCH_SIP6_10_TABLE_EXACT_MATCH_AUTO_LEARNED_ENTRY_INDEX_E,

    /* SIP6_10 EPCL */
    PRV_DXCH_SIP6_10_TABLE_EGRESS_PCL_QUEUE_GROUP_LATENCY_PROFILE_CONFIG_E,

    /* SIP6_10 IPFIX FIRST N PACKETS */
    PRV_DXCH_SIP6_10_TABLE_INGRESS_POLICER_0_IPFIX_FIRST_N_PACKETS_E,
    PRV_DXCH_SIP6_10_TABLE_INGRESS_POLICER_1_IPFIX_FIRST_N_PACKETS_E,
    PRV_DXCH_SIP6_10_TABLE_EGRESS_POLICER_IPFIX_FIRST_N_PACKETS_E,

    /* SIP6_10 */
    PRV_DXCH_SIP6_10_TABLE_SOURCE_PORT_HASH_ENTRY_E,
    PRV_DXCH_SIP6_10_TABLE_IPCL0_SOURCE_PORT_CONFIG_E,
    PRV_DXCH_SIP6_10_TABLE_IPCL1_SOURCE_PORT_CONFIG_E,
    PRV_DXCH_SIP6_10_TABLE_IPCL2_SOURCE_PORT_CONFIG_E,

    /* CNC counters for blocks 32..63 */
    PRV_DXCH_SIP6_10_TABLE_CNC_32_COUNTERS_E ,
    PRV_DXCH_SIP6_10_TABLE_CNC_63_COUNTERS_E = PRV_DXCH_SIP6_10_TABLE_CNC_32_COUNTERS_E + 31,

    PRV_DXCH_SIP6_10_TABLE___LAST_E, /* Hawk */

    PRV_DXCH_SIP6_15_TABLE_TXQ_SDQ_PORT_PFC_STATE_E,
    PRV_DXCH_SIP6_15_TABLE___LAST_E, /* Phoenix */

    PRV_DXCH_SIP6_20_EGF_QAG_PORT_VLAN_Q_OFFSET_E,
    PRV_DXCH_SIP6_20_TABLE___LAST_E, /* Harrier */

    PRV_DXCH_SIP6_30_TABLE_IPCL0_UDB_REPLACEMENT_E,
    PRV_DXCH_SIP6_30_TABLE_IPCL1_UDB_REPLACEMENT_E,
    PRV_DXCH_SIP6_30_TABLE_IPCL2_UDB_REPLACEMENT_E,

    PRV_DXCH_SIP6_30_TABLE_SMU_IRF_SNG_E,
    PRV_DXCH_SIP6_30_TABLE_SMU_IRF_COUNTING_E,

    PRV_DXCH_SIP6_30_TABLE_TXQ_PDS_NEXT_DESC,
    PRV_DXCH_SIP6_30_TABLE_TXQ_PDS_WRITE_POINTER,
    PRV_DXCH_SIP6_30_TABLE_TXQ_PDS_READ_POINTER,

    PRV_DXCH_SIP6_30_TABLE_TXQ_SDQ_QBV_CFG,

    PRV_DXCH_SIP6_30_TABLE_PREQ_SRF_MAPPING_E,
    PRV_DXCH_SIP6_30_TABLE_PREQ_SRF_CONFIG_E,
    PRV_DXCH_SIP6_30_TABLE_PREQ_SRF_COUNTERS_E,
    PRV_DXCH_SIP6_30_TABLE_PREQ_SRF_DAEMON_E,
    PRV_DXCH_SIP6_30_TABLE_PREQ_SRF_HISTORY_BUFFER_E,
    PRV_DXCH_SIP6_30_TABLE_PREQ_SRF_ZERO_BIT_VECTOR_0_E,
    PRV_DXCH_SIP6_30_TABLE_PREQ_SRF_ZERO_BIT_VECTOR_1_E,

    PRV_DXCH_SIP6_30_TABLE___LAST_E, /* Ironman */

    /* HW tables that are NOT under CPSS control */
    PRV_DXCH_INTERNAL_TABLE_PACKET_DATA_PARITY_E,
    PRV_DXCH_INTERNAL_TABLE_PACKET_DATA_ECC_E,
    PRV_DXCH_INTERNAL_TABLE_DESCRIPTORS_CONTROL_E,
    PRV_DXCH_INTERNAL_TABLE_PACKET_DATA_CONTROL_PARITY_E,
    PRV_DXCH_INTERNAL_TABLE_PACKET_DATA_CONTROL_ECC_E,
    PRV_DXCH_INTERNAL_TABLE_PHA_RAM_E,
    PRV_DXCH_INTERNAL_TABLE_CM3_RAM_E,
    PRV_DXCH_INTERNAL_TABLE_MANAGEMENT_E,

    /* !!!must be last !!!*/
    PRV_DXCH_DEVICE_TABLE___LAST_E /* last 'any' device direct tables */

}PRV_DXCH_BOBCAT2_TABLES_ENT;

/* support index 0..31 to represent 16 blocks from CNC0 and CNC1*/
#define    PRV_CPSS_DXCH_SIP5_TABLE_CNC_COUNTERS_MAC(index) \
    /*PRV_CPSS_BOBCAT2_TABLE_CNC_0_COUNTERS_E + index */    \
    {CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + (index), PRV_CPSS_DXCH_UNIT_CNC_0_E + ((index)/16), \
    {0x2e000000 + 0x01000000 * ((index)/16)  + 0x10000 + 0x2000*(index%16),                          \
        BITS_TO_BYTES_ALIGNMENT_MAC(64), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE}

/* support index 0..31 to represent 16 blocks from CNC2 and CNC3*/
#define    PRV_CPSS_DXCH_SIP6_10_TABLE_CNC_COUNTERS_MAC(index)                                             \
    ,{PRV_DXCH_SIP6_10_TABLE_CNC_32_COUNTERS_E + index,                                                    \
    {CPSS_DXCH_SIP6_10_TABLE_CNC_32_COUNTERS_E + (index), PRV_CPSS_DXCH_UNIT_CNC_2_E + ((index)/16),       \
    {0x10000 + 0x2000*(index%16),                                                                          \
        BITS_TO_BYTES_ALIGNMENT_MAC(64), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE}}


/* Bobcat2 direct table information */
/* NOTE: index to this table is from PRV_DXCH_BOBCAT2_TABLES_ENT */
static const PRV_CPSS_DXCH_TABLES_INFO_DIRECT_UNIT_STC directAccessBobcat2TableInfo[] =
{
    {CPSS_DXCH_TABLE_STATISTICAL_RATE_LIMIT_E, PRV_CPSS_DXCH_UNIT_EQ_E,
    {0x0D000000 + 0x00100000, 4 , PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_TABLE_CPU_CODE_E, PRV_CPSS_DXCH_UNIT_EQ_E,
    {0x0D000000 + 0x00120000, 4 , PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_TABLE_QOS_PROFILE_E, PRV_CPSS_DXCH_UNIT_EQ_E,
    {0x0D000000 + 0x00110000, 4 , PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH3_TABLE_INGRESS_VLAN_TRANSLATION_E, PRV_CPSS_DXCH_UNIT_TTI_E,
    /* PRV_DXCH_XCAT_TABLE_INGRESS_TRANSLATION_E */
    {0x01000000 + 0x00014000, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_XCAT_TABLE_INGRESS_PCL_UDB_CONFIG_E, PRV_CPSS_DXCH_UNIT_TTI_E,
    /* PRV_DXCH_XCAT_TABLE_IPCL_UDB_CFG_E */
    {0x01000000 + 0x00020000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_TTI_PCL_UDB_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_TABLE_VLAN_PORT_PROTOCOL_E, PRV_CPSS_DXCH_UNIT_TTI_E,
    /* PRV_DXCH_XCAT_TABLE_VLAN_PORT_PROTOCOL_E */
    {0x01000000 + 0x00008000, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH3_TABLE_MAC2ME_E, PRV_CPSS_DXCH_UNIT_TTI_E,
    /* PRV_CPSS_XCAT_TABLE_MAC2ME_E */
    {0x01000000 + 0x00001600, 0x20, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E, PRV_CPSS_DXCH_UNIT_L2I_E,
    {0x03000000 + 0x002C0000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_L2I_INGRESS_VLAN_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_INGRESS_BRIDGE_PORT_MEMBERS_E, PRV_CPSS_DXCH_UNIT_L2I_E,
    {0x03000000 + 0x00280000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_L2I_INGRESS_BRIDGE_PORT_MEMBERS_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_INGRESS_SPAN_STATE_GROUP_INDEX_E, PRV_CPSS_DXCH_UNIT_L2I_E,
    {0x03000000 + 0x002E0000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_L2I_INGRESS_SPAN_STATE_GROUP_INDEX_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_LION_TABLE_STG_INGRESS_E, PRV_CPSS_DXCH_UNIT_L2I_E,
    /* PRV_DXCH_LION_TABLE_STG_INGRESS_E             */
    {0x03000000 + 0x00240000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_L2I_INGRESS_STP_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_LION_TABLE_TXQ_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG_E, PRV_CPSS_DXCH_UNIT_TXQ_DQ_E,
    /*PRV_CPSS_LION_TABLE_TXQ_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG_E*/
    {0x40000000 + 0x00006000 , BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_PRIO_TOKEN_BUCKET_CONFIGURATION_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_LION_TABLE_TXQ_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG_E, PRV_CPSS_DXCH_UNIT_TXQ_DQ_E,
    /*PRV_CPSS_LION_TABLE_TXQ_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG_E*/
    {0x40000000 + 0x0000A000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_PORT_TOKEN_BUCKET_CONFIGURATION_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_LION_TABLE_TXQ_EGRESS_STC_E, PRV_CPSS_DXCH_UNIT_TXQ_DQ_E,
    /*PRV_CPSS_LION_TABLE_TXQ_EGRESS_STC_E*/
    {0x40000000 + 0x0000D800, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_EGRESS_STC_TABLE_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},
    /* Bobcat2, Caelum, Bobcat3 start new tables */
    /* TTI tables */
    {CPSS_DXCH_SIP5_TABLE_TTI_PHYSICAL_PORT_ATTRIBUTE_E, PRV_CPSS_DXCH_UNIT_TTI_E,
    {0x01000000 + 0x00100000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_TTI_PHYSICAL_PORT_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E, PRV_CPSS_DXCH_UNIT_TTI_E,
    {0x01000000 + 0x00210000 , BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_TTI_DEFAULT_PORT_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_POST_TTI_LOOKUP_INGRESS_EPORT_E, PRV_CPSS_DXCH_UNIT_TTI_E,
    {0x01000000 + 0x00230000 , 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    /* Bridge Tables */
    {CPSS_DXCH_SIP5_TABLE_BRIDGE_INGRESS_EPORT_E, PRV_CPSS_DXCH_UNIT_L2I_E,
    {0x03000000 + 0x00220000 , BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_L2I_EPORT_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_BRIDGE_INGRESS_EPORT_LEARN_PRIO_E, PRV_CPSS_DXCH_UNIT_L2I_E,
    {0x03000000 + 0x00210000 , BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_L2I_EPORT_LEARN_PRIO_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_BRIDGE_SOURCE_TRUNK_ATTRIBUTE_E, PRV_CPSS_DXCH_UNIT_L2I_E,
    {0x03000000 + 0x00202000 , BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_L2I_SOURCE_TRUNK_ATTRIBUTE_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_BRIDGE_PHYSICAL_PORT_E, PRV_CPSS_DXCH_UNIT_L2I_E,
    {0x03000000 + 0x00200000 , BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_L2I_PHYSICAL_PORT_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_BRIDGE_PHYSICAL_PORT_RATE_LIMIT_COUNTERS_E, PRV_CPSS_DXCH_UNIT_L2I_E,
    {0x03000000 + 0x00201000 , BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_L2I_PHYSICAL_PORT_RATE_LIMIT_COUNTERS_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

     /* EQ Tables */
    {CPSS_DXCH_SIP5_TABLE_EQ_INGRESS_EPORT_E, PRV_CPSS_DXCH_UNIT_EQ_E,
    {0x0D000000 + 0x00700000, 4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_EQ_INGRESS_STC_PHYSICAL_PORT_E, PRV_CPSS_DXCH_UNIT_EQ_E,
    {0x0D000000 + 0x00040000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_EQ_INGRESS_STC_PHYSICAL_PORT_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_EQ_INGRESS_MIRROR_PHYSICAL_PORT_E, PRV_CPSS_DXCH_UNIT_EQ_E,
    {0x0D000000 + 0x0000B300, 4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_EQ_TRUNK_LTT_E, PRV_CPSS_DXCH_UNIT_EQ_E,
    {0x0D000000 + 0x00160000, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_EQ_L2_ECMP_LTT_E, PRV_CPSS_DXCH_UNIT_EQ_E,
    {0x0D000000 + 0x00800000, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_EQ_L2_ECMP_E, PRV_CPSS_DXCH_UNIT_EQ_E,
    {0x0D000000 + 0x00900000, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_EQ_EPORT_TO_PHYSICAL_PORT_TARGET_MAPPING_E, PRV_CPSS_DXCH_UNIT_EQ_E,
    {0x0D000000 + 0x00c00000, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_EQ_TO_CPU_RATE_LIMITER_CONFIG_E, PRV_CPSS_DXCH_UNIT_EQ_E,
    {0x0D000000 + 0x00130000, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_EQ_TX_PROTECTION_E, PRV_CPSS_DXCH_UNIT_EQ_E,
    {0x0D000000 + 0x00a00000, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_EQ_EPORT_TO_LOC_MAPPING_E, PRV_CPSS_DXCH_UNIT_EQ_E,
    {0x0D000000 + 0x00a80000, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_EQ_PROTECTION_LOC_E, PRV_CPSS_DXCH_UNIT_EQ_E,
    {0x0D000000 + 0x00b00000, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    /* LPM memory */
    {CPSS_DXCH_SIP5_TABLE_LPM_MEM_E, PRV_CPSS_DXCH_UNIT_LPM_E,
    {0x58000000 + 0x00000000, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    /* IPv4 VRF Tables */
    {CPSS_DXCH_SIP5_TABLE_IPV4_VRF_E, PRV_CPSS_DXCH_UNIT_LPM_E,
    {0x58000000 + 0x00D10000, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    /* IPv6 VRF Tables */
    {CPSS_DXCH_SIP5_TABLE_IPV6_VRF_E, PRV_CPSS_DXCH_UNIT_LPM_E,
    {0x58000000 + 0x00D20000, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    /* FCoE VRF Tables */
    {CPSS_DXCH_SIP5_TABLE_FCOE_VRF_E, PRV_CPSS_DXCH_UNIT_LPM_E,
    {0x58000000 + 0x00D30000, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    /* ECMP Table */
    {CPSS_DXCH_SIP5_TABLE_ECMP_E, PRV_CPSS_DXCH_UNIT_LPM_E,
    {0x58000000 + 0x00D40000, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    /* EGF_EFT Tables */
    {CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_EFT_FAST_STACK_FAILOVER_SECONDARY_TARGET_PORT_E, PRV_CPSS_DXCH_UNIT_EGF_EFT_E,
    {0x35000000 + 0x00000000, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    /* EGF_QAG Tables */
    {CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_EGRESS_EPORT_E, PRV_CPSS_DXCH_UNIT_EGF_QAG_E,
    {0x3B000000 + 0x00800000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_EGF_QAG_EGRESS_EPORT_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_EGRESS_VLAN_E, PRV_CPSS_DXCH_UNIT_EGF_QAG_E,
    {0x3B000000 + 0x00000000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_EGF_QAG_EGRESS_EVLAN_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_E, PRV_CPSS_DXCH_UNIT_EGF_QAG_E,
    {0x3B000000 + 0x00900000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_EGF_QAG_TARGET_PORT_MAPPER_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_CPU_CODE_TO_LOOPBACK_MAPPER_E, PRV_CPSS_DXCH_UNIT_EGF_QAG_E,
    {0x3B000000 + 0x00900400, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_EGF_QAG_CPU_CODE_TO_LOOPBACK_MAPPER_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_FWD_FROM_CPU_TO_LOOPBACK_MAPPER_E, PRV_CPSS_DXCH_UNIT_EGF_QAG_E,
    {0x3B000000 + 0x00900800, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_EGF_QAG_FWD_FROM_CPU_TO_LOOPBACK_MAPPER_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    /* EGF_SHT Tables */
    {CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_EGRESS_EPORT_E, PRV_CPSS_DXCH_UNIT_EGF_SHT_E,
    {0x48000000 + 0x02200000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_EGF_SHT_EGRESS_EPORT_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_DEVICE_MAP_TABLE_E, PRV_CPSS_DXCH_UNIT_EGF_SHT_E,
    {0x48000000 + 0x02100000, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_LION_TABLE_VLAN_EGRESS_E, PRV_CPSS_DXCH_UNIT_EGF_SHT_E,
    {0x48000000 + 0x03000000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_EGF_SHT_EGRESS_VLAN_BITS_SIZE_CNS) , PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_VLAN_ATTRIBUTES_E, PRV_CPSS_DXCH_UNIT_EGF_SHT_E,
    {0x48000000 + 0x03400000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_EGF_SHT_EGRESS_VLAN_ATTRIBUTES_BITS_SIZE_CNS) , PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_VLAN_SPANNING_E, PRV_CPSS_DXCH_UNIT_EGF_SHT_E,
    {0x48000000 + 0x03440000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_EGF_SHT_EGRESS_VLAN_SPANNING_BITS_SIZE_CNS) , PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_VLAN_MAPPER_E, PRV_CPSS_DXCH_UNIT_EGF_SHT_E,
    {0x48000000 + 0x02110000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_EGF_SHT_EGRESS_VLAN_MAPPER_BITS_SIZE_CNS) , PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_TABLE_MULTICAST_E, PRV_CPSS_DXCH_UNIT_EGF_SHT_E,
    {0x48000000 + 0x020C0000, BITS_TO_BYTES_ALIGNMENT_MAC(BMP_256_PORTS_CNS) , PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_LION_TABLE_STG_EGRESS_E, PRV_CPSS_DXCH_UNIT_EGF_SHT_E,
    {0x48000000 + 0x02040000, BITS_TO_BYTES_ALIGNMENT_MAC(BMP_256_PORTS_CNS) , PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_LION_TABLE_PORT_ISOLATION_L2_E, PRV_CPSS_DXCH_UNIT_EGF_SHT_E,
    {0x48000000 + 0x00000000, BITS_TO_BYTES_ALIGNMENT_MAC(BMP_256_PORTS_CNS) , PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_LION_TABLE_PORT_ISOLATION_L3_E, PRV_CPSS_DXCH_UNIT_EGF_SHT_E,
    {0x48000000 + 0x04000000, BITS_TO_BYTES_ALIGNMENT_MAC(BMP_256_PORTS_CNS) , PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_LION_TABLE_TXQ_SOURCE_ID_MEMBERS_E, PRV_CPSS_DXCH_UNIT_EGF_SHT_E,
    {0x48000000 + 0x02080000, BITS_TO_BYTES_ALIGNMENT_MAC(BMP_256_PORTS_CNS) , PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_LION_TABLE_TXQ_NON_TRUNK_MEMBERS_E, PRV_CPSS_DXCH_UNIT_EGF_SHT_E,
    {0x48000000 + 0x020E0000, BITS_TO_BYTES_ALIGNMENT_MAC(BMP_256_PORTS_CNS) , PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_NON_TRUNK_MEMBERS2_E, PRV_CPSS_DXCH_UNIT_EGF_SHT_E,
    {0x48000000 + 0x02060000, BITS_TO_BYTES_ALIGNMENT_MAC(BMP_256_PORTS_CNS) , PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_EPORT_FILTER_E, PRV_CPSS_DXCH_UNIT_EGF_SHT_E,
    {0x48000000 + 0x020A0000, BITS_TO_BYTES_ALIGNMENT_MAC(BMP_256_PORTS_CNS) , PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_LION_TABLE_TXQ_DESIGNATED_PORT_E, PRV_CPSS_DXCH_UNIT_EGF_SHT_E,
    {0x48000000 + 0x02120000, BITS_TO_BYTES_ALIGNMENT_MAC(BMP_256_PORTS_CNS) , PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    /* HA Tables */
    {CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E, PRV_CPSS_DXCH_UNIT_HA_E,
    {0x0F000000 + 0x00100000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_HA_EGRESS_EPORT_1_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_2_E, PRV_CPSS_DXCH_UNIT_HA_E,
    {0x0F000000 + 0x00030000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_HA_EGRESS_EPORT_2_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_1_E, PRV_CPSS_DXCH_UNIT_HA_E,
    {0x0F000000 + 0x00001000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_HA_EGRESS_PHYSICAL_PORT_1_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E, PRV_CPSS_DXCH_UNIT_HA_E,
    {0x0F000000 + 0x00002000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_HA_EGRESS_PHYSICAL_PORT_2_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_HA_QOS_PROFILE_TO_EXP_E, PRV_CPSS_DXCH_UNIT_HA_E,
    {0x0F000000 + 0x00004000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_HA_QOS_PROFILE_TO_EXP_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_HA_EPCL_UDB_CONFIG_E, PRV_CPSS_DXCH_UNIT_HA_E,
    {0x0F000000 + 0x00060000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_HA_EPCL_UDB_CONFIG_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_HA_PTP_DOMAIN_E, PRV_CPSS_DXCH_UNIT_HA_E,
    {0x0F000000 + 0x00040000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_HA_PTP_DOMAIN_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_HA_GENERIC_TS_PROFILE_E, PRV_CPSS_DXCH_UNIT_HA_E,
    {0x0F000000 + 0x00050000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_HA_GEN_TS_PROFILE_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_HA_GLOBAL_MAC_SA_E, PRV_CPSS_DXCH_UNIT_HA_E,
    {0x0F000000 + 0x00003000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_HA_GLOBAL_MAC_SA_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH2_TABLE_TUNNEL_START_CONFIG_E, PRV_CPSS_DXCH_UNIT_HA_E,
    {0x0F000000 + 0x00200000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_HA_TUNNEL_START_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH3_TABLE_EGRESS_VLAN_TRANSLATION_E, PRV_CPSS_DXCH_UNIT_HA_E,
    {0x0F000000 + 0x00010000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_HA_EGRESS_VLAN_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_TABLE_ROUTE_HA_MAC_SA_E, PRV_CPSS_DXCH_UNIT_HA_E,
    {0x0F000000 + 0x00020000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_HA_EGRESS_VLAN_MAC_SA_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},


    /* MLL Tables */
    {CPSS_DXCH_SIP5_TABLE_IP_MLL_E, PRV_CPSS_DXCH_UNIT_MLL_E,
    {0x1D000000 + 0x00080000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_MLL_ENTRY_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_L2_MLL_E,/* must be the same as 'IP_MLL' */ PRV_CPSS_DXCH_UNIT_MLL_E,
    {0x1D000000 + 0x00080000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_MLL_ENTRY_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_L2_MLL_LTT_E, PRV_CPSS_DXCH_UNIT_MLL_E,
    {0x1D000000 + 0x00040000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_L2_MLL_LTT_ENTRY_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    /*IPCL tables */
    {CPSS_DXCH_TABLE_PCL_CONFIG_E, PRV_CPSS_DXCH_UNIT_PCL_E ,/* table for first lookup */
    {0x02000000 + 0x00010000,BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_PCL_PCL_CONFIG_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_XCAT2_TABLE_INGRESS_PCL_LOOKUP01_CONFIG_E, PRV_CPSS_DXCH_UNIT_PCL_E ,  /* table for second lookup  */
    {0x02000000 + 0x00020000,BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_PCL_PCL_CONFIG_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_XCAT_TABLE_INGRESS_PCL_LOOKUP1_CONFIG_E, PRV_CPSS_DXCH_UNIT_PCL_E ,  /* table for third lookup */
    {0x02000000 + 0x00030000,BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_PCL_PCL_CONFIG_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_LION_TABLE_TRUNK_HASH_MASK_CRC_E, PRV_CPSS_DXCH_UNIT_PCL_E ,
    {0x02000000 + 0x00000C00 , BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_PCL_TRUNK_HASH_MASK_CRC_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_IPCL0_UDB_SELECT_E, PRV_CPSS_DXCH_UNIT_PCL_E ,
    {0x02000000 + 0x00040000 , BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_IPCL_UDB_SELECT_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_IPCL1_UDB_SELECT_E, PRV_CPSS_DXCH_UNIT_PCL_E ,
    {0x02000000 + 0x00042000 , BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_IPCL_UDB_SELECT_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_IPCL2_UDB_SELECT_E, PRV_CPSS_DXCH_UNIT_PCL_E ,
    {0x02000000 + 0x00044000 , BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_IPCL_UDB_SELECT_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_TCAM_PCL_TTI_ACTION_E, PRV_CPSS_DXCH_UNIT_TCAM_E ,
    {0x05000000 + 0x00200000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_TCAM_PCL_TTI_ACTION_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_ERMRK_TIMESTAMP_CFG_E, PRV_CPSS_DXCH_UNIT_ERMRK_E ,
    {0x15000000 + 0x00005000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_ERMRK_TIMESTAMP_CFG_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_ERMRK_PTP_LOCAL_ACTION_E, PRV_CPSS_DXCH_UNIT_ERMRK_E ,
    {0x15000000 + 0x00100000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_ERMRK_PTP_LOCAL_ACTION_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_ERMRK_PTP_TARGET_PORT_E, PRV_CPSS_DXCH_UNIT_ERMRK_E ,
    {0x15000000 + 0x00200000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_ERMRK_PTP_TARGET_PORT_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_ERMRK_PTP_SOURCE_PORT_E, PRV_CPSS_DXCH_UNIT_ERMRK_E ,
    {0x15000000 + 0x00300000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_ERMRK_PTP_SOURCE_PORT_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_ERMRK_QOS_DSCP_MAP_E, PRV_CPSS_DXCH_UNIT_ERMRK_E ,
    {0x15000000 + 0x00003000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_ERMRK_QOS_DSCP_MAP_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_ERMRK_QOS_TC_DP_MAP_E, PRV_CPSS_DXCH_UNIT_ERMRK_E ,
    {0x15000000 + 0x00003300, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_ERMRK_QOS_TC_DP_MAP_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},


    {CPSS_DXCH_SIP5_TABLE_TCAM_E, PRV_CPSS_DXCH_UNIT_TCAM_E ,
    {0x05000000 + 0x00000000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_TCAM_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    /* TRILL Tables */
    {CPSS_DXCH_SIP5_TABLE_ADJACENCY_E, PRV_CPSS_DXCH_UNIT_TTI_E,
    /* CPSS_DXCH_SIP5_TABLE_ADJACENCY_E */
    {0x01000000 + 0x00006000, 0xC, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    /* Policer Tables */
    {CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_E_ATTRIBUTES_E, PRV_CPSS_DXCH_UNIT_IPLR_E,
    {0x0B000000 + 0x10000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_IPVX_POLICER_E_ATTRIBUTES_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_REMARKING_E, PRV_CPSS_DXCH_UNIT_IPLR_E,
    {0x0B000000 + 0x80000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_IPVX_POLICER_REMARKING_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_HIERARCHICAL_E, PRV_CPSS_DXCH_UNIT_IPLR_E,
    {0x0B000000 + 0x90000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_IPVX_POLICER_HIERARCHICAL_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_METERING_E, PRV_CPSS_DXCH_UNIT_IPLR_E,
    {0x0B000000 + 0x100000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_IPVX_POLICER_METERING_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_E_ATTRIBUTES_E, PRV_CPSS_DXCH_UNIT_IPLR_1_E,
    {0x20000000 + 0x10000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_IPVX_POLICER_E_ATTRIBUTES_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_REMARKING_E, PRV_CPSS_DXCH_UNIT_IPLR_1_E,
    {0x20000000 + 0x80000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_IPVX_POLICER_REMARKING_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_METERING_E, PRV_CPSS_DXCH_UNIT_IPLR_1_E,
    {0x20000000 + 0x100000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_IPVX_POLICER_METERING_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_EGRESS_POLICER_E_ATTRIBUTES_E, PRV_CPSS_DXCH_UNIT_EPLR_E,
    {0x0E000000 + 0x10000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_IPVX_POLICER_E_ATTRIBUTES_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_XCAT_TABLE_EGRESS_POLICER_REMARKING_E, PRV_CPSS_DXCH_UNIT_EPLR_E,
    {0x0E000000 + 0x80000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_IPVX_POLICER_REMARKING_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_EGRESS_POLICER_METERING_E, PRV_CPSS_DXCH_UNIT_EPLR_E,
    {0x0E000000 + 0x100000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_IPVX_POLICER_METERING_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    /* IPVX Tables */
    {CPSS_DXCH_SIP5_TABLE_IPVX_INGRESS_EPORT_E, PRV_CPSS_DXCH_UNIT_IPVX_E,
    /* CPSS_DXCH_SIP5_TABLE_IPVX_INGRESS_EPORT_E */
    {0x06000000 + 0x00200000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_IPVX_ROUTER_EPORT_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_IPVX_EVLAN_E, PRV_CPSS_DXCH_UNIT_IPVX_E,
    /* CPSS_DXCH_SIP5_TABLE_IPVX_EVLAN_E */
    {0x06000000 + 0x00100000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_IPVX_ROUTER_EVLAN_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_IPVX_ROUTER_NEXTHOP_E, PRV_CPSS_DXCH_UNIT_IPVX_E,
    /* CPSS_DXCH_SIP5_TABLE_IPVX_ROUTER_NEXTHOP_E */
    {0x06000000 + 0x00400000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_IPVX_ROUTER_NEXTHOP_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_IPVX_ROUTER_NEXTHOP_AGE_BITS_E, PRV_CPSS_DXCH_UNIT_IPVX_E,
    /* CPSS_DXCH_SIP5_TABLE_IPVX_ROUTER_NEXTHOP_AGE_BITS_E */
    {0x06000000 + 0x00020000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_IPVX_ROUTER_NEXTHOP_AGE_BITS_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_IPVX_ROUTER_ACCESS_MATRIX_E, PRV_CPSS_DXCH_UNIT_IPVX_E,
    /* CPSS_DXCH_SIP5_TABLE_IPVX_ROUTER_ACCESS_MATRIX_E */
    {0x06000000 + 0x00060000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_IPVX_ROUTER_ACCESS_MATRIX_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_IPVX_ROUTER_QOS_PROFILE_OFFSETS_E, PRV_CPSS_DXCH_UNIT_IPVX_E,
    /* CPSS_DXCH_SIP5_TABLE_IPVX_ROUTER_QOS_PROFILE_OFFSETS_E */
    {0x06000000 + 0x00010000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_IPVX_ROUTER_QOS_PROFILE_OFFSETS_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_E, PRV_CPSS_DXCH_UNIT_IOAM_E,
    /* CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_E */
    {0x1C000000 + 0x00070000, 0x10, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_KEEPALIVE_AGING_E, PRV_CPSS_DXCH_UNIT_IOAM_E,
    /* CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_KEEPALIVE_AGING_E */
    {0x1C000000 + 0x00000C00, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_MEG_EXCEPTION_E, PRV_CPSS_DXCH_UNIT_IOAM_E,
    /* CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_MEG_EXCEPTION_E */
    {0x1C000000 + 0x00010000, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_SOURCE_INTERFACE_EXCEPTION_E, PRV_CPSS_DXCH_UNIT_IOAM_E,
    /* CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_SOURCE_INTERFACE_EXCEPTION_E */
    {0x1C000000 + 0x00018000, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_INVALID_KEEPALIVE_HASH_E, PRV_CPSS_DXCH_UNIT_IOAM_E,
    /* CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_INVALID_KEEPALIVE_HASH_E */
    {0x1C000000 + 0x00020000, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_KEEPALIVE_EXCESS_E, PRV_CPSS_DXCH_UNIT_IOAM_E,
    /* CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_KEEPALIVE_EXCESS_E */
    {0x1C000000 + 0x00028000, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_EXCEPTION_SUMMARY_E, PRV_CPSS_DXCH_UNIT_IOAM_E,
    /* CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_EXCEPTION_SUMMARY_E */
    {0x1C000000 + 0x00030000, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_RDI_STATUS_CHANGE_EXCEPTION_E, PRV_CPSS_DXCH_UNIT_IOAM_E,
    /* CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_RDI_STATUS_CHANGE_EXCEPTION_E */
    {0x1C000000 + 0x00038000, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_TX_PERIOD_EXCEPTION_E, PRV_CPSS_DXCH_UNIT_IOAM_E,
    /* CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_TX_PERIOD_EXCEPTION_E */
    {0x1C000000 + 0x00040000, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_OPCODE_PACKET_COMMAND_E, PRV_CPSS_DXCH_UNIT_IOAM_E,
    /* CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_OPCODE_PACKET_COMMAND_E */
    {0x1C000000 + 0x00060000, 0x8, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_E, PRV_CPSS_DXCH_UNIT_EOAM_E,
    /* CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_E */
    {0x1E000000 + 0x00070000, 0x10, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_KEEPALIVE_AGING_E, PRV_CPSS_DXCH_UNIT_EOAM_E,
    /* CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_KEEPALIVE_AGING_E */
    {0x1E000000 + 0x00000C00, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_MEG_EXCEPTION_E, PRV_CPSS_DXCH_UNIT_EOAM_E,
    /* CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_MEG_EXCEPTION_E */
    {0x1E000000 + 0x00010000, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_SOURCE_INTERFACE_EXCEPTION_E, PRV_CPSS_DXCH_UNIT_EOAM_E,
    /* CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_SOURCE_INTERFACE_EXCEPTION_E */
    {0x1E000000 + 0x00018000, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_INVALID_KEEPALIVE_HASH_E, PRV_CPSS_DXCH_UNIT_EOAM_E,
    /* CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_INVALID_KEEPALIVE_HASH_E */
    {0x1E000000 + 0x00020000, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_KEEPALIVE_EXCESS_E, PRV_CPSS_DXCH_UNIT_EOAM_E,
    /* CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_KEEPALIVE_EXCESS_E */
    {0x1E000000 + 0x00028000, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_EXCEPTION_SUMMARY_E, PRV_CPSS_DXCH_UNIT_EOAM_E,
    /* CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_EXCEPTION_SUMMARY_E */
    {0x1E000000 + 0x00030000, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_RDI_STATUS_CHANGE_EXCEPTION_E, PRV_CPSS_DXCH_UNIT_EOAM_E,
    /* CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_RDI_STATUS_CHANGE_EXCEPTION_E */
    {0x1E000000 + 0x00038000, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_TX_PERIOD_EXCEPTION_E, PRV_CPSS_DXCH_UNIT_EOAM_E,
    /* CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_TX_PERIOD_EXCEPTION_E */
    {0x1E000000 + 0x00040000, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_OPCODE_PACKET_COMMAND_E, PRV_CPSS_DXCH_UNIT_EOAM_E,
    /* CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_OPCODE_PACKET_COMMAND_E */
    {0x1E000000 + 0x00060000, 0x8, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_OAM_LM_OFFSET_E, PRV_CPSS_DXCH_UNIT_ERMRK_E,
    /* CPSS_DXCH_SIP5_TABLE_OAM_LM_OFFSET_E */
    {0x15000000 + 0x00002000, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_TTI_QOS_MAP_DSCP_TO_QOS_PROFILE_E, PRV_CPSS_DXCH_UNIT_TTI_E,
    {0x01000000 + 0x00000800, 0x80, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_TTI_QOS_MAP_UP_CFI_TO_QOS_PROFILE_E, PRV_CPSS_DXCH_UNIT_TTI_E,
    {0x01000000 + 0x00000E00, 0x20, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_TTI_QOS_MAP_EXP_TO_QOS_PROFILE_E, PRV_CPSS_DXCH_UNIT_TTI_E,
    {0x01000000 + 0x00000700, 0x10, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_TTI_QOS_MAP_DSCP_TO_DSCP_E, PRV_CPSS_DXCH_UNIT_TTI_E,
    {0x01000000 + 0x00000400, 0x40, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_TTI_UDB_CONFIG_E, PRV_CPSS_DXCH_UNIT_TTI_E,
    {0x01000000 + 0x00040000, 0x40, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_TTI_PTP_COMMAND_E, PRV_CPSS_DXCH_UNIT_TTI_E,
    {0x01000000 + 0x00004000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_TTI_PTP_COMMAND_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    /* EPCL unit */
    {CPSS_DXCH2_TABLE_EGRESS_PCL_CONFIG_E, PRV_CPSS_DXCH_UNIT_EPCL_E,
    {0x14000000 + 0x00008000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_EPCL_PCL_CONFIG_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},


    {CPSS_DXCH_SIP5_TABLE_EPCL_UDB_SELECT_E, PRV_CPSS_DXCH_UNIT_EPCL_E,
    {0x14000000 + 0x00010000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_EPCL_UDB_SELECT_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    /* BMA unit */
    {CPSS_DXCH_SIP5_TABLE_BMA_PORT_MAPPING_E, PRV_CPSS_DXCH_UNIT_BMA_E,
    {0x2D000000 + 0x0001A000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_BMA_PORT_MAPPING_BIT_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_BMA_MULTICAST_COUNTERS_E, PRV_CPSS_DXCH_UNIT_BMA_E,
    {0x2D000000 + 0x00000000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_BMA_MULTICAST_COUNTERS_BIT_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    /* Tail Drop Tables */
    {CPSS_DXCH_SIP5_TABLE_TAIL_DROP_MAX_QUEUE_LIMITS_E, PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E,
    {0x3C000000 + 0x000A1000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_TAIL_DROP_MAX_QUEUE_LIMITS_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_TAIL_DROP_EQ_QUEUE_LIMITS_DP0_E, PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E,
    {0x3C000000 + 0x000A1800, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_TAIL_DROP_EQ_QUEUE_LIMITS_DP0_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_TAIL_DROP_DQ_QUEUE_BUF_LIMITS_E, PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E,
    {0x3C000000 + 0x000A2000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_TAIL_DROP_DQ_QUEUE_BUF_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_TAIL_DROP_DQ_QUEUE_DESC_LIMITS_E, PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E,
    {0x3C000000 + 0x000A2800, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_TAIL_DROP_DQ_QUEUE_DESC_LIMITS_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_TAIL_DROP_EQ_QUEUE_LIMITS_DP1_E, PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E,
    {0x3C000000 + 0x000A3000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_TAIL_DROP_EQ_QUEUE_LIMITS_DP1_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_TAIL_DROP_EQ_QUEUE_LIMITS_DP2_E, PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E,
    {0x3C000000 + 0x000A3400, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_TAIL_DROP_EQ_QUEUE_LIMITS_DP2_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_TAIL_DROP_COUNTERS_Q_MAIN_BUFF_E, PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E,
    {0x3C000000 + 0x000A7000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_TAIL_DROP_COUNTERS_MAIN_BUFF_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_TAIL_DROP_COUNTERS_Q_MAIN_MC_BUFF_E, PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E,
    {0x3C000000 + 0x000A8000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_TAIL_DROP_COUNTERS_MAIN_MC_BUFF_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_TXQ_LINK_LIST_COUNTERS_Q_MAIN_DESC_E, PRV_CPSS_DXCH_UNIT_TXQ_LL_E,
    {0x3D000000 + 0x0008C000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_TXQ_LINK_LIST_COUNTERS_MAIN_DESC_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_PFC_MODE_PROFILE_TC_XOFF_THRESHOLDS_E, PRV_CPSS_DXCH_UNIT_TXQ_PFC_E,
    {0x3E000000 + 0x00001800, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_PFC_MODE_PROFILE_TC_XOFF_THRESHOLDS_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_PFC_MODE_PROFILE_TC_XON_THRESHOLDS_E, PRV_CPSS_DXCH_UNIT_TXQ_PFC_E,
    {0x3E000000 + 0x00001A00, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_PFC_MODE_PROFILE_TC_XON_THRESHOLDS_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_PFC_LLFC_COUNTERS_E, PRV_CPSS_DXCH_UNIT_TXQ_PFC_E,
    {0x3E000000 + 0x00002000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_PFC_LLFC_COUNTERS_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_TM_DROP_DROP_MASKING_E, PRV_CPSS_DXCH_UNIT_TM_DROP_E,
    {0x09000000 + 0x00001000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_TM_DROP_DROP_MASKING_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_TM_DROP_QUEUE_PROFILE_ID_E, PRV_CPSS_DXCH_UNIT_TM_DROP_E,
    {0x09000000 + 0x00010000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_TM_DROP_QUEUE_PROFILE_ID_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_TM_QMAP_CPU_CODE_TO_TC_E, PRV_CPSS_DXCH_UNIT_TM_QMAP_E,
    {0x0A000000 + 0x00001000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_TM_QMAP_CPU_CODE_TO_TC_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_TM_QMAP_TARGET_DEV_TO_INDEX_E, PRV_CPSS_DXCH_UNIT_TM_QMAP_E,
    {0x0A000000 + 0x00010000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_TM_QMAP_TARGET_DEV_TO_INDEX_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_TM_QMAP_POLICING_ENABLE_E, PRV_CPSS_DXCH_UNIT_TM_QMAP_E,
    {0x0A000000 + 0x00020000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_TM_QMAP_TM_QMAP_POLICING_ENABLE_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_TM_QMAP_QUEUE_ID_SELECTOR_E, PRV_CPSS_DXCH_UNIT_TM_QMAP_E,
    {0x0A000000 + 0x00030000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_TM_QMAP_QUEUE_ID_SELECTOR_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_TM_INGR_GLUE_L1_PKT_LEN_OFFSET_E, PRV_CPSS_DXCH_UNIT_TM_INGRESS_GLUE_E,
    {0x18000000 + 0x00001000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_TM_INGR_GLUE_L1_PKT_LEN_OFFSET_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    {CPSS_DXCH_SIP5_TABLE_TM_EGR_GLUE_AGING_QUEUE_PROFILE_E, PRV_CPSS_DXCH_UNIT_TM_EGRESS_GLUE_E,
    {0x19000000 + 0x00004000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_TM_EGR_GLUE_AGING_QUEUE_PROFILE_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    /*PRV_DXCH_BOBCAT2_TABLE_TM_EGRESS_GLUE_AGING_PROFILE_THESHOLDS_E*/
    {CPSS_DXCH_SIP5_TABLE_TM_EGR_GLUE_AGING_PROFILE_THESHOLDS_E, PRV_CPSS_DXCH_UNIT_TM_EGRESS_GLUE_E,
    {0x19000000 + 0x00006000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_TM_EGR_GLUE_AGING_PROFILE_THESHOLDS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    /*PRV_DXCH_BOBCAT2_TABLE_TM_EGRESS_GLUE_TARGET_INTERFACE_E*/
    {CPSS_DXCH_SIP5_TABLE_TM_EGR_GLUE_TARGET_INTERFACE_E, PRV_CPSS_DXCH_UNIT_TM_EGRESS_GLUE_E,
    {0x19000000 + 0x00006200, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_TM_EGR_GLUE_TARGET_INTERFACE_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    /*PRV_DXCH_BOBCAT2_TABLE_TM_FCU_ETH_DMA_TO_TM_PORT_MAPPING_E*/
    {CPSS_DXCH_SIP5_TABLE_TM_FCU_ETH_DMA_TO_TM_PORT_MAPPING_E, PRV_CPSS_DXCH_UNIT_TM_FCU_E,
    {0x08000000 + 0x00000500, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_TM_FCU_ETH_DMA_TO_TM_PORT_MAPPING_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    /*PRV_DXCH_BOBCAT2_TABLE_TM_FCU_ILK_DMA_TO_TM_PORT_MAPPING_E*/
    {CPSS_DXCH_SIP5_TABLE_TM_FCU_ILK_DMA_TO_TM_PORT_MAPPING_E, PRV_CPSS_DXCH_UNIT_TM_FCU_E,
    {0x08000000 + 0x00000700, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_TM_FCU_ILK_DMA_TO_TM_PORT_MAPPING_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    /*PRV_DXCH_BOBCAT2_TABLE_TM_FCU_TC_PORT_TO_CNODE_PORT_MAPPING_E*/
    {CPSS_DXCH_SIP5_TABLE_TM_FCU_TC_PORT_TO_CNODE_PORT_MAPPING_E, PRV_CPSS_DXCH_UNIT_TM_FCU_E,
    {0x08000000 + 0x00001000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_TM_FCU_TC_PORT_TO_CNODE_PORT_MAPPING_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    /*PRV_DXCH_BOBCAT2_TABLE_TM_FCU_PORT_INGRESS_TIMERS_CONFIG_E*/
    {CPSS_DXCH_SIP5_TABLE_TM_FCU_PORT_INGRESS_TIMERS_CONFIG_E, PRV_CPSS_DXCH_UNIT_TM_FCU_E,
    {0x08000000 + 0x00003000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_TM_FCU_PORT_INGRESS_TIMERS_CONFIG_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    /*PRV_DXCH_BOBCAT2_TABLE_TM_FCU_INGRESS_TIMERS_E*/
    {CPSS_DXCH_SIP5_TABLE_TM_FCU_INGRESS_TIMERS_E, PRV_CPSS_DXCH_UNIT_TM_FCU_E,
    {0x08000000 + 0x00008000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_TM_FCU_INGRESS_TIMERS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    /*PRV_DXCH_BOBCAT2_TABLE_TM_FCU_PFC_PORT_TO_PHYSICAL_PORT_MAPPING_E*/
    {CPSS_DXCH_SIP5_TABLE_TM_FCU_PORT_TO_PHYSICAL_PORT_MAPPING_E, PRV_CPSS_DXCH_UNIT_TM_FCU_E,
    {0x08000000 + 0x00000100, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_TM_FCU_PORT_TO_PHYSICAL_PORT_MAPPING_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    /*PRV_DXCH_BOBCAT2_TABLE_CN_SAMPLE_INTERVALS_E*/
    {CPSS_DXCH_SIP5_TABLE_CN_SAMPLE_INTERVALS_E, PRV_CPSS_DXCH_UNIT_TXQ_QCN_E,
    {0x3F000000 + 0x00002000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_CN_SAMPLE_INTERVALS_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    /*PRV_CPSS_BOBCAT2_TABLE_INGRESS_POLICER_0_COUNTING_E*/
    {CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_COUNTING_E, PRV_CPSS_DXCH_UNIT_IPLR_E,
    {0x0B000000 + 0x100000+0x40000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_IPVX_POLICER_COUNTING_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    /*PRV_CPSS_BOBCAT2_TABLE_INGRESS_POLICER_1_COUNTING_E*/
    {CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_COUNTING_E, PRV_CPSS_DXCH_UNIT_IPLR_1_E,
    {0x20000000 + 0x100000+0x40000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_IPVX_POLICER_COUNTING_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    /*PRV_CPSS_BOBCAT2_TABLE_EGRESS_POLICER_COUNTING_E*/
    {CPSS_DXCH_SIP5_TABLE_EGRESS_POLICER_COUNTING_E, PRV_CPSS_DXCH_UNIT_EPLR_E,
    {0x0E000000 + 0x100000+0x40000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_IPVX_POLICER_COUNTING_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    /*PRV_CPSS_BOBCAT2_TABLE_INGRESS_POLICER_0_IPFIX_ALERT_WRAPAROUND_E*/
    {CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_IPFIX_ALERT_WRAPAROUND_E, PRV_CPSS_DXCH_UNIT_IPLR_E,
    {0x0B000000 + 0x800, BITS_TO_BYTES_ALIGNMENT_MAC(32), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    /*PRV_CPSS_BOBCAT2_TABLE_INGRESS_POLICER_1_IPFIX_ALERT_WRAPAROUND_E*/
    {CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_IPFIX_ALERT_WRAPAROUND_E, PRV_CPSS_DXCH_UNIT_IPLR_1_E,
    {0x20000000 + 0x800, BITS_TO_BYTES_ALIGNMENT_MAC(32), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    /*PRV_CPSS_BOBCAT2_TABLE_EGRESS_POLICER_IPFIX_ALERT_WRAPAROUND_E*/
    {CPSS_DXCH_SIP5_TABLE_EGRESS_POLICER_IPFIX_ALERT_WRAPAROUND_E, PRV_CPSS_DXCH_UNIT_EPLR_E,
    {0x0E000000 + 0x800, BITS_TO_BYTES_ALIGNMENT_MAC(32), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    /*PRV_CPSS_BOBCAT2_TABLE_INGRESS_POLICER_0_IPFIX_ALERT_AGING_E*/
    {CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_IPFIX_ALERT_AGING_E, PRV_CPSS_DXCH_UNIT_IPLR_E,
    {0x0B000000 + 0x1000, BITS_TO_BYTES_ALIGNMENT_MAC(32), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    /*PRV_CPSS_BOBCAT2_TABLE_INGRESS_POLICER_1_IPFIX_ALERT_AGING_E*/
    {CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_IPFIX_ALERT_AGING_E, PRV_CPSS_DXCH_UNIT_IPLR_1_E,
    {0x20000000 + 0x1000, BITS_TO_BYTES_ALIGNMENT_MAC(32), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    /*PRV_CPSS_BOBCAT2_TABLE_EGRESS_POLICER_IPFIX_ALERT_AGING_E*/
    {CPSS_DXCH_SIP5_TABLE_EGRESS_POLICER_IPFIX_ALERT_AGING_E, PRV_CPSS_DXCH_UNIT_EPLR_E,
    {0x0E000000 + 0x1000, BITS_TO_BYTES_ALIGNMENT_MAC(32), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE},

    PRV_CPSS_DXCH_SIP5_TABLE_CNC_COUNTERS_MAC( 0),
    PRV_CPSS_DXCH_SIP5_TABLE_CNC_COUNTERS_MAC( 1),
    PRV_CPSS_DXCH_SIP5_TABLE_CNC_COUNTERS_MAC( 2),
    PRV_CPSS_DXCH_SIP5_TABLE_CNC_COUNTERS_MAC( 3),
    PRV_CPSS_DXCH_SIP5_TABLE_CNC_COUNTERS_MAC( 4),
    PRV_CPSS_DXCH_SIP5_TABLE_CNC_COUNTERS_MAC( 5),
    PRV_CPSS_DXCH_SIP5_TABLE_CNC_COUNTERS_MAC( 6),
    PRV_CPSS_DXCH_SIP5_TABLE_CNC_COUNTERS_MAC( 7),
    PRV_CPSS_DXCH_SIP5_TABLE_CNC_COUNTERS_MAC( 8),
    PRV_CPSS_DXCH_SIP5_TABLE_CNC_COUNTERS_MAC( 9),
    PRV_CPSS_DXCH_SIP5_TABLE_CNC_COUNTERS_MAC(10),
    PRV_CPSS_DXCH_SIP5_TABLE_CNC_COUNTERS_MAC(11),
    PRV_CPSS_DXCH_SIP5_TABLE_CNC_COUNTERS_MAC(12),
    PRV_CPSS_DXCH_SIP5_TABLE_CNC_COUNTERS_MAC(13),
    PRV_CPSS_DXCH_SIP5_TABLE_CNC_COUNTERS_MAC(14),
    PRV_CPSS_DXCH_SIP5_TABLE_CNC_COUNTERS_MAC(15),
    PRV_CPSS_DXCH_SIP5_TABLE_CNC_COUNTERS_MAC(16),
    PRV_CPSS_DXCH_SIP5_TABLE_CNC_COUNTERS_MAC(17),
    PRV_CPSS_DXCH_SIP5_TABLE_CNC_COUNTERS_MAC(18),
    PRV_CPSS_DXCH_SIP5_TABLE_CNC_COUNTERS_MAC(19),
    PRV_CPSS_DXCH_SIP5_TABLE_CNC_COUNTERS_MAC(20),
    PRV_CPSS_DXCH_SIP5_TABLE_CNC_COUNTERS_MAC(21),
    PRV_CPSS_DXCH_SIP5_TABLE_CNC_COUNTERS_MAC(22),
    PRV_CPSS_DXCH_SIP5_TABLE_CNC_COUNTERS_MAC(23),
    PRV_CPSS_DXCH_SIP5_TABLE_CNC_COUNTERS_MAC(24),
    PRV_CPSS_DXCH_SIP5_TABLE_CNC_COUNTERS_MAC(25),
    PRV_CPSS_DXCH_SIP5_TABLE_CNC_COUNTERS_MAC(26),
    PRV_CPSS_DXCH_SIP5_TABLE_CNC_COUNTERS_MAC(27),
    PRV_CPSS_DXCH_SIP5_TABLE_CNC_COUNTERS_MAC(28),
    PRV_CPSS_DXCH_SIP5_TABLE_CNC_COUNTERS_MAC(29),
    PRV_CPSS_DXCH_SIP5_TABLE_CNC_COUNTERS_MAC(30),
    PRV_CPSS_DXCH_SIP5_TABLE_CNC_COUNTERS_MAC(31)

};

/*size of array directAccessBobcat2TableInfo */
static const GT_U32 directAccessBobcat2TableInfoSize =
    (sizeof(directAccessBobcat2TableInfo) / sizeof(directAccessBobcat2TableInfo[0]));

/* Bobcat2 B0 direct table information , that need to override A0 info from directAccessBobcat2TableInfo */
static const PRV_CPSS_DXCH_TABLES_INFO_DIRECT_EXT_STC directAccessBobcat2B0TableInfo_overrideA0[] =
{
    /* TTI  */
    {CPSS_DXCH_TABLE_VLAN_PORT_PROTOCOL_E,
    {0x01000000 + 0x00010000, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

    {CPSS_DXCH_SIP5_TABLE_POST_TTI_LOOKUP_INGRESS_EPORT_E,
    {0x01000000 + 0x00240000 , 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

    {CPSS_DXCH3_TABLE_INGRESS_VLAN_TRANSLATION_E,
    {0x01000000 + 0x00050000, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

    {CPSS_DXCH_SIP5_TABLE_INGRESS_SPAN_STATE_GROUP_INDEX_E,
    {0x03000000 + 0x00218000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_L2I_INGRESS_SPAN_STATE_GROUP_INDEX_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

    /* L2 */
    {CPSS_DXCH_SIP5_TABLE_BRIDGE_INGRESS_EPORT_E,
    {0x03000000 + 0x00300000 , BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_L2I_EPORT_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

     /* PLR */
    {CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_E_ATTRIBUTES_E,
    {0x0B000000 + 0x00040000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_IPVX_POLICER_E_ATTRIBUTES_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

    {CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_E_ATTRIBUTES_E,
    {0x20000000 + 0x00040000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_IPVX_POLICER_E_ATTRIBUTES_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

    {CPSS_DXCH_SIP5_TABLE_EGRESS_POLICER_E_ATTRIBUTES_E,
    {0x0E000000 + 0x00040000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_IPVX_POLICER_E_ATTRIBUTES_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

    /* TXQ */
    {CPSS_DXCH_SIP5_TABLE_TXQ_LINK_LIST_COUNTERS_Q_MAIN_DESC_E,
    {0x3D000000 + 0x0008D000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_TXQ_LINK_LIST_COUNTERS_MAIN_DESC_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

    /* HA */
    {CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E,
    {0x0F000000 + 0x00100000, BITS_TO_BYTES_ALIGNMENT_MAC(BOBCAT_B0_HA_EGRESS_EPORT_1_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

    {CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_2_E,
    {0x0F000000 + 0x00200000, BITS_TO_BYTES_ALIGNMENT_MAC(BOBCAT_B0_HA_EGRESS_EPORT_2_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

    {CPSS_DXCH2_TABLE_TUNNEL_START_CONFIG_E,
    {0x0F000000 + 0x00400000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_HA_TUNNEL_START_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

    /* ERMRK */
    {CPSS_DXCH_SIP5_TABLE_ERMRK_TIMESTAMP_CFG_E,
    {0x15000000 + 0x00005000, BITS_TO_BYTES_ALIGNMENT_MAC(BOBCAT2_B0_ERMRK_TIMESTAMP_CFG_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}}

    /*CPFC*/
    ,{CPSS_DXCH_SIP5_TABLE_PFC_LLFC_COUNTERS_E,
    {0x3e000000 + 0x00008000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_PFC_LLFC_COUNTERS_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}}


};

/* XCat A1 revision Logical Target Mapping table indirect info */
static const PRV_CPSS_DXCH_TABLES_INFO_INDIRECT_STC xcatA1LogicalTargetMappingTblInfo =
    {0x0B0000A0, 0x0B00A000, 0, 2, 0, 0, 1};

static const PRV_CPSS_DXCH_TABLES_INFO_STC dxChXcat3TablesInfo[CPSS_DXCH_XCAT3_TABLE_LAST_E] =
{
    /*CPSS_DXCH_TABLE_VLAN_PORT_PROTOCOL_E*/
    {   _1K,
        PRV_CPSS_DXCHXCAT_PRTCL_BASED_VLAN_ENTRY_WORDS_COUNT_CNS,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_VLAN_PORT_PROTOCOL_E],
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_VLAN_PORT_PROTOCOL_E]},

    /*CPSS_DXCH_TABLE_PORT_VLAN_QOS_E*/
    {   PRV_CPSS_DXCH_PORT_TBLS_MAX_INDEX_CNS,
        3,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_PORT_VLAN_QOS_E],
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_PORT_VLAN_QOS_E]},

    /*CPSS_DXCH_TABLE_TRUNK_MEMBERS_E*/
    {   PRV_CPSS_DXCH_TRUNK_MEMBERS_TABLE_MAX_ENTRIES_CNS,
        PRV_CPSS_DXCH_TRUNK_ENTRY_WORDS_COUNT_CNS,
        PRV_CPSS_DXCH_INDIRECT_ACCESS_E,
        &indirectAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_TRUNK_MEMBERS_E],
        PRV_CPSS_DXCH_INDIRECT_ACCESS_E,
        &indirectAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_TRUNK_MEMBERS_E]},

    /*CPSS_DXCH_TABLE_STATISTICAL_RATE_LIMIT_E*/
    {   PRV_CPSS_DXCH_RATE_LIMIT_TABLE_MAX_ENTRIES_CNS,
        PRV_CPSS_DXCH_RATE_LIMIT_ENTRY_WORDS_COUNT_CNS,
        PRV_CPSS_DXCH_INDIRECT_ACCESS_E,
        &indirectAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_STATISTICAL_RATE_LIMIT_E],
        PRV_CPSS_DXCH_INDIRECT_ACCESS_E,
        &indirectAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_STATISTICAL_RATE_LIMIT_E]},

    /*CPSS_DXCH_TABLE_CPU_CODE_E*/
    {   PRV_CPSS_DXCH_CPU_CODE_TABLE_MAX_ENTRIES_CNS,
        PRV_CPSS_DXCH_CPU_CODE_ENTRY_WORDS_COUNT_CNS,
        PRV_CPSS_DXCH_INDIRECT_ACCESS_E,
        &indirectAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_CPU_CODE_E],
        PRV_CPSS_DXCH_INDIRECT_ACCESS_E,
        &indirectAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_CPU_CODE_E]},

    /*CPSS_DXCH_TABLE_PCL_CONFIG_E*/     /*first lookup (00)*/
    {   4224,
        2,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_IPCL_CFG_E],
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_IPCL_CFG_E]},

    /*CPSS_DXCH_TABLE_QOS_PROFILE_E*/
    {   PRV_CPSS_DXCH2_QOS_PROFILE_NUM_MAX_CNS,
        PRV_CPSS_DXCH_QOS_PROFILE_ENTRY_WORDS_COUNT_CNS,
        PRV_CPSS_DXCH_INDIRECT_ACCESS_E,
        &indirectAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_QOS_PROFILE_E],
        PRV_CPSS_DXCH_INDIRECT_ACCESS_E,
        &indirectAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_QOS_PROFILE_E]},

    /* CPSS_DXCH_TABLE_REMARKING_E */
    {   0,
        0,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        NULL,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        NULL,
    },

    /* CPSS_DXCH_TABLE_STG_E */
    {   PRV_CPSS_DXCH_STG_MAX_NUM_CNS,
        PRV_CPSS_DXCH_STG_ENTRY_WORDS_SIZE_CNS,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_STG_E],
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_STG_E]},

    /* CPSS_DXCH_TABLE_VLAN_E */
    {   PRV_CPSS_DXCH_MAX_NUM_VLANS_CNS,
        PRV_CPSS_DXCHXCAT_VLAN_ENTRY_WORDS_COUNT_CNS,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_VLAN_E],
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_VLAN_E]},

    /*CPSS_DXCH_TABLE_MULTICAST_E*/
    {   _12K,
        PRV_CPSS_DXCH_MULTICAST_ENTRY_WORDS_COUNT_CNS,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_MULTICAST_E],
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_MULTICAST_E]},

    /*CPSS_DXCH_TABLE_ROUTE_HA_MAC_SA_E*/
    {   4160,
        PRV_CPSS_DXCH_ROUTE_HA_MAC_SA_ENTRY_WORDS_COUNT_CNS,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_ROUTE_HA_MAC_SA_E],
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_ROUTE_HA_MAC_SA_E]},

    /* CPSS_DXCH_TABLE_ROUTE_HA_ARP_DA_E */
    {   PRV_CPSS_DXCH_ROUTE_HA_ARP_DA_TABLE_MAX_ENTRIES_CNS,
        PRV_CPSS_DXCH2_ROUTE_HA_ARP_DA_ENTRY_WORDS_COUNT_CNS,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_ROUTE_HA_ARP_DA_E],
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_ROUTE_HA_ARP_DA_E]},

    /*CPSS_DXCH_TABLE_FDB_E*/
    {   _16K,
        PRV_CPSS_DXCH_FDB_MAC_ENTRY_WORDS_SIZE_CNS,
        PRV_CPSS_DXCH_INDIRECT_ACCESS_E,
        &indirectAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_FDB_E],
        PRV_CPSS_DXCH_INDIRECT_ACCESS_E,
        &indirectAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_FDB_E]},

    /*CPSS_DXCH_TABLE_POLICER_E*/
    {   0,
        0,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        NULL,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        NULL,
    },

    /* CPSS_DXCH_TABLE_POLICER_COUNTERS_E */
    {   0,
        0,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        NULL,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        NULL,
    },

    /* CPSS_DXCH2_TABLE_EGRESS_PCL_CONFIG_E */
    {   4160,
        1,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_EPCL_CFG_E],
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_EPCL_CFG_E]},

    /*CPSS_DXCH2_TABLE_TUNNEL_START_CONFIG_E*/
    {   PRV_CPSS_DXCH_ROUTE_HA_ARP_DA_TABLE_MAX_ENTRIES_CNS,
        PRV_CPSS_DXCH2_ROUTE_HA_ARP_DA_ENTRY_WORDS_COUNT_CNS,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_TUNNEL_START_CFG_E],
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_TUNNEL_START_CFG_E]},

    /* CPSS_DXCH2_TABLE_QOS_PROFILE_TO_ROUTE_BLOCK_E */
    {   PRV_CPSS_DXCH2_QOS_TO_ROUTE_BLOCK_TABLE_MAX_ENTRIES_CNS,
        PRV_CPSS_DXCH2_QOS_TO_ROUTE_BLOCK_WORDS_COUNT_CNS,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_QOS_PROFILE_TO_ROUTE_BLOCK_E],
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_QOS_PROFILE_TO_ROUTE_BLOCK_E]},

    /* CPSS_DXCH2_TABLE_ROUTE_ACCESS_MATRIX_E */
    {   PRV_CPSS_DXCH2_ROUTE_ACCESS_MATRIX_TABLE_MAX_ENTRIES_CNS,
        PRV_CPSS_DXCH2_ROUTE_ACCESS_MATRIX_WORDS_COUNT_CNS,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_ROUTE_ACCESS_MATRIX_E],
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_ROUTE_ACCESS_MATRIX_E]},

    /* CPSS_DXCH2_LTT_TT_ACTION_E */
    {   0,
        0,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        NULL,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        NULL,
    },

    /* CPSS_DXCH2_UC_MC_ROUTE_NEXT_HOP_E */
    {   PRV_CPSS_DXCH2_UC_MC_ROUTE_NEXT_HOP_TABLE_MAX_ENTRIES_CNS,
        PRV_CPSS_DXCH2_UC_MC_ROUTE_NEXT_HOP_WORDS_COUNT_CNS,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_UC_MC_ROUTE_NEXT_HOP_E],
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_UC_MC_ROUTE_NEXT_HOP_E]},

    /* CPSS_DXCH2_ROUTE_NEXT_HOP_AGE_E */
    {   64,
        PRV_CPSS_DXCH2_ROUTE_NEXT_HOP_AGE_COUNT_CNS,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_ROUTE_NEXT_HOP_AGE_E],
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_ROUTE_NEXT_HOP_AGE_E]},

    /*CPSS_DXCH3_TABLE_MAC2ME_E*/
    {   PRV_CPSS_DXCH3_MAC2ME_TABLE_MAX_ENTRIES_CNS,
        PRV_CPSS_DXCH3_MAC2ME_WORDS_COUNT_CNS,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_MAC2ME_E],
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_MAC2ME_E]},


    /*CPSS_DXCH3_TABLE_INGRESS_VLAN_TRANSLATION_E*/
    {   PRV_CPSS_DXCH3_VLAN_TRANSLATION_TABLE_MAX_ENTRIES_CNS,
        PRV_CPSS_DXCH3_VLAN_TRANSLATION_WORDS_COUNT_CNS,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_INGRESS_TRANSLATION_E],
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_INGRESS_TRANSLATION_E]},

    /*CPSS_DXCH3_TABLE_EGRESS_VLAN_TRANSLATION_E*/
    {   PRV_CPSS_DXCH3_VLAN_TRANSLATION_TABLE_MAX_ENTRIES_CNS,
        PRV_CPSS_DXCH3_VLAN_TRANSLATION_WORDS_COUNT_CNS,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_EGRESS_TRANSLATION_E],
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_EGRESS_TRANSLATION_E]},

    /*CPSS_DXCH3_TABLE_VRF_ID_E*/
    {   PRV_CPSS_DXCH3_VRF_ID_TABLE_MAX_ENTRIES_CNS,
        PRV_CPSS_DXCH3_VRF_ID_WORDS_COUNT_CNS,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_VRF_ID_E],
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_VRF_ID_E]},

    /*CPSS_DXCH3_LTT_TT_ACTION_E*/
    {   PRV_CPSS_DXCH3_LTT_TT_ACTION_TABLE_MAX_ENTRIES_CNS,
        PRV_CPSS_XCAT_LTT_TT_ACTION_WORDS_COUNT_CNS,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_LTT_TT_ACTION_E],
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_LTT_TT_ACTION_E]},

    /*CPSS_DXCH_XCAT_TABLE_INGRESS_PCL_LOOKUP1_CONFIG_E*//*third lookup(1)*/
    {   4224,
        2,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_IPCL_LOOKUP1_CFG_E],
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_IPCL_LOOKUP1_CFG_E]},

    /*CPSS_DXCH_XCAT_TABLE_INGRESS_PCL_UDB_CONFIG_E*/
    {   8,
        8,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_IPCL_UDB_CFG_E],
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_IPCL_UDB_CFG_E]},

    /*CPSS_DXCH_XCAT_TABLE_LOGICAL_TARGET_MAPPING_E*/
    {   2048,
        2,
        PRV_CPSS_DXCH_INDIRECT_ACCESS_E,
        &indirectAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_LOGICAL_TARGET_MAPPING_E],
        PRV_CPSS_DXCH_INDIRECT_ACCESS_E,
        &indirectAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_LOGICAL_TARGET_MAPPING_E]},

    /*CPSS_DXCH_XCAT_TABLE_BCN_PROFILES_E*/
    {   8,
        4,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_BCN_PROFILE_E],
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_BCN_PROFILE_E]},

    /*CPSS_DXCH_XCAT_TABLE_EGRESS_POLICER_REMARKING_E*/
    {   80,
        2,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_EGRESS_POLICER_REMARKING_E],
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_EGRESS_POLICER_REMARKING_E]},

    /*CPSS_DXCH_XCAT_TABLE_EQ_VLAN_MAPPING_E*/
    {   4096,
        1,
        PRV_CPSS_DXCH_INDIRECT_ACCESS_E,
        &indirectAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_VLAN_MAPPING_E],
        PRV_CPSS_DXCH_INDIRECT_ACCESS_E,
        &indirectAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_VLAN_MAPPING_E]},

    /*CPSS_DXCH_XCAT_TABLE_EQ_LP_EGRESS_VLAN_MEMBER_E*/
    {   2048,
        2,
        PRV_CPSS_DXCH_INDIRECT_ACCESS_E,
        &indirectAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_LP_EGRESS_VLAN_MEMBER_E],
        PRV_CPSS_DXCH_INDIRECT_ACCESS_E,
        &indirectAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_LP_EGRESS_VLAN_MEMBER_E]},

    /*CPSS_DXCH_XCAT_TABLE_MLL_L2MLL_VIDX_ENABLE_E*/
    {   128,
        1,
        PRV_CPSS_DXCH_INDIRECT_ACCESS_E,
        &indirectAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_L2MLL_VIDX_ENABLE_E],
        PRV_CPSS_DXCH_INDIRECT_ACCESS_E,
        &indirectAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_L2MLL_VIDX_ENABLE_E]},

    /*CPSS_DXCH_XCAT_TABLE_MLL_L2MLL_POINTER_MAPPING_E,*/
    {   1024,
        1,
        PRV_CPSS_DXCH_INDIRECT_ACCESS_E,
        &indirectAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_L2MLL_POINTER_MAPPING_E],
        PRV_CPSS_DXCH_INDIRECT_ACCESS_E,
        &indirectAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_L2MLL_POINTER_MAPPING_E]},

    /*CPSS_DXCH_XCAT_TABLE_INGRESS_ECID_E*/
    {   PRV_CPSS_DXCH_MAX_VLANS_NUM_CNS,
        1,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_INGRESS_ECID_E],
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_INGRESS_ECID_E]},

    /*CPSS_DXCH_XCAT_TABLE_MLL_E*/
    {   2048,
        4,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_MLL_E],
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_MLL_E]},

    /* CPSS_DXCH_XCAT_TABLE_MIB_COUNTERS_E */
    {   0,
        0,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        NULL,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        NULL,
    },

    /*CPSS_DXCH_LION_TABLE_PORT_ISOLATION_L2_E*/
    {   2176,
        1,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_PORT_ISOLATION_L2_E],
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_PORT_ISOLATION_L2_E]},

    /*CPSS_DXCH_LION_TABLE_PORT_ISOLATION_L3_E*/
    {   2176,
        1,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_PORT_ISOLATION_L3_E],
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_PORT_ISOLATION_L3_E]},

    /*CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_METERING_E*/
    {   2048,
        BITS_TO_WORDS_MAC(229),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_INGRESS_POLICER_METERING_0_E],
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_INGRESS_POLICER_METERING_0_E]},

    /*CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_METERING_E*/
    {   2048,
        BITS_TO_WORDS_MAC(229),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_INGRESS_POLICER_METERING_1_E],
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_INGRESS_POLICER_METERING_1_E]},

    /*CPSS_DXCH_SIP5_TABLE_EGRESS_POLICER_METERING_E*/
    {   512,
        BITS_TO_WORDS_MAC(229),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_EGRESS_POLICER_METERING_E],
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_EGRESS_POLICER_METERING_E]},

    /*CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_COUNTING_E*/
    {   2048,
        BITS_TO_WORDS_MAC(229),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_INGRESS_POLICER_COUNTING_0_E],
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_INGRESS_POLICER_COUNTING_0_E]},

    /*CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_COUNTING_E*/
    {   2048,
        BITS_TO_WORDS_MAC(229),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_INGRESS_POLICER_COUNTING_1_E],
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_INGRESS_POLICER_COUNTING_1_E]},

    /*CPSS_DXCH_SIP5_TABLE_EGRESS_POLICER_COUNTING_E*/
    {   512,
        BITS_TO_WORDS_MAC(229),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_EGRESS_POLICER_COUNTING_E],
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessXcat3TableInfo[PRV_DXCH_XCAT3_TABLE_EGRESS_POLICER_COUNTING_E]}

};

static const GT_U32 dxChXcat3TableInfoSize =
    (NUM_ELEMENTS_IN_ARR_MAC(dxChXcat3TablesInfo));

/* lion  2 table*/
static const PRV_CPSS_DXCH_TABLES_INFO_STC lion2TablesInfo[CPSS_DXCH_LION2_TABLE_LAST_E] =
{
    /* CPSS_DXCH_TABLE_VLAN_PORT_PROTOCOL_E */
    {_1KB , PRV_CPSS_DXCHXCAT_PRTCL_BASED_VLAN_ENTRY_WORDS_COUNT_CNS ,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E, &directAccessLion2TableInfoExt[PRV_DXCH_XCAT_TABLE_VLAN_PORT_PROTOCOL_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E, &directAccessLion2TableInfoExt[PRV_DXCH_XCAT_TABLE_VLAN_PORT_PROTOCOL_E].directAccessInfo},

    /* CPSS_DXCH_TABLE_PORT_VLAN_QOS_E */
    {PRV_CPSS_DXCH_PORT_TBLS_MAX_INDEX_CNS , 3 ,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E, &directAccessLion2TableInfoExt[PRV_DXCH_XCAT_TABLE_PORT_VLAN_QOS_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E, &directAccessLion2TableInfoExt[PRV_DXCH_XCAT_TABLE_PORT_VLAN_QOS_E].directAccessInfo},

    /* CPSS_DXCH_TABLE_TRUNK_MEMBERS_E */
    {PRV_CPSS_DXCH_TRUNK_MEMBERS_TABLE_MAX_ENTRIES_CNS, PRV_CPSS_DXCH_TRUNK_ENTRY_WORDS_COUNT_CNS,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E, directAccessTableInfo
                                           + CPSS_DXCH_TABLE_TRUNK_MEMBERS_E,
    PRV_CPSS_DXCH_DIRECT_ACCESS_E, directAccessTableInfo
                                          + CPSS_DXCH_TABLE_TRUNK_MEMBERS_E},

    /* CPSS_DXCH_TABLE_STATISTICAL_RATE_LIMIT_E */
    {PRV_CPSS_DXCH_RATE_LIMIT_TABLE_MAX_ENTRIES_CNS, PRV_CPSS_DXCH_RATE_LIMIT_ENTRY_WORDS_COUNT_CNS,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E, directAccessTableInfo +
                                    CPSS_DXCH_TABLE_STATISTICAL_RATE_LIMIT_E,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E, directAccessTableInfo +
                                   CPSS_DXCH_TABLE_STATISTICAL_RATE_LIMIT_E},

    /* CPSS_DXCH_TABLE_CPU_CODE_E */
    {PRV_CPSS_DXCH_CPU_CODE_TABLE_MAX_ENTRIES_CNS, PRV_CPSS_DXCH_CPU_CODE_ENTRY_WORDS_COUNT_CNS,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E, directAccessTableInfo +
                                                  CPSS_DXCH_TABLE_CPU_CODE_E,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E, directAccessTableInfo +
                                                 CPSS_DXCH_TABLE_CPU_CODE_E},

    /* CPSS_DXCH_TABLE_PCL_CONFIG_E */
    {4224, 2,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E, &directAccessLion2TableInfoExt[PRV_DXCH_XCAT_TABLE_IPCL_CFG_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E, &directAccessLion2TableInfoExt[PRV_DXCH_XCAT_TABLE_IPCL_CFG_E].directAccessInfo},

    /* CPSS_DXCH_TABLE_QOS_PROFILE_E */
    {PRV_CPSS_DXCH2_QOS_PROFILE_NUM_MAX_CNS, PRV_CPSS_DXCH_QOS_PROFILE_ENTRY_WORDS_COUNT_CNS,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E, directAccessTableInfo +
                                               CPSS_DXCH_TABLE_QOS_PROFILE_E,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E, directAccessTableInfo +
                                              CPSS_DXCH_TABLE_QOS_PROFILE_E},
    /* CPSS_DXCH_TABLE_REMARKING_E */
    {PRV_CPSS_DXCH2_QOS_PROFILE_NUM_MAX_CNS, PRV_CPSS_DXCH_QOS_PROFILE_ENTRY_WORDS_COUNT_CNS,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E, directAccessTableInfo +
                                                 CPSS_DXCH_TABLE_REMARKING_E,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E, directAccessTableInfo +
                                                CPSS_DXCH_TABLE_REMARKING_E},
    /* CPSS_DXCH_TABLE_STG_E */
    {0, 0,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E, &directAccessLion2TableInfoExt[PRV_DXCH_XCAT_TABLE_STG_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E, &directAccessLion2TableInfoExt[PRV_DXCH_XCAT_TABLE_STG_E].directAccessInfo},

    /* CPSS_DXCH_TABLE_VLAN_E */
    {0,0,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E, &directAccessLion2TableInfoExt[PRV_DXCH_XCAT_TABLE_VLAN_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E, &directAccessLion2TableInfoExt[PRV_DXCH_XCAT_TABLE_VLAN_E].directAccessInfo},

    /* CPSS_DXCH_TABLE_MULTICAST_E */
    {PRV_CPSS_DXCH_MAX_NUM_VLANS_CNS, 3,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E, &directAccessLion2TableInfoExt[PRV_DXCH_XCAT_TABLE_MULTICAST_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E, &directAccessLion2TableInfoExt[PRV_DXCH_XCAT_TABLE_MULTICAST_E].directAccessInfo},

    /* CPSS_DXCH_TABLE_ROUTE_HA_MAC_SA_E */
    {4160, PRV_CPSS_DXCH_ROUTE_HA_MAC_SA_ENTRY_WORDS_COUNT_CNS,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E, &directAccessLion2TableInfoExt[PRV_CPSS_XCAT_TABLE_ROUTE_HA_MAC_SA_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E, &directAccessLion2TableInfoExt[PRV_CPSS_XCAT_TABLE_ROUTE_HA_MAC_SA_E].directAccessInfo},

    /* CPSS_DXCH_TABLE_ROUTE_HA_ARP_DA_E */
    {PRV_CPSS_DXCH_ROUTE_HA_ARP_DA_TABLE_MAX_ENTRIES_CNS,
        PRV_CPSS_DXCH2_ROUTE_HA_ARP_DA_ENTRY_WORDS_COUNT_CNS,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E, &directAccessLion2TableInfoExt[PRV_CPSS_XCAT_TABLE_ROUTE_HA_ARP_DA_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E, &directAccessLion2TableInfoExt[PRV_CPSS_XCAT_TABLE_ROUTE_HA_ARP_DA_E].directAccessInfo},

    /* CPSS_DXCH_TABLE_FDB_E */
    {_64K , PRV_CPSS_DXCH_FDB_MAC_ENTRY_WORDS_SIZE_CNS,
        PRV_CPSS_DXCH_INDIRECT_ACCESS_E, indirectAccessTableInfo +
                                                       CPSS_DXCH_TABLE_FDB_E,
        PRV_CPSS_DXCH_INDIRECT_ACCESS_E, indirectAccessTableInfo +
                                                      CPSS_DXCH_TABLE_FDB_E},

    /* CPSS_DXCH_TABLE_POLICER_E - not supported for CH3.
        The dedicated algorithm is used for the table */
    {0, 0,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E, directAccessTableInfo +
                                                   CPSS_DXCH_TABLE_POLICER_E,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E, directAccessTableInfo +
                                                  CPSS_DXCH_TABLE_POLICER_E},

    /* CPSS_DXCH_TABLE_POLICER_COUNTERS_E  - not supported for CH3.
        The dedicated algorithm is used for the table */
    {0, 0,
        PRV_CPSS_DXCH_INDIRECT_ACCESS_E, directAccessTableInfo +
                                          CPSS_DXCH_TABLE_POLICER_COUNTERS_E,
        PRV_CPSS_DXCH_INDIRECT_ACCESS_E, directAccessTableInfo +
                                         CPSS_DXCH_TABLE_POLICER_COUNTERS_E},

    /* CPSS_DXCH2_TABLE_EGRESS_PCL_CONFIG_E */
    {4160, 1,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E, &directAccessLion2TableInfoExt[PRV_DXCH_XCAT_TABLE_EPCL_CFG_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E, &directAccessLion2TableInfoExt[PRV_DXCH_XCAT_TABLE_EPCL_CFG_E].directAccessInfo},

    /* CPSS_DXCH2_TABLE_TUNNEL_START_CONFIG_E */
    {PRV_CPSS_DXCH_ROUTE_HA_ARP_DA_TABLE_MAX_ENTRIES_CNS, 7,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E, &directAccessLion2TableInfoExt[PRV_DXCH_XCAT_TABLE_TUNNEL_START_CFG_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E, &directAccessLion2TableInfoExt[PRV_DXCH_XCAT_TABLE_TUNNEL_START_CFG_E].directAccessInfo},

    /* CPSS_DXCH2_TABLE_QOS_PROFILE_TO_ROUTE_BLOCK_E */
    {PRV_CPSS_DXCH2_QOS_TO_ROUTE_BLOCK_TABLE_MAX_ENTRIES_CNS,
                                         PRV_CPSS_DXCH2_QOS_TO_ROUTE_BLOCK_WORDS_COUNT_CNS,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E, directAccessTableInfo +
                               CPSS_DXCH2_TABLE_QOS_PROFILE_TO_ROUTE_BLOCK_E,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E, directAccessTableInfo +
                              CPSS_DXCH2_TABLE_QOS_PROFILE_TO_ROUTE_BLOCK_E},

    /* CPSS_DXCH2_TABLE_ROUTE_ACCESS_MATRIX_E */
    {PRV_CPSS_DXCH2_ROUTE_ACCESS_MATRIX_TABLE_MAX_ENTRIES_CNS,
                                        PRV_CPSS_DXCH2_ROUTE_ACCESS_MATRIX_WORDS_COUNT_CNS,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E, directAccessTableInfo +
                                      CPSS_DXCH2_TABLE_ROUTE_ACCESS_MATRIX_E,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E, directAccessTableInfo +
                                     CPSS_DXCH2_TABLE_ROUTE_ACCESS_MATRIX_E},

    /* CPSS_DXCH2_LTT_TT_ACTION_E */
    {0, 0, PRV_CPSS_DXCH_DIRECT_ACCESS_E, NULL,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E, NULL},

    /* CPSS_DXCH2_UC_MC_ROUTE_NEXT_HOP_E */
    {PRV_CPSS_DXCH2_UC_MC_ROUTE_NEXT_HOP_TABLE_MAX_ENTRIES_CNS,
                                       PRV_CPSS_DXCH2_UC_MC_ROUTE_NEXT_HOP_WORDS_COUNT_CNS,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E, directAccessTableInfo +
                                           CPSS_DXCH2_UC_MC_ROUTE_NEXT_HOP_E,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E, directAccessTableInfo +
                                          CPSS_DXCH2_UC_MC_ROUTE_NEXT_HOP_E},

     /* CPSS_DXCH2_ROUTE_NEXT_HOP_AGE_E */
    {64,
                                    PRV_CPSS_DXCH2_ROUTE_NEXT_HOP_AGE_COUNT_CNS,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E, directAccessTableInfo +
                                             CPSS_DXCH2_ROUTE_NEXT_HOP_AGE_E,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E, directAccessTableInfo +
                                             CPSS_DXCH2_ROUTE_NEXT_HOP_AGE_E},

    /*CPSS_DXCH3_TABLE_MAC2ME_E */
    {PRV_CPSS_DXCH3_MAC2ME_TABLE_MAX_ENTRIES_CNS,
                                    PRV_CPSS_DXCH3_MAC2ME_WORDS_COUNT_CNS,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E, &directAccessLion2TableInfoExt[PRV_CPSS_XCAT_TABLE_MAC2ME_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E, &directAccessLion2TableInfoExt[PRV_CPSS_XCAT_TABLE_MAC2ME_E].directAccessInfo},

     /* CPSS_DXCH3_TABLE_INGRESS_VLAN_TRANSLATION_E */
    {PRV_CPSS_DXCH3_VLAN_TRANSLATION_TABLE_MAX_ENTRIES_CNS,
                                    PRV_CPSS_DXCH3_VLAN_TRANSLATION_WORDS_COUNT_CNS,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E, &directAccessLion2TableInfoExt[PRV_DXCH_XCAT_TABLE_INGRESS_TRANSLATION_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E, &directAccessLion2TableInfoExt[PRV_DXCH_XCAT_TABLE_INGRESS_TRANSLATION_E].directAccessInfo},

     /* CPSS_DXCH3_TABLE_EGRESS_VLAN_TRANSLATION_E */
    {PRV_CPSS_DXCH3_VLAN_TRANSLATION_TABLE_MAX_ENTRIES_CNS,
                                    PRV_CPSS_DXCH3_VLAN_TRANSLATION_WORDS_COUNT_CNS,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E, &directAccessLion2TableInfoExt[PRV_DXCH_XCAT_TABLE_EGRESS_TRANSLATION_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E, &directAccessLion2TableInfoExt[PRV_DXCH_XCAT_TABLE_EGRESS_TRANSLATION_E].directAccessInfo},

    /* CPSS_DXCH3_TABLE_VRF_ID_E */
    {0, 0,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E, &directAccessLion2TableInfoExt[PRV_DXCH_XCAT_TABLE_VRF_ID_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E, &directAccessLion2TableInfoExt[PRV_DXCH_XCAT_TABLE_VRF_ID_E].directAccessInfo},

    /* CPSS_DXCH3_LTT_TT_ACTION_E */
    {PRV_CPSS_DXCH3_LTT_TT_ACTION_TABLE_MAX_ENTRIES_CNS, PRV_CPSS_XCAT_LTT_TT_ACTION_WORDS_COUNT_CNS,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E, &directAccessLion2TableInfoExt[PRV_DXCH_XCAT_LTT_TT_ACTION_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E, &directAccessLion2TableInfoExt[PRV_DXCH_XCAT_LTT_TT_ACTION_E].directAccessInfo},

    /* CPSS_DXCH_XCAT_TABLE_INGRESS_PCL_LOOKUP1_CONFIG_E */
    {4224, 2,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E, &directAccessLion2TableInfoExt[PRV_DXCH_XCAT_TABLE_IPCL_LOOKUP1_CFG_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E, &directAccessLion2TableInfoExt[PRV_DXCH_XCAT_TABLE_IPCL_LOOKUP1_CFG_E].directAccessInfo},

     /* CPSS_DXCH_XCAT_TABLE_INGRESS_PCL_UDB_CONFIG_E */
    {12, 8,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E, &directAccessLion2TableInfoExt[PRV_DXCH_XCAT_TABLE_IPCL_UDB_CFG_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E, &directAccessLion2TableInfoExt[PRV_DXCH_XCAT_TABLE_IPCL_UDB_CFG_E].directAccessInfo},

     /* CPSS_DXCH_XCAT_TABLE_LOGICAL_TARGET_MAPPING_E */
    {512, 1,
        PRV_CPSS_DXCH_INDIRECT_ACCESS_E, &xcatA1LogicalTargetMappingTblInfo,
        PRV_CPSS_DXCH_INDIRECT_ACCESS_E, &xcatA1LogicalTargetMappingTblInfo},

     /* CPSS_DXCH_XCAT_TABLE_BCN_PROFILES_E */
    {0, 0,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E, &directAccessLion2TableInfoExt[PRV_DXCH_XCAT_TABLE_BCN_PROFILE_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E, &directAccessLion2TableInfoExt[PRV_DXCH_XCAT_TABLE_BCN_PROFILE_E].directAccessInfo},

     /* CPSS_DXCH_XCAT_TABLE_EGRESS_POLICER_REMARKING_E */
    {80, 2,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E, &directAccessLion2TableInfoExt[PRV_DXCH_XCAT_TABLE_EGRESS_POLICER_REMARKING_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E, &directAccessLion2TableInfoExt[PRV_DXCH_XCAT_TABLE_EGRESS_POLICER_REMARKING_E].directAccessInfo},

     /* CPSS_DXCH_XCAT_TABLE_EQ_VLAN_MAPPING_E */
    {0, 0,
        PRV_CPSS_DXCH_INDIRECT_ACCESS_E, NULL,
        PRV_CPSS_DXCH_INDIRECT_ACCESS_E, NULL},

     /* CPSS_DXCH_XCAT_TABLE_EQ_LP_EGRESS_VLAN_MEMBER_E */
    {0, 0,
        PRV_CPSS_DXCH_INDIRECT_ACCESS_E, NULL,
        PRV_CPSS_DXCH_INDIRECT_ACCESS_E, NULL},

     /* CPSS_DXCH_XCAT_TABLE_MLL_L2MLL_VIDX_ENABLE_E */
    {0, 0,
        PRV_CPSS_DXCH_INDIRECT_ACCESS_E, NULL,
        PRV_CPSS_DXCH_INDIRECT_ACCESS_E, NULL},

     /* CPSS_DXCH_XCAT_TABLE_MLL_L2MLL_POINTER_MAPPING_E */
    {0, 0,
        PRV_CPSS_DXCH_INDIRECT_ACCESS_E, NULL,
        PRV_CPSS_DXCH_INDIRECT_ACCESS_E, NULL},

    /* CPSS_DXCH_XCAT_TABLE_INGRESS_ECID_E */
    {0, 0,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E, NULL,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E, NULL},

    /* CPSS_DXCH_XCAT_TABLE_MLL_E */
    {0, 0,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E, NULL,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E, NULL}

    /* in runtime the 'Extra tables' of the lion 2 will be copied here*/
};

/* lion 2 */
static const GT_U32 lion2TableInfoSize = CPSS_DXCH_LION2_TABLE_LAST_E;

/* support index 0..31 to represent 16 blocks from CNC0 and CNC1*/
#define BC2_CNC_TABLES_INFO_MAC(index) \
    /*CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + index */               \
    {   _1K,                                                         \
        BITS_TO_WORDS_MAC(64),                                       \
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,                               \
        &directAccessBobcat2TableInfo[PRV_CPSS_BOBCAT2_TABLE_CNC_0_COUNTERS_E + index].directAccessInfo, \
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,                                                                   \
        &directAccessBobcat2TableInfo[PRV_CPSS_BOBCAT2_TABLE_CNC_0_COUNTERS_E + index].directAccessInfo}


/* bobcat 2 table*/
static const PRV_CPSS_DXCH_TABLES_INFO_STC bobcat2TablesInfo[CPSS_DXCH_BOBCAT2_TABLE_LAST_E] =
{
   /*CPSS_DXCH_TABLE_VLAN_PORT_PROTOCOL_E*/
    {   _8K, /*PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.tti.numDefaultEports * 16;*//*512 * 16 = _8K (16 protocols * 512 default ePorts)*/
        1,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TTI_VLAN_PORT_PROTOCOL_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TTI_VLAN_PORT_PROTOCOL_E].directAccessInfo},

    /*CPSS_DXCH_TABLE_PORT_VLAN_QOS_E*/
    {   0,
        0,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        NULL,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        NULL},

    /*    CPSS_DXCH_TABLE_TRUNK_MEMBERS_E*/
    {   0,
        0,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        NULL,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        NULL},

    /*CPSS_DXCH_TABLE_STATISTICAL_RATE_LIMIT_E*/
    {   PRV_CPSS_DXCH_RATE_LIMIT_TABLE_MAX_ENTRIES_CNS,
        PRV_CPSS_DXCH_RATE_LIMIT_ENTRY_WORDS_COUNT_CNS,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EQ_STATISTICAL_RATE_LIMIT_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EQ_STATISTICAL_RATE_LIMIT_E].directAccessInfo},

    /*CPSS_DXCH_TABLE_CPU_CODE_E*/
    {    PRV_CPSS_DXCH_CPU_CODE_TABLE_MAX_ENTRIES_CNS,
        PRV_CPSS_DXCH_CPU_CODE_ENTRY_WORDS_COUNT_CNS,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EQ_CPU_CODE_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EQ_CPU_CODE_E].directAccessInfo},

    /*CPSS_DXCH_TABLE_PCL_CONFIG_E*/     /*first lookup (00)*/
    {   4352 ,
        BITS_TO_WORDS_MAC(SIP5_PCL_PCL_CONFIG_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_PCL_CONFIG_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_PCL_CONFIG_E].directAccessInfo},

    /*CPSS_DXCH_TABLE_QOS_PROFILE_E*/
    {   _1K,
        PRV_CPSS_DXCH_QOS_PROFILE_ENTRY_WORDS_COUNT_CNS,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EQ_QOS_PROFILE_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EQ_QOS_PROFILE_E].directAccessInfo},

    /* CPSS_DXCH_TABLE_REMARKING_E */
    {   0,
        0,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        NULL,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        NULL},

    /* CPSS_DXCH_TABLE_STG_E */
    {   0,
        0,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        NULL,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        NULL},

    /* CPSS_DXCH_TABLE_VLAN_E */
    {   0,
        0,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        NULL,
        PRV_CPSS_DXCH_INDIRECT_ACCESS_E,
        NULL},/*indirect*/

    /*CPSS_DXCH_TABLE_MULTICAST_E*/
    {   _4K,
        BITS_TO_WORDS_MAC(BMP_256_PORTS_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EGF_SHT_MULTICAST_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EGF_SHT_MULTICAST_E].directAccessInfo},

    /*CPSS_DXCH_TABLE_ROUTE_HA_MAC_SA_E*/
    {   _4K,
        BITS_TO_WORDS_MAC(SIP5_HA_EGRESS_VLAN_MAC_SA_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_HA_ROUTE_MAC_SA_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_HA_ROUTE_MAC_SA_E].directAccessInfo},

    /* CPSS_DXCH_TABLE_ROUTE_HA_ARP_DA_E */
    {   0,
        0,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        NULL,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        NULL},

    /*CPSS_DXCH_TABLE_FDB_E*/
    {   _256K ,
        BITS_TO_WORDS_MAC(SIP5_FDB_FDB_BITS_SIZE_CNS) ,
        PRV_CPSS_DXCH_INDIRECT_ACCESS_E,
        indirectAccessTableInfo_sip_5 + PRV_CPSS_SIP5_FDB_TABLE_INDIRECT_E,
        PRV_CPSS_DXCH_INDIRECT_ACCESS_E,
        indirectAccessTableInfo_sip_5 + PRV_CPSS_SIP5_FDB_TABLE_INDIRECT_E},/*indirect*/

    /*CPSS_DXCH_TABLE_POLICER_E*/
    {   0,
        0,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        NULL,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        NULL},

    /* CPSS_DXCH_TABLE_POLICER_COUNTERS_E */
    {   0,
        0,
        PRV_CPSS_DXCH_INDIRECT_ACCESS_E,
        NULL,
        PRV_CPSS_DXCH_INDIRECT_ACCESS_E,
        NULL},/*indirect*/

    /* CPSS_DXCH2_TABLE_EGRESS_PCL_CONFIG_E */
    {    4352,
        BITS_TO_WORDS_MAC(SIP5_EPCL_PCL_CONFIG_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EPCL_EGRESS_PCL_CONFIG_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EPCL_EGRESS_PCL_CONFIG_E].directAccessInfo},

    /*CPSS_DXCH2_TABLE_TUNNEL_START_CONFIG_E*/
    {   _16K,
        BITS_TO_WORDS_MAC(SIP5_HA_TUNNEL_START_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_HA_TUNNEL_START_CONFIG_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_HA_TUNNEL_START_CONFIG_E].directAccessInfo},

    /* CPSS_DXCH2_TABLE_QOS_PROFILE_TO_ROUTE_BLOCK_E */
    {   0,
        0,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        NULL,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        NULL},

    /* CPSS_DXCH2_TABLE_ROUTE_ACCESS_MATRIX_E */
    {   0,
        0,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        NULL,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        NULL},

    /* CPSS_DXCH2_LTT_TT_ACTION_E */
    {   0,
        0,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        NULL,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        NULL},

    /* CPSS_DXCH2_UC_MC_ROUTE_NEXT_HOP_E */
    {    0,
        0,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        NULL,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        NULL},

     /* CPSS_DXCH2_ROUTE_NEXT_HOP_AGE_E */
    {    0,
        0,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        NULL,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        NULL},

    /*CPSS_DXCH3_TABLE_MAC2ME_E*/
    {   128,
        6, /* support 6 words */
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TTI_MAC2ME_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TTI_MAC2ME_E].directAccessInfo},

    /*CPSS_DXCH3_TABLE_INGRESS_VLAN_TRANSLATION_E*/
    {   PRV_CPSS_DXCH3_VLAN_TRANSLATION_TABLE_MAX_ENTRIES_CNS,
        PRV_CPSS_DXCH3_VLAN_TRANSLATION_WORDS_COUNT_CNS,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TTI_INGRESS_VLAN_TRANSLATION_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TTI_INGRESS_VLAN_TRANSLATION_E].directAccessInfo},

    /*CPSS_DXCH3_TABLE_EGRESS_VLAN_TRANSLATION_E*/
    {   _8K,
        BITS_TO_WORDS_MAC(SIP5_HA_EGRESS_VLAN_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_HA_EGRESS_VLAN_TRANSLATION_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_HA_EGRESS_VLAN_TRANSLATION_E].directAccessInfo},

    /*CPSS_DXCH3_TABLE_VRF_ID_E*/
    {   0,
        0,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        NULL,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        NULL},

    /*CPSS_DXCH3_LTT_TT_ACTION_E*/
    {   0,
        0,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        NULL,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        NULL},

    /*CPSS_DXCH_XCAT_TABLE_INGRESS_PCL_LOOKUP1_CONFIG_E*//*third lookup(1)*/
    {   4352,
        BITS_TO_WORDS_MAC(SIP5_PCL_PCL_CONFIG_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_PCL_INGRESS_LOOKUP1_CONFIG_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_PCL_INGRESS_LOOKUP1_CONFIG_E].directAccessInfo},

    /*CPSS_DXCH_XCAT_TABLE_INGRESS_PCL_UDB_CONFIG_E*/
    {   16,
        BITS_TO_WORDS_MAC(SIP5_TTI_PCL_UDB_BITS_SIZE_CNS),/* changed from 8 words to 18 words */
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TTI_INGRESS_PCL_UDB_CONFIG_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TTI_INGRESS_PCL_UDB_CONFIG_E].directAccessInfo},

    /*CPSS_DXCH_XCAT_TABLE_LOGICAL_TARGET_MAPPING_E*/
    {   0,
        0,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        NULL,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        NULL},

    /*CPSS_DXCH_XCAT_TABLE_BCN_PROFILES_E*/
    {   0,
        0,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        NULL,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        NULL},

    /*CPSS_DXCH_XCAT_TABLE_EGRESS_POLICER_REMARKING_E*/
    {   80,/* iplr hold 1024 but EPLR hold only 80 entries ! */
        BITS_TO_WORDS_MAC(SIP5_IPVX_POLICER_REMARKING_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EPLR_EGRESS_POLICER_REMARKING_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EPLR_EGRESS_POLICER_REMARKING_E].directAccessInfo},

    /*CPSS_DXCH_XCAT_TABLE_EQ_VLAN_MAPPING_E*/
    {   0,
        0,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        NULL,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        NULL},

    /*CPSS_DXCH_XCAT_TABLE_EQ_LP_EGRESS_VLAN_MEMBER_E*/
    {   0,
        0,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        NULL,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        NULL},

    /*PRV_CPSS_DXCH_XCAT_MLL_L2MLL_VIDX_ENABLE_E*/
    {   0,
        0,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        NULL,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        NULL},

    /*PRV_CPSS_DXCH_XCAT_MLL_L2MLL_POINTER_MAPPING_E*/
    {   0,
        0,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        NULL,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        NULL},

    /*PRV_CPSS_DXCH_XCAT_INGRESS_ECID_E*/
    {   0,
        0,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        NULL,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        NULL},

    /*PRV_CPSS_DXCH_XCAT_INGRESS_MLL_E*/
    {   0,
        0,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        NULL,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        NULL},

    /* CPSS_DXCH_XCAT_TABLE_MIB_COUNTERS_E */
    {   0,
        0,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        NULL,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        NULL},

    /*CPSS_DXCH_LION_TABLE_PORT_ISOLATION_L2_E*/
    {   2176,
        BITS_TO_WORDS_MAC(BMP_256_PORTS_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EGF_SHT_PORT_ISOLATION_L2_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EGF_SHT_PORT_ISOLATION_L2_E].directAccessInfo},

    /*CPSS_DXCH_LION_TABLE_PORT_ISOLATION_L3_E*/
    {   2176,
        BITS_TO_WORDS_MAC(BMP_256_PORTS_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EGF_SHT_PORT_ISOLATION_L3_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EGF_SHT_PORT_ISOLATION_L3_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_METERING_E*/
    {   _8K,
        BITS_TO_WORDS_MAC(SIP5_IPVX_POLICER_METERING_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_IPLR_INGRESS_POLICER_0_METERING_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_IPLR_INGRESS_POLICER_0_METERING_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_METERING_E*/
    {   _8K,
        BITS_TO_WORDS_MAC(SIP5_IPVX_POLICER_METERING_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_IPLR_1_INGRESS_POLICER_1_METERING_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_IPLR_1_INGRESS_POLICER_1_METERING_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_EGRESS_POLICER_METERING_E*/
    {   _8K,
        BITS_TO_WORDS_MAC(SIP5_IPVX_POLICER_METERING_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EPLR_EGRESS_POLICER_METERING_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EPLR_EGRESS_POLICER_METERING_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_COUNTING_E*/
    {   _8K,
        BITS_TO_WORDS_MAC(SIP5_IPVX_POLICER_COUNTING_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_CPSS_BOBCAT2_TABLE_INGRESS_POLICER_0_COUNTING_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_CPSS_BOBCAT2_TABLE_INGRESS_POLICER_0_COUNTING_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_COUNTING_E*/
    {   _8K,
        BITS_TO_WORDS_MAC(SIP5_IPVX_POLICER_COUNTING_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_CPSS_BOBCAT2_TABLE_INGRESS_POLICER_1_COUNTING_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_CPSS_BOBCAT2_TABLE_INGRESS_POLICER_1_COUNTING_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_EGRESS_POLICER_COUNTING_E*/
    {   _8K,
        BITS_TO_WORDS_MAC(SIP5_IPVX_POLICER_COUNTING_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_CPSS_BOBCAT2_TABLE_EGRESS_POLICER_COUNTING_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_CPSS_BOBCAT2_TABLE_EGRESS_POLICER_COUNTING_E].directAccessInfo},

    /*CPSS_DXCH_XCAT2_TABLE_INGRESS_PCL_LOOKUP01_CONFIG_E*/
    {   4352,
        BITS_TO_WORDS_MAC(SIP5_PCL_PCL_CONFIG_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_PCL_INGRESS_LOOKUP01_CONFIG_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_PCL_INGRESS_LOOKUP01_CONFIG_E].directAccessInfo},

    /*CPSS_DXCH_LION_TABLE_TRUNK_HASH_MASK_CRC_E*/
    {   (BIT_4 + SIP5_PCL_NUM_FLOWS_CNS), /* num entries = 4 bit index + 16 'flows' */
        BITS_TO_WORDS_MAC(SIP5_PCL_TRUNK_HASH_MASK_CRC_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_PCL_TRUNK_HASH_MASK_CRC_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_PCL_TRUNK_HASH_MASK_CRC_E].directAccessInfo},

    /*CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E*/
    {   _8K,
        BITS_TO_WORDS_MAC(SIP5_L2I_INGRESS_VLAN_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_L2I_VLAN_INGRESS_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_L2I_VLAN_INGRESS_E].directAccessInfo},

    /*CPSS_DXCH_LION_TABLE_VLAN_EGRESS_E*/
    {   _8K,
        BITS_TO_WORDS_MAC(SIP5_EGF_SHT_EGRESS_VLAN_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EGF_SHT_VLAN_EGRESS_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EGF_SHT_VLAN_EGRESS_E].directAccessInfo},

    /*CPSS_DXCH_LION_TABLE_STG_INGRESS_E*/
    {   _4K,
        BITS_TO_WORDS_MAC(SIP5_L2I_INGRESS_STP_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_L2I_STG_INGRESS_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_L2I_STG_INGRESS_E].directAccessInfo},

    /*CPSS_DXCH_LION_TABLE_STG_EGRESS_E*/
    {   _4K,
        BITS_TO_WORDS_MAC(BMP_256_PORTS_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EGF_SHT_STG_EGRESS_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EGF_SHT_STG_EGRESS_E].directAccessInfo},

    /*CPSS_DXCH_LION_TABLE_TXQ_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG_E*/
    {   SIP5_SINGLE_PORT_GROUP_TXQ_DQ_PORTS_NUM_CNS,
        BITS_TO_WORDS_MAC(SIP5_PRIO_TOKEN_BUCKET_CONFIGURATION_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TXQ_DQ_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TXQ_DQ_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG_E].directAccessInfo},

    /*CPSS_DXCH_LION_TABLE_TXQ_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG_E*/
    {   SIP5_SINGLE_PORT_GROUP_TXQ_DQ_PORTS_NUM_CNS,
        BITS_TO_WORDS_MAC(SIP5_PORT_TOKEN_BUCKET_CONFIGURATION_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TXQ_DQ_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TXQ_DQ_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG_E].directAccessInfo},

    /*CPSS_DXCH_LION_TABLE_TXQ_SOURCE_ID_MEMBERS_E*/
    {   _4K,
        BITS_TO_WORDS_MAC(BMP_256_PORTS_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EGF_SHT_TXQ_SOURCE_ID_MEMBERS_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EGF_SHT_TXQ_SOURCE_ID_MEMBERS_E].directAccessInfo},

    /*CPSS_DXCH_LION_TABLE_TXQ_NON_TRUNK_MEMBERS_E*/
    {   256,
        BITS_TO_WORDS_MAC(BMP_256_PORTS_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EGF_SHT_TXQ_NON_TRUNK_MEMBERS_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EGF_SHT_TXQ_NON_TRUNK_MEMBERS_E].directAccessInfo},

    /*CPSS_DXCH_LION_TABLE_TXQ_DESIGNATED_PORT_E*/
    {   128,
        BITS_TO_WORDS_MAC(BMP_256_PORTS_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EGF_SHT_TXQ_DESIGNATED_PORT_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EGF_SHT_TXQ_DESIGNATED_PORT_E].directAccessInfo},

    /*CPSS_DXCH_LION_TABLE_TXQ_EGRESS_STC_E*/
    {   SIP5_SINGLE_PORT_GROUP_TXQ_DQ_PORTS_NUM_CNS,
        3,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TXQ_DQ_EGRESS_STC_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TXQ_DQ_EGRESS_STC_E].directAccessInfo},

    /*CPSS_DXCH_LION_TABLE_ROUTER_VLAN_URPF_STC_E*/
    {   0,
        0,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        NULL,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        NULL},

    /*CPSS_DXCH_LION_TABLE_LAST_E*/
    {   0,
        0,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        NULL,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        NULL},

    /*CPSS_DXCH_SIP5_TABLE_TTI_PHYSICAL_PORT_ATTRIBUTE_E*/
    {   256,
        BITS_TO_WORDS_MAC(SIP5_TTI_PHYSICAL_PORT_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TTI_PHYSICAL_PORT_ATTRIBUTE_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TTI_PHYSICAL_PORT_ATTRIBUTE_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E*/
    {   512,
        BITS_TO_WORDS_MAC(SIP5_TTI_DEFAULT_PORT_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TTI_PRE_LOOKUP_INGRESS_EPORT_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TTI_PRE_LOOKUP_INGRESS_EPORT_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_POST_TTI_LOOKUP_INGRESS_EPORT_E*/
    {  _8K,
        1,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TTI_POST_LOOKUP_INGRESS_EPORT_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TTI_POST_LOOKUP_INGRESS_EPORT_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_BRIDGE_INGRESS_EPORT_E*/
    {   _8K,
        BITS_TO_WORDS_MAC(SIP5_L2I_EPORT_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_L2I_BRIDGE_INGRESS_EPORT_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_L2I_BRIDGE_INGRESS_EPORT_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_BRIDGE_INGRESS_EPORT_LEARN_PRIO_E*/
    {   _1K,
        BITS_TO_WORDS_MAC(SIP5_L2I_EPORT_LEARN_PRIO_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_L2I_BRIDGE_INGRESS_EPORT_LEARN_PRIO_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_L2I_BRIDGE_INGRESS_EPORT_LEARN_PRIO_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_BRIDGE_SOURCE_TRUNK_ATTRIBUTE_E*/
    {   512,
        BITS_TO_WORDS_MAC(SIP5_L2I_SOURCE_TRUNK_ATTRIBUTE_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_L2I_BRIDGE_SOURCE_TRUNK_ATTRIBUTE_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_L2I_BRIDGE_SOURCE_TRUNK_ATTRIBUTE_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_BRIDGE_PHYSICAL_PORT_E*/
    {   256,
        BITS_TO_WORDS_MAC(SIP5_L2I_PHYSICAL_PORT_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_L2I_BRIDGE_PHYSICAL_PORT_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_L2I_BRIDGE_PHYSICAL_PORT_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_BRIDGE_PHYSICAL_PORT_RATE_LIMIT_COUNTERS_E*/
    {   256,
        BITS_TO_WORDS_MAC(SIP5_L2I_PHYSICAL_PORT_RATE_LIMIT_COUNTERS_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_L2I_BRIDGE_PHYSICAL_PORT_RATE_LIMIT_COUNTERS_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_L2I_BRIDGE_PHYSICAL_PORT_RATE_LIMIT_COUNTERS_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_EQ_INGRESS_EPORT_E*/
    {   _2K,/* 4 eports per entry */
        1,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EQ_INGRESS_EPORT_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EQ_INGRESS_EPORT_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_EQ_INGRESS_STC_PHYSICAL_PORT_E*/
    {   256,
        BITS_TO_WORDS_MAC(SIP5_EQ_INGRESS_STC_PHYSICAL_PORT_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EQ_INGRESS_STC_PHYSICAL_PORT_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EQ_INGRESS_STC_PHYSICAL_PORT_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_EQ_INGRESS_MIRROR_PHYSICAL_PORT_E*/
    {   32,
        1,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EQ_INGRESS_MIRROR_PHYSICAL_PORT_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EQ_INGRESS_MIRROR_PHYSICAL_PORT_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_EQ_TRUNK_LTT_E*/
    {   _4K,
        1,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EQ_TRUNK_LTT_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EQ_TRUNK_LTT_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_EQ_L2_ECMP_LTT_E*/
    {   _8K,
        1,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EQ_L2_ECMP_LTT_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EQ_L2_ECMP_LTT_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_EQ_L2_ECMP_E*/
    {   _8K,
        1,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EQ_L2_ECMP_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EQ_L2_ECMP_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_EQ_EPORT_TO_PHYSICAL_PORT_TARGET_MAPPING_E*/
    {   _8K,
        1,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EQ_EPORT_TO_PHYSICAL_PORT_TARGET_MAPPING_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EQ_EPORT_TO_PHYSICAL_PORT_TARGET_MAPPING_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_EQ_TO_CPU_RATE_LIMITER_CONFIG_E*/
    {   256,
        1,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EQ_TO_CPU_RATE_LIMITER_CONFIG_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EQ_TO_CPU_RATE_LIMITER_CONFIG_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_EQ_TX_PROTECTION_E*/
    {   256,
        1,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EQ_TX_PROTECTION_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EQ_TX_PROTECTION_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_EQ_EPORT_TO_LOC_MAPPING_E*/
    {   _4K,
        1,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EQ_EPORT_TO_LOC_MAPPING_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EQ_EPORT_TO_LOC_MAPPING_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_EQ_PROTECTION_LOC_E*/
    {   64,
        1,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EQ_PROTECTION_LOC_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EQ_PROTECTION_LOC_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_LPM_MEM_E*/
    {   20*_16K,/* 20 consecutive memories of 16K */
        1,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_LPM_MEM_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_LPM_MEM_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_IPV4_VRF_E*/
    {   _4K,
        1,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_LPM_IPV4_VRF_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_LPM_IPV4_VRF_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_IPV6_VRF_E*/
    {   _4K,
        1,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_LPM_IPV6_VRF_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_LPM_IPV6_VRF_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_FCOE_VRF_E*/
    {   _4K,
        1,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_LPM_FCOE_VRF_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_LPM_FCOE_VRF_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_ECMP_E*/
    {   12 * _1K,
        1,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_LPM_ECMP_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_LPM_ECMP_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_EFT_FAST_STACK_FAILOVER_SECONDARY_TARGET_PORT_E*/
    {   256,
        1,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EGF_EFT_EGRESS_FAST_STACK_FAILOVER_SECONDARY_TARGET_PORT_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EGF_EFT_EGRESS_FAST_STACK_FAILOVER_SECONDARY_TARGET_PORT_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_EGRESS_EPORT_E*/
    {   _8K,
        BITS_TO_WORDS_MAC(SIP5_EGF_QAG_EGRESS_EPORT_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EGF_QAG_EGRESS_EGRESS_EPORT_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EGF_QAG_EGRESS_EGRESS_EPORT_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_EGRESS_VLAN_E*/
    {   _8K,
        BITS_TO_WORDS_MAC(SIP5_EGF_QAG_EGRESS_EVLAN_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EGF_QAG_EGRESS_EGRESS_VLAN_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EGF_QAG_EGRESS_EGRESS_VLAN_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_E*/
    {   256,
        BITS_TO_WORDS_MAC(SIP5_EGF_QAG_TARGET_PORT_MAPPER_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EGF_QAG_EGRESS_TARGET_PORT_MAPPER_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EGF_QAG_EGRESS_TARGET_PORT_MAPPER_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_CPU_CODE_TO_LOOPBACK_MAPPER_E*/
    {   256,
        BITS_TO_WORDS_MAC(SIP5_EGF_QAG_CPU_CODE_TO_LOOPBACK_MAPPER_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EGF_QAG_EGRESS_CPU_CODE_TO_LOOPBACK_MAPPER_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EGF_QAG_EGRESS_CPU_CODE_TO_LOOPBACK_MAPPER_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_FWD_FROM_CPU_TO_LOOPBACK_MAPPER_E*/
    {   256,
        BITS_TO_WORDS_MAC(SIP5_EGF_QAG_FWD_FROM_CPU_TO_LOOPBACK_MAPPER_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EGF_QAG_EGRESS_FWD_FROM_CPU_TO_LOOPBACK_MAPPER_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EGF_QAG_EGRESS_FWD_FROM_CPU_TO_LOOPBACK_MAPPER_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_EGRESS_EPORT_E*/
    {   _8K,
        BITS_TO_WORDS_MAC(SIP5_EGF_SHT_EGRESS_EPORT_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EGF_SHT_EGRESS_EGRESS_EPORT_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EGF_SHT_EGRESS_EGRESS_EPORT_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_DEVICE_MAP_TABLE_E*/
    {   _4K,
        1,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EGF_SHT_EGRESS_DEVICE_MAP_TABLE_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EGF_SHT_EGRESS_DEVICE_MAP_TABLE_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E*/
    {   _8K,
        BITS_TO_WORDS_MAC(SIP5_HA_EGRESS_EPORT_1_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_HA_EGRESS_EPORT_1_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_HA_EGRESS_EPORT_1_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_2_E*/
    {   _8K,
        BITS_TO_WORDS_MAC(SIP5_HA_EGRESS_EPORT_2_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_HA_EGRESS_EPORT_2_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_HA_EGRESS_EPORT_2_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_1_E*/
    {   256,
        BITS_TO_WORDS_MAC(SIP5_HA_EGRESS_PHYSICAL_PORT_1_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_HA_PHYSICAL_PORT_1_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_HA_PHYSICAL_PORT_1_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E*/
    {   256,
        BITS_TO_WORDS_MAC(SIP5_HA_EGRESS_PHYSICAL_PORT_2_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_HA_PHYSICAL_PORT_2_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_HA_PHYSICAL_PORT_2_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_HA_QOS_PROFILE_TO_EXP_E*/
    {   _1K,
        BITS_TO_WORDS_MAC(SIP5_HA_QOS_PROFILE_TO_EXP_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_HA_QOS_PROFILE_TO_EXP_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_HA_QOS_PROFILE_TO_EXP_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_HA_EPCL_UDB_CONFIG_E*/
    {   16,
        BITS_TO_WORDS_MAC(SIP5_HA_EPCL_UDB_CONFIG_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_HA_EPCL_UDB_CONFIG_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_HA_EPCL_UDB_CONFIG_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_HA_PTP_DOMAIN_E*/
    {   1280,
        BITS_TO_WORDS_MAC(SIP5_HA_PTP_DOMAIN_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_HA_PTP_DOMAIN_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_HA_PTP_DOMAIN_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_HA_GENERIC_TS_PROFILE_E*/
    {   8,
        BITS_TO_WORDS_MAC(SIP5_HA_GEN_TS_PROFILE_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_HA_GENERIC_TS_PROFILE_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_HA_GENERIC_TS_PROFILE_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_HA_GLOBAL_MAC_SA_E*/
    {   256,
        BITS_TO_WORDS_MAC(SIP5_HA_GLOBAL_MAC_SA_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_HA_GLOBAL_MAC_SA_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_HA_GLOBAL_MAC_SA_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_IP_MLL_E*/
    {   _16K,
        BITS_TO_WORDS_MAC(SIP5_MLL_ENTRY_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_MLL_IP_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_MLL_IP_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_L2_MLL_E*/
    {   _16K,
        BITS_TO_WORDS_MAC(SIP5_MLL_ENTRY_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_MLL_L2_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_MLL_L2_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_L2_MLL_LTT_E*/
    {   _32K,
        BITS_TO_WORDS_MAC(SIP5_L2_MLL_LTT_ENTRY_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_MLL_L2_LTT_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_MLL_L2_LTT_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_ADJACENCY_E*/
    {   PRV_CPSS_DXCH_MAX_NUM_ADJACENCY_CNS,
        3,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TTI_ADJACENCY_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TTI_ADJACENCY_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_E_ATTRIBUTES_E*/
    {   _8K,
        BITS_TO_WORDS_MAC(SIP5_IPVX_POLICER_E_ATTRIBUTES_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_IPLR_INGRESS_POLICER_0_E_ATTRIBUTES_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_IPLR_INGRESS_POLICER_0_E_ATTRIBUTES_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_REMARKING_E*/
    {   _1K,
        BITS_TO_WORDS_MAC(SIP5_IPVX_POLICER_REMARKING_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_IPLR_INGRESS_POLICER_0_REMARKING_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_IPLR_INGRESS_POLICER_0_REMARKING_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_HIERARCHICAL_E*/
    {   _8K,
        BITS_TO_WORDS_MAC(SIP5_IPVX_POLICER_HIERARCHICAL_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_IPLR_INGRESS_POLICER_0_HIERARCHICAL_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_IPLR_INGRESS_POLICER_0_HIERARCHICAL_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_E_ATTRIBUTES_E*/
    {   _8K,
        BITS_TO_WORDS_MAC(SIP5_IPVX_POLICER_E_ATTRIBUTES_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_IPLR_1_INGRESS_POLICER_1_E_ATTRIBUTES_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_IPLR_1_INGRESS_POLICER_1_E_ATTRIBUTES_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_REMARKING_E*/
    {   _1K,
        BITS_TO_WORDS_MAC(SIP5_IPVX_POLICER_REMARKING_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_IPLR_1_INGRESS_POLICER_1_REMARKING_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_IPLR_1_INGRESS_POLICER_1_REMARKING_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_EGRESS_POLICER_E_ATTRIBUTES_E*/
    {   _8K,
        BITS_TO_WORDS_MAC(SIP5_IPVX_POLICER_E_ATTRIBUTES_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EPLR_EGRESS_POLICER_E_ATTRIBUTES_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EPLR_EGRESS_POLICER_E_ATTRIBUTES_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_IPVX_INGRESS_EPORT_E*/
    {   512,
        BITS_TO_WORDS_MAC(SIP5_IPVX_ROUTER_EPORT_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_IPVX_INGRESS_EPORT_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_IPVX_INGRESS_EPORT_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_IPVX_EVLAN_E*/
    {   _8K,
        BITS_TO_WORDS_MAC(SIP5_IPVX_ROUTER_EVLAN_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_IPVX_EVLAN_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_IPVX_EVLAN_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_IPVX_ROUTER_NEXTHOP_E*/
    {   _24K,
        BITS_TO_WORDS_MAC(SIP5_IPVX_ROUTER_NEXTHOP_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_IPVX_ROUTER_NEXTHOP_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_IPVX_ROUTER_NEXTHOP_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_IPVX_ROUTER_NEXTHOP_AGE_BITS_E*/
    {   768,
        BITS_TO_WORDS_MAC(SIP5_IPVX_ROUTER_NEXTHOP_AGE_BITS_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_IPVX_ROUTER_NEXTHOP_AGE_BITS_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_IPVX_ROUTER_NEXTHOP_AGE_BITS_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_IPVX_ROUTER_ACCESS_MATRIX_E*/
    {   192,
        BITS_TO_WORDS_MAC(SIP5_IPVX_ROUTER_ACCESS_MATRIX_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_IPVX_ROUTER_ACCESS_MATRIX_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_IPVX_ROUTER_ACCESS_MATRIX_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_IPVX_ROUTER_QOS_PROFILE_OFFSETS_E*/
    {   256,
        BITS_TO_WORDS_MAC(SIP5_IPVX_ROUTER_QOS_PROFILE_OFFSETS_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_IPVX_ROUTER_QOS_PROFILE_OFFSETS_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_IPVX_ROUTER_QOS_PROFILE_OFFSETS_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_E*/
    {   _2K,
        3,
        PRV_CPSS_DXCH_INDIRECT_ACCESS_E,
        indirectAccessTableInfo_sip_5 + PRV_CPSS_SIP5_IOAM_TABLE_INDIRECT_E,
        PRV_CPSS_DXCH_INDIRECT_ACCESS_E,
        indirectAccessTableInfo_sip_5 + PRV_CPSS_SIP5_IOAM_TABLE_INDIRECT_E},/*indirect*/

    /*CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_KEEPALIVE_AGING_E*/
    {   64,
        1,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_IOAM_INGRESS_OAM_KEEPALIVE_AGING_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_IOAM_INGRESS_OAM_KEEPALIVE_AGING_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_MEG_EXCEPTION_E*/
    {   64,
        1,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_IOAM_INGRESS_OAM_MEG_EXCEPTION_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_IOAM_INGRESS_OAM_MEG_EXCEPTION_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_SOURCE_INTERFACE_EXCEPTION_E*/
    {   64,
        1,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_IOAM_INGRESS_OAM_SOURCE_INTERFACE_EXCEPTION_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_IOAM_INGRESS_OAM_SOURCE_INTERFACE_EXCEPTION_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_INVALID_KEEPALIVE_HASH_E*/
    {   64,
        1,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_IOAM_INGRESS_OAM_INVALID_KEEPALIVE_HASH_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_IOAM_INGRESS_OAM_INVALID_KEEPALIVE_HASH_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_KEEPALIVE_EXCESS_E*/
    {   64,
        1,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_IOAM_INGRESS_OAM_KEEPALIVE_EXCESS_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_IOAM_INGRESS_OAM_KEEPALIVE_EXCESS_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_EXCEPTION_SUMMARY_E*/
    {   64,
        1,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_IOAM_INGRESS_OAM_EXCEPTION_SUMMARY_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_IOAM_INGRESS_OAM_EXCEPTION_SUMMARY_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_RDI_STATUS_CHANGE_EXCEPTION_E*/
    {   64,
        1,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_IOAM_INGRESS_OAM_RDI_STATUS_CHANGE_EXCEPTION_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_IOAM_INGRESS_OAM_RDI_STATUS_CHANGE_EXCEPTION_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_TX_PERIOD_EXCEPTION_E*/
    {   64,
        1,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_IOAM_INGRESS_OAM_TX_PERIOD_EXCEPTION_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_IOAM_INGRESS_OAM_TX_PERIOD_EXCEPTION_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_OPCODE_PACKET_COMMAND_E*/
    {   256,
        2,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_IOAM_INGRESS_OAM_OPCODE_PACKET_COMMAND_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_IOAM_INGRESS_OAM_OPCODE_PACKET_COMMAND_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_E*/
    {   _2K,
        3,
        PRV_CPSS_DXCH_INDIRECT_ACCESS_E,
        indirectAccessTableInfo_sip_5 + PRV_CPSS_SIP5_EOAM_TABLE_INDIRECT_E,
        PRV_CPSS_DXCH_INDIRECT_ACCESS_E,
        indirectAccessTableInfo_sip_5 + PRV_CPSS_SIP5_EOAM_TABLE_INDIRECT_E},/*indirect*/

    /*CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_KEEPALIVE_AGING_E*/
    {   64,
        1,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EOAM_EGRESS_OAM_KEEPALIVE_AGING_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EOAM_EGRESS_OAM_KEEPALIVE_AGING_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_MEG_EXCEPTION_E*/
    {   64,
        1,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EOAM_EGRESS_OAM_MEG_EXCEPTION_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EOAM_EGRESS_OAM_MEG_EXCEPTION_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_SOURCE_INTERFACE_EXCEPTION_E*/
    {   64,
        1,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EOAM_EGRESS_OAM_SOURCE_INTERFACE_EXCEPTION_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EOAM_EGRESS_OAM_SOURCE_INTERFACE_EXCEPTION_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_INVALID_KEEPALIVE_HASH_E*/
    {   64,
        1,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EOAM_EGRESS_OAM_INVALID_KEEPALIVE_HASH_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EOAM_EGRESS_OAM_INVALID_KEEPALIVE_HASH_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_KEEPALIVE_EXCESS_E*/
    {   64,
        1,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EOAM_EGRESS_OAM_KEEPALIVE_EXCESS_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EOAM_EGRESS_OAM_KEEPALIVE_EXCESS_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_EXCEPTION_SUMMARY_E*/
    {   64,
        1,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EOAM_EGRESS_OAM_EXCEPTION_SUMMARY_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EOAM_EGRESS_OAM_EXCEPTION_SUMMARY_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_RDI_STATUS_CHANGE_EXCEPTION_E*/
    {   64,
        1,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EOAM_EGRESS_OAM_RDI_STATUS_CHANGE_EXCEPTION_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EOAM_EGRESS_OAM_RDI_STATUS_CHANGE_EXCEPTION_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_TX_PERIOD_EXCEPTION_E*/
    {   64,
        1,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EOAM_EGRESS_OAM_TX_PERIOD_EXCEPTION_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EOAM_EGRESS_OAM_TX_PERIOD_EXCEPTION_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_OPCODE_PACKET_COMMAND_E*/
    {   256,
        2,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EOAM_EGRESS_OAM_OPCODE_PACKET_COMMAND_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EOAM_EGRESS_OAM_OPCODE_PACKET_COMMAND_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_OAM_LM_OFFSET_E*/
    {   128,
        1,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_ERMRK_OAM_LM_OFFSET_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_ERMRK_OAM_LM_OFFSET_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_TTI_QOS_MAP_DSCP_TO_QOS_PROFILE_E*/
    {   12,
        BITS_TO_WORDS_MAC(640),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TTI_QOS_MAP_DSCP_TO_QOS_PROFILE_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TTI_QOS_MAP_DSCP_TO_QOS_PROFILE_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_TTI_QOS_MAP_UP_CFI_TO_QOS_PROFILE_E*/
    {   12,
        BITS_TO_WORDS_MAC(160),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TTI_QOS_MAP_UP_CFI_TO_QOS_PROFILE_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TTI_QOS_MAP_UP_CFI_TO_QOS_PROFILE_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_TTI_QOS_MAP_EXP_TO_QOS_PROFILE_E*/
    {   12,
        BITS_TO_WORDS_MAC(80),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TTI_QOS_MAP_EXP_TO_QOS_PROFILE_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TTI_QOS_MAP_EXP_TO_QOS_PROFILE_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_TTI_QOS_MAP_DSCP_TO_DSCP_E*/
    {   12,
        BITS_TO_WORDS_MAC(384),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TTI_QOS_MAP_DSCP_TO_DSCP_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TTI_QOS_MAP_DSCP_TO_DSCP_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_TTI_UDB_CONFIG_E*/
    {   20,
        BITS_TO_WORDS_MAC(330),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TTI_UDB_CONFIG_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TTI_UDB_CONFIG_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_TTI_PTP_COMMAND_E*/
    {   256,
        BITS_TO_WORDS_MAC(SIP5_TTI_PTP_COMMAND_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TTI_PTP_COMMAND_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TTI_PTP_COMMAND_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_IPCL0_UDB_SELECT_E*/
    {   96,
        BITS_TO_WORDS_MAC(SIP5_IPCL_UDB_SELECT_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_PCL_IPCL0_UDB_SELECT_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_PCL_IPCL0_UDB_SELECT_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_IPCL1_UDB_SELECT_E*/
    {   96,
        BITS_TO_WORDS_MAC(SIP5_IPCL_UDB_SELECT_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_PCL_IPCL1_UDB_SELECT_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_PCL_IPCL1_UDB_SELECT_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_IPCL2_UDB_SELECT_E*/
    {   96,
        BITS_TO_WORDS_MAC(SIP5_IPCL_UDB_SELECT_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_PCL_IPCL2_UDB_SELECT_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_PCL_IPCL2_UDB_SELECT_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_TCAM_PCL_TTI_ACTION_E*/
    {   49152,
        BITS_TO_WORDS_MAC(SIP5_TCAM_PCL_TTI_ACTION_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TCAM_PCL_TTI_ACTION_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TCAM_PCL_TTI_ACTION_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_ERMRK_TIMESTAMP_CFG_E*/
    {   128,
        BITS_TO_WORDS_MAC(SIP5_ERMRK_TIMESTAMP_CFG_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_ERMRK_TIMESTAMP_CFG_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_ERMRK_TIMESTAMP_CFG_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_ERMRK_PTP_LOCAL_ACTION_E*/
    {   32768,
        BITS_TO_WORDS_MAC(SIP5_ERMRK_PTP_LOCAL_ACTION_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_ERMRK_PTP_LOCAL_ACTION_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_ERMRK_PTP_LOCAL_ACTION_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_ERMRK_PTP_TARGET_PORT_E*/
    {   256,
        BITS_TO_WORDS_MAC(SIP5_ERMRK_PTP_TARGET_PORT_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_ERMRK_PTP_TARGET_PORT_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_ERMRK_PTP_TARGET_PORT_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_ERMRK_PTP_SOURCE_PORT_E*/
    {   256,
        BITS_TO_WORDS_MAC(SIP5_ERMRK_PTP_SOURCE_PORT_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_ERMRK_PTP_SOURCE_PORT_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_ERMRK_PTP_SOURCE_PORT_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_ERMRK_QOS_DSCP_MAP_E*/
    {   192,
        BITS_TO_WORDS_MAC(SIP5_ERMRK_QOS_DSCP_MAP_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_ERMRK_QOS_DSCP_MAP_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_ERMRK_QOS_DSCP_MAP_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_ERMRK_QOS_TC_DP_MAP_E*/
    {   192,
        BITS_TO_WORDS_MAC(SIP5_ERMRK_QOS_TC_DP_MAP_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_ERMRK_QOS_TC_DP_MAP_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_ERMRK_QOS_TC_DP_MAP_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_TCAM_E*/
    {   98304,
        BITS_TO_WORDS_MAC(SIP5_TCAM_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TCAM_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TCAM_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_INGRESS_BRIDGE_PORT_MEMBERS_E*/
    {   _8K,
        BITS_TO_WORDS_MAC(SIP5_L2I_INGRESS_BRIDGE_PORT_MEMBERS_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_L2I_INGRESS_BRIDGE_PORT_MEMBERS_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_L2I_INGRESS_BRIDGE_PORT_MEMBERS_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_INGRESS_SPAN_STATE_GROUP_INDEX_E*/
    {   _8K,
        BITS_TO_WORDS_MAC(SIP5_L2I_INGRESS_SPAN_STATE_GROUP_INDEX_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_L2I_INGRESS_SPAN_STATE_GROUP_INDEX_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_L2I_INGRESS_SPAN_STATE_GROUP_INDEX_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_VLAN_ATTRIBUTES_E*/
    {   _8K,
        BITS_TO_WORDS_MAC(SIP5_EGF_SHT_EGRESS_VLAN_ATTRIBUTES_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EGF_SHT_EGRESS_VLAN_ATTRIBUTES_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EGF_SHT_EGRESS_VLAN_ATTRIBUTES_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_VLAN_SPANNING_E*/
    {   _8K,
        BITS_TO_WORDS_MAC(SIP5_EGF_SHT_EGRESS_VLAN_SPANNING_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EGF_SHT_EGRESS_VLAN_SPANNING_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EGF_SHT_EGRESS_VLAN_SPANNING_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_VLAN_MAPPER_E*/
    {   _8K,
        BITS_TO_WORDS_MAC(SIP5_EGF_SHT_EGRESS_VLAN_MAPPER_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EGF_SHT_EGRESS_VLAN_MAPPER_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EGF_SHT_EGRESS_VLAN_MAPPER_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_NON_TRUNK_MEMBERS2_E*/
    {   _4K,
        BITS_TO_WORDS_MAC(BMP_256_PORTS_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EGF_SHT_EGRESS_NON_TRUNK_MEMBERS2_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EGF_SHT_EGRESS_NON_TRUNK_MEMBERS2_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_EPORT_FILTER_E*/
    {   _4K,
        BITS_TO_WORDS_MAC(BMP_256_PORTS_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EGF_SHT_EGRESS_EPORT_FILTER_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EGF_SHT_EGRESS_EPORT_FILTER_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_EPCL_UDB_SELECT_E*/
    {   96,
        BITS_TO_WORDS_MAC(SIP5_EPCL_UDB_SELECT_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EPCL_UDB_SELECT_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_EPCL_UDB_SELECT_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_BMA_PORT_MAPPING_E*/
    {   256,
        BITS_TO_WORDS_MAC(SIP5_BMA_PORT_MAPPING_BIT_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_BMA_PORT_MAPPING_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_BMA_PORT_MAPPING_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_BMA_MULTICAST_COUNTERS_E*/
    {   _8K,
        BITS_TO_WORDS_MAC(SIP5_BMA_MULTICAST_COUNTERS_BIT_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_BMA_MULTICAST_COUNTERS_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_BMA_MULTICAST_COUNTERS_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_TAIL_DROP_MAX_QUEUE_LIMITS_E*/
    {   128,
        BITS_TO_WORDS_MAC(SIP5_TAIL_DROP_MAX_QUEUE_LIMITS_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TXQ_QUEUE_TAIL_DROP_MAX_QUEUE_LIMITS_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TXQ_QUEUE_TAIL_DROP_MAX_QUEUE_LIMITS_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_TAIL_DROP_EQ_QUEUE_LIMITS_DP0_E*/
    {   128,
        BITS_TO_WORDS_MAC(SIP5_TAIL_DROP_EQ_QUEUE_LIMITS_DP0_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TXQ_QUEUE_TAIL_DROP_EQ_QUEUE_LIMITS_DP0_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TXQ_QUEUE_TAIL_DROP_EQ_QUEUE_LIMITS_DP0_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_TAIL_DROP_DQ_QUEUE_BUF_LIMITS_E*/
    {   128,
        BITS_TO_WORDS_MAC(SIP5_TAIL_DROP_DQ_QUEUE_BUF_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TXQ_QUEUE_TAIL_DROP_DQ_QUEUE_BUF_LIMITS_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TXQ_QUEUE_TAIL_DROP_DQ_QUEUE_BUF_LIMITS_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_TAIL_DROP_DQ_QUEUE_DESC_LIMITS_E*/
    {   128,
        BITS_TO_WORDS_MAC(SIP5_TAIL_DROP_DQ_QUEUE_DESC_LIMITS_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TXQ_QUEUE_TAIL_DROP_DQ_QUEUE_DESC_LIMITS_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TXQ_QUEUE_TAIL_DROP_DQ_QUEUE_DESC_LIMITS_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_TAIL_DROP_EQ_QUEUE_LIMITS_DP1_E*/
    {   128,
        BITS_TO_WORDS_MAC(SIP5_TAIL_DROP_EQ_QUEUE_LIMITS_DP1_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TXQ_QUEUE_TAIL_DROP_EQ_QUEUE_LIMITS_DP1_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TXQ_QUEUE_TAIL_DROP_EQ_QUEUE_LIMITS_DP1_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_TAIL_DROP_EQ_QUEUE_LIMITS_DP2_E*/
    {   128,
        BITS_TO_WORDS_MAC(SIP5_TAIL_DROP_EQ_QUEUE_LIMITS_DP2_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TXQ_QUEUE_TAIL_DROP_EQ_QUEUE_LIMITS_DP2_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TXQ_QUEUE_TAIL_DROP_EQ_QUEUE_LIMITS_DP2_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_TAIL_DROP_COUNTERS_Q_MAIN_BUFF_E*/
    {   576,
        BITS_TO_WORDS_MAC(SIP5_TAIL_DROP_COUNTERS_MAIN_BUFF_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TXQ_QUEUE_TAIL_DROP_COUNTERS_Q_MAIN_BUFF_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TXQ_QUEUE_TAIL_DROP_COUNTERS_Q_MAIN_BUFF_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_TAIL_DROP_COUNTERS_Q_MAIN_MC_BUFF_E*/
    {   576,
        BITS_TO_WORDS_MAC(SIP5_TAIL_DROP_COUNTERS_MAIN_MC_BUFF_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TXQ_QUEUE_TAIL_DROP_COUNTERS_Q_MAIN_MC_BUFF_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TXQ_QUEUE_TAIL_DROP_COUNTERS_Q_MAIN_MC_BUFF_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_TXQ_LINK_LIST_COUNTERS_Q_MAIN_DESC_E*/
    {   576,
        BITS_TO_WORDS_MAC(SIP5_TXQ_LINK_LIST_COUNTERS_MAIN_DESC_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TXQ_LL_TXQ_LINK_LIST_COUNTERS_Q_MAIN_DESC_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TXQ_LL_TXQ_LINK_LIST_COUNTERS_Q_MAIN_DESC_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_PFC_MODE_PROFILE_TC_XOFF_THRESHOLDS_E*/
    {   64,
        BITS_TO_WORDS_MAC(SIP5_PFC_MODE_PROFILE_TC_XOFF_THRESHOLDS_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TXQ_PFC_MODE_PROFILE_TC_XOFF_THRESHOLDS_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TXQ_PFC_MODE_PROFILE_TC_XOFF_THRESHOLDS_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_PFC_MODE_PROFILE_TC_XON_THRESHOLDS_E*/
    {   64,
        BITS_TO_WORDS_MAC(SIP5_PFC_MODE_PROFILE_TC_XON_THRESHOLDS_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TXQ_PFC_MODE_PROFILE_TC_XON_THRESHOLDS_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TXQ_PFC_MODE_PROFILE_TC_XON_THRESHOLDS_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_PFC_LLFC_COUNTERS_E*/
    {   1024,
        BITS_TO_WORDS_MAC(SIP5_PFC_LLFC_COUNTERS_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TXQ_PFC_LLFC_COUNTERS_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TXQ_PFC_LLFC_COUNTERS_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_TM_DROP_DROP_MASKING_E*/
    {   128,
        BITS_TO_WORDS_MAC(SIP5_TM_DROP_DROP_MASKING_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TM_DROP_DROP_MASKING_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TM_DROP_DROP_MASKING_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_TM_DROP_QUEUE_PROFILE_ID_E*/
    {   1024,
        BITS_TO_WORDS_MAC(SIP5_TM_DROP_QUEUE_PROFILE_ID_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TM_DROP_QUEUE_PROFILE_ID_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TM_DROP_QUEUE_PROFILE_ID_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_TM_QMAP_CPU_CODE_TO_TC_E*/
    {   256,
        BITS_TO_WORDS_MAC(SIP5_TM_QMAP_CPU_CODE_TO_TC_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TM_QMAP_CPU_CODE_TO_TC_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TM_QMAP_CPU_CODE_TO_TC_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_TM_QMAP_TARGET_DEV_TO_INDEX_E*/
    {   1024,
        BITS_TO_WORDS_MAC(SIP5_TM_QMAP_TARGET_DEV_TO_INDEX_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TM_QMAP_TARGET_DEV_TO_INDEX_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TM_QMAP_TARGET_DEV_TO_INDEX_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_TM_QMAP_POLICING_ENABLE_E*/
    {   1024,
        BITS_TO_WORDS_MAC(SIP5_TM_QMAP_TM_QMAP_POLICING_ENABLE_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TM_QMAP_POLICING_ENABLE_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TM_QMAP_POLICING_ENABLE_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_TM_QMAP_QUEUE_ID_SELECTOR_E*/
    {   256,
        BITS_TO_WORDS_MAC(SIP5_TM_QMAP_QUEUE_ID_SELECTOR_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TM_QMAP_QUEUE_ID_SELECTOR_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TM_QMAP_QUEUE_ID_SELECTOR_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_TM_INGR_GLUE_L1_PKT_LEN_OFFSET_E*/
    {   64,
        BITS_TO_WORDS_MAC(SIP5_TM_INGR_GLUE_L1_PKT_LEN_OFFSET_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TM_INGRESS_GLUE_L1_PKT_LEN_OFFSET_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TM_INGRESS_GLUE_L1_PKT_LEN_OFFSET_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_TM_EGR_GLUE_AGING_QUEUE_PROFILE_E*/
    {   1024,
        BITS_TO_WORDS_MAC(SIP5_TM_EGR_GLUE_AGING_QUEUE_PROFILE_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TM_EGRESS_GLUE_AGING_QUEUE_PROFILE_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TM_EGRESS_GLUE_AGING_QUEUE_PROFILE_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_TM_EGR_GLUE_AGING_PROFILE_THESHOLDS_E*/
    {   16,
        BITS_TO_WORDS_MAC(SIP5_TM_EGR_GLUE_AGING_PROFILE_THESHOLDS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TM_EGRESS_GLUE_AGING_PROFILE_THESHOLDS_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TM_EGRESS_GLUE_AGING_PROFILE_THESHOLDS_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_TM_EGR_GLUE_TARGET_INTERFACE_E*/
    {   64,
        BITS_TO_WORDS_MAC(SIP5_TM_EGR_GLUE_TARGET_INTERFACE_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TM_EGRESS_GLUE_TARGET_INTERFACE_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TM_EGRESS_GLUE_TARGET_INTERFACE_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_TM_FCU_ETH_DMA_TO_TM_PORT_MAPPING_E*/
    {   32,
        BITS_TO_WORDS_MAC(SIP5_TM_FCU_ETH_DMA_TO_TM_PORT_MAPPING_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TM_FCU_ETH_DMA_TO_TM_PORT_MAPPING_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TM_FCU_ETH_DMA_TO_TM_PORT_MAPPING_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_TM_FCU_ILK_DMA_TO_TM_PORT_MAPPING_E*/
    {   32,
        BITS_TO_WORDS_MAC(SIP5_TM_FCU_ILK_DMA_TO_TM_PORT_MAPPING_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TM_FCU_ILK_DMA_TO_TM_PORT_MAPPING_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TM_FCU_ILK_DMA_TO_TM_PORT_MAPPING_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_TM_FCU_TC_PORT_TO_CNODE_PORT_MAPPING_E*/
    {   64,
        BITS_TO_WORDS_MAC(SIP5_TM_FCU_TC_PORT_TO_CNODE_PORT_MAPPING_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TM_FCU_TC_PORT_TO_CNODE_PORT_MAPPING_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TM_FCU_TC_PORT_TO_CNODE_PORT_MAPPING_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_TM_FCU_PORT_INGRESS_TIMERS_CONFIG_E*/
    {   64,
        BITS_TO_WORDS_MAC(SIP5_TM_FCU_PORT_INGRESS_TIMERS_CONFIG_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TM_FCU_PORT_INGRESS_TIMERS_CONFIG_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TM_FCU_PORT_INGRESS_TIMERS_CONFIG_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_TM_FCU_INGRESS_TIMERS_E*/
    {   64,
        BITS_TO_WORDS_MAC(SIP5_TM_FCU_INGRESS_TIMERS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TM_FCU_INGRESS_TIMERS_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_TM_FCU_INGRESS_TIMERS_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_TM_FCU_PORT_TO_PHYSICAL_PORT_MAPPING_E*/
    {   64,
        BITS_TO_WORDS_MAC(SIP5_TM_FCU_PORT_TO_PHYSICAL_PORT_MAPPING_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_PORT_TO_PHYSICAL_PORT_MAPPING_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_PORT_TO_PHYSICAL_PORT_MAPPING_E].directAccessInfo},


    /*CPSS_DXCH_SIP5_TABLE_CN_SAMPLE_INTERVALS_E*/
    {   576,
        BITS_TO_WORDS_MAC(SIP5_CN_SAMPLE_INTERVALS_BITS_SIZE_CNS),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_CN_SAMPLE_INTERVALS_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_CN_SAMPLE_INTERVALS_E].directAccessInfo},


    /*CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_IPFIX_ALERT_WRAPAROUND_E*/
    {   256,
        BITS_TO_WORDS_MAC(32),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_CPSS_BOBCAT2_TABLE_INGRESS_POLICER_0_IPFIX_ALERT_WRAPAROUND_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_CPSS_BOBCAT2_TABLE_INGRESS_POLICER_0_IPFIX_ALERT_WRAPAROUND_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_IPFIX_ALERT_WRAPAROUND_E*/
    {   256,
        BITS_TO_WORDS_MAC(32),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_CPSS_BOBCAT2_TABLE_INGRESS_POLICER_1_IPFIX_ALERT_WRAPAROUND_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_CPSS_BOBCAT2_TABLE_INGRESS_POLICER_1_IPFIX_ALERT_WRAPAROUND_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_EGRESS_POLICER_IPFIX_ALERT_WRAPAROUND_E*/
    {   256,
        BITS_TO_WORDS_MAC(32),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_CPSS_BOBCAT2_TABLE_EGRESS_POLICER_IPFIX_ALERT_WRAPAROUND_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_CPSS_BOBCAT2_TABLE_EGRESS_POLICER_IPFIX_ALERT_WRAPAROUND_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_IPFIX_ALERT_AGING_E*/
    {   256,
        BITS_TO_WORDS_MAC(32),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_CPSS_BOBCAT2_TABLE_INGRESS_POLICER_0_IPFIX_ALERT_AGING_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_CPSS_BOBCAT2_TABLE_INGRESS_POLICER_0_IPFIX_ALERT_AGING_E].directAccessInfo},


    /*CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_IPFIX_ALERT_AGING_E*/
    {   256,
        BITS_TO_WORDS_MAC(32),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_CPSS_BOBCAT2_TABLE_INGRESS_POLICER_1_IPFIX_ALERT_AGING_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_CPSS_BOBCAT2_TABLE_INGRESS_POLICER_1_IPFIX_ALERT_AGING_E].directAccessInfo},

    /*CPSS_DXCH_SIP5_TABLE_EGRESS_POLICER_IPFIX_ALERT_AGING_E*/
    {   256,
        BITS_TO_WORDS_MAC(32),
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_CPSS_BOBCAT2_TABLE_EGRESS_POLICER_IPFIX_ALERT_AGING_E].directAccessInfo,
        PRV_CPSS_DXCH_DIRECT_ACCESS_E,
        &directAccessBobcat2TableInfo[PRV_CPSS_BOBCAT2_TABLE_EGRESS_POLICER_IPFIX_ALERT_AGING_E].directAccessInfo},

    BC2_CNC_TABLES_INFO_MAC( 0),
    BC2_CNC_TABLES_INFO_MAC( 1),
    BC2_CNC_TABLES_INFO_MAC( 2),
    BC2_CNC_TABLES_INFO_MAC( 3),
    BC2_CNC_TABLES_INFO_MAC( 4),
    BC2_CNC_TABLES_INFO_MAC( 5),
    BC2_CNC_TABLES_INFO_MAC( 6),
    BC2_CNC_TABLES_INFO_MAC( 7),
    BC2_CNC_TABLES_INFO_MAC( 8),
    BC2_CNC_TABLES_INFO_MAC( 9),
    BC2_CNC_TABLES_INFO_MAC(10),
    BC2_CNC_TABLES_INFO_MAC(11),
    BC2_CNC_TABLES_INFO_MAC(12),
    BC2_CNC_TABLES_INFO_MAC(13),
    BC2_CNC_TABLES_INFO_MAC(14),
    BC2_CNC_TABLES_INFO_MAC(15),
    BC2_CNC_TABLES_INFO_MAC(16),
    BC2_CNC_TABLES_INFO_MAC(17),
    BC2_CNC_TABLES_INFO_MAC(18),
    BC2_CNC_TABLES_INFO_MAC(19),
    BC2_CNC_TABLES_INFO_MAC(20),
    BC2_CNC_TABLES_INFO_MAC(21),
    BC2_CNC_TABLES_INFO_MAC(22),
    BC2_CNC_TABLES_INFO_MAC(23),
    BC2_CNC_TABLES_INFO_MAC(24),
    BC2_CNC_TABLES_INFO_MAC(25),
    BC2_CNC_TABLES_INFO_MAC(26),
    BC2_CNC_TABLES_INFO_MAC(27),
    BC2_CNC_TABLES_INFO_MAC(28),
    BC2_CNC_TABLES_INFO_MAC(29),
    BC2_CNC_TABLES_INFO_MAC(30),
    BC2_CNC_TABLES_INFO_MAC(31)
};

/* bobcat 2 */
static const GT_U32 bobcat2TableInfoSize =
    (NUM_ELEMENTS_IN_ARR_MAC(bobcat2TablesInfo));


/* Bobcat2 B0 table information , that need to override A0 info from bobcat2TablesInfo */
/* NOTE : the entries in this array need to be matched with entries in
        bobcat2TablesInfo , according to field */
static const PRV_CPSS_DXCH_TABLES_INFO_EXT_STC bobcat2B0TablesInfo_overrideA0[] =
{
    /* HA */
    {CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E,
        /*CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E*/
        {   _8K,
            BITS_TO_WORDS_MAC(BOBCAT_B0_HA_EGRESS_EPORT_1_BITS_SIZE_CNS),
            PRV_CPSS_DXCH_DIRECT_ACCESS_E,
            &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_HA_EGRESS_EPORT_1_E].directAccessInfo,
            PRV_CPSS_DXCH_DIRECT_ACCESS_E,
            &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_HA_EGRESS_EPORT_1_E].directAccessInfo}
    },

    {CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_2_E,
        /*CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_2_E*/
        {   _8K,
            BITS_TO_WORDS_MAC(BOBCAT_B0_HA_EGRESS_EPORT_2_BITS_SIZE_CNS),
            PRV_CPSS_DXCH_DIRECT_ACCESS_E,
            &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_HA_EGRESS_EPORT_2_E].directAccessInfo,
            PRV_CPSS_DXCH_DIRECT_ACCESS_E,
            &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_HA_EGRESS_EPORT_2_E].directAccessInfo}
    },

    /* ERMRK */
    {CPSS_DXCH_SIP5_TABLE_ERMRK_TIMESTAMP_CFG_E,
        /*CPSS_DXCH_SIP5_TABLE_ERMRK_TIMESTAMP_CFG_E*/
        {   128,
            BITS_TO_WORDS_MAC(BOBCAT2_B0_ERMRK_TIMESTAMP_CFG_BITS_SIZE_CNS),
            PRV_CPSS_DXCH_DIRECT_ACCESS_E,
            &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_ERMRK_TIMESTAMP_CFG_E].directAccessInfo,
            PRV_CPSS_DXCH_DIRECT_ACCESS_E,
            &directAccessBobcat2TableInfo[PRV_DXCH_BOBCAT2_TABLE_ERMRK_TIMESTAMP_CFG_E].directAccessInfo},
    }

};

/* override table bobcat2B0TablesInfo_overrideA0 size */
static const GT_U32 bobcat2B0TablesInfo_overrideA0Size =
    (NUM_ELEMENTS_IN_ARR_MAC(bobcat2B0TablesInfo_overrideA0));


/* size of directAccessBobkTableInfo table */
static const GT_U32 directAccessBobkTableInfoSize = PRV_DXCH_BOBK_TABLE___LAST_E;

typedef struct{
    PRV_DXCH_BOBCAT2_TABLES_ENT             newIndex; /* index in directAccessBobkTableInfo */
    PRV_CPSS_DXCH_TABLES_INFO_DIRECT_UNIT_STC    directAccessInfo;
}BOBK_TABLES_INFO_DIRECT_NEW_INDEX_STC;

/* Bobk direct table information , that are new (not exists in bobcat2 A0/B0) */
/* register new tables that not exists in bobcat2 !!!! */
static const BOBK_TABLES_INFO_DIRECT_NEW_INDEX_STC directAccessBobkTableInfo_new[] =
{
    {PRV_DXCH_BOBK_TABLE_PLR_INGRESS_POLICER_0_METERING_CONFIG_E,/* index in directAccessBobkTableInfo */
        {CPSS_DXCH_SIP5_15_TABLE_INGRESS_POLICER_0_METERING_CONFIG_E, PRV_CPSS_DXCH_UNIT_IPLR_E,
         {0x0B000000 + 0x00300000, BITS_TO_BYTES_ALIGNMENT_MAC(BOBK_POLICER_METERING_CONFIG_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE}}

    ,{PRV_DXCH_BOBK_TABLE_PLR_INGRESS_POLICER_1_METERING_CONFIG_E,/* index in directAccessBobkTableInfo */
         {CPSS_DXCH_SIP5_15_TABLE_INGRESS_POLICER_1_METERING_CONFIG_E, PRV_CPSS_DXCH_UNIT_IPLR_1_E,
         {0x20000000 + 0x00300000, BITS_TO_BYTES_ALIGNMENT_MAC(BOBK_POLICER_METERING_CONFIG_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE}}

    ,{PRV_DXCH_BOBK_TABLE_PLR_EGRESS_POLICER_METERING_CONFIG_E,/* index in directAccessBobkTableInfo */
         {CPSS_DXCH_SIP5_15_TABLE_EGRESS_POLICER_METERING_CONFIG_E, PRV_CPSS_DXCH_UNIT_EPLR_E,
         {0x0E000000 + 0x00300000, BITS_TO_BYTES_ALIGNMENT_MAC(BOBK_POLICER_METERING_CONFIG_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE}}

    ,{PRV_DXCH_BOBK_TABLE_PLR_INGRESS_POLICER_0_METERING_CONFORM_SIGN_E,/* index in directAccessBobkTableInfo */
        {CPSS_DXCH_SIP5_15_TABLE_INGRESS_POLICER_0_METERING_CONFORM_SIGN_E, PRV_CPSS_DXCH_UNIT_IPLR_E,
         {0x0B000000 + 0x00400000, BITS_TO_BYTES_ALIGNMENT_MAC(BOBK_POLICER_METERING_CONFORM_SIGN_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE}}

    ,{PRV_DXCH_BOBK_TABLE_PLR_INGRESS_POLICER_1_METERING_CONFORM_SIGN_E,/* index in directAccessBobkTableInfo */
         {CPSS_DXCH_SIP5_15_TABLE_INGRESS_POLICER_1_METERING_CONFORM_SIGN_E, PRV_CPSS_DXCH_UNIT_IPLR_1_E,
         {0x20000000 + 0x00400000, BITS_TO_BYTES_ALIGNMENT_MAC(BOBK_POLICER_METERING_CONFORM_SIGN_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE}}

    ,{PRV_DXCH_BOBK_TABLE_PLR_EGRESS_POLICER_METERING_CONFORM_SIGN_E,/* index in directAccessBobkTableInfo */
         {CPSS_DXCH_SIP5_15_TABLE_EGRESS_POLICER_METERING_CONFORM_SIGN_E, PRV_CPSS_DXCH_UNIT_EPLR_E,
         {0x0E000000 + 0x00400000, BITS_TO_BYTES_ALIGNMENT_MAC(BOBK_POLICER_METERING_CONFORM_SIGN_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE}}
};

#define BOBK_NUM_EPORTS_MAC     _6K /*8k in bc2 */
#define BOBK_NUM_PHY_PORTS_MAC  128 /*256 in bc2 */
#define BOBK_NUM_EVLANS_MAC     (_4K + 512) /*4.5k*/ /*8k in bc2 */
#define BOBK_NUM_STG_MAC        _1K /*4K in bc2 */
#define BOBK_NUM_IPVX_NH_MAC    _8K /* 24K in bc2 */
#define BOBK_NUM_MLL_MAC        _4K /*8K in bc2 */
#define BOBK_NUM_EVIDX_MAC       _8K /*16K in bc2 */
#define BOBK_NUM_L3_ECMP_MAC    _4K /*12K in bc2 */
#define BOBK_NUM_IPLR_METERS_MAC    _6K
#define BOBK_NUM_EPLR_METERS_MAC    _4K
#define BOBK_NUM_BMA_PORT_MAP_ENTRIES_MAC    (4*256)

#define BOBK_L2I_INGRESS_STP_BITS_SIZE_CNS                  (2*BOBK_NUM_PHY_PORTS_MAC)/* 2 bits per port */
#define BOBK_L2I_INGRESS_BRIDGE_PORT_MEMBERS_BITS_SIZE_CNS  BOBK_NUM_PHY_PORTS_MAC

/* Bobk direct table information , that need to override bobcat2 info from directAccessBobkTableInfo */
static const PRV_CPSS_DXCH_TABLES_INFO_DIRECT_EXT_STC directAccessBobkTableInfo_overrideBobcat2[] =
{
    /*L2i tables */
    {CPSS_DXCH_LION_TABLE_STG_INGRESS_E,
    {NOT_MODIFIED_VALUE_CNS, BITS_TO_BYTES_ALIGNMENT_MAC(BOBK_L2I_INGRESS_STP_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

    {CPSS_DXCH_SIP5_TABLE_INGRESS_BRIDGE_PORT_MEMBERS_E,
    {NOT_MODIFIED_VALUE_CNS, BITS_TO_BYTES_ALIGNMENT_MAC(BOBK_L2I_INGRESS_BRIDGE_PORT_MEMBERS_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},


    /* MLL Tables */
    {CPSS_DXCH_SIP5_TABLE_IP_MLL_E,
    {0x1D000000 + 0x00100000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_MLL_ENTRY_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

    {CPSS_DXCH_SIP5_TABLE_L2_MLL_E,/* must be the same as 'IP_MLL' */
    {0x1D000000 + 0x00100000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_MLL_ENTRY_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

    /*IOAM*/
    {CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_E,
    {0x1C000000 + 0x00080000, 0x10, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

    {CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_KEEPALIVE_AGING_E,
    {0x1C000000 + 0x00007000, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

    /*EOAM*/
    {CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_E,
    {0x1E000000 + 0x00080000, 0x10, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

    {CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_KEEPALIVE_AGING_E,
    {0x1E000000 + 0x00007000, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

};

/* bobk table - in runtime copy table from this of bobcat2_b0 ... and continue from there ... */
/* bobk info */
static const GT_U32 bobkTableInfoSize = CPSS_DXCH_SIP5_15_TABLE_LAST_E;

/* "not modify" value for pointers:
   0xFFFFFFFF for 32 bits CPUs, 0xFFFFFFFF FFFFFFFF for 64 bits CPUs*/
#define NOT_MODIFIED_POINTER_CNS  ((void *)(~(GT_UINTPTR)0))

/* override value if new value is 'modified' */
#define OVERRIDE_DIRECT_INFO_IF_MODIFIED_MAC(_overrideInfoPtr,_infoPtr,_field) \
    if(_overrideInfoPtr->_field != NOT_MODIFIED_VALUE_CNS)                     \
        _infoPtr->_field = _overrideInfoPtr->_field

/* override value of pointer if new value is 'modified' */
#define OVERRIDE_DIRECT_INFO_IF_MODIFIED_POINTER_MAC(_overrideInfoPtr,_infoPtr,_field) \
    if(_overrideInfoPtr->_field != NOT_MODIFIED_POINTER_CNS)                           \
        _infoPtr->_field = _overrideInfoPtr->_field

/* set valid field only : maxNumOfEntries */
#define OVERRIDE_INFO_NUM_ENTRIES_MAC(maxNumOfEntries)                  \
        {   maxNumOfEntries,                   /*maxNumOfEntries*/      \
            NOT_MODIFIED_VALUE_CNS,            /*entrySize      */      \
            NOT_MODIFIED_VALUE_CNS,            /*readAccessType */      \
            NOT_MODIFIED_POINTER_CNS,          /*readTablePtr   */      \
            NOT_MODIFIED_VALUE_CNS,            /*writeAccessType*/      \
            NOT_MODIFIED_POINTER_CNS}          /*writeTablePtr  */

/* set valid field only : entrySize */
#define OVERRIDE_INFO_WIDTH_MAC(entrySize_inBits)                       \
        {   NOT_MODIFIED_VALUE_CNS,            /*maxNumOfEntries*/      \
            BITS_TO_WORDS_MAC(entrySize_inBits),/*entrySize      */     \
            NOT_MODIFIED_VALUE_CNS,            /*readAccessType */      \
            NOT_MODIFIED_POINTER_CNS,          /*readTablePtr   */      \
            NOT_MODIFIED_VALUE_CNS,            /*writeAccessType*/      \
            NOT_MODIFIED_POINTER_CNS}          /*writeTablePtr  */

/* set valid field only : maxNumOfEntries, entrySize
   NOTE: entrySize is in 'bits' !!!
*/
#define OVERRIDE_INFO_NUM_ENTRIES_AND_WIDTH_MAC(maxNumOfEntries,entrySize_inBits) \
        {   maxNumOfEntries,                   /*maxNumOfEntries*/      \
            BITS_TO_WORDS_MAC(entrySize_inBits),/*entrySize      */     \
            NOT_MODIFIED_VALUE_CNS,            /*readAccessType */      \
            NOT_MODIFIED_POINTER_CNS,          /*readTablePtr   */      \
            NOT_MODIFIED_VALUE_CNS,            /*writeAccessType*/      \
            NOT_MODIFIED_POINTER_CNS}          /*writeTablePtr  */



/*  table information , that need to override Bobcat2 B0 info from bobkTablesInfo */
/* NOTE : the entries in this array need to be matched with entries in
        bobcat2TablesInfo , according to field */
static const PRV_CPSS_DXCH_TABLES_INFO_EXT_STC bobkTablesInfo_overrideBobcat2[] =
{
    /* L2i */
    {CPSS_DXCH_SIP5_TABLE_BRIDGE_INGRESS_EPORT_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(BOBK_NUM_EPORTS_MAC)
    },
    {CPSS_DXCH_SIP5_TABLE_BRIDGE_INGRESS_EPORT_LEARN_PRIO_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(BOBK_NUM_EPORTS_MAC/8)/* 8 eports per entry */
    },
    {CPSS_DXCH_SIP5_TABLE_BRIDGE_PHYSICAL_PORT_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(BOBK_NUM_PHY_PORTS_MAC)
    },
    {CPSS_DXCH_SIP5_TABLE_BRIDGE_PHYSICAL_PORT_RATE_LIMIT_COUNTERS_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(BOBK_NUM_PHY_PORTS_MAC)
    },
    {CPSS_DXCH_LION_TABLE_STG_INGRESS_E,
        OVERRIDE_INFO_NUM_ENTRIES_AND_WIDTH_MAC(BOBK_NUM_STG_MAC , BOBK_L2I_INGRESS_STP_BITS_SIZE_CNS)
    },
    {CPSS_DXCH_SIP5_TABLE_INGRESS_SPAN_STATE_GROUP_INDEX_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(BOBK_NUM_EVLANS_MAC)
    },
    {CPSS_DXCH_SIP5_TABLE_INGRESS_BRIDGE_PORT_MEMBERS_E,
        OVERRIDE_INFO_NUM_ENTRIES_AND_WIDTH_MAC(BOBK_NUM_EVLANS_MAC , BOBK_L2I_INGRESS_BRIDGE_PORT_MEMBERS_BITS_SIZE_CNS)
    },
    {CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(BOBK_NUM_EVLANS_MAC)
    },

    /* tti */
    {CPSS_DXCH_SIP5_TABLE_POST_TTI_LOOKUP_INGRESS_EPORT_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(BOBK_NUM_EPORTS_MAC)
    },

    {CPSS_DXCH_SIP5_TABLE_TTI_PHYSICAL_PORT_ATTRIBUTE_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(BOBK_NUM_PHY_PORTS_MAC)
    },

    /* epcl */
    {CPSS_DXCH2_TABLE_EGRESS_PCL_CONFIG_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(_4K + BOBK_NUM_PHY_PORTS_MAC)
    },

    /* eq */
    {CPSS_DXCH_SIP5_TABLE_EQ_INGRESS_EPORT_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(BOBK_NUM_EPORTS_MAC / 4) /* 4 eports per entry */
    },

    {CPSS_DXCH_SIP5_TABLE_EQ_L2_ECMP_LTT_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(BOBK_NUM_EPORTS_MAC)
    },

    {CPSS_DXCH_SIP5_TABLE_EQ_L2_ECMP_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(_4K)
    },

    {CPSS_DXCH_SIP5_TABLE_EQ_TX_PROTECTION_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(BOBK_NUM_EPORTS_MAC / 32) /* 32 eports per entry */
    },

    {CPSS_DXCH_SIP5_TABLE_EQ_EPORT_TO_LOC_MAPPING_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(BOBK_NUM_EPORTS_MAC / 2) /* 2 eports per entry */
    },

    {CPSS_DXCH_SIP5_TABLE_EQ_EPORT_TO_PHYSICAL_PORT_TARGET_MAPPING_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(BOBK_NUM_EPORTS_MAC)
    },

    /*ERMRK*/
    {CPSS_DXCH_SIP5_TABLE_ERMRK_PTP_TARGET_PORT_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(BOBK_NUM_PHY_PORTS_MAC)
    },

    {CPSS_DXCH_SIP5_TABLE_ERMRK_PTP_SOURCE_PORT_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(BOBK_NUM_PHY_PORTS_MAC)
    },

    /*ha*/
    {CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(BOBK_NUM_EPORTS_MAC)
    },
    {CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_2_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(BOBK_NUM_EPORTS_MAC)
    },

    {CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_1_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(BOBK_NUM_PHY_PORTS_MAC)
    },
    {CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(BOBK_NUM_PHY_PORTS_MAC)
    },

    {CPSS_DXCH2_TABLE_TUNNEL_START_CONFIG_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(_8K)
    },

    {CPSS_DXCH3_TABLE_EGRESS_VLAN_TRANSLATION_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(BOBK_NUM_EVLANS_MAC)
    },

    {CPSS_DXCH_SIP5_TABLE_HA_PTP_DOMAIN_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(BOBK_NUM_PHY_PORTS_MAC * 5) /* each port with 5 domains */
    },

    /* ipcl */
    /*
        CPSS_DXCH_TABLE_PCL_CONFIG_E,
        CPSS_DXCH_XCAT2_TABLE_INGRESS_PCL_LOOKUP01_CONFIG_E,
        CPSS_DXCH_XCAT_TABLE_INGRESS_PCL_LOOKUP1_CONFIG_E

        supports : 4K + 256 -- same as in BC2 !!!
        so no 'override' needed
    */

    /* ipvx*/
    {CPSS_DXCH_SIP5_TABLE_IPVX_INGRESS_EPORT_E,
        OVERRIDE_INFO_NUM_ENTRIES_AND_WIDTH_MAC((BOBK_NUM_EPORTS_MAC / 16) /* 16 eports per entry */, BOBK_IPVX_ROUTER_EPORT_BITS_SIZE_CNS)
    },

    {CPSS_DXCH_SIP5_TABLE_IPVX_ROUTER_NEXTHOP_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(BOBK_NUM_IPVX_NH_MAC)
    },

    {CPSS_DXCH_SIP5_TABLE_IPVX_EVLAN_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(BOBK_NUM_EVLANS_MAC)
    },


    {CPSS_DXCH_SIP5_TABLE_IPVX_ROUTER_NEXTHOP_AGE_BITS_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(BOBK_NUM_IPVX_NH_MAC / 32) /* 32 NH per entry */
    },

    /* mll */
    {CPSS_DXCH_SIP5_TABLE_IP_MLL_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(BOBK_NUM_MLL_MAC)
    },

    {CPSS_DXCH_SIP5_TABLE_L2_MLL_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(BOBK_NUM_MLL_MAC)
    },

    {CPSS_DXCH_SIP5_TABLE_L2_MLL_LTT_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(BOBK_NUM_EVIDX_MAC)
    },

    /* egf */
        /*qag*/
    {CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_EGRESS_EPORT_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(BOBK_NUM_EPORTS_MAC)
    },
    {CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_EGRESS_VLAN_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(BOBK_NUM_EVLANS_MAC)/* NOTE: keep width as if 256 ports exists !!! */
    },

    {CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(BOBK_NUM_PHY_PORTS_MAC)
    },

    {CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_FWD_FROM_CPU_TO_LOOPBACK_MAPPER_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(BOBK_NUM_PHY_PORTS_MAC)
    },

        /*eft*/
    {CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_EFT_FAST_STACK_FAILOVER_SECONDARY_TARGET_PORT_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(BOBK_NUM_PHY_PORTS_MAC)
    },

        /*sht*/
    {CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_VLAN_SPANNING_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(BOBK_NUM_EVLANS_MAC)
    },

    {CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_EGRESS_EPORT_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(BOBK_NUM_EPORTS_MAC)
    },

    {CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_VLAN_ATTRIBUTES_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(BOBK_NUM_EVLANS_MAC)
    },

    {CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_VLAN_MAPPER_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(BOBK_NUM_EVLANS_MAC)
    },

    {CPSS_DXCH_LION_TABLE_STG_EGRESS_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(BOBK_NUM_STG_MAC) /* NOTE: keep width as if 256 ports exists !!! */
    },

    /*CPSS_DXCH_LION_TABLE_TXQ_NON_TRUNK_MEMBERS_E, --> NOTE: keep width as if 256 ports exists !!! */
    /*CPSS_DXCH_LION_TABLE_PORT_ISOLATION_L2_E,  --> NOTE: keep width as if 256 ports exists !!! */
    /*CPSS_DXCH_LION_TABLE_PORT_ISOLATION_L3_E,  --> NOTE: keep width as if 256 ports exists !!! */
    /*CPSS_DXCH_TABLE_MULTICAST_E,  --> NOTE: keep width as if 256 ports exists !!! */
    /*CPSS_DXCH_LION_TABLE_TXQ_SOURCE_ID_MEMBERS_E,  --> NOTE: keep width as if 256 ports exists !!! */
    /*CPSS_DXCH_LION_TABLE_TXQ_DESIGNATED_PORT_E,  --> NOTE: keep width as if 256 ports exists !!! */
    {CPSS_DXCH_LION_TABLE_VLAN_EGRESS_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(BOBK_NUM_EVLANS_MAC )/* NOTE: keep width as if 256 ports exists !!! */
    },

    /*lpm*/
    CPSS_TBD_BOOKMARK_BOBCAT2_BOBK
    /* need to support (6K * 20) sub memories of 16K range for each of the 20 */
    {CPSS_DXCH_SIP5_TABLE_LPM_MEM_E,
        {   NOT_MODIFIED_VALUE_CNS,            /*maxNumOfEntries*/
            NOT_MODIFIED_VALUE_CNS,            /*entrySize      */
            NOT_MODIFIED_VALUE_CNS,            /*readAccessType */
            NOT_MODIFIED_POINTER_CNS,          /*readTablePtr   */
            NOT_MODIFIED_VALUE_CNS,            /*writeAccessType*/
            NOT_MODIFIED_POINTER_CNS}          /*writeTablePtr  */
    },

    {CPSS_DXCH_SIP5_TABLE_ECMP_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(BOBK_NUM_L3_ECMP_MAC)
    },

    {CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_E,
        OVERRIDE_INFO_WIDTH_MAC(104)
    },

    {CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_E,
        OVERRIDE_INFO_WIDTH_MAC(104)
    },

    /* PLR */
    {CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_METERING_E,
        OVERRIDE_INFO_NUM_ENTRIES_AND_WIDTH_MAC(BOBK_NUM_IPLR_METERS_MAC,
            BOBK_POLICER_METERING_TOKEN_BUCKET_BITS_SIZE_CNS)
    },
    {CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_METERING_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(0)  /* not valid - use CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_METERING_E for access */
    },
    {CPSS_DXCH_SIP5_TABLE_EGRESS_POLICER_METERING_E,
        OVERRIDE_INFO_NUM_ENTRIES_AND_WIDTH_MAC(BOBK_NUM_EPLR_METERS_MAC,
            BOBK_POLICER_METERING_TOKEN_BUCKET_BITS_SIZE_CNS)
    },
    {CPSS_DXCH_SIP5_15_TABLE_INGRESS_POLICER_0_METERING_CONFORM_SIGN_E,
        OVERRIDE_INFO_NUM_ENTRIES_AND_WIDTH_MAC(BOBK_NUM_IPLR_METERS_MAC,
            BOBK_POLICER_METERING_CONFORM_SIGN_BITS_SIZE_CNS)
    },
    {CPSS_DXCH_SIP5_15_TABLE_INGRESS_POLICER_1_METERING_CONFORM_SIGN_E,
        OVERRIDE_INFO_NUM_ENTRIES_AND_WIDTH_MAC(BOBK_NUM_IPLR_METERS_MAC,
            BOBK_POLICER_METERING_CONFORM_SIGN_BITS_SIZE_CNS)
    },
    {CPSS_DXCH_SIP5_15_TABLE_EGRESS_POLICER_METERING_CONFORM_SIGN_E,
        OVERRIDE_INFO_NUM_ENTRIES_AND_WIDTH_MAC(BOBK_NUM_EPLR_METERS_MAC,
            BOBK_POLICER_METERING_CONFORM_SIGN_BITS_SIZE_CNS)
    },

    {CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_COUNTING_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(BOBK_NUM_IPLR_METERS_MAC)
    },
    {CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_COUNTING_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(BOBK_NUM_IPLR_METERS_MAC)
    },
    {CPSS_DXCH_SIP5_TABLE_EGRESS_POLICER_COUNTING_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(BOBK_NUM_EPLR_METERS_MAC)
    },
    {CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_E_ATTRIBUTES_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(BOBK_NUM_EPORTS_MAC) /*more eports than evlans*/
    },
    {CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_E_ATTRIBUTES_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(BOBK_NUM_EPORTS_MAC) /*more eports than evlans*/
    },
    {CPSS_DXCH_SIP5_TABLE_EGRESS_POLICER_E_ATTRIBUTES_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(BOBK_NUM_EPORTS_MAC)  /*more eports than evlans*/
    },

    {CPSS_DXCH_SIP5_15_TABLE_INGRESS_POLICER_0_METERING_CONFIG_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(BOBK_NUM_IPLR_METERS_MAC)
    },
    {CPSS_DXCH_SIP5_15_TABLE_EGRESS_POLICER_METERING_CONFIG_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(BOBK_NUM_EPLR_METERS_MAC)
    },

    {CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_HIERARCHICAL_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(BOBK_NUM_IPLR_METERS_MAC)
    },

    /* BMA */
    {CPSS_DXCH_SIP5_TABLE_BMA_PORT_MAPPING_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(BOBK_NUM_BMA_PORT_MAP_ENTRIES_MAC)
    }
};

#define ALDRIN_NUM_IPLR_METERS_MAC _2K
#define ALDRIN_NUM_EPLR_METERS_MAC _2K
#define ALDRIN_NUM_FDB_MAC         _32K

static const PRV_CPSS_DXCH_TABLES_INFO_EXT_STC aldrinTablesInfo_overrideBobk[] =
{
    {CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_METERING_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(ALDRIN_NUM_IPLR_METERS_MAC)
    },
    {CPSS_DXCH_SIP5_TABLE_EGRESS_POLICER_METERING_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(ALDRIN_NUM_EPLR_METERS_MAC)
    },

    {CPSS_DXCH_SIP5_15_TABLE_INGRESS_POLICER_0_METERING_CONFIG_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(ALDRIN_NUM_IPLR_METERS_MAC)
    },
    {CPSS_DXCH_SIP5_15_TABLE_EGRESS_POLICER_METERING_CONFIG_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(ALDRIN_NUM_EPLR_METERS_MAC)
    },

    {CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_HIERARCHICAL_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(ALDRIN_NUM_IPLR_METERS_MAC)
    },

    {CPSS_DXCH_SIP5_15_TABLE_INGRESS_POLICER_0_METERING_CONFORM_SIGN_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(ALDRIN_NUM_IPLR_METERS_MAC)
    },
    {CPSS_DXCH_SIP5_15_TABLE_INGRESS_POLICER_1_METERING_CONFORM_SIGN_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(ALDRIN_NUM_IPLR_METERS_MAC)
    },
    {CPSS_DXCH_SIP5_15_TABLE_EGRESS_POLICER_METERING_CONFORM_SIGN_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(ALDRIN_NUM_EPLR_METERS_MAC)
    },
    {CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_COUNTING_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(ALDRIN_NUM_IPLR_METERS_MAC)
    },
    {CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_COUNTING_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(ALDRIN_NUM_IPLR_METERS_MAC)
    },
    {CPSS_DXCH_SIP5_TABLE_EGRESS_POLICER_COUNTING_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(ALDRIN_NUM_EPLR_METERS_MAC)
    },
    {CPSS_DXCH_TABLE_FDB_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(ALDRIN_NUM_FDB_MAC)
    },
};


typedef enum{
    ENTRY_TYPE_NOT_VALID_E/* indication that table is not valid (0 entries)*/

    ,ENTRY_TYPE_EPORT_E
    ,ENTRY_TYPE_PHY_PORT_E
    ,ENTRY_TYPE_MAX_PHY_PORT_E
    ,ENTRY_TYPE_DEFAULT_EPORT_E
    ,ENTRY_TYPE_EVLAN_MEMBERS_E /* evlan table that hold port members (subject to 'port mode') */
    ,ENTRY_TYPE_EVLAN_E
    ,ENTRY_TYPE_EVIDX_E
    ,ENTRY_TYPE_VIDX_E
    ,ENTRY_TYPE_STG_MEMBERS_E /* stg table that hold port members (subject to 'port mode') */
    ,ENTRY_TYPE_L2_ECMP_E
    ,ENTRY_TYPE_L2_LTT_ECMP_E
    ,ENTRY_TYPE_L3_ECMP_E
    ,ENTRY_TYPE_TUNNEL_START_E
    ,ENTRY_TYPE_PORT_ISOLATION_E
    ,ENTRY_TYPE_IPVX_NH_E
    ,ENTRY_TYPE_LPM_VRFID_E
    ,ENTRY_TYPE_PBR_LEAF_E
    ,ENTRY_TYPE_MLL_LTT_E
    ,ENTRY_TYPE_MLL_PAIRS_E     /* 2 MLL entries in single line */
    ,ENTRY_TYPE_IPLR_METERING_E
    ,ENTRY_TYPE_EPLR_METERING_E
    ,ENTRY_TYPE_IPLR_IPFIX_E
    ,ENTRY_TYPE_EPLR_IPFIX_E
    ,ENTRY_TYPE_IPLR_MAX_OF_COUNTING_IPFIX_E /* max between counting and ipfix */
    ,ENTRY_TYPE_EPLR_MAX_OF_COUNTING_IPFIX_E /* max between counting and ipfix */
    ,ENTRY_TYPE_IOAM_E
    ,ENTRY_TYPE_EOAM_E
    ,ENTRY_TYPE_TXQ_QUEUES_E
    ,ENTRY_TYPE_TXQ_PORTS_E
    ,ENTRY_TYPE_LOC_E
    ,ENTRY_TYPE_EXPLICIT_NUM_E
    ,ENTRY_TYPE_SOURCE_ID_MEMBERS_E
    ,ENTRY_TYPE_NON_TRUNK_MEMBERS_E
    ,ENTRY_TYPE_TXQ_DQ_PORTS_E
    ,ENTRY_TYPE_MAX_EPORT_EVLANS_E/* max between eport and evlans */
    ,ENTRY_TYPE_QCN_E
    ,ENTRY_TYPE_QOS_PROFILES_E
    ,ENTRY_TYPE_THREAD_ID_NUM_E
    ,ENTRY_TYPE_DP_PORTS_NUM_E /* DP ports per DP */
    ,ENTRY_TYPE_8_TC_NUM_E     /* per 8 TC (traffic class) */
    ,ENTRY_TYPE_PSI_PORT_E     /* number of PSI ports */
    ,ENTRY_TYPE_DP_Q_E     /* number of queues  in DP */
    ,ENTRY_TYPE_FDB_E      /* max number of HW FDB entries */
    ,ENTRY_TYPE_EM_E       /* max number of HW EM  entries */
    ,ENTRY_TYPE_TCAM_E     /* number of tcam entries (10B rules) */
    ,ENTRY_TYPE_PDX_QUEUE_GROUP_NUM_E   /* number of entries in 'DX Queue Group Map 0' in PDX unit (CPSS_DXCH_SIP6_TXQ_PDX_DX_QGRPMAP_E) */
    ,ENTRY_TYPE_DP_PER_TILE_E/* number of  data path per tile */
    ,ENTRY_TYPE_MAC_2_ME_E
    ,ENTRY_TYPE_STREAM_NUM_E        /* number of streams : in Ironman = 2K */
    ,ENTRY_TYPE_STREAM_SRF_NUM_E    /* number of SRF (stream recovery functions) : in Ironman = 1K */
    ,ENTRY_TYPE_STREAM_SRF_HIST_NUM_E    /* number of SRF history buffers : in Ironman = 1K */
    /* must be last */
    ,ENTRY_TYPE___LAST__E

}ENTRY_TYPE_ENT;

/* value to field TABLE_NUM_ENTRIES_INFO_STC::divider to indicate that 'multi'
   times the number of entries needed (without impact on prvCpssDxChTableEngineMultiEntriesInLineIndexAndGlobalBitConvert())
   caller to 'table entry' is explicitly set index in range 0..multi*'num entries' (where : 'num entries' derived from entryType)
*/
#define MULITIPLE_INDICATION_MAC(multi) (FRACTION_INDICATION_CNS | (multi))

typedef struct{
    CPSS_DXCH_TABLE_ENT tableType;
    ENTRY_TYPE_ENT  entryType;
    GT_U32   divider;/* number of 'info' per entry.
                        for example: entry with 8 eports per entry , divide = 8
                       NOTE: value 0 or 1 are ignored.
                       NOTE: if value hold FRACTION_INDICATION_CNS then logic
                            is 'multiplier' (instead of divider)

                       NOTE: if value hold VERTICAL_INDICATION_CNS then logic
                        the x entries in the first 'line' are not : 0,1
                        the x entries in the first 'line' are     : 0,depth

                        meaning that line y start with 'entry' y

                        NOTE: if value FRACTION_HALF_TABLE_INDICATION_CNS
                        meaning that single logical entry is spread on 2 half of the table.
                        in indexes : x,(x+'depth')

                        */
    GT_U32          additionalFixedNumberOfEntries;/* additional number of entries.
                            like PCL configuration table that hold 4K + 'phy ports' entries
                            */
}TABLE_NUM_ENTRIES_INFO_STC;

#define NOT_VALID_TABLE_MAC(table) \
    {table,ENTRY_TYPE_NOT_VALID_E,0,0}
/*xxx*/
/* SIP5_20 : info about the number of entries in the tables */
static const TABLE_NUM_ENTRIES_INFO_STC   sip5_20tableNumEntriesInfoArr[] =
{
    /* L2i */
    {CPSS_DXCH_SIP5_TABLE_BRIDGE_INGRESS_EPORT_E,
        ENTRY_TYPE_EPORT_E,0,0}
    ,{CPSS_DXCH_SIP5_TABLE_BRIDGE_INGRESS_EPORT_LEARN_PRIO_E,
        ENTRY_TYPE_EPORT_E,8,0}/* 8 eports per entry */
    ,{CPSS_DXCH_SIP5_TABLE_BRIDGE_PHYSICAL_PORT_E,
        ENTRY_TYPE_EXPLICIT_NUM_E,0,256}/* supports only 256 ports although the device may work in 512 ports mode */
    ,{CPSS_DXCH_SIP5_TABLE_BRIDGE_PHYSICAL_PORT_RATE_LIMIT_COUNTERS_E,
        ENTRY_TYPE_EXPLICIT_NUM_E,0,256}/* supports only 256 ports although the device may work in 512 ports mode */
    ,{CPSS_DXCH_LION_TABLE_STG_INGRESS_E,
        ENTRY_TYPE_STG_MEMBERS_E,VERTICAL_INDICATION_CNS | 2,0}/*2 STGs per entry (each support 256 ports * 2 bits)*/
    ,{CPSS_DXCH_SIP5_TABLE_INGRESS_SPAN_STATE_GROUP_INDEX_E,
        ENTRY_TYPE_EVLAN_E,0,0}
    ,{CPSS_DXCH_SIP5_TABLE_INGRESS_BRIDGE_PORT_MEMBERS_E,
        ENTRY_TYPE_EVLAN_MEMBERS_E,VERTICAL_INDICATION_CNS | 2,0}/*2 vlans per entry*/
    ,{CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
        ENTRY_TYPE_EVLAN_E,0,0}

    /* tti */
    ,{CPSS_DXCH_SIP5_TABLE_POST_TTI_LOOKUP_INGRESS_EPORT_E,
        ENTRY_TYPE_EPORT_E,0,0}
    ,{CPSS_DXCH_SIP5_TABLE_TTI_PHYSICAL_PORT_ATTRIBUTE_E,
        ENTRY_TYPE_PHY_PORT_E,0,0}
    ,{CPSS_DXCH_SIP5_20_TABLE_TTI_PHYSICAL_PORT_ATTRIBUTE_2_E,
        ENTRY_TYPE_PHY_PORT_E,0,0}
    ,{CPSS_DXCH_SIP5_20_TABLE_TTI_QCN_TO_TIMER_PAUSE_MAP_E,
        ENTRY_TYPE_QCN_E,0,0}

    /* epcl */
    ,{CPSS_DXCH2_TABLE_EGRESS_PCL_CONFIG_E,
        ENTRY_TYPE_EXPLICIT_NUM_E,0,_4K+512}/* supports 512 additional to 4K (BC3,Aldrin2) */

    /* eq */
    ,{CPSS_DXCH_SIP5_TABLE_EQ_INGRESS_EPORT_E,
        ENTRY_TYPE_EPORT_E,4,0}/* 4 eports per entry */
    ,{CPSS_DXCH_SIP5_TABLE_EQ_L2_ECMP_LTT_E,
        ENTRY_TYPE_L2_LTT_ECMP_E,0,0}
    ,{CPSS_DXCH_SIP5_TABLE_EQ_L2_ECMP_E,
        ENTRY_TYPE_L2_ECMP_E,0,0}
    ,{CPSS_DXCH_SIP5_TABLE_EQ_TX_PROTECTION_E,
        ENTRY_TYPE_EPORT_E,32,0}/* 32 eports per entry */
    ,{CPSS_DXCH_SIP5_TABLE_EQ_EPORT_TO_LOC_MAPPING_E,
        ENTRY_TYPE_EPORT_E,2,0}/* 2 eports per entry */
    ,{CPSS_DXCH_SIP5_TABLE_EQ_EPORT_TO_PHYSICAL_PORT_TARGET_MAPPING_E,
        ENTRY_TYPE_EPORT_E,0,0}
    ,{CPSS_DXCH_SIP5_TABLE_EQ_INGRESS_MIRROR_PHYSICAL_PORT_E,
        ENTRY_TYPE_PHY_PORT_E,8,0}/* 8 ports in entry */
    ,{CPSS_DXCH_SIP5_TABLE_EQ_INGRESS_STC_PHYSICAL_PORT_E,
        ENTRY_TYPE_PHY_PORT_E,0,0}

    /*ERMRK*/
    ,{CPSS_DXCH_SIP5_TABLE_ERMRK_PTP_LOCAL_ACTION_E,
        ENTRY_TYPE_PHY_PORT_E,(FRACTION_INDICATION_CNS | 16),0}/* 16 PtpMessageType */
    ,{CPSS_DXCH_SIP5_TABLE_ERMRK_PTP_TARGET_PORT_E,
        ENTRY_TYPE_PHY_PORT_E,0,0}
    ,{CPSS_DXCH_SIP5_TABLE_ERMRK_PTP_SOURCE_PORT_E,
        ENTRY_TYPE_PHY_PORT_E,0,0}

    /*ha*/
    ,{CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E,
        ENTRY_TYPE_EPORT_E,0,0}
    ,{CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_2_E,
        ENTRY_TYPE_EPORT_E,0,0}

    ,{CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_1_E,
        ENTRY_TYPE_PHY_PORT_E,0,0}
    ,{CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
        ENTRY_TYPE_PHY_PORT_E,0,0}

    ,{CPSS_DXCH2_TABLE_TUNNEL_START_CONFIG_E,
        ENTRY_TYPE_TUNNEL_START_E,0,0}

    ,{CPSS_DXCH3_TABLE_EGRESS_VLAN_TRANSLATION_E,
        ENTRY_TYPE_EVLAN_E,0,0}

    ,{CPSS_DXCH_SIP5_TABLE_HA_PTP_DOMAIN_E,
        ENTRY_TYPE_MAX_PHY_PORT_E , (FRACTION_INDICATION_CNS | 5),0} /* each port with 5 domains */

    /* ipcl */
    ,{CPSS_DXCH_TABLE_PCL_CONFIG_E,
        ENTRY_TYPE_EXPLICIT_NUM_E,0,_4K+512}/* supports 512 additional to 4K (BC3,Aldrin2) */
    ,{CPSS_DXCH_XCAT2_TABLE_INGRESS_PCL_LOOKUP01_CONFIG_E,
        ENTRY_TYPE_EXPLICIT_NUM_E,0,_4K+512}/* supports 512 additional to 4K (BC3,Aldrin2) */
    ,{CPSS_DXCH_XCAT_TABLE_INGRESS_PCL_LOOKUP1_CONFIG_E,
        ENTRY_TYPE_EXPLICIT_NUM_E,0,_4K+512}/* supports 512 additional to 4K (BC3,Aldrin2) */

    /* ipvx*/
    ,{CPSS_DXCH_SIP5_TABLE_IPVX_INGRESS_EPORT_E,
        ENTRY_TYPE_EPORT_E,16,0}/* 16 eports per entry */
    ,{CPSS_DXCH_SIP5_TABLE_IPVX_ROUTER_NEXTHOP_E,
        ENTRY_TYPE_IPVX_NH_E,0,0}
    ,{CPSS_DXCH_SIP5_TABLE_IPVX_EVLAN_E,
        ENTRY_TYPE_EVLAN_E,0,0}
    ,{CPSS_DXCH_SIP5_TABLE_IPVX_ROUTER_NEXTHOP_AGE_BITS_E,
        ENTRY_TYPE_IPVX_NH_E , 32,0}/* 32 NH per entry */

    /* mll */
    ,{CPSS_DXCH_SIP5_TABLE_IP_MLL_E,
        ENTRY_TYPE_MLL_PAIRS_E,0,0}

    ,{CPSS_DXCH_SIP5_TABLE_L2_MLL_E,
        ENTRY_TYPE_MLL_PAIRS_E,0,0}

    ,{CPSS_DXCH_SIP5_TABLE_L2_MLL_LTT_E,
        ENTRY_TYPE_MLL_LTT_E,0,0}

    /* egf */
        /*qag*/
    ,{CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_EGRESS_EPORT_E,
        ENTRY_TYPE_EPORT_E,0,0}
    ,{CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_EGRESS_VLAN_E,
        ENTRY_TYPE_EVLAN_MEMBERS_E, (FRACTION_INDICATION_CNS | 16),0}/*each evlan with 16 lines*/
    ,{CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_E,
        ENTRY_TYPE_PHY_PORT_E,0,0}
    ,{CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_FWD_FROM_CPU_TO_LOOPBACK_MAPPER_E,
        ENTRY_TYPE_PHY_PORT_E,0,0}
    ,{CPSS_DXCH_SIP5_20_TABLE_EGF_QAG_PORT_TARGET_ATTRIBUTES_E,
        ENTRY_TYPE_PHY_PORT_E,0,0}
    ,{CPSS_DXCH_SIP5_20_TABLE_EGF_QAG_PORT_SOURCE_ATTRIBUTES_E,
        ENTRY_TYPE_PHY_PORT_E,0,0}

        /*eft*/
    ,{CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_EFT_FAST_STACK_FAILOVER_SECONDARY_TARGET_PORT_E,
        ENTRY_TYPE_PHY_PORT_E,0,0}
        /*sht*/
    ,{CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_VLAN_SPANNING_E,
        ENTRY_TYPE_EVLAN_E,0,0}
    ,{CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_EGRESS_EPORT_E,
        ENTRY_TYPE_EPORT_E,0,0}
    ,{CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_VLAN_ATTRIBUTES_E,
        ENTRY_TYPE_EVLAN_E,0,0}
    ,{CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_VLAN_MAPPER_E,
        ENTRY_TYPE_EVLAN_E,0,0}

    ,{CPSS_DXCH_LION_TABLE_STG_EGRESS_E,
        ENTRY_TYPE_STG_MEMBERS_E,FRACTION_HALF_TABLE_INDICATION_CNS | 1/*place holder*/,0}

    ,{CPSS_DXCH_LION_TABLE_VLAN_EGRESS_E,
        ENTRY_TYPE_EVLAN_MEMBERS_E,FRACTION_HALF_TABLE_INDICATION_CNS | 1/*place holder*/,0}

    ,{CPSS_DXCH_LION_TABLE_TXQ_SOURCE_ID_MEMBERS_E,
        ENTRY_TYPE_SOURCE_ID_MEMBERS_E,FRACTION_HALF_TABLE_INDICATION_CNS | 1/*place holder*/,0}

    ,{CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_NON_TRUNK_MEMBERS2_E,
        ENTRY_TYPE_NON_TRUNK_MEMBERS_E,FRACTION_HALF_TABLE_INDICATION_CNS | 1/*place holder*/,0}


    ,{CPSS_DXCH_SIP5_TABLE_ECMP_E,
        ENTRY_TYPE_L3_ECMP_E,0,0}

    /* PLR */
    ,{CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_METERING_E,
        ENTRY_TYPE_IPLR_METERING_E,0,0}
    ,{CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_METERING_E,
        ENTRY_TYPE_NOT_VALID_E,0,0}/*not valid !!!*/
    ,{CPSS_DXCH_SIP5_TABLE_EGRESS_POLICER_METERING_E,
        ENTRY_TYPE_NOT_VALID_E,0,0}/*not valid !!!*/
    ,{CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_HIERARCHICAL_E,/*only in PLR0*/
        ENTRY_TYPE_IPLR_METERING_E,0,0}
    ,{CPSS_DXCH_SIP5_15_TABLE_INGRESS_POLICER_0_METERING_CONFIG_E,
        ENTRY_TYPE_IPLR_METERING_E,0,0}
    ,{CPSS_DXCH_SIP5_15_TABLE_INGRESS_POLICER_1_METERING_CONFIG_E,
        ENTRY_TYPE_NOT_VALID_E,0,0}/*not valid !!!*/
    ,{CPSS_DXCH_SIP5_15_TABLE_EGRESS_POLICER_METERING_CONFIG_E,
        ENTRY_TYPE_NOT_VALID_E,0,0}/*not valid !!!*/

    ,{CPSS_DXCH_SIP5_15_TABLE_INGRESS_POLICER_0_METERING_CONFORM_SIGN_E,
        ENTRY_TYPE_IPLR_METERING_E,0,0}
    ,{CPSS_DXCH_SIP5_15_TABLE_INGRESS_POLICER_1_METERING_CONFORM_SIGN_E,
        ENTRY_TYPE_IPLR_METERING_E,0,0}
    ,{CPSS_DXCH_SIP5_15_TABLE_EGRESS_POLICER_METERING_CONFORM_SIGN_E,
        ENTRY_TYPE_EPLR_METERING_E,0,0}

    ,{CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_IPFIX_ALERT_WRAPAROUND_E,
        ENTRY_TYPE_IPLR_IPFIX_E,32,0} /* 32: 1 indication bit per IPFIX entry */
    ,{CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_IPFIX_ALERT_WRAPAROUND_E,
        ENTRY_TYPE_IPLR_IPFIX_E,32,0} /* 32: 1 indication bit per IPFIX entry */
    ,{CPSS_DXCH_SIP5_TABLE_EGRESS_POLICER_IPFIX_ALERT_WRAPAROUND_E,
        ENTRY_TYPE_EPLR_IPFIX_E,32,0} /* 32: 1 indication bit per IPFIX entry */

    ,{CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_IPFIX_ALERT_AGING_E,
        ENTRY_TYPE_IPLR_IPFIX_E,32,0} /* 32: 1 indication bit per IPFIX entry */
    ,{CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_IPFIX_ALERT_AGING_E,
        ENTRY_TYPE_IPLR_IPFIX_E,32,0} /* 32: 1 indication bit per IPFIX entry */
    ,{CPSS_DXCH_SIP5_TABLE_EGRESS_POLICER_IPFIX_ALERT_AGING_E,
        ENTRY_TYPE_EPLR_IPFIX_E,32,0} /* 32: 1 indication bit per IPFIX entry */


    ,{CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_COUNTING_E,
        ENTRY_TYPE_IPLR_MAX_OF_COUNTING_IPFIX_E,0,0}
    ,{CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_COUNTING_E,
        ENTRY_TYPE_IPLR_MAX_OF_COUNTING_IPFIX_E,0,0}
    ,{CPSS_DXCH_SIP5_TABLE_EGRESS_POLICER_COUNTING_E,
        ENTRY_TYPE_EPLR_MAX_OF_COUNTING_IPFIX_E,0,0}

    ,{CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_E_ATTRIBUTES_E,
        ENTRY_TYPE_MAX_EPORT_EVLANS_E,0,0}
    ,{CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_E_ATTRIBUTES_E,
        ENTRY_TYPE_MAX_EPORT_EVLANS_E,0,0}
    ,{CPSS_DXCH_SIP5_TABLE_EGRESS_POLICER_E_ATTRIBUTES_E,
        ENTRY_TYPE_MAX_EPORT_EVLANS_E,0,0}

    ,NOT_VALID_TABLE_MAC(CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_COUNTING_E)
    ,NOT_VALID_TABLE_MAC(CPSS_DXCH_SIP5_TABLE_EGRESS_POLICER_COUNTING_E)

    /* OAM */
    ,{CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_KEEPALIVE_AGING_E,
        ENTRY_TYPE_IOAM_E,32,0}/*32 oam per line */
    ,{CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_MEG_EXCEPTION_E,
         ENTRY_TYPE_IOAM_E,32,0}/*32 oam per line */
    ,{CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_SOURCE_INTERFACE_EXCEPTION_E,
         ENTRY_TYPE_IOAM_E,32,0}/*32 oam per line */
    ,{CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_INVALID_KEEPALIVE_HASH_E,
         ENTRY_TYPE_IOAM_E,32,0}/*32 oam per line */
    ,{CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_KEEPALIVE_EXCESS_E,
         ENTRY_TYPE_IOAM_E,32,0}/*32 oam per line */
    ,{CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_EXCEPTION_SUMMARY_E,
         ENTRY_TYPE_IOAM_E,32,0}/*32 oam per line */
    ,{CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_RDI_STATUS_CHANGE_EXCEPTION_E,
         ENTRY_TYPE_IOAM_E,32,0}/*32 oam per line */
    ,{CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_TX_PERIOD_EXCEPTION_E,
         ENTRY_TYPE_IOAM_E,32,0}/*32 oam per line */
    ,{CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_E,
         ENTRY_TYPE_IOAM_E,0,0}

    ,{CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_KEEPALIVE_AGING_E,
         ENTRY_TYPE_EOAM_E,32,0}/*32 oam per line */
    ,{CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_MEG_EXCEPTION_E,
         ENTRY_TYPE_EOAM_E,32,0}/*32 oam per line */
    ,{CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_SOURCE_INTERFACE_EXCEPTION_E,
         ENTRY_TYPE_EOAM_E,32,0}/*32 oam per line */
    ,{CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_INVALID_KEEPALIVE_HASH_E,
         ENTRY_TYPE_EOAM_E,32,0}/*32 oam per line */
    ,{CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_KEEPALIVE_EXCESS_E,
         ENTRY_TYPE_EOAM_E,32,0}/*32 oam per line */
    ,{CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_EXCEPTION_SUMMARY_E,
         ENTRY_TYPE_EOAM_E,32,0}/*32 oam per line */
    ,{CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_RDI_STATUS_CHANGE_EXCEPTION_E,
         ENTRY_TYPE_EOAM_E,32,0}/*32 oam per line */
    ,{CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_TX_PERIOD_EXCEPTION_E,
         ENTRY_TYPE_EOAM_E,32,0}/*32 oam per line */
    ,{CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_E,
         ENTRY_TYPE_EOAM_E,0,0}

    ,{CPSS_DXCH_SIP5_TABLE_TTI_PTP_COMMAND_E,
        ENTRY_TYPE_NOT_VALID_E,0,0}

    ,{CPSS_DXCH_SIP5_TABLE_TXQ_LINK_LIST_COUNTERS_Q_MAIN_DESC_E,
         ENTRY_TYPE_NOT_VALID_E,0,0} /* does not exist in BC3 */
    ,{CPSS_DXCH_SIP5_TABLE_TAIL_DROP_COUNTERS_Q_MAIN_BUFF_E,
         ENTRY_TYPE_TXQ_QUEUES_E,0,0}
    ,{CPSS_DXCH_SIP5_TABLE_TAIL_DROP_COUNTERS_Q_MAIN_MC_BUFF_E,
         ENTRY_TYPE_NOT_VALID_E,0,0} /* does not exist in BC3 */


    ,{CPSS_DXCH_SIP5_TABLE_EQ_PROTECTION_LOC_E,
        ENTRY_TYPE_LOC_E,32,0}/*32 entries per line */
    ,{CPSS_DXCH_SIP5_20_TABLE_EQ_TX_PROTECTION_LOC_E,
        ENTRY_TYPE_LOC_E,32,0}/*32 entries per line */

    ,{CPSS_DXCH_SIP5_20_TABLE_EGF_QAG_TC_DP_MAPPER_E,
        ENTRY_TYPE_EXPLICIT_NUM_E,0,_4K}/*explicit state that 4K entries*/

    /*CPFC*/
    ,{CPSS_DXCH_SIP5_TABLE_PFC_LLFC_COUNTERS_E,
        ENTRY_TYPE_EXPLICIT_NUM_E,0,624}/*explicit state that 624 entries*/

    /*TXQ DQ */
    ,{CPSS_DXCH_LION_TABLE_TXQ_EGRESS_STC_E,                        /*DQ[0]*/
         ENTRY_TYPE_TXQ_DQ_PORTS_E,0,0}
    ,{CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TXQ_EGRESS_STC__DQ_1_E,/*DQ[1]*/
         ENTRY_TYPE_TXQ_DQ_PORTS_E,0,0}
    ,{CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TXQ_EGRESS_STC__DQ_2_E,/*DQ[2]*/
         ENTRY_TYPE_TXQ_DQ_PORTS_E,0,0}
    ,{CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TXQ_EGRESS_STC__DQ_3_E,/*DQ[3]*/
         ENTRY_TYPE_TXQ_DQ_PORTS_E,0,0}
    ,{CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TXQ_EGRESS_STC__DQ_4_E,/*DQ[4]*/
         ENTRY_TYPE_TXQ_DQ_PORTS_E,0,0}
    ,{CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TXQ_EGRESS_STC__DQ_5_E,/*DQ[5]*/
         ENTRY_TYPE_TXQ_DQ_PORTS_E,0,0}

    ,{CPSS_DXCH_LION_TABLE_TXQ_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG_E,                    /*DQ[0]*/
         ENTRY_TYPE_TXQ_DQ_PORTS_E,0,0}
    ,{CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG__DQ_1_E,/*DQ[1]*/
         ENTRY_TYPE_TXQ_DQ_PORTS_E,0,0}
    ,{CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG__DQ_2_E,/*DQ[2]*/
         ENTRY_TYPE_TXQ_DQ_PORTS_E,0,0}
    ,{CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG__DQ_3_E,/*DQ[3]*/
         ENTRY_TYPE_TXQ_DQ_PORTS_E,0,0}
    ,{CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG__DQ_4_E,/*DQ[4]*/
         ENTRY_TYPE_TXQ_DQ_PORTS_E,0,0}
    ,{CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG__DQ_5_E,/*DQ[5]*/
         ENTRY_TYPE_TXQ_DQ_PORTS_E,0,0}

    ,{CPSS_DXCH_LION_TABLE_TXQ_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG_E,                    /*DQ[0]*/
         ENTRY_TYPE_TXQ_DQ_PORTS_E,0,0}
    ,{CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG__DQ_1_E,/*DQ[1]*/
         ENTRY_TYPE_TXQ_DQ_PORTS_E,0,0}
    ,{CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG__DQ_2_E,/*DQ[2]*/
         ENTRY_TYPE_TXQ_DQ_PORTS_E,0,0}
    ,{CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG__DQ_3_E,/*DQ[3]*/
         ENTRY_TYPE_TXQ_DQ_PORTS_E,0,0}
    ,{CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG__DQ_4_E,/*DQ[4]*/
         ENTRY_TYPE_TXQ_DQ_PORTS_E,0,0}
    ,{CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG__DQ_5_E,/*DQ[5]*/
         ENTRY_TYPE_TXQ_DQ_PORTS_E,0,0}

    /* pipe 1 support */
    ,{CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_EQ_INGRESS_STC_PHYSICAL_PORT_PIPE_1_E,
        ENTRY_TYPE_PHY_PORT_E,0,0}

    /* LPM pipe 1 */
    ,{CPSS_DXCH_SIP5_20_TABLE_LPM_MEM_PIPE_1_E,
        ENTRY_TYPE_EXPLICIT_NUM_E,0,(20*_16K)}   /* 20 blocks of 16K each */

    ,{CPSS_DXCH_SIP5_TABLE_BMA_PORT_MAPPING_E,
        ENTRY_TYPE_PHY_PORT_E,0,0}

    /* must be last */
    ,{CPSS_DXCH_TABLE_LAST_E , ENTRY_TYPE___LAST__E , 0, 0}
};

/*xxx*/
/* Aldrin2 table - in runtime copy table from this of Bobcat3 ... and continue from there ... */
static const GT_U32 aldrin2TableInfoSize = CPSS_DXCH_SIP5_25_TABLE_LAST_E;

/* size of aldrin2directAccessTableInfo table */
static const GT_U32 aldrin2directAccessTableInfoSize = PRV_DXCH_ALDRIN2_TABLE___LAST_E;

/* Aldrin2 direct table information , that are new (not exists in BC3) */
/* register new tables that not exists in BC3 !!!! */
static const BOBK_TABLES_INFO_DIRECT_NEW_INDEX_STC aldrin2directAccessTableInfo_new[] =
{
    {PRV_DXCH_ALDRIN2_TABLE_IPVX_ROUTER_ECMP_POINTER_E,
        {CPSS_DXCH_SIP5_25_TABLE_IPVX_ROUTER_ECMP_POINTER_E, PRV_CPSS_DXCH_UNIT_IPVX_E,
        {0x00300000, BITS_TO_BYTES_ALIGNMENT_MAC(15),NOT_MODIFIED_VALUE_CNS},GT_FALSE}
    }
    ,{PRV_DXCH_ALDRIN2_TABLE_TAIL_DROP_PORT_BUFFER_FILL_LEVEL_E,
        {CPSS_DXCH_SIP5_25_TABLE_TAIL_DROP_PORT_MAX_BUFFER_FILL_LEVEL_E, PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E,
        {0x000E0000, BITS_TO_BYTES_ALIGNMENT_MAC(20),NOT_MODIFIED_VALUE_CNS},GT_FALSE}
    }
    ,{PRV_DXCH_ALDRIN2_TABLE_TAIL_DROP_QUEUE_BUFFER_FILL_LEVEL_E,
        {CPSS_DXCH_SIP5_25_TABLE_TAIL_DROP_QUEUE_MAX_BUFFER_FILL_LEVEL_E, PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E,
        {0x000E8000, BITS_TO_BYTES_ALIGNMENT_MAC(20),NOT_MODIFIED_VALUE_CNS},GT_FALSE}
    }
    /* NOTE: addresses in this table MUST be '0' based (relative to unit) and
       not global address of the device*/
};
/* Aldrin2 direct table information , that need to override BC3 info from aldrin2directAccessTableInfo */
static const PRV_CPSS_DXCH_TABLES_INFO_DIRECT_EXT_STC aldrin2directAccessTableInfo_overrideBobcat3[] =
{
    /* NOTE: addresses in this table MUST be '0' based (relative to unit) and
       not global address of the device*/

    /* NOTE: the alignment when stated NOT_MODIFIED_VALUE_CNS :
        is taken auto calculated from bobcat3TablesInfo_overrideBobk[] */
    {CPSS_DXCH_TABLE_LAST_E, {0,0,0}}
};
/*  table information , that need to override BC3 info from aldrin2TablesInfo */
/* NOTE : the entries in this array need to be matched with entries in
   bobcat3TablesInfo , according to field */
static const PRV_CPSS_DXCH_TABLES_INFO_EXT_STC aldrin2TablesInfo_overrideBobcat3[] =
{
    /*L2i*/
    {CPSS_DXCH_SIP5_TABLE_INGRESS_BRIDGE_PORT_MEMBERS_E,
        OVERRIDE_INFO_WIDTH_MAC(BMP_128_PORTS_CNS)/*maxNumOfEntries comes from sip5_25tableNumEntriesInfoArr*/
    },
    {CPSS_DXCH_LION_TABLE_STG_INGRESS_E,
        OVERRIDE_INFO_WIDTH_MAC(2*BMP_128_PORTS_CNS)/*maxNumOfEntries comes from sip5_25tableNumEntriesInfoArr*/
    },

    /*SHT*/
    {CPSS_DXCH_LION_TABLE_VLAN_EGRESS_E,
        OVERRIDE_INFO_WIDTH_MAC(BMP_128_PORTS_CNS)
    },
    {CPSS_DXCH_TABLE_MULTICAST_E,
        OVERRIDE_INFO_WIDTH_MAC(BMP_128_PORTS_CNS)
    },
    {CPSS_DXCH_LION_TABLE_TXQ_NON_TRUNK_MEMBERS_E,
        OVERRIDE_INFO_WIDTH_MAC(BMP_128_PORTS_CNS)
    },
    {CPSS_DXCH_LION_TABLE_TXQ_DESIGNATED_PORT_E,
        OVERRIDE_INFO_WIDTH_MAC(BMP_128_PORTS_CNS)
    },
    {CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_NON_TRUNK_MEMBERS2_E,
        OVERRIDE_INFO_WIDTH_MAC(BMP_128_PORTS_CNS)
    },
    {CPSS_DXCH_LION_TABLE_PORT_ISOLATION_L2_E,
        OVERRIDE_INFO_WIDTH_MAC(BMP_128_PORTS_CNS)
    },
    {CPSS_DXCH_LION_TABLE_PORT_ISOLATION_L3_E,
        OVERRIDE_INFO_WIDTH_MAC(BMP_128_PORTS_CNS)
    },
    {CPSS_DXCH_LION_TABLE_TXQ_SOURCE_ID_MEMBERS_E,
        OVERRIDE_INFO_WIDTH_MAC(BMP_128_PORTS_CNS)
    },
    {CPSS_DXCH_LION_TABLE_STG_EGRESS_E,
        OVERRIDE_INFO_WIDTH_MAC(BMP_128_PORTS_CNS)
    },
    {CPSS_DXCH_SIP5_TABLE_CN_SAMPLE_INTERVALS_E,
        OVERRIDE_INFO_NUM_ENTRIES_AND_WIDTH_MAC(800/*maxNumOfEntries*/,40/*entrySize_inBits*/)
    },
};
/* SIP5_25 : info about the number of entries in the tables (override sip5_20tableNumEntriesInfoArr) */
static const TABLE_NUM_ENTRIES_INFO_STC   sip5_25tableNumEntriesInfoArr[] =
{
    /* L2i */
     {CPSS_DXCH_LION_TABLE_STG_INGRESS_E,
        ENTRY_TYPE_STG_MEMBERS_E,0,0}/*single STGs per entry (was 2 in sip_5_20)*/
    ,{CPSS_DXCH_SIP5_TABLE_INGRESS_BRIDGE_PORT_MEMBERS_E,
        ENTRY_TYPE_EVLAN_MEMBERS_E,0,0}/*single vlans per entry (was 2 in sip_5_20)*/
    ,{CPSS_DXCH_SIP5_TABLE_BRIDGE_PHYSICAL_PORT_E,
        ENTRY_TYPE_PHY_PORT_E,0,0}/* supports 128 ports */
    ,{CPSS_DXCH_SIP5_TABLE_BRIDGE_PHYSICAL_PORT_RATE_LIMIT_COUNTERS_E,
        ENTRY_TYPE_PHY_PORT_E,0,0}

    /* ipvx */
    /* new table */
    ,{CPSS_DXCH_SIP5_25_TABLE_IPVX_ROUTER_ECMP_POINTER_E,
        ENTRY_TYPE_IPVX_NH_E,0,0}

    /* Max Buffer fill levels */
    ,{CPSS_DXCH_SIP5_25_TABLE_TAIL_DROP_PORT_MAX_BUFFER_FILL_LEVEL_E,
        ENTRY_TYPE_TXQ_PORTS_E,0,0}

    ,{CPSS_DXCH_SIP5_25_TABLE_TAIL_DROP_QUEUE_MAX_BUFFER_FILL_LEVEL_E,
        ENTRY_TYPE_TXQ_QUEUES_E,0,0}

    ,{CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_EGRESS_VLAN_E,
        ENTRY_TYPE_EVLAN_MEMBERS_E, (FRACTION_INDICATION_CNS | 8),0}/*each evlan with 8 lines (supporting 128 ports)*/

    /* LPM memory for half mode */
    ,{CPSS_DXCH_SIP5_20_TABLE_LPM_MEM_PIPE_1_E,
        ENTRY_TYPE_EXPLICIT_NUM_E,0,(20*_16K)}   /* 20 blocks of 16K each */

    /*CPFC*/
    ,{CPSS_DXCH_SIP5_TABLE_PFC_LLFC_COUNTERS_E,
        ENTRY_TYPE_EXPLICIT_NUM_E,0,800}/*explicit state that 800 entries*/

    /* tables below do not exist in Aldrin2 */
    ,NOT_VALID_TABLE_MAC(CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TXQ_EGRESS_STC__DQ_4_E)
    ,NOT_VALID_TABLE_MAC(CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TXQ_EGRESS_STC__DQ_5_E)
    ,NOT_VALID_TABLE_MAC(CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG__DQ_4_E)
    ,NOT_VALID_TABLE_MAC(CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG__DQ_5_E)
    ,NOT_VALID_TABLE_MAC(CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG__DQ_4_E)
    ,NOT_VALID_TABLE_MAC(CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG__DQ_5_E)
    ,NOT_VALID_TABLE_MAC(CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TTI_PORT_MAPPING__DQ_4_E)
    ,NOT_VALID_TABLE_MAC(CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TTI_PORT_MAPPING__DQ_5_E)
    ,NOT_VALID_TABLE_MAC(CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_EQ_INGRESS_STC_PHYSICAL_PORT_PIPE_1_E)

    /* must be last */
    ,{CPSS_DXCH_TABLE_LAST_E , ENTRY_TYPE___LAST__E , 0, 0}
};

/* Falcon table - in runtime copy table from this of Bobcat3 ... and continue from there ... */
static const GT_U32 falconTableInfoSize = CPSS_DXCH_SIP6_TABLE_LAST_E;

/* size of falcon_directAccessTableInfo table */
static const GT_U32 falcon_directAccessTableInfoSize = PRV_DXCH_FALCON_TABLE___LAST_E;

#define DUP_3_TABLES_TABLE(t1,t2,t3 , p1,p2,p3) \
    {t1,{p1,p2,p3}},                            \
    {t2,{p1,p2,p3}},                            \
    {t3,{p1,p2,p3}}

#define DUP_2_TABLES_TABLE(t1,t2 , p1,p2,p3)    \
    {t1,{p1,p2,p3}},                            \
    {t2,{p1,p2,p3}}


/* Falcon direct table information , that need to override Aldrin2 info from falcon_directAccessTableInfo */
static const PRV_CPSS_DXCH_TABLES_INFO_DIRECT_EXT_STC falcon_directAccessTableInfo_overrideAldrin2[] =
{
    /* NOTE: addresses in this table MUST be '0' based (relative to unit) and
       not global address of the device*/

    /* L2i */
    {CPSS_DXCH_LION_TABLE_STG_INGRESS_E,
        {
            NOT_MODIFIED_VALUE_CNS,
            BITS_TO_BYTES_ALIGNMENT_MAC(2 * BMP_1K_PORTS_CNS),/* 2 bits per port */
            NOT_MODIFIED_VALUE_CNS
        }
    },

    {CPSS_DXCH_SIP5_TABLE_INGRESS_BRIDGE_PORT_MEMBERS_E,
        {
            0x00340000,
            BITS_TO_BYTES_ALIGNMENT_MAC(BMP_1K_PORTS_CNS),/* 1 bit per port */
            NOT_MODIFIED_VALUE_CNS
        }
    },

    {CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E,
        {
            0x00440000,
            BITS_TO_BYTES_ALIGNMENT_MAC(102),
            NOT_MODIFIED_VALUE_CNS
        }
    },

    {CPSS_DXCH_SIP5_TABLE_BRIDGE_INGRESS_EPORT_E,
        {
            0x00500000,
            NOT_MODIFIED_VALUE_CNS,
            NOT_MODIFIED_VALUE_CNS
        }
    },
    /* lpm */
    {CPSS_DXCH_SIP5_TABLE_LPM_MEM_E,
        {
            NOT_MODIFIED_VALUE_CNS,
            BITS_TO_BYTES_ALIGNMENT_MAC(115),
            NOT_MODIFIED_VALUE_CNS
        }
    },
    {CPSS_DXCH_SIP5_TABLE_IPV4_VRF_E,
        {
            0x00F10000,
            NOT_MODIFIED_VALUE_CNS,
            NOT_MODIFIED_VALUE_CNS
        }
    },
    {CPSS_DXCH_SIP5_TABLE_IPV6_VRF_E,
        {
            0x00F20000,
            NOT_MODIFIED_VALUE_CNS,
            NOT_MODIFIED_VALUE_CNS
        }
    },
    {CPSS_DXCH_SIP5_TABLE_FCOE_VRF_E,
        {
            0x00F30000,
            NOT_MODIFIED_VALUE_CNS,
            NOT_MODIFIED_VALUE_CNS
        }
    },

    /* IPCL1,2 tables */
    {
        /* table for second stage lookup (first stage removed on Falcon) */
        CPSS_DXCH_TABLE_PCL_CONFIG_E,
        {
            0x00020000,
            BITS_TO_BYTES_ALIGNMENT_MAC(30),
            PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS
        }
    },

    {
        /* table for third stage lookup (first stage removed on Falcon) */
        CPSS_DXCH_XCAT2_TABLE_INGRESS_PCL_LOOKUP01_CONFIG_E,
        {
            0x00030000,
            BITS_TO_BYTES_ALIGNMENT_MAC(30),
            PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS
        }
    },

    {
        /* table for second stage lookup (first stage removed on Falcon) */
        CPSS_DXCH_SIP5_TABLE_IPCL0_UDB_SELECT_E,
        {
            0x00042000,
            BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_20_IPCL_UDB_SELECT_BITS_SIZE_CNS),
            PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS
        }
    },

    {
        /* table for third stage lookup (first stage removed on Falcon) */
        CPSS_DXCH_SIP5_TABLE_IPCL1_UDB_SELECT_E,
        {
            0x00044000,
            BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_20_IPCL_UDB_SELECT_BITS_SIZE_CNS),
            PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS
        }
    },

    /* HA */
    {CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E,
    {0x00080000, BITS_TO_BYTES_ALIGNMENT_MAC(96), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},
    {CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_2_E,
    {0x00060000, BITS_TO_BYTES_ALIGNMENT_MAC(25), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},
    {CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_1_E,
    {0x00001000, BITS_TO_BYTES_ALIGNMENT_MAC(27), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},
    {CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
    {0x00004000, BITS_TO_BYTES_ALIGNMENT_MAC(82), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},
    {CPSS_DXCH_SIP5_TABLE_HA_QOS_PROFILE_TO_EXP_E,
    {0x00008000, BITS_TO_BYTES_ALIGNMENT_MAC(3), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},
    {CPSS_DXCH_SIP5_TABLE_HA_EPCL_UDB_CONFIG_E,
    {0x00002000, BITS_TO_BYTES_ALIGNMENT_MAC(600), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},
    {CPSS_DXCH_SIP5_TABLE_HA_PTP_DOMAIN_E,
    {0x00040000, BITS_TO_BYTES_ALIGNMENT_MAC(35), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},
    {CPSS_DXCH_SIP5_TABLE_HA_GENERIC_TS_PROFILE_E,
    {0x00050000, BITS_TO_BYTES_ALIGNMENT_MAC(537), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},
    {CPSS_DXCH_SIP5_TABLE_HA_GLOBAL_MAC_SA_E,
    {0x00009000, BITS_TO_BYTES_ALIGNMENT_MAC(48), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},
    {CPSS_DXCH_TABLE_ROUTE_HA_MAC_SA_E,
    {0x00020000, BITS_TO_BYTES_ALIGNMENT_MAC(12), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},
    {CPSS_DXCH2_TABLE_TUNNEL_START_CONFIG_E,
    {0x00200000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_HA_TUNNEL_START_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

    /* SHT */
    {CPSS_DXCH_LION_TABLE_PORT_ISOLATION_L2_E,
    {0x00040000, BITS_TO_BYTES_ALIGNMENT_MAC(128), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},
    {CPSS_DXCH_LION_TABLE_PORT_ISOLATION_L3_E,
    {0x00050000, BITS_TO_BYTES_ALIGNMENT_MAC(128), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},
    {CPSS_DXCH_LION_TABLE_STG_EGRESS_E,
    {0x00060000, BITS_TO_BYTES_ALIGNMENT_MAC(128), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},
    {CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_NON_TRUNK_MEMBERS2_E,
    {0x00090000, BITS_TO_BYTES_ALIGNMENT_MAC(128), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},
    {CPSS_DXCH_LION_TABLE_TXQ_SOURCE_ID_MEMBERS_E,
    {0x00068000, BITS_TO_BYTES_ALIGNMENT_MAC(128), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},
    {CPSS_DXCH_TABLE_MULTICAST_E,
    {0x00070000, BITS_TO_BYTES_ALIGNMENT_MAC(128), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},
    {CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_DEVICE_MAP_TABLE_E,
    {0x000A0000, BITS_TO_BYTES_ALIGNMENT_MAC(14), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},
    {CPSS_DXCH_LION_TABLE_TXQ_DESIGNATED_PORT_E,
    {0x000A4000, BITS_TO_BYTES_ALIGNMENT_MAC(1024), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},
    {CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_EGRESS_EPORT_E,
    {0x00078000, BITS_TO_BYTES_ALIGNMENT_MAC(29), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},
    {CPSS_DXCH_LION_TABLE_TXQ_NON_TRUNK_MEMBERS_E,
    {0x000A8000, BITS_TO_BYTES_ALIGNMENT_MAC(1024), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},
    {CPSS_DXCH_LION_TABLE_VLAN_EGRESS_E,
    {0x00000000, BITS_TO_BYTES_ALIGNMENT_MAC(128), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},
    {CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_VLAN_ATTRIBUTES_E,
    {0x00080000, BITS_TO_BYTES_ALIGNMENT_MAC(6), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},
    {CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_VLAN_SPANNING_E,
    {0x00088000, BITS_TO_BYTES_ALIGNMENT_MAC(12), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

    /*QAG*/
    {CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_EGRESS_VLAN_E,
    {0x00000000, BITS_TO_BYTES_ALIGNMENT_MAC(48), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},
    {CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_EGRESS_EPORT_E,
    {0x00080000, BITS_TO_BYTES_ALIGNMENT_MAC(44), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},
    {CPSS_DXCH_SIP5_20_TABLE_EGF_QAG_TC_DP_MAPPER_E,
    {0x00090000, BITS_TO_BYTES_ALIGNMENT_MAC(10), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},
    {CPSS_DXCH_SIP5_20_TABLE_EGF_QAG_PORT_TARGET_ATTRIBUTES_E,
    {0x00096000, BITS_TO_BYTES_ALIGNMENT_MAC(18), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},
    {CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_E,
    {0x00097000, BITS_TO_BYTES_ALIGNMENT_MAC(15), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},
    {CPSS_DXCH_SIP5_20_TABLE_EGF_QAG_PORT_SOURCE_ATTRIBUTES_E,
    {0x00098000, BITS_TO_BYTES_ALIGNMENT_MAC(9), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},
    {CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_CPU_CODE_TO_LOOPBACK_MAPPER_E,
    {0x0009A000, BITS_TO_BYTES_ALIGNMENT_MAC(12), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

    /*EQ*/
    {CPSS_DXCH_SIP5_TABLE_EQ_INGRESS_MIRROR_PHYSICAL_PORT_E,
    {0x0000b400, NOT_MODIFIED_VALUE_CNS, NOT_MODIFIED_VALUE_CNS}},

    /*EFT*/
    {CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_EFT_FAST_STACK_FAILOVER_SECONDARY_TARGET_PORT_E,
    {0x00000000, BITS_TO_BYTES_ALIGNMENT_MAC(10), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

    /* table only in IPLR0 */
    {CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_HIERARCHICAL_E,
    {0x00020000, BITS_TO_BYTES_ALIGNMENT_MAC(14), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

    /* IPLR0,1, EPLR */
    DUP_3_TABLES_TABLE (
        CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_E_ATTRIBUTES_E,
        CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_E_ATTRIBUTES_E,
        CPSS_DXCH_SIP5_TABLE_EGRESS_POLICER_E_ATTRIBUTES_E,
        0x00040000, BITS_TO_BYTES_ALIGNMENT_MAC(32), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS),
    DUP_3_TABLES_TABLE (
        CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_REMARKING_E,
        CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_REMARKING_E,
        CPSS_DXCH_XCAT_TABLE_EGRESS_POLICER_REMARKING_E,
        0x00010000, BITS_TO_BYTES_ALIGNMENT_MAC(51), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS),
    DUP_3_TABLES_TABLE (
        CPSS_DXCH_SIP5_15_TABLE_INGRESS_POLICER_0_METERING_CONFORM_SIGN_E,
        CPSS_DXCH_SIP5_15_TABLE_INGRESS_POLICER_1_METERING_CONFORM_SIGN_E,
        CPSS_DXCH_SIP5_15_TABLE_EGRESS_POLICER_METERING_CONFORM_SIGN_E,
        0x00030000, BITS_TO_BYTES_ALIGNMENT_MAC(2), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS),
    DUP_3_TABLES_TABLE (
        CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_METERING_E,
        CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_METERING_E,
        CPSS_DXCH_SIP5_TABLE_EGRESS_POLICER_METERING_E,
        0x00080000, BITS_TO_BYTES_ALIGNMENT_MAC(211), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS),
    DUP_3_TABLES_TABLE (
        CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_COUNTING_E,
        CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_COUNTING_E,
        CPSS_DXCH_SIP5_TABLE_EGRESS_POLICER_COUNTING_E,
        0x000C0000, BITS_TO_BYTES_ALIGNMENT_MAC(211), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS),
    DUP_2_TABLES_TABLE (
        CPSS_DXCH_SIP5_15_TABLE_INGRESS_POLICER_0_METERING_CONFIG_E,
        CPSS_DXCH_SIP5_15_TABLE_INGRESS_POLICER_1_METERING_CONFIG_E,
        0x00060000, BITS_TO_BYTES_ALIGNMENT_MAC(55), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS),
    {CPSS_DXCH_SIP5_15_TABLE_EGRESS_POLICER_METERING_CONFIG_E,
        {0x00060000, BITS_TO_BYTES_ALIGNMENT_MAC(45), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

    /* NOTE: the alignment when stated NOT_MODIFIED_VALUE_CNS :
        is taken auto calculated from bobcat3TablesInfo_overrideBobk[] */
    {CPSS_DXCH_TABLE_LAST_E, {0,0,0}}
};

/* SIP6 : info about the number of entries in the tables (override sip5_25tableNumEntriesInfoArr) */
static const TABLE_NUM_ENTRIES_INFO_STC   sip6_tableNumEntriesInfoArr[] =
{
     {CPSS_DXCH_TABLE_FDB_E,
        ENTRY_TYPE_FDB_E,0,0}
    ,{CPSS_DXCH_SIP6_TABLE_EXACT_MATCH_E,
        ENTRY_TYPE_EM_E,0,0}
    ,{CPSS_DXCH_SIP5_TABLE_TCAM_E,
        ENTRY_TYPE_TCAM_E,0,0}

    /* IPCL */
    ,{CPSS_DXCH_SIP6_TABLE_INGRESS_PCL_HASH_MODE_CRC_E,
        ENTRY_TYPE_EXPLICIT_NUM_E,0,16}
    ,{CPSS_DXCH_TABLE_PCL_CONFIG_E,
         ENTRY_TYPE_EXPLICIT_NUM_E,0,5120}
    ,{CPSS_DXCH_XCAT2_TABLE_INGRESS_PCL_LOOKUP01_CONFIG_E,
         ENTRY_TYPE_EXPLICIT_NUM_E,0,5120}
    ,{CPSS_DXCH_XCAT_TABLE_INGRESS_PCL_LOOKUP1_CONFIG_E,
        ENTRY_TYPE_NOT_VALID_E,0,0} /* the table was removed from the device */
    ,{CPSS_DXCH_SIP5_TABLE_IPCL2_UDB_SELECT_E,
        ENTRY_TYPE_NOT_VALID_E,0,0} /* the table was removed from the device */

    ,{CPSS_DXCH_SIP5_TABLE_IPCL0_UDB_SELECT_E,
        ENTRY_TYPE_EXPLICIT_NUM_E,0,112}  /* More entries than in Aldrin2 */
    ,{CPSS_DXCH_SIP5_TABLE_IPCL1_UDB_SELECT_E,
        ENTRY_TYPE_EXPLICIT_NUM_E,0,112}  /* More entries than in Aldrin2 */


    /* BMA */
    ,{CPSS_DXCH_SIP5_TABLE_BMA_MULTICAST_COUNTERS_E,
        ENTRY_TYPE_EXPLICIT_NUM_E,0,1344}

    /* EPCL */
    ,{CPSS_DXCH_SIP6_TABLE_EGRESS_PCL_EXACT_MATCH_PROFILE_ID_MAPPING_E,
        ENTRY_TYPE_EXPLICIT_NUM_E,0,128}
    ,{CPSS_DXCH_SIP6_TABLE_EGRESS_PCL_PORT_LATENCY_MONITORING_E,
        ENTRY_TYPE_EXPLICIT_NUM_E,0,2048}
    ,{CPSS_DXCH_SIP6_TABLE_EGRESS_PCL_SOURCE_PHYSICAL_PORT_MAPPING_E,
        ENTRY_TYPE_PHY_PORT_E,0,0}
    ,{CPSS_DXCH_SIP6_TABLE_EGRESS_PCL_TARGET_PHYSICAL_PORT_MAPPING_E,
        ENTRY_TYPE_PHY_PORT_E,0,0}
    ,{CPSS_DXCH2_TABLE_EGRESS_PCL_CONFIG_E,
         ENTRY_TYPE_EXPLICIT_NUM_E,0,5120}

    /* EQ */
    ,{CPSS_DXCH_SIP5_TABLE_EQ_INGRESS_STC_PHYSICAL_PORT_E,
        ENTRY_TYPE_NOT_VALID_E,0,0} /*the table was removed from the device */


    /* EGF_SHT */
    /*2.51.3 Removal of EGF "ePort Egress Filtering"*/
    /* Eport EVlan Filter*/
    /* NOTE: according to Design team the RAM was removed , although CIDER keep showing this table */
    ,{CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_EPORT_FILTER_E,
        ENTRY_TYPE_NOT_VALID_E,0,0}/*the table was removed from the device */

    ,{CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_FWD_FROM_CPU_TO_LOOPBACK_MAPPER_E,
        ENTRY_TYPE_NOT_VALID_E,0,0}/*the table was removed from the device */

    ,{CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_VLAN_MAPPER_E,
        ENTRY_TYPE_NOT_VALID_E,0,0}/*the table was removed from the device */

    ,{CPSS_DXCH_TABLE_MULTICAST_E,
        ENTRY_TYPE_EXPLICIT_NUM_E,FRACTION_HALF_TABLE_INDICATION_CNS | 1/*place holder*/,_2K}

    /* in sip 6 each PLR unit has it's own instance of the Metering Configuration table.
        in sip 20 the access to Metering Configuration table is done by IPLR0 */
    ,{CPSS_DXCH_SIP5_15_TABLE_EGRESS_POLICER_METERING_CONFIG_E,
        ENTRY_TYPE_EPLR_METERING_E,0,0}/* valid !!! (was not valid in sip 5.20,5.25) */
    ,{CPSS_DXCH_SIP5_15_TABLE_INGRESS_POLICER_1_METERING_CONFIG_E,
        ENTRY_TYPE_IPLR_METERING_E,0,0}/* valid !!! (was not valid in sip 5.20,5.25) */

    /* IPLR0,1,EPLR */
    ,{CPSS_DXCH_SIP6_TABLE_INGRESS_POLICER_0_QOS_ATTRIBUTE_TABLE_E,
        ENTRY_TYPE_QOS_PROFILES_E,VERTICAL_INDICATION_CNS | 4,0} /* 4 QOS profiles in entry */
    ,{CPSS_DXCH_SIP6_TABLE_INGRESS_POLICER_1_QOS_ATTRIBUTE_TABLE_E,
        ENTRY_TYPE_QOS_PROFILES_E,VERTICAL_INDICATION_CNS | 4,0} /* 4 QOS profiles in entry */
    ,{CPSS_DXCH_SIP6_TABLE_EGRESS_POLICER_QOS_ATTRIBUTE_TABLE_E,
        ENTRY_TYPE_QOS_PROFILES_E,VERTICAL_INDICATION_CNS | 4,0} /* 4 QOS profiles in entry */

    ,{CPSS_DXCH_SIP6_TABLE_INGRESS_POLICER_0_PORT_ATTRIBUTE_TABLE_E,
        ENTRY_TYPE_PHY_PORT_E,0,0}
    ,{CPSS_DXCH_SIP6_TABLE_INGRESS_POLICER_1_PORT_ATTRIBUTE_TABLE_E,
        ENTRY_TYPE_PHY_PORT_E,0,0}
    ,{CPSS_DXCH_SIP6_TABLE_EGRESS_POLICER_PORT_ATTRIBUTE_TABLE_E,
        ENTRY_TYPE_PHY_PORT_E,0,0}

    /* L2i */
    ,{CPSS_DXCH_LION_TABLE_STG_INGRESS_E,
        ENTRY_TYPE_STG_MEMBERS_E,VERTICAL_INDICATION_CNS | 8,0}/*8 STGs per entry (each support 128 ports * 2 bits)*/
    ,{CPSS_DXCH_SIP5_TABLE_INGRESS_BRIDGE_PORT_MEMBERS_E,
        ENTRY_TYPE_EXPLICIT_NUM_E,0,1024} /* 1K lines to hold 8K eVLANs regardless of supported eVLANs. 8 vlans per line (each 128 ports * 1 bit)*/
    ,{CPSS_DXCH_SIP5_TABLE_BRIDGE_PHYSICAL_PORT_E,
        ENTRY_TYPE_EXPLICIT_NUM_E,0,128}/* supports 128 ports */
    ,{CPSS_DXCH_SIP5_TABLE_BRIDGE_PHYSICAL_PORT_RATE_LIMIT_COUNTERS_E,
        ENTRY_TYPE_EXPLICIT_NUM_E,0,128}

    /* LPM - the 'LPM_MEM_PIPE_1' should not be used because 2 pipes share the same LPM */
    ,{CPSS_DXCH_SIP5_20_TABLE_LPM_MEM_PIPE_1_E,
        ENTRY_TYPE_NOT_VALID_E,0,0}/*the table was removed from the device */

    ,{ CPSS_DXCH_SIP5_TABLE_LPM_MEM_E,
        ENTRY_TYPE_EXPLICIT_NUM_E,0,983040}
        /* LPM aging memory */
    ,{ CPSS_DXCH_SIP6_TABLE_LPM_AGING_E,
        ENTRY_TYPE_EXPLICIT_NUM_E,0,62920}
     /*PDX*/

    ,{CPSS_DXCH_SIP6_TXQ_PDX_DX_QGRPMAP_E,ENTRY_TYPE_PDX_QUEUE_GROUP_NUM_E,0,0}

     /*PDS*/
    ,PRV_SET_SIZE_MULTI_TILE_DP_TABLE(PDS_PER_QUEUE_COUNTERS,ENTRY_TYPE_EXPLICIT_NUM_E,400)
    /*debug*/
     ,PRV_SET_SIZE_MULTI_TILE_DP_TABLE(PDS_DATA_STORAGE,ENTRY_TYPE_EXPLICIT_NUM_E,3072)
     ,PRV_SET_SIZE_MULTI_TILE_DP_TABLE(PDS_NXT_TABLE,ENTRY_TYPE_EXPLICIT_NUM_E,1968)
     ,PRV_SET_SIZE_MULTI_TILE_DP_TABLE(PDS_PID_TABLE,ENTRY_TYPE_EXPLICIT_NUM_E,7872)
     ,PRV_SET_SIZE_MULTI_TILE_DP_TABLE(PDS_HEAD_HEAD,ENTRY_TYPE_EXPLICIT_NUM_E,400)
     ,PRV_SET_SIZE_MULTI_TILE_DP_TABLE(PDS_HEAD_TAIL,ENTRY_TYPE_EXPLICIT_NUM_E,400)
     ,PRV_SET_SIZE_MULTI_TILE_DP_TABLE(PDS_TAIL_TAIL,ENTRY_TYPE_EXPLICIT_NUM_E,400)
     ,PRV_SET_SIZE_MULTI_TILE_DP_TABLE(PDS_TAIL_HEAD,ENTRY_TYPE_EXPLICIT_NUM_E,400)
     ,PRV_SET_SIZE_MULTI_TILE_DP_TABLE(PDS_FRAG_HEAD,ENTRY_TYPE_EXPLICIT_NUM_E,400)
     ,PRV_SET_SIZE_MULTI_TILE_DP_TABLE(PDS_FRAG_TAIL,ENTRY_TYPE_EXPLICIT_NUM_E,400)
    /*SDQ*/
    ,PRV_SET_SIZE_MULTI_TILE_DP_TABLE(SDQ_QUEUE_CFG,ENTRY_TYPE_EXPLICIT_NUM_E,400)
    ,PRV_SET_SIZE_MULTI_TILE_DP_TABLE(SDQ_QUEUE_CRDT_BLNC,ENTRY_TYPE_EXPLICIT_NUM_E,400)
    ,PRV_SET_SIZE_MULTI_TILE_DP_TABLE(SDQ_QUEUE_ELIG_STATE,ENTRY_TYPE_EXPLICIT_NUM_E,400)
    ,PRV_SET_SIZE_MULTI_TILE_DP_TABLE(SDQ_PORT_CRDT_BLNC,ENTRY_TYPE_EXPLICIT_NUM_E,9)
    ,PRV_SET_SIZE_MULTI_TILE_DP_TABLE(SDQ_SEL_LIST_PTRS,ENTRY_TYPE_EXPLICIT_NUM_E,18)

    /*PFCC*/
    ,PRV_SET_SIZE_CENTRAL_DP_TABLE(PFCC_PFCC_CFG,ENTRY_TYPE_EXPLICIT_NUM_E,314)

    /* TTI */
    ,{CPSS_DXCH_SIP6_TABLE_TTI_PORT_TO_QUEUE_TRANSLATION_TABLE_E,
        ENTRY_TYPE_PHY_PORT_E,0,0}

    ,{CPSS_DXCH_SIP6_TABLE_TTI_VLAN_TO_VRF_ID_TABLE_E,
        ENTRY_TYPE_EVLAN_E,0,0}

    ,{CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
        ENTRY_TYPE_DEFAULT_EPORT_E,0,0}

    ,{CPSS_DXCH_TABLE_VLAN_PORT_PROTOCOL_E,
        ENTRY_TYPE_DEFAULT_EPORT_E,FRACTION_INDICATION_CNS | 16,0}


    /* PREQ */
    ,{CPSS_DXCH_SIP6_TABLE_PREQ_CNC_PORT_MAPPING_E,
        ENTRY_TYPE_EXPLICIT_NUM_E, 0, 1024} /*Queue Group Index is mapped to 16 port profiles*/

    ,{CPSS_DXCH_SIP6_TABLE_PREQ_QUEUE_CONFIGURATIONS_E,
        ENTRY_TYPE_EXPLICIT_NUM_E, 0, 256}

    ,{CPSS_DXCH_SIP6_TABLE_PREQ_QUEUE_DP_CONFIGURATIONS_E,
        ENTRY_TYPE_EXPLICIT_NUM_E, 0, 768}

    ,{CPSS_DXCH_SIP6_TABLE_PREQ_PORT_PROFILE_E,
        ENTRY_TYPE_EXPLICIT_NUM_E, 0, 16}

    ,{CPSS_DXCH_SIP6_TABLE_PREQ_TARGET_PHYSICAL_PORT_E,
        ENTRY_TYPE_PHY_PORT_E, 0, 0}

    /* EQ */
    ,{CPSS_DXCH_SIP6_TABLE_EQ_L2ECMP_PATH_UTILIZATION_TABLE0_E,
        ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}

    ,{CPSS_DXCH_SIP6_TABLE_EQ_L2ECMP_PATH_UTILIZATION_TABLE1_E,
        ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}

    ,{CPSS_DXCH_SIP6_TABLE_EQ_L2ECMP_PATH_UTILIZATION_TABLE2_E,
        ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}

    ,{CPSS_DXCH_SIP6_TABLE_EQ_L2ECMP_PATH_UTILIZATION_TABLE3_E,
        ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}

    /* IPVX */
    ,{CPSS_DXCH_SIP5_25_TABLE_IPVX_ROUTER_ECMP_POINTER_E,
        ENTRY_TYPE_IPVX_NH_E,VERTICAL_INDICATION_CNS | 4,0}/* 4 pointers (entries) per line */

    ,{CPSS_DXCH_SIP6_TABLE_IPVX_ECMP_E, /* replace CPSS_DXCH_SIP5_TABLE_ECMP_E */
        ENTRY_TYPE_EXPLICIT_NUM_E,VERTICAL_INDICATION_CNS | 2,  6144}/* 2 entries per line */

    ,{CPSS_DXCH_SIP6_TABLE_PBR_E,
        ENTRY_TYPE_PBR_LEAF_E, 5, 0}/* 5 entries per line */


    ,{CPSS_DXCH_SIP5_TABLE_ECMP_E,
        ENTRY_TYPE_NOT_VALID_E,0,0}/* using CPSS_DXCH_SIP6_TABLE_IPVX_ECMP_E instead */

    /*HA*/
    ,{CPSS_DXCH_TABLE_ROUTE_HA_MAC_SA_E,
        ENTRY_TYPE_EVLAN_E,0,0}/* in sip5 : 4K entries , in sip6 according to eVlans */

    /* LMU */
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 0,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 1,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 2,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 3,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 4,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 5,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 6,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 7,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 8,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 9,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 10,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 11,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 12,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 13,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 14,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 15,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 16,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 17,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 18,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 19,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 20,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 21,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 22,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 23,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 24,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 25,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 26,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 27,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 28,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 29,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 30,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 31,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}

    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 0,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 1,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 2,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 3,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 4,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 5,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 6,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 7,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 8,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 9,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 10,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 11,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 12,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 13,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 14,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 15,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 16,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 17,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 18,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 19,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 20,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 21,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 22,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 23,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 24,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 25,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 26,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 27,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 28,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 29,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 30,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 31,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}

    /* PHA unit */
    ,{CPSS_DXCH_SIP6_TABLE_PHA_PPA_THREADS_CONF_1_E,
      ENTRY_TYPE_THREAD_ID_NUM_E, 0, 0}
    ,{CPSS_DXCH_SIP6_TABLE_PHA_PPA_THREADS_CONF_2_E,
      ENTRY_TYPE_THREAD_ID_NUM_E, 0, 0}

    ,{CPSS_DXCH_SIP6_TABLE_PHA_SOURCE_PHYSICAL_PORT_E,
      ENTRY_TYPE_PHY_PORT_E, 0, 0}
    ,{CPSS_DXCH_SIP6_TABLE_PHA_TARGET_PHYSICAL_PORT_E,
      ENTRY_TYPE_PHY_PORT_E, 0, 0}

    ,{CPSS_DXCH_SIP6_TABLE_PHA_FW_IMAGE_E,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 1024}
    ,{CPSS_DXCH_SIP6_TABLE_PHA_SHARED_DMEM_E,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 2048}

    ,{CPSS_DXCH_SIP5_TABLE_TAIL_DROP_MAX_QUEUE_LIMITS_E,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP5_TABLE_TAIL_DROP_EQ_QUEUE_LIMITS_DP0_E,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP5_TABLE_TAIL_DROP_DQ_QUEUE_BUF_LIMITS_E,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP5_TABLE_TAIL_DROP_DQ_QUEUE_DESC_LIMITS_E,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP5_TABLE_TAIL_DROP_EQ_QUEUE_LIMITS_DP1_E,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP5_TABLE_TAIL_DROP_EQ_QUEUE_LIMITS_DP2_E,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP5_TABLE_PFC_MODE_PROFILE_TC_XOFF_THRESHOLDS_E,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP5_TABLE_PFC_MODE_PROFILE_TC_XON_THRESHOLDS_E,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP5_TABLE_PFC_LLFC_COUNTERS_E,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP5_TABLE_CN_SAMPLE_INTERVALS_E,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TTI_PORT_MAPPING__DQ_0_E,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TTI_PORT_MAPPING__DQ_1_E,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TTI_PORT_MAPPING__DQ_2_E,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TTI_PORT_MAPPING__DQ_3_E,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TTI_PORT_MAPPING__DQ_4_E,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TTI_PORT_MAPPING__DQ_5_E,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}

    /* must be last */
    ,{CPSS_DXCH_TABLE_LAST_E , ENTRY_TYPE___LAST__E , 0, 0}
};

/* Falcon direct table information , that are new (not exists in Aldrin2) */
/* register new tables that not exists in Aldrin2 !!!! */
static const BOBK_TABLES_INFO_DIRECT_NEW_INDEX_STC falcon_directAccessTableInfo_new[] =
{
    /***************************************************/
    /* shared tables are accessed via the 'orig units' */
    /* FDB,TS/ARP,LPM,ExactMatch - only via the units  */
    /***************************************************/

    /* IPCL tables */
     {PRV_DXCH_SIP6_TABLE_INGRESS_PCL_HASH_MODE_CRC_TABLE_E,/* index in falcon_directAccessTableInfo */
        {CPSS_DXCH_SIP6_TABLE_INGRESS_PCL_HASH_MODE_CRC_E, PRV_CPSS_DXCH_UNIT_PCL_E,
         {0x00001000, BITS_TO_BYTES_ALIGNMENT_MAC(160), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE}}

    /* IPLR0,1,EPLR tables */
    ,{PRV_DXCH_SIP6_TABLE_INGRESS_POLICER_0_QOS_ATTRIBUTE_TABLE_E,/* index in falcon_directAccessTableInfo */
        {CPSS_DXCH_SIP6_TABLE_INGRESS_POLICER_0_QOS_ATTRIBUTE_TABLE_E, PRV_CPSS_DXCH_UNIT_IPLR_E,
         {0x00005000, BITS_TO_BYTES_ALIGNMENT_MAC(20), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE}}

    ,{PRV_DXCH_SIP6_TABLE_INGRESS_POLICER_1_QOS_ATTRIBUTE_TABLE_E,/* index in falcon_directAccessTableInfo */
        {CPSS_DXCH_SIP6_TABLE_INGRESS_POLICER_1_QOS_ATTRIBUTE_TABLE_E, PRV_CPSS_DXCH_UNIT_IPLR_1_E,
         {0x00005000, BITS_TO_BYTES_ALIGNMENT_MAC(20), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE}}

    ,{PRV_DXCH_SIP6_TABLE_EGRESS_POLICER_QOS_ATTRIBUTE_TABLE_E,/* index in falcon_directAccessTableInfo */
        {CPSS_DXCH_SIP6_TABLE_EGRESS_POLICER_QOS_ATTRIBUTE_TABLE_E, PRV_CPSS_DXCH_UNIT_EPLR_E,
         {0x00005000, BITS_TO_BYTES_ALIGNMENT_MAC(20), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE}}

    ,{PRV_DXCH_SIP6_TABLE_INGRESS_POLICER_0_PORT_ATTRIBUTE_TABLE_E,/* index in falcon_directAccessTableInfo */
        {CPSS_DXCH_SIP6_TABLE_INGRESS_POLICER_0_PORT_ATTRIBUTE_TABLE_E, PRV_CPSS_DXCH_UNIT_IPLR_E,
         {0x00004000, BITS_TO_BYTES_ALIGNMENT_MAC(13), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE}}

    ,{PRV_DXCH_SIP6_TABLE_INGRESS_POLICER_1_PORT_ATTRIBUTE_TABLE_E,/* index in falcon_directAccessTableInfo */
        {CPSS_DXCH_SIP6_TABLE_INGRESS_POLICER_1_PORT_ATTRIBUTE_TABLE_E, PRV_CPSS_DXCH_UNIT_IPLR_1_E,
         {0x00004000, BITS_TO_BYTES_ALIGNMENT_MAC(13), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE}}

    ,{PRV_DXCH_SIP6_TABLE_EGRESS_POLICER_PORT_ATTRIBUTE_TABLE_E,/* index in falcon_directAccessTableInfo */
        {CPSS_DXCH_SIP6_TABLE_EGRESS_POLICER_PORT_ATTRIBUTE_TABLE_E, PRV_CPSS_DXCH_UNIT_EPLR_E,
         {0x00004000, BITS_TO_BYTES_ALIGNMENT_MAC(13), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE}}

     /*TXQ -PDX*/

    ,PRV_DECLARE_SINGLE_INSTANCE_CENTRAL_DP_TABLE(PDX_DX_QGRPMAP,0x00000000,PDX,18)


    /*TXQ -PDS*/

    ,PRV_DECLARE_MULTI_TILE_DP_TABLE(PDS_PER_QUEUE_COUNTERS,0x00030000,PDS,44)
     /*debug*/
    ,PRV_DECLARE_MULTI_TILE_DP_TABLE(PDS_DATA_STORAGE,0x00000000,PDS,62)
    ,PRV_DECLARE_MULTI_TILE_DP_TABLE(PDS_NXT_TABLE,0x00020000,PDS,12)
    ,PRV_DECLARE_MULTI_TILE_DP_TABLE(PDS_PID_TABLE,0x00010000,PDS,13)
    ,PRV_DECLARE_MULTI_TILE_DP_TABLE(PDS_HEAD_HEAD,0x00028800,PDS,28)
    ,PRV_DECLARE_MULTI_TILE_DP_TABLE(PDS_HEAD_TAIL,0x00028000,PDS,28)
    ,PRV_DECLARE_MULTI_TILE_DP_TABLE(PDS_TAIL_TAIL,0x00027000,PDS,28)
    ,PRV_DECLARE_MULTI_TILE_DP_TABLE(PDS_TAIL_HEAD,0x00027800,PDS,28)
    ,PRV_DECLARE_MULTI_TILE_DP_TABLE(PDS_FRAG_HEAD,0x00029800,PDS,18)
    ,PRV_DECLARE_MULTI_TILE_DP_TABLE(PDS_FRAG_TAIL,0x00029000,PDS,19)
    /*TXQ -SDQ*/
    ,PRV_DECLARE_MULTI_TILE_DP_TABLE(SDQ_QUEUE_CFG,0x00002000,SDQ,50)
    ,PRV_DECLARE_MULTI_TILE_DP_TABLE(SDQ_QUEUE_CRDT_BLNC,0x00007000,SDQ,35)
    ,PRV_DECLARE_MULTI_TILE_DP_TABLE(SDQ_QUEUE_ELIG_STATE,0x00009000,SDQ,8)
    ,PRV_DECLARE_MULTI_TILE_DP_TABLE(SDQ_PORT_CRDT_BLNC,0x00006100,SDQ,37)
    ,PRV_DECLARE_MULTI_TILE_DP_TABLE(SDQ_SEL_LIST_PTRS,0x00006300,SDQ,20)

     /* TxQ PFCC  tables */
    ,PRV_DECLARE_CENTRAL_DP_TABLE(PFCC_PFCC_CFG,0x00001000,PFCC,32)

        /* TTI tables */

        ,{PRV_DXCH_SIP6_TABLE_TTI_PORT_TO_QUEUE_TRANSLATION_TABLE_E,/* index in falcon_directAccessTableInfo */
        {CPSS_DXCH_SIP6_TABLE_TTI_PORT_TO_QUEUE_TRANSLATION_TABLE_E, PRV_CPSS_DXCH_UNIT_TTI_E,
              {0x00060000, BITS_TO_BYTES_ALIGNMENT_MAC(10), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE}}

        ,{PRV_CPSS_DXCH_SIP6_TABLE_TTI_VLAN_TO_VRF_ID_TABLE_E,/* index in falcon_directAccessTableInfo */
        {CPSS_DXCH_SIP6_TABLE_TTI_VLAN_TO_VRF_ID_TABLE_E, PRV_CPSS_DXCH_UNIT_TTI_E,
              {0x00070000, BITS_TO_BYTES_ALIGNMENT_MAC(12), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE}}

        /* PREQ */
        ,{PRV_DXCH_SIP6_TABLE_PREQ_CNC_PORT_MAPPING_E,
            {CPSS_DXCH_SIP6_TABLE_PREQ_CNC_PORT_MAPPING_E, PRV_CPSS_DXCH_UNIT_PREQ_E,
            {0x00010000, BITS_TO_BYTES_ALIGNMENT_MAC(26), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE}}

        ,{PRV_DXCH_SIP6_TABLE_PREQ_QUEUE_CONFIGURATIONS_E,
            {CPSS_DXCH_SIP6_TABLE_PREQ_QUEUE_CONFIGURATIONS_E, PRV_CPSS_DXCH_UNIT_PREQ_E,
            {0x00030000, BITS_TO_BYTES_ALIGNMENT_MAC(67), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE}}

        ,{PRV_DXCH_SIP6_TABLE_PREQ_QUEUE_DP_CONFIGURATIONS_E,
            {CPSS_DXCH_SIP6_TABLE_PREQ_QUEUE_DP_CONFIGURATIONS_E, PRV_CPSS_DXCH_UNIT_PREQ_E,
            {0x00040000, BITS_TO_BYTES_ALIGNMENT_MAC(88), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE}}

        ,{PRV_DXCH_SIP6_TABLE_PREQ_PORT_PROFILE_E,
            {CPSS_DXCH_SIP6_TABLE_PREQ_PORT_PROFILE_E, PRV_CPSS_DXCH_UNIT_PREQ_E,
            {0x00050000, BITS_TO_BYTES_ALIGNMENT_MAC(92), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE}}

        ,{PRV_DXCH_SIP6_TABLE_PREQ_TARGET_PHYSICAL_PORT_E,
            {CPSS_DXCH_SIP6_TABLE_PREQ_TARGET_PHYSICAL_PORT_E, PRV_CPSS_DXCH_UNIT_PREQ_E,
            {0x00060000, BITS_TO_BYTES_ALIGNMENT_MAC(6), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE}}

        /* EQ */
        ,{PRV_DXCH_SIP6_TABLE_EQ_L2ECMP_PATH_UTILIZATION_TABLE0_E,
            {CPSS_DXCH_SIP6_TABLE_EQ_L2ECMP_PATH_UTILIZATION_TABLE0_E, PRV_CPSS_DXCH_UNIT_EQ_E,
            {0x000a0000, BITS_TO_BYTES_ALIGNMENT_MAC(136), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE}}

        ,{PRV_DXCH_SIP6_TABLE_EQ_L2ECMP_PATH_UTILIZATION_TABLE1_E,
            {CPSS_DXCH_SIP6_TABLE_EQ_L2ECMP_PATH_UTILIZATION_TABLE1_E, PRV_CPSS_DXCH_UNIT_EQ_E,
            {0x000b0000, BITS_TO_BYTES_ALIGNMENT_MAC(136), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE}}

        ,{PRV_DXCH_SIP6_TABLE_EQ_L2ECMP_PATH_UTILIZATION_TABLE2_E,
            {CPSS_DXCH_SIP6_TABLE_EQ_L2ECMP_PATH_UTILIZATION_TABLE2_E, PRV_CPSS_DXCH_UNIT_EQ_E,
            {0x000c0000, BITS_TO_BYTES_ALIGNMENT_MAC(136), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE}}

        ,{PRV_DXCH_SIP6_TABLE_EQ_L2ECMP_PATH_UTILIZATION_TABLE3_E,
            {CPSS_DXCH_SIP6_TABLE_EQ_L2ECMP_PATH_UTILIZATION_TABLE3_E, PRV_CPSS_DXCH_UNIT_EQ_E,
            {0x000d0000, BITS_TO_BYTES_ALIGNMENT_MAC(136), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE}}

        /* IPVX */
        ,{PRV_DXCH_SIP6_TABLE_IPVX_ECMP_E,/* index in falcon_directAccessTableInfo */
            {CPSS_DXCH_SIP6_TABLE_IPVX_ECMP_E, PRV_CPSS_DXCH_UNIT_IPVX_E,
            {0x00250000, BITS_TO_BYTES_ALIGNMENT_MAC((29*2)), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE}}

        /* LPM */
        ,{PRV_DXCH_SIP6_TABLE_PBR_E,
            {CPSS_DXCH_SIP6_TABLE_PBR_E, PRV_CPSS_DXCH_UNIT_LPM_E,
            {0x00000000, BITS_TO_BYTES_ALIGNMENT_MAC(115), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}

        /* LPM Aging */
        ,{PRV_DXCH_SIP6_TABLE_LPM_AGING_E,
            {CPSS_DXCH_SIP6_TABLE_LPM_AGING_E, PRV_CPSS_DXCH_UNIT_LPM_E,
            {0x00F80000, BITS_TO_BYTES_ALIGNMENT_MAC(32), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}


        /* LMU */
        ,{PRV_DXCH_SIP6_TABLE_LMU_STATISTICS_TABLE_E + 0,
            {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 0, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_0_E,
            {0x00430000, BITS_TO_BYTES_ALIGNMENT_MAC(186), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_LMU_STATISTICS_TABLE_E + 1,
            {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 1, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_0_E,
            {0x004B0000, BITS_TO_BYTES_ALIGNMENT_MAC(186), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_LMU_STATISTICS_TABLE_E + 2,
            {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 2, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_1_E,
            {0x00430000, BITS_TO_BYTES_ALIGNMENT_MAC(186), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_LMU_STATISTICS_TABLE_E + 3,
            {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 3, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_1_E,
            {0x004B0000, BITS_TO_BYTES_ALIGNMENT_MAC(186), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_LMU_STATISTICS_TABLE_E + 4,
            {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 4, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_2_E,
            {0x00430000, BITS_TO_BYTES_ALIGNMENT_MAC(186), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_LMU_STATISTICS_TABLE_E + 5,
            {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 5, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_2_E,
            {0x004B0000, BITS_TO_BYTES_ALIGNMENT_MAC(186), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_LMU_STATISTICS_TABLE_E + 6,
            {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 6, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_3_E,
            {0x00430000, BITS_TO_BYTES_ALIGNMENT_MAC(186), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_LMU_STATISTICS_TABLE_E + 7,
            {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 7, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_3_E,
            {0x004B0000, BITS_TO_BYTES_ALIGNMENT_MAC(186), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_LMU_STATISTICS_TABLE_E + 8,
            {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 8, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_0_E + PRV_CPSS_DXCH_UNIT_IN_TILE_1_BASE_E,
            {0x00430000, BITS_TO_BYTES_ALIGNMENT_MAC(186), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_LMU_STATISTICS_TABLE_E + 9,
            {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 9, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_0_E + PRV_CPSS_DXCH_UNIT_IN_TILE_1_BASE_E,
            {0x004B0000, BITS_TO_BYTES_ALIGNMENT_MAC(186), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_LMU_STATISTICS_TABLE_E + 10,
            {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 10, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_1_E + PRV_CPSS_DXCH_UNIT_IN_TILE_1_BASE_E,
            {0x00430000, BITS_TO_BYTES_ALIGNMENT_MAC(186), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_LMU_STATISTICS_TABLE_E + 11,
            {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 11, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_1_E + PRV_CPSS_DXCH_UNIT_IN_TILE_1_BASE_E,
            {0x004B0000, BITS_TO_BYTES_ALIGNMENT_MAC(186), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_LMU_STATISTICS_TABLE_E + 12,
            {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 12, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_2_E + PRV_CPSS_DXCH_UNIT_IN_TILE_1_BASE_E,
            {0x00430000, BITS_TO_BYTES_ALIGNMENT_MAC(186), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_LMU_STATISTICS_TABLE_E + 13,
            {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 13, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_2_E + PRV_CPSS_DXCH_UNIT_IN_TILE_1_BASE_E,
            {0x004B0000, BITS_TO_BYTES_ALIGNMENT_MAC(186), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_LMU_STATISTICS_TABLE_E + 14,
            {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 14, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_3_E + PRV_CPSS_DXCH_UNIT_IN_TILE_1_BASE_E,
            {0x00430000, BITS_TO_BYTES_ALIGNMENT_MAC(186), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_LMU_STATISTICS_TABLE_E + 15,
            {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 15, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_3_E + PRV_CPSS_DXCH_UNIT_IN_TILE_1_BASE_E,
            {0x004B0000, BITS_TO_BYTES_ALIGNMENT_MAC(186), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_LMU_STATISTICS_TABLE_E + 16,
            {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 16, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_0_E + PRV_CPSS_DXCH_UNIT_IN_TILE_2_BASE_E,
            {0x00430000, BITS_TO_BYTES_ALIGNMENT_MAC(186), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_LMU_STATISTICS_TABLE_E + 17,
            {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 17, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_0_E + PRV_CPSS_DXCH_UNIT_IN_TILE_2_BASE_E,
            {0x004B0000, BITS_TO_BYTES_ALIGNMENT_MAC(186), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_LMU_STATISTICS_TABLE_E + 18,
            {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 18, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_1_E + PRV_CPSS_DXCH_UNIT_IN_TILE_2_BASE_E,
            {0x00430000, BITS_TO_BYTES_ALIGNMENT_MAC(186), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_LMU_STATISTICS_TABLE_E + 19,
            {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 19, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_1_E + PRV_CPSS_DXCH_UNIT_IN_TILE_2_BASE_E,
            {0x004B0000, BITS_TO_BYTES_ALIGNMENT_MAC(186), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_LMU_STATISTICS_TABLE_E + 20,
            {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 20, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_2_E + PRV_CPSS_DXCH_UNIT_IN_TILE_2_BASE_E,
            {0x00430000, BITS_TO_BYTES_ALIGNMENT_MAC(186), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_LMU_STATISTICS_TABLE_E + 21,
            {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 21, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_2_E + PRV_CPSS_DXCH_UNIT_IN_TILE_2_BASE_E,
            {0x004B0000, BITS_TO_BYTES_ALIGNMENT_MAC(186), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_LMU_STATISTICS_TABLE_E + 22,
            {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 22, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_3_E + PRV_CPSS_DXCH_UNIT_IN_TILE_2_BASE_E,
            {0x00430000, BITS_TO_BYTES_ALIGNMENT_MAC(186), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_LMU_STATISTICS_TABLE_E + 23,
            {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 23, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_3_E + PRV_CPSS_DXCH_UNIT_IN_TILE_2_BASE_E,
            {0x004B0000, BITS_TO_BYTES_ALIGNMENT_MAC(186), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_LMU_STATISTICS_TABLE_E + 24,
            {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 24, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_0_E + PRV_CPSS_DXCH_UNIT_IN_TILE_3_BASE_E,
            {0x00430000, BITS_TO_BYTES_ALIGNMENT_MAC(186), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_LMU_STATISTICS_TABLE_E + 25,
            {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 25, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_0_E + PRV_CPSS_DXCH_UNIT_IN_TILE_3_BASE_E,
            {0x004B0000, BITS_TO_BYTES_ALIGNMENT_MAC(186), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_LMU_STATISTICS_TABLE_E + 26,
            {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 26, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_1_E + PRV_CPSS_DXCH_UNIT_IN_TILE_3_BASE_E,
            {0x00430000, BITS_TO_BYTES_ALIGNMENT_MAC(186), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_LMU_STATISTICS_TABLE_E + 27,
            {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 27, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_1_E + PRV_CPSS_DXCH_UNIT_IN_TILE_3_BASE_E,
            {0x004B0000, BITS_TO_BYTES_ALIGNMENT_MAC(186), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_LMU_STATISTICS_TABLE_E + 28,
            {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 28, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_2_E + PRV_CPSS_DXCH_UNIT_IN_TILE_3_BASE_E,
            {0x00430000, BITS_TO_BYTES_ALIGNMENT_MAC(186), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_LMU_STATISTICS_TABLE_E + 29,
            {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 29, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_2_E + PRV_CPSS_DXCH_UNIT_IN_TILE_3_BASE_E,
            {0x004B0000, BITS_TO_BYTES_ALIGNMENT_MAC(186), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_LMU_STATISTICS_TABLE_E + 30,
            {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 30, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_3_E + PRV_CPSS_DXCH_UNIT_IN_TILE_3_BASE_E,
            {0x00430000, BITS_TO_BYTES_ALIGNMENT_MAC(186), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_LMU_STATISTICS_TABLE_E + 31,
            {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 31, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_3_E + PRV_CPSS_DXCH_UNIT_IN_TILE_3_BASE_E,
            {0x004B0000, BITS_TO_BYTES_ALIGNMENT_MAC(186), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}

        ,{PRV_DXCH_SIP6_TABLE_LMU_CONFIGURATION_TABLE_E + 0,
            {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 0, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_0_E,
            {0x00434000, BITS_TO_BYTES_ALIGNMENT_MAC(90), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_LMU_CONFIGURATION_TABLE_E + 1,
            {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 1, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_0_E,
            {0x004B4000, BITS_TO_BYTES_ALIGNMENT_MAC(90), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_LMU_CONFIGURATION_TABLE_E + 2,
            {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 2, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_1_E,
            {0x00434000, BITS_TO_BYTES_ALIGNMENT_MAC(90), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_LMU_CONFIGURATION_TABLE_E + 3,
            {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 3, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_1_E,
            {0x004B4000, BITS_TO_BYTES_ALIGNMENT_MAC(90), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_LMU_CONFIGURATION_TABLE_E + 4,
            {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 4, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_2_E,
            {0x00434000, BITS_TO_BYTES_ALIGNMENT_MAC(90), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_LMU_CONFIGURATION_TABLE_E + 5,
            {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 5, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_2_E,
            {0x004B4000, BITS_TO_BYTES_ALIGNMENT_MAC(90), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_LMU_CONFIGURATION_TABLE_E + 6,
            {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 6, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_3_E,
            {0x00434000, BITS_TO_BYTES_ALIGNMENT_MAC(90), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_LMU_CONFIGURATION_TABLE_E + 7,
            {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 7, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_3_E,
            {0x004B4000, BITS_TO_BYTES_ALIGNMENT_MAC(90), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_LMU_CONFIGURATION_TABLE_E + 8,
            {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 8, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_0_E + PRV_CPSS_DXCH_UNIT_IN_TILE_1_BASE_E,
            {0x00434000, BITS_TO_BYTES_ALIGNMENT_MAC(90), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_LMU_CONFIGURATION_TABLE_E + 9,
            {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 9, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_0_E + PRV_CPSS_DXCH_UNIT_IN_TILE_1_BASE_E,
            {0x004B4000, BITS_TO_BYTES_ALIGNMENT_MAC(90), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_LMU_CONFIGURATION_TABLE_E + 10,
            {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 10, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_1_E + PRV_CPSS_DXCH_UNIT_IN_TILE_1_BASE_E,
            {0x00434000, BITS_TO_BYTES_ALIGNMENT_MAC(90), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_LMU_CONFIGURATION_TABLE_E + 11,
            {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 11, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_1_E + PRV_CPSS_DXCH_UNIT_IN_TILE_1_BASE_E,
            {0x004B4000, BITS_TO_BYTES_ALIGNMENT_MAC(90), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_LMU_CONFIGURATION_TABLE_E + 12,
            {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 12, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_2_E + PRV_CPSS_DXCH_UNIT_IN_TILE_1_BASE_E,
            {0x00434000, BITS_TO_BYTES_ALIGNMENT_MAC(90), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_LMU_CONFIGURATION_TABLE_E + 13,
            {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 13, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_2_E + PRV_CPSS_DXCH_UNIT_IN_TILE_1_BASE_E,
            {0x004B4000, BITS_TO_BYTES_ALIGNMENT_MAC(90), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_LMU_CONFIGURATION_TABLE_E + 14,
            {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 14, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_3_E + PRV_CPSS_DXCH_UNIT_IN_TILE_1_BASE_E,
            {0x00434000, BITS_TO_BYTES_ALIGNMENT_MAC(90), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_LMU_CONFIGURATION_TABLE_E + 15,
            {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 15, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_3_E + PRV_CPSS_DXCH_UNIT_IN_TILE_1_BASE_E,
            {0x004B4000, BITS_TO_BYTES_ALIGNMENT_MAC(90), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_LMU_CONFIGURATION_TABLE_E + 16,
            {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 16, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_0_E + PRV_CPSS_DXCH_UNIT_IN_TILE_2_BASE_E,
            {0x00434000, BITS_TO_BYTES_ALIGNMENT_MAC(90), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_LMU_CONFIGURATION_TABLE_E + 17,
            {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 17, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_0_E + PRV_CPSS_DXCH_UNIT_IN_TILE_2_BASE_E,
            {0x004B4000, BITS_TO_BYTES_ALIGNMENT_MAC(90), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_LMU_CONFIGURATION_TABLE_E + 18,
            {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 18, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_1_E + PRV_CPSS_DXCH_UNIT_IN_TILE_2_BASE_E,
            {0x00434000, BITS_TO_BYTES_ALIGNMENT_MAC(90), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_LMU_CONFIGURATION_TABLE_E + 19,
            {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 19, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_1_E + PRV_CPSS_DXCH_UNIT_IN_TILE_2_BASE_E,
            {0x004B4000, BITS_TO_BYTES_ALIGNMENT_MAC(90), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_LMU_CONFIGURATION_TABLE_E + 20,
            {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 20, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_2_E + PRV_CPSS_DXCH_UNIT_IN_TILE_2_BASE_E,
            {0x00434000, BITS_TO_BYTES_ALIGNMENT_MAC(90), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_LMU_CONFIGURATION_TABLE_E + 21,
            {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 21, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_2_E + PRV_CPSS_DXCH_UNIT_IN_TILE_2_BASE_E,
            {0x004B4000, BITS_TO_BYTES_ALIGNMENT_MAC(90), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_LMU_CONFIGURATION_TABLE_E + 22,
            {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 22, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_3_E + PRV_CPSS_DXCH_UNIT_IN_TILE_2_BASE_E,
            {0x00434000, BITS_TO_BYTES_ALIGNMENT_MAC(90), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_LMU_CONFIGURATION_TABLE_E + 23,
            {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 23, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_3_E + PRV_CPSS_DXCH_UNIT_IN_TILE_2_BASE_E,
            {0x004B4000, BITS_TO_BYTES_ALIGNMENT_MAC(90), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_LMU_CONFIGURATION_TABLE_E + 24,
            {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 24, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_0_E + PRV_CPSS_DXCH_UNIT_IN_TILE_3_BASE_E,
            {0x00434000, BITS_TO_BYTES_ALIGNMENT_MAC(90), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_LMU_CONFIGURATION_TABLE_E + 25,
            {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 25, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_0_E + PRV_CPSS_DXCH_UNIT_IN_TILE_3_BASE_E,
            {0x004B4000, BITS_TO_BYTES_ALIGNMENT_MAC(90), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_LMU_CONFIGURATION_TABLE_E + 26,
            {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 26, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_1_E + PRV_CPSS_DXCH_UNIT_IN_TILE_3_BASE_E,
            {0x00434000, BITS_TO_BYTES_ALIGNMENT_MAC(90), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_LMU_CONFIGURATION_TABLE_E + 27,
            {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 27, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_1_E + PRV_CPSS_DXCH_UNIT_IN_TILE_3_BASE_E,
            {0x004B4000, BITS_TO_BYTES_ALIGNMENT_MAC(90), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_LMU_CONFIGURATION_TABLE_E + 28,
            {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 28, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_2_E + PRV_CPSS_DXCH_UNIT_IN_TILE_3_BASE_E,
            {0x00434000, BITS_TO_BYTES_ALIGNMENT_MAC(90), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_LMU_CONFIGURATION_TABLE_E + 29,
            {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 29, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_2_E + PRV_CPSS_DXCH_UNIT_IN_TILE_3_BASE_E,
            {0x004B4000, BITS_TO_BYTES_ALIGNMENT_MAC(90), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_LMU_CONFIGURATION_TABLE_E + 30,
            {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 30, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_3_E + PRV_CPSS_DXCH_UNIT_IN_TILE_3_BASE_E,
            {0x00434000, BITS_TO_BYTES_ALIGNMENT_MAC(90), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_LMU_CONFIGURATION_TABLE_E + 31,
            {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 31, PRV_CPSS_DXCH_UNIT_BASE_RAVEN_3_E + PRV_CPSS_DXCH_UNIT_IN_TILE_3_BASE_E,
            {0x004B4000, BITS_TO_BYTES_ALIGNMENT_MAC(90), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}


        ,{PRV_DXCH_SIP6_TABLE_EGRESS_PCL_EXACT_MATCH_PROFILE_ID_MAPPING_E,
            {CPSS_DXCH_SIP6_TABLE_EGRESS_PCL_EXACT_MATCH_PROFILE_ID_MAPPING_E, PRV_CPSS_DXCH_UNIT_EPCL_E,
            {0x00016000, BITS_TO_BYTES_ALIGNMENT_MAC(8),  PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_EGRESS_PCL_PORT_LATENCY_MONITORING_E,
            {CPSS_DXCH_SIP6_TABLE_EGRESS_PCL_PORT_LATENCY_MONITORING_E, PRV_CPSS_DXCH_UNIT_EPCL_E,
            {0x00014000, BITS_TO_BYTES_ALIGNMENT_MAC(32), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_EGRESS_PCL_SOURCE_PHYSICAL_PORT_MAPPING_E,
            {CPSS_DXCH_SIP6_TABLE_EGRESS_PCL_SOURCE_PHYSICAL_PORT_MAPPING_E, PRV_CPSS_DXCH_UNIT_EPCL_E,
            {0x00013000, BITS_TO_BYTES_ALIGNMENT_MAC(17), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
        ,{PRV_DXCH_SIP6_TABLE_EGRESS_PCL_TARGET_PHYSICAL_PORT_MAPPING_E,
            {CPSS_DXCH_SIP6_TABLE_EGRESS_PCL_TARGET_PHYSICAL_PORT_MAPPING_E, PRV_CPSS_DXCH_UNIT_EPCL_E,
            {0x00012000, BITS_TO_BYTES_ALIGNMENT_MAC(17), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}

    /* PHA unit */
    ,{PRV_DXCH_SIP6_TABLE_PHA_PPA_THREADS_CONF_1_E,
        {CPSS_DXCH_SIP6_TABLE_PHA_PPA_THREADS_CONF_1_E, PRV_CPSS_DXCH_UNIT_PHA_E,
        {0x007F0000, BITS_TO_BYTES_ALIGNMENT_MAC(23), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}

    ,{PRV_DXCH_SIP6_TABLE_PHA_PPA_THREADS_CONF_2_E,
        {CPSS_DXCH_SIP6_TABLE_PHA_PPA_THREADS_CONF_2_E, PRV_CPSS_DXCH_UNIT_PHA_E,
        {0x007F8000, BITS_TO_BYTES_ALIGNMENT_MAC(149), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}

    ,{PRV_DXCH_SIP6_TABLE_PHA_SOURCE_PHYSICAL_PORT_E,
        {CPSS_DXCH_SIP6_TABLE_PHA_SOURCE_PHYSICAL_PORT_E, PRV_CPSS_DXCH_UNIT_PHA_E,
        {0x007F4000, BITS_TO_BYTES_ALIGNMENT_MAC(16), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}

    ,{PRV_DXCH_SIP6_TABLE_PHA_TARGET_PHYSICAL_PORT_E,
        {CPSS_DXCH_SIP6_TABLE_PHA_TARGET_PHYSICAL_PORT_E, PRV_CPSS_DXCH_UNIT_PHA_E,
        {0x007F6000, BITS_TO_BYTES_ALIGNMENT_MAC(16), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}

    ,{PRV_DXCH_SIP6_TABLE_PHA_FW_IMAGE_E,
        {CPSS_DXCH_SIP6_TABLE_PHA_FW_IMAGE_E, PRV_CPSS_DXCH_UNIT_PHA_E,
        {0x00040000, BITS_TO_BYTES_ALIGNMENT_MAC(128), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}

    ,{PRV_DXCH_SIP6_TABLE_PHA_SHARED_DMEM_E,
        {CPSS_DXCH_SIP6_TABLE_PHA_SHARED_DMEM_E, PRV_CPSS_DXCH_UNIT_PHA_E,
        {0x007C0000, BITS_TO_BYTES_ALIGNMENT_MAC(32), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}

    /* NOTE: addresses in this table MUST be '0' based (relative to unit) and
       not global address of the device*/
    ,{PRV_DXCH_DEVICE_TABLE___LAST_E , {0, 0, {0, 0, 0}, GT_FALSE}}
};

/*  table information , that need to override Aldrin2 info from falconTablesInfo */
/* NOTE : the entries in this array need to be matched with entries in
   aldrin2TablesInfo , according to field */
static const PRV_CPSS_DXCH_TABLES_INFO_EXT_STC falconTablesInfo_overrideAldrin2[] =
{
    /* FDB */
    {CPSS_DXCH_TABLE_FDB_E,
        OVERRIDE_INFO_WIDTH_MAC(115)
    },

    /* IPCL */
    {CPSS_DXCH_XCAT_TABLE_INGRESS_PCL_UDB_CONFIG_E,
        OVERRIDE_INFO_WIDTH_MAC(840)
    },

    /* EPCL */
    {CPSS_DXCH2_TABLE_EGRESS_PCL_CONFIG_E,
        OVERRIDE_INFO_WIDTH_MAC(30)
    },

    /* IPLR0,1, EPLR */
    {
        CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_E_ATTRIBUTES_E,
        OVERRIDE_INFO_WIDTH_MAC(SIP6_IPVX_POLICER_E_ATTRIBUTES_BITS_SIZE_CNS)
    },
    {
        CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_E_ATTRIBUTES_E,
        OVERRIDE_INFO_WIDTH_MAC(SIP6_IPVX_POLICER_E_ATTRIBUTES_BITS_SIZE_CNS)
    },
    {
        CPSS_DXCH_SIP5_TABLE_EGRESS_POLICER_E_ATTRIBUTES_E,
        OVERRIDE_INFO_WIDTH_MAC(SIP6_IPVX_POLICER_E_ATTRIBUTES_BITS_SIZE_CNS)
    },

    /*EGF-SHT*/
    {CPSS_DXCH_LION_TABLE_TXQ_NON_TRUNK_MEMBERS_E,
        OVERRIDE_INFO_WIDTH_MAC(BMP_1K_PORTS_CNS)
    },
    {CPSS_DXCH_LION_TABLE_TXQ_DESIGNATED_PORT_E,
        OVERRIDE_INFO_WIDTH_MAC(BMP_1K_PORTS_CNS)
    },
    {CPSS_DXCH_TABLE_ROUTE_HA_MAC_SA_E,
        OVERRIDE_INFO_WIDTH_MAC(12) /* 12 LSBs of MAC_SRC for SIP6, for SIP5 - 8 bits */
    },

    /* L2i */
    {CPSS_DXCH_SIP5_TABLE_INGRESS_BRIDGE_PORT_MEMBERS_E,
        OVERRIDE_INFO_WIDTH_MAC(BMP_1K_PORTS_CNS)
    },

    {CPSS_DXCH_LION_TABLE_STG_INGRESS_E,
        OVERRIDE_INFO_NUM_ENTRIES_AND_WIDTH_MAC(256, (2 * BMP_1K_PORTS_CNS))/* 2 bits per port */
    },

    /* LPM - the entry size is less than the alignment ! */
    {CPSS_DXCH_SIP5_TABLE_LPM_MEM_E,
        OVERRIDE_INFO_WIDTH_MAC(115)
    },

    {CPSS_DXCH_SIP5_25_TABLE_IPVX_ROUTER_ECMP_POINTER_E,
        OVERRIDE_INFO_WIDTH_MAC((15*4))/* 4 entries per line , 15 bits per entry . */
    },

    /* TTI */
    {CPSS_DXCH_SIP5_TABLE_TTI_UDB_CONFIG_E,
        OVERRIDE_INFO_WIDTH_MAC(SIP6_TTI_UDB_CONFIG_BITS_SIZE_CNS)},

    /* TCAM */
    {CPSS_DXCH_SIP5_TABLE_TCAM_PCL_TTI_ACTION_E,
        OVERRIDE_INFO_WIDTH_MAC(247)},

    /* PHA - the entry size is less than the alignment ! */
    {CPSS_DXCH_SIP6_TABLE_PHA_PPA_THREADS_CONF_2_E,
        OVERRIDE_INFO_WIDTH_MAC(160)},

    /* ERMRK */
    {CPSS_DXCH_SIP5_TABLE_ERMRK_TIMESTAMP_CFG_E,
        OVERRIDE_INFO_WIDTH_MAC(34)},

    /* IPCL - the entry size is less than the alignment ! */
    {CPSS_DXCH_SIP6_TABLE_INGRESS_PCL_HASH_MODE_CRC_E,
        OVERRIDE_INFO_WIDTH_MAC(160)},

    /* LPM - Line width uses 115 bits which is 4 words ! */
    {CPSS_DXCH_SIP5_TABLE_LPM_MEM_E,
        OVERRIDE_INFO_WIDTH_MAC(115)},

    /* LMU - the entry size is less than the alignment ! */
    {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 0  , OVERRIDE_INFO_WIDTH_MAC(186)},
    {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 1  , OVERRIDE_INFO_WIDTH_MAC(186)},
    {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 2  , OVERRIDE_INFO_WIDTH_MAC(186)},
    {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 3  , OVERRIDE_INFO_WIDTH_MAC(186)},
    {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 4  , OVERRIDE_INFO_WIDTH_MAC(186)},
    {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 5  , OVERRIDE_INFO_WIDTH_MAC(186)},
    {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 6  , OVERRIDE_INFO_WIDTH_MAC(186)},
    {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 7  , OVERRIDE_INFO_WIDTH_MAC(186)},
    {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 8  , OVERRIDE_INFO_WIDTH_MAC(186)},
    {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 9  , OVERRIDE_INFO_WIDTH_MAC(186)},
    {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 10 , OVERRIDE_INFO_WIDTH_MAC(186)},
    {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 11 , OVERRIDE_INFO_WIDTH_MAC(186)},
    {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 12 , OVERRIDE_INFO_WIDTH_MAC(186)},
    {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 13 , OVERRIDE_INFO_WIDTH_MAC(186)},
    {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 14 , OVERRIDE_INFO_WIDTH_MAC(186)},
    {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 15 , OVERRIDE_INFO_WIDTH_MAC(186)},
    {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 16 , OVERRIDE_INFO_WIDTH_MAC(186)},
    {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 17 , OVERRIDE_INFO_WIDTH_MAC(186)},
    {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 18 , OVERRIDE_INFO_WIDTH_MAC(186)},
    {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 19 , OVERRIDE_INFO_WIDTH_MAC(186)},
    {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 20 , OVERRIDE_INFO_WIDTH_MAC(186)},
    {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 21 , OVERRIDE_INFO_WIDTH_MAC(186)},
    {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 22 , OVERRIDE_INFO_WIDTH_MAC(186)},
    {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 23 , OVERRIDE_INFO_WIDTH_MAC(186)},
    {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 24 , OVERRIDE_INFO_WIDTH_MAC(186)},
    {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 25 , OVERRIDE_INFO_WIDTH_MAC(186)},
    {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 26 , OVERRIDE_INFO_WIDTH_MAC(186)},
    {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 27 , OVERRIDE_INFO_WIDTH_MAC(186)},
    {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 28 , OVERRIDE_INFO_WIDTH_MAC(186)},
    {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 29 , OVERRIDE_INFO_WIDTH_MAC(186)},
    {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 30 , OVERRIDE_INFO_WIDTH_MAC(186)},
    {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 31 , OVERRIDE_INFO_WIDTH_MAC(186)},
    /* LMU - the entry size is less than the alignment ! */
    {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 0  , OVERRIDE_INFO_WIDTH_MAC(90)},
    {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 1  , OVERRIDE_INFO_WIDTH_MAC(90)},
    {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 2  , OVERRIDE_INFO_WIDTH_MAC(90)},
    {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 3  , OVERRIDE_INFO_WIDTH_MAC(90)},
    {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 4  , OVERRIDE_INFO_WIDTH_MAC(90)},
    {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 5  , OVERRIDE_INFO_WIDTH_MAC(90)},
    {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 6  , OVERRIDE_INFO_WIDTH_MAC(90)},
    {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 7  , OVERRIDE_INFO_WIDTH_MAC(90)},
    {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 8  , OVERRIDE_INFO_WIDTH_MAC(90)},
    {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 9  , OVERRIDE_INFO_WIDTH_MAC(90)},
    {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 10 , OVERRIDE_INFO_WIDTH_MAC(90)},
    {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 11 , OVERRIDE_INFO_WIDTH_MAC(90)},
    {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 12 , OVERRIDE_INFO_WIDTH_MAC(90)},
    {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 13 , OVERRIDE_INFO_WIDTH_MAC(90)},
    {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 14 , OVERRIDE_INFO_WIDTH_MAC(90)},
    {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 15 , OVERRIDE_INFO_WIDTH_MAC(90)},
    {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 16 , OVERRIDE_INFO_WIDTH_MAC(90)},
    {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 17 , OVERRIDE_INFO_WIDTH_MAC(90)},
    {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 18 , OVERRIDE_INFO_WIDTH_MAC(90)},
    {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 19 , OVERRIDE_INFO_WIDTH_MAC(90)},
    {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 20 , OVERRIDE_INFO_WIDTH_MAC(90)},
    {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 21 , OVERRIDE_INFO_WIDTH_MAC(90)},
    {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 22 , OVERRIDE_INFO_WIDTH_MAC(90)},
    {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 23 , OVERRIDE_INFO_WIDTH_MAC(90)},
    {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 24 , OVERRIDE_INFO_WIDTH_MAC(90)},
    {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 25 , OVERRIDE_INFO_WIDTH_MAC(90)},
    {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 26 , OVERRIDE_INFO_WIDTH_MAC(90)},
    {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 27 , OVERRIDE_INFO_WIDTH_MAC(90)},
    {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 28 , OVERRIDE_INFO_WIDTH_MAC(90)},
    {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 29 , OVERRIDE_INFO_WIDTH_MAC(90)},
    {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 30 , OVERRIDE_INFO_WIDTH_MAC(90)},
    {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 31 , OVERRIDE_INFO_WIDTH_MAC(90)}

    /* Exact Match indirect configuration for falcon */
    ,{CPSS_DXCH_SIP6_TABLE_EXACT_MATCH_E,
        {_128K,BITS_TO_WORDS_MAC(SIP6_EM_EXACT_MATCH_BITS_SIZE_CNS),
            PRV_CPSS_DXCH_INDIRECT_ACCESS_E,&indirectAccessTableInfo_falcon[PRV_CPSS_SIP6_EXACT_MATCH_TABLE_INDIRECT_E],
            PRV_CPSS_DXCH_INDIRECT_ACCESS_E,&indirectAccessTableInfo_falcon[PRV_CPSS_SIP6_EXACT_MATCH_TABLE_INDIRECT_E]}}

    ,{CPSS_DXCH_SIP6_TABLE_EQ_L2ECMP_PATH_UTILIZATION_TABLE0_E,
        OVERRIDE_INFO_WIDTH_MAC(136)}
    ,{CPSS_DXCH_SIP6_TABLE_EQ_L2ECMP_PATH_UTILIZATION_TABLE1_E,
        OVERRIDE_INFO_WIDTH_MAC(136)}
    ,{CPSS_DXCH_SIP6_TABLE_EQ_L2ECMP_PATH_UTILIZATION_TABLE2_E,
        OVERRIDE_INFO_WIDTH_MAC(136)}
    ,{CPSS_DXCH_SIP6_TABLE_EQ_L2ECMP_PATH_UTILIZATION_TABLE3_E,
        OVERRIDE_INFO_WIDTH_MAC(136)}

    ,{CPSS_DXCH_SIP6_TABLE_PREQ_QUEUE_DP_CONFIGURATIONS_E,
        OVERRIDE_INFO_WIDTH_MAC(88)}
    ,{CPSS_DXCH_SIP6_TABLE_PREQ_QUEUE_CONFIGURATIONS_E,
        OVERRIDE_INFO_WIDTH_MAC(67)}

};

/*xxx*/
/* Hawk table - in runtime copy table from this of Falcon ... and continue from there ... */
static const GT_U32 hawkTableInfoSize = CPSS_DXCH_SIP6_10_TABLE_LAST_E;

/* size of table hawk_directAccessTableInfo */
static const GT_U32 hawk_directAccessTableInfoSize = PRV_DXCH_SIP6_10_TABLE___LAST_E;

#define PER_DP_OVERRIDE_TABLE_AND_ADDR(table,addr,numOfBits,dpIndex)   \
     {table+dpIndex,                                                   \
          {addr,BITS_TO_BYTES_ALIGNMENT_MAC(numOfBits), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}}

/* tables that are per DP that need override in hawk_directAccessTableInfo_overrideFalcon[] */
#define HAWK_DP_OVERRIDE_TABLE_AND_ADDR(table,addr,numOfBits)           \
     PER_DP_OVERRIDE_TABLE_AND_ADDR(table,addr,numOfBits,0/*dpIndex*/)  \
    ,PER_DP_OVERRIDE_TABLE_AND_ADDR(table,addr,numOfBits,1/*dpIndex*/)  \
    ,PER_DP_OVERRIDE_TABLE_AND_ADDR(table,addr,numOfBits,2/*dpIndex*/)  \
    ,PER_DP_OVERRIDE_TABLE_AND_ADDR(table,addr,numOfBits,3/*dpIndex*/)  \
    ,PER_DP_OVERRIDE_TABLE_AND_ADDR(table,0x1/*addr*/,0/*numOfBits*/, 4) \
    ,PER_DP_OVERRIDE_TABLE_AND_ADDR(table,0x1/*addr*/,0/*numOfBits*/, 5) \
    ,PER_DP_OVERRIDE_TABLE_AND_ADDR(table,0x1/*addr*/,0/*numOfBits*/, 6) \
    ,PER_DP_OVERRIDE_TABLE_AND_ADDR(table,0x1/*addr*/,0/*numOfBits*/, 7) \
    ,PER_DP_OVERRIDE_TABLE_AND_ADDR(table,0x1/*addr*/,0/*numOfBits*/, 8) \
    ,PER_DP_OVERRIDE_TABLE_AND_ADDR(table,0x1/*addr*/,0/*numOfBits*/, 9) \
    ,PER_DP_OVERRIDE_TABLE_AND_ADDR(table,0x1/*addr*/,0/*numOfBits*/,10) \
    ,PER_DP_OVERRIDE_TABLE_AND_ADDR(table,0x1/*addr*/,0/*numOfBits*/,11) \
    ,PER_DP_OVERRIDE_TABLE_AND_ADDR(table,0x1/*addr*/,0/*numOfBits*/,12) \
    ,PER_DP_OVERRIDE_TABLE_AND_ADDR(table,0x1/*addr*/,0/*numOfBits*/,13) \
    ,PER_DP_OVERRIDE_TABLE_AND_ADDR(table,0x1/*addr*/,0/*numOfBits*/,14) \
    ,PER_DP_OVERRIDE_TABLE_AND_ADDR(table,0x1/*addr*/,0/*numOfBits*/,15) \
    ,PER_DP_OVERRIDE_TABLE_AND_ADDR(table,0x1/*addr*/,0/*numOfBits*/,16) \
    ,PER_DP_OVERRIDE_TABLE_AND_ADDR(table,0x1/*addr*/,0/*numOfBits*/,17) \
    ,PER_DP_OVERRIDE_TABLE_AND_ADDR(table,0x1/*addr*/,0/*numOfBits*/,18) \
    ,PER_DP_OVERRIDE_TABLE_AND_ADDR(table,0x1/*addr*/,0/*numOfBits*/,19) \
    ,PER_DP_OVERRIDE_TABLE_AND_ADDR(table,0x1/*addr*/,0/*numOfBits*/,20) \
    ,PER_DP_OVERRIDE_TABLE_AND_ADDR(table,0x1/*addr*/,0/*numOfBits*/,21) \
    ,PER_DP_OVERRIDE_TABLE_AND_ADDR(table,0x1/*addr*/,0/*numOfBits*/,22) \
    ,PER_DP_OVERRIDE_TABLE_AND_ADDR(table,0x1/*addr*/,0/*numOfBits*/,23) \
    ,PER_DP_OVERRIDE_TABLE_AND_ADDR(table,0x1/*addr*/,0/*numOfBits*/,24) \
    ,PER_DP_OVERRIDE_TABLE_AND_ADDR(table,0x1/*addr*/,0/*numOfBits*/,25) \
    ,PER_DP_OVERRIDE_TABLE_AND_ADDR(table,0x1/*addr*/,0/*numOfBits*/,26) \
    ,PER_DP_OVERRIDE_TABLE_AND_ADDR(table,0x1/*addr*/,0/*numOfBits*/,27) \
    ,PER_DP_OVERRIDE_TABLE_AND_ADDR(table,0x1/*addr*/,0/*numOfBits*/,28) \
    ,PER_DP_OVERRIDE_TABLE_AND_ADDR(table,0x1/*addr*/,0/*numOfBits*/,29) \
    ,PER_DP_OVERRIDE_TABLE_AND_ADDR(table,0x1/*addr*/,0/*numOfBits*/,30) \
    ,PER_DP_OVERRIDE_TABLE_AND_ADDR(table,0x1/*addr*/,0/*numOfBits*/,31)

/* Hawk direct table information , that need to override Falcon info from hawk_directAccessTableInfo */
/*  falcon_directAccessTableInfo_overrideAldrin2 */
static const PRV_CPSS_DXCH_TABLES_INFO_DIRECT_EXT_STC hawk_directAccessTableInfo_overrideFalcon[] =
{
    /* SHT */
    {CPSS_DXCH_LION_TABLE_PORT_ISOLATION_L2_E,
    {0x00040000, BITS_TO_BYTES_ALIGNMENT_MAC(128), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},
    {CPSS_DXCH_LION_TABLE_PORT_ISOLATION_L3_E,
    {0x00060000, BITS_TO_BYTES_ALIGNMENT_MAC(128), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},
    {CPSS_DXCH_LION_TABLE_STG_EGRESS_E,
    {0x000B0000, BITS_TO_BYTES_ALIGNMENT_MAC(128), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},
    {CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_NON_TRUNK_MEMBERS2_E,
    {0x00090000, BITS_TO_BYTES_ALIGNMENT_MAC(128), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},
    {CPSS_DXCH_LION_TABLE_TXQ_SOURCE_ID_MEMBERS_E,
    {0x000F0000, BITS_TO_BYTES_ALIGNMENT_MAC(128), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},
    {CPSS_DXCH_TABLE_MULTICAST_E,
    {0x000C0000, BITS_TO_BYTES_ALIGNMENT_MAC(128), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},
    {CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_DEVICE_MAP_TABLE_E,
    {0x000A8000, BITS_TO_BYTES_ALIGNMENT_MAC(12), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},
    {CPSS_DXCH_LION_TABLE_TXQ_DESIGNATED_PORT_E,
    {0x000AD000, BITS_TO_BYTES_ALIGNMENT_MAC(128), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},
    {CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_EGRESS_EPORT_E,
    {0x00078000, BITS_TO_BYTES_ALIGNMENT_MAC(29), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},
    {CPSS_DXCH_LION_TABLE_TXQ_NON_TRUNK_MEMBERS_E,
    {0x000AC000, BITS_TO_BYTES_ALIGNMENT_MAC(128), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},
    {CPSS_DXCH_LION_TABLE_VLAN_EGRESS_E,
    {0x00000000, BITS_TO_BYTES_ALIGNMENT_MAC(128), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},
    {CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_VLAN_ATTRIBUTES_E,
    {0x00080000, BITS_TO_BYTES_ALIGNMENT_MAC(8), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},
    {CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_VLAN_SPANNING_E,
    {0x00088000, BITS_TO_BYTES_ALIGNMENT_MAC(12), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

    HAWK_DP_OVERRIDE_TABLE_AND_ADDR(CPSS_DXCH_SIP6_TXQ_PDS_PER_QUEUE_COUNTERS_E,0x00040000/*addr*/,44/*bits*/),
    HAWK_DP_OVERRIDE_TABLE_AND_ADDR(CPSS_DXCH_SIP6_TXQ_PDS_HEAD_HEAD_E,0x00038800,28),
    HAWK_DP_OVERRIDE_TABLE_AND_ADDR(CPSS_DXCH_SIP6_TXQ_PDS_HEAD_TAIL_E,0x00038000,28),
    HAWK_DP_OVERRIDE_TABLE_AND_ADDR(CPSS_DXCH_SIP6_TXQ_PDS_TAIL_TAIL_E,0x00037000,28),
    HAWK_DP_OVERRIDE_TABLE_AND_ADDR(CPSS_DXCH_SIP6_TXQ_PDS_TAIL_HEAD_E,0x00037800,28),
    HAWK_DP_OVERRIDE_TABLE_AND_ADDR(CPSS_DXCH_SIP6_TXQ_PDS_FRAG_HEAD_E,0x00039800,18),
    HAWK_DP_OVERRIDE_TABLE_AND_ADDR(CPSS_DXCH_SIP6_TXQ_PDS_FRAG_TAIL_E,0x00039000,19),
    /* SDQ */
    HAWK_DP_OVERRIDE_TABLE_AND_ADDR(CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_CRDT_BLNC_E,0x00008000/*addr*/,35/*bits*/),
    HAWK_DP_OVERRIDE_TABLE_AND_ADDR(CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_ELIG_STATE_E,0x0000a000/*addr*/,11/*bits*/),
    HAWK_DP_OVERRIDE_TABLE_AND_ADDR(CPSS_DXCH_SIP6_TXQ_SDQ_PORT_CRDT_BLNC_E,0x00007200/*addr*/,36/*bits*/),
    HAWK_DP_OVERRIDE_TABLE_AND_ADDR(CPSS_DXCH_SIP6_TXQ_SDQ_SEL_LIST_PTRS_E,0x00007400/*addr*/,18/*bits*/)

   /* Hawk IPCL0,1,2 tables */
    ,{CPSS_DXCH_TABLE_PCL_CONFIG_E,
     {0x00010000,BITS_TO_BYTES_ALIGNMENT_MAC(SIP6_10_PCL_PCL_CONFIG_BITS_SIZE_CNS),PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},
    {CPSS_DXCH_XCAT2_TABLE_INGRESS_PCL_LOOKUP01_CONFIG_E,
     {0x00020000,BITS_TO_BYTES_ALIGNMENT_MAC(SIP6_10_PCL_PCL_CONFIG_BITS_SIZE_CNS),PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},
    {CPSS_DXCH_XCAT_TABLE_INGRESS_PCL_LOOKUP1_CONFIG_E,
     {0x00030000,BITS_TO_BYTES_ALIGNMENT_MAC(SIP6_10_PCL_PCL_CONFIG_BITS_SIZE_CNS),PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

    {CPSS_DXCH_SIP5_TABLE_IPCL0_UDB_SELECT_E,
     {0x00040000,BITS_TO_BYTES_ALIGNMENT_MAC(SIP6_10_IPCL_UDB_SELECT_BITS_SIZE_CNS),PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},
    {CPSS_DXCH_SIP5_TABLE_IPCL1_UDB_SELECT_E,
     {0x00044000,BITS_TO_BYTES_ALIGNMENT_MAC(SIP6_10_IPCL_UDB_SELECT_BITS_SIZE_CNS),PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},
    {CPSS_DXCH_SIP5_TABLE_IPCL2_UDB_SELECT_E,
     {0x00048000,BITS_TO_BYTES_ALIGNMENT_MAC(SIP6_10_IPCL_UDB_SELECT_BITS_SIZE_CNS),PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

    {CPSS_DXCH_LION_TABLE_TRUNK_HASH_MASK_CRC_E,
     {0x00002000,BITS_TO_BYTES_ALIGNMENT_MAC(SIP6_10_PCL_TRUNK_HASH_MASK_CRC_BITS_SIZE_CNS),PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

    /* Hawk EPCL */
    {CPSS_DXCH_SIP6_TABLE_EGRESS_PCL_TARGET_PHYSICAL_PORT_MAPPING_E,
    {0x00012000, BITS_TO_BYTES_ALIGNMENT_MAC(18), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

    /* IPLR0,1, EPLR */
    DUP_3_TABLES_TABLE (
        CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_IPFIX_ALERT_WRAPAROUND_E,
        CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_IPFIX_ALERT_WRAPAROUND_E,
        CPSS_DXCH_SIP5_TABLE_EGRESS_POLICER_IPFIX_ALERT_WRAPAROUND_E,
        0x00006000, BITS_TO_BYTES_ALIGNMENT_MAC(32), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS),
    DUP_3_TABLES_TABLE (
        CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_IPFIX_ALERT_AGING_E,
        CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_IPFIX_ALERT_AGING_E,
        CPSS_DXCH_SIP5_TABLE_EGRESS_POLICER_IPFIX_ALERT_AGING_E,
        0x00008000, BITS_TO_BYTES_ALIGNMENT_MAC(32), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS),
    DUP_3_TABLES_TABLE (
        CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_COUNTING_E,
        CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_COUNTING_E,
        CPSS_DXCH_SIP5_TABLE_EGRESS_POLICER_COUNTING_E,
        0x00200000, BITS_TO_BYTES_ALIGNMENT_MAC(229), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS),

    /* HA */

    /* ROUTER_ARP_DA and TUNNEL_START_TABLE */
    {CPSS_DXCH2_TABLE_TUNNEL_START_CONFIG_E,
    {0x00400000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_HA_TUNNEL_START_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

    /* L2I */
    {CPSS_DXCH_LION_TABLE_STG_INGRESS_E,
    {NOT_MODIFIED_VALUE_CNS, BITS_TO_BYTES_ALIGNMENT_MAC(256), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

    /* NOTE: the alignment when stated NOT_MODIFIED_VALUE_CNS :
        is taken auto calculated from falconTablesInfo_overrideAldrin2[] */
    {CPSS_DXCH_TABLE_LAST_E, {0,0,0}}
};

#define PER_DP_OVERRIDE_TABLE_NUM_ENTRIES(table,entryType,divider,additionalFixedNumberOfEntries,dpIndex)   \
     {table+dpIndex,entryType,divider,additionalFixedNumberOfEntries}

/* tables that are per DP that need override in hawk_directAccessTableInfo_overrideFalcon[] */
#define HAWK_DP_OVERRIDE_TABLE_NUM_ENTRIES(table,entryType,divider,additionalFixedNumberOfEntries)           \
     PER_DP_OVERRIDE_TABLE_NUM_ENTRIES(table,entryType,divider,additionalFixedNumberOfEntries,0/*dpIndex*/)  \
    ,PER_DP_OVERRIDE_TABLE_NUM_ENTRIES(table,entryType,divider,additionalFixedNumberOfEntries,1/*dpIndex*/)  \
    ,PER_DP_OVERRIDE_TABLE_NUM_ENTRIES(table,entryType,divider,additionalFixedNumberOfEntries,2/*dpIndex*/)  \
    ,PER_DP_OVERRIDE_TABLE_NUM_ENTRIES(table,entryType,divider,additionalFixedNumberOfEntries,3/*dpIndex*/)  \
    ,NOT_VALID_TABLE_MAC(table+ 4) \
    ,NOT_VALID_TABLE_MAC(table+ 5) \
    ,NOT_VALID_TABLE_MAC(table+ 6) \
    ,NOT_VALID_TABLE_MAC(table+ 7) \
    ,NOT_VALID_TABLE_MAC(table+ 8) \
    ,NOT_VALID_TABLE_MAC(table+ 9) \
    ,NOT_VALID_TABLE_MAC(table+10) \
    ,NOT_VALID_TABLE_MAC(table+11) \
    ,NOT_VALID_TABLE_MAC(table+12) \
    ,NOT_VALID_TABLE_MAC(table+13) \
    ,NOT_VALID_TABLE_MAC(table+14) \
    ,NOT_VALID_TABLE_MAC(table+15) \
    ,NOT_VALID_TABLE_MAC(table+16) \
    ,NOT_VALID_TABLE_MAC(table+17) \
    ,NOT_VALID_TABLE_MAC(table+18) \
    ,NOT_VALID_TABLE_MAC(table+19) \
    ,NOT_VALID_TABLE_MAC(table+20) \
    ,NOT_VALID_TABLE_MAC(table+21) \
    ,NOT_VALID_TABLE_MAC(table+22) \
    ,NOT_VALID_TABLE_MAC(table+23) \
    ,NOT_VALID_TABLE_MAC(table+24) \
    ,NOT_VALID_TABLE_MAC(table+25) \
    ,NOT_VALID_TABLE_MAC(table+26) \
    ,NOT_VALID_TABLE_MAC(table+27) \
    ,NOT_VALID_TABLE_MAC(table+28) \
    ,NOT_VALID_TABLE_MAC(table+29) \
    ,NOT_VALID_TABLE_MAC(table+30) \
    ,NOT_VALID_TABLE_MAC(table+31)


/* SIP6_10 : info about the number of entries in the tables (override sip6_tableNumEntriesInfoArr) */
static const TABLE_NUM_ENTRIES_INFO_STC   sip6_10_tableNumEntriesInfoArr[] =
{
     {CPSS_DXCH_SIP6_10_TABLE_EXACT_MATCH_AUTO_LEARNED_ENTRY_INDEX_E,
        ENTRY_TYPE_IPLR_IPFIX_E,0,0}/* yes ! auto learn in EM is according to number of IPFIX ! */

     /* IPFIX FIRST N PACKETS */
     ,{CPSS_DXCH_SIP6_10_TABLE_INGRESS_POLICER_0_IPFIX_FIRST_N_PACKETS_E,
        ENTRY_TYPE_IPLR_IPFIX_E,32,0} /* 32: 1 indication bit per IPFIX entry */
     ,{CPSS_DXCH_SIP6_10_TABLE_INGRESS_POLICER_1_IPFIX_FIRST_N_PACKETS_E,
        ENTRY_TYPE_IPLR_IPFIX_E,32,0}  /* 32: 1 indication bit per IPFIX entry */
     ,{CPSS_DXCH_SIP6_10_TABLE_EGRESS_POLICER_IPFIX_FIRST_N_PACKETS_E,
        ENTRY_TYPE_EPLR_IPFIX_E,32,0}  /* 32: 1 indication bit per IPFIX entry */

    /* SHT */
    ,{CPSS_DXCH_LION_TABLE_PORT_ISOLATION_L2_E,
        ENTRY_TYPE_PORT_ISOLATION_E,0,0}
    ,{CPSS_DXCH_LION_TABLE_PORT_ISOLATION_L3_E,
        ENTRY_TYPE_PORT_ISOLATION_E,0,0}
    ,{CPSS_DXCH_TABLE_MULTICAST_E,
        ENTRY_TYPE_VIDX_E,0,0}

     /* PPU */
     ,{CPSS_DXCH_SIP6_10_TABLE_PPU_ACTION_TABLE_0_E,
        ENTRY_TYPE_EXPLICIT_NUM_E,0,64}
     ,{CPSS_DXCH_SIP6_10_TABLE_PPU_ACTION_TABLE_1_E,
             ENTRY_TYPE_EXPLICIT_NUM_E,0,64}
     ,{CPSS_DXCH_SIP6_10_TABLE_PPU_ACTION_TABLE_2_E,
             ENTRY_TYPE_EXPLICIT_NUM_E,0,64}
     ,{CPSS_DXCH_SIP6_10_TABLE_PPU_DAU_PROFILE_TABLE_E,
             ENTRY_TYPE_EXPLICIT_NUM_E,0,16}

     /* IPvx */
    ,{CPSS_DXCH_SIP6_TABLE_IPVX_ECMP_E, /* replace CPSS_DXCH_SIP5_TABLE_ECMP_E */
        ENTRY_TYPE_EXPLICIT_NUM_E,VERTICAL_INDICATION_CNS | 2,  _2K}/* 2 entries per line */

    /* LPM Aging */
    ,{ CPSS_DXCH_SIP6_TABLE_LPM_AGING_E,
        ENTRY_TYPE_EXPLICIT_NUM_E,0,45056}

    /* IPCL */
    ,{CPSS_DXCH_TABLE_PCL_CONFIG_E,
        ENTRY_TYPE_EXPLICIT_NUM_E,0,6144} /* More num of entries than in falcon */
    ,{CPSS_DXCH_XCAT2_TABLE_INGRESS_PCL_LOOKUP01_CONFIG_E,
        ENTRY_TYPE_EXPLICIT_NUM_E,0,6144} /* More num of entries than in falcon */
    ,{CPSS_DXCH_XCAT_TABLE_INGRESS_PCL_LOOKUP1_CONFIG_E,
        ENTRY_TYPE_EXPLICIT_NUM_E,0,6144} /* the table was restored to the device */
    ,{CPSS_DXCH_SIP5_TABLE_IPCL0_UDB_SELECT_E,
        ENTRY_TYPE_EXPLICIT_NUM_E,0,112}  /* same num of entries as in falcon */
    ,{CPSS_DXCH_SIP5_TABLE_IPCL1_UDB_SELECT_E,
        ENTRY_TYPE_EXPLICIT_NUM_E,0,112}  /* same num of entries as in falcon */
    ,{CPSS_DXCH_SIP5_TABLE_IPCL2_UDB_SELECT_E,
        ENTRY_TYPE_EXPLICIT_NUM_E,0,112}  /* the table was restored to the device */
    ,{CPSS_DXCH_SIP6_10_TABLE_IPCL0_SOURCE_PORT_CONFIG_E,
        ENTRY_TYPE_PHY_PORT_E,0,0}
    ,{CPSS_DXCH_SIP6_10_TABLE_IPCL1_SOURCE_PORT_CONFIG_E,
        ENTRY_TYPE_PHY_PORT_E,0,0}
    ,{CPSS_DXCH_SIP6_10_TABLE_IPCL2_SOURCE_PORT_CONFIG_E,
        ENTRY_TYPE_PHY_PORT_E,0,0}

    /* EPCL */
    ,{CPSS_DXCH_SIP6_10_TABLE_EGRESS_PCL_QUEUE_GROUP_LATENCY_PROFILE_CONFIG_E,
        ENTRY_TYPE_EXPLICIT_NUM_E,0,128}
    ,{CPSS_DXCH_SIP6_TABLE_EGRESS_PCL_TARGET_PHYSICAL_PORT_MAPPING_E,
        ENTRY_TYPE_PHY_PORT_E,0,0}

    /* EQ */
    ,{CPSS_DXCH_SIP6_10_TABLE_SOURCE_PORT_HASH_ENTRY_E,
        ENTRY_TYPE_PHY_PORT_E,0,0}

    /*PDS*/
    ,HAWK_DP_OVERRIDE_TABLE_NUM_ENTRIES(CPSS_DXCH_SIP6_TXQ_PDS_PER_QUEUE_COUNTERS_E,
        ENTRY_TYPE_DP_Q_E,0,0)
    ,HAWK_DP_OVERRIDE_TABLE_NUM_ENTRIES(CPSS_DXCH_SIP6_TXQ_PDS_HEAD_HEAD_E,
        ENTRY_TYPE_DP_Q_E,0,0)
    ,HAWK_DP_OVERRIDE_TABLE_NUM_ENTRIES(CPSS_DXCH_SIP6_TXQ_PDS_HEAD_TAIL_E,
        ENTRY_TYPE_DP_Q_E,0,0)
    ,HAWK_DP_OVERRIDE_TABLE_NUM_ENTRIES(CPSS_DXCH_SIP6_TXQ_PDS_TAIL_TAIL_E,
        ENTRY_TYPE_DP_Q_E,0,0)
    ,HAWK_DP_OVERRIDE_TABLE_NUM_ENTRIES(CPSS_DXCH_SIP6_TXQ_PDS_TAIL_HEAD_E,
        ENTRY_TYPE_DP_Q_E,0,0)
    ,HAWK_DP_OVERRIDE_TABLE_NUM_ENTRIES(CPSS_DXCH_SIP6_TXQ_PDS_FRAG_HEAD_E,
        ENTRY_TYPE_DP_Q_E,0,0)
    ,HAWK_DP_OVERRIDE_TABLE_NUM_ENTRIES(CPSS_DXCH_SIP6_TXQ_PDS_FRAG_TAIL_E,
        ENTRY_TYPE_DP_Q_E,0,0)
    ,HAWK_DP_OVERRIDE_TABLE_NUM_ENTRIES(CPSS_DXCH_SIP6_TXQ_PDS_DATA_STORAGE_E,ENTRY_TYPE_EXPLICIT_NUM_E,0,4080)
    ,HAWK_DP_OVERRIDE_TABLE_NUM_ENTRIES(CPSS_DXCH_SIP6_TXQ_PDS_NXT_TABLE_E,ENTRY_TYPE_EXPLICIT_NUM_E,0,2236)
    ,HAWK_DP_OVERRIDE_TABLE_NUM_ENTRIES(CPSS_DXCH_SIP6_TXQ_PDS_PID_TABLE_E,ENTRY_TYPE_EXPLICIT_NUM_E,0,8944)

    /* SDQ */

    ,HAWK_DP_OVERRIDE_TABLE_NUM_ENTRIES(CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_CFG_E,
        ENTRY_TYPE_DP_Q_E,0,0)

    ,HAWK_DP_OVERRIDE_TABLE_NUM_ENTRIES(CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_CRDT_BLNC_E,
        ENTRY_TYPE_DP_Q_E,0,0)

    ,HAWK_DP_OVERRIDE_TABLE_NUM_ENTRIES(CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_ELIG_STATE_E,
       ENTRY_TYPE_DP_Q_E,0,0)

    ,HAWK_DP_OVERRIDE_TABLE_NUM_ENTRIES(CPSS_DXCH_SIP6_TXQ_SDQ_SEL_LIST_PTRS_E,
        ENTRY_TYPE_DP_PORTS_NUM_E,MULITIPLE_INDICATION_MAC(2),0)  /* each DP port : 2 priorities */
    ,HAWK_DP_OVERRIDE_TABLE_NUM_ENTRIES(CPSS_DXCH_SIP6_TXQ_SDQ_PORT_CRDT_BLNC_E,ENTRY_TYPE_EXPLICIT_NUM_E,0,27)

    /* there are only 4 LMUs in AC5P. Invalidate non existing tables. */
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 4,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 5,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 6,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 7,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 8,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 9,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 10,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 11,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 12,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 13,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 14,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 15,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 16,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 17,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 18,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 19,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 20,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 21,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 22,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 23,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 24,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 25,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 26,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 27,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 28,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 29,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 30,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 31,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}


    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 4,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 5,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 6,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 7,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 8,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 9,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 10,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 11,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 12,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 13,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 14,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 15,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 16,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 17,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 18,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 19,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 20,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 21,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 22,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 23,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 24,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 25,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 26,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 27,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 28,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 29,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 30,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 31,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}

    /* must be last */
    ,{CPSS_DXCH_TABLE_LAST_E , ENTRY_TYPE___LAST__E , 0, 0}
};

/* Hawk direct table information , that are new (not exists in Falcon) */
/* register new tables that not exists in Falcon !!!! */
static const BOBK_TABLES_INFO_DIRECT_NEW_INDEX_STC hawk_directAccessTableInfo_new[] =
{
     /* SIP 6.10 - PPU unit */
    {PRV_DXCH_SIP6_10_TABLE_PPU_ACTION_TABLE_0_E,
        {CPSS_DXCH_SIP6_10_TABLE_PPU_ACTION_TABLE_0_E,  PRV_CPSS_DXCH_UNIT_PPU_E,
        {0x00003000, BITS_TO_BYTES_ALIGNMENT_MAC(379), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}

    ,{PRV_DXCH_SIP6_10_TABLE_PPU_ACTION_TABLE_1_E,
        {CPSS_DXCH_SIP6_10_TABLE_PPU_ACTION_TABLE_1_E,  PRV_CPSS_DXCH_UNIT_PPU_E,
        {0x00005000, BITS_TO_BYTES_ALIGNMENT_MAC(379), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}

    ,{PRV_DXCH_SIP6_10_TABLE_PPU_ACTION_TABLE_2_E,
        {CPSS_DXCH_SIP6_10_TABLE_PPU_ACTION_TABLE_2_E,  PRV_CPSS_DXCH_UNIT_PPU_E,
        {0x00007000, BITS_TO_BYTES_ALIGNMENT_MAC(379), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}

    ,{PRV_DXCH_SIP6_10_TABLE_PPU_DAU_PROFILE_TABLE_E,
        {CPSS_DXCH_SIP6_10_TABLE_PPU_DAU_PROFILE_TABLE_E,  PRV_CPSS_DXCH_UNIT_PPU_E,
        {0x00011000, BITS_TO_BYTES_ALIGNMENT_MAC(480), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}

    /* LMU unit moved from GOP to PCA */
    ,{PRV_DXCH_SIP6_TABLE_LMU_STATISTICS_TABLE_E + 0,
       {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 0, PRV_CPSS_DXCH_UNIT_PCA_LMU_0_E,
       {0x00000000, BITS_TO_BYTES_ALIGNMENT_MAC(186), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
    ,{PRV_DXCH_SIP6_TABLE_LMU_STATISTICS_TABLE_E + 1,
       {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 1, PRV_CPSS_DXCH_UNIT_PCA_LMU_1_E,
       {0x00000000, BITS_TO_BYTES_ALIGNMENT_MAC(186), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
    ,{PRV_DXCH_SIP6_TABLE_LMU_STATISTICS_TABLE_E + 2,
       {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 2, PRV_CPSS_DXCH_UNIT_PCA_LMU_2_E,
       {0x00000000, BITS_TO_BYTES_ALIGNMENT_MAC(186), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}
    ,{PRV_DXCH_SIP6_TABLE_LMU_STATISTICS_TABLE_E + 3,
       {CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 3, PRV_CPSS_DXCH_UNIT_PCA_LMU_3_E,
       {0x00000000, BITS_TO_BYTES_ALIGNMENT_MAC(186), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}

    ,{PRV_DXCH_SIP6_TABLE_LMU_CONFIGURATION_TABLE_E + 0,
       { CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 0, PRV_CPSS_DXCH_UNIT_PCA_LMU_0_E,
       { 0x00004000, BITS_TO_BYTES_ALIGNMENT_MAC(90), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS }, GT_FALSE }}
    ,{PRV_DXCH_SIP6_TABLE_LMU_CONFIGURATION_TABLE_E + 1,
       { CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 1, PRV_CPSS_DXCH_UNIT_PCA_LMU_1_E,
       { 0x00004000, BITS_TO_BYTES_ALIGNMENT_MAC(90), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS }, GT_FALSE }}
    ,{PRV_DXCH_SIP6_TABLE_LMU_CONFIGURATION_TABLE_E + 2,
       { CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 2, PRV_CPSS_DXCH_UNIT_PCA_LMU_2_E,
       { 0x00004000, BITS_TO_BYTES_ALIGNMENT_MAC(90), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS }, GT_FALSE }}
    ,{PRV_DXCH_SIP6_TABLE_LMU_CONFIGURATION_TABLE_E + 3,
       { CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 3, PRV_CPSS_DXCH_UNIT_PCA_LMU_3_E,
       { 0x00004000, BITS_TO_BYTES_ALIGNMENT_MAC(90), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS }, GT_FALSE }}

    /* IPCL */
    ,{PRV_DXCH_SIP6_10_TABLE_IPCL0_SOURCE_PORT_CONFIG_E,
      {CPSS_DXCH_SIP6_10_TABLE_IPCL0_SOURCE_PORT_CONFIG_E, PRV_CPSS_DXCH_UNIT_PCL_E,
      { 0x00003000, BITS_TO_BYTES_ALIGNMENT_MAC(24), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS }, GT_FALSE }}

    ,{PRV_DXCH_SIP6_10_TABLE_IPCL1_SOURCE_PORT_CONFIG_E,
      {CPSS_DXCH_SIP6_10_TABLE_IPCL1_SOURCE_PORT_CONFIG_E, PRV_CPSS_DXCH_UNIT_PCL_E,
      { 0x00004000, BITS_TO_BYTES_ALIGNMENT_MAC(24), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS }, GT_FALSE }}

    ,{PRV_DXCH_SIP6_10_TABLE_IPCL2_SOURCE_PORT_CONFIG_E,
      {CPSS_DXCH_SIP6_10_TABLE_IPCL2_SOURCE_PORT_CONFIG_E, PRV_CPSS_DXCH_UNIT_PCL_E,
      { 0x00005000, BITS_TO_BYTES_ALIGNMENT_MAC(24), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS }, GT_FALSE }}

    /* new table */
    ,{PRV_DXCH_SIP6_10_TABLE_EXACT_MATCH_AUTO_LEARNED_ENTRY_INDEX_E,
      {CPSS_DXCH_SIP6_10_TABLE_EXACT_MATCH_AUTO_LEARNED_ENTRY_INDEX_E, PRV_CPSS_DXCH_UNIT_EM_E,
      { 0x00040000, BITS_TO_BYTES_ALIGNMENT_MAC(20), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS }, GT_FALSE }}

    /* EPCL */
    ,{PRV_DXCH_SIP6_10_TABLE_EGRESS_PCL_QUEUE_GROUP_LATENCY_PROFILE_CONFIG_E,
        {CPSS_DXCH_SIP6_10_TABLE_EGRESS_PCL_QUEUE_GROUP_LATENCY_PROFILE_CONFIG_E,  PRV_CPSS_DXCH_UNIT_EPCL_E,
        {0x00017000, BITS_TO_BYTES_ALIGNMENT_MAC(9), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}

    /* IPFIX FIRST N PACKETS */
    ,{PRV_DXCH_SIP6_10_TABLE_INGRESS_POLICER_0_IPFIX_FIRST_N_PACKETS_E,
    {CPSS_DXCH_SIP6_10_TABLE_INGRESS_POLICER_0_IPFIX_FIRST_N_PACKETS_E, PRV_CPSS_DXCH_UNIT_IPLR_E,
    {0x00070000, BITS_TO_BYTES_ALIGNMENT_MAC(32), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}

    ,{PRV_DXCH_SIP6_10_TABLE_INGRESS_POLICER_1_IPFIX_FIRST_N_PACKETS_E,
    {CPSS_DXCH_SIP6_10_TABLE_INGRESS_POLICER_1_IPFIX_FIRST_N_PACKETS_E, PRV_CPSS_DXCH_UNIT_IPLR_1_E,
    {0x00070000, BITS_TO_BYTES_ALIGNMENT_MAC(32), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}

    ,{PRV_DXCH_SIP6_10_TABLE_EGRESS_POLICER_IPFIX_FIRST_N_PACKETS_E,
    {CPSS_DXCH_SIP6_10_TABLE_EGRESS_POLICER_IPFIX_FIRST_N_PACKETS_E, PRV_CPSS_DXCH_UNIT_EPLR_E,
    {0x00070000, BITS_TO_BYTES_ALIGNMENT_MAC(32), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}

    /* EQ */
    ,{PRV_DXCH_SIP6_10_TABLE_SOURCE_PORT_HASH_ENTRY_E,
        {CPSS_DXCH_SIP6_10_TABLE_SOURCE_PORT_HASH_ENTRY_E,  PRV_CPSS_DXCH_UNIT_EQ_E,
        {0x000e0000, BITS_TO_BYTES_ALIGNMENT_MAC(16), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}, GT_FALSE}}

    PRV_CPSS_DXCH_SIP6_10_TABLE_CNC_COUNTERS_MAC(0)
    PRV_CPSS_DXCH_SIP6_10_TABLE_CNC_COUNTERS_MAC(1)
    PRV_CPSS_DXCH_SIP6_10_TABLE_CNC_COUNTERS_MAC(2)
    PRV_CPSS_DXCH_SIP6_10_TABLE_CNC_COUNTERS_MAC(3)
    PRV_CPSS_DXCH_SIP6_10_TABLE_CNC_COUNTERS_MAC(4)
    PRV_CPSS_DXCH_SIP6_10_TABLE_CNC_COUNTERS_MAC(5)
    PRV_CPSS_DXCH_SIP6_10_TABLE_CNC_COUNTERS_MAC(6)
    PRV_CPSS_DXCH_SIP6_10_TABLE_CNC_COUNTERS_MAC(7)
    PRV_CPSS_DXCH_SIP6_10_TABLE_CNC_COUNTERS_MAC(8)
    PRV_CPSS_DXCH_SIP6_10_TABLE_CNC_COUNTERS_MAC(9)
    PRV_CPSS_DXCH_SIP6_10_TABLE_CNC_COUNTERS_MAC(10)
    PRV_CPSS_DXCH_SIP6_10_TABLE_CNC_COUNTERS_MAC(11)
    PRV_CPSS_DXCH_SIP6_10_TABLE_CNC_COUNTERS_MAC(12)
    PRV_CPSS_DXCH_SIP6_10_TABLE_CNC_COUNTERS_MAC(13)
    PRV_CPSS_DXCH_SIP6_10_TABLE_CNC_COUNTERS_MAC(14)
    PRV_CPSS_DXCH_SIP6_10_TABLE_CNC_COUNTERS_MAC(15)
    PRV_CPSS_DXCH_SIP6_10_TABLE_CNC_COUNTERS_MAC(16)
    PRV_CPSS_DXCH_SIP6_10_TABLE_CNC_COUNTERS_MAC(17)
    PRV_CPSS_DXCH_SIP6_10_TABLE_CNC_COUNTERS_MAC(18)
    PRV_CPSS_DXCH_SIP6_10_TABLE_CNC_COUNTERS_MAC(19)
    PRV_CPSS_DXCH_SIP6_10_TABLE_CNC_COUNTERS_MAC(20)
    PRV_CPSS_DXCH_SIP6_10_TABLE_CNC_COUNTERS_MAC(21)
    PRV_CPSS_DXCH_SIP6_10_TABLE_CNC_COUNTERS_MAC(22)
    PRV_CPSS_DXCH_SIP6_10_TABLE_CNC_COUNTERS_MAC(23)
    PRV_CPSS_DXCH_SIP6_10_TABLE_CNC_COUNTERS_MAC(24)
    PRV_CPSS_DXCH_SIP6_10_TABLE_CNC_COUNTERS_MAC(25)
    PRV_CPSS_DXCH_SIP6_10_TABLE_CNC_COUNTERS_MAC(26)
    PRV_CPSS_DXCH_SIP6_10_TABLE_CNC_COUNTERS_MAC(27)
    PRV_CPSS_DXCH_SIP6_10_TABLE_CNC_COUNTERS_MAC(28)
    PRV_CPSS_DXCH_SIP6_10_TABLE_CNC_COUNTERS_MAC(29)
    PRV_CPSS_DXCH_SIP6_10_TABLE_CNC_COUNTERS_MAC(30)
    PRV_CPSS_DXCH_SIP6_10_TABLE_CNC_COUNTERS_MAC(31)

    /* NOTE: addresses in this table MUST be '0' based (relative to unit) and
       not global address of the device*/
    ,{PRV_DXCH_DEVICE_TABLE___LAST_E , {0, 0, {0, 0, 0}, GT_FALSE}}
};

/*  table information , that need to override Falcon info from hawkTablesInfo */
/* NOTE : the entries in this array need to be matched with entries in
   falconTablesInfo , according to field */
static const PRV_CPSS_DXCH_TABLES_INFO_EXT_STC hawkTablesInfo_overrideFalcon[] =
{
    /* there is bug ... it is not enough to set this info in hawk_directAccessTableInfo_overrideFalcon[] */

    {CPSS_DXCH_LION_TABLE_TXQ_NON_TRUNK_MEMBERS_E,
        OVERRIDE_INFO_WIDTH_MAC(BMP_128_PORTS_CNS)
    },
    {CPSS_DXCH_LION_TABLE_TXQ_DESIGNATED_PORT_E,
        OVERRIDE_INFO_WIDTH_MAC(BMP_128_PORTS_CNS)
    },

    {CPSS_DXCH_TABLE_PCL_CONFIG_E,
        OVERRIDE_INFO_WIDTH_MAC(SIP6_10_PCL_PCL_CONFIG_BITS_SIZE_CNS)},
    {CPSS_DXCH_XCAT2_TABLE_INGRESS_PCL_LOOKUP01_CONFIG_E,
        OVERRIDE_INFO_WIDTH_MAC(SIP6_10_PCL_PCL_CONFIG_BITS_SIZE_CNS)},
    {CPSS_DXCH_XCAT_TABLE_INGRESS_PCL_LOOKUP1_CONFIG_E,
        OVERRIDE_INFO_WIDTH_MAC(SIP6_10_PCL_PCL_CONFIG_BITS_SIZE_CNS)},

    {CPSS_DXCH_SIP5_TABLE_IPCL0_UDB_SELECT_E,
        OVERRIDE_INFO_WIDTH_MAC(SIP6_10_IPCL_UDB_SELECT_BITS_SIZE_CNS)
    },
    {CPSS_DXCH_SIP5_TABLE_IPCL1_UDB_SELECT_E,
        OVERRIDE_INFO_WIDTH_MAC(SIP6_10_IPCL_UDB_SELECT_BITS_SIZE_CNS)
    },
    {CPSS_DXCH_SIP5_TABLE_IPCL2_UDB_SELECT_E,
        OVERRIDE_INFO_WIDTH_MAC(SIP6_10_IPCL_UDB_SELECT_BITS_SIZE_CNS)
    },
    {CPSS_DXCH_LION_TABLE_TRUNK_HASH_MASK_CRC_E,
        OVERRIDE_INFO_WIDTH_MAC(SIP6_10_PCL_TRUNK_HASH_MASK_CRC_BITS_SIZE_CNS)
    },

    {CPSS_DXCH_SIP6_TXQ_PFCC_PFCC_CFG_E,
       OVERRIDE_INFO_NUM_ENTRIES_AND_WIDTH_MAC(SIP6_10_PFCC_CFG__TABLE_SIZE_CNS,SIP6_10_PFCC_CFG__ENTRY_BITS_SIZE_CNS)
    },

    /* HA */
    { CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E,
        OVERRIDE_INFO_WIDTH_MAC(SIP6_10_HA_EGRESS_EPORT_1_BITS_SIZE_CNS)
    },

    { CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_1_E,
        OVERRIDE_INFO_WIDTH_MAC(SIP6_10_HA_EGRESS_PHYSICAL_PORT_1_BITS_SIZE_CNS)
    },

    {CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
        OVERRIDE_INFO_WIDTH_MAC(SIP6_10_HA_EGRESS_PHYSICAL_PORT_2_BITS_SIZE_CNS)
    },

    {CPSS_DXCH_SIP5_TABLE_HA_EPCL_UDB_CONFIG_E,
        OVERRIDE_INFO_WIDTH_MAC(SIP6_10_HA_EPCL_UDB_CONFIG_BITS_SIZE_CNS)
    },


    /* TTI */
    { CPSS_DXCH_SIP5_TABLE_TTI_PHYSICAL_PORT_ATTRIBUTE_E,
        OVERRIDE_INFO_WIDTH_MAC(SIP6_10_TTI_PHYSICAL_PORT_BITS_SIZE_CNS)
    },

    {CPSS_DXCH_SIP5_TABLE_TTI_UDB_CONFIG_E,
        OVERRIDE_INFO_WIDTH_MAC(SIP6_10_TTI_UDB_CONFIG_BITS_SIZE_CNS)
    },

    /* ROUTER_ARP_DA and TUNNEL_START */
    {CPSS_DXCH2_TABLE_TUNNEL_START_CONFIG_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(_32K)
    },

    /* EPCL */
    {CPSS_DXCH_SIP5_TABLE_EPCL_UDB_SELECT_E,
        OVERRIDE_INFO_WIDTH_MAC(SIP6_10_EPCL_UDB_SELECT_BITS_SIZE_CNS)
    },
    {CPSS_DXCH_SIP6_TABLE_EGRESS_PCL_EXACT_MATCH_PROFILE_ID_MAPPING_E,
        OVERRIDE_INFO_NUM_ENTRIES_AND_WIDTH_MAC(256, 32) /* 256 entries aligned to 32 bits */
    },

    /* L2I */
    {CPSS_DXCH_LION_TABLE_STG_INGRESS_E,
        OVERRIDE_INFO_NUM_ENTRIES_AND_WIDTH_MAC(4096, (2 * BMP_128_PORTS_CNS))/* 2 bits per port */
    },

    /* LMU - the entry size is less than the alignment */
    {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 0  , OVERRIDE_INFO_WIDTH_MAC(90)},
    {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 1  , OVERRIDE_INFO_WIDTH_MAC(90)},
    {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 2  , OVERRIDE_INFO_WIDTH_MAC(90)},
    {CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 3  , OVERRIDE_INFO_WIDTH_MAC(90)},

    /* PPU tables - the entry size is less than the alignment */
    {CPSS_DXCH_SIP6_10_TABLE_PPU_ACTION_TABLE_0_E  , OVERRIDE_INFO_WIDTH_MAC(379)},
    {CPSS_DXCH_SIP6_10_TABLE_PPU_ACTION_TABLE_1_E  , OVERRIDE_INFO_WIDTH_MAC(379)},
    {CPSS_DXCH_SIP6_10_TABLE_PPU_ACTION_TABLE_2_E  , OVERRIDE_INFO_WIDTH_MAC(379)},
    {CPSS_DXCH_SIP6_10_TABLE_PPU_DAU_PROFILE_TABLE_E  , OVERRIDE_INFO_WIDTH_MAC(480)}

};

/*xxx*/
/* tables that are per DP in Phoenix -- single DP device  */
#define PHOENIX_DP_OVERRIDE_TABLE_NUM_ENTRIES(table,entryType,divider,additionalFixedNumberOfEntries)        \
     PER_DP_OVERRIDE_TABLE_NUM_ENTRIES(table,entryType,divider,additionalFixedNumberOfEntries,0/*dpIndex*/)  \
    ,NOT_VALID_TABLE_MAC(table+ 1) \
    ,NOT_VALID_TABLE_MAC(table+ 2) \
    ,NOT_VALID_TABLE_MAC(table+ 3) \
    ,NOT_VALID_TABLE_MAC(table+ 4) \
    ,NOT_VALID_TABLE_MAC(table+ 5) \
    ,NOT_VALID_TABLE_MAC(table+ 6) \
    ,NOT_VALID_TABLE_MAC(table+ 7) \
    ,NOT_VALID_TABLE_MAC(table+ 8) \
    ,NOT_VALID_TABLE_MAC(table+ 9) \
    ,NOT_VALID_TABLE_MAC(table+10) \
    ,NOT_VALID_TABLE_MAC(table+11) \
    ,NOT_VALID_TABLE_MAC(table+12) \
    ,NOT_VALID_TABLE_MAC(table+13) \
    ,NOT_VALID_TABLE_MAC(table+14) \
    ,NOT_VALID_TABLE_MAC(table+15) \
    ,NOT_VALID_TABLE_MAC(table+16) \
    ,NOT_VALID_TABLE_MAC(table+17) \
    ,NOT_VALID_TABLE_MAC(table+18) \
    ,NOT_VALID_TABLE_MAC(table+19) \
    ,NOT_VALID_TABLE_MAC(table+20) \
    ,NOT_VALID_TABLE_MAC(table+21) \
    ,NOT_VALID_TABLE_MAC(table+22) \
    ,NOT_VALID_TABLE_MAC(table+23) \
    ,NOT_VALID_TABLE_MAC(table+24) \
    ,NOT_VALID_TABLE_MAC(table+25) \
    ,NOT_VALID_TABLE_MAC(table+26) \
    ,NOT_VALID_TABLE_MAC(table+27) \
    ,NOT_VALID_TABLE_MAC(table+28) \
    ,NOT_VALID_TABLE_MAC(table+29) \
    ,NOT_VALID_TABLE_MAC(table+30) \
    ,NOT_VALID_TABLE_MAC(table+31)

/* invalidate not existing per DP tables  */
#define PHOENIX_DP_INVALIDATE_NOT_EXISTING_INSTANCES(table)        \
     NOT_VALID_TABLE_MAC(table+ 1) \
    ,NOT_VALID_TABLE_MAC(table+ 2) \
    ,NOT_VALID_TABLE_MAC(table+ 3) \
    ,NOT_VALID_TABLE_MAC(table+ 4) \
    ,NOT_VALID_TABLE_MAC(table+ 5) \
    ,NOT_VALID_TABLE_MAC(table+ 6) \
    ,NOT_VALID_TABLE_MAC(table+ 7) \
    ,NOT_VALID_TABLE_MAC(table+ 8) \
    ,NOT_VALID_TABLE_MAC(table+ 9) \
    ,NOT_VALID_TABLE_MAC(table+10) \
    ,NOT_VALID_TABLE_MAC(table+11) \
    ,NOT_VALID_TABLE_MAC(table+12) \
    ,NOT_VALID_TABLE_MAC(table+13) \
    ,NOT_VALID_TABLE_MAC(table+14) \
    ,NOT_VALID_TABLE_MAC(table+15) \
    ,NOT_VALID_TABLE_MAC(table+16) \
    ,NOT_VALID_TABLE_MAC(table+17) \
    ,NOT_VALID_TABLE_MAC(table+18) \
    ,NOT_VALID_TABLE_MAC(table+19) \
    ,NOT_VALID_TABLE_MAC(table+20) \
    ,NOT_VALID_TABLE_MAC(table+21) \
    ,NOT_VALID_TABLE_MAC(table+22) \
    ,NOT_VALID_TABLE_MAC(table+23) \
    ,NOT_VALID_TABLE_MAC(table+24) \
    ,NOT_VALID_TABLE_MAC(table+25) \
    ,NOT_VALID_TABLE_MAC(table+26) \
    ,NOT_VALID_TABLE_MAC(table+27) \
    ,NOT_VALID_TABLE_MAC(table+28) \
    ,NOT_VALID_TABLE_MAC(table+29) \
    ,NOT_VALID_TABLE_MAC(table+30) \
    ,NOT_VALID_TABLE_MAC(table+31)


/* tables that are per Tile in Phoenix -- single Tile  */
#define PHOENIX_TILE_OVERRIDE_TABLE_NUM_ENTRIES(table,entryType,divider,additionalFixedNumberOfEntries)        \
     PER_DP_OVERRIDE_TABLE_NUM_ENTRIES(table,entryType,divider,additionalFixedNumberOfEntries,0/*dpIndex*/)  \
    ,NOT_VALID_TABLE_MAC(table+ 1) \
    ,NOT_VALID_TABLE_MAC(table+ 2) \
    ,NOT_VALID_TABLE_MAC(table+ 3) \

#define PHOENIX_DP_OVERRIDE_TABLE_AND_ADDR(table,addr,numOfBits)       \
     PER_DP_OVERRIDE_TABLE_AND_ADDR(table,addr       ,  numOfBits   , 0/*dpIndex*/) \
    ,PER_DP_OVERRIDE_TABLE_AND_ADDR(table,0x1/*addr*/,0/*numOfBits*/, 1/*dpIndex*/) \
    ,PER_DP_OVERRIDE_TABLE_AND_ADDR(table,0x1/*addr*/,0/*numOfBits*/, 2/*dpIndex*/) \
    ,PER_DP_OVERRIDE_TABLE_AND_ADDR(table,0x1/*addr*/,0/*numOfBits*/, 3/*dpIndex*/)

#define PHOENIX_CENTRAL_OVERRIDE_TABLE_AND_ADDR(table,addr,numOfBits)       \
     PER_DP_OVERRIDE_TABLE_AND_ADDR(table,addr       ,  numOfBits   , 0/*dpIndex*/)

/* Phoenix table - in runtime copy table from this of Hawk ... and continue from there ... */
static const GT_U32 phoenixTableInfoSize = CPSS_DXCH_SIP6_15_TABLE_LAST_E;

/* size of table phoenix_directAccessTableInfo */
static const GT_U32 phoenix_directAccessTableInfoSize = PRV_DXCH_SIP6_15_TABLE___LAST_E;

/* SIP6_15 : info about the number of entries in the tables (override sip6_10_tableNumEntriesInfoArr) */
static const TABLE_NUM_ENTRIES_INFO_STC   sip6_15_tableNumEntriesInfoArr[] =
{

    /*PDS*/
    PHOENIX_DP_OVERRIDE_TABLE_NUM_ENTRIES(CPSS_DXCH_SIP6_TXQ_PDS_PER_QUEUE_COUNTERS_E,ENTRY_TYPE_EXPLICIT_NUM_E,0,512)
   ,PHOENIX_DP_OVERRIDE_TABLE_NUM_ENTRIES(CPSS_DXCH_SIP6_TXQ_PDS_DATA_STORAGE_E,ENTRY_TYPE_EXPLICIT_NUM_E,0,4080)
   ,PHOENIX_DP_OVERRIDE_TABLE_NUM_ENTRIES(CPSS_DXCH_SIP6_TXQ_PDS_NXT_TABLE_E,ENTRY_TYPE_EXPLICIT_NUM_E,0,3452)
   ,PHOENIX_DP_OVERRIDE_TABLE_NUM_ENTRIES(CPSS_DXCH_SIP6_TXQ_PDS_TAIL_TAIL_E,ENTRY_TYPE_EXPLICIT_NUM_E,0,512)
   ,PHOENIX_DP_OVERRIDE_TABLE_NUM_ENTRIES(CPSS_DXCH_SIP6_TXQ_PDS_TAIL_HEAD_E,ENTRY_TYPE_EXPLICIT_NUM_E,0,512)
   ,PHOENIX_DP_OVERRIDE_TABLE_NUM_ENTRIES(CPSS_DXCH_SIP6_TXQ_PDS_HEAD_TAIL_E,ENTRY_TYPE_EXPLICIT_NUM_E,0,512)
   ,PHOENIX_DP_OVERRIDE_TABLE_NUM_ENTRIES(CPSS_DXCH_SIP6_TXQ_PDS_HEAD_HEAD_E,ENTRY_TYPE_EXPLICIT_NUM_E,0,512)
   ,PHOENIX_DP_OVERRIDE_TABLE_NUM_ENTRIES(CPSS_DXCH_SIP6_TXQ_PDS_FRAG_TAIL_E,ENTRY_TYPE_EXPLICIT_NUM_E,0,512)
   ,PHOENIX_DP_OVERRIDE_TABLE_NUM_ENTRIES(CPSS_DXCH_SIP6_TXQ_PDS_FRAG_HEAD_E,ENTRY_TYPE_EXPLICIT_NUM_E,0,512)

    /* following tables has same size, just invalidate not existing instances */
    ,PHOENIX_DP_INVALIDATE_NOT_EXISTING_INSTANCES(CPSS_DXCH_SIP6_TXQ_PDS_PID_TABLE_E)

    /* SDQ */

    ,PHOENIX_DP_OVERRIDE_TABLE_NUM_ENTRIES(CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_CFG_E,
        ENTRY_TYPE_DP_Q_E,0,0)

     ,PHOENIX_DP_OVERRIDE_TABLE_NUM_ENTRIES(CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_CRDT_BLNC_E,
        ENTRY_TYPE_DP_Q_E,0,0)

    ,PHOENIX_DP_OVERRIDE_TABLE_NUM_ENTRIES(CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_ELIG_STATE_E,
       ENTRY_TYPE_DP_Q_E,0,0)

   ,PHOENIX_DP_OVERRIDE_TABLE_NUM_ENTRIES(CPSS_DXCH_SIP6_TXQ_SDQ_PORT_CRDT_BLNC_E,ENTRY_TYPE_EXPLICIT_NUM_E,0,56)

   ,PHOENIX_DP_OVERRIDE_TABLE_NUM_ENTRIES(CPSS_DXCH_SIP6_TXQ_SDQ_SEL_LIST_PTRS_E,ENTRY_TYPE_EXPLICIT_NUM_E,0,112)
   ,{CPSS_DXCH_SIP6_15_TXQ_SDQ_PORT_PFC_STATE_E,ENTRY_TYPE_EXPLICIT_NUM_E,0,56}
    /*TXQ - PFCC*/
    ,PHOENIX_TILE_OVERRIDE_TABLE_NUM_ENTRIES(CPSS_DXCH_SIP6_TXQ_PFCC_PFCC_CFG_E,ENTRY_TYPE_EXPLICIT_NUM_E,0,96)

    ,{CPSS_DXCH_LION_TABLE_STG_INGRESS_E,
        ENTRY_TYPE_STG_MEMBERS_E,0,0}

    /* IPvx */
    ,{CPSS_DXCH_SIP6_TABLE_IPVX_ECMP_E, /* replace CPSS_DXCH_SIP5_TABLE_ECMP_E */
        ENTRY_TYPE_EXPLICIT_NUM_E,VERTICAL_INDICATION_CNS | 2,  _2K}/* 2 entries per line */

    ,{CPSS_DXCH_TABLE_PCL_CONFIG_E,
        ENTRY_TYPE_EXPLICIT_NUM_E,0,4224}
    ,{CPSS_DXCH_XCAT2_TABLE_INGRESS_PCL_LOOKUP01_CONFIG_E,
        ENTRY_TYPE_EXPLICIT_NUM_E,0,4224}
    ,{CPSS_DXCH_XCAT_TABLE_INGRESS_PCL_LOOKUP1_CONFIG_E,
        ENTRY_TYPE_EXPLICIT_NUM_E,0,4224}
    ,{CPSS_DXCH2_TABLE_EGRESS_PCL_CONFIG_E,
        ENTRY_TYPE_EXPLICIT_NUM_E,0,4224}

    /* LPM Aging */
    ,{ CPSS_DXCH_SIP6_TABLE_LPM_AGING_E,
        ENTRY_TYPE_EXPLICIT_NUM_E,0,4680}

    /* AC5X has single LMU. Invalidate non existing tables */
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 1,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 2,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 3,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}

    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 1,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 2,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 3,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}

    /* AC5X, Harrier has only one CNC unit - 16 blocks */
    ,{CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 16,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 17,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 18,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 19,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 20,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 21,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 22,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 23,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 24,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 25,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 26,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 27,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 28,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 29,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP5_TABLE_CNC_0_COUNTERS_E + 30,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP5_TABLE_CNC_31_COUNTERS_E,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_10_TABLE_CNC_32_COUNTERS_E + 0,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_10_TABLE_CNC_32_COUNTERS_E + 1,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_10_TABLE_CNC_32_COUNTERS_E + 2,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_10_TABLE_CNC_32_COUNTERS_E + 3,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_10_TABLE_CNC_32_COUNTERS_E + 4,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_10_TABLE_CNC_32_COUNTERS_E + 5,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_10_TABLE_CNC_32_COUNTERS_E + 6,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_10_TABLE_CNC_32_COUNTERS_E + 7,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_10_TABLE_CNC_32_COUNTERS_E + 8,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_10_TABLE_CNC_32_COUNTERS_E + 9,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_10_TABLE_CNC_32_COUNTERS_E + 10,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_10_TABLE_CNC_32_COUNTERS_E + 11,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_10_TABLE_CNC_32_COUNTERS_E + 12,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_10_TABLE_CNC_32_COUNTERS_E + 13,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_10_TABLE_CNC_32_COUNTERS_E + 14,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_10_TABLE_CNC_32_COUNTERS_E + 15,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_10_TABLE_CNC_32_COUNTERS_E + 16,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_10_TABLE_CNC_32_COUNTERS_E + 17,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_10_TABLE_CNC_32_COUNTERS_E + 18,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_10_TABLE_CNC_32_COUNTERS_E + 19,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_10_TABLE_CNC_32_COUNTERS_E + 20,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_10_TABLE_CNC_32_COUNTERS_E + 21,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_10_TABLE_CNC_32_COUNTERS_E + 22,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_10_TABLE_CNC_32_COUNTERS_E + 23,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_10_TABLE_CNC_32_COUNTERS_E + 24,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_10_TABLE_CNC_32_COUNTERS_E + 25,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_10_TABLE_CNC_32_COUNTERS_E + 26,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_10_TABLE_CNC_32_COUNTERS_E + 27,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_10_TABLE_CNC_32_COUNTERS_E + 28,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_10_TABLE_CNC_32_COUNTERS_E + 29,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_10_TABLE_CNC_32_COUNTERS_E + 30,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}
    ,{CPSS_DXCH_SIP6_10_TABLE_CNC_63_COUNTERS_E,
      ENTRY_TYPE_NOT_VALID_E, 0, 0}

    /* must be last */
    ,{CPSS_DXCH_TABLE_LAST_E , ENTRY_TYPE___LAST__E , 0, 0}
};
/*  table information , that need to override Hawk info from hawkTablesInfo */
/* NOTE : the entries in this array need to be matched with entries in
   falconTablesInfo , according to field */
static const PRV_CPSS_DXCH_TABLES_INFO_EXT_STC phoenixTablesInfo_overrideHawk[] =
{
    /* there is bug ... it is not enough to set this info in phoenix_directAccessTableInfo_overrideFalcon[] */

    {CPSS_DXCH_TABLE_FDB_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(_32K)
    },



};
/* Phoenix direct table information , that need to override Hawk info from phoenix_directAccessTableInfo */
static const PRV_CPSS_DXCH_TABLES_INFO_DIRECT_EXT_STC phoenix_directAccessTableInfo_overrideHawk[] =
{
    /*PDX*/
    PHOENIX_CENTRAL_OVERRIDE_TABLE_AND_ADDR(        CPSS_DXCH_SIP6_TXQ_PDX_DX_QGRPMAP_E,
        0x00000000/*addr*/,20/*bits*/)

    /*SDQ*/
    ,PHOENIX_DP_OVERRIDE_TABLE_AND_ADDR(CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_CRDT_BLNC_E,0x00009000/*addr*/,35/*bits*/)
    ,PHOENIX_DP_OVERRIDE_TABLE_AND_ADDR(CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_ELIG_STATE_E,0x0000b000/*addr*/,11/*bits*/)
    ,PHOENIX_DP_OVERRIDE_TABLE_AND_ADDR(CPSS_DXCH_SIP6_TXQ_SDQ_PORT_CRDT_BLNC_E,0x00008200/*addr*/,38/*bits*/)
    ,PHOENIX_DP_OVERRIDE_TABLE_AND_ADDR(CPSS_DXCH_SIP6_TXQ_SDQ_SEL_LIST_PTRS_E,0x00008600/*addr*/,20/*bits*/)

    /*PFCC*/
    ,PHOENIX_DP_OVERRIDE_TABLE_AND_ADDR(CPSS_DXCH_SIP6_TXQ_PFCC_PFCC_CFG_E,0x00001000/*addr*/,17/*bits*/)
    /* NOTE: the alignment when stated NOT_MODIFIED_VALUE_CNS :
        is taken auto calculated from hawkTablesInfo_overrideFalcon[] */
    ,{CPSS_DXCH_TABLE_LAST_E, {0,0,0}}
};

/* Phoenix direct table information , that are new (not exists in Hawk) */
/* register new tables that not exists in Hawk !!!! */
static const BOBK_TABLES_INFO_DIRECT_NEW_INDEX_STC phoenix_directAccessTableInfo_new[] =
{


  {PRV_DXCH_SIP6_15_TABLE_TXQ_SDQ_PORT_PFC_STATE_E,
         {CPSS_DXCH_SIP6_15_TXQ_SDQ_PORT_PFC_STATE_E, PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ0_E,
         {0x00008400/*addr*/, BITS_TO_BYTES_ALIGNMENT_MAC(16), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE}},

    /* NOTE: addresses in this table MUST be '0' based (relative to unit) and
       not global address of the device*/
    {PRV_DXCH_DEVICE_TABLE___LAST_E , {0, 0, {0, 0, 0}, GT_FALSE}}
};

#define IRONMAN_DP_OVERRIDE_TABLE_NUM_ENTRIES(table,entryType,divider,additionalFixedNumberOfEntries)           \
     PER_DP_OVERRIDE_TABLE_NUM_ENTRIES(table,entryType,divider,additionalFixedNumberOfEntries,0/*dpIndex*/)  \
    ,NOT_VALID_TABLE_MAC(table+ 1) \
    ,NOT_VALID_TABLE_MAC(table+ 2) \
    ,NOT_VALID_TABLE_MAC(table+ 3)


/* tables that are per DP that need override in hawk_directAccessTableInfo_overrideFalcon[] */
#define HARRIER_DP_OVERRIDE_TABLE_NUM_ENTRIES(table,entryType,divider,additionalFixedNumberOfEntries)           \
     PER_DP_OVERRIDE_TABLE_NUM_ENTRIES(table,entryType,divider,additionalFixedNumberOfEntries,0/*dpIndex*/)  \
    ,PER_DP_OVERRIDE_TABLE_NUM_ENTRIES(table,entryType,divider,additionalFixedNumberOfEntries,1/*dpIndex*/)  \
    ,PER_DP_OVERRIDE_TABLE_NUM_ENTRIES(table,entryType,divider,additionalFixedNumberOfEntries,2/*dpIndex*/)  \
    ,NOT_VALID_TABLE_MAC(table+ 3)

/* tables that are per DP that need override in hawk_directAccessTableInfo_overrideFalcon[] */
#define HARRIER_DP_CENTRAL_OVERRIDE_TABLE_NUM_ENTRIES(table,entryType,divider,additionalFixedNumberOfEntries)           \
     PER_DP_OVERRIDE_TABLE_NUM_ENTRIES(table,entryType,divider,additionalFixedNumberOfEntries,0/*dpIndex*/)  \


#define HARRIER_CENTRAL_OVERRIDE_TABLE_AND_ADDR(table,addr,numOfBits)       \
     PER_DP_OVERRIDE_TABLE_AND_ADDR(table,addr       ,  numOfBits   , 0/*dpIndex*/)

/* tables that are per DP that need override in hawk_directAccessTableInfo_overrideFalcon[] */
#define HARRIER_DP_OVERRIDE_TABLE_AND_ADDR(table,addr,numOfBits)           \
     PER_DP_OVERRIDE_TABLE_AND_ADDR(table,addr,numOfBits,0/*dpIndex*/)  \
    ,PER_DP_OVERRIDE_TABLE_AND_ADDR(table,addr,numOfBits,1/*dpIndex*/)  \
    ,PER_DP_OVERRIDE_TABLE_AND_ADDR(table,addr,numOfBits,2/*dpIndex*/)  \
    ,PER_DP_OVERRIDE_TABLE_AND_ADDR(table,0x1/*addr*/,0/*numOfBits*/,3/*dpIndex*/)

#define IRONMAN_DP_OVERRIDE_TABLE_AND_ADDR(table,addr,numOfBits)           \
     PER_DP_OVERRIDE_TABLE_AND_ADDR(table,addr,numOfBits,0/*dpIndex*/)  \
    ,PER_DP_OVERRIDE_TABLE_AND_ADDR(table,0x1/*addr*/,0/*numOfBits*/,1/*dpIndex*/) \
    ,PER_DP_OVERRIDE_TABLE_AND_ADDR(table,0x1/*addr*/,0/*numOfBits*/,2/*dpIndex*/) \
    ,PER_DP_OVERRIDE_TABLE_AND_ADDR(table,0x1/*addr*/,0/*numOfBits*/,3/*dpIndex*/)


/* Harrier table - in runtime copy table from this of Hawk ... and continue from there ... */
static const GT_U32 harrierTableInfoSize = CPSS_DXCH_SIP6_20_TABLE_LAST_E;

/* size of table harrier_directAccessTableInfo */
static const GT_U32 harrier_directAccessTableInfoSize = PRV_DXCH_SIP6_20_TABLE___LAST_E;

/* SIP6_20 : info about the number of entries in the tables (override sip6_10_tableNumEntriesInfoArr) */
static const TABLE_NUM_ENTRIES_INFO_STC   sip6_20_tableNumEntriesInfoArr[] =
{

    /*PDS*/
    HARRIER_DP_OVERRIDE_TABLE_NUM_ENTRIES(CPSS_DXCH_SIP6_TXQ_PDS_PER_QUEUE_COUNTERS_E,ENTRY_TYPE_EXPLICIT_NUM_E,0,512)
   ,HARRIER_DP_OVERRIDE_TABLE_NUM_ENTRIES(CPSS_DXCH_SIP6_TXQ_PDS_DATA_STORAGE_E,ENTRY_TYPE_EXPLICIT_NUM_E,0,4080)

   ,HARRIER_DP_OVERRIDE_TABLE_NUM_ENTRIES(CPSS_DXCH_SIP6_TXQ_PDS_NXT_TABLE_E,ENTRY_TYPE_EXPLICIT_NUM_E,0,3452)
   ,HARRIER_DP_OVERRIDE_TABLE_NUM_ENTRIES(CPSS_DXCH_SIP6_TXQ_PDS_TAIL_TAIL_E,ENTRY_TYPE_EXPLICIT_NUM_E,0,512)
   ,HARRIER_DP_OVERRIDE_TABLE_NUM_ENTRIES(CPSS_DXCH_SIP6_TXQ_PDS_TAIL_HEAD_E,ENTRY_TYPE_EXPLICIT_NUM_E,0,512)
   ,HARRIER_DP_OVERRIDE_TABLE_NUM_ENTRIES(CPSS_DXCH_SIP6_TXQ_PDS_HEAD_TAIL_E,ENTRY_TYPE_EXPLICIT_NUM_E,0,512)
   ,HARRIER_DP_OVERRIDE_TABLE_NUM_ENTRIES(CPSS_DXCH_SIP6_TXQ_PDS_HEAD_HEAD_E,ENTRY_TYPE_EXPLICIT_NUM_E,0,512)
   ,HARRIER_DP_OVERRIDE_TABLE_NUM_ENTRIES(CPSS_DXCH_SIP6_TXQ_PDS_FRAG_TAIL_E,ENTRY_TYPE_EXPLICIT_NUM_E,0,512)
   ,HARRIER_DP_OVERRIDE_TABLE_NUM_ENTRIES(CPSS_DXCH_SIP6_TXQ_PDS_FRAG_HEAD_E,ENTRY_TYPE_EXPLICIT_NUM_E,0,512)

    /* SDQ */
    ,HARRIER_DP_OVERRIDE_TABLE_NUM_ENTRIES(CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_CFG_E,ENTRY_TYPE_DP_Q_E,0,0)
    ,HARRIER_DP_OVERRIDE_TABLE_NUM_ENTRIES(CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_CRDT_BLNC_E,ENTRY_TYPE_DP_Q_E,0,0)
    ,HARRIER_DP_OVERRIDE_TABLE_NUM_ENTRIES(CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_ELIG_STATE_E,ENTRY_TYPE_DP_Q_E,0,0)
    ,HARRIER_DP_OVERRIDE_TABLE_NUM_ENTRIES(CPSS_DXCH_SIP6_TXQ_SDQ_PORT_CRDT_BLNC_E,ENTRY_TYPE_EXPLICIT_NUM_E,0,27)
    ,HARRIER_DP_OVERRIDE_TABLE_NUM_ENTRIES(CPSS_DXCH_SIP6_TXQ_SDQ_SEL_LIST_PTRS_E,ENTRY_TYPE_EXPLICIT_NUM_E,0,54)
    ,{CPSS_DXCH_SIP6_15_TXQ_SDQ_PORT_PFC_STATE_E,ENTRY_TYPE_EXPLICIT_NUM_E,0,27}
    /*TXQ - PFCC*/
    ,HARRIER_DP_OVERRIDE_TABLE_NUM_ENTRIES(CPSS_DXCH_SIP6_TXQ_PFCC_PFCC_CFG_E,ENTRY_TYPE_EXPLICIT_NUM_E,0,148)

    ,{CPSS_DXCH_LION_TABLE_STG_INGRESS_E,
        ENTRY_TYPE_STG_MEMBERS_E,0,0}

    /* IPvx */
    ,{CPSS_DXCH_SIP6_TABLE_IPVX_ECMP_E, /* replace CPSS_DXCH_SIP5_TABLE_ECMP_E */
        ENTRY_TYPE_EXPLICIT_NUM_E,VERTICAL_INDICATION_CNS | 2,  _2K}/* 2 entries per line */

    ,{CPSS_DXCH_SIP6_20_EGF_QAG_PORT_VLAN_Q_OFFSET_E,ENTRY_TYPE_EXPLICIT_NUM_E,0,6144}

    ,{CPSS_DXCH3_TABLE_MAC2ME_E,
        ENTRY_TYPE_MAC_2_ME_E,0,0}

    /* Harrier has 3 LMUs, add tables of LMUs 1-2 */
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 1,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E + 2,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}

    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 1,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}
    ,{CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E + 2,
      ENTRY_TYPE_EXPLICIT_NUM_E, 0, 512}

    /* must be last */
    ,{CPSS_DXCH_TABLE_LAST_E , ENTRY_TYPE___LAST__E , 0, 0}
};
/*  table information , that need to override Hawk info from hawkTablesInfo */
/* NOTE : the entries in this array need to be matched with entries in
   falconTablesInfo , according to field */
static const PRV_CPSS_DXCH_TABLES_INFO_EXT_STC harrierTablesInfo_overridePhoenix[] =
{
    /* there is bug ... it is not enough to set this info in harrier_directAccessTableInfo_overrideFalcon[] */

    {CPSS_DXCH_TABLE_FDB_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(_32K)
    },



};
/* Harrier direct table information , that need to override Hawk info from harrier_directAccessTableInfo */
static const PRV_CPSS_DXCH_TABLES_INFO_DIRECT_EXT_STC harrier_directAccessTableInfo_overridePhoenix[] =
{
    /*PDX*/
    HARRIER_CENTRAL_OVERRIDE_TABLE_AND_ADDR(        CPSS_DXCH_SIP6_TXQ_PDX_DX_QGRPMAP_E,
        0x00000000/*addr*/,20/*bits*/)

    /*SDQ*/
    ,HARRIER_DP_OVERRIDE_TABLE_AND_ADDR(        CPSS_DXCH_SIP6_TXQ_SDQ_PORT_CRDT_BLNC_E,0x00008200/*addr*/,36/*bits*/)
    ,HARRIER_DP_OVERRIDE_TABLE_AND_ADDR(        CPSS_DXCH_SIP6_TXQ_SDQ_SEL_LIST_PTRS_E,0x00008600/*addr*/,18/*bits*/)


    /*PFCC*/
    ,HARRIER_DP_OVERRIDE_TABLE_AND_ADDR(CPSS_DXCH_SIP6_TXQ_PFCC_PFCC_CFG_E,0x00001000/*addr*/,18/*bits*/)
    /* NOTE: the alignment when stated NOT_MODIFIED_VALUE_CNS :
        is taken auto calculated from hawkTablesInfo_overrideFalcon[] */
    ,{CPSS_DXCH_TABLE_LAST_E, {0,0,0}}
};

/* Harrier direct table information , that are new (not exists in Hawk) */
/* register new tables that not exists in Hawk !!!! */
static const BOBK_TABLES_INFO_DIRECT_NEW_INDEX_STC harrier_directAccessTableInfo_new[] =
{


  {PRV_DXCH_SIP6_15_TABLE_TXQ_SDQ_PORT_PFC_STATE_E,
         {CPSS_DXCH_SIP6_15_TXQ_SDQ_PORT_PFC_STATE_E, PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ0_E,
         {0x00008400/*addr*/, BITS_TO_BYTES_ALIGNMENT_MAC(16), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE}},

  {PRV_DXCH_SIP6_20_EGF_QAG_PORT_VLAN_Q_OFFSET_E,
         {CPSS_DXCH_SIP6_20_EGF_QAG_PORT_VLAN_Q_OFFSET_E, PRV_CPSS_DXCH_UNIT_EGF_QAG_E,
         {0x000a8000/*addr*/, BITS_TO_BYTES_ALIGNMENT_MAC(16), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE}},

    /* NOTE: addresses in this table MUST be '0' based (relative to unit) and
       not global address of the device*/
    {PRV_DXCH_DEVICE_TABLE___LAST_E , {0, 0, {0, 0, 0}, GT_FALSE}}
};

/* ironman_L table - in runtime copy table from this of Harrier ... and continue from there ... */
static const GT_U32 ironman_L_TableInfoSize = CPSS_DXCH_SIP6_30_TABLE_LAST_E;

/* size of table ironman_L_directAccessTableInfo */
static const GT_U32 ironman_L_directAccessTableInfoSize = PRV_DXCH_SIP6_30_TABLE___LAST_E;

/* SIP6_30 : info about the number of entries in the tables (override sip6_10_tableNumEntriesInfoArr) */
static const TABLE_NUM_ENTRIES_INFO_STC   ironman_L_tableNumEntriesInfoArr[] =
{
     {CPSS_DXCH_XCAT_TABLE_INGRESS_PCL_LOOKUP1_CONFIG_E,
        ENTRY_TYPE_NOT_VALID_E,0,0} /* the table was removed from the device */

    ,{CPSS_DXCH_TABLE_PCL_CONFIG_E,
        ENTRY_TYPE_EXPLICIT_NUM_E,0,_4K+64/*4160*/}
    ,{CPSS_DXCH_XCAT2_TABLE_INGRESS_PCL_LOOKUP01_CONFIG_E,
        ENTRY_TYPE_EXPLICIT_NUM_E,0,_4K+64/*4160*/}

    ,{CPSS_DXCH_SIP5_TABLE_IPCL1_UDB_SELECT_E,
        ENTRY_TYPE_NOT_VALID_E,0,0} /* the table was removed from the device */
    ,{CPSS_DXCH_SIP5_TABLE_IPCL2_UDB_SELECT_E,
        ENTRY_TYPE_NOT_VALID_E,0,0} /* the table was removed from the device */

    ,{CPSS_DXCH_SIP5_TABLE_EPCL_UDB_SELECT_E,
        ENTRY_TYPE_NOT_VALID_E,0,0} /* the table was removed from the device */

    ,{CPSS_DXCH_SIP6_30_TABLE_IPCL0_UDB_REPLACEMENT_E,
        ENTRY_TYPE_EXPLICIT_NUM_E,0,112}
    ,{CPSS_DXCH_SIP6_30_TABLE_IPCL1_UDB_REPLACEMENT_E,
        ENTRY_TYPE_EXPLICIT_NUM_E,0,112}

    ,{CPSS_DXCH_SIP6_10_TABLE_IPCL2_SOURCE_PORT_CONFIG_E,
        ENTRY_TYPE_NOT_VALID_E,0,0}

    ,IRONMAN_DP_OVERRIDE_TABLE_NUM_ENTRIES(CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_CFG_E,ENTRY_TYPE_EXPLICIT_NUM_E,0,456)
    ,IRONMAN_DP_OVERRIDE_TABLE_NUM_ENTRIES(CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_CRDT_BLNC_E,ENTRY_TYPE_EXPLICIT_NUM_E,0,456)
    ,IRONMAN_DP_OVERRIDE_TABLE_NUM_ENTRIES(CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_ELIG_STATE_E,ENTRY_TYPE_EXPLICIT_NUM_E,0,456)
    ,IRONMAN_DP_OVERRIDE_TABLE_NUM_ENTRIES(CPSS_DXCH_SIP6_TXQ_SDQ_PORT_CRDT_BLNC_E,ENTRY_TYPE_EXPLICIT_NUM_E,0,57)
    ,IRONMAN_DP_OVERRIDE_TABLE_NUM_ENTRIES(CPSS_DXCH_SIP6_TXQ_SDQ_SEL_LIST_PTRS_E,ENTRY_TYPE_EXPLICIT_NUM_E,0,114)
    ,IRONMAN_DP_OVERRIDE_TABLE_NUM_ENTRIES(CPSS_DXCH_SIP6_15_TXQ_SDQ_PORT_PFC_STATE_E,ENTRY_TYPE_EXPLICIT_NUM_E,0,57)

    ,{CPSS_DXCH_SIP6_30_TABLE_TXQ_PDS_NEXT_DESC_E,
        ENTRY_TYPE_EXPLICIT_NUM_E,0,25064}

    ,{CPSS_DXCH_SIP6_30_TABLE_TXQ_PDS_WRITE_POINTER_E,
        ENTRY_TYPE_EXPLICIT_NUM_E,0,456}

    ,{CPSS_DXCH_SIP6_30_TABLE_TXQ_PDS_READ_POINTER_E,
        ENTRY_TYPE_EXPLICIT_NUM_E,0,456}

     ,{CPSS_DXCH_SIP6_30_TXQ_SDQ_QBV_CFG_E,
        ENTRY_TYPE_EXPLICIT_NUM_E,0,7424}

    ,PER_DP_OVERRIDE_TABLE_NUM_ENTRIES(CPSS_DXCH_SIP6_TXQ_PFCC_PFCC_CFG_E,ENTRY_TYPE_EXPLICIT_NUM_E,0,81,0/*dpNum*/)

    ,{CPSS_DXCH_SIP5_TABLE_INGRESS_BRIDGE_PORT_MEMBERS_E,
        ENTRY_TYPE_EVLAN_MEMBERS_E,0,0} /* override 'explicit' value set for sip6 device */
    ,{CPSS_DXCH_SIP5_TABLE_BRIDGE_PHYSICAL_PORT_E,
        ENTRY_TYPE_EPORT_E,0,0}         /* override 'explicit' value set for sip6 device */
    ,{CPSS_DXCH_SIP5_TABLE_BRIDGE_PHYSICAL_PORT_RATE_LIMIT_COUNTERS_E,
        ENTRY_TYPE_EPORT_E,0,0}         /* override 'explicit' value set for sip6 device */

    ,{CPSS_DXCH_SIP6_30_TABLE_SMU_IRF_SNG_E,
        ENTRY_TYPE_STREAM_NUM_E,0,0}
    ,{CPSS_DXCH_SIP6_30_TABLE_SMU_IRF_COUNTING_E,
        ENTRY_TYPE_STREAM_NUM_E,0,0}

    ,{CPSS_DXCH_SIP6_30_TABLE_PREQ_SRF_MAPPING_E,
        ENTRY_TYPE_STREAM_NUM_E,0,0}

    ,{CPSS_DXCH_SIP6_30_TABLE_PREQ_SRF_CONFIG_E,
        ENTRY_TYPE_STREAM_SRF_NUM_E,0,0}
    ,{CPSS_DXCH_SIP6_30_TABLE_PREQ_SRF_COUNTERS_E,
        ENTRY_TYPE_STREAM_SRF_NUM_E,0,0}
    ,{CPSS_DXCH_SIP6_30_TABLE_PREQ_SRF_DAEMON_E,
        ENTRY_TYPE_STREAM_SRF_NUM_E,0,0}

    ,{CPSS_DXCH_SIP6_30_TABLE_PREQ_SRF_HISTORY_BUFFER_E,
        ENTRY_TYPE_STREAM_SRF_HIST_NUM_E,0,0}
    ,{CPSS_DXCH_SIP6_30_TABLE_PREQ_SRF_ZERO_BIT_VECTOR_0_E,
        ENTRY_TYPE_STREAM_SRF_HIST_NUM_E,0,0}
    ,{CPSS_DXCH_SIP6_30_TABLE_PREQ_SRF_ZERO_BIT_VECTOR_1_E,
        ENTRY_TYPE_STREAM_SRF_HIST_NUM_E,0,0}


    /* must be last */
    ,{CPSS_DXCH_TABLE_LAST_E , ENTRY_TYPE___LAST__E , 0, 0}
};
/*  table information , that need to override Harrier info from harrierTablesInfo */
/* NOTE : the entries in this array need to be matched with entries in
   harrierTablesInfo , according to field */
static const PRV_CPSS_DXCH_TABLES_INFO_EXT_STC ironman_L_TablesInfo_overrideHarrier[] =
{
    /* there is bug ... it is not enough to set this info in ironman_L_directAccessTableInfo_overrideHarrier[] */
    {CPSS_DXCH_TABLE_FDB_E,
        OVERRIDE_INFO_NUM_ENTRIES_MAC(_32K)
    },
    /*L2i*/
    {CPSS_DXCH_SIP5_TABLE_INGRESS_BRIDGE_PORT_MEMBERS_E,
        OVERRIDE_INFO_WIDTH_MAC(BMP_64_PORTS_CNS)
    },
    {CPSS_DXCH_LION_TABLE_STG_INGRESS_E,
        OVERRIDE_INFO_WIDTH_MAC(2*BMP_64_PORTS_CNS)
    },

    /*SHT*/
    {CPSS_DXCH_LION_TABLE_VLAN_EGRESS_E,
        OVERRIDE_INFO_WIDTH_MAC(BMP_64_PORTS_CNS)
    },
    {CPSS_DXCH_TABLE_MULTICAST_E,
        OVERRIDE_INFO_WIDTH_MAC(BMP_64_PORTS_CNS)
    },
    {CPSS_DXCH_LION_TABLE_TXQ_NON_TRUNK_MEMBERS_E,
        OVERRIDE_INFO_WIDTH_MAC(BMP_64_PORTS_CNS)
    },
    {CPSS_DXCH_LION_TABLE_TXQ_DESIGNATED_PORT_E,
        OVERRIDE_INFO_WIDTH_MAC(BMP_64_PORTS_CNS)
    },
    {CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_NON_TRUNK_MEMBERS2_E,
        OVERRIDE_INFO_WIDTH_MAC(BMP_64_PORTS_CNS)
    },
    {CPSS_DXCH_LION_TABLE_PORT_ISOLATION_L2_E,
        OVERRIDE_INFO_WIDTH_MAC(BMP_64_PORTS_CNS)
    },
    {CPSS_DXCH_LION_TABLE_PORT_ISOLATION_L3_E,
        OVERRIDE_INFO_WIDTH_MAC(BMP_64_PORTS_CNS)
    },
    {CPSS_DXCH_LION_TABLE_TXQ_SOURCE_ID_MEMBERS_E,
        OVERRIDE_INFO_WIDTH_MAC(BMP_64_PORTS_CNS)
    },
    {CPSS_DXCH_LION_TABLE_STG_EGRESS_E,
        OVERRIDE_INFO_WIDTH_MAC(BMP_64_PORTS_CNS)
    },
    {CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E,
        OVERRIDE_INFO_WIDTH_MAC(SIP6_30_TTI_DEFAULT_PORT_BITS_SIZE_CNS)
    },

    {CPSS_DXCH_SIP5_TABLE_IPCL0_UDB_SELECT_E,
        OVERRIDE_INFO_WIDTH_MAC(SIP6_30_IPCL_UDB_SELECT_BITS_SIZE_CNS)
    },

    { CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E,
        OVERRIDE_INFO_WIDTH_MAC(SIP6_30_HA_EGRESS_EPORT_1_BITS_SIZE_CNS)
    },
    { CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_1_E,
        OVERRIDE_INFO_WIDTH_MAC(SIP6_30_HA_EGRESS_PHYSICAL_PORT_1_BITS_SIZE_CNS)
    },
    { CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E,
        OVERRIDE_INFO_WIDTH_MAC(SIP6_30_HA_EGRESS_PHYSICAL_PORT_2_BITS_SIZE_CNS)
    },

    { CPSS_DXCH_SIP6_TABLE_PREQ_TARGET_PHYSICAL_PORT_E,
        OVERRIDE_INFO_WIDTH_MAC(12)/*was 6*/
    },

};
/* ironman_L direct table information , that need to override Harrier info from ironman_L_directAccessTableInfo */
static const PRV_CPSS_DXCH_TABLES_INFO_DIRECT_EXT_STC ironman_L_directAccessTableInfo_overrideHarrier[] =
{
   /* Ironman not have first IPCL , so need to define only 2 IPCL tables */
    {CPSS_DXCH_TABLE_PCL_CONFIG_E,
     {0x00020000,BITS_TO_BYTES_ALIGNMENT_MAC(SIP6_10_PCL_PCL_CONFIG_BITS_SIZE_CNS),PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},
    {CPSS_DXCH_XCAT2_TABLE_INGRESS_PCL_LOOKUP01_CONFIG_E,
     {0x00030000,BITS_TO_BYTES_ALIGNMENT_MAC(SIP6_10_PCL_PCL_CONFIG_BITS_SIZE_CNS),PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

    {CPSS_DXCH_SIP6_10_TABLE_IPCL0_SOURCE_PORT_CONFIG_E,
     {0x00004000,BITS_TO_BYTES_ALIGNMENT_MAC(24),PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},
    {CPSS_DXCH_SIP6_10_TABLE_IPCL1_SOURCE_PORT_CONFIG_E,
     {0x00005000,BITS_TO_BYTES_ALIGNMENT_MAC(24),PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

    {CPSS_DXCH_SIP5_TABLE_IPCL0_UDB_SELECT_E,
     {0x00044000,BITS_TO_BYTES_ALIGNMENT_MAC(SIP6_30_IPCL_UDB_SELECT_BITS_SIZE_CNS),PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

    {CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_E,
    {0x00097000, BITS_TO_BYTES_ALIGNMENT_MAC(19), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS}},

    IRONMAN_DP_OVERRIDE_TABLE_AND_ADDR(        CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_CFG_E,0x00002000/*addr*/,50/*bits*/)
   ,IRONMAN_DP_OVERRIDE_TABLE_AND_ADDR(        CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_CRDT_BLNC_E,0x00009000/*addr*/,35/*bits*/)
   ,IRONMAN_DP_OVERRIDE_TABLE_AND_ADDR(        CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_ELIG_STATE_E,0x0000b000/*addr*/,11/*bits*/)
   ,IRONMAN_DP_OVERRIDE_TABLE_AND_ADDR(        CPSS_DXCH_SIP6_TXQ_SDQ_PORT_CRDT_BLNC_E,0x00008200/*addr*/,37/*bits*/)
   ,IRONMAN_DP_OVERRIDE_TABLE_AND_ADDR(        CPSS_DXCH_SIP6_TXQ_SDQ_SEL_LIST_PTRS_E,0x00008600/*addr*/,20/*bits*/)
   ,IRONMAN_DP_OVERRIDE_TABLE_AND_ADDR(        CPSS_DXCH_SIP6_15_TXQ_SDQ_PORT_PFC_STATE_E,0x00008400/*addr*/,16/*bits*/)


   ,PER_DP_OVERRIDE_TABLE_AND_ADDR(        CPSS_DXCH_SIP6_TXQ_PFCC_PFCC_CFG_E,0x00001000/*addr*/,17/*bits*/,0/*dpNum*/)

    /* NOTE: the alignment when stated NOT_MODIFIED_VALUE_CNS :
        is taken auto calculated from ironman_L_TablesInfo_overrideHarrier[] */
    ,{CPSS_DXCH_TABLE_LAST_E, {0,0,0}}
};

/* ironman_L direct table information , that are new (not exists in Harrier) */
/* register new tables that not exists in Harrier !!!! */
static const BOBK_TABLES_INFO_DIRECT_NEW_INDEX_STC ironman_L_directAccessTableInfo_new[] =
{
  {PRV_DXCH_SIP6_30_TABLE_IPCL0_UDB_REPLACEMENT_E,
         {CPSS_DXCH_SIP6_30_TABLE_IPCL0_UDB_REPLACEMENT_E, PRV_CPSS_DXCH_UNIT_PCL_E,
         {0x00052000/*addr*/, BITS_TO_BYTES_ALIGNMENT_MAC(SIP6_30_IPCL_UDB_REPLACEMENT_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE}},

  {PRV_DXCH_SIP6_30_TABLE_IPCL1_UDB_REPLACEMENT_E,
         {CPSS_DXCH_SIP6_30_TABLE_IPCL1_UDB_REPLACEMENT_E, PRV_CPSS_DXCH_UNIT_PCL_E,
         {0x00054000/*addr*/, BITS_TO_BYTES_ALIGNMENT_MAC(SIP6_30_IPCL_UDB_REPLACEMENT_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE}},

  {PRV_DXCH_SIP6_30_TABLE_SMU_IRF_SNG_E,
         {CPSS_DXCH_SIP6_30_TABLE_SMU_IRF_SNG_E , PRV_CPSS_DXCH_UNIT_SMU_E,
         {0x00004000/*addr*/, BITS_TO_BYTES_ALIGNMENT_MAC(SIP6_30_SMU_SNG_IRF_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE}},

  {PRV_DXCH_SIP6_30_TABLE_SMU_IRF_COUNTING_E,
         {CPSS_DXCH_SIP6_30_TABLE_SMU_IRF_COUNTING_E , PRV_CPSS_DXCH_UNIT_SMU_E,
         {0x00020000/*addr*/, BITS_TO_BYTES_ALIGNMENT_MAC(SIP6_30_SMU_IRF_COUNTERS_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE}},

  {PRV_DXCH_SIP6_30_TABLE_PREQ_SRF_MAPPING_E,
         {CPSS_DXCH_SIP6_30_TABLE_PREQ_SRF_MAPPING_E , PRV_CPSS_DXCH_UNIT_PREQ_E,
         {0x00070000/*addr*/, BITS_TO_BYTES_ALIGNMENT_MAC(SIP6_30_PREQ_SRF_MAPPING_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE}},


  {PRV_DXCH_SIP6_30_TABLE_PREQ_SRF_CONFIG_E,
         {CPSS_DXCH_SIP6_30_TABLE_PREQ_SRF_CONFIG_E , PRV_CPSS_DXCH_UNIT_PREQ_E,
         {0x00080000/*addr*/, BITS_TO_BYTES_ALIGNMENT_MAC(SIP6_30_PREQ_SRF_CONFIG_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE}},
  {PRV_DXCH_SIP6_30_TABLE_PREQ_SRF_COUNTERS_E,
         {CPSS_DXCH_SIP6_30_TABLE_PREQ_SRF_COUNTERS_E , PRV_CPSS_DXCH_UNIT_PREQ_E,
         {0x000B0000/*addr*/, BITS_TO_BYTES_ALIGNMENT_MAC(SIP6_30_PREQ_SRF_COUNTERS_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE}},
  {PRV_DXCH_SIP6_30_TABLE_PREQ_SRF_DAEMON_E,
         {CPSS_DXCH_SIP6_30_TABLE_PREQ_SRF_DAEMON_E , PRV_CPSS_DXCH_UNIT_PREQ_E,
         {0x00090000/*addr*/, BITS_TO_BYTES_ALIGNMENT_MAC(SIP6_30_PREQ_SRF_DAEMON_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE}},
  {PRV_DXCH_SIP6_30_TABLE_PREQ_SRF_HISTORY_BUFFER_E,
         {CPSS_DXCH_SIP6_30_TABLE_PREQ_SRF_HISTORY_BUFFER_E , PRV_CPSS_DXCH_UNIT_PREQ_E,
         {0x000a0000/*addr*/, BITS_TO_BYTES_ALIGNMENT_MAC(SIP6_30_PREQ_SRF_HISTORY_BUFFER_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE}},
  {PRV_DXCH_SIP6_30_TABLE_PREQ_SRF_ZERO_BIT_VECTOR_0_E,
         {CPSS_DXCH_SIP6_30_TABLE_PREQ_SRF_ZERO_BIT_VECTOR_0_E , PRV_CPSS_DXCH_UNIT_PREQ_E,
         {0x000C0000/*addr*/, BITS_TO_BYTES_ALIGNMENT_MAC(SIP6_30_PREQ_SRF_ZERO_BIT_VECTOR_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE}},
  {PRV_DXCH_SIP6_30_TABLE_PREQ_SRF_ZERO_BIT_VECTOR_1_E,
         {CPSS_DXCH_SIP6_30_TABLE_PREQ_SRF_ZERO_BIT_VECTOR_1_E , PRV_CPSS_DXCH_UNIT_PREQ_E,
         {0x000C1000/*addr*/, BITS_TO_BYTES_ALIGNMENT_MAC(SIP6_30_PREQ_SRF_ZERO_BIT_VECTOR_BITS_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE}},

  {PRV_DXCH_SIP6_30_TABLE_TXQ_PDS_NEXT_DESC,
         {CPSS_DXCH_SIP6_30_TABLE_TXQ_PDS_NEXT_DESC_E , PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS0_E,
         {0x00000000/*addr*/, 74/*bits*/, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE}},


  {PRV_DXCH_SIP6_30_TABLE_TXQ_PDS_WRITE_POINTER,
         {CPSS_DXCH_SIP6_30_TABLE_TXQ_PDS_WRITE_POINTER_E , PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS0_E,
         {0x00062000/*addr*/, 15/*bits*/, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE}},


  {PRV_DXCH_SIP6_30_TABLE_TXQ_PDS_READ_POINTER,
         {CPSS_DXCH_SIP6_30_TABLE_TXQ_PDS_READ_POINTER_E , PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS0_E,
         {0x00063000/*addr*/, 15/*bits*/, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE}},



  {PRV_DXCH_SIP6_30_TABLE_TXQ_SDQ_QBV_CFG,
         {CPSS_DXCH_SIP6_30_TXQ_SDQ_QBV_CFG_E , PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ0_E,
         {0x00010000/*addr*/, 50/*bits*/, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE}},


    /* NOTE: addresses in this table MUST be '0' based (relative to unit) and
       not global address of the device*/
    {PRV_DXCH_DEVICE_TABLE___LAST_E , {0, 0, {0, 0, 0}, GT_FALSE}}
};

/*xxx*/
/* bobcat3 table - in runtime copy table from this of bobk ... and continue from there ... */
static const GT_U32 bobcat3TableInfoSize = CPSS_DXCH_SIP5_20_TABLE_LAST_E;

/* bobcat3 table directAccessBobcat3TableInfo size */
static const GT_U32 directAccessBobcat3TableInfoSize = PRV_DXCH_BOBCAT3_TABLE___LAST_E;

#define MULTI_INSTANCE_TABLE_ENTRY_MAC(                             \
        localTableType/*PRV_DXCH_BOBCAT2_TABLES_ENT*/,              \
        globalTableType/*CPSS_DXCH_TABLE_ENT*/,                 \
        baseUnitIndex /*PRV_CPSS_DXCH_UNIT_ENT*/,                   \
        instanceId ,                                                \
        table_addr,                                                 \
        bytesAlignment)                                             \
     {localTableType + instanceId,                                  \
        {globalTableType + instanceId, baseUnitIndex + instanceId,  \
        {table_addr, bytesAlignment, NOT_MODIFIED_VALUE_CNS},GT_FALSE}\
    }

#define DP_MULTI_INSTANCE_TABLE_ENTRY_MAC(                          \
        localTableType/*PRV_DXCH_BOBCAT2_TABLES_ENT*/,              \
        globalTableType/*CPSS_DXCH_TABLE_ENT*/,                 \
        baseUnitIndex /*PRV_CPSS_DXCH_UNIT_ENT*/,                   \
        table_addr,                                                 \
        bytesAlignment)                                             \
     MULTI_INSTANCE_TABLE_ENTRY_MAC(localTableType,globalTableType,baseUnitIndex,0,table_addr,bytesAlignment) \
    ,MULTI_INSTANCE_TABLE_ENTRY_MAC(localTableType,globalTableType,baseUnitIndex,1,table_addr,bytesAlignment) \
    ,MULTI_INSTANCE_TABLE_ENTRY_MAC(localTableType,globalTableType,baseUnitIndex,2,table_addr,bytesAlignment) \
    ,MULTI_INSTANCE_TABLE_ENTRY_MAC(localTableType,globalTableType,baseUnitIndex,3,table_addr,bytesAlignment) \
    ,MULTI_INSTANCE_TABLE_ENTRY_MAC(localTableType,globalTableType,baseUnitIndex,4,table_addr,bytesAlignment)



/* Bobcat3 direct table information , that are new (not exists in bobk) */
/* register new tables that not exists in bobk !!!! */
static const BOBK_TABLES_INFO_DIRECT_NEW_INDEX_STC directAccessBobcat3TableInfo_new[] =
{
    /* NOTE: addresses in this table MUST be '0' based (relative to unit) and
       not global address of the device*/

    /* NOTE: the alignment is taken auto calculated from bobcat3TablesInfo_overrideBobk[] */

    {PRV_CPSS_BOBCAT3_TABLE_EGF_QAG_PORT_TARGET_ATTRIBUTES_E,
        {CPSS_DXCH_SIP5_20_TABLE_EGF_QAG_PORT_TARGET_ATTRIBUTES_E, PRV_CPSS_DXCH_UNIT_EGF_QAG_E,
        {0x00921000, NOT_MODIFIED_VALUE_CNS, NOT_MODIFIED_VALUE_CNS},GT_FALSE}
    }

    ,{PRV_CPSS_BOBCAT3_TABLE_TTI_PHYSICAL_PORT_ATTRIBUTE_2_E,
        {CPSS_DXCH_SIP5_20_TABLE_TTI_PHYSICAL_PORT_ATTRIBUTE_2_E, PRV_CPSS_DXCH_UNIT_TTI_E,
        {0x00160000, NOT_MODIFIED_VALUE_CNS, NOT_MODIFIED_VALUE_CNS},GT_FALSE}
    }

    ,{PRV_CPSS_BOBCAT3_TABLE_TTI_QCN_TO_TIMER_PAUSE_MAP_E,
        {CPSS_DXCH_SIP5_20_TABLE_TTI_QCN_TO_TIMER_PAUSE_MAP_E, PRV_CPSS_DXCH_UNIT_TTI_E,
        {0x00300000, NOT_MODIFIED_VALUE_CNS, NOT_MODIFIED_VALUE_CNS},GT_FALSE}
    }

    ,{PRV_CPSS_BOBCAT3_TABLE_EQ_TX_PROTECTION_LOC_E,
        {CPSS_DXCH_SIP5_20_TABLE_EQ_TX_PROTECTION_LOC_E, PRV_CPSS_DXCH_UNIT_EQ_E,
        {0x00b10000, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE}
    }

    ,{PRV_CPSS_BOBCAT3_TABLE_EGF_QAG_TC_DP_MAPPER_E,
        {CPSS_DXCH_SIP5_20_TABLE_EGF_QAG_TC_DP_MAPPER_E, PRV_CPSS_DXCH_UNIT_EGF_QAG_E,
        {0x00900000, NOT_MODIFIED_VALUE_CNS, NOT_MODIFIED_VALUE_CNS},GT_FALSE}
    }

    ,{PRV_CPSS_BOBCAT3_TABLE_EGF_QAG_PORT_SOURCE_ATTRIBUTES_E,
        {CPSS_DXCH_SIP5_20_TABLE_EGF_QAG_PORT_SOURCE_ATTRIBUTES_E, PRV_CPSS_DXCH_UNIT_EGF_QAG_E,
        {0x00923000, NOT_MODIFIED_VALUE_CNS, NOT_MODIFIED_VALUE_CNS},GT_FALSE}
    }

    /* generate extra 5 tables of CPSS_DXCH_LION_TABLE_TXQ_EGRESS_STC_E
       to support the DQ[1] .. DQ[5] */
    ,DP_MULTI_INSTANCE_TABLE_ENTRY_MAC(
        PRV_CPSS_BOBCAT3_MULTI_INSTANCE_TABLE_TXQ_EGRESS_STC__DQ_1_E,
        CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TXQ_EGRESS_STC__DQ_1_E,
        PRV_CPSS_DXCH_UNIT_TXQ_DQ1_E ,
        0x0000D800 , BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_EGRESS_STC_TABLE_BITS_SIZE_CNS))

    /* generate extra 5 tables of CPSS_DXCH_LION_TABLE_TXQ_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG_E
       to support the DQ[1] .. DQ[5] */
    ,DP_MULTI_INSTANCE_TABLE_ENTRY_MAC(
        PRV_CPSS_BOBCAT3_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG__DQ_1_E,
        CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG__DQ_1_E,
        PRV_CPSS_DXCH_UNIT_TXQ_DQ1_E ,
        0x00006000 , BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_PRIO_TOKEN_BUCKET_CONFIGURATION_BITS_SIZE_CNS))

    /* generate extra 5 tables of CPSS_DXCH_LION_TABLE_TXQ_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG_E
       to support the DQ[1] .. DQ[5] */
    ,DP_MULTI_INSTANCE_TABLE_ENTRY_MAC(
        PRV_CPSS_BOBCAT3_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG__DQ_1_E,
        CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG__DQ_1_E,
        PRV_CPSS_DXCH_UNIT_TXQ_DQ1_E ,
        0x0000A000 , BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_PORT_TOKEN_BUCKET_CONFIGURATION_BITS_SIZE_CNS))

    ,{PRV_CPSS_BOBCAT3_MULTI_INSTANCE_TABLE_TTI_PORT_MAPPING__DQ_0_E,
        {CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TTI_PORT_MAPPING__DQ_0_E, PRV_CPSS_DXCH_UNIT_TXQ_DQ_E,
         {0x00011000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_20_TTI_PORT_MAPPING_BITS_SIZE_CNS), NOT_MODIFIED_VALUE_CNS},GT_FALSE}
    }
    /* generate extra 5 tables  (DQ[1]..DQ[5] for  PRV_CPSS_BOBCAT3_MULTI_INSTANCE_TABLE_TTI_PORT_MAPPING__DQ_0_E */
    ,DP_MULTI_INSTANCE_TABLE_ENTRY_MAC(
        PRV_CPSS_BOBCAT3_MULTI_INSTANCE_TABLE_TTI_PORT_MAPPING__DQ_1_E,
        CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TTI_PORT_MAPPING__DQ_1_E,
        PRV_CPSS_DXCH_UNIT_TXQ_DQ1_E ,
        0x00011000 , BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_20_TTI_PORT_MAPPING_BITS_SIZE_CNS))

    /* extra tables of CPSS_DXCH_SIP5_TABLE_EQ_INGRESS_STC_PHYSICAL_PORT_E
       to support the pipe 1 */
    ,{PRV_CPSS_BOBCAT3_MULTI_INSTANCE_TABLE_EQ_INGRESS_STC_PHYSICAL_PORT_PIPE_1_E,
        {CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_EQ_INGRESS_STC_PHYSICAL_PORT_PIPE_1_E, PRV_CPSS_DXCH_UNIT_EQ_1_E,
        {0x00040000, BITS_TO_BYTES_ALIGNMENT_MAC(SIP5_EQ_INGRESS_STC_PHYSICAL_PORT_SIZE_CNS), PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE}
    }

    /* LPM memory for pipe 1 (applicable only in half memory mode) */
    ,{PRV_CPSS_BOBCAT3_LPM_MEM_PIPE_1_E,
        {CPSS_DXCH_SIP5_20_TABLE_LPM_MEM_PIPE_1_E, PRV_CPSS_DXCH_UNIT_LPM_1_E,
        {0x00000000, 0x4, PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS},GT_FALSE}
    }
};

/* Bobcat3 direct table information , that need to override bobk info from directAccessBobcat3TableInfo */
static const PRV_CPSS_DXCH_TABLES_INFO_DIRECT_EXT_STC directAccessBobcat3TableInfo_overrideBobk[] =
{
    /* NOTE: addresses in this table MUST be '0' based (relative to unit) and
       not global address of the device*/

    /* NOTE: the alignment when stated NOT_MODIFIED_VALUE_CNS :
        is taken auto calculated from bobcat3TablesInfo_overrideBobk[] */

    {CPSS_DXCH_SIP5_TABLE_HA_GLOBAL_MAC_SA_E,
    {0x00005000, NOT_MODIFIED_VALUE_CNS, NOT_MODIFIED_VALUE_CNS}},

    {CPSS_DXCH_LION_TABLE_TXQ_NON_TRUNK_MEMBERS_E,
    {0x02400000, NOT_MODIFIED_VALUE_CNS, NOT_MODIFIED_VALUE_CNS}},

    {CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_CPU_CODE_TO_LOOPBACK_MAPPER_E,
    {0x00920000, NOT_MODIFIED_VALUE_CNS, NOT_MODIFIED_VALUE_CNS}},

    {CPSS_DXCH_SIP5_TABLE_BMA_PORT_MAPPING_E,
    {0x0005A000, NOT_MODIFIED_VALUE_CNS, NOT_MODIFIED_VALUE_CNS}},

    /* NOTE: this table in sip5_20 is called "Port Enq Attributes Table" (Port_Enq_Attributes_Table) */
    {CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_E,
    {0x00922000, NOT_MODIFIED_VALUE_CNS, NOT_MODIFIED_VALUE_CNS}},

    {CPSS_DXCH_SIP5_TABLE_TAIL_DROP_COUNTERS_Q_MAIN_BUFF_E,
    {0x000d0000, NOT_MODIFIED_VALUE_CNS, NOT_MODIFIED_VALUE_CNS}},

    {CPSS_DXCH_SIP5_TABLE_CN_SAMPLE_INTERVALS_E,
    {0x00010000, NOT_MODIFIED_VALUE_CNS, NOT_MODIFIED_VALUE_CNS}},

    {CPSS_DXCH_SIP5_TABLE_ADJACENCY_E,
     {NOT_MODIFIED_VALUE_CNS, 0xC /*changed from 0x10*/ ,  NOT_MODIFIED_VALUE_CNS}},

    {CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_COUNTING_E,
    {0x00200000, NOT_MODIFIED_VALUE_CNS, NOT_MODIFIED_VALUE_CNS}}
};

/*  table information , that need to override Bobk info from bobcat3TablesInfo */
/* NOTE : the entries in this array need to be matched with entries in
        bobkTablesInfo , according to field */
static const PRV_CPSS_DXCH_TABLES_INFO_EXT_STC bobcat3TablesInfo_overrideBobk[] =
{
    /*SHT*/
    {CPSS_DXCH_LION_TABLE_VLAN_EGRESS_E,
        OVERRIDE_INFO_WIDTH_MAC(BMP_256_PORTS_CNS)
    },
    {CPSS_DXCH_TABLE_MULTICAST_E,
        OVERRIDE_INFO_WIDTH_MAC(BMP_512_PORTS_CNS)
    },
    {CPSS_DXCH_LION_TABLE_TXQ_NON_TRUNK_MEMBERS_E,
        OVERRIDE_INFO_WIDTH_MAC(BMP_512_PORTS_CNS)
    },
    {CPSS_DXCH_LION_TABLE_TXQ_DESIGNATED_PORT_E,
        OVERRIDE_INFO_WIDTH_MAC(BMP_512_PORTS_CNS)
    },
    {CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_EPORT_FILTER_E,
        OVERRIDE_INFO_NUM_ENTRIES_AND_WIDTH_MAC(_16K,64)
    },
    {CPSS_DXCH_SIP5_TABLE_INGRESS_BRIDGE_PORT_MEMBERS_E,
        OVERRIDE_INFO_WIDTH_MAC(BMP_512_PORTS_CNS)
    },
    {CPSS_DXCH_LION_TABLE_STG_INGRESS_E,
        OVERRIDE_INFO_WIDTH_MAC(2*BMP_512_PORTS_CNS)
    },
    {CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_EGRESS_VLAN_E,
        OVERRIDE_INFO_WIDTH_MAC(SIP5_20_EGF_QAG_EGRESS_EVLAN_BITS_SIZE_CNS)
    },
    {CPSS_DXCH_XCAT_TABLE_INGRESS_PCL_UDB_CONFIG_E,
        OVERRIDE_INFO_WIDTH_MAC(SIP5_20_TTI_PCL_UDB_BITS_SIZE_CNS)
    },
    {CPSS_DXCH_SIP5_TABLE_IPCL0_UDB_SELECT_E,
        OVERRIDE_INFO_WIDTH_MAC(SIP5_20_IPCL_UDB_SELECT_BITS_SIZE_CNS)
    },
    {CPSS_DXCH_SIP5_TABLE_IPCL1_UDB_SELECT_E,
        OVERRIDE_INFO_WIDTH_MAC(SIP5_20_IPCL_UDB_SELECT_BITS_SIZE_CNS)
    },
    {CPSS_DXCH_SIP5_TABLE_IPCL2_UDB_SELECT_E,
        OVERRIDE_INFO_WIDTH_MAC(SIP5_20_IPCL_UDB_SELECT_BITS_SIZE_CNS)
    },
    {CPSS_DXCH_SIP5_20_TABLE_EGF_QAG_PORT_TARGET_ATTRIBUTES_E,
        OVERRIDE_INFO_WIDTH_MAC(SIP5_20_EGF_QAG_PORT_TARGET_BITS_SIZE_CNS)
    },
    {CPSS_DXCH_SIP5_20_TABLE_TTI_PHYSICAL_PORT_ATTRIBUTE_2_E,
        OVERRIDE_INFO_WIDTH_MAC(SIP5_20_TTI_PHYSICAL_PORT_2_BITS_SIZE_CNS)
    },
    {CPSS_DXCH_SIP5_TABLE_IPVX_ROUTER_NEXTHOP_E,
        OVERRIDE_INFO_WIDTH_MAC(SIP5_20_IPVX_ROUTER_NEXTHOP_BITS_SIZE_CNS)
    },
    {CPSS_DXCH_SIP5_TABLE_IP_MLL_E,
        OVERRIDE_INFO_WIDTH_MAC(SIP5_20_MLL_ENTRY_BITS_SIZE_CNS)
    },
    {CPSS_DXCH_SIP5_TABLE_L2_MLL_E,
        OVERRIDE_INFO_WIDTH_MAC(SIP5_20_MLL_ENTRY_BITS_SIZE_CNS)
    },
    {CPSS_DXCH_SIP5_TABLE_CN_SAMPLE_INTERVALS_E,
        OVERRIDE_INFO_NUM_ENTRIES_AND_WIDTH_MAC(4608/*maxNumOfEntries*/,40/*entrySize_inBits*/)
    },

    {CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TTI_PORT_MAPPING__DQ_0_E,
        OVERRIDE_INFO_NUM_ENTRIES_AND_WIDTH_MAC(128/*maxNumOfEntries*/,
                                                SIP5_20_TTI_PORT_MAPPING_BITS_SIZE_CNS)
    },
    {CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TTI_PORT_MAPPING__DQ_1_E,
        OVERRIDE_INFO_NUM_ENTRIES_AND_WIDTH_MAC(128/*maxNumOfEntries*/,
                                                SIP5_20_TTI_PORT_MAPPING_BITS_SIZE_CNS)
    },
    {CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TTI_PORT_MAPPING__DQ_2_E,
        OVERRIDE_INFO_NUM_ENTRIES_AND_WIDTH_MAC(128/*maxNumOfEntries*/,
                                                SIP5_20_TTI_PORT_MAPPING_BITS_SIZE_CNS)
    },
    {CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TTI_PORT_MAPPING__DQ_3_E,
        OVERRIDE_INFO_NUM_ENTRIES_AND_WIDTH_MAC(128/*maxNumOfEntries*/,
                                                SIP5_20_TTI_PORT_MAPPING_BITS_SIZE_CNS)
    },
    {CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TTI_PORT_MAPPING__DQ_4_E,
        OVERRIDE_INFO_NUM_ENTRIES_AND_WIDTH_MAC(128/*maxNumOfEntries*/,
                                                SIP5_20_TTI_PORT_MAPPING_BITS_SIZE_CNS)
    },
    {CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TTI_PORT_MAPPING__DQ_5_E,
        OVERRIDE_INFO_NUM_ENTRIES_AND_WIDTH_MAC(128/*maxNumOfEntries*/,
                                                SIP5_20_TTI_PORT_MAPPING_BITS_SIZE_CNS)
    },
    {CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_EQ_INGRESS_STC_PHYSICAL_PORT_PIPE_1_E,
        OVERRIDE_INFO_WIDTH_MAC(SIP5_EGRESS_STC_TABLE_BITS_SIZE_CNS)
    },
    {CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TXQ_EGRESS_STC__DQ_1_E,
        OVERRIDE_INFO_WIDTH_MAC(SIP5_EGRESS_STC_TABLE_BITS_SIZE_CNS)
    },
    {CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TXQ_EGRESS_STC__DQ_2_E,
        OVERRIDE_INFO_WIDTH_MAC(SIP5_EGRESS_STC_TABLE_BITS_SIZE_CNS)
    },
    {CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TXQ_EGRESS_STC__DQ_3_E,
        OVERRIDE_INFO_WIDTH_MAC(SIP5_EGRESS_STC_TABLE_BITS_SIZE_CNS)
    },
    {CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TXQ_EGRESS_STC__DQ_4_E,
        OVERRIDE_INFO_WIDTH_MAC(SIP5_EGRESS_STC_TABLE_BITS_SIZE_CNS)
    },
    {CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TXQ_EGRESS_STC__DQ_5_E,
        OVERRIDE_INFO_WIDTH_MAC(SIP5_EGRESS_STC_TABLE_BITS_SIZE_CNS)
    },


};


static GT_STATUS lion2InitTable(void);
static GT_STATUS bobcat2InitTable(GT_U8 devNum);
static GT_STATUS bobkInitTable(GT_U8 devNum);
static GT_STATUS bobcat3InitTable(GT_U8 devNum);
static GT_STATUS aldrin2InitTable(GT_U8 devNum);
static GT_STATUS falconInitTable(GT_U8 devNum);
static GT_STATUS hawkInitTable(GT_U8 devNum);
static GT_STATUS phoenixInitTable(GT_U8 devNum);
static GT_STATUS harrierInitTable(GT_U8 devNum);
static GT_STATUS ironmanInitTable(GT_U8 devNum);

extern void  copyBits(
    IN GT_U32                  *targetMemPtr,
    IN GT_U32                  targetStartBit,
    IN GT_U32                  *sourceMemPtr,
    IN GT_U32                  sourceStartBit,
    IN GT_U32                  numBits
);

extern void copyBitsMasked
(
    INOUT GT_U32   *targetMemPtr,
    IN    GT_U32   targetStartBit,
    IN    GT_U32   *sourceMemPtr,
    IN    GT_U32   sourceStartBit,
    IN    GT_U32   numBits,
    IN    GT_U32   *maskPtr
);

static GT_STATUS directReadTableHwEntries
(
    IN GT_U8                          devNum,
    IN GT_U32                         portGroupId,
    IN PRV_CPSS_DXCH_TABLES_INFO_STC *tableInfoPtr,
    IN GT_U32                         entryIndex,
    IN GT_U32                         entriesNum,
    OUT GT_U32                       *entryValuePtr
);

/**
* @internal directAccessGlobalIndexToLocalIndexConvert function
* @endinternal
*
* @brief   direct tables info :
*         find global index in the array and return the 'Local index' of the
*         matched index.
* @param[in] globalIndex              - global Index (need to match infoDirectUnitPtr[localIndex].globalIndex)
* @param[in] infoDirectUnitPtr        - (pointer to) direct table info
* @param[in] numOfElements            - number of elements under infoDirectUnitPtr
*
* @param[out] localIndexPtr            - (pointer to) the local index that matched
* @param[out] infoDirectUnitPtr        == globalIndex
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_FOUND             - when globalIndex was not matched
*/
static GT_STATUS directAccessGlobalIndexToLocalIndexConvert(
    IN CPSS_DXCH_TABLE_ENT                      globalIndex,
    IN const PRV_CPSS_DXCH_TABLES_INFO_DIRECT_UNIT_STC    *infoDirectUnitPtr,
    IN GT_U32                                       numOfElements,
    OUT GT_U32                                      *localIndexPtr
)
{
    GT_U32      ii;

    *localIndexPtr = 0;

    for(ii = 0 ; ii < numOfElements; ii++)
    {
        if(infoDirectUnitPtr[ii].globalIndex == globalIndex)
        {
            /* match found */
            *localIndexPtr = ii;
            return GT_OK;
        }
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);

}

/**
* @internal checkStatusOfPreviousAccess function
* @endinternal
*
* @brief   Check status of previous access.
*
* @note   APPLICABLE DEVICES:       xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - the port group Id
* @param[in] tableInfoPtr             - pointer to table
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*/
static GT_STATUS checkStatusOfPreviousAccess
(
    IN GT_U8                                 devNum,
    IN GT_U32                                portGroupId,
    IN PRV_CPSS_DXCH_TABLES_INFO_INDIRECT_STC *tableInfoPtr
)
{
    return prvCpssPortGroupBusyWait(devNum,portGroupId,
                tableInfoPtr->controlReg,
                tableInfoPtr->trigBit,
                GT_FALSE);
}


/**
* @internal setReadWriteCommandToControlReg function
* @endinternal
*
* @brief   Set Read / Write command to Control register.
*
* @note   APPLICABLE DEVICES:       xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - the port group Id
* @param[in] tableInfoPtr             - pointer to table
* @param[in] entryIndex               - index in the table
* @param[in] accessAction             - access action to table: PRV_CPSS_DXCH_ACCESS_ACTION_READ_E,
*                                      PRV_CPSS_DXCH_ACCESS_ACTION_WRITE_E
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
*/
static GT_STATUS setReadWriteCommandToControlReg
(
    IN GT_U8                                    devNum,
    IN GT_U32                                   portGroupId,
    IN PRV_CPSS_DXCH_TABLES_INFO_INDIRECT_STC   *tableInfoPtr,
    IN GT_U32                                   entryIndex,
    IN PRV_CPSS_DXCH_ACCESS_ACTION_ENT          accessAction
)
{
   GT_U32 controlValue;

   controlValue  = (entryIndex << tableInfoPtr->indexBit);

   /* trigger the action */
   controlValue |= (1 << tableInfoPtr->trigBit);

   /* add the specific table when this control register has more than one table
   if there is no specific table, specificTableValue = 0*/
   controlValue |= (tableInfoPtr->specificTableValue <<
                     tableInfoPtr->specificTableBit);

   /* set type of action - R/W */
   controlValue |= (accessAction << tableInfoPtr->actionBit);

   return prvCpssHwPpPortGroupWriteRegister(devNum,portGroupId, tableInfoPtr->controlReg, controlValue);


}

/*******************************************************************************
* tableFieldsFormatInfoGet
*
* DESCRIPTION:
*       function return the table format that relate to the specific table.
*       for not supported table --> return NULL
*
* INPUTS:
*       devNum          - device number
*       tableType       - the specific table name
*
* OUTPUTS:
*       None.
*
*
* RETURNS:
*       pointer to 'Table format info'
*
* COMMENTS:
*
*
*******************************************************************************/
static PRV_CPSS_DXCH_PP_TABLE_FORMAT_INFO_STC *tableFieldsFormatInfoGet
(
    IN GT_U8 devNum,
    IN CPSS_DXCH_TABLE_ENT tableType
)
{
    PRV_CPSS_DXCH_TABLE_FORMAT_ENT tableFormat;
    switch(tableType)
    {
        /* TTI */
        case CPSS_DXCH_SIP5_TABLE_TTI_PHYSICAL_PORT_ATTRIBUTE_E:
            tableFormat = PRV_CPSS_DXCH_TABLE_FORMAT_TTI_PHYSICAL_PORT_ATTRIBUTE_E;
            break;
        case CPSS_DXCH_SIP5_TABLE_PRE_TTI_LOOKUP_INGRESS_EPORT_E:
            tableFormat = PRV_CPSS_DXCH_TABLE_FORMAT_TTI_DEFAULT_EPORT_E;
            break;
        case CPSS_DXCH_SIP5_TABLE_POST_TTI_LOOKUP_INGRESS_EPORT_E:
            tableFormat = PRV_CPSS_DXCH_TABLE_FORMAT_TTI_EPORT_ATTRIBUTES_E;
            break;
        case CPSS_DXCH_TABLE_VLAN_PORT_PROTOCOL_E:
            tableFormat = PRV_CPSS_DXCH_TABLE_FORMAT_VLAN_PORT_PROTOCOL_E;
            break;

        /* L2I */
        case CPSS_DXCH_SIP5_TABLE_BRIDGE_INGRESS_EPORT_E:
            tableFormat = PRV_CPSS_DXCH_TABLE_FORMAT_BRIDGE_INGRESS_EPORT_E;
            break;
        case CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E:
            tableFormat = PRV_CPSS_DXCH_TABLE_FORMAT_BRIDGE_INGRESS_EVLAN_E;
            break;

        /* FDB */
        case CPSS_DXCH_TABLE_FDB_E:
            tableFormat = PRV_CPSS_DXCH_TABLE_FORMAT_FDB_E;
            break;

        /* MLL */
        case CPSS_DXCH_SIP5_TABLE_IP_MLL_E:
            tableFormat = PRV_CPSS_DXCH_TABLE_FORMAT_IP_MLL_E;
            break;
        case CPSS_DXCH_SIP5_TABLE_L2_MLL_E:
            tableFormat = PRV_CPSS_DXCH_TABLE_FORMAT_L2_MLL_E;
            break;

        /* EGF_QAG */
        case CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_EGRESS_EPORT_E:
            tableFormat = PRV_CPSS_DXCH_TABLE_FORMAT_EGF_QAG_EGRESS_EPORT_E;
            break;

        case CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_E:
            tableFormat = PRV_CPSS_DXCH_TABLE_FORMAT_EGF_QAG_TARGET_PORT_MAPPER_E;
            break;

        case CPSS_DXCH_SIP5_20_TABLE_EGF_QAG_PORT_TARGET_ATTRIBUTES_E:
            tableFormat = PRV_CPSS_DXCH_TABLE_FORMAT_EGF_QAG_PORT_TARGET_ATTRIBUTES_E;
            break;

        case CPSS_DXCH_SIP5_20_TABLE_EGF_QAG_PORT_SOURCE_ATTRIBUTES_E:
            tableFormat = PRV_CPSS_DXCH_TABLE_FORMAT_EGF_QAG_PORT_SOURCE_ATTRIBUTES_E;
            break;

        /* EGF_SHT */
        case CPSS_DXCH_LION_TABLE_VLAN_EGRESS_E:
            tableFormat = PRV_CPSS_DXCH_TABLE_FORMAT_EGF_SHT_EGRESS_EVLAN_E;
            break;
        case CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_VLAN_ATTRIBUTES_E:
            tableFormat = PRV_CPSS_DXCH_TABLE_FORMAT_EGF_SHT_VLAN_ATTRIBUTES_E;
            break;

        case CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_EGRESS_EPORT_E:
            tableFormat = PRV_CPSS_DXCH_TABLE_FORMAT_EGF_SHT_EGRESS_EPORT_E;
            break;

        /* HA */
        case CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_1_E:
            tableFormat = PRV_CPSS_DXCH_TABLE_FORMAT_HA_EGRESS_EPORT_1_E;
            break;
        case CPSS_DXCH_SIP5_TABLE_HA_EGRESS_EPORT_2_E:
            tableFormat = PRV_CPSS_DXCH_TABLE_FORMAT_HA_EGRESS_EPORT_2_E;
            break;
        case CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_1_E:
            tableFormat = PRV_CPSS_DXCH_TABLE_FORMAT_HA_PHYSICAL_PORT_1_E;
            break;
        case CPSS_DXCH_SIP5_TABLE_HA_PHYSICAL_PORT_2_E:
            tableFormat = PRV_CPSS_DXCH_TABLE_FORMAT_HA_PHYSICAL_PORT_2_E;
            break;
        case CPSS_DXCH2_TABLE_TUNNEL_START_CONFIG_E:
            tableFormat = PRV_CPSS_DXCH_TABLE_FORMAT_TUNNEL_START_E;
            break;
        case CPSS_DXCH_SIP5_TABLE_HA_GENERIC_TS_PROFILE_E:
            tableFormat = PRV_CPSS_DXCH_TABLE_FORMAT_HA_GENERIC_TS_PROFILE_E;
            break;
        /*case PRV_CPSS_SIP5_TABLE_NAT44_CONFIG_E:
            tableFormat = PRV_CPSS_DXCH_TABLE_FORMAT_NAT44_E;
            break;*/
        case CPSS_DXCH_SIP5_TABLE_IPVX_INGRESS_EPORT_E:
            tableFormat = PRV_CPSS_DXCH_TABLE_FORMAT_ROUTER_EPORT_E;
            break;
        /* PREQ */
        case CPSS_DXCH_SIP6_TABLE_PREQ_PORT_PROFILE_E:
            tableFormat = PRV_CPSS_DXCH_TABLE_FORMAT_PREQ_PORT_PROFILE_E;
            break;
        case CPSS_DXCH_SIP6_TABLE_PREQ_QUEUE_DP_CONFIGURATIONS_E:
            tableFormat = PRV_CPSS_DXCH_TABLE_FORMAT_PREQ_PROFILES_E;
            break;
        case CPSS_DXCH_SIP6_TABLE_PREQ_QUEUE_CONFIGURATIONS_E:
            tableFormat = PRV_CPSS_DXCH_TABLE_FORMAT_PREQ_QUEUE_CONFIGURATIONS_E;
            break;
        case CPSS_DXCH_SIP6_TABLE_PREQ_CNC_PORT_MAPPING_E:
            tableFormat = PRV_CPSS_DXCH_TABLE_FORMAT_PREQ_QUEUE_PORT_MAPPING_E;
            break;
        case CPSS_DXCH_SIP6_TABLE_PREQ_TARGET_PHYSICAL_PORT_E:
            tableFormat = PRV_CPSS_DXCH_TABLE_FORMAT_PREQ_TARGET_PHYSICAL_PORT_E;
            break;
        /* EXACT MATCH  */
        case CPSS_DXCH_SIP6_TABLE_EXACT_MATCH_E:
            tableFormat = PRV_CPSS_DXCH_TABLE_FORMAT_EXACT_MATCH_E;
            break;
        case CPSS_DXCH_SIP6_10_TABLE_EXACT_MATCH_AUTO_LEARNED_ENTRY_INDEX_E:
            tableFormat = PRV_CPSS_DXCH_TABLE_FORMAT_EXACT_MATCH_AUTO_LEARNED_ENTRY_INDEX_E;
            break;

        /* Sip 6.10 : PPU */
        case CPSS_DXCH_SIP6_10_TABLE_PPU_ACTION_TABLE_0_E:
        case CPSS_DXCH_SIP6_10_TABLE_PPU_ACTION_TABLE_1_E:
        case CPSS_DXCH_SIP6_10_TABLE_PPU_ACTION_TABLE_2_E:
            tableFormat = PRV_CPSS_DXCH_TABLE_FORMAT_PPU_ACTION_E;
            break;
        case CPSS_DXCH_SIP6_10_TABLE_PPU_DAU_PROFILE_TABLE_E:
            tableFormat = PRV_CPSS_DXCH_TABLE_FORMAT_PPU_DAU_PROFILE_E;
            break;

        case CPSS_DXCH_SIP6_30_TABLE_SMU_IRF_SNG_E        :
            tableFormat = PRV_CPSS_DXCH_TABLE_FORMAT_SMU_IRF_SNG_E;
            break;
        case CPSS_DXCH_SIP6_30_TABLE_SMU_IRF_COUNTING_E   :
            tableFormat = PRV_CPSS_DXCH_TABLE_FORMAT_SMU_IRF_COUNTERS_E;
            break;

        case CPSS_DXCH_SIP6_30_TABLE_PREQ_SRF_MAPPING_E   :
            tableFormat = PRV_CPSS_DXCH_TABLE_FORMAT_PREQ_SRF_MAPPING_E;
            break;
        case CPSS_DXCH_SIP6_30_TABLE_PREQ_SRF_CONFIG_E    :
            tableFormat = PRV_CPSS_DXCH_TABLE_FORMAT_PREQ_SRF_CONFIG_E;
            break;
        case CPSS_DXCH_SIP6_30_TABLE_PREQ_SRF_COUNTERS_E  :
            tableFormat = PRV_CPSS_DXCH_TABLE_FORMAT_PREQ_SRF_COUNTERS_E;
            break;
        case CPSS_DXCH_SIP6_30_TABLE_PREQ_SRF_DAEMON_E    :
            tableFormat = PRV_CPSS_DXCH_TABLE_FORMAT_PREQ_SRF_DAEMON_E;
            break;

        default:
            return NULL;
    }

    return &PRV_TABLE_FORMAT_INFO(devNum)[tableFormat];
}

/**
* @internal convertFieldInfoToGlobalBit function
* @endinternal
*
* @brief   function converts : fieldWordNum,fieldOffset,fieldLength
*         from value relate to PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS
* @param[in] devNum                   - device number
* @param[in] tableType                - the specific table name
* @param[in,out] fieldWordNumPtr          - field word number  PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS
* @param[in,out] fieldOffsetPtr           - field offset       field name
* @param[in,out] fieldLengthPtr           - field length        number of fields or bit offset in field
* @param[in] value                    -  is checked to not 'over flow' the number of bits.
*                                      this is to remove the need from the 'cpss APIs' to be aware to field size!
* @param[in,out] fieldWordNumPtr          - field word number PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS
* @param[in,out] fieldOffsetPtr           - field offset      global bit position
* @param[in,out] fieldLengthPtr           - field length       number of bits
*
* @retval GT_OK                    - success
* @retval GT_BAD_STATE             - fail
* @retval GT_OUT_OF_RANGE          - when value > (max valid value)
*/
static GT_STATUS    convertFieldInfoToGlobalBit(
    IN GT_U8        devNum,
    IN CPSS_DXCH_TABLE_ENT tableType,
    INOUT GT_U32   *fieldWordNumPtr,
    INOUT GT_U32   *fieldOffsetPtr,
    INOUT GT_U32   *fieldLengthPtr,
    IN    GT_U32    value
)
{
    const PRV_CPSS_DXCH_PP_TABLE_FORMAT_INFO_STC *tableFieldsFormatInfoPtr;
    const PRV_CPSS_ENTRY_FORMAT_TABLE_STC *tableFieldsFormatPtr;
    GT_U32   fieldId = *fieldOffsetPtr;
    GT_U32   totalLength;
    GT_U32   numOfFields = *fieldLengthPtr;
    GT_U32   tmpLen;
    GT_U32   tmpOffset;

    tableFieldsFormatInfoPtr = tableFieldsFormatInfoGet(devNum, tableType);
    if(tableFieldsFormatInfoPtr == NULL)
    {
        /* the table is not bound to fields format */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    tableFieldsFormatPtr = tableFieldsFormatInfoPtr->fieldsInfoPtr;

    if(tableFieldsFormatPtr == NULL)
    {
        /* the table is not bound to fields format */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    /* convert the value into 'global field offsets' */
    *fieldWordNumPtr = PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS;
    GET_FIELD_INFO_BY_ID_MAC(tableFieldsFormatPtr, fieldId, *fieldOffsetPtr,totalLength);

    if(numOfFields)
    {
        /* indication to use the 'Auto calc' or to use 'Length caller' ...
           this to support setting of consecutive' fields ! */
        while(--numOfFields)
        {
            fieldId ++;
            GET_FIELD_INFO_BY_ID_MAC(tableFieldsFormatPtr, fieldId, tmpOffset,tmpLen);
            totalLength += tmpLen;
        }
    }

    *fieldLengthPtr = totalLength;

    if(totalLength <= 32 &&
        (value > BIT_MASK_MAC(totalLength)))
    {
        /* value > (max valid value) */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal convertSubFieldInfoToGlobalBit function
* @endinternal
*
* @brief   the function retain parameters of 'sub-field' with in 'parent field'.
*         function converts : fieldWordNum,fieldOffset,fieldLength
*         from value relate to PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS
*         to value relate to PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS
* @param[in] devNum                   - device number
* @param[in] tableType                - the specific table name
* @param[in] subFieldOffset           - the bit index of the sub field (within the 'parent field')
* @param[in] subFieldLength           - the length of the sub field
* @param[in,out] fieldWordNumPtr          - field word number  PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS
* @param[in,out] fieldOffsetPtr           - field offset       field name
* @param[in,out] fieldLengthPtr           - not used
* @param[in] value                    -  is checked to not 'over flow' the number of bits.
*                                      this is to remove the need from the 'cpss APIs' to be aware to field size!
* @param[in,out] fieldWordNumPtr          - field word number PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS
* @param[in,out] fieldOffsetPtr           - field offset      global bit position
* @param[in,out] fieldLengthPtr           - field length       number of bits
*
* @retval GT_OK                    - success
* @retval GT_BAD_STATE             - fail
*/
static GT_STATUS    convertSubFieldInfoToGlobalBit(
    IN GT_U8       devNum,
    IN CPSS_DXCH_TABLE_ENT tableType,
    IN    GT_U32   subFieldOffset,
    IN    GT_U32   subFieldLength,
    INOUT GT_U32   *fieldWordNumPtr,
    INOUT GT_U32   *fieldOffsetPtr,
    INOUT GT_U32   *fieldLengthPtr,
    IN    GT_U32    value
)
{
    GT_STATUS    rc;


    /* rest the fieldLength , before calling convertFieldInfoToGlobalBit */
    *fieldLengthPtr = PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS;

    rc = convertFieldInfoToGlobalBit(devNum,tableType,fieldWordNumPtr,fieldOffsetPtr,fieldLengthPtr,value);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* (*fieldWordNumPtr) value is PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS */

    /* calc global bit index of the sub field */
    (*fieldOffsetPtr) += subFieldOffset;

    /* use the length as defined by the caller for the sub field */
    *fieldLengthPtr = subFieldLength;

    return GT_OK;

}

/**
* @internal convertPatternFieldInfoToGlobalBit function
* @endinternal
*
* @brief   function converts : fieldWordNum,fieldOffset,fieldLength
*         from value related to PRV_CPSS_DXCH_TABLE_WORD_INDICATE_PATTERN_FIELD_NAME_CNS
*         to value relate to PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS
* @param[in] devNum                   - device number
* @param[in] tableType                - the specific table name
* @param[in,out] entryIndexPtr            - the entry index
* @param[in,out] fieldWordNumPtr          - field word number  PRV_CPSS_DXCH_TABLE_WORD_INDICATE_PATTERN_FIELD_NAME_CNS
* @param[in,out] fieldOffsetPtr           - field offset       field name
* @param[in,out] fieldLengthPtr           - not used
* @param[in] val                      - value is checked to not 'overflow' the number of bits.
*                                      this is to remove the need from the 'cpss APIs' to be aware to field size!
* @param[in,out] entryIndexPtr            - the table entry index
* @param[in,out] fieldWordNumPtr          - field word number PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS
* @param[in,out] fieldOffsetPtr           - field offset      global bit position
* @param[in,out] fieldLengthPtr           - field length       number of bits
*
* @retval GT_OK                    - success
* @retval GT_BAD_STATE             - fail
*/
static GT_STATUS    convertPatternFieldInfoToGlobalBit(
    IN GT_U8       devNum,
    IN CPSS_DXCH_TABLE_ENT tableType,
    INOUT GT_U32   *entryIndexPtr,
    INOUT GT_U32   *fieldWordNumPtr,
    INOUT GT_U32   *fieldOffsetPtr,
    INOUT GT_U32   *fieldLengthPtr,
    IN    GT_U32    val
)
{
    GT_STATUS    rc;
    PRV_CPSS_DXCH_PP_TABLE_FORMAT_INFO_STC *  fieldsInfoPtr;
    GT_U32 patternIndex;

    fieldsInfoPtr = tableFieldsFormatInfoGet(devNum, tableType);
    if(fieldsInfoPtr == NULL)
    {
        /* the table is not bound to fields format */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    /* Convert entry index to pattern index */
    patternIndex = (*entryIndexPtr) % fieldsInfoPtr->patternNum;

    /* Claculate global field bit and length */
    rc = convertFieldInfoToGlobalBit(devNum, tableType, fieldWordNumPtr, fieldOffsetPtr, fieldLengthPtr, val);
    if(rc != GT_OK)
    {
        return rc;
    }


    /* Calculate global bit index pattern field */
    (*fieldOffsetPtr) += patternIndex * fieldsInfoPtr->patternBits;

    /* Calculate table entry index */
    (*entryIndexPtr) /= fieldsInfoPtr->patternNum;

    return GT_OK;

}

/**
* @internal indirectReadTableEntry function
* @endinternal
*
* @brief   Read a whole entry from table by indirect access method.
*
* @note   APPLICABLE DEVICES:       xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - the portGroupId
* @param[in] tableType                - the specific table name
* @param[in] tableInfoPtr             - pointer to table information
* @param[in] entryIndex               - index in the table
* @param[in] tablePtr                 - pointer to indirect information
*
* @param[out] entryValuePtr            - pointer to the data read from the table
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*/
static GT_STATUS indirectReadTableEntry
(
    IN GT_U8                                    devNum,
    IN GT_U32                                   portGroupId,
    IN CPSS_DXCH_TABLE_ENT                  tableType,
    IN PRV_CPSS_DXCH_TABLES_INFO_STC            *tableInfoPtr,
    IN GT_U32                                   entryIndex,
    IN PRV_CPSS_DXCH_TABLES_INFO_INDIRECT_STC   *tablePtr,
    OUT GT_U32                                  *entryValuePtr
)
{
    GT_STATUS   rc;
    GT_U32      wordNum; /* iterator */

    /* busy wait for the control reg to show "read was done" */
    rc = checkStatusOfPreviousAccess(devNum, portGroupId,tablePtr);
    if(rc != GT_OK)
        return rc;
    /* set Control register for Read action */
    rc = setReadWriteCommandToControlReg(devNum, portGroupId, tablePtr, entryIndex,
                                          PRV_CPSS_DXCH_ACCESS_ACTION_READ_E);
    if(rc != GT_OK)
        return rc;

    /* busy wait for the control reg to show "read was done" */
    rc = checkStatusOfPreviousAccess(devNum, portGroupId,tablePtr);
    if(rc != GT_OK)
        return rc;

    /* table entry read - read to the buffer  - entryValuePtr != NULL */
    /* field table read, field table write - entryValuePtr == NULL  */
    if( entryValuePtr != NULL)
    {
        /* VLAN and STG indirect access tables need word swapping */
        if((tableType == CPSS_DXCH_TABLE_VLAN_E) ||
           (tableType == CPSS_DXCH_TABLE_STG_E))
        {
            for(wordNum = 0; wordNum < tableInfoPtr->entrySize; wordNum++)
            {
                /* read data register and perform swapping */
                rc = prvCpssHwPpPortGroupReadRegister(devNum, portGroupId,tablePtr->dataReg + wordNum * 4,
                                           entryValuePtr +
                                      (tableInfoPtr->entrySize - wordNum - 1));
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        }
        else
        {
            /* read all data registers */
            rc =  prvCpssHwPpPortGroupReadRam(devNum,portGroupId,
                            tablePtr->dataReg,
                            tableInfoPtr->entrySize,/* num of words */
                            entryValuePtr);
        }

    }

    return rc;
}

/**
* @internal prvCpssDxChTableEntryIndirectPrevDoneCheck function
* @endinternal
*
* @brief   Check that previous access to indirect table done.
*          Function use busy-wait pooling till it done.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - The port group id.
*                                      relevant only to 'multi-port-group' devices.
*                                      supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] tableType                - the specific table name
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*/
GT_STATUS prvCpssDxChTableEntryIndirectPrevDoneCheck
(
    IN GT_U8                            devNum,
    IN GT_U32                           portGroupId,
    IN CPSS_DXCH_TABLE_ENT              tableType
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_TABLES_INFO_STC           *tableInfoPtr; /* pointer to table entry */
    /* pointer to indirect table info */
    PRV_CPSS_DXCH_TABLES_INFO_INDIRECT_STC *tablePtr;

    tableInfoPtr = PRV_TABLE_INFO_PTR_GET_MAC(devNum,tableType);

    tablePtr =
         (PRV_CPSS_DXCH_TABLES_INFO_INDIRECT_STC*)(tableInfoPtr->writeTablePtr);

    /* busy wait for the control reg to show "read was done" */
    rc = checkStatusOfPreviousAccess(devNum, portGroupId,tablePtr);
    return rc;
}

/**
* @internal prvCpssDxChWriteTableEntry_indirect function
* @endinternal
*
* @brief   Write a whole entry to table by indirect access method.
*
* @note   APPLICABLE DEVICES:       xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - the port group Id
* @param[in] tableType                - the specific table name
* @param[in] tableInfoPtr             - pointer to table information
* @param[in] entryIndex               - index in the table
* @param[in] entryValuePtr            - (pointer to) the data for writing into the table
* @param[in] shadowEntryMaskPtr       - (pointer to) a mask. Ignored if entryValuePtr is NULL.
*                                      Used for updating shadow table entry only.
*                                      If NULL -  the entry specified by entryValuePtr
*                                      will be written to a shadow table as is.
*                                      If not NULL  - only bits that are raised in the
*                                      mask will be updated in the shadow table entry.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*/
GT_STATUS prvCpssDxChWriteTableEntry_indirect
(
    IN GT_U8                            devNum,
    IN GT_U32                           portGroupId,
    IN CPSS_DXCH_TABLE_ENT              tableType,
    IN const PRV_CPSS_DXCH_TABLES_INFO_STC    *tableInfoPtr,
    IN GT_U32                           entryIndex,
    IN GT_U32                           *entryValuePtr,
    IN GT_U32                           *shadowEntryMaskPtr
)
{
    GT_U32      wordNum; /* iterator */
    GT_STATUS   rc;
    /* pointer to indirect table info */
    PRV_CPSS_DXCH_TABLES_INFO_INDIRECT_STC *tablePtr;
    GT_U32      address;  /* address to write the information */

    tablePtr =
         (PRV_CPSS_DXCH_TABLES_INFO_INDIRECT_STC*)(tableInfoPtr->writeTablePtr);

    /* busy wait for the control reg to show "read was done" */
    rc = checkStatusOfPreviousAccess(devNum, portGroupId,tablePtr);
    if(rc != GT_OK)
        return rc;

    /* entryValuePtr == NULL for Cheetah VB init of VLAN table */
    if(entryValuePtr != NULL)
    {
        /* update HW table shadow (if supported and requested) */
        rc = prvCpssDxChPortGroupShadowLineUpdateMasked(devNum, portGroupId,
                                                        tableType, entryIndex,
                                                        entryValuePtr,
                                                        shadowEntryMaskPtr);
        if (rc != GT_OK)
        {
           CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "update hw table shadow failed");
        }

        /* VLAN and STG indirect access tables need word swapping */
        if((tableType == CPSS_DXCH_TABLE_VLAN_E) ||
           (tableType == CPSS_DXCH_TABLE_STG_E))
        {
            for(wordNum = 0; wordNum < tableInfoPtr->entrySize; wordNum++)
            {
                address = tablePtr->dataReg + wordNum * 4;
                /* write to data register and perform swapping */
                rc = prvCpssHwPpPortGroupWriteRegister(devNum, portGroupId,address,
                           entryValuePtr[tableInfoPtr->entrySize - wordNum - 1]);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }

        }
        else
        {
           /* non-direct table --- write to it as continues memory */
            rc = prvCpssHwPpPortGroupWriteRam(devNum,portGroupId,
                             tablePtr->dataReg,
                             tableInfoPtr->entrySize,
                             entryValuePtr);

            if (rc != GT_OK)
            {
                return rc;
            }

        }

    }
    else
    {
        /* updating shadow is not supported when entry value pointer is not given */
        CPSS_TBD_BOOKMARK
    }

    /* set Control register for Write action */
    return setReadWriteCommandToControlReg(devNum, portGroupId, tablePtr, entryIndex,
                                            PRV_CPSS_DXCH_ACCESS_ACTION_WRITE_E);

}

/**
* @internal indirectReadTableEntryField function
* @endinternal
*
* @brief   Read a field of entry from table by indirect access method.
*
* @note   APPLICABLE DEVICES:       xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - the port group Id
* @param[in] tableType                - the specific table name
* @param[in] tableInfoPtr             - pointer to table information
* @param[in] entryIndex               - index in the table
* @param[in] fieldWordNum             - field word number
* @param[in] fieldOffset              - field offset
* @param[in] fieldLength              - field length
*
* @param[out] fieldValuePtr            - pointer to the data read from the table
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*/

static GT_STATUS indirectReadTableEntryField
(
    IN GT_U8                        devNum,
    IN GT_U32                       portGroupId,
    IN CPSS_DXCH_TABLE_ENT       tableType,
    IN PRV_CPSS_DXCH_TABLES_INFO_STC *tableInfoPtr,
    IN GT_U32                       entryIndex,
    IN GT_U32                       fieldWordNum,
    IN GT_U32                       fieldOffset,
    IN GT_U32                       fieldLength,
    OUT GT_U32                      *fieldValuePtr
)
{
    GT_STATUS   rc;
    GT_U32 address; /* address to read from */
    /* pointer to indirect table info */
    PRV_CPSS_DXCH_TABLES_INFO_INDIRECT_STC *tablePtr;

    tablePtr = (PRV_CPSS_DXCH_TABLES_INFO_INDIRECT_STC*)(tableInfoPtr->readTablePtr);

    /* prepare to read action */
    rc = indirectReadTableEntry(devNum,portGroupId,tableType,tableInfoPtr,
                                entryIndex, tablePtr, NULL);
    if(rc != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);

    /* VLAN and STG indirect access tables need word swapping */
    if((tableType == CPSS_DXCH_TABLE_VLAN_E) ||
       (tableType == CPSS_DXCH_TABLE_STG_E))
    {
        fieldWordNum = tableInfoPtr->entrySize - fieldWordNum - 1;
    }

    address = tablePtr->dataReg + fieldWordNum * 0x4;

    /* read the field from the data register */
    if(prvCpssHwPpPortGroupGetRegField(devNum, portGroupId,address, fieldOffset, fieldLength, fieldValuePtr))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    return GT_OK;

}


/**
* @internal prvCpssDxChWriteTableEntryField_indirect function
* @endinternal
*
* @brief   Read a field from the indirect access table.
*
* @note   APPLICABLE DEVICES:       xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - the port group Id
* @param[in] tableType                - the specific table name
* @param[in] tableInfoPtr             - pointer to table information
* @param[in] entryIndex               - index in the table
* @param[in] fieldWordNum             - field word number
* @param[in] fieldOffset              - field offset
* @param[in] fieldLength              - field length
* @param[in] fieldValue               - value that will be written to the table
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*/

static GT_STATUS prvCpssDxChWriteTableEntryField_indirect
(
    IN GT_U8                          devNum,
    IN GT_U32                         portGroupId,
    IN CPSS_DXCH_TABLE_ENT         tableType,
    IN PRV_CPSS_DXCH_TABLES_INFO_STC  *tableInfoPtr,
    IN GT_U32                         entryIndex,
    IN GT_U32                         fieldWordNum,
    IN GT_U32                         fieldOffset,
    IN GT_U32                         fieldLength,
    IN GT_U32                         fieldValue
)
{
    GT_STATUS   rc;
    GT_U32 address; /* address to read from */
    /* pointer to indirect table info */
    PRV_CPSS_DXCH_TABLES_INFO_INDIRECT_STC *tablePtr;

    tablePtr =
         (PRV_CPSS_DXCH_TABLES_INFO_INDIRECT_STC*)(tableInfoPtr->writeTablePtr);

    /* read / write tableEntryField per port group */
    if(portGroupId == CPSS_PORT_GROUP_UNAWARE_MODE_CNS && PRV_CPSS_IS_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))
    {
        GT_PORT_GROUPS_BMP  portGroupsBmp;/* bmp of port groups */
        if(GT_TRUE ==
            prvCpssDuplicatedMultiPortGroupsGet(devNum,portGroupId,tablePtr->controlReg,
                &portGroupsBmp,NULL,NULL))
        {
            /* when the entry involve 'per port info' and 'global info'
             (like vlan entry) between hemispheres , need to update each port group
             according to it's existing info and not according to 'representative'
             port group */

            PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum, portGroupsBmp , portGroupId)
            {
                rc = prvCpssDxChWriteTableEntryField_indirect(
                    devNum, portGroupId, tableType, tableInfoPtr,
                    entryIndex, fieldWordNum,
                    fieldOffset, fieldLength, fieldValue);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
            PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum, portGroupsBmp , portGroupId)

            return GT_OK;
        }
    }

    /* read the entry -- put entry data to Data register */
    rc = indirectReadTableEntry(devNum,portGroupId , tableType, tableInfoPtr,
                                 entryIndex, tablePtr, NULL);
    if(rc != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);

    /* VLAN and STG indirect access tables need word swapping */
    if(((GT_U32)tableType == CPSS_DXCH_TABLE_VLAN_E) ||
       (tableType == CPSS_DXCH_TABLE_STG_E))
    {
        fieldWordNum = tableInfoPtr->entrySize - fieldWordNum - 1;
    }

    address = tablePtr->dataReg + fieldWordNum * 0x4;

    /* non-direct table write */
    rc =  prvCpssHwPpPortGroupSetRegField(devNum, portGroupId,address,fieldOffset,fieldLength,fieldValue);
    if(rc != GT_OK)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);

    /* set Control register for Write action */
    return setReadWriteCommandToControlReg(devNum, portGroupId ,tablePtr, entryIndex,
                                            PRV_CPSS_DXCH_ACCESS_ACTION_WRITE_E);

}


/**
* @internal prvCpssDxChReadTableEntry function
* @endinternal
*
* @brief   Read a whole entry from the table.
*
* @note   APPLICABLE DEVICES:       xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] tableType                - the specific table name
* @param[in] entryIndex               - index in the table
*
* @param[out] entryValuePtr            - (pointer to) the data read from the table
*                                      may be NULL in the case of indirect table.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In the case of entryValuePtr == NULL and indirect table the function
*       just send read entry command to device. And entry is stored in the
*       data registers of a indirect table
*
*/
GT_STATUS prvCpssDxChReadTableEntry

(
    IN GT_U8                   devNum,
    IN CPSS_DXCH_TABLE_ENT tableType,
    IN GT_U32                  entryIndex,
    OUT GT_U32                 *entryValuePtr
)
{
    GT_U32      portGroupId;/* support multi-port-groups device for table accesses with index = 'per port' */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    /* convert if needed the entry index , get specific portGroupId (or all port groups) */
    prvCpssDxChIndexAsPortNumConvert(devNum,tableType,entryIndex,
                                     &portGroupId , &entryIndex,&tableType);

    return prvCpssDxChPortGroupReadTableEntry(devNum,portGroupId,tableType,entryIndex,entryValuePtr);

}

/**
* @internal prvCpssDxChWriteTableEntry function
* @endinternal
*
* @brief   Write a whole entry to the table.
*
* @note   APPLICABLE DEVICES:       xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] tableType                - the specific table name
* @param[in] entryIndex               - index in the table
* @param[in] entryValuePtr            - (pointer to) the data that will be written to the table
*                                      may be NULL in the case of indirect table.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In the case of entryValuePtr == NULL and indirect table the function
*       just send write entry command to device. And entry is taken from the
*       data registers of a indirect table.
*
*/
GT_STATUS prvCpssDxChWriteTableEntry
(
    IN GT_U8                   devNum,
    IN CPSS_DXCH_TABLE_ENT tableType,
    IN GT_U32                  entryIndex,
    IN GT_U32                  *entryValuePtr
)
{
    GT_U32      portGroupId;/* support multi-port-groups device for table accesses with index = 'per port' */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    /* convert if needed the entry index , get specific portGroupId (or all port groups) */
    prvCpssDxChIndexAsPortNumConvert(devNum,tableType,entryIndex,
                                     &portGroupId , &entryIndex,&tableType);

    return prvCpssDxChPortGroupWriteTableEntry(devNum,portGroupId,tableType,entryIndex,entryValuePtr);
}

/**
* @internal prvCpssDxChReadTableEntryField function
* @endinternal
*
* @brief   Read a field from the table.
*
* @note   APPLICABLE DEVICES:       xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] tableType                - the specific table name
* @param[in] entryIndex               - index in the table
* @param[in] fieldWordNum             - field word number
*                                      use PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS
*                                      if need global offset in the field of fieldOffset
*                                      use PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS
*                                      if need 'field name' in the field of fieldOffset
* @param[in] fieldOffset              - field offset
* @param[in] fieldLength              - field length
*                                      when fieldWordNum == PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS
*                                      the field 'fieldLength' is used ONLY if  it's value != 0
*                                      and then it means 'Mumber of consecutive fields' !!!
*
* @param[out] fieldValuePtr            - (pointer to) the data read from the table
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChReadTableEntryField
(
    IN GT_U8                  devNum,
    IN CPSS_DXCH_TABLE_ENT tableType,
    IN GT_U32                 entryIndex,
    IN GT_U32                 fieldWordNum,
    IN GT_U32                 fieldOffset,
    IN GT_U32                 fieldLength,
    OUT GT_U32                *fieldValuePtr
)
{
    GT_U32      portGroupId;/* support multi-port-groups device for table accesses with index = 'per port' */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    /* convert if needed the entry index , get specific portGroupId (or all port groups) */
    prvCpssDxChIndexAsPortNumConvert(devNum,tableType,entryIndex,
                                     &portGroupId , &entryIndex,&tableType);

    return prvCpssDxChPortGroupReadTableEntryField(devNum,portGroupId ,
                tableType,entryIndex,fieldWordNum,
                fieldOffset,fieldLength,fieldValuePtr);
}

/**
* @internal prvCpssDxChWriteTableEntryField function
* @endinternal
*
* @brief   Write a field to the table.
*
* @note   APPLICABLE DEVICES:       xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] tableType                - the specific table name
* @param[in] entryIndex               - index in the table
* @param[in] fieldWordNum             - field word number
*                                      use PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS
*                                      if need global offset in the field of fieldOffset
*                                      use PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS
*                                      if need 'field name' in the field of fieldOffset
* @param[in] fieldOffset              - field offset
* @param[in] fieldLength              - field length
*                                      when fieldWordNum == PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS
*                                      the field 'fieldLength' is used ONLY if  it's value != 0
*                                      and then it means 'Mumber of consecutive fields' !!!
* @param[in] fieldValue               - the data write to the table
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChWriteTableEntryField
(
    IN GT_U8                  devNum,
    IN CPSS_DXCH_TABLE_ENT tableType,
    IN GT_U32                 entryIndex,
    IN GT_U32                 fieldWordNum,
    IN GT_U32                 fieldOffset,
    IN GT_U32                 fieldLength,
    IN GT_U32                 fieldValue
)
{
    GT_U32      portGroupId;/* support multi-port-groups device for table accesses with index = 'per port' */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    /* convert if needed the entry index , get specific portGroupId (or all port groups) */
    prvCpssDxChIndexAsPortNumConvert(devNum,tableType,entryIndex,
                                     &portGroupId , &entryIndex,&tableType);

    return prvCpssDxChPortGroupWriteTableEntryField(devNum,portGroupId ,
                tableType,entryIndex,fieldWordNum,
                fieldOffset,fieldLength,fieldValue);

}

/**
* @internal checkIsDevSupportLmuTable function
* @endinternal
*
* @brief   Function checks if the device supports LMU table by check chiplet status
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - PP device number
* @param[in] lmuTableIndex            - relative index of LMU table
*
* @retval GT_TRUE                  - the device   supports the table
* @retval GT_FALSE                 - the device NOT supports the table
*/
static GT_BOOL  checkIsDevSupportLmuTable(
    IN GT_U8      devNum ,
    IN GT_U32     lmuTableIndex
)
{
    GT_U32 ravenNum;       /* global number of Raven chiplet     */
    GT_U32 ravenInTileNum; /* relative number of Raven in a tile */
    GT_U32 tileNum;        /* tile number                        */

    /* specific check is only for Falcon */
    if(PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_FALCON_E)
        return GT_TRUE;

    /* each Raven has two tables */
    ravenNum = lmuTableIndex >> 1;
    /* each tile holds four Ravens */
    tileNum = ravenNum >> 2;
    ravenInTileNum = ravenNum & 0x3;

    if((1 << tileNum) & PRV_CPSS_PP_MAC(devNum)->multiPipe.mirroredTilesBmp)
    {
        /* mirrored tile - Raven 'sit' on mirrored Addresses */
        ravenInTileNum   = 3 - ravenInTileNum;
        ravenNum = (tileNum << 2) + ravenInTileNum;
    }

    /* check that Raven is exist in device */
    if(0 == (PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->activeRavensBitmap & (1 << ravenNum)))
    {
        return GT_FALSE;
    }

    return GT_TRUE;
}

/**
* @internal prvCpssDxChTableNumEntriesGet function
* @endinternal
*
* @brief   get the number of entries in a table
*         needed for debug purpose
*
* @note   APPLICABLE DEVICES:       xCat3; AC5; Lion2; Bobcat2; Bobcat3; Aldrin; Aldrin2; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] tableType                - the specific table name
*
* @param[out] numEntriesPtr            - (pointer to) number of entries in the table
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTableNumEntriesGet
(
    IN GT_U8                    devNum,
    IN CPSS_DXCH_TABLE_ENT      tableType,
    OUT GT_U32                  *numEntriesPtr
)
{
    GT_U32      tileId;                                        /* tileId iterator */
    GT_U32      numOfTiles;                                    /* number of tiles */
    GT_U32      regAddr = 0;
    PRV_CPSS_DXCH_TABLES_INFO_DIRECT_STC   *directTablePtr;    /* pointer to direct access table entry info.*/
    PRV_CPSS_DXCH_TABLES_INFO_INDIRECT_STC *indirectTablePtr;  /* pointer to indirect access table entry info.*/
    PRV_CPSS_DXCH_TABLES_INFO_STC          *tableInfoPtr;      /* pointer to table info */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    /* validity check */
    if((GT_U32)tableType >= PRV_CPSS_DXCH_TABLE_SIZE_GET_MAC(devNum))
    {
       return /* not error */ GT_OUT_OF_RANGE;
    }

    if(numEntriesPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    tableInfoPtr = PRV_TABLE_INFO_PTR_GET_MAC(devNum,tableType);
    *numEntriesPtr = tableInfoPtr->maxNumOfEntries;

    /* For SIP6 TXQ and LMU tables are multiple and present in each tile ( all 4 Tiles ),
       to avoid error crash in tables related to non existing tiles,
       the numEntries is set to "Zero" for the table of non existing tiles*/

    if(*numEntriesPtr != 0 &&
        PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)/* only Falcon */
    {

        if(tableInfoPtr->readAccessType == PRV_CPSS_DXCH_INDIRECT_ACCESS_E)
        {
            indirectTablePtr = (PRV_CPSS_DXCH_TABLES_INFO_INDIRECT_STC*)(tableInfoPtr->readTablePtr);
            regAddr = indirectTablePtr->controlReg;
        }
        else
        {
            directTablePtr = (PRV_CPSS_DXCH_TABLES_INFO_DIRECT_STC*)(tableInfoPtr->readTablePtr);
            regAddr = directTablePtr->baseAddress;
        }

        tileId = regAddr/FALCON_TILE_OFFSET_CNS;
        numOfTiles = PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;

        if(tileId >= numOfTiles)
        {
            *numEntriesPtr = 0;
        }
        else if(tableType >= CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E &&
                tableType <= CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_LAST_E)
        {
            /* check disabled chiplets */
            if (!checkIsDevSupportLmuTable(devNum, tableType - CPSS_DXCH_SIP6_TABLE_LMU_CONFIGURATION_E))
            {
                *numEntriesPtr = 0;
            }
        }
        else if(tableType >= CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E &&
                tableType <= CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_LAST_E)
        {
            /* check disabled chiplets */
            if (!checkIsDevSupportLmuTable(devNum, tableType - CPSS_DXCH_SIP6_TABLE_LMU_STATISTICS_E))
            {
                *numEntriesPtr = 0;
            }
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChTableEntrySizeGet function
* @endinternal
*
* @brief   get the entry size in 32 bit words
*         needed for debug purpose
*
* @note   APPLICABLE DEVICES:       xCat3; AC5; Lion2; Bobcat2; Bobcat3; Aldrin; Aldrin2; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] tableType                - the specific table name
*
* @param[out] entrySizePtr            - (pointer to) entry size in 32 bit words
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvCpssDxChTableEntrySizeGet
(
    IN GT_U8                    devNum,
    IN CPSS_DXCH_TABLE_ENT      tableType,
    OUT GT_U32                  *entrySizePtr
)
{
    GT_STATUS                      rc;             /* return code */
    PRV_CPSS_DXCH_TABLES_INFO_STC  *tableInfoPtr;  /* pointer to table info */
    GT_U32                         ratio;      /* number of entries for in single line or number of lines  for in single entry. */
    GT_BOOL                        isMultiple; /*  indication that ratio is 'multiple' or 'fraction'.
                                             GT_TRUE  - ratio is 'multiple' (number of entries for in single line)
                                             GT_FALSE - ratio is 'fraction' (number of lines  for in single entry) */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    /* validity check */
    if((GT_U32)tableType >= PRV_CPSS_DXCH_TABLE_SIZE_GET_MAC(devNum))
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);

    if(entrySizePtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }

    tableInfoPtr = PRV_TABLE_INFO_PTR_GET_MAC(devNum,tableType);
    *entrySizePtr = tableInfoPtr->entrySize;

    rc = prvCpssDxChTableEngineToHwRatioGet(devNum,tableType,&ratio,&isMultiple);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(ratio == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    if(ratio != 1)
    {
        if(isMultiple == GT_FALSE)
        {
            /* ratio is 'fraction' (number of lines  in single entry) */
            *entrySizePtr = ratio * tableInfoPtr->entrySize;
        }
    }

    return GT_OK;
}

/* tables that supported only by device with TM */
const CPSS_DXCH_TABLE_ENT prvCpssDxChHwTrafficManagerTablesArr[] = {
     CPSS_DXCH_SIP5_TABLE_TM_FCU_ETH_DMA_TO_TM_PORT_MAPPING_E
    ,CPSS_DXCH_SIP5_TABLE_TM_FCU_ILK_DMA_TO_TM_PORT_MAPPING_E
    ,CPSS_DXCH_SIP5_TABLE_TM_FCU_TC_PORT_TO_CNODE_PORT_MAPPING_E
    ,CPSS_DXCH_SIP5_TABLE_TM_FCU_PORT_INGRESS_TIMERS_CONFIG_E
    ,CPSS_DXCH_SIP5_TABLE_TM_FCU_INGRESS_TIMERS_E
    ,CPSS_DXCH_SIP5_TABLE_TM_FCU_PORT_TO_PHYSICAL_PORT_MAPPING_E

    ,CPSS_DXCH_SIP5_TABLE_TM_DROP_DROP_MASKING_E
    ,CPSS_DXCH_SIP5_TABLE_TM_DROP_QUEUE_PROFILE_ID_E
    ,CPSS_DXCH_SIP5_TABLE_TM_QMAP_CPU_CODE_TO_TC_E
    ,CPSS_DXCH_SIP5_TABLE_TM_QMAP_TARGET_DEV_TO_INDEX_E
    ,CPSS_DXCH_SIP5_TABLE_TM_QMAP_POLICING_ENABLE_E
    ,CPSS_DXCH_SIP5_TABLE_TM_QMAP_QUEUE_ID_SELECTOR_E
    ,CPSS_DXCH_SIP5_TABLE_TM_INGR_GLUE_L1_PKT_LEN_OFFSET_E
    ,CPSS_DXCH_SIP5_TABLE_TM_EGR_GLUE_AGING_QUEUE_PROFILE_E
    ,CPSS_DXCH_SIP5_TABLE_TM_EGR_GLUE_AGING_PROFILE_THESHOLDS_E
    ,CPSS_DXCH_SIP5_TABLE_TM_EGR_GLUE_TARGET_INTERFACE_E

    /* must be last */
    ,CPSS_DXCH_TABLE_LAST_E
};

/**
* @internal xcat3InitTable function
* @endinternal
*
* @brief   Allocate and init the xCat3, AC5 tables info DB
*
* @retval GT_OK                    - on success.
* @retval GT_OUT_OF_CPU_MEM        - on malloc failure
*/
static GT_STATUS xcat3InitTable
(
    void
)
{
    PRV_CPSS_DXCH_TABLES_INFO_STC   *tableInfoPtr; /* pointer to table info */
    GT_U32                           ii;           /* iterator */

    if (PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(dxChXcat3TablesInfoPtr))
    {
        /* table is already set */
        return GT_OK;
    }

    /* allocate shared global table */
    tableInfoPtr = cpssOsMalloc(sizeof(dxChXcat3TablesInfo));
    if (tableInfoPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }

    PRV_SHARED_HW_INIT_TABLES_DB_VAR_SET(dxChXcat3TablesInfoPtr, tableInfoPtr);

    /* copy content from constant DB */
    for(ii = 0 ; ii < dxChXcat3TableInfoSize ; ii++)
    {
        tableInfoPtr[ii] = dxChXcat3TablesInfo[ii];
    }

    return GT_OK;
}

#define GET_TEMP_TABLES_PER_FAMILY_ARR  \
    PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(tempTablesPerFamilyArr)

#define GET_INIT_TABLES_PER_FAMILY_ARR  \
    PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(tablesDbInitPreFamily)

#define SET_TABLES_DB_DONE_MAC(_family) \
    GET_INIT_TABLES_PER_FAMILY_ARR[_family>>5] |= (1 << ((_family) & 0x1f))

#define GET_TABLES_DB_DONE_MAC(_family) \
    ((GET_INIT_TABLES_PER_FAMILY_ARR[_family>>5] & (1 << ((_family) & 0x1f))) ? 1 : 0)

/**
* @internal allocTableInfo function
* @endinternal
*
* @brief   Allocate and tables info DB : global of the device and direct
*
* @param[in] numEntries_tablesInfoArr  - number of entries needed for tablesInfoArr
* @param[in] numEntries_directAccessTableInfoArr  - number of entries needed for directAccessTablesInfoArr
*
* @param[out] tablesInfoArrPtr          - (pointer to) tablesInfoArr
* @param[out] directAccessTablesInfoArrPtr          - (pointer to) directAccessTablesInfoArrPtr
*
* @retval GT_OK                    - on success.
* @retval GT_OUT_OF_CPU_MEM        - on malloc failure
*/
static GT_STATUS    allocTableInfo(
    IN  GT_U32                          numEntries_tablesInfoArr,
    OUT PRV_CPSS_DXCH_TABLES_INFO_STC** tablesInfoArrPtr,
    IN  GT_U32                          numEntries_directAccessTableInfoArr,
    OUT PRV_CPSS_DXCH_TABLES_INFO_DIRECT_UNIT_STC** directAccessTablesInfoArrPtr
)
{
    GT_U32  size;

    size = numEntries_tablesInfoArr * sizeof(PRV_CPSS_DXCH_TABLES_INFO_STC);

    (*tablesInfoArrPtr) = cpssOsMalloc(size);
    if ((*tablesInfoArrPtr) == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }
    cpssOsMemSet((*tablesInfoArrPtr), 0, size);

    size = numEntries_directAccessTableInfoArr * sizeof(PRV_CPSS_DXCH_TABLES_INFO_DIRECT_UNIT_STC);

    (*directAccessTablesInfoArrPtr) = cpssOsMalloc(size);
    if ((*directAccessTablesInfoArrPtr) == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }
    cpssOsMemSet((*directAccessTablesInfoArrPtr), 0, size);

    return GT_OK;
}

/**
* @internal prvCpssDxChTablesAccessInit function
* @endinternal
*
* @brief   Initializes all structures for table access.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] regularInit              - GT_TRUE normal init procedure
*                                       GT_FALSE short init procedure,
*                                                fine tuning is not available.
*/
GT_STATUS  prvCpssDxChTablesAccessInit
(
    IN GT_U8    devNum,
    IN GT_BOOL  regularInit
)
{
    PRV_CPSS_DXCH_PP_CONFIG_FINE_TUNING_STC *fineTuningPtr;
    PRV_CPSS_DXCH_TABLES_INFO_STC      *accessTableInfoPtr;/*pointer to tables info*/
    GT_U32                              accessTableInfoSize;/*tables info size */
    GT_STATUS   rc = GT_OK;
    GT_U32  ii,tableIndex;
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }

    fineTuningPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning;

    switch(PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_AC5_E:
        case CPSS_PP_FAMILY_DXCH_XCAT3_E:
            rc = xcat3InitTable();
            if (rc != GT_OK)
            {
                return rc;
            }

            accessTableInfoPtr = PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(dxChXcat3TablesInfoPtr);
            accessTableInfoSize = dxChXcat3TableInfoSize;
            break;
        case CPSS_PP_FAMILY_DXCH_LION2_E:
            rc = lion2InitTable();
            if (rc != GT_OK)
            {
                return rc;
            }
            accessTableInfoPtr = PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(lion2TablesInfoPtr);
            accessTableInfoSize = lion2TableInfoSize;
            break;
        case CPSS_PP_FAMILY_DXCH_BOBCAT3_E:
            rc = bobcat3InitTable(devNum);
            if (rc != GT_OK)
            {
                return rc;
            }
            accessTableInfoPtr = PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(bobcat3TablesInfoPtr);
            accessTableInfoSize = bobcat3TableInfoSize;
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            rc = aldrin2InitTable(devNum);
            if (rc != GT_OK)
            {
                return rc;
            }
            accessTableInfoPtr = PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(aldrin2TablesInfoPtr);
            accessTableInfoSize = aldrin2TableInfoSize;
            break;
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            rc = falconInitTable(devNum);
            if (rc != GT_OK)
            {
                return rc;
            }
            accessTableInfoPtr = PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(falconTablesInfoPtr);
            accessTableInfoSize = falconTableInfoSize;
            break;
        case CPSS_PP_FAMILY_DXCH_AC5P_E:
            rc = hawkInitTable(devNum);
            if (rc != GT_OK)
            {
                return rc;
            }
            accessTableInfoPtr = PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(hawkTablesInfoPtr);
            accessTableInfoSize = hawkTableInfoSize;
            break;
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
            rc = phoenixInitTable(devNum);
            if (rc != GT_OK)
            {
                return rc;
            }
            accessTableInfoPtr = PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(phoenixTablesInfoPtr);
            accessTableInfoSize = phoenixTableInfoSize;
            break;
        case CPSS_PP_FAMILY_DXCH_HARRIER_E:
            rc = harrierInitTable(devNum);
            if (rc != GT_OK)
            {
                return rc;
            }
            accessTableInfoPtr = PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(harrierTablesInfoPtr);
            accessTableInfoSize = harrierTableInfoSize;
            break;
        case CPSS_PP_FAMILY_DXCH_BOBCAT2_E:
            if (PRV_CPSS_PP_MAC(devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E)
            {
                rc = bobkInitTable(devNum);
                if (rc != GT_OK)
                {
                    return rc;
                }
                accessTableInfoPtr = PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(bobkTablesInfoPtr);
                accessTableInfoSize = bobkTableInfoSize;
            }
            else
            {
                rc = bobcat2InitTable(devNum);
                if (rc != GT_OK)
                {
                    return rc;
                }
                accessTableInfoPtr = PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(bobcat2TablesInfoPtr);
                accessTableInfoSize = bobcat2TableInfoSize;
            }
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
        case CPSS_PP_FAMILY_DXCH_AC3X_E  :
            /* aldrin is based on cetus design - use bobk tables */
            rc = bobkInitTable(devNum);
            if (rc != GT_OK)
            {
                return rc;
            }
            accessTableInfoPtr = PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(bobkTablesInfoPtr);
            accessTableInfoSize = bobkTableInfoSize;
            break;
        case CPSS_PP_FAMILY_DXCH_IRONMAN_E:
            rc = ironmanInitTable(devNum);
            if (rc != GT_OK)
            {
                return rc;
            }
            accessTableInfoPtr  = PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(ironman_L_TablesInfoPtr);
            accessTableInfoSize = ironman_L_TableInfoSize;
            break;
        default:
            /* not supported/implemented yet */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_DXCH_PP_MAC(devNum)->accessTableInfoPtr = accessTableInfoPtr;
    PRV_CPSS_DXCH_PP_MAC(devNum)->accessTableInfoSize = accessTableInfoSize;

    SET_TABLES_DB_DONE_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily);

    if (!PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        /* update tables size according to "fine tuning" */
        accessTableInfoPtr[CPSS_DXCH3_LTT_TT_ACTION_E].maxNumOfEntries =
            PRV_CPSS_DXCH_ROUTER_TUNNEL_TERM_GET_MAX_SIZE_MAC(devNum);
        accessTableInfoPtr[CPSS_DXCH2_UC_MC_ROUTE_NEXT_HOP_E].maxNumOfEntries =
            fineTuningPtr->tableSize.routerNextHop;
    }

    if (regularInit)
    {
        /* Need to take the bigger value between ARP entries / 4 and TS entries */
        if ((PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.routerArp/4) >
            (PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.tunnelStart))
        {
            accessTableInfoPtr[CPSS_DXCH2_TABLE_TUNNEL_START_CONFIG_E].maxNumOfEntries =
                PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.routerArp / 4;
        }
        else
        {
            accessTableInfoPtr[CPSS_DXCH2_TABLE_TUNNEL_START_CONFIG_E].maxNumOfEntries =
                PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.tunnelStart;
        }
    }

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        /* each 'line' contains 2 'regular' TS entries */
        accessTableInfoPtr[CPSS_DXCH2_TABLE_TUNNEL_START_CONFIG_E].maxNumOfEntries /= 2;

        /* each TCAM entry have two HW entries.
           entries for banks 12-15 are not valid but exist in address space.
           maxNumOfEntries = TCAM_Size_in_10B_entries * 2 * (16/12) */
        if(PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.tunnelTerm != 0)
        {
            accessTableInfoPtr[CPSS_DXCH_SIP5_TABLE_TCAM_E].maxNumOfEntries = (PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.tunnelTerm * 16)/6;
        }
        else /* the device hold no TTI to TCAM connection , so use the 'PCL' number */
        {
            accessTableInfoPtr[CPSS_DXCH_SIP5_TABLE_TCAM_E].maxNumOfEntries = (PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.policyTcamRaws * 16)/6;
        }

        /* TCAM action per two TCAM HW entries (one logical rule)*/
        accessTableInfoPtr[CPSS_DXCH_SIP5_TABLE_TCAM_PCL_TTI_ACTION_E].maxNumOfEntries =
                       accessTableInfoPtr[CPSS_DXCH_SIP5_TABLE_TCAM_E].maxNumOfEntries / 2;

        /* TM tables supported starting from SIP_5 devices */
        if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.trafficManager.supported == GT_FALSE)
        {
            for(ii = 0 ; prvCpssDxChHwTrafficManagerTablesArr[ii] != CPSS_DXCH_TABLE_LAST_E ; ii++)
            {
                tableIndex = prvCpssDxChHwTrafficManagerTablesArr[ii];
                accessTableInfoPtr[tableIndex].maxNumOfEntries = 0;
            }
        }
    }

    if(PRV_CPSS_DXCH_BOBCAT2_A0_CHECK_MAC(devNum))
    {
        /* the table not applicable for 'A0' revision */
        tableIndex = CPSS_DXCH_SIP5_TABLE_TM_FCU_PORT_TO_PHYSICAL_PORT_MAPPING_E;
        accessTableInfoPtr[tableIndex].maxNumOfEntries = 0;

        tableIndex = CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_IPFIX_ALERT_WRAPAROUND_E;
        accessTableInfoPtr[tableIndex].maxNumOfEntries = 64;
        tableIndex = CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_IPFIX_ALERT_WRAPAROUND_E;
        accessTableInfoPtr[tableIndex].maxNumOfEntries = 64;
        tableIndex = CPSS_DXCH_SIP5_TABLE_EGRESS_POLICER_IPFIX_ALERT_WRAPAROUND_E;
        accessTableInfoPtr[tableIndex].maxNumOfEntries = 64;

        tableIndex = CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_0_IPFIX_ALERT_AGING_E;
        accessTableInfoPtr[tableIndex].maxNumOfEntries = 64;
        tableIndex = CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_IPFIX_ALERT_AGING_E;
        accessTableInfoPtr[tableIndex].maxNumOfEntries = 64;
        tableIndex = CPSS_DXCH_SIP5_TABLE_EGRESS_POLICER_IPFIX_ALERT_AGING_E;
        accessTableInfoPtr[tableIndex].maxNumOfEntries = 64;
    }


    /* Due to Cheetah 3 Errata - " FEr#2028: Direct access under traffic to Pre-Egress
     Engine (EQ) tables is not functional"
    -- see PRV_CPSS_DXCH3_DIRECT_ACCESS_TO_EQ_ADDRESS_SPACE_WA_E */
    if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum, PRV_CPSS_DXCH3_DIRECT_ACCESS_TO_EQ_ADDRESS_SPACE_WA_E))
    {
        /* CPSS_DXCH_TABLE_TRUNK_MEMBERS_E */
        accessTableInfoPtr[CPSS_DXCH_TABLE_TRUNK_MEMBERS_E].readAccessType =
            PRV_CPSS_DXCH_INDIRECT_ACCESS_E;
        accessTableInfoPtr[CPSS_DXCH_TABLE_TRUNK_MEMBERS_E].readTablePtr =
            indirectAccessTableInfo + CPSS_DXCH_TABLE_TRUNK_MEMBERS_E;
        accessTableInfoPtr[CPSS_DXCH_TABLE_TRUNK_MEMBERS_E].writeAccessType =
            PRV_CPSS_DXCH_INDIRECT_ACCESS_E;
        accessTableInfoPtr[CPSS_DXCH_TABLE_TRUNK_MEMBERS_E].writeTablePtr =
            indirectAccessTableInfo + CPSS_DXCH_TABLE_TRUNK_MEMBERS_E;

        /* CPSS_DXCH_TABLE_STATISTICAL_RATE_LIMIT_E */
        accessTableInfoPtr[CPSS_DXCH_TABLE_STATISTICAL_RATE_LIMIT_E].readAccessType =
            PRV_CPSS_DXCH_INDIRECT_ACCESS_E;
        accessTableInfoPtr[CPSS_DXCH_TABLE_STATISTICAL_RATE_LIMIT_E].readTablePtr =
            indirectAccessTableInfo + CPSS_DXCH_TABLE_STATISTICAL_RATE_LIMIT_E;
        accessTableInfoPtr[CPSS_DXCH_TABLE_STATISTICAL_RATE_LIMIT_E].writeAccessType =
            PRV_CPSS_DXCH_INDIRECT_ACCESS_E;
        accessTableInfoPtr[CPSS_DXCH_TABLE_STATISTICAL_RATE_LIMIT_E].writeTablePtr =
            indirectAccessTableInfo + CPSS_DXCH_TABLE_STATISTICAL_RATE_LIMIT_E;

        /* CPSS_DXCH_TABLE_CPU_CODE_E */
        accessTableInfoPtr[CPSS_DXCH_TABLE_CPU_CODE_E].readAccessType =
            PRV_CPSS_DXCH_INDIRECT_ACCESS_E;
        accessTableInfoPtr[CPSS_DXCH_TABLE_CPU_CODE_E].readTablePtr =
            indirectAccessTableInfo + CPSS_DXCH_TABLE_CPU_CODE_E;
        accessTableInfoPtr[CPSS_DXCH_TABLE_CPU_CODE_E].writeAccessType =
            PRV_CPSS_DXCH_INDIRECT_ACCESS_E;
        accessTableInfoPtr[CPSS_DXCH_TABLE_CPU_CODE_E].writeTablePtr =
            indirectAccessTableInfo + CPSS_DXCH_TABLE_CPU_CODE_E;

        /* CPSS_DXCH_TABLE_QOS_PROFILE_E */
        accessTableInfoPtr[CPSS_DXCH_TABLE_QOS_PROFILE_E].readAccessType =
            PRV_CPSS_DXCH_INDIRECT_ACCESS_E;
        accessTableInfoPtr[CPSS_DXCH_TABLE_QOS_PROFILE_E].readTablePtr =
            indirectAccessTableInfo + CPSS_DXCH_TABLE_QOS_PROFILE_E;
        accessTableInfoPtr[CPSS_DXCH_TABLE_QOS_PROFILE_E].writeAccessType =
            PRV_CPSS_DXCH_INDIRECT_ACCESS_E;
        accessTableInfoPtr[CPSS_DXCH_TABLE_QOS_PROFILE_E].writeTablePtr =
            indirectAccessTableInfo + CPSS_DXCH_TABLE_QOS_PROFILE_E;
    }
    if (!((tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E) &&
         (tempSystemRecovery_Info.systemRecoveryState == CPSS_SYSTEM_RECOVERY_INIT_STATE_E)) )
    {
        /* do additional TTI tables init */
        rc = prvCpssDxChTtiDbTablesInit(devNum);
    }

    return rc;
}

/**
* @internal prvCpssDxChPortGroupWriteTableEntryFieldList function
* @endinternal
*
* @brief   Write a list of fields to the table.
*
* @note   APPLICABLE DEVICES:       xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - the port group Id , to support multi-port-group devices that need to access
*                                      specific port group
* @param[in] tableType                - HW table Id
* @param[in] entryIndex               - entry Index
* @param[in] entryMemoBufArr[]        - the work memory for read, update and write the entry
* @param[in] fieldsAmount             - amount of updated fields in the entry
* @param[in] fieldOffsetArr[]         - (array) the offset of the field in bits
*                                      from the entry origin
* @param[in] fieldLengthArr[]         - (array) the length of the field in bits,
*                                      0 - means to skip this subfield
* @param[in] fieldValueArr[]          - (array) the value of the field
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*/
GT_STATUS prvCpssDxChPortGroupWriteTableEntryFieldList
(
    IN GT_U8                                  devNum,
    IN GT_U32                                 portGroupId,
    IN CPSS_DXCH_TABLE_ENT                tableType,
    IN GT_U32                                 entryIndex,
    IN GT_U32                                 entryMemoBufArr[],
    IN GT_U32                                 fieldsAmount,
    IN GT_U32                                 fieldOffsetArr[],
    IN GT_U32                                 fieldLengthArr[],
    IN GT_U32                                 fieldValueArr[]
)
{
    GT_STATUS                  res;           /* return code               */
    GT_U32                     idx;           /* field index               */
    GT_U32                     wordIdx;       /* word index                */
    GT_U32                     wordShift;     /* Shift in the word         */
    GT_U32                     len0;          /* length in the 1-th word   */
    GT_U32                     len1;          /* length in the 2-th word   */
    GT_U32                     entryMaskBuf[PRV_CPSS_DXCH_TABLE_MAX_ENTRY_SIZE_CNS];

    /* read / write tableEntryField per port group */
    if(portGroupId == CPSS_PORT_GROUP_UNAWARE_MODE_CNS)
    {
        /* update table in all port groups */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(devNum, portGroupId)
        {
            res = prvCpssDxChPortGroupWriteTableEntryFieldList(
                devNum, portGroupId, tableType, entryIndex,
                entryMemoBufArr, fieldsAmount,
                fieldOffsetArr, fieldLengthArr, fieldValueArr);
            if( res != GT_OK)
            {
                return res;
            }
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(devNum, portGroupId)

        return GT_OK;
    }

    cpssOsMemSet(entryMaskBuf, 0, sizeof(entryMaskBuf));

    for (idx = 0; (idx < fieldsAmount); idx++)
    {
        if (fieldLengthArr[idx] > 32)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        if (fieldLengthArr[idx] == 0)
            continue;

        /* raise field's bits in the mask */
        if (((fieldOffsetArr[idx] + fieldLengthArr[idx]) >> 5) >= PRV_CPSS_DXCH_TABLE_MAX_ENTRY_SIZE_CNS)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        raiseBitsInMemory(entryMaskBuf, fieldOffsetArr[idx], fieldLengthArr[idx]);


        wordIdx   = fieldOffsetArr[idx] >> 5;
        wordShift = fieldOffsetArr[idx] & 0x1F;

        /* the length of the field or of it's intersection with the first word */
        len0 = ((wordShift + fieldLengthArr[idx]) <= 32)
            ? fieldLengthArr[idx] : (32 - wordShift);

        /* copy the field or it's intersection with the first word */
        U32_SET_FIELD_MASKED_MAC(
           entryMemoBufArr[wordIdx], wordShift, len0, fieldValueArr[idx]);

        /* copy the field intersection with the second word */
        if (len0 != fieldLengthArr[idx])
        {
            len1 = fieldLengthArr[idx] - len0;
            U32_SET_FIELD_MASKED_MAC(
               entryMemoBufArr[wordIdx + 1] ,0 /*offset*/, len1,
               (fieldValueArr[idx] >> len0));
        }
    }

    return  prvCpssDxChPortGroupWriteTableEntryMasked(
        devNum, portGroupId,tableType, entryIndex, entryMemoBufArr, entryMaskBuf);
}

/**
* @internal prvCpssDxChPortGroupReadTableEntryFieldList function
* @endinternal
*
* @brief   Read a list of fields from the table.
*
* @note   APPLICABLE DEVICES:       xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - the port group Id , to support multi-port-group devices that need to access
*                                      specific port group
* @param[in] tableId                  - HW table Id
* @param[in] entryIndex               - entry Index
* @param[in] entryMemoBufArr[]        - the work memory for read, update and write the entry
* @param[in] fieldsAmount             - amount of updated fields in the entry
* @param[in] fieldOffsetArr[]         - (array) the offset of the field in bits
*                                      from the entry origin
* @param[in] fieldLengthArr[]         - (array) the length of the field in bits,
*                                      0 - means to skip this subfield
*
* @param[out] fieldValueArr[]          - (array) the value of the field
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*/
GT_STATUS prvCpssDxChPortGroupReadTableEntryFieldList
(
    IN  GT_U8                                  devNum,
    IN GT_U32                                 portGroupId,
    IN  CPSS_DXCH_TABLE_ENT                tableId,
    IN  GT_U32                                 entryIndex,
    IN  GT_U32                                 entryMemoBufArr[],
    IN  GT_U32                                 fieldsAmount,
    IN  GT_U32                                 fieldOffsetArr[],
    IN  GT_U32                                 fieldLengthArr[],
    OUT GT_U32                                 fieldValueArr[]
)
{
    GT_STATUS                  res;           /* return code               */
    GT_U32                     idx;           /* field index               */
    GT_U32                     wordIdx;       /* word index                */
    GT_U32                     wordShift;     /* Shift in the word         */
    GT_U32                     len0;          /* length in the 1-th word   */
    GT_U32                     len1;          /* length in the 2-th word   */

    res = prvCpssDxChPortGroupReadTableEntry(
        devNum, portGroupId, tableId, entryIndex, entryMemoBufArr);
    if (res != GT_OK)
    {
        return res;
    }

    for (idx = 0; (idx < fieldsAmount); idx++)
    {
        if (fieldLengthArr[idx] > 32)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        if (fieldLengthArr[idx] == 0)
        {
            fieldValueArr[idx] = 0;
            continue;
        }

        wordIdx   = fieldOffsetArr[idx] >> 5;
        wordShift = fieldOffsetArr[idx] & 0x1F;

        /* the length of the field or of it's intersection with the first word */
        len0 = ((wordShift + fieldLengthArr[idx]) <= 32)
            ? fieldLengthArr[idx] : (32 - wordShift);

        /* copy the field or it's intersection with the first word */
        fieldValueArr[idx] =
            U32_GET_FIELD_MAC(entryMemoBufArr[wordIdx], wordShift, len0);

        /* copy the field intersection with the second word */
        if (len0 != fieldLengthArr[idx])
        {
            len1 = fieldLengthArr[idx] - len0;
            fieldValueArr[idx] |=
                ((U32_GET_FIELD_MAC(
                    entryMemoBufArr[wordIdx + 1], 0/*offset*/, len1))
                 << len0);
        }
    }

    return  GT_OK;
}


#ifdef CHEETAH_TABLE_DEBUG

GT_STATUS readWriteFieldTest
(
    IN GT_U8                  devNum,
    IN CPSS_DXCH_TABLE_ENT tableType
)
{
    PRV_CPSS_DXCH_TABLES_INFO_STC *tableInfoPtr; /* pointer to table info */
    GT_U32 field;
    GT_U32 readVal;
    GT_U32 wordNum;
    GT_U32 fieldOffset;
    GT_STATUS rc;
    GT_STATUS fieldLength;

    fieldLength = 32;
    tableInfoPtr = &(PRV_CPSS_DXCH_PP_MAC(devNum)->accessTableInfoPtr[tableType]);


    cpssOsSrand(5);

    for(wordNum = 0; wordNum < tableInfoPtr->entrySize; wordNum++)
    {
        for(fieldOffset = 0; fieldOffset < 0; fieldOffset+=fieldLength)
        {
            field = cpssOsRand();
            rc= coreCheetahWriteTableEntryField(devNum,tableType,1,wordNum,
                                                fieldOffset, fieldLength,field);
            if(rc != GT_OK)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);

        }
    }

    cpssOsSrand(5);
    for(wordNum = 0; wordNum < tableInfoPtr->entrySize; wordNum++)
    {
        for(fieldOffset = 0; fieldOffset < 0; fieldOffset+=fieldLength)
        {
            field = cpssOsRand();
            rc= coreCheetahReadTableEntryField(devNum,tableType,1,wordNum,
                                            fieldOffset, fieldLength, &readVal);
            if(rc != GT_OK)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            if(field != readVal)
            {
                cpssOsPrintf("\n wordNum =  %d ",wordNum);
                cpssOsPrintf("\n fieldOffset =  %d ",fieldOffset);
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }

        }

    }
    cpssOsPrintf("\n Field test OK, tableType =  %d ",tableType);


    return GT_OK;
}




GT_U32 getMaskBits
(
    IN CPSS_DXCH_TABLE_ENT tableType
)
{
    GT_U32     maskBits;


        switch(tableType)
        {
            case CPSS_DXCH_TABLE_VLAN_PORT_PROTOCOL_E:
                maskBits = 0x7ff;
                break;

            case CPSS_DXCH_TABLE_PORT_VLAN_QOS_E:
                maskBits = 0x7ff;
                break;

            case CPSS_DXCH_TABLE_TRUNK_MEMBERS_E:
                maskBits = 0x7ff;
                break;

            case CPSS_DXCH_TABLE_STATISTICAL_RATE_LIMIT_E:
                maskBits = 0xffffffff;
                break;

            case CPSS_DXCH_TABLE_CPU_CODE_E:
                maskBits = 0x3fff;
                break;

            case CPSS_DXCH_TABLE_PCL_CONFIG_E:
                maskBits = 0x1fffffff;
                break;

            case CPSS_DXCH_TABLE_QOS_PROFILE_E:
                maskBits = 0x3fff;
                break;

            case CPSS_DXCH_TABLE_REMARKING_E:
                maskBits = 0xff;
                break;

            case CPSS_DXCH_TABLE_STG_E:
                maskBits = 0x3fffff;
                break;

            case CPSS_DXCH_TABLE_VLAN_E:
                maskBits = 0xff7d7ffb;
                break;

            case CPSS_DXCH_TABLE_MULTICAST_E:
                maskBits = 0xfffffff;/* word1 */
                break;

            case CPSS_DXCH_TABLE_ROUTE_HA_MAC_SA_E:
                maskBits = 0xff;
                break;

            case CPSS_DXCH_TABLE_ROUTE_HA_ARP_DA_E:
                maskBits = 0xffff; /* word1 */
                break;

            case CPSS_DXCH_TABLE_FDB_E:
                maskBits = 0x3ff;
                break;

            default:
                maskBits = 0xffffffff;
                break;
        }

        return maskBits;
}



GT_STATUS readWriteEntryTest
(
    IN GT_U8                  devNum,
    IN CPSS_DXCH_TABLE_ENT tableType
)
{
    PRV_CPSS_DXCH_TABLES_INFO_STC *tableInfoPtr; /* pointer to table info */
    GT_U32 wordValue[CH_TABLE_ENTRY_MAX_WORDS_SIZE_CNS];
    GT_U32 readValue[CH_TABLE_ENTRY_MAX_WORDS_SIZE_CNS];
    GT_U32 wordNum;
    GT_U32 seed;
    GT_U32 entryNum;
    GT_STATUS rc;
#ifdef TBL_DEBUG_SIMULATION
    GT_STATUS i;
    GT_U32     trunkId;
    GT_U32     trunkMemberIdx;
    GT_U32 directReadValue[CH_TABLE_ENTRY_MAX_WORDS_SIZE_CNS];
    GT_U32 regAddress;
#endif /*TBL_DEBUG_SIMULATION*/


    tableInfoPtr = &(PRV_CPSS_DXCH_PP_MAC(devNum)->accessTableInfoPtr[tableType]);

    seed = 5;
    /*tableInfoPtr = PRV_CPSS_DXCH_TABLE_GET(devNum,tableType);*/
    cpssOsSrand(seed);


    for(entryNum = 0; entryNum < tableInfoPtr->maxNumOfEntries; entryNum++)
    {
        for(wordNum = 0; wordNum < tableInfoPtr->entrySize; wordNum++)
        {
            wordValue[wordNum] = cpssOsRand() & getMaskBits(tableType);
        }

        rc = coreCheetahWriteTableEntry(devNum, tableType, entryNum, wordValue);
        if(rc != GT_OK)
            return rc;

    }

    cpssOsSrand(seed);

    for(entryNum = 0; entryNum < tableInfoPtr->maxNumOfEntries; entryNum++)
    {
#ifdef TBL_DEBUG_SIMULATION
        switch(tableType)
        {
            case CPSS_DXCH_TABLE_VLAN_PORT_PROTOCOL_E:
                for (i = 0; i < tableInfoPtr->entrySize; i++)
                {
                  regAddress = SMEM_CHT_PROT_VLAN_QOS_TBL_ADDR(i, entryNum);
                  rc = prvCpssHwPpReadRegister(devNum, regAddress,
                                                      &directReadValue[i]);
                }
                break;

            case CPSS_DXCH_TABLE_PORT_VLAN_QOS_E:
                regAddress = SMEM_CHT_VLAN_QOS_TBL_ADDR(entryNum);
                break;

            case CPSS_DXCH_TABLE_TRUNK_MEMBERS_E:

                trunkId = (entryNum >> 3) & 0x7f;
                trunkMemberIdx = entryNum &  0x7;
                regAddress = SMEM_CHT_TRUNK_TBL_ADDR(trunkMemberIdx, trunkId);
                break;

            case CPSS_DXCH_TABLE_STATISTICAL_RATE_LIMIT_E:
                regAddress = SMEM_CHT_STAT_RATE_TBL_ADDR(entryNum);
                break;

            case CPSS_DXCH_TABLE_CPU_CODE_E:
                regAddress = SMEM_CHT_CPU_CODE_TBL_ADDR(entryNum);

                break;
            case CPSS_DXCH_TABLE_PCL_CONFIG_E:
                regAddress = SMEM_CHT_PCL_ID_TBL_ADDR(entryNum);
                break;

            case CPSS_DXCH_TABLE_QOS_PROFILE_E:
                regAddress = SMEM_CHT_QOS_TBL_ADDR(entryNum);
                break;

            case CPSS_DXCH_TABLE_REMARKING_E:
                regAddress = SMEM_CHT_POLICER_QOS_TBL_ADDR(entryNum);
                break;

            case CPSS_DXCH_TABLE_STG_E:
                regAddress = SMEM_CHT_STP_TBL_ADDR(entryNum);
                break;

            case CPSS_DXCH_TABLE_VLAN_E:
                regAddress = SMEM_CHT_VLAN_TBL_ADDR(entryNum);
                break;

            case CPSS_DXCH_TABLE_MULTICAST_E:
                regAddress = SMEM_CHT_MCST_TBL_ADDR(entryNum);
                break;

            case CPSS_DXCH_TABLE_ROUTE_HA_MAC_SA_E:
                regAddress = SMEM_CHT_MAC_SA_TBL_ADDR(entryNum);
                break;

            case CPSS_DXCH_TABLE_ROUTE_HA_ARP_DA_E:
                regAddress = SMEM_CHT_ARP_TBL_ADDR(entryNum);
                break;

            case CPSS_DXCH_TABLE_FDB_E:
                regAddress = SMEM_CHT_MAC_TBL_ADDR(entryNum);
                break;

            case CPSS_DXCH_TABLE_POLICER_E:
                regAddress = SMEM_CHT_POLICER_TBL_ADDR(entryNum);
                break;

            case CPSS_DXCH_TABLE_POLICER_COUNTERS_E:
                regAddress = SMEM_CHT_POLICER_CNT_TBL_ADDR(entryNum);
                break;

            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);

        }

        if (tableType != CPSS_DXCH_TABLE_VLAN_PORT_PROTOCOL_E)
        {
            rc = prvCpssHwPpReadRam(devNum, regAddress,  tableInfoPtr->entrySize,
                                                               directReadValue);
            if(rc != GT_OK)
                return rc;

        }
#endif  /*TBL_DEBUG_SIMULATION*/

        rc = coreCheetahReadTableEntry(devNum,tableType, entryNum, readValue);
        if(rc != GT_OK)
            return rc;

        for(wordNum = 0; wordNum < tableInfoPtr->entrySize; wordNum++)
        {
#ifdef TBL_DEBUG_SIMULATION

            if(readValue[wordNum] != directReadValue[wordNum])
            {
                cpssOsPrintf("\n Direct: %d = wordNum",wordNum);
                cpssOsPrintf("\n Direct: %d = entrydNum",entryNum);

            }

#endif  /*TBL_DEBUG_SIMULATION*/

            wordValue[wordNum] = cpssOsRand() & getMaskBits(tableType);

            if(readValue[wordNum] != wordValue[wordNum])
            {
                cpssOsPrintf("\n Indirect %d = wordNum",wordNum);
                cpssOsPrintf("\n Indirect %d = entrydNum",entryNum);

                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }

        }
    }
    readWriteFieldTest(devNum,tableType);
    return GT_OK;
}
#endif /*CHEETAH_TABLE_DEBUG*/


/**
* @internal lion2InitTable function
* @endinternal
*
* @brief   allocate and init the Lion2 tables info
*
* @retval GT_OK                    - on success.
* @retval GT_OUT_OF_CPU_MEM        - on malloc failure
*/
static GT_STATUS lion2InitTable
(
    void
)
{
    PRV_CPSS_DXCH_TABLES_INFO_STC   *tableInfoPtr;      /* pointer to current table info */
    PRV_CPSS_DXCH_TABLES_INFO_STC   *sharedTableInfoPtr;/* pointer to table info in shared memory */
    GT_U32  ii;
    GT_U32  index;

    if (PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(lion2TablesInfoPtr))
    {
        /* table is already set */
        return GT_OK;
    }

    /* allocate shared global table */
    sharedTableInfoPtr = cpssOsMalloc(sizeof(lion2TablesInfo));
    if (sharedTableInfoPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }

    PRV_SHARED_HW_INIT_TABLES_DB_VAR_SET(lion2TablesInfoPtr, sharedTableInfoPtr);

    /* copy content from constant DB */
    for(ii = 0 ; ii < lion2TableInfoSize ; ii++)
    {
        sharedTableInfoPtr[ii] = lion2TablesInfo[ii];
    }


    /* the lion2TablesInfo has xCat and xCat2 based info.
       override it with  Lion2 specific info. */
    for(ii = 0 ; ii < directAccessLion2TableInfoExtNumEntries ; ii++)
    {
        if( directAccessLion2TableInfoExt[ii].directAccessInfo.baseAddress ==
            SPECIAL_ADDRESS_NOT_MODIFIED_E)
        {
            /* no need to modify the entry (same as XCAT) */
            continue;
        }

        index = directAccessLion2TableInfoExt[ii].globalIndex;

        sharedTableInfoPtr[index].readTablePtr =
        sharedTableInfoPtr[index].writeTablePtr =
            &directAccessLion2TableInfoExt[ii].directAccessInfo;
    }

    /******************/
    /* set new tables */
    /******************/

    /* ingress VLAN */
    tableInfoPtr = &sharedTableInfoPtr[CPSS_DXCH_LION_TABLE_VLAN_INGRESS_E];
    tableInfoPtr->maxNumOfEntries = _4K;
    tableInfoPtr->entrySize = 5;

    /* egress VLAN */
    tableInfoPtr = &sharedTableInfoPtr[CPSS_DXCH_LION_TABLE_VLAN_EGRESS_E];
    tableInfoPtr->maxNumOfEntries = _4K;
    tableInfoPtr->entrySize = 9;

    /* ingress STG */
    tableInfoPtr = &sharedTableInfoPtr[CPSS_DXCH_LION_TABLE_STG_INGRESS_E];
    tableInfoPtr->maxNumOfEntries = 256;
    tableInfoPtr->entrySize = 4;

    /* egress STG */
    tableInfoPtr = &sharedTableInfoPtr[CPSS_DXCH_LION_TABLE_STG_EGRESS_E];
    tableInfoPtr->maxNumOfEntries = 256;
    tableInfoPtr->entrySize = 4;

    /* port isolation L2 */
    tableInfoPtr = &sharedTableInfoPtr[CPSS_DXCH_LION_TABLE_PORT_ISOLATION_L2_E];
    tableInfoPtr->maxNumOfEntries = _2K+128;
    tableInfoPtr->entrySize = 3;

    /* port isolation L3 */
    tableInfoPtr = &sharedTableInfoPtr[CPSS_DXCH_LION_TABLE_PORT_ISOLATION_L3_E];
    tableInfoPtr->maxNumOfEntries = _2K+128;
    tableInfoPtr->entrySize = 3;

    /* txq shaper per port per TC */
    tableInfoPtr = &sharedTableInfoPtr[CPSS_DXCH_LION_TABLE_TXQ_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG_E];
    tableInfoPtr->maxNumOfEntries = 16;/* local ports num */
    tableInfoPtr->entrySize = 14;

    /* txq shaper per port */
    tableInfoPtr = &sharedTableInfoPtr[CPSS_DXCH_LION_TABLE_TXQ_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG_E];
    tableInfoPtr->maxNumOfEntries = 16;/* local ports num */
    tableInfoPtr->entrySize = 2;


    /* txq sourceId members table */
    tableInfoPtr = &sharedTableInfoPtr[CPSS_DXCH_LION_TABLE_TXQ_SOURCE_ID_MEMBERS_E];
    tableInfoPtr->maxNumOfEntries = 32;/* source Id groups  */
    tableInfoPtr->entrySize = 2;

    /* txq non-trunk members table */
    tableInfoPtr = &sharedTableInfoPtr[CPSS_DXCH_LION_TABLE_TXQ_NON_TRUNK_MEMBERS_E];
    tableInfoPtr->maxNumOfEntries = 128;/* trunkId (include 0)  */
    tableInfoPtr->entrySize = 2;


    /* txq designated members table */
    tableInfoPtr = &sharedTableInfoPtr[CPSS_DXCH_LION_TABLE_TXQ_DESIGNATED_PORT_E];
    tableInfoPtr->maxNumOfEntries = 64;/* num entries  */
    tableInfoPtr->entrySize = 2;


    /* TXQ - egress STC table */
    tableInfoPtr = &sharedTableInfoPtr[CPSS_DXCH_LION_TABLE_TXQ_EGRESS_STC_E];
    tableInfoPtr->maxNumOfEntries = 16;/* local ports number !!! --
                                          but we will emulate index as 'global port' */
    tableInfoPtr->entrySize = 3;

    /* VLAN UC Rpf mode table */
    tableInfoPtr = &sharedTableInfoPtr[CPSS_DXCH_LION_TABLE_ROUTER_VLAN_URPF_STC_E];
    tableInfoPtr->maxNumOfEntries = 256;/* num entries  */
    tableInfoPtr->entrySize = 1;

    /* PCL unit - hash mask table (for CRC hashing for trunk) */
    tableInfoPtr = &sharedTableInfoPtr[CPSS_DXCH_LION_TABLE_TRUNK_HASH_MASK_CRC_E];
    tableInfoPtr->maxNumOfEntries = (BIT_4 + PCL_NUM_FLOWS_CNS); /* num entries = 4 bit index + 12 'flows' */
    tableInfoPtr->entrySize = 3;

    /***************************/
    /*set new tables like xcat2*/
    /***************************/
    /* IPCL lookup01 Configuration table */
    tableInfoPtr = &sharedTableInfoPtr[CPSS_DXCH_XCAT2_TABLE_INGRESS_PCL_LOOKUP01_CONFIG_E];
    tableInfoPtr->maxNumOfEntries = 4224;
    tableInfoPtr->entrySize = 2;
    tableInfoPtr->readAccessType = PRV_CPSS_DXCH_DIRECT_ACCESS_E;
    tableInfoPtr->writeAccessType = PRV_CPSS_DXCH_DIRECT_ACCESS_E;
    tableInfoPtr->readTablePtr =
        &directAccessXCAT2TableInfo[PRV_DXCH_XCAT2_TABLE_IPCL_LOOKUP01_CFG_E];
    tableInfoPtr->writeTablePtr =
        &directAccessXCAT2TableInfo[PRV_DXCH_XCAT2_TABLE_IPCL_LOOKUP01_CFG_E];


    /* IPCL lookup1 Configuration table changed base address */
    tableInfoPtr = &sharedTableInfoPtr[CPSS_DXCH_XCAT_TABLE_INGRESS_PCL_LOOKUP1_CONFIG_E];
    tableInfoPtr->maxNumOfEntries = 4224;
    tableInfoPtr->entrySize = 2;
    tableInfoPtr->readAccessType = PRV_CPSS_DXCH_DIRECT_ACCESS_E;
    tableInfoPtr->writeAccessType = PRV_CPSS_DXCH_DIRECT_ACCESS_E;
    tableInfoPtr->readTablePtr =
        &directAccessXCAT2TableInfo[PRV_DXCH_XCAT2_TABLE_IPCL_LOOKUP1_CFG_E];
    tableInfoPtr->writeTablePtr =
        &directAccessXCAT2TableInfo[PRV_DXCH_XCAT2_TABLE_IPCL_LOOKUP1_CFG_E];

    return GT_OK;
}


/**
* @internal bobcat2CopyTable function
* @endinternal
*
* @brief   Allocate and init the Bobcat2 table info DB
*
* @retval GT_OK                    - on success.
* @retval GT_OUT_OF_CPU_MEM        - on malloc failure
*/
static GT_STATUS bobcat2CopyTable
(
    void
)
{
    GT_U32                           ii;           /* iterator */
    GT_STATUS   rc;

    if (PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(bobcat2TablesInfoPtr) == NULL)
    {
        rc = allocTableInfo(bobcat2TableInfoSize,
            &PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(bobcat2TablesInfoPtr),
            directAccessBobcat2TableInfoSize,
            &PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(directAccessBobcat2TableInfoPtr));
        if(rc != GT_OK)
        {
            return rc;
        }

        {
            PRV_CPSS_DXCH_TABLES_INFO_DIRECT_UNIT_STC   *tableInfoPtr; /* pointer to table info */

            /* allocate shared global table */
            tableInfoPtr = PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(directAccessBobcat2TableInfoPtr);

            /* copy content from constant DB */
            for(ii = 0; (ii < directAccessBobcat2TableInfoSize); ii++)
            {
                tableInfoPtr[ii] = directAccessBobcat2TableInfo[ii];
            }
        }

        {
            PRV_CPSS_DXCH_TABLES_INFO_STC   *tableInfoPtr; /* pointer to table info */
            const GT_CHAR* startPtr = (const GT_CHAR*)directAccessBobcat2TableInfo;
            const GT_CHAR* endPtr = startPtr + sizeof(directAccessBobcat2TableInfo);

            /* allocate shared global table */
            tableInfoPtr = PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(bobcat2TablesInfoPtr);

            for(ii = 0; (ii < bobcat2TableInfoSize); ii++)
            {
                /* copy content from constant DB */
                tableInfoPtr[ii] = bobcat2TablesInfo[ii];

                /* update references to directAccessBobcat2TableInfo */
                if (((const GT_CHAR*)(tableInfoPtr[ii].readTablePtr) >= startPtr)
                    && ((const GT_CHAR*)(tableInfoPtr[ii].readTablePtr) < endPtr))
                {
                    tableInfoPtr[ii].readTablePtr = (GT_VOID*)
                        ((const GT_CHAR*)PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(directAccessBobcat2TableInfoPtr)
                         + ((const GT_CHAR*)(tableInfoPtr[ii].readTablePtr) - startPtr));
                }
                if (((const GT_CHAR *)(tableInfoPtr[ii].writeTablePtr) >= startPtr)
                    && ((const GT_CHAR*)(tableInfoPtr[ii].writeTablePtr) < endPtr))
                {
                    tableInfoPtr[ii].writeTablePtr = (GT_VOID*)
                        ((const GT_CHAR*)PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(directAccessBobcat2TableInfoPtr)
                         + ((const GT_CHAR*)(tableInfoPtr[ii].writeTablePtr) - startPtr));
                }
            }
        }
    }

    if (PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(bobcat2B0TablesInfo_overrideA0Ptr) == NULL)
    {
        PRV_CPSS_DXCH_TABLES_INFO_EXT_STC   *tableInfoPtr; /* pointer to table info */
        const GT_CHAR* startPtr = (const GT_CHAR*)directAccessBobcat2TableInfo;
        const GT_CHAR* endPtr = startPtr + sizeof(directAccessBobcat2TableInfo);

        /* allocate shared global table */
        tableInfoPtr = cpssOsMalloc(sizeof(bobcat2B0TablesInfo_overrideA0));
        if (tableInfoPtr == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        }

        PRV_SHARED_HW_INIT_TABLES_DB_VAR_SET(bobcat2B0TablesInfo_overrideA0Ptr, tableInfoPtr);

        for(ii = 0; (ii < bobcat2B0TablesInfo_overrideA0Size); ii++)
        {
            /* copy content from constant DB */
            tableInfoPtr[ii] = bobcat2B0TablesInfo_overrideA0[ii];

            /* update references to directAccessBobcat2TableInfo */
            if (((const GT_CHAR*)(tableInfoPtr[ii].tableInfo.readTablePtr) >= startPtr)
                && ((const GT_CHAR*)(tableInfoPtr[ii].tableInfo.readTablePtr) < endPtr))
            {
                tableInfoPtr[ii].tableInfo.readTablePtr = (GT_VOID*)
                    ((const GT_CHAR*)PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(directAccessBobcat2TableInfoPtr)
                 + ((const GT_CHAR *)(tableInfoPtr[ii].tableInfo.readTablePtr) - startPtr));
            }
            if (((const GT_CHAR *)(tableInfoPtr[ii].tableInfo.writeTablePtr) >= startPtr)
                && ((const GT_CHAR*)(tableInfoPtr[ii].tableInfo.writeTablePtr) < endPtr))
            {
                tableInfoPtr[ii].tableInfo.writeTablePtr = (GT_VOID*)
                    ((const GT_CHAR*)PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(directAccessBobcat2TableInfoPtr)
                     + ((const GT_CHAR*)(tableInfoPtr[ii].tableInfo.writeTablePtr) - startPtr));
            }
        }
    }

    return GT_OK;
}

GT_VOID initTablesFormatDb
(
  IN GT_U8  devNum
)
{
    PRV_CPSS_DXCH_PP_TABLE_FORMAT_INFO_STC * initTablesFormatPtr;

    initTablesFormatPtr = PRV_TABLE_FORMAT_INFO(devNum);

    /* TTI */
    BIND_FIELDS_FORMAT_TO_TABLE_MAC(initTablesFormatPtr ,
        PRV_CPSS_DXCH_TABLE_FORMAT_TTI_DEFAULT_EPORT_E ,
        prvCpssDxChSip5TtiDefaultEportTableFieldsFormat);

    BIND_FIELDS_FORMAT_TO_TABLE_MAC(initTablesFormatPtr ,
        PRV_CPSS_DXCH_TABLE_FORMAT_TTI_PHYSICAL_PORT_ATTRIBUTE_E ,
        prvCpssDxChSip5TtiPhysicalPortTableFieldsFormat);

    BIND_FIELDS_FORMAT_TO_TABLE_MAC(initTablesFormatPtr ,
        PRV_CPSS_DXCH_TABLE_FORMAT_TTI_EPORT_ATTRIBUTES_E ,
        prvCpssDxChSip5TtiEportAttributesTableFieldsFormat);


    BIND_FIELDS_FORMAT_TO_TABLE_MAC(initTablesFormatPtr ,
        PRV_CPSS_DXCH_TABLE_FORMAT_TTI_ACTION_E ,
        prvCpssDxChSip5TtiActionTableFieldsFormat);

    /* VLAN */
    BIND_FIELDS_FORMAT_TO_TABLE_MAC(initTablesFormatPtr ,
        PRV_CPSS_DXCH_TABLE_FORMAT_VLAN_PORT_PROTOCOL_E ,
        prvCpssDxChSip5TtiDefaultPortProtocolEvlanAndQosConfigTableFieldsFormat);

    BIND_FIELDS_FORMAT_TO_TABLE_MAC(initTablesFormatPtr ,
        PRV_CPSS_DXCH_TABLE_FORMAT_BRIDGE_INGRESS_EVLAN_E ,
        prvCpssDxChSip5L2iIngressVlanTableFieldsFormat);


    /* L2 */
    BIND_FIELDS_FORMAT_TO_TABLE_MAC(initTablesFormatPtr ,
        PRV_CPSS_DXCH_TABLE_FORMAT_BRIDGE_INGRESS_EPORT_E ,
        prvCpssDxChSip5L2iEPortTableFieldsFormat);

    BIND_FIELDS_FORMAT_TO_TABLE_MAC(initTablesFormatPtr ,
        PRV_CPSS_DXCH_TABLE_FORMAT_FDB_E ,
        prvCpssDxChSip5FdbFdbTableFieldsFormat);

    /* IP MLL */
    BIND_FIELDS_FORMAT_TO_TABLE_MAC(initTablesFormatPtr ,
        PRV_CPSS_DXCH_TABLE_FORMAT_IP_MLL_E ,
        prvCpssDxChSip5IpMllTableFieldsFormat);

    /* L2 MLL */
    BIND_FIELDS_FORMAT_TO_TABLE_MAC(initTablesFormatPtr ,
        PRV_CPSS_DXCH_TABLE_FORMAT_L2_MLL_E ,
        prvCpssDxChSip5L2MllTableFieldsFormat);

    /* EGF */
    BIND_FIELDS_FORMAT_TO_TABLE_MAC(initTablesFormatPtr ,
        PRV_CPSS_DXCH_TABLE_FORMAT_EGF_QAG_EGRESS_EPORT_E ,
        prvCpssDxChSip5EgfQagEgressEPortTableFieldsFormat);

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        /* set later inside initTablesDbSip6(...) , so no need to set it now */
    }
    else
        if(!PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
        {
            BIND_FIELDS_FORMAT_TO_TABLE_MAC(initTablesFormatPtr ,
                PRV_CPSS_DXCH_TABLE_FORMAT_EGF_SHT_EGRESS_EVLAN_E ,
                prvCpssDxChSip5EgfShtEgressEVlanTableFieldsFormat);
        }
        else
        {
            if(PRV_CPSS_DXCH_BMP_OF_TABLES_IN_HALF_SIZE_MODE_GET_MAC(devNum,CPSS_DXCH_LION_TABLE_VLAN_EGRESS_E))
            {
                BIND_FIELDS_FORMAT_TO_TABLE_MAC(initTablesFormatPtr ,
                    PRV_CPSS_DXCH_TABLE_FORMAT_EGF_SHT_EGRESS_EVLAN_E ,
                    prvCpssDxChSip5_20EgfShtEgressEVlanTableFieldsFormat_512Ports_mode);
            }
            else
            {
                BIND_FIELDS_FORMAT_TO_TABLE_MAC(initTablesFormatPtr ,
                    PRV_CPSS_DXCH_TABLE_FORMAT_EGF_SHT_EGRESS_EVLAN_E ,
                    prvCpssDxChSip5_20EgfShtEgressEVlanTableFieldsFormat);
            }
        }

        BIND_FIELDS_FORMAT_TO_TABLE_MAC(initTablesFormatPtr ,
            PRV_CPSS_DXCH_TABLE_FORMAT_EGF_QAG_TARGET_PORT_MAPPER_E ,
            prvCpssDxChSip5EgfQagTargetPortMapperTableFieldsFormat);

        if(!PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            BIND_FIELDS_FORMAT_TO_TABLE_MAC(initTablesFormatPtr ,
                PRV_CPSS_DXCH_TABLE_FORMAT_EGF_SHT_VLAN_ATTRIBUTES_E ,
                prvCpssDxChSip5EgfShtEgressEVlanAttributesTableFieldsFormat);
        }

        BIND_FIELDS_FORMAT_TO_TABLE_MAC(initTablesFormatPtr ,
            PRV_CPSS_DXCH_TABLE_FORMAT_EGF_SHT_EGRESS_EPORT_E ,
            prvCpssDxChSip5EgfShtEgressEPortTableFieldsFormat);

        /* HA */
        BIND_FIELDS_FORMAT_TO_TABLE_MAC(initTablesFormatPtr ,
            PRV_CPSS_DXCH_TABLE_FORMAT_HA_EGRESS_EPORT_1_E ,
            prvCpssDxChSip5HaEPort1TableFieldsFormat);

        BIND_FIELDS_FORMAT_TO_TABLE_MAC(initTablesFormatPtr ,
            PRV_CPSS_DXCH_TABLE_FORMAT_HA_EGRESS_EPORT_2_E ,
            prvCpssDxChSip5HaEPort2TableFieldsFormat);

        BIND_FIELDS_FORMAT_TO_TABLE_MAC(initTablesFormatPtr ,
            PRV_CPSS_DXCH_TABLE_FORMAT_HA_PHYSICAL_PORT_1_E ,
            prvCpssDxChSip5HaPhysicalPort1TableFieldsFormat);

        if(!PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            BIND_FIELDS_FORMAT_TO_TABLE_MAC(initTablesFormatPtr ,
                PRV_CPSS_DXCH_TABLE_FORMAT_HA_PHYSICAL_PORT_2_E ,
                prvCpssDxChSip5HaPhysicalPort2TableFieldsFormat);
        }

        /* TS  */
        BIND_FIELDS_FORMAT_TO_TABLE_MAC(initTablesFormatPtr ,
            PRV_CPSS_DXCH_TABLE_FORMAT_TUNNEL_START_E ,
            prvCpssDxChSip5HaTunnelStartTableFieldsFormat);
        if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            BIND_FIELDS_FORMAT_TO_TABLE_MAC(initTablesFormatPtr,
                PRV_CPSS_DXCH_TABLE_FORMAT_GENERIC_TUNNEL_START_E,
                prvCpssDxChSip6HaTunnelStartTableFieldsFormat);
        }

        BIND_FIELDS_FORMAT_TO_TABLE_MAC(initTablesFormatPtr ,
            PRV_CPSS_DXCH_TABLE_FORMAT_HA_GENERIC_TS_PROFILE_E ,
            prvCpssDxChSip5HaGenTunnelStartProfileTableFieldsFormat);

        /* PCL */
        BIND_FIELDS_FORMAT_TO_TABLE_MAC(initTablesFormatPtr ,
            PRV_CPSS_DXCH_TABLE_FORMAT_IPCL_ACTION_E ,
            prvCpssDxChSip5IpclActionTableFieldsFormat);

        BIND_FIELDS_FORMAT_TO_TABLE_MAC(initTablesFormatPtr ,
            PRV_CPSS_DXCH_TABLE_FORMAT_EPCL_ACTION_E ,
            prvCpssDxChSip5EpclActionTableFieldsFormat);

        BIND_FIELDS_FORMAT_TO_TABLE_MAC(initTablesFormatPtr ,
            PRV_CPSS_DXCH_TABLE_FORMAT_FDB_AU_MSG_E ,
            prvCpssDxChSip5FdbAuMsgTableFieldsFormat);

        BIND_FIELDS_FORMAT_TO_TABLE_MAC(initTablesFormatPtr ,
            PRV_CPSS_DXCH_TABLE_FORMAT_IPVX_ROUTER_NEXT_HOP_E ,
            prvCpssDxChSip5IpvxRouterNextHopTableFieldsFormat);


        BIND_FIELDS_FORMAT_TO_TABLE_MAC(initTablesFormatPtr ,
            PRV_CPSS_DXCH_TABLE_FORMAT_ROUTER_EPORT_E ,
            prvCpssDxChSip5IpvxRouterEportTableFieldsFormat);

        BIND_FIELDS_FORMAT_TO_TABLE_MAC(initTablesFormatPtr ,
            PRV_CPSS_DXCH_TABLE_FORMAT_PLR_METER_E ,
            prvCpssDxChSip5PlrMeteringTableFieldsFormat);

        if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
        {
            /* Override with BC2 B0 specific formats */
            BIND_FIELDS_FORMAT_TO_TABLE_MAC(initTablesFormatPtr ,
                PRV_CPSS_DXCH_TABLE_FORMAT_TTI_PHYSICAL_PORT_ATTRIBUTE_E ,
                prvCpssDxChBobcat2B0TtiPhysicalPortTableFieldsFormat);

            BIND_FIELDS_FORMAT_TO_TABLE_MAC(initTablesFormatPtr ,
                PRV_CPSS_DXCH_TABLE_FORMAT_BRIDGE_INGRESS_EVLAN_E ,
                prvCpssDxChBobcat2B0L2iIngressVlanTableFieldsFormat);

            BIND_FIELDS_FORMAT_TO_TABLE_MAC(initTablesFormatPtr ,
                PRV_CPSS_DXCH_TABLE_FORMAT_EGF_QAG_EGRESS_EPORT_E ,
                prvCpssDxChBobcat2B0EgfQagEgressEPortTableFieldsFormat);

            BIND_FIELDS_FORMAT_TO_TABLE_MAC(initTablesFormatPtr ,
                PRV_CPSS_DXCH_TABLE_FORMAT_HA_EGRESS_EPORT_1_E ,
                prvCpssDxChBobcat2B0HaEPort1TableFieldsFormat);


            BIND_FIELDS_FORMAT_TO_TABLE_MAC(initTablesFormatPtr ,
                PRV_CPSS_DXCH_TABLE_FORMAT_HA_EGRESS_EPORT_2_E ,
                prvCpssDxChBobcat2B0HaEPort2TableFieldsFormat);

            /* NAT44 - only in B0 */
            BIND_FIELDS_FORMAT_TO_TABLE_MAC(initTablesFormatPtr ,
                PRV_CPSS_DXCH_TABLE_FORMAT_NAT44_E ,
                prvCpssDxChSip5HaNat44TableFieldsFormat);

            BIND_FIELDS_FORMAT_TO_TABLE_MAC(initTablesFormatPtr ,
                PRV_CPSS_DXCH_TABLE_FORMAT_TTI_ACTION_E ,
                prvCpssDxChSip5_10TtiActionTableFieldsFormat);

        }

        if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
        {
            BIND_FIELDS_FORMAT_TO_TABLE_MAC(initTablesFormatPtr ,
                PRV_CPSS_DXCH_TABLE_FORMAT_HA_EGRESS_EPORT_1_E ,
                prvCpssDxChBobkHaEPort1TableFieldsFormat);

            BIND_FIELDS_FORMAT_TO_TABLE_MAC(initTablesFormatPtr ,
                PRV_CPSS_DXCH_TABLE_FORMAT_ROUTER_EPORT_E ,
                prvCpssDxChBobKIpvxRouterEportTableFieldsFormat);

            /* new table and format (in sip5_15) */
            BIND_FIELDS_FORMAT_TO_TABLE_MAC(initTablesFormatPtr ,
                PRV_CPSS_DXCH_TABLE_FORMAT_PLR_METER_CONFIG_E ,
                prvCpssDxChSip5_15PlrMeteringConfigurationTableFieldsFormat);

            /* new table and format (in sip5_15) */
            BIND_FIELDS_FORMAT_TO_TABLE_MAC(initTablesFormatPtr,
                PRV_CPSS_DXCH_TABLE_FORMAT_PLR_METER_CONFORM_SIGN_E,
                prvCpssDxChPlrMeteringConformanceSignTableFieldsFormat);

            if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                /* NAT66 - only in Falcon */
                BIND_FIELDS_FORMAT_TO_TABLE_MAC(initTablesFormatPtr ,
                    PRV_CPSS_DXCH_TABLE_FORMAT_NAT66_E ,
                    prvCpssDxChSip6HaNat66TableFieldsFormat);
            }
            else
            {
                /* NAT66 - only in BobK */
                BIND_FIELDS_FORMAT_TO_TABLE_MAC(initTablesFormatPtr ,
                    PRV_CPSS_DXCH_TABLE_FORMAT_NAT66_E ,
                    prvCpssDxChSip5HaNat66TableFieldsFormat);
            }
        }

        if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
        {
            BIND_FIELDS_FORMAT_TO_TABLE_MAC(initTablesFormatPtr ,
                PRV_CPSS_DXCH_TABLE_FORMAT_HA_EGRESS_EPORT_1_E ,
                prvCpssDxChSip5_20HaEPort1TableFieldsFormat);

            BIND_FIELDS_FORMAT_TO_TABLE_MAC(initTablesFormatPtr ,
                PRV_CPSS_DXCH_TABLE_FORMAT_EGF_QAG_PORT_TARGET_ATTRIBUTES_E ,
                prvCpssDxChSip5_20EgfQagPortTargetAttributesTableFieldsFormat);


            BIND_FIELDS_FORMAT_TO_TABLE_MAC(initTablesFormatPtr ,
                PRV_CPSS_DXCH_TABLE_FORMAT_EGF_QAG_PORT_SOURCE_ATTRIBUTES_E ,
                prvCpssDxChSip5_20EgfQagPortSourceAttributesTableFieldsFormat);

            BIND_FIELDS_FORMAT_TO_TABLE_MAC(initTablesFormatPtr ,
                PRV_CPSS_DXCH_TABLE_FORMAT_EPCL_ACTION_E ,
                prvCpssDxChSip5_20EpclActionTableFieldsFormat);

            BIND_FIELDS_FORMAT_TO_TABLE_MAC(initTablesFormatPtr ,
                PRV_CPSS_DXCH_TABLE_FORMAT_TTI_PHYSICAL_PORT_ATTRIBUTE_E,
                prvCpssDxChSip5_20TtiPhysicalPortTableFieldsFormat);

            BIND_FIELDS_FORMAT_TO_TABLE_MAC(initTablesFormatPtr ,
                PRV_CPSS_DXCH_TABLE_FORMAT_IPVX_ROUTER_NEXT_HOP_E ,
                prvCpssDxChSip5_20IpvxRouterNextHopTableFieldsFormat);

            BIND_FIELDS_FORMAT_TO_TABLE_MAC(initTablesFormatPtr ,
                PRV_CPSS_DXCH_TABLE_FORMAT_IP_MLL_E ,
                prvCpssDxChSip5_20IpMllTableFieldsFormat);

            BIND_FIELDS_FORMAT_TO_TABLE_MAC(initTablesFormatPtr ,
                PRV_CPSS_DXCH_TABLE_FORMAT_L2_MLL_E ,
                prvCpssDxChSip5_20L2MllTableFieldsFormat);

            BIND_FIELDS_FORMAT_TO_TABLE_MAC(initTablesFormatPtr ,
                PRV_CPSS_DXCH_TABLE_FORMAT_PLR_METER_CONFIG_E ,
                prvCpssDxChSip5_20PlrMeteringConfigurationTableFieldsFormat);

            BIND_FIELDS_FORMAT_TO_TABLE_MAC(initTablesFormatPtr ,
                PRV_CPSS_DXCH_TABLE_FORMAT_FDB_E ,
                prvCpssDxChSip5_20FdbFdbTableFieldsFormat);

            BIND_FIELDS_FORMAT_TO_TABLE_MAC(initTablesFormatPtr ,
                PRV_CPSS_DXCH_TABLE_FORMAT_FDB_AU_MSG_E ,
                prvCpssDxChSip5_20FdbAuMsgTableFieldsFormat);

            BIND_FIELDS_FORMAT_TO_TABLE_MAC(initTablesFormatPtr ,
                PRV_CPSS_DXCH_TABLE_FORMAT_IPCL_ACTION_E ,
                prvCpssDxChSip5_20IpclActionTableFieldsFormat);

            BIND_FIELDS_FORMAT_TO_TABLE_MAC(initTablesFormatPtr ,
                PRV_CPSS_DXCH_TABLE_FORMAT_TTI_ACTION_E ,
                prvCpssDxChSip5_20TtiActionTableFieldsFormat);


            BIND_FIELDS_FORMAT_TO_TABLE_MAC(initTablesFormatPtr ,
                PRV_CPSS_DXCH_TABLE_FORMAT_BRIDGE_INGRESS_EPORT_E ,
                prvCpssDxChSip5_20L2iEPortTableFieldsFormat);
        }
}

/**
* @internal initTablesDb function
* @endinternal
*
* @brief   init the Bobcat2, Caelum, Aldrin, AC3X, Bobcat3 tables info
*
* @param[in] devNum                   - device number
*                                       NONE
*/
static GT_STATUS initTablesDb
(
    IN GT_U8  devNum
)
{
    GT_U32  ii;
    GT_STATUS rc;

    CPSS_DXCH_TABLE_ENT  globalIndex;
    GT_U32                   localIndex;

    initTablesFormatDb(devNum);

    if (PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        bobcat2CopyTable();
    }

    if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        if (PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(bobkTablesInfoPtr) == NULL)
        {
            rc = allocTableInfo(bobkTableInfoSize,
                &PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(bobkTablesInfoPtr),
                directAccessBobkTableInfoSize,
                &PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(directAccessBobkTableInfoPtr));
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        /* call SIP6 to overload/add tables to initTablesFormatPtr */
        rc = initTablesDbSip6(devNum);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    if (PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
    {
        /* override the direct access info with B0 specifics */
        for(ii = 0;
            ii < NUM_ELEMENTS_IN_ARR_MAC(directAccessBobcat2B0TableInfo_overrideA0);
            ii++)
        {
            globalIndex = directAccessBobcat2B0TableInfo_overrideA0[ii].globalIndex;

            rc = directAccessGlobalIndexToLocalIndexConvert(globalIndex,
                PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(directAccessBobcat2TableInfoPtr),
                directAccessBobcat2TableInfoSize,
                &localIndex);/* index in directAccessBobcat2TableInfo */
            if(rc != GT_OK)
            {
                return rc;
            }

            /* override the direct access info with B0 specifics */
            PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(
                directAccessBobcat2TableInfoPtr)[localIndex].directAccessInfo =
                directAccessBobcat2B0TableInfo_overrideA0[ii].directAccessInfo;
        }

        /* override the tables info with B0 specifics (probably needed for entry width) */
        for(ii = 0;
            ii < bobcat2B0TablesInfo_overrideA0Size;
            ii++)
        {
            globalIndex = PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(bobcat2B0TablesInfo_overrideA0Ptr)[ii].globalIndex;

            /* the table MUST be 'globally' indexed !!!! --> no convert needed */
            localIndex = globalIndex;

            if(localIndex >= bobcat2TableInfoSize)
            {
                /* error in the DB !!! (do not cause access violation in the array) */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }

            /* override the direct access info with B0 specifics */
            PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(bobcat2TablesInfoPtr)[localIndex] =
                PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(bobcat2B0TablesInfo_overrideA0Ptr)[ii].tableInfo;
        }
    }

    return GT_OK;
}

/**
* @internal bobcat2InitTable function
* @endinternal
*
* @brief   init the Bobcat2 tables info
*
* @param[in] devNum                   - device number
*                                       NONE
*/
static GT_STATUS bobcat2InitTable
(
     IN GT_U8 devNum
)
{
    GT_STATUS rc;

    rc = initTablesDb(devNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* For Bobcat2 A0:
        Table:  Queue Descriptor Limits - Dequeue
                Entry width - 14 bit
                Line Address Alignment: 8 bytes
        Therefore macro BITS_TO_WORDS_MAC is wrong */
    if(!PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
    {
        /* override the direct access info with 40 bits instead of 14 - to get 8 bytes alignment */
        PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(directAccessBobcat2TableInfoPtr)[
            PRV_DXCH_BOBCAT2_TABLE_TXQ_QUEUE_TAIL_DROP_DQ_QUEUE_DESC_LIMITS_E].directAccessInfo.step =
            BITS_TO_BYTES_ALIGNMENT_MAC(40);
    }

    return GT_OK;
}

/**
* @internal bobkNewTablesInit function
* @endinternal
*
* @brief   init new tables
*/
static GT_STATUS bobkNewTablesInit
(
     void
)
{
    GT_U32  ii;
    /* new tables */
    ii = CPSS_DXCH_SIP5_15_TABLE_INGRESS_POLICER_0_METERING_CONFIG_E;
    PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(bobkTablesInfoPtr)[ii].maxNumOfEntries =
        BOBK_NUM_IPLR_METERS_MAC;
    PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(bobkTablesInfoPtr)[ii].entrySize =
        BITS_TO_WORDS_MAC(BOBK_POLICER_METERING_CONFIG_BITS_SIZE_CNS);

    /* the table is not accessible - use CPSS_DXCH_SIP5_15_TABLE_INGRESS_POLICER_0_METERING_CONFIG_E */
    ii = CPSS_DXCH_SIP5_15_TABLE_INGRESS_POLICER_1_METERING_CONFIG_E;
    PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(bobkTablesInfoPtr)[ii].maxNumOfEntries = 0;
    PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(bobkTablesInfoPtr)[ii].entrySize =
        BITS_TO_WORDS_MAC(BOBK_POLICER_METERING_CONFIG_BITS_SIZE_CNS);

    /* copy info from IPLR 1 , and update the difference */
    ii = CPSS_DXCH_SIP5_15_TABLE_EGRESS_POLICER_METERING_CONFIG_E;
    PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(bobkTablesInfoPtr)[ii].maxNumOfEntries =
        BOBK_NUM_EPLR_METERS_MAC;
    PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(bobkTablesInfoPtr)[ii].entrySize =
        BITS_TO_WORDS_MAC(BOBK_POLICER_METERING_CONFIG_BITS_SIZE_CNS);

    /* copy info from EPLR , and update the difference */
    ii = CPSS_DXCH_SIP5_15_TABLE_INGRESS_POLICER_0_METERING_CONFORM_SIGN_E;
    PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(bobkTablesInfoPtr)[ii].maxNumOfEntries =
        BOBK_NUM_IPLR_METERS_MAC;
    PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(bobkTablesInfoPtr)[ii].entrySize =
        BITS_TO_WORDS_MAC(BOBK_POLICER_METERING_CONFORM_SIGN_BITS_SIZE_CNS);

    /* copy info from IPLR 1 , and update the difference */
    ii = CPSS_DXCH_SIP5_15_TABLE_INGRESS_POLICER_1_METERING_CONFORM_SIGN_E;
    PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(bobkTablesInfoPtr)[ii].maxNumOfEntries =
        BOBK_NUM_IPLR_METERS_MAC;
    PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(bobkTablesInfoPtr)[ii].entrySize =
        BITS_TO_WORDS_MAC(BOBK_POLICER_METERING_CONFORM_SIGN_BITS_SIZE_CNS);

    /* copy info from EPLR 0 , and update the difference */
    ii = CPSS_DXCH_SIP5_15_TABLE_EGRESS_POLICER_METERING_CONFORM_SIGN_E;
    PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(bobkTablesInfoPtr)[ii].maxNumOfEntries =
        BOBK_NUM_EPLR_METERS_MAC;
    PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(bobkTablesInfoPtr)[ii].entrySize =
        BITS_TO_WORDS_MAC(BOBK_POLICER_METERING_CONFORM_SIGN_BITS_SIZE_CNS);

    return GT_OK;
}
/**
* @internal bobkInitTableSpecifics function
* @endinternal
*
* @brief   update the Bobk specific tables info
*/
static GT_STATUS bobkInitTableSpecifics
(
     GT_VOID
)
{
    CPSS_DXCH_TABLE_ENT  globalIndex;
    GT_U32                   localIndex;
    GT_U32  ii,jj;
    PRV_CPSS_DXCH_TABLES_INFO_STC *tblInfoPtr;
    const PRV_CPSS_DXCH_TABLES_INFO_STC *tblInfoOverridePtr;
    PRV_CPSS_DXCH_TABLES_INFO_STC   *tableInfoPtr;/* pointer to current table info */
    GT_U32  lastIndexIn_directAccessBobcat2TableInfo =
        (directAccessBobcat2TableInfoSize - 1);
    GT_U32  offsetIn_directAccessBobcat2TableInfo;
    const PRV_CPSS_DXCH_TABLES_INFO_DIRECT_UNIT_STC  *directAccessPtr;
    GT_U32 baseAddr;/* table base address */

    if (PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(directAccessBobkTableInfo_initDone) == GT_TRUE)
    {
        /* all operations are 'single time' on the filling the common bobk info
           and not 'per device' */
        return GT_OK;
    }

    PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(directAccessBobkTableInfo_initDone) = GT_TRUE;

    /* init the tables info with bc2 B0 specifics */
    for(ii = 0;
        ii < NUM_ELEMENTS_IN_ARR_MAC(bobcat2TablesInfo);
        ii++)
    {
        /* copy all elements from bobcat2TablesInfo */
        PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(bobkTablesInfoPtr)[ii] =
            PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(bobcat2TablesInfoPtr)[ii];
    }


    /* init the direct access info with bc2 B0 specifics */
    for(ii = 0;
        ii < directAccessBobcat2TableInfoSize;
        ii++)
    {
        /* copy all elements from directAccessBobcat2TableInfo */
        PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(directAccessBobkTableInfoPtr)[ii] =
            PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(directAccessBobcat2TableInfoPtr)[ii];
    }


    /* add new entries that not exists in bobcat2 */
    /* bind 'direct table info' to the 'table info' */
    for(ii = 0;
        ii < NUM_ELEMENTS_IN_ARR_MAC(directAccessBobkTableInfo_new);
        ii++)
    {
        localIndex = directAccessBobkTableInfo_new[ii].newIndex;

        PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(directAccessBobkTableInfoPtr)[localIndex] =
            directAccessBobkTableInfo_new[ii].directAccessInfo;

        /* bind 'direct table info' to the 'table info' */
        globalIndex =
            PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(directAccessBobkTableInfoPtr)[localIndex].globalIndex;

        PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(bobkTablesInfoPtr)[globalIndex].readAccessType =
        PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(bobkTablesInfoPtr)[globalIndex].writeAccessType =
            PRV_CPSS_DXCH_DIRECT_ACCESS_E;
        PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(bobkTablesInfoPtr)[globalIndex].readTablePtr =
        PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(bobkTablesInfoPtr)[globalIndex].writeTablePtr =
            &PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(directAccessBobkTableInfoPtr)[
                localIndex].directAccessInfo;
    }

    /* override all indications of directAccessBobcat2TableInfo with
       directAccessBobkTableInfo */
    for(ii = 0; ii < bobkTableInfoSize; ii++)
    {
        tableInfoPtr = &PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(bobkTablesInfoPtr)[ii];

        if(tableInfoPtr->readAccessType == PRV_CPSS_DXCH_DIRECT_ACCESS_E &&
           tableInfoPtr->readTablePtr != NULL)
        {
            /* calculate the offset of readTablePtr in directAccessBobcat2TableInfo
               and replace it with offset in directAccessBobkTableInfo */
            directAccessPtr = tableInfoPtr->readTablePtr;

            /* handle the 'read' part */
            if(directAccessPtr >=
               &PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(directAccessBobcat2TableInfoPtr)[0] &&
               directAccessPtr <=
               &PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(directAccessBobcat2TableInfoPtr)[
                    lastIndexIn_directAccessBobcat2TableInfo+1])
            {
                offsetIn_directAccessBobcat2TableInfo =
                    (GT_U32)((GT_CHAR*)directAccessPtr -
                             (GT_CHAR*)PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(directAccessBobcat2TableInfoPtr));
                offsetIn_directAccessBobcat2TableInfo /= sizeof(PRV_CPSS_DXCH_TABLES_INFO_DIRECT_UNIT_STC);

                tableInfoPtr->readTablePtr =
                    &PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(directAccessBobkTableInfoPtr)[
                        offsetIn_directAccessBobcat2TableInfo].directAccessInfo;
            }
        }

        if(tableInfoPtr->writeAccessType == PRV_CPSS_DXCH_DIRECT_ACCESS_E &&
           tableInfoPtr->writeTablePtr != NULL)
        {
            /* calculate the offset of writeTablePtr in directAccessBobcat2TableInfo
               and replace it with offset in directAccessBobkTableInfo */
            directAccessPtr = tableInfoPtr->writeTablePtr;

            /* handle the 'write' part */
            if(directAccessPtr >= &PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(directAccessBobcat2TableInfoPtr)[0] &&
               directAccessPtr <= &PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(directAccessBobcat2TableInfoPtr)[
                    lastIndexIn_directAccessBobcat2TableInfo+1])
            {
                offsetIn_directAccessBobcat2TableInfo =
                    (GT_U32)((GT_CHAR*)directAccessPtr -
                             (GT_CHAR*)PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(directAccessBobcat2TableInfoPtr));
                offsetIn_directAccessBobcat2TableInfo /= sizeof(PRV_CPSS_DXCH_TABLES_INFO_DIRECT_UNIT_STC);

                tableInfoPtr->writeTablePtr =
                    &PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(directAccessBobkTableInfoPtr)[
                        offsetIn_directAccessBobcat2TableInfo].directAccessInfo;
            }
        }
    }


    /* override the direct access info with bobk specifics */
    for(ii = 0;
        ii < NUM_ELEMENTS_IN_ARR_MAC(bobkTablesInfo_overrideBobcat2);
        ii++)
    {
        globalIndex = bobkTablesInfo_overrideBobcat2[ii].globalIndex;

        localIndex = globalIndex;

        /* override the direct access info with bobk specifics */
        tblInfoPtr = &PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(bobkTablesInfoPtr)[localIndex];
        tblInfoOverridePtr = &bobkTablesInfo_overrideBobcat2[ii].tableInfo;

        OVERRIDE_DIRECT_INFO_IF_MODIFIED_MAC(tblInfoOverridePtr,tblInfoPtr,maxNumOfEntries);
        OVERRIDE_DIRECT_INFO_IF_MODIFIED_MAC(tblInfoOverridePtr,tblInfoPtr,entrySize);
        OVERRIDE_DIRECT_INFO_IF_MODIFIED_MAC(tblInfoOverridePtr,tblInfoPtr,readAccessType);
        OVERRIDE_DIRECT_INFO_IF_MODIFIED_POINTER_MAC(tblInfoOverridePtr,tblInfoPtr,readTablePtr);
        OVERRIDE_DIRECT_INFO_IF_MODIFIED_MAC(tblInfoOverridePtr,tblInfoPtr,writeAccessType);
        OVERRIDE_DIRECT_INFO_IF_MODIFIED_POINTER_MAC(tblInfoOverridePtr,tblInfoPtr,writeTablePtr);
    }

    for(ii = 0;
        ii < NUM_ELEMENTS_IN_ARR_MAC(directAccessBobkTableInfo_overrideBobcat2);
        ii++)
    {
        globalIndex = directAccessBobkTableInfo_overrideBobcat2[ii].globalIndex;

        /* look for this global index in directAccessBobkTableInfo[] */
        for(jj = 0 ;
            jj < directAccessBobkTableInfoSize;
            jj++)
        {
            if (PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(directAccessBobkTableInfoPtr)[
                jj].globalIndex == globalIndex)
            {
                if (NOT_MODIFIED_VALUE_CNS ==
                    directAccessBobkTableInfo_overrideBobcat2[ii].directAccessInfo.baseAddress)
                {
                    /* keep the original base address */
                    baseAddr = PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(directAccessBobkTableInfoPtr)[
                        jj].directAccessInfo.baseAddress;
                }
                else
                {
                    /* got new base address than this of bc2 */
                    baseAddr = directAccessBobkTableInfo_overrideBobcat2[ii].directAccessInfo.baseAddress;
                }

                PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(directAccessBobkTableInfoPtr)[
                    jj].directAccessInfo =
                    directAccessBobkTableInfo_overrideBobcat2[ii].directAccessInfo;
                /* set proper base address */
                PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(directAccessBobkTableInfoPtr)[
                    jj].directAccessInfo.baseAddress = baseAddr;

                break;
            }
        }
    }

    return bobkNewTablesInit();
}
/**
* @internal bobkInitTable function
* @endinternal
*
* @brief   init the Bobk tables info
*
* @param[in] devNum                   - device number
*                                       NONE
*/
static GT_STATUS bobkInitTable
(
     IN GT_U8 devNum
)
{
    GT_STATUS rc;

    /* most tables are like in bobcat2 ... */
    rc = bobcat2InitTable(devNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* do specific bobk adaptations */
    rc = bobkInitTableSpecifics();/* not 'per device' */
    if(rc != GT_OK)
    {
        return rc;
    }

    if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E) ||
        (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E))
    {
        GT_U32  ii;
        GT_U32  globalIndex,localIndex;
        PRV_CPSS_DXCH_TABLES_INFO_STC *tblInfoPtr;
        const PRV_CPSS_DXCH_TABLES_INFO_STC *tblInfoOverridePtr;


        /* override the direct access info with Aldrin specifics */
        for(ii = 0;
            ii < NUM_ELEMENTS_IN_ARR_MAC(aldrinTablesInfo_overrideBobk);
            ii++)
        {
            globalIndex = aldrinTablesInfo_overrideBobk[ii].globalIndex;

            localIndex = globalIndex;

            if(localIndex >= bobkTableInfoSize)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "trying to access out of range");
            }

            /* override the direct access info with bobk specifics */
            tblInfoPtr = &PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(bobkTablesInfoPtr)[localIndex];
            tblInfoOverridePtr = &aldrinTablesInfo_overrideBobk[ii].tableInfo;

            OVERRIDE_DIRECT_INFO_IF_MODIFIED_MAC(tblInfoOverridePtr,tblInfoPtr,maxNumOfEntries);
            OVERRIDE_DIRECT_INFO_IF_MODIFIED_MAC(tblInfoOverridePtr,tblInfoPtr,entrySize);
            OVERRIDE_DIRECT_INFO_IF_MODIFIED_MAC(tblInfoOverridePtr,tblInfoPtr,readAccessType);
            OVERRIDE_DIRECT_INFO_IF_MODIFIED_POINTER_MAC(tblInfoOverridePtr,tblInfoPtr,readTablePtr);
            OVERRIDE_DIRECT_INFO_IF_MODIFIED_MAC(tblInfoOverridePtr,tblInfoPtr,writeAccessType);
            OVERRIDE_DIRECT_INFO_IF_MODIFIED_POINTER_MAC(tblInfoOverridePtr,tblInfoPtr,writeTablePtr);
        }

    }


    return GT_OK;
}


/**
* @internal genericInitTableSpecifics function
* @endinternal
*
* @brief   generic update of specific tables info , to fill - devTablesInfo , directAccessTableInfo
*
* @note   APPLICABLE DEVICES:       Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X;
*
* @param[in] devNum                   - device number
* @param[in] prevDevSpecialIndication - special devNum indication for understand the offset
*                                      of table from the start of the unit ... for 'large' units.
* @param[in] numEntriesInfo[]         - (pointer to) hold number of entries for parametric tables.
*                                      -- general info --
* @param[in] prevDevTablesInfo[]      - number of elements in prevDevTablesInfo
* @param[in] prevDevTablesInfo[]      - (pointer to) previous device to copy 'general' info from.
* @param[in] devTablesInfo[]          - number of elements in devTablesInfo_override
* @param[in] devTablesInfo[]          - (pointer to) 'general' info for override info copied from previous device.
*                                      -- direct access --
* @param[in] prevDevDirectAccessTableInfo[] - number of elements in prevDevDirectAccessTableInfo
* @param[in] prevDevDirectAccessTableInfo[] - (pointer to) previous device to copy 'direct access' info from.
* @param[in] directAccessTableInfo[]  - number of elements in directAccessTableInfo_override
* @param[in] directAccessTableInfo[]  - (pointer to) 'direct access' info for override info copied from previous device.
* @param[in] directAccessTableInfo[]  - number of elements in directAccessTableInfo_new
* @param[in] directAccessTableInfo[]  - (pointer to) 'direct access' info for new info that not exists in previous device.
*                                      -- indirect access --
* @param[in] numElements_bindIndirect - number of elements in bindIndirect
* @param[in] bindIndirect[]           - (pointer to) 'indirect access' connection to DB.
*                                      ---
* @param[in] devTablesInfo[]          - number of elements in devTablesInfo
* @param[in] directAccessTableInfo[]  - number of elements in directAccessTableInfo
*
* @param[out] devTablesInfo[]          - (pointer to) 'general' info for the device.
* @param[out] directAccessTableInfo[]  - (pointer to) 'direct access' info for the device.
*                                       NONE
*/
static GT_STATUS genericInitTableSpecifics
(
    IN GT_U8 devNum,
    IN GT_U32 prevDevSpecialIndication,

    IN const TABLE_NUM_ENTRIES_INFO_STC   numEntriesInfo[],

    IN GT_U32 numElements_prevDevTablesInfo,
    IN const PRV_CPSS_DXCH_TABLES_INFO_STC prevDevTablesInfo[],

    IN GT_U32 numElements_devTablesInfo_override,
    IN const PRV_CPSS_DXCH_TABLES_INFO_EXT_STC        devTablesInfo_override[],

    IN GT_U32 numElements_prevDevDirectAccessTableInfo,
    IN const PRV_CPSS_DXCH_TABLES_INFO_DIRECT_UNIT_STC prevDevDirectAccessTableInfo[],

    IN GT_U32 numElements_directAccessTableInfo_override,
    IN const PRV_CPSS_DXCH_TABLES_INFO_DIRECT_EXT_STC directAccessTableInfo_override[],

    IN GT_U32 numElements_directAccessTableInfo_new,
    IN const BOBK_TABLES_INFO_DIRECT_NEW_INDEX_STC directAccessTableInfo_new[],

    IN GT_U32 numElements_bindIndirect,
    IN const PRV_CPSS_DXCH_TABLES_INFO_INDIRECT_STC bindIndirect[],

    IN GT_U32 numElements_devTablesInfo,
    OUT PRV_CPSS_DXCH_TABLES_INFO_STC devTablesInfo[],

    IN GT_U32 numElements_directAccessTableInfo,
    OUT PRV_CPSS_DXCH_TABLES_INFO_DIRECT_UNIT_STC directAccessTableInfo[]
)
{
    CPSS_DXCH_TABLE_ENT  globalIndex;
    GT_U32                   localIndex;
    GT_U32  ii,jj;
    PRV_CPSS_DXCH_TABLES_INFO_STC *tblInfoPtr;
    const PRV_CPSS_DXCH_TABLES_INFO_STC *tblInfoOverridePtr;
    PRV_CPSS_DXCH_TABLES_INFO_STC   *tableInfoPtr;/* pointer to current table info */
    GT_U32  lastIndexIn_directAccessTableInfo = numElements_prevDevDirectAccessTableInfo;
    GT_U32  offsetIn_directAccessTableInfo;
    const PRV_CPSS_DXCH_TABLES_INFO_DIRECT_UNIT_STC  *directAccessPtr;
    GT_U32 numOfEntries;/* number of entries in a table */
    PRV_CPSS_DXCH_UNIT_ENT unitIndex;/* unit index */
    GT_U32  unitBaseAddr;/* base address of the unit in the device */
    GT_U32  unitBaseAddr_prevDev;/* base address of the unit in the 'prev device' */
    GT_BOOL     didError;/* indication for error */
    GT_BOOL     found;/* found indication */

    if(GET_TABLES_DB_DONE_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        /* not needed more than once for this family. */
        return GT_OK;
    }

    /* init the tables info with previous specifics */
    for(ii = 0;
        ii < numElements_prevDevTablesInfo;
        ii++)
    {
        /* copy all elements from prevDevTablesInfo */
        devTablesInfo[ii] = prevDevTablesInfo[ii];
    }


    /* init the direct access info with device specifics */
    for(ii = 0;
        ii < numElements_prevDevDirectAccessTableInfo;
        ii++)
    {
        /* copy all elements from directAccessTableInfo */
        directAccessTableInfo[ii] = prevDevDirectAccessTableInfo[ii];
    }

    /* add new entries that not exists in previous */
    /* bind 'direct table info' to the 'table info' */
    for(ii = 0;
        ii < numElements_directAccessTableInfo_new;
        ii++)
    {
        localIndex = directAccessTableInfo_new[ii].newIndex;
        if(localIndex == PRV_DXCH_DEVICE_TABLE___LAST_E)
        {
            /* just place holder*/
            continue;
        }

        directAccessTableInfo[localIndex] =
            directAccessTableInfo_new[ii].directAccessInfo;

        /* bind 'direct table info' to the 'table info' */
        globalIndex = directAccessTableInfo[localIndex].globalIndex;

        devTablesInfo[globalIndex].readAccessType =
        devTablesInfo[globalIndex].writeAccessType = PRV_CPSS_DXCH_DIRECT_ACCESS_E;
        devTablesInfo[globalIndex].readTablePtr =
        devTablesInfo[globalIndex].writeTablePtr = &directAccessTableInfo[localIndex].directAccessInfo;


        if (NOT_MODIFIED_VALUE_CNS ==
            directAccessTableInfo[localIndex].directAccessInfo.nextWordOffset)
        {
            directAccessTableInfo[localIndex].directAccessInfo.nextWordOffset = PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS;
        }

        if(directAccessTableInfo[localIndex].directAccessInfo.step == 0xFFFFFFFF)
        {
            /* default value */
            directAccessTableInfo[localIndex].directAccessInfo.step = 4;
        }

        /* calculate entry size from 'alignment' of the table <step> */
        devTablesInfo[globalIndex].entrySize =
            directAccessTableInfo[localIndex].directAccessInfo.step / 4;
    }

    /* override all indications of prevDevDirectAccessTableInfo with
        directAccessTableInfo */
    for(ii = 0; ii <  numElements_devTablesInfo; ii++)
    {
        tableInfoPtr = &devTablesInfo[ii];

        if(tableInfoPtr->readAccessType == PRV_CPSS_DXCH_DIRECT_ACCESS_E &&
           tableInfoPtr->readTablePtr != NULL)
        {
            /* calculate the offset of readTablePtr in prevDevDirectAccessTableInfo
               and replace it with offset in directAccessTableInfo */
            directAccessPtr = tableInfoPtr->readTablePtr;

            /* handle the 'read' part */
            if(directAccessPtr >= &prevDevDirectAccessTableInfo[0] &&
               directAccessPtr < &prevDevDirectAccessTableInfo[lastIndexIn_directAccessTableInfo+1])
            {
                offsetIn_directAccessTableInfo =
                    (GT_U32)((GT_CHAR*)directAccessPtr - (GT_CHAR*)prevDevDirectAccessTableInfo);
                offsetIn_directAccessTableInfo /= sizeof(PRV_CPSS_DXCH_TABLES_INFO_DIRECT_UNIT_STC);

                tableInfoPtr->readTablePtr = &directAccessTableInfo[offsetIn_directAccessTableInfo].directAccessInfo;
            }
        }

        if(tableInfoPtr->writeAccessType == PRV_CPSS_DXCH_DIRECT_ACCESS_E &&
           tableInfoPtr->writeTablePtr != NULL)
        {
            /* calculate the offset of writeTablePtr in prevDevDirectAccessTableInfo
               and replace it with offset in directAccessTableInfo */
            directAccessPtr = tableInfoPtr->writeTablePtr;

            /* handle the 'write' part */
            if(directAccessPtr >= &prevDevDirectAccessTableInfo[0] &&
               directAccessPtr < &prevDevDirectAccessTableInfo[lastIndexIn_directAccessTableInfo+1])
            {
                offsetIn_directAccessTableInfo =
                    (GT_U32)((GT_CHAR*)directAccessPtr - (GT_CHAR*)prevDevDirectAccessTableInfo);
                offsetIn_directAccessTableInfo /= sizeof(PRV_CPSS_DXCH_TABLES_INFO_DIRECT_UNIT_STC);

                tableInfoPtr->writeTablePtr = &directAccessTableInfo[offsetIn_directAccessTableInfo].directAccessInfo;
            }
        }
    }


    /* override the direct access info with bobcat3 specifics */
    for(ii = 0;
        ii < numElements_devTablesInfo_override;
        ii++)
    {
        globalIndex = devTablesInfo_override[ii].globalIndex;
        if(globalIndex == CPSS_DXCH_TABLE_LAST_E)
        {
            /* place holder */
            continue;
        }

        localIndex = globalIndex;

        /* override the direct access info with device specifics */
        tblInfoPtr = &devTablesInfo[localIndex];
        tblInfoOverridePtr = &devTablesInfo_override[ii].tableInfo;


        /* maxNumOfEntries field overrinen below */
        /*OVERRIDE_DIRECT_INFO_IF_MODIFIED_MAC(tblInfoOverridePtr,tblInfoPtr,maxNumOfEntries);*/
        OVERRIDE_DIRECT_INFO_IF_MODIFIED_MAC(tblInfoOverridePtr,tblInfoPtr,entrySize);
        OVERRIDE_DIRECT_INFO_IF_MODIFIED_MAC(tblInfoOverridePtr,tblInfoPtr,readAccessType);
        OVERRIDE_DIRECT_INFO_IF_MODIFIED_POINTER_MAC(tblInfoOverridePtr,tblInfoPtr,readTablePtr);
        OVERRIDE_DIRECT_INFO_IF_MODIFIED_MAC(tblInfoOverridePtr,tblInfoPtr,writeAccessType);
        OVERRIDE_DIRECT_INFO_IF_MODIFIED_POINTER_MAC(tblInfoOverridePtr,tblInfoPtr,writeTablePtr);

        if(tblInfoOverridePtr->entrySize != NOT_MODIFIED_VALUE_CNS)
        {
           if(tblInfoPtr->writeAccessType == PRV_CPSS_DXCH_DIRECT_ACCESS_E)
            {
                /* auto calculate the info for alignment */
                ((PRV_CPSS_DXCH_TABLES_INFO_DIRECT_STC*)
                    tblInfoPtr->writeTablePtr)->step =
                        BITS_TO_BYTES_ALIGNMENT_MAC(tblInfoOverridePtr->entrySize * 32);
            }
            else
            if(tblInfoPtr->readAccessType == PRV_CPSS_DXCH_DIRECT_ACCESS_E)
            {
                /* auto calculate the info for alignment */
                ((PRV_CPSS_DXCH_TABLES_INFO_DIRECT_STC*)
                    tblInfoPtr->readTablePtr)->step =
                        BITS_TO_BYTES_ALIGNMENT_MAC(tblInfoOverridePtr->entrySize * 32);
            }
        }

    }

    for(ii = 0;
        ii < numElements_directAccessTableInfo_override;
        ii++)
    {
        globalIndex = directAccessTableInfo_override[ii].globalIndex;
        if(globalIndex == CPSS_DXCH_TABLE_LAST_E)
        {
            /* just place holder*/
            continue;
        }

        /* look for this global index in directAccessTableInfo[] */
        for(jj = 0 ;
            jj < numElements_directAccessTableInfo;
            jj++)
        {
            if(directAccessTableInfo[jj].globalIndex != globalIndex)
            {
                continue;
            }

            if (NOT_MODIFIED_VALUE_CNS !=
                directAccessTableInfo_override[ii].directAccessInfo.baseAddress)
            {
                /* got new base address than this of prev device */
                directAccessTableInfo[jj].directAccessInfo.baseAddress =
                    directAccessTableInfo_override[ii].directAccessInfo.baseAddress;
                /* indicate that address did not get yet the unit base address */
                directAccessTableInfo[jj].initWasDone = GT_FALSE;
            }

            if (NOT_MODIFIED_VALUE_CNS !=
                directAccessTableInfo_override[ii].directAccessInfo.step)
            {
                /* got new step than this of prev device */
                directAccessTableInfo[jj].directAccessInfo.step =
                    directAccessTableInfo_override[ii].directAccessInfo.step;
            }

            if (NOT_MODIFIED_VALUE_CNS !=
                directAccessTableInfo_override[ii].directAccessInfo.nextWordOffset)
            {
                /* got new nextWordOffset than this of prev device */
                directAccessTableInfo[jj].directAccessInfo.nextWordOffset =
                    directAccessTableInfo_override[ii].directAccessInfo.nextWordOffset;
            }

            break;
        }
    }

    /* devTablesInfo[globalIndex].maxNumOfEntries calculated upper */
    /* if it is overriden it must be fixed here                    */
    for(ii = 0;
        ii < numElements_devTablesInfo_override;
        ii++)
    {
        globalIndex = devTablesInfo_override[ii].globalIndex;
        if (globalIndex == CPSS_DXCH_TABLE_LAST_E)
        {
            /* place holder */
            continue;
        }

        /* override the direct access info with device specifics */
        tblInfoPtr = &devTablesInfo[globalIndex];
        tblInfoOverridePtr = &devTablesInfo_override[ii].tableInfo;

        OVERRIDE_DIRECT_INFO_IF_MODIFIED_MAC(tblInfoOverridePtr,tblInfoPtr, maxNumOfEntries);
    }

    /* set proper addresses according to the actual addresses of the units in the device */
    for(ii = 0 ;
        ii < numElements_directAccessTableInfo;
        ii++)
    {
        /* need to skip tables that already aligned the addresses */
        if(directAccessTableInfo[ii].initWasDone == GT_TRUE)
        {
            /* The sip6 address can not be '|' and must use '+' so we can't add base
               addresses more than single time */
            continue;
        }
        /* indicate that the table is doing base address alignment */
        directAccessTableInfo[ii].initWasDone = GT_TRUE;

        unitIndex = directAccessTableInfo[ii].unitIndex;
        unitBaseAddr = prvCpssDxChHwUnitBaseAddrGet(devNum,unitIndex,&didError);

        if (1 == (unitBaseAddr & 1))
        {
            /* the unit is not valid for the device .... allow it. */
            /* set the table with 'bad address'  ... that will cause fatal error
               on simulation */
            directAccessTableInfo[ii].directAccessInfo.baseAddress =
                SPECIAL_ADDRESS_NOT_EXISTS_E;
        }
        else
        {
            if(didError == GT_TRUE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }

            switch(unitIndex)
            {
                case PRV_CPSS_DXCH_UNIT_EGF_SHT_E:

                    found = GT_FALSE;
                    /* special treat for entries that exists in 'directAccessTableInfo_new' */
                    for(jj = 0; jj < numElements_directAccessTableInfo_new ; jj++)
                    {
                        if(directAccessTableInfo_new[jj].directAccessInfo.globalIndex ==
                           directAccessTableInfo[ii].globalIndex)
                        {
                            found = GT_TRUE;
                            break;
                        }
                    }

                    if(found == GT_FALSE)
                    {
                        /* special treat for entries that exists in 'directAccessTableInfo_override' */
                        for(jj = 0; jj < numElements_directAccessTableInfo_override ; jj++)
                        {
                            if(directAccessTableInfo_override[jj].globalIndex ==
                               directAccessTableInfo[ii].globalIndex)
                            {
                                found = GT_TRUE;
                                break;
                            }
                        }
                    }


                    if(found == GT_TRUE)
                    {
                        /* found in the 'directAccessTableInfo_new/directAccessTableInfo_override' ..
                           so do not use 'prev device' info   */
                        /* ASSUME that the address is 0 based in 'directAccessTableInfo_new/directAccessTableInfo_override' */
                        directAccessTableInfo[ii].directAccessInfo.baseAddress += unitBaseAddr;
                    }
                    else
                    {
                        /* this is large unit , that takes more than single unit memory space.
                            need to calculate the offset from the start.

                            for that we need the base address of the unit in the 'prev device'
                        */
                        unitBaseAddr_prevDev =
                            prvCpssDxChHwUnitBaseAddrGet(prevDevSpecialIndication,unitIndex,NULL);

                        directAccessTableInfo[ii].directAccessInfo.baseAddress -= unitBaseAddr_prevDev;
                        directAccessTableInfo[ii].directAccessInfo.baseAddress += unitBaseAddr;
                    }

                    break;
                default:
                    UPDATE_ADDR_WITH_NEW_UNIT_MAC(directAccessTableInfo[ii].directAccessInfo.baseAddress,
                        unitBaseAddr);
                    break;
            }
        }
    }

    for(ii = 0;
        numEntriesInfo[ii].tableType != CPSS_DXCH_TABLE_LAST_E;
        ii++)
    {
        globalIndex = numEntriesInfo[ii].tableType;

        switch(numEntriesInfo[ii].entryType)
        {
            case ENTRY_TYPE_NOT_VALID_E:
                numOfEntries = 0;
                break;
            case ENTRY_TYPE_EPORT_E:
                /* from 'flexFieldNumBitsSupport'  */
                /* according top LIMITED_RESOURCES */
                numOfEntries = 1 + PRV_CPSS_DXCH_PP_HW_INFO_RESOURCE_MAX_VALUE_GET_MAC(devNum,ePort);
                break;
            case ENTRY_TYPE_PHY_PORT_E:
                /* from 'flexFieldNumBitsSupport' */
                numOfEntries = 1 + PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_PHY_PORT_MAC(devNum);
                break;
            case ENTRY_TYPE_MAX_PHY_PORT_E:
                numOfEntries =
                            PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) ?  128 :
                            PRV_CPSS_SIP_6_CHECK_MAC(devNum)    ? 1024 :
                            512 /*BC3 and Aldrin2 */;
                break;
            case ENTRY_TYPE_DEFAULT_EPORT_E:
                numOfEntries = PRV_CPSS_DXCH_MAX_DEFAULT_EPORT_NUMBER_MAC(devNum);
                break;
            case ENTRY_TYPE_EVLAN_MEMBERS_E:
                /* evlan table that hold port members (subject to 'port mode') */
                numOfEntries = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesVlanMembers;
                break;
            case ENTRY_TYPE_EVLAN_E:
                numOfEntries = 1 + PRV_CPSS_DXCH_PP_HW_INFO_RESOURCE_MAX_VALUE_GET_MAC(devNum,vid);
                break;
            case ENTRY_TYPE_EVIDX_E:
                /* from 'flexFieldNumBitsSupport' */
                numOfEntries = 1 + PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_VIDX_MAC(devNum);
                break;
            case ENTRY_TYPE_STG_MEMBERS_E:
                /* stg table that hold port members (subject to 'port mode') */
                numOfEntries = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesStgMembers;
                break;
            case ENTRY_TYPE_L2_ECMP_E:
                numOfEntries = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesL2Ecmp;
                break;
            case ENTRY_TYPE_L2_LTT_ECMP_E:
                numOfEntries = 1 + PRV_CPSS_DXCH_PP_HW_INFO_LAST_INDEX_IN_L2_LTT_ECMP_MAC(devNum);
                break;
            case ENTRY_TYPE_L3_ECMP_E:
                numOfEntries = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesL3Ecmp;
                break;
            case ENTRY_TYPE_TUNNEL_START_E:
                numOfEntries = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesTunnelStart;
                /* each 'line' contains 2 'regular' TS entries */
                numOfEntries /= 2;
                break;
             case ENTRY_TYPE_PORT_ISOLATION_E:
                numOfEntries = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesPortIsolation;
                break;
             case ENTRY_TYPE_VIDX_E:
                numOfEntries = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesVidx;
                break;
            case ENTRY_TYPE_IPVX_NH_E:
                numOfEntries = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesIpNh;
                break;
            case ENTRY_TYPE_LPM_VRFID_E:
                numOfEntries = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesLpmVrfId;
                break;
            case ENTRY_TYPE_PBR_LEAF_E:
                numOfEntries = PRV_CPSS_DXCH_PP_MAC(devNum)->moduleCfg.ip.maxNumOfPbrEntries;
                break;
            case ENTRY_TYPE_MLL_LTT_E:
                numOfEntries = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesMllLtt;
                break;
            case ENTRY_TYPE_MLL_PAIRS_E:
                numOfEntries = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesMllPairs;
                break;
            case ENTRY_TYPE_IPLR_METERING_E:
                numOfEntries = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesIplrMetering;
                break;
            case ENTRY_TYPE_EPLR_METERING_E:
                numOfEntries = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesEplrMetering;
                break;
            case ENTRY_TYPE_IPLR_IPFIX_E:
                numOfEntries = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesIplrIpfix;
                break;
            case ENTRY_TYPE_EPLR_IPFIX_E:
                numOfEntries = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesEplrIpfix;
                break;
            case ENTRY_TYPE_IPLR_MAX_OF_COUNTING_IPFIX_E:
                numOfEntries = MAX(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesIplrMetering,
                                   PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesIplrIpfix);
                break;
            case ENTRY_TYPE_EPLR_MAX_OF_COUNTING_IPFIX_E:
                numOfEntries = MAX(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesEplrMetering,
                                   PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesEplrIpfix);
                break;
            case ENTRY_TYPE_IOAM_E:
                numOfEntries = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesOam;
                break;
            case ENTRY_TYPE_EOAM_E:
                if(GT_TRUE == PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.oam.notSupportEgressOam)
                {
                    /* The EOAM unit not supported ! */
                    numOfEntries = 0;
                }
                else
                {
                    numOfEntries = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesOam;
                }
                break;
            case ENTRY_TYPE_TXQ_QUEUES_E:
                numOfEntries = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesTxqQueue;
                break;
            case ENTRY_TYPE_TXQ_PORTS_E:
                numOfEntries = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.numTxqDq *
                               PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.txqDqNumPortsPerDp;
                break;
            case ENTRY_TYPE_LOC_E:
                numOfEntries = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesProtectionLoc;
                break;
            case ENTRY_TYPE_EXPLICIT_NUM_E:
                /* the explicit value is in parameter <additionalFixedNumberOfEntries>*/
                numOfEntries = 0xFFFFFFFF;
                break;
            case ENTRY_TYPE_SOURCE_ID_MEMBERS_E:
                numOfEntries = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesSrcIdMembers;
                break;
            case ENTRY_TYPE_NON_TRUNK_MEMBERS_E:
                numOfEntries = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesNonTrunkMembers;
                break;
            case ENTRY_TYPE_TXQ_DQ_PORTS_E:
                numOfEntries = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.txqDqNumPortsPerDp;
                break;
            case ENTRY_TYPE_MAX_EPORT_EVLANS_E:
                /* max between eport and evlans */
                numOfEntries = 1 + MAX(
                    (PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_PORT_MAC(devNum))/*eports*/,
                    (PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_VID_MAC(devNum))); /*evlans*/
                break;
            case ENTRY_TYPE_QCN_E:
                /* from 'flexFieldNumBitsSupport' */
                numOfEntries = 64;
                break;

            case ENTRY_TYPE_QOS_PROFILES_E:
                numOfEntries = 1024;
                break;

            case ENTRY_TYPE_THREAD_ID_NUM_E:
                numOfEntries = 256;/* relevant to sip6 only */
                break;

            case ENTRY_TYPE_DP_PORTS_NUM_E:
                numOfEntries = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.info[0].dataPathNumOfPorts;/* relevant to sip6 only */
                break;

            case ENTRY_TYPE_8_TC_NUM_E:
                numOfEntries = 8;/* all the devices */
                break;

            case ENTRY_TYPE_PSI_PORT_E:
                numOfEntries = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.pdqNumPorts;
                break;
            case ENTRY_TYPE_DP_Q_E:
                numOfEntries = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDpNumOfQueues;
                break;

            case ENTRY_TYPE_FDB_E:/* regardless to shared memory actual allocation and current used MHT and 'fineTuning' */
                numOfEntries = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesFdb;
                break;
            case ENTRY_TYPE_EM_E :/* regardless to shared memory actual allocation and current used MHT and 'fineTuning' */
                numOfEntries = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesEm;
                break;
            case ENTRY_TYPE_TCAM_E:
                numOfEntries = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.tcam.maxFloors * _3K;
                break;
            case ENTRY_TYPE_PDX_QUEUE_GROUP_NUM_E:
                numOfEntries = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.pdxNumQueueGroups;
                break;
            case ENTRY_TYPE_DP_PER_TILE_E:
                numOfEntries = MAX_DP_IN_TILE(devNum);
                break;
            case ENTRY_TYPE_MAC_2_ME_E:
                numOfEntries = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.tti.TTI_mac2me_numEntries ?
                               PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.tti.TTI_mac2me_numEntries :
                               128;
                break;
            case ENTRY_TYPE_STREAM_NUM_E:
                numOfEntries = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesStream;
                break;
            case ENTRY_TYPE_STREAM_SRF_NUM_E:
                numOfEntries = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesStreamSrf;
                break;
            case ENTRY_TYPE_STREAM_SRF_HIST_NUM_E:
                numOfEntries = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesStreamSrfHist;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);
        }

        if(numOfEntries)/* check that the table is valid */
        {
            if(numOfEntries == 0xFFFFFFFF)
            {
                /*case of ENTRY_TYPE_EXPLICIT_NUM_E*/
                /* the explicit value is in parameter <additionalFixedNumberOfEntries>*/
                numOfEntries = 0;
            }
            else
            {
                if(numEntriesInfo[ii].divider & (FRACTION_INDICATION_CNS | FRACTION_HALF_TABLE_INDICATION_CNS))
                {
                    /* the line hold 'fraction' of entry , therefore the 'divider' is acting as 'multiplier' */
                    /* so instead of '/=' we do '*=' */
                    /* each 'entry' spread on more than single LINE */
                    numOfEntries *= numEntriesInfo[ii].divider & (~(FRACTION_INDICATION_CNS | FRACTION_HALF_TABLE_INDICATION_CNS));
                }
                else
                if(numEntriesInfo[ii].divider > 1)
                {
                    /* each LINE hold more than single 'entry' */
                    numOfEntries /= numEntriesInfo[ii].divider & (~VERTICAL_INDICATION_CNS);
                }
            }

            if(numEntriesInfo[ii].additionalFixedNumberOfEntries)
            {
                numOfEntries += numEntriesInfo[ii].additionalFixedNumberOfEntries;
            }

            /* this is actually 'number of lines' in the table */
            devTablesInfo[globalIndex].maxNumOfEntries = numOfEntries;
        }
        else
        {
            /* clear the fields of this table as it is not valid */
            devTablesInfo[globalIndex].maxNumOfEntries = 0;

            /* DO NOT reset the fields of 'readTablePtr','writeTablePtr' to allow
               'removed table to be 'reused' by next gen device.

               for example:
               table : CPSS_DXCH_SIP5_15_TABLE_EGRESS_POLICER_METERING_CONFIG_E
               was valid in bobk
               was removed in BC3
               was reused in Falcon ! (need to use bobk's direct/indirect info)
            */
        }
    }

    if (bindIndirect)
    {
                                          /* list of tables DB entries */
        CPSS_DXCH_TABLE_ENT  indirectTablesArr[] = { CPSS_DXCH_TABLE_FDB_E,
                                             CPSS_DXCH_SIP5_TABLE_INGRESS_OAM_E,
                                             CPSS_DXCH_SIP5_TABLE_EGRESS_OAM_E,
                                             CPSS_DXCH_SIP6_TABLE_EXACT_MATCH_E};

        if (numElements_bindIndirect > sizeof(indirectTablesArr)/sizeof(indirectTablesArr[0]))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

        /* bind read/write pointers with indirect info */
        for (ii = 0; ii < numElements_bindIndirect; ii++)
        {
            globalIndex = indirectTablesArr[ii];
            devTablesInfo[globalIndex].readTablePtr =
            devTablesInfo[globalIndex].writeTablePtr = &bindIndirect[ii];
        }
    }

    return GT_OK;
}
/**
* @internal bobcat3InitTable function
* @endinternal
*
* @brief   init the Bobcat3 tables info
*
* @param[in] devNum                   - device number
*                                       NONE
*/
static GT_STATUS bobcat3InitTable
(
     IN GT_U8 devNum
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_TABLES_INFO_STC*               tablesInfoArr;
    PRV_CPSS_DXCH_TABLES_INFO_DIRECT_UNIT_STC*   directAccessTableInfoArr;
    PRV_CPSS_DXCH_TEMP_TABLES_DB_STC*            myFamilyTableInfoPtr;

    /* most tables are like in bobk ... */
    rc = bobkInitTable(devNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    myFamilyTableInfoPtr     = &GET_TEMP_TABLES_PER_FAMILY_ARR[PRV_CPSS_PP_MAC(devNum)->devFamily];
    tablesInfoArr            = myFamilyTableInfoPtr->bobcat3Tables.tablesInfoArr;
    directAccessTableInfoArr = myFamilyTableInfoPtr->bobcat3Tables.directAccessTableInfoArr;

    if(!tablesInfoArr)
    {
        rc = allocTableInfo(bobcat3TableInfoSize,&tablesInfoArr,
            directAccessBobcat3TableInfoSize,&directAccessTableInfoArr);
        if(rc != GT_OK)
        {
            return rc;
        }

        myFamilyTableInfoPtr->bobcat3Tables.tablesInfoArr            = tablesInfoArr;
        myFamilyTableInfoPtr->bobcat3Tables.directAccessTableInfoArr = directAccessTableInfoArr;

        if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
        {
            PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(bobcat3TablesInfoPtr)            = tablesInfoArr;
            PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(directAccessBobcat3TableInfoPtr) = directAccessTableInfoArr;
        }
    }

    /* do specific bobcat3 adaptations */
    rc = genericInitTableSpecifics(
            devNum,
            /*  special devNum indication for understand the offset of table from
                the start of the unit ... for 'large' units.*/
            BOBK_DEVICE_INDICATION_CNS ,

            /* info about the number of entries in the tables */
            sip5_20tableNumEntriesInfoArr,

            /* use bobk as 'base' for the info */
            bobkTableInfoSize,
            PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(bobkTablesInfoPtr),
            /* the tables used to override values of bobk that are not ok for bobcat3 -- general info */
            NUM_ELEMENTS_IN_ARR_MAC(bobcat3TablesInfo_overrideBobk),
            bobcat3TablesInfo_overrideBobk,

            /* use bobk as 'base' for the info */
            directAccessBobkTableInfoSize,
            PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(directAccessBobkTableInfoPtr),

            /* the tables used to override values of bobk that are not ok for bobcat3 -- direct access info */
            NUM_ELEMENTS_IN_ARR_MAC(directAccessBobcat3TableInfo_overrideBobk),
            directAccessBobcat3TableInfo_overrideBobk,

            /* the new tables (not exist in bobk) for bobcat3 -- direct access info */
            NUM_ELEMENTS_IN_ARR_MAC(directAccessBobcat3TableInfo_new),
            directAccessBobcat3TableInfo_new,

            /* indirect info table */
            NUM_ELEMENTS_IN_ARR_MAC(indirectAccessTableInfo_sip_5_20),
            indirectAccessTableInfo_sip_5_20,

            /* the table to fill for bc3 -- general info */
            bobcat3TableInfoSize,
            tablesInfoArr,

            /* the table to fill for bc3 -- direct access info */
            directAccessBobcat3TableInfoSize,
            directAccessTableInfoArr);

    return rc;
}

/**
* @internal aldrin2InitTable function
* @endinternal
*
* @brief   init the Aldrin2 tables info
*
* @param[in] devNum                   - device number
*                                       NONE
*/
static GT_STATUS aldrin2InitTable
(
     IN GT_U8 devNum
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_TABLES_INFO_STC*               tablesInfoArr;
    PRV_CPSS_DXCH_TABLES_INFO_DIRECT_UNIT_STC*   directAccessTableInfoArr;
    PRV_CPSS_DXCH_TABLES_INFO_STC*               prev_tablesInfoArr;
    PRV_CPSS_DXCH_TABLES_INFO_DIRECT_UNIT_STC*   prev_directAccessTableInfoArr;
    PRV_CPSS_DXCH_TEMP_TABLES_DB_STC*            myFamilyTableInfoPtr;

    /* most tables are like in BC3 ... */
    rc = bobcat3InitTable(devNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    myFamilyTableInfoPtr     = &GET_TEMP_TABLES_PER_FAMILY_ARR[PRV_CPSS_PP_MAC(devNum)->devFamily];
    tablesInfoArr            = myFamilyTableInfoPtr->aldrin2Tables.tablesInfoArr;
    directAccessTableInfoArr = myFamilyTableInfoPtr->aldrin2Tables.directAccessTableInfoArr;

    prev_tablesInfoArr            = myFamilyTableInfoPtr->bobcat3Tables.tablesInfoArr;
    prev_directAccessTableInfoArr = myFamilyTableInfoPtr->bobcat3Tables.directAccessTableInfoArr;

    if(!tablesInfoArr)
    {
        rc = allocTableInfo(aldrin2TableInfoSize,&tablesInfoArr,
            aldrin2directAccessTableInfoSize,&directAccessTableInfoArr);
        if(rc != GT_OK)
        {
            return rc;
        }

        myFamilyTableInfoPtr->aldrin2Tables.tablesInfoArr            = tablesInfoArr;
        myFamilyTableInfoPtr->aldrin2Tables.directAccessTableInfoArr = directAccessTableInfoArr;

        if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
        {
            PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(aldrin2TablesInfoPtr)            = tablesInfoArr;
            PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(aldrin2directAccessTableInfoPtr) = directAccessTableInfoArr;
        }
    }

    /* do specific Aldrin2 adaptations */
    rc = genericInitTableSpecifics(
            devNum,
            /*  special devNum indication for understand the offset of table from
                the start of the unit ... for 'large' units.*/
            BC3_DEVICE_INDICATION_CNS ,

            /* info about the number of entries in the tables */
            sip5_25tableNumEntriesInfoArr,

            /* use BC3 as 'base' for the info */
            bobcat3TableInfoSize,
            prev_tablesInfoArr,
            /* the tables used to override values of BC3 that are not ok for Aldrin2 -- general info */
            NUM_ELEMENTS_IN_ARR_MAC(aldrin2TablesInfo_overrideBobcat3),
            aldrin2TablesInfo_overrideBobcat3,

            /* use BC3 as 'base' for the info */
            directAccessBobcat3TableInfoSize,
            prev_directAccessTableInfoArr,

            /* the tables used to override values of BC3 that are not ok for Aldrin2 -- direct access info */
            NUM_ELEMENTS_IN_ARR_MAC(aldrin2directAccessTableInfo_overrideBobcat3),
            aldrin2directAccessTableInfo_overrideBobcat3,

            /* the new tables (not exist in BC3) for Aldrin2 -- direct access info */
            NUM_ELEMENTS_IN_ARR_MAC(aldrin2directAccessTableInfo_new),
            aldrin2directAccessTableInfo_new,

            /* indirect info is copied from BC3 DB */
            0,
            NULL,

            /* the table to fill for aldrin2 -- general info */
            aldrin2TableInfoSize,
            tablesInfoArr,

            /* the table to fill for Aldrin2 -- direct access info */
            aldrin2directAccessTableInfoSize,
            directAccessTableInfoArr);

    return rc;
}

/**
* @internal falconInitTable function
* @endinternal
*
* @brief   init the falcon tables info
*
* @param[in] devNum                   - device number
*                                       NONE
*/
static GT_STATUS falconInitTable
(
     IN GT_U8 devNum
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_TABLES_INFO_STC*               tablesInfoArr;
    PRV_CPSS_DXCH_TABLES_INFO_DIRECT_UNIT_STC*   directAccessTableInfoArr;
    PRV_CPSS_DXCH_TABLES_INFO_STC*               prev_tablesInfoArr;
    PRV_CPSS_DXCH_TABLES_INFO_DIRECT_UNIT_STC*   prev_directAccessTableInfoArr;
    PRV_CPSS_DXCH_TEMP_TABLES_DB_STC*            myFamilyTableInfoPtr;

    /* most tables are like in Aldrin2 ... */
    rc = aldrin2InitTable(devNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    myFamilyTableInfoPtr     = &GET_TEMP_TABLES_PER_FAMILY_ARR[PRV_CPSS_PP_MAC(devNum)->devFamily];
    tablesInfoArr            = myFamilyTableInfoPtr->falconTables.tablesInfoArr;
    directAccessTableInfoArr = myFamilyTableInfoPtr->falconTables.directAccessTableInfoArr;

    prev_tablesInfoArr            = myFamilyTableInfoPtr->aldrin2Tables.tablesInfoArr;
    prev_directAccessTableInfoArr = myFamilyTableInfoPtr->aldrin2Tables.directAccessTableInfoArr;

    if(!tablesInfoArr)
    {
        rc = allocTableInfo(falconTableInfoSize,&tablesInfoArr,
            falcon_directAccessTableInfoSize,&directAccessTableInfoArr);
        if(rc != GT_OK)
        {
            return rc;
        }

        myFamilyTableInfoPtr->falconTables.tablesInfoArr            = tablesInfoArr;
        myFamilyTableInfoPtr->falconTables.directAccessTableInfoArr = directAccessTableInfoArr;

        if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
        {
            PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(falconTablesInfoPtr)            = tablesInfoArr;
            PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(falcon_directAccessTableInfoPtr) = directAccessTableInfoArr;
        }
    }


    /* do specific Falcon adaptations */
    rc = genericInitTableSpecifics(
            devNum,
            /*  special devNum indication for understand the offset of table from
                the start of the unit ... for 'large' units.*/
            ALDRIN2_DEVICE_INDICATION_CNS ,

            /* info about the number of entries in the tables */
            sip6_tableNumEntriesInfoArr,

            /* use Aldrin2 as 'base' for the info */
            aldrin2TableInfoSize,
            prev_tablesInfoArr,

            /* the tables used to override values of Aldrin2 that are not ok for Falcon -- general info */
            NUM_ELEMENTS_IN_ARR_MAC(falconTablesInfo_overrideAldrin2),
            falconTablesInfo_overrideAldrin2,

            /* use Aldrin2 as 'base' for the info */
            aldrin2directAccessTableInfoSize,
            prev_directAccessTableInfoArr,

            /* the tables used to override values of Aldrin2 that are not ok for falcon -- direct access info */
            NUM_ELEMENTS_IN_ARR_MAC(falcon_directAccessTableInfo_overrideAldrin2),
            falcon_directAccessTableInfo_overrideAldrin2,

            /* the new tables (not exist in Aldrin2) for falcon -- direct access info */
            NUM_ELEMENTS_IN_ARR_MAC(falcon_directAccessTableInfo_new),
            falcon_directAccessTableInfo_new,

            /* indirect info table */
            NUM_ELEMENTS_IN_ARR_MAC(indirectAccessTableInfo_falcon),
            indirectAccessTableInfo_falcon,

            /* the table to fill for falcon -- general info */
            falconTableInfoSize,
            tablesInfoArr,

            /* the table to fill for falcon -- direct access info */
            falcon_directAccessTableInfoSize,
            directAccessTableInfoArr);

    return rc;
}


/**
* @internal hawkInitTable function
* @endinternal
*
* @brief   init the hawk tables info
*
* @param[in] devNum                   - device number
*                                       NONE
*/
static GT_STATUS hawkInitTable
(
     IN GT_U8 devNum
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_TABLES_INFO_STC*               tablesInfoArr;
    PRV_CPSS_DXCH_TABLES_INFO_DIRECT_UNIT_STC*   directAccessTableInfoArr;
    PRV_CPSS_DXCH_TABLES_INFO_STC*               prev_tablesInfoArr;
    PRV_CPSS_DXCH_TABLES_INFO_DIRECT_UNIT_STC*   prev_directAccessTableInfoArr;
    PRV_CPSS_DXCH_TEMP_TABLES_DB_STC*            myFamilyTableInfoPtr;

    /* most tables are like in Falcon ... */
    rc = falconInitTable(devNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    myFamilyTableInfoPtr     = &GET_TEMP_TABLES_PER_FAMILY_ARR[PRV_CPSS_PP_MAC(devNum)->devFamily];
    tablesInfoArr            = myFamilyTableInfoPtr->hawkTables.tablesInfoArr;
    directAccessTableInfoArr = myFamilyTableInfoPtr->hawkTables.directAccessTableInfoArr;

    prev_tablesInfoArr            = myFamilyTableInfoPtr->falconTables.tablesInfoArr;
    prev_directAccessTableInfoArr = myFamilyTableInfoPtr->falconTables.directAccessTableInfoArr;

    if(!tablesInfoArr)
    {
        rc = allocTableInfo(hawkTableInfoSize,&tablesInfoArr,
            hawk_directAccessTableInfoSize,&directAccessTableInfoArr);
        if(rc != GT_OK)
        {
            return rc;
        }

        myFamilyTableInfoPtr->hawkTables.tablesInfoArr            = tablesInfoArr;
        myFamilyTableInfoPtr->hawkTables.directAccessTableInfoArr = directAccessTableInfoArr;

        if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
        {
            PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(hawkTablesInfoPtr)            = tablesInfoArr;
            PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(hawk_directAccessTableInfoPtr) = directAccessTableInfoArr;
        }
    }


    /* do specific AC5P adaptations */
    rc = genericInitTableSpecifics(
            devNum,
            /*  special devNum indication for understand the offset of table from
                the start of the unit ... for 'large' units.*/
            FALCON_DEVICE_INDICATION_CNS ,

            /* info about the number of entries in the tables */
            sip6_10_tableNumEntriesInfoArr,

            /* use Falcon as 'base' for the info */
            falconTableInfoSize,
            prev_tablesInfoArr,

            /* the tables used to override values of Falcon that are not ok for Hawk -- general info */
            NUM_ELEMENTS_IN_ARR_MAC(hawkTablesInfo_overrideFalcon),
            hawkTablesInfo_overrideFalcon,

            /* use Falcon as 'base' for the info */
            falcon_directAccessTableInfoSize,
            prev_directAccessTableInfoArr,

            /* the tables used to override values of Falcon that are not ok for Hawk -- direct access info */
            NUM_ELEMENTS_IN_ARR_MAC(hawk_directAccessTableInfo_overrideFalcon),
            hawk_directAccessTableInfo_overrideFalcon,

            /* the new tables (not exist in Falcon) for Hawk -- direct access info */
            NUM_ELEMENTS_IN_ARR_MAC(hawk_directAccessTableInfo_new),
            hawk_directAccessTableInfo_new,

            /* indirect info table */
            NUM_ELEMENTS_IN_ARR_MAC(indirectAccessTableInfo_ac5p),
            indirectAccessTableInfo_ac5p,

            /* the table to fill for Hawk -- general info */
            hawkTableInfoSize,
            tablesInfoArr,

            /* the table to fill for Hawk -- direct access info */
            hawk_directAccessTableInfoSize,
            directAccessTableInfoArr);

    return rc;
}

/**
* @internal phoenixInitTable function
* @endinternal
*
* @brief   init the phoenix tables info
*
* @param[in] devNum                   - device number
*                                       NONE
*/
static GT_STATUS phoenixInitTable
(
     IN GT_U8 devNum
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_TABLES_INFO_STC*               tablesInfoArr;
    PRV_CPSS_DXCH_TABLES_INFO_DIRECT_UNIT_STC*   directAccessTableInfoArr;
    PRV_CPSS_DXCH_TABLES_INFO_STC*               prev_tablesInfoArr;
    PRV_CPSS_DXCH_TABLES_INFO_DIRECT_UNIT_STC*   prev_directAccessTableInfoArr;
    PRV_CPSS_DXCH_TEMP_TABLES_DB_STC*            myFamilyTableInfoPtr;

    /* most tables are like in Hawk ... */
    rc = hawkInitTable(devNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    myFamilyTableInfoPtr     = &GET_TEMP_TABLES_PER_FAMILY_ARR[PRV_CPSS_PP_MAC(devNum)->devFamily];
    tablesInfoArr            = myFamilyTableInfoPtr->phoenixTables.tablesInfoArr;
    directAccessTableInfoArr = myFamilyTableInfoPtr->phoenixTables.directAccessTableInfoArr;

    prev_tablesInfoArr            = myFamilyTableInfoPtr->hawkTables.tablesInfoArr;
    prev_directAccessTableInfoArr = myFamilyTableInfoPtr->hawkTables.directAccessTableInfoArr;

    if(!tablesInfoArr)
    {
        rc = allocTableInfo(phoenixTableInfoSize,&tablesInfoArr,
            phoenix_directAccessTableInfoSize,&directAccessTableInfoArr);
        if(rc != GT_OK)
        {
            return rc;
        }

        myFamilyTableInfoPtr->phoenixTables.tablesInfoArr            = tablesInfoArr;
        myFamilyTableInfoPtr->phoenixTables.directAccessTableInfoArr = directAccessTableInfoArr;

        if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E)
        {
            PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(phoenixTablesInfoPtr)            = tablesInfoArr;
            PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(phoenix_directAccessTableInfoPtr) = directAccessTableInfoArr;
        }
    }

    /* do specific AC5X adaptations */
    rc = genericInitTableSpecifics(
            devNum,
            /*  special devNum indication for understand the offset of table from
                the start of the unit ... for 'large' units.*/
            HAWK_DEVICE_INDICATION_CNS ,

            /* info about the number of entries in the tables */
            sip6_15_tableNumEntriesInfoArr,

            /* use AC5P as 'base' for the info */
            hawkTableInfoSize,
            prev_tablesInfoArr,

            /* the tables used to override values of Hawk that are not ok for Phoenix -- general info */
            NUM_ELEMENTS_IN_ARR_MAC(phoenixTablesInfo_overrideHawk),
            phoenixTablesInfo_overrideHawk,

            /* use AC5P as 'base' for the info */
            hawk_directAccessTableInfoSize,
            prev_directAccessTableInfoArr,

            /* the tables used to override values of Hawk that are not ok for Phoenix -- direct access info */
            NUM_ELEMENTS_IN_ARR_MAC(phoenix_directAccessTableInfo_overrideHawk),
            phoenix_directAccessTableInfo_overrideHawk,

            /* the new tables (not exist in Hawk) for Phoenix -- direct access info */
            NUM_ELEMENTS_IN_ARR_MAC(phoenix_directAccessTableInfo_new),
            phoenix_directAccessTableInfo_new,

            /* indirect info table */
            NUM_ELEMENTS_IN_ARR_MAC(indirectAccessTableInfo_ac5x),
            indirectAccessTableInfo_ac5x,

            /* the table to fill for Phoenix -- general info */
            phoenixTableInfoSize,
            tablesInfoArr,

            /* the table to fill for Phoenix -- direct access info */
            phoenix_directAccessTableInfoSize,
            directAccessTableInfoArr);

    return rc;
}
/**
* @internal harrierInitTable function
* @endinternal
*
* @brief   init the harrier tables info
*
* @param[in] devNum                   - device number
*                                       NONE
*/
static GT_STATUS harrierInitTable
(
     IN GT_U8 devNum
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_TABLES_INFO_STC*               tablesInfoArr;
    PRV_CPSS_DXCH_TABLES_INFO_DIRECT_UNIT_STC*   directAccessTableInfoArr;
    PRV_CPSS_DXCH_TABLES_INFO_STC*               prev_tablesInfoArr;
    PRV_CPSS_DXCH_TABLES_INFO_DIRECT_UNIT_STC*   prev_directAccessTableInfoArr;
    PRV_CPSS_DXCH_TEMP_TABLES_DB_STC*            myFamilyTableInfoPtr;

    /* most tables are like in Phoenix ... */
    rc = phoenixInitTable(devNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    myFamilyTableInfoPtr     = &GET_TEMP_TABLES_PER_FAMILY_ARR[PRV_CPSS_PP_MAC(devNum)->devFamily];
    tablesInfoArr            = myFamilyTableInfoPtr->harrierTables.tablesInfoArr;
    directAccessTableInfoArr = myFamilyTableInfoPtr->harrierTables.directAccessTableInfoArr;

    prev_tablesInfoArr            = myFamilyTableInfoPtr->phoenixTables.tablesInfoArr;
    prev_directAccessTableInfoArr = myFamilyTableInfoPtr->phoenixTables.directAccessTableInfoArr;

    if(!tablesInfoArr)
    {
        rc = allocTableInfo(harrierTableInfoSize,&tablesInfoArr,
            harrier_directAccessTableInfoSize,&directAccessTableInfoArr);
        if(rc != GT_OK)
        {
            return rc;
        }

        myFamilyTableInfoPtr->harrierTables.tablesInfoArr            = tablesInfoArr;
        myFamilyTableInfoPtr->harrierTables.directAccessTableInfoArr = directAccessTableInfoArr;

        if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_HARRIER_E)
        {
            PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(harrierTablesInfoPtr)            = tablesInfoArr;
            PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(harrier_directAccessTableInfoPtr) = directAccessTableInfoArr;
        }
    }

    /* do specific Harrier adaptations */
    rc = genericInitTableSpecifics(
            devNum,
            /*  special devNum indication for understand the offset of table from
                the start of the unit ... for 'large' units.*/
            PHOENIX_DEVICE_INDICATION_CNS ,

            /* info about the number of entries in the tables */
            sip6_20_tableNumEntriesInfoArr,

            /* use AC5X as 'base' for the info */
            phoenixTableInfoSize,
            prev_tablesInfoArr,

            /* the tables used to override values of Hawk that are not ok for Harrier -- general info */
            NUM_ELEMENTS_IN_ARR_MAC(harrierTablesInfo_overridePhoenix),
            harrierTablesInfo_overridePhoenix,

            /* use AC5X as 'base' for the info */
            phoenix_directAccessTableInfoSize,
            prev_directAccessTableInfoArr,

            /* the tables used to override values of Hawk that are not ok for Harrier -- direct access info */
            NUM_ELEMENTS_IN_ARR_MAC(harrier_directAccessTableInfo_overridePhoenix),
            harrier_directAccessTableInfo_overridePhoenix,

            /* the new tables (not exist in Hawk) for Harrier -- direct access info */
            NUM_ELEMENTS_IN_ARR_MAC(harrier_directAccessTableInfo_new),
            harrier_directAccessTableInfo_new,

            /* indirect info table */
            NUM_ELEMENTS_IN_ARR_MAC(indirectAccessTableInfo_harrier),
            indirectAccessTableInfo_harrier,

            /* the table to fill for Harrier -- general info */
            harrierTableInfoSize,
            tablesInfoArr,

            /* the table to fill for Harrier -- direct access info */
            harrier_directAccessTableInfoSize,
            directAccessTableInfoArr);

    return rc;
}

/**
* @internal ironmanInitTable function
* @endinternal
*
* @brief   init the ironman tables info
*
* @param[in] devNum                   - device number
*                                       NONE
*/
static GT_STATUS ironmanInitTable
(
     IN GT_U8 devNum
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_TABLES_INFO_STC*               tablesInfoArr;
    PRV_CPSS_DXCH_TABLES_INFO_DIRECT_UNIT_STC*   directAccessTableInfoArr;
    PRV_CPSS_DXCH_TABLES_INFO_STC*               prev_tablesInfoArr;
    PRV_CPSS_DXCH_TABLES_INFO_DIRECT_UNIT_STC*   prev_directAccessTableInfoArr;
    PRV_CPSS_DXCH_TEMP_TABLES_DB_STC*            myFamilyTableInfoPtr;

    /* most tables are like in Harrier ... */
    rc = harrierInitTable(devNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    myFamilyTableInfoPtr     = &GET_TEMP_TABLES_PER_FAMILY_ARR[PRV_CPSS_PP_MAC(devNum)->devFamily];
    tablesInfoArr            = myFamilyTableInfoPtr->ironmanTables.tablesInfoArr;
    directAccessTableInfoArr = myFamilyTableInfoPtr->ironmanTables.directAccessTableInfoArr;

    prev_tablesInfoArr            = myFamilyTableInfoPtr->harrierTables.tablesInfoArr;
    prev_directAccessTableInfoArr = myFamilyTableInfoPtr->harrierTables.directAccessTableInfoArr;

    if(!tablesInfoArr)
    {
        rc = allocTableInfo(ironman_L_TableInfoSize,&tablesInfoArr,
            ironman_L_directAccessTableInfoSize,&directAccessTableInfoArr);
        if(rc != GT_OK)
        {
            return rc;
        }

        myFamilyTableInfoPtr->ironmanTables.tablesInfoArr            = tablesInfoArr;
        myFamilyTableInfoPtr->ironmanTables.directAccessTableInfoArr = directAccessTableInfoArr;

        if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E)
        {
            PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(ironman_L_TablesInfoPtr)            = tablesInfoArr;
            PRV_SHARED_HW_INIT_TABLES_DB_VAR_GET(ironman_L_directAccessTableInfoPtr) = directAccessTableInfoArr;
        }
    }

    /* do specific ironman_L adaptations */
    rc = genericInitTableSpecifics(
            devNum,
            /*  special devNum indication for understand the offset of table from
                the start of the unit ... for 'large' units.*/
            IRONMAN_DEVICE_INDICATION_CNS ,

            /* info about the number of entries in the tables */
            ironman_L_tableNumEntriesInfoArr,

            /* use Harrier as 'base' for the info */
            harrierTableInfoSize,
            prev_tablesInfoArr,

            /* the tables used to override values of Harrier that are not ok for Ironman_L -- general info */
            NUM_ELEMENTS_IN_ARR_MAC(ironman_L_TablesInfo_overrideHarrier),
            ironman_L_TablesInfo_overrideHarrier,

            /* use Harrier as 'base' for the info */
            harrier_directAccessTableInfoSize,
            prev_directAccessTableInfoArr,

            /* the tables used to override values of Harrier that are not ok for Ironman_L -- direct access info */
            NUM_ELEMENTS_IN_ARR_MAC(ironman_L_directAccessTableInfo_overrideHarrier),
            ironman_L_directAccessTableInfo_overrideHarrier,

            /* the new tables (not exist in Harrier) for Ironman_L -- direct access info */
            NUM_ELEMENTS_IN_ARR_MAC(ironman_L_directAccessTableInfo_new),
            ironman_L_directAccessTableInfo_new,

            /* indirect info table */
            NUM_ELEMENTS_IN_ARR_MAC(indirectAccessTableInfo_ironMan),
            indirectAccessTableInfo_ironMan,

            /* the table to fill for ironman_L -- general info */
            ironman_L_TableInfoSize,
            tablesInfoArr,

            /* the table to fill for ironman_L -- direct access info */
            ironman_L_directAccessTableInfoSize,
            directAccessTableInfoArr);

    return rc;
}

/**
* @internal prvCpssDxChIndexAsPortNumConvert function
* @endinternal
*
* @brief   convert index that is portNum ('Global port num') to portGroupId and
*         'Local port num'
*         for tables that are not with index = portNum , no modification in the
*         index , and portGroupId will be 'first active port group == 0'
*         for non multi-port-groups device : portGroupId is 'all port groups' , no index conversion
*
* @note   APPLICABLE DEVICES:       xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] tableType                - the specific table name
* @param[in] entryIndex               - index in the table
*
* @param[out] portGroupIdPtr           - (pointer to) the portGroupId to use
* @param[out] updatedEntryIndexPtr     - (pointer to) the updated index
* @param[out] updatedTableTypePtr      - (pointer to) the NEW specific table name to use
*                                       none
*/
GT_VOID prvCpssDxChIndexAsPortNumConvert
(
    IN GT_U8                devNum,
    IN CPSS_DXCH_TABLE_ENT  tableType,
    IN GT_U32               entryIndex,
    OUT GT_U32              *portGroupIdPtr,
    OUT GT_U32              *updatedEntryIndexPtr,
    OUT CPSS_DXCH_TABLE_ENT *updatedTableTypePtr
)
{
    GT_STATUS   rc;
    GT_U32  portNum; /* temp local port number */
    GT_U32  offset;  /* temp offset of port from the base port index */
    GT_U32  newIndex;/* temp index for the port calculation */
    CPSS_DXCH_TABLE_ENT newTableOffset;
    GT_U32  dmaPortNum;/* temp DMA port number */
    GT_U32  dpIndex ,pipeId;/* temp DP index , local DMA and Pipe number */

    GT_U32  exactMatchNumOfBanks=0; /*temp number of exact match banks for index Modify  */

    /* converts that done also for 'non-multi port group' device */
    *updatedTableTypePtr = tableType;

    /*default is 'unaware'*/
    *portGroupIdPtr = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;


    if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.numTxqDq > 1)/*Aldrin2/BC3*/
    {
        switch(tableType)
        {
            case CPSS_DXCH_LION_TABLE_TXQ_EGRESS_STC_E:
            case CPSS_DXCH_LION_TABLE_TXQ_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG_E:
            case CPSS_DXCH_LION_TABLE_TXQ_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG_E:
                offset = entryIndex / PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.txqDqNumPortsPerDp;
                newIndex = entryIndex % PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.txqDqNumPortsPerDp;

                if(tableType == CPSS_DXCH_LION_TABLE_TXQ_EGRESS_STC_E)
                {
                    newTableOffset = CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TXQ_EGRESS_STC__DQ_1_E;
                }
                else
                if(tableType == CPSS_DXCH_LION_TABLE_TXQ_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG_E)
                {
                    newTableOffset = CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG__DQ_1_E;
                }
                else
                if(tableType == CPSS_DXCH_LION_TABLE_TXQ_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG_E)
                {
                    newTableOffset = CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG__DQ_1_E;
                }
                else
                {
                    newTableOffset = 0;
                    /* ERROR */
                    *updatedTableTypePtr = CPSS_DXCH_TABLE_LAST_E;/* will generate error in the caller */
                    newIndex = 0;
                    offset = 0;
                }

                if(offset != 0)
                {
                    *updatedTableTypePtr = newTableOffset + (offset-1);

                    if((offset-1) > (CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TXQ_EGRESS_STC__DQ_LAST_E -
                                     CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_TXQ_EGRESS_STC__DQ_1_E))
                    {
                        /* ERROR */
                        *updatedTableTypePtr = CPSS_DXCH_TABLE_LAST_E;/* will generate error in the caller */
                        newIndex = 0;
                    }
                }

                entryIndex = newIndex;
                break;
            default:
                break;
        }
    }

    if(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipes > 1)/*BC3*/
    {
        switch(tableType)
        {
            case CPSS_DXCH_SIP5_TABLE_EQ_INGRESS_STC_PHYSICAL_PORT_E:
                /* we must read/write from/to single Pipe */

                portNum = entryIndex;

                /* we must convert the physical port to DMA Number and to find the 'pipeId' */
                /* for ports without mapping ... consider pipe 0 (should not happen with real application) */
                /* do CPSS_LOG_WARNING message  */
                rc = prvCpssDxChPortPhysicalPortMapCheckAndConvert(devNum, portNum, PRV_CPSS_DXCH_PORT_TYPE_RxDMA_E, &dmaPortNum);
                if(rc != GT_OK)
                {
                    /* keep index as as into the pipe '0' */
                    break;
                }

                rc = prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(devNum,dmaPortNum,&dpIndex ,NULL);
                if(rc != GT_OK)
                {
                    break;
                }

                pipeId = (dpIndex * PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipes) /
                                    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp;

                if (pipeId > 1)
                {
                    /* ERROR */
                    *updatedTableTypePtr = CPSS_DXCH_TABLE_LAST_E;/* will generate error in the caller */
                    newIndex = 0;
                }
                else if (pipeId == 1)
                {
                    *updatedTableTypePtr = CPSS_DXCH_SIP5_20_MULTI_INSTANCE_TABLE_EQ_INGRESS_STC_PHYSICAL_PORT_PIPE_1_E;
                }

                break;

            default:
                break;
        }
    }

    switch(tableType)
    {
        case CPSS_DXCH_TABLE_MULTICAST_E:
            if(!PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                break;
            }

            if(entryIndex == FLOOD_VIDX_CNS &&
               FLOOD_VIDX_CNS > PRV_CPSS_DXCH_PP_HW_INFO_LAST_INDEX_IN_TABLE_VIDX_MAC(devNum))
            {
                /* allow the CPSS APIs to keep working with 0xFFF ,
                   and do the convert at this layer */
                entryIndex = PRV_CPSS_DXCH_PP_HW_INFO_LAST_INDEX_IN_TABLE_VIDX_MAC(devNum);
            }
            break;
        case CPSS_DXCH_SIP6_TABLE_EXACT_MATCH_E:
            if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                /* change the index format to match GM pattern (4 msb bits for bank number )
                   if the index goes to WM, the index will change back to its original pattern
                   (number of bits depends on number of banks )
                */
                PRV_CPSS_DXCH_EXACT_MATCH_NUM_OF_BANKS_GET_MAC(devNum,exactMatchNumOfBanks);
                if (exactMatchNumOfBanks == 0)
                {
                    /* error will be obtained by caller */
                    *updatedTableTypePtr = CPSS_DXCH_TABLE_LAST_E + 1;
                    return;
                }
                *updatedEntryIndexPtr = ((entryIndex /exactMatchNumOfBanks /*shift right 2,3,4 places */ ) << 4)/*shift left 4 places*/
                                        | (entryIndex & (exactMatchNumOfBanks-1 /*num of bits to take */ ));
                return;
            }
            break;
        case CPSS_DXCH_TABLE_FDB_E:
            if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                GT_U32 numBitsPerBank = PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.fdbNumOfBitsPerBank;
                /* for indirect accsess the index For all MHT modes: 4/8/16 bank design, entry bits are as follows:
                    [3:0] - Bank number
                    [19:4] - Address inside bank*/
                if (numBitsPerBank == 4)
                {
                    /* No need conversion for MHT16 - all 4 bits are used for bank ID */
                    break;
                }
                *updatedEntryIndexPtr = (entryIndex >> numBitsPerBank) << 4 | (entryIndex & ((1 << numBitsPerBank) - 1));
                return;
            }
            break;
        default:
            break;
    }


    if(PRV_CPSS_IS_LION_STYLE_MULTI_PORT_GROUPS_DEVICE_MAC(devNum) == 0)
    {
        /* no conversion for non-multi port group device */
        *updatedEntryIndexPtr = entryIndex;

        return ;
    }

    switch(tableType)
    {
        case CPSS_DXCH_TABLE_VLAN_PORT_PROTOCOL_E:
            /* special case :
                this is a '16 entries per port' table
                see calculations in function cpssDxChBrgVlanPortProtoVlanQosSet(...)
            */
            /* the index supposed to be ((portNum * 16) + entryNum)  */
            portNum = entryIndex / 16;/*portNum*/
            offset = entryIndex % 16; /*entryNum*/

            if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE)
            {
                /*no global port to local convert needed*/

                /* the new index is the 'input index'*/
                newIndex = entryIndex;
            }
            else
            {
                *portGroupIdPtr = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,portNum);
                newIndex = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);
                /* build the new index from the local port and the offset */
                newIndex = newIndex * 16 + offset;
            }

            *updatedEntryIndexPtr = newIndex;
            break;
        case CPSS_DXCH_TABLE_PORT_VLAN_QOS_E:
        case CPSS_DXCH_LION_TABLE_TXQ_EGRESS_STC_E:
            /* the index supposed to be portNum */
            *portGroupIdPtr = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,entryIndex);
            *updatedEntryIndexPtr = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,entryIndex);
            break;
        case CPSS_DXCH_LION_TABLE_TXQ_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG_E:
        case CPSS_DXCH_LION_TABLE_TXQ_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG_E:
            *portGroupIdPtr = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,entryIndex);
            /* the index supposed to be localPortNum , but the CPU port must be converted to 0xF */
            *updatedEntryIndexPtr = entryIndex & 0xf;
            break;

        default:
            /*no global port to local convert needed*/
            *updatedEntryIndexPtr = entryIndex;
            break;
    }

    return;
}

/**
* @internal writeTableEntry_splitOn2entries function
* @endinternal
*
* @brief   function to write logical entry that is split to 2 lines in 2 parts of
*         the table.(offset of 1/2 table between them)
*         NOTE: function must be called only when :
*         0 == PRV_CPSS_DXCH_BMP_OF_TABLES_IN_HALF_SIZE_MODE_GET_MAC(devNum,tableType)
*
* @note   APPLICABLE DEVICES:       xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - the port group Id , to support multi-port-group devices that need to access
*                                      specific port group
* @param[in] tableType                - the specific table name
* @param[in] entryIndex               - index in the table (of the first half of entry)
* @param[in] entryValuePtr            - (pointer to) the data that will be written to the table
*                                      may be NULL in the case of indirect table.
* @param[in] entryMaskPtr             - (pointer to) a mask. Ignored if entryValuePtr is NULL.
*                                      If NULL -  the entry specified by entryValuePtr will be
*                                      written to HW table as is.
*                                      If not NULL  - only bits that are raised in the mask
*                                      will be updated in the HW entry.
* @param[in] halfEntryBitOffset       - offset of the second part of the data (bit index) in entryValuePtr[]
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS writeTableEntry_splitOn2entries
(
    IN GT_U8                    devNum,
    IN GT_U32                   portGroupId,
    IN CPSS_DXCH_TABLE_ENT      tableType,
    IN GT_U32                   entryIndex,
    IN GT_U32                  *entryValuePtr,
    IN GT_U32                  *entryMaskPtr,
    IN GT_U32                   halfEntryBitOffset
)
{
    GT_STATUS   rc = GT_FAIL;
    GT_U32 ii ;
    GT_U32 divisionMode;
    GT_U32 entryIndexOffset, entryBitOffset;
    PRV_CPSS_DXCH_TABLES_INFO_STC *tableInfoPtr; /* pointer to table info */

    tableInfoPtr = PRV_TABLE_INFO_PTR_GET_MAC(devNum,tableType);

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        /* number of lines that required for one entry*/
        divisionMode  = TABLE_MODE_TO_DIVISION_NUMBER_MAC(devNum);
        /* step size between entry lines*/
        entryIndexOffset = (tableType == CPSS_DXCH_LION_TABLE_VLAN_EGRESS_E ? 1024 : 256);
        /* EGF - sht tables lines are fixed to 128 bit ( qag not ..??) (4 * 32 (int) = 128)*/
        entryBitOffset = 4;
    }
    else
    {
        /* number of lines that required for one entry*/
        divisionMode = 2;
        /* step size between entry lines is half table*/
        entryIndexOffset = (tableInfoPtr->maxNumOfEntries / 2 );
        /* EGF - sht tables lines are fixed to 256 bit .(8 * 32 (int) = 256)*/
        entryBitOffset = halfEntryBitOffset>>5; /* 8 */
    }

    for ( ii = 0 ; ii < divisionMode; ii++ )
    {
        rc = prvCpssDxChPortGroupWriteTableEntryMasked(
            devNum,
            portGroupId,
            tableType,
            entryIndex + (entryIndexOffset*ii),
            &entryValuePtr[entryBitOffset*ii],
            (entryMaskPtr == NULL? NULL : &entryMaskPtr[entryBitOffset*ii]));
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "failed to write table[%d] : part %d of the entry index [%d]",
                tableType, ii, entryIndex);
        }
    }

    return rc;
}

/**
* @internal prvCpssDxChPortGroupWriteTableEntry_splitOn2entries function
* @endinternal
*
* @brief   function to write logical entry that is split to 2 lines in 2 parts of
*         the table.(offset of 1/2 table between them)
*
* @note   APPLICABLE DEVICES:       xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - the port group Id , to support multi-port-group devices that need to access
*                                      specific port group
* @param[in] tableType                - the specific table name
* @param[in] entryIndex               - index in the table (of the first half of entry)
* @param[in] entryValuePtr            - (pointer to) the data that will be written to the table
*                                      may be NULL in the case of indirect table.
* @param[in] entryMaskPtr             - (pointer to) a mask. Ignored if entryValuePtr is NULL.
*                                      If NULL -  the entry specified by entryValuePtr will be
*                                      written to HW table as is.
*                                      If not NULL  - only bits that are raised in the mask
*                                      will be updated in the HW entry.
* @param[in] halfEntryBitOffset       - offset of the second part of the data (bit index) in entryValuePtr[]
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChPortGroupWriteTableEntry_splitOn2entries
(
    IN GT_U8                    devNum,
    IN GT_U32                   portGroupId,
    IN CPSS_DXCH_TABLE_ENT      tableType,
    IN GT_U32                   entryIndex,
    IN GT_U32                  *entryValuePtr,
    IN GT_U32                  *entryMaskPtr,
    IN GT_U32                   halfEntryBitOffset
)
{
    GT_STATUS   rc;
    GT_U32      needToRestoreHalfTableMode;

    needToRestoreHalfTableMode = PRV_CPSS_DXCH_BMP_OF_TABLES_IN_HALF_SIZE_MODE_GET_MAC(devNum,tableType);
    if(needToRestoreHalfTableMode)
    {
        /* remove the option to get 'FRACTION_HALF_TABLE_INDICATION_CNS' from function
            prvCpssDxChTableEngineMultiEntriesInLineIndexAndGlobalBitConvert() that is called from
            prvCpssDxChPortGroupWriteTableEntry(...) */
        PRV_CPSS_DXCH_BMP_OF_TABLES_IN_HALF_SIZE_MODE_CLEAR_MAC(devNum,tableType);
    }

    rc = writeTableEntry_splitOn2entries(
        devNum,
        portGroupId,
        tableType,
        entryIndex,
        entryValuePtr,
        entryMaskPtr,
        halfEntryBitOffset
    );
    if(needToRestoreHalfTableMode)
    {
        /* restore value */
        PRV_CPSS_DXCH_BMP_OF_TABLES_IN_HALF_SIZE_MODE_SET_MAC(devNum,tableType);
    }

    return rc;
}

/**
* @internal prvCpssDxChPortGroupWriteTableEntryMasked function
* @endinternal
*
* @brief   Either write a whole entry into HW table or update HW entry bits
*         specified by a mask. If Shadow DB table exists it will be updated too.
*         Works for specific portGroupId only.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - the port group Id , to support multi-port-group devices that need to access
*                                      specific port group
* @param[in] tableType                - the specific table name
* @param[in] entryIndex               - index in the table
* @param[in] entryValuePtr            - (pointer to) the data that will be written to the table
*                                      may be NULL in the case of indirect table.
* @param[in] entryMaskPtr             - (pointer to) a mask array.
*                                      Ignored if entryValuePtr is NULL.
*                                      If NULL -  the entry specified by entryValuePtr will be
*                                      written to HW table as is.
*                                      If not NULL  - only bits that are raised in the mask
*                                      will be updated in the HW entry.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In the case of entryValuePtr == NULL and indirect table the function
*       just send write entry command to device. And entry is taken from the
*       data registers of a indirect table.
*
*/
GT_STATUS prvCpssDxChPortGroupWriteTableEntryMasked
(
    IN GT_U8                    devNum,
    IN GT_U32                   portGroupId,
    IN CPSS_DXCH_TABLE_ENT      tableType,
    IN GT_U32                   entryIndex,
    IN GT_U32                  *entryValuePtr,
    IN GT_U32                  *entryMaskPtr
)
{
    PRV_CPSS_DXCH_TABLES_INFO_STC *tableInfoPtr; /* pointer to table info */
    /* pointer to direct access table information */
    PRV_CPSS_DXCH_TABLES_INFO_DIRECT_STC *tablePtr;
    GT_STATUS   status = GT_OK;
    GT_U32  entryMemoBufArr[PRV_CPSS_DXCH_TABLE_MAX_ENTRY_SIZE_CNS]; /* entry prepared for writing into HW */
    GT_U32  entryMaskBuf[PRV_CPSS_DXCH_TABLE_MAX_ENTRY_SIZE_CNS]; /* used for updating of shadow table */
    GT_U32  address;  /* address to write the value */
    GT_U32  i;
    GT_U32  tmpReadBuff;
    GT_U32  tmpPortGroupId = portGroupId;/* temporary port group id */
    GT_U32  originPortGroupId = portGroupId;
    GT_U32  orig_entryIndex; /* orig entry index as calculate in this function */
    GT_U32  *orig_entryValuePtr;/* original pointer from the caller */
    GT_U32  *orig_entryMaskPtr;
    GT_U32  startBit;/* start bit in the case of multi entries in single LINE */
    GT_U32  numBitsPerEntry;/*the number of bits that the 'entry' is using in the LINE.*/
    GT_U32  numEntriesPerLine;/*the number of entries in single LINE.
                        NOTE: if value hold FRACTION_INDICATION_CNS meaning that the
                        number in the lower 31 bits is 'fraction' (1/x) and not (x)*/
    GT_U32  numLinesPerEntry = 1;/* the number of lines that the entry occupy.
                        NOTE: value used only if MORE than 1 */
    GT_U32  lineIndex;/* line index when iteration over numLinesPerEntry */
    GT_BOOL isHwFormatDifferent; /* false - if entryIndex, entryValuePtr can be used
                                    as is for HW table updating i.e. just write whole
                                    entryValuePtr into HW table with index = entryIndex */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    if(portGroupId == CPSS_PORT_GROUP_UNAWARE_MODE_CNS ||
       (PRV_CPSS_IS_LION_STYLE_MULTI_PORT_GROUPS_DEVICE_MAC(devNum) == 0))
    {
        /* this needed for direct calls to prvCpssDxChPortGroupReadTableEntry with
           portGroupId = CPSS_PORT_GROUP_UNAWARE_MODE_CNS , and still need convert
           of the entryIndex.
           for example :
           CPSS_DXCH_LION_TABLE_TXQ_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG_E
        */

        /* convert if needed the entry index , get specific portGroupId (or all port groups) */
        prvCpssDxChIndexAsPortNumConvert(devNum,tableType,entryIndex, &tmpPortGroupId , &entryIndex,&tableType);
    }

    /* validity check */
    if((GT_U32)tableType >= PRV_CPSS_DXCH_TABLE_SIZE_GET_MAC(devNum))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);


    tableInfoPtr = PRV_TABLE_INFO_PTR_GET_MAC(devNum,tableType);

    if(tableInfoPtr->maxNumOfEntries == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(
            GT_NO_SUCH, "tableType[%d] - is not valid for the device (no such table)",
            tableType);
    }


    orig_entryIndex = entryIndex;
    orig_entryValuePtr = entryValuePtr;
    orig_entryMaskPtr  = entryMaskPtr;
    startBit = 0;
    numBitsPerEntry = 0;/*dont care*/
    numEntriesPerLine = 0;/*dont care*/
    /* check if the table entry is actually implemented as
       'several entries' in single 'line'
    */
    isHwFormatDifferent = prvCpssDxChTableEngineMultiEntriesInLineIndexAndGlobalBitConvert(
        devNum,tableType, INOUT &entryIndex,INOUT &startBit,
        OUT &numBitsPerEntry , OUT &numEntriesPerLine);

    if(entryIndex >= tableInfoPtr->maxNumOfEntries)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(
            GT_OUT_OF_RANGE, "tableType[%d] , orig_entryIndex [%d] ,index[%d] is out of range [0..%d]",
            tableType, orig_entryIndex, entryIndex, tableInfoPtr->maxNumOfEntries-1);
    }
    if(isHwFormatDifferent)
    {
        if(numEntriesPerLine & FRACTION_HALF_TABLE_INDICATION_CNS)
        {
            GT_U32 divisionMode = TABLE_MODE_TO_DIVISION_NUMBER_MAC(devNum);

            if (orig_entryIndex >= (tableInfoPtr->maxNumOfEntries / divisionMode))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "tableType [%d] index[%d] is out of range [0..%d]",tableType,
                    orig_entryIndex,
                    (tableInfoPtr->maxNumOfEntries/divisionMode)-1);
            }
            /* indication that the entry uses multiple lines but not consecutive */
            status = prvCpssDxChPortGroupWriteTableEntry_splitOn2entries(
                devNum, portGroupId, tableType,
                entryIndex, /* this is the orig entry index case of bc3 and new line index in case of falcon*/
                entryValuePtr,
                entryMaskPtr,
                numBitsPerEntry                      /* halfEntryBitOffset   */
            );

            return status;
        }

        if(numEntriesPerLine & FRACTION_INDICATION_CNS)
        {
            /* indication that the entry uses multiple lines */
            numLinesPerEntry = numEntriesPerLine - FRACTION_INDICATION_CNS;

        }

        if((tableInfoPtr->entrySize * numLinesPerEntry) > PRV_CPSS_DXCH_TABLE_MAX_ENTRY_SIZE_CNS)
        {
            /* need to set PRV_CPSS_DXCH_TABLE_MAX_ENTRY_SIZE_CNS with larger value ?! */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

        if(tableInfoPtr->writeAccessType == PRV_CPSS_DXCH_INDIRECT_ACCESS_E)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);
        }

        /***** Write a whole entry of HW table by direct access method *****/

        /* Prepare buffer for the HW. It stores multi HW entries in
           single LINE , because the caller is not aware about the actual size
           of the HW entry. */


        entryValuePtr = &entryMemoBufArr[0];

        if (orig_entryMaskPtr != NULL || 1 == numLinesPerEntry)
        {

            /* 1) Need to fill the entry value buffer with all HW entries
                  (lines) relevant for 'logical' entry.
               2) Need prepare the entry mask buffer to be passed
                  into prvCpssDxChPortGroupShadowLineUpdateMasked */

            /*********** prepare entry value buffer ***********/

            /* read all HW entries that appropriate to 'logical' incomping entry index */
            status = directReadTableHwEntries(devNum, portGroupId, tableInfoPtr,
                                            entryIndex,
                                            numLinesPerEntry,
                                            entryMemoBufArr);
            if (status != GT_OK)
            {
                return status;
            }

            /*********** prepare entry mask buffer ***********/

            entryMaskPtr = &entryMaskBuf[0];
            cpssOsMemSet(entryMaskBuf, 0, sizeof(entryMaskBuf));
            if (numLinesPerEntry == 1)
            {
                /*If numLinesPerEntry==1 it is possible the entry occupies just a
                  part of HW table entry. Prepare mask to update a relevant part only. */
                if (NULL == orig_entryMaskPtr)
                {
                    raiseBitsInMemory(entryMaskBuf, startBit, numBitsPerEntry);
                }
                else
                {
                    copyBits(entryMaskBuf, startBit, orig_entryMaskPtr, 0, numBitsPerEntry);
                }
            }
            else
            {
                 /* orig_entryMaskPtr != NULL */
                for (lineIndex = 0; lineIndex < numLinesPerEntry; lineIndex++)
                {
                    /* fill prepared mask buffer with caller mask value */
                    copyBits(entryMaskBuf, /* target */
                             lineIndex * tableInfoPtr->entrySize * 32,/* start bit in target */
                             orig_entryMaskPtr,/*source*/
                             lineIndex*numBitsPerEntry,/* start bit In the source */
                             numBitsPerEntry);/* number of bits in the entry */
                }
            }
        }

        /****** update buffer with caller value and mask ******/
        for (lineIndex = 0; lineIndex < numLinesPerEntry; lineIndex++)
        {
            /* update prepared entry buffer with caller value */
            copyBitsMasked(entryMemoBufArr, /* target */
                           startBit + lineIndex * tableInfoPtr->entrySize * 32,/* start bit in target */
                           orig_entryValuePtr,/*source*/
                           lineIndex*numBitsPerEntry,/* start bit In the source */
                           numBitsPerEntry,/* number of bits in the entry */
                           orig_entryMaskPtr);/* mask. Ignored if NULL */
        }

    }
    else if (orig_entryMaskPtr != NULL)
    {

        /* incomming entry format is the same as HW format but we need apply
         * the mask. So still need read HW entry */
        entryValuePtr = entryMemoBufArr;
        if (tableInfoPtr->readAccessType == PRV_CPSS_DXCH_INDIRECT_ACCESS_E)
        {
            /* Read a whole entry from table by indirect access method */
            status = indirectReadTableEntry(
                devNum, portGroupId, tableType, tableInfoPtr, entryIndex,
                (PRV_CPSS_DXCH_TABLES_INFO_INDIRECT_STC*)(tableInfoPtr->readTablePtr),
                entryMemoBufArr);
        }
        else
        {
            /* Read a whole entry from table by direct access method */
            status = directReadTableHwEntries(devNum, portGroupId, tableInfoPtr,
                entryIndex, 1, entryMemoBufArr);
        }

        if (status != GT_OK)
        {
            return status;
        }

        /* update entry bits with caller info according to the mask */
        copyBitsMasked(entryMemoBufArr,    /* target */
                       0,                  /* start bit in target */
                       orig_entryValuePtr, /*source*/
                       0,                  /* start bit In the source */
                       tableInfoPtr->entrySize * 32, /* number of bits in the entry */
                       orig_entryMaskPtr);      /* mask for source */
    }

    /* Check if HW access required. */
    if(PRV_SHAREG_GLOBAL_TABLE_ACCESS_OBJECT.hwAccessTableEntryWriteFunc != NULL)
    {

       status = PRV_SHAREG_GLOBAL_TABLE_ACCESS_OBJECT.hwAccessTableEntryWriteFunc(
                                                  devNum, tmpPortGroupId,
                                                  tableType,
                                                  orig_entryIndex,
                                                  orig_entryValuePtr,
                                                  orig_entryMaskPtr,
                                                  originPortGroupId,
                                                  CPSS_DRV_HW_ACCESS_STAGE_PRE_E,
                                                  status);
       if (status != GT_OK)
       {
           return (status == GT_ABORTED) ? GT_OK : status;
       }

    }

    if(tableInfoPtr->writeAccessType == PRV_CPSS_DXCH_INDIRECT_ACCESS_E)
    {
        /* if mask is specified use the entry value with applied mask
         * instead of original value */
        GT_U32 *maskedEntryPtr =  (NULL == orig_entryMaskPtr) ?
            orig_entryValuePtr : entryValuePtr;

        /* Write a whole entry to table by indirect access method */
        status = prvCpssDxChWriteTableEntry_indirect(devNum, portGroupId,tableType, tableInfoPtr,
                                         orig_entryIndex,
                                         maskedEntryPtr, /* entry with applied mask (if specified) */
                                         entryMaskPtr); /* mask for updating shadow entry only */
    }
    else
    {
        /* there is no information to write */
        if(entryValuePtr == NULL)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);

        /* Write a whole entry to table by direct access method */
        tablePtr =
            (PRV_CPSS_DXCH_TABLES_INFO_DIRECT_STC*)(tableInfoPtr->writeTablePtr);

        if(tablePtr->baseAddress == SPECIAL_ADDRESS_NOT_EXISTS_E)
        {
            /* the table not supported */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        for(lineIndex = 0 ; lineIndex < numLinesPerEntry ; lineIndex++ , entryIndex++)
        {
            /* update HW table shadow (if supported and requested) */
            status = prvCpssDxChPortGroupShadowLineUpdateMasked(devNum, portGroupId,
                               tableType, entryIndex, entryValuePtr, entryMaskPtr);
            if (status != GT_OK)
            {
               CPSS_LOG_ERROR_AND_RETURN_MAC(status, "update hw table shadow failed");
            }


            address = tablePtr->baseAddress + entryIndex * tablePtr->step;

            if (tablePtr->nextWordOffset == PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS)
            {
                /* VLT tables WA */
                if((GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum, PRV_CPSS_DXCH3_VLT_TABLES_WRITE_DIRECT_ACCESS_WA_E)) &&
                   ((CPSS_DXCH_TABLE_STG_E == tableType) ||
                   (CPSS_DXCH_TABLE_VLAN_E == tableType)))
                {
                    status  = prvCpssHwPpPortGroupReadRam (devNum, portGroupId, address, 1, &tmpReadBuff);
                    if(status != GT_OK)
                    {
                       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                    }
                }

                if(tableInfoPtr->writeAccessType == PRV_CPSS_DXCH_MGCAM_ACCESS_E)
                {
                    status = prvCpssHwPpPortGroupMgCamWriteRam(devNum, portGroupId, address, tableInfoPtr->entrySize,
                                                                         entryValuePtr);
                }
                else
                {
                    status = prvCpssHwPpPortGroupWriteRam(devNum, portGroupId, address, tableInfoPtr->entrySize,
                                                                         entryValuePtr);
                }

                if (status != GT_OK)
                {
                   CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }
            }
            else
            {
                address -= tablePtr->nextWordOffset;

                for (i=0; i<tableInfoPtr->entrySize; i++)
                {
                    address += tablePtr->nextWordOffset;

                    if(tableInfoPtr->writeAccessType == PRV_CPSS_DXCH_MGCAM_ACCESS_E)
                    {
                        status = prvCpssHwPpPortGroupMgCamWriteRam(devNum, portGroupId, address, 1, &entryValuePtr[i]);
                    }
                    else
                    {
                        status = prvCpssHwPpPortGroupWriteRam(devNum, portGroupId, address, 1, &entryValuePtr[i]);
                    }

                    if (status != GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                    }
                }

                status = GT_OK;
            }

            /* jump to next partial entry */
            entryValuePtr += tableInfoPtr->entrySize;
            if (entryMaskPtr)
            {
                entryMaskPtr += tableInfoPtr->entrySize;
            }
        }
        entryIndex -= numLinesPerEntry;/* restore entryIndex */
    }

    /* Check if HW access required. */
    if(PRV_SHAREG_GLOBAL_TABLE_ACCESS_OBJECT.hwAccessTableEntryWriteFunc != NULL)
    {
       /* Don't care of returned status */
       PRV_SHAREG_GLOBAL_TABLE_ACCESS_OBJECT.hwAccessTableEntryWriteFunc(
                                                  devNum, tmpPortGroupId,
                                                  tableType,
                                                  orig_entryIndex,
                                                  orig_entryValuePtr,
                                                  orig_entryMaskPtr,
                                                  originPortGroupId,
                                                  CPSS_DRV_HW_ACCESS_STAGE_POST_E,
                                                  status);
    }

    return status;
}

/**
* @internal prvCpssDxChWriteTableEntryMasked function
* @endinternal
*
* @brief   Either write a whole entry into HW table or update HW entry bits
*         specified by a mask. If Shadow DB table exists it will be updated too.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] tableType                - the specific table name
* @param[in] entryIndex               - index in the table
* @param[in] entryValuePtr            - (pointer to) the data that will be written to the table
*                                      may be NULL in the case of indirect table.
* @param[in] entryMaskPtr             - (pointer to) a mask array.
*                                      Ignored if entryValuePtr is NULL.
*                                      If NULL -  the entry specified by entryValuePtr will be
*                                      written to HW table as is.
*                                      If not NULL  - only bits that are raised in the mask
*                                      will be updated in the HW entry.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In the case of entryValuePtr == NULL and indirect table the function
*       just send write entry command to device. And entry is taken from the
*       data registers of a indirect table.
*
*/
GT_STATUS prvCpssDxChWriteTableEntryMasked
(
    IN GT_U8                    devNum,
    IN CPSS_DXCH_TABLE_ENT      tableType,
    IN GT_U32                   entryIndex,
    IN GT_U32                  *entryValuePtr,
    IN GT_U32                  *entryMaskPtr
)
{
    GT_U32      portGroupId;/* support multi-port-groups device for table accesses with index = 'per port' */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    /* convert if needed the entry index , get specific portGroupId (or all port groups) */
    prvCpssDxChIndexAsPortNumConvert(devNum,tableType,entryIndex,
                          &portGroupId , &entryIndex,&tableType);

    return prvCpssDxChPortGroupWriteTableEntryMasked(devNum,portGroupId,
                            tableType, entryIndex, entryValuePtr, entryMaskPtr);
}


/**
* @internal prvCpssDxChPortGroupWriteTableEntry function
* @endinternal
*
* @brief   Write (for specific portGroupId) a whole entry to the HW table.
*         If Shadow DB table exists it will be updated too.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - the port group Id , to support multi-port-group devices that need to access
*                                      specific port group
* @param[in] tableType                - the specific table name
* @param[in] entryIndex               - index in the table
* @param[in] entryValuePtr            - (pointer to) the data that will be written to the table
*                                      may be NULL in the case of indirect table.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In the case of entryValuePtr == NULL and indirect table the function
*       just send write entry command to device. And entry is taken from the
*       data registers of a indirect table.
*
*/
GT_STATUS prvCpssDxChPortGroupWriteTableEntry
(
    IN GT_U8                    devNum,
    IN GT_U32                   portGroupId,
    IN CPSS_DXCH_TABLE_ENT  tableType,
    IN GT_U32                   entryIndex,
    IN GT_U32                  *entryValuePtr
)
{
    return prvCpssDxChPortGroupWriteTableEntryMasked(devNum,
                                                     portGroupId,
                                                     tableType,
                                                     entryIndex,
                                                     entryValuePtr,
                                                     NULL);
}

/**
* @internal readTableEntry_splitOn2entries function
* @endinternal
*
* @brief   function to read logical entry that is split to 2 lines in 2 parts of
*         the table.(offset of 1/2 table between them)
*         NOTE: function must be called only when :
*         0 == PRV_CPSS_DXCH_BMP_OF_TABLES_IN_HALF_SIZE_MODE_GET_MAC(devNum,tableType)
*
* @note   APPLICABLE DEVICES:       xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - the port group Id , to support multi-port-group devices that need to access
*                                      specific port group
* @param[in] tableType                - the specific table name
* @param[in] entryIndex               - index in the table (of the first half of entry)
* @param[in] halfTableIndexOffset     - offset of the second part of the entry from the 'entryIndex'
* @param[in] halfEntryBitOffset       - offset of the second part of the data (bit index) in entryValuePtr[]
*
* @param[out] entryValuePtr            - (pointer to) the data read from the table
*                                      may be NULL in the case of indirect table.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS readTableEntry_splitOn2entries
(
    IN GT_U8                    devNum,
    IN GT_U32                   portGroupId,
    IN CPSS_DXCH_TABLE_ENT  tableType,
    IN GT_U32                   entryIndex,
    OUT GT_U32                  *entryValuePtr,
    IN GT_U32                   halfEntryBitOffset
)
{

    GT_STATUS   rc = GT_FAIL;
    GT_U32 ii ;
    GT_U32 divisionMode;
    GT_U32 entryIndexOffset, entryBitOffset;
    PRV_CPSS_DXCH_TABLES_INFO_STC *tableInfoPtr; /* pointer to table info */

    tableInfoPtr = PRV_TABLE_INFO_PTR_GET_MAC(devNum,tableType);


    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        /* number of lines that required for one entry*/
        divisionMode  = TABLE_MODE_TO_DIVISION_NUMBER_MAC(devNum) ;
        /* step size between entry lines*/
        entryIndexOffset = (tableType == CPSS_DXCH_LION_TABLE_VLAN_EGRESS_E ? 1024 : 256);
        /* EGF - sht tables lines are fixed to 128 bit */
        entryBitOffset = 4;
    }
    else
    {
        /* number of lines that required for one entry*/
        divisionMode = 2;
        /* step size between entry lines is half table*/
        entryIndexOffset = (tableInfoPtr->maxNumOfEntries / 2 );
        /* EGF - sht tables lines are fixed to 256 bit .(8 * 32 (int) = 256)*/
        entryBitOffset = halfEntryBitOffset>>5; /* 8 */
    }

    for ( ii = 0 ; ii < divisionMode; ii++ )
    {
        rc = prvCpssDxChPortGroupReadTableEntry(
            devNum,
            portGroupId,
            tableType,
            entryIndex + (entryIndexOffset*ii),
            &entryValuePtr[entryBitOffset*ii]);

        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "failed to write table[%d] : part %d of the entry index [%d]",
                tableType, ii, entryIndex);
        }
    }

    return rc;
}

/**
* @internal prvCpssDxChPortGroupReadTableEntry_splitOn2entries function
* @endinternal
*
* @brief   function to read logical entry that is split to 2 lines in 2 parts of
*         the table.(offset of 1/2 table between them)
*
* @note   APPLICABLE DEVICES:       xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - the port group Id , to support multi-port-group devices that need to access
*                                      specific port group
* @param[in] tableType                - the specific table name
* @param[in] entryIndex               - index in the table (of the first half of entry)
* @param[in] halfEntryBitOffset       - offset of the second part of the data (bit index) in entryValuePtr[]
*
* @param[out] entryValuePtr            - (pointer to) the data read from the table
*                                      may be NULL in the case of indirect table.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChPortGroupReadTableEntry_splitOn2entries
(
    IN GT_U8                    devNum,
    IN GT_U32                   portGroupId,
    IN CPSS_DXCH_TABLE_ENT  tableType,
    IN GT_U32                   entryIndex,
    OUT GT_U32                  *entryValuePtr,
    IN GT_U32                   halfEntryBitOffset
)
{
    GT_STATUS   rc;
    GT_U32      needToRestoreHalfTableMode;

    needToRestoreHalfTableMode = PRV_CPSS_DXCH_BMP_OF_TABLES_IN_HALF_SIZE_MODE_GET_MAC(devNum,tableType);
    if(needToRestoreHalfTableMode)
    {
        /* remove the option to get 'FRACTION_HALF_TABLE_INDICATION_CNS' from function
            prvCpssDxChTableEngineMultiEntriesInLineIndexAndGlobalBitConvert() that is called from
            prvCpssDxChPortGroupReadTableEntry(...) */
        PRV_CPSS_DXCH_BMP_OF_TABLES_IN_HALF_SIZE_MODE_CLEAR_MAC(devNum,tableType);
    }

    rc = readTableEntry_splitOn2entries(
        devNum,
        portGroupId,
        tableType,
        entryIndex,
        entryValuePtr,
        halfEntryBitOffset
    );
    if(needToRestoreHalfTableMode)
    {
        /* restore value */
        PRV_CPSS_DXCH_BMP_OF_TABLES_IN_HALF_SIZE_MODE_SET_MAC(devNum,tableType);
    }

    return rc;
}

/**
* @internal prvCpssDxChPortGroupReadTableEntry function
* @endinternal
*
* @brief   Read a whole entry from the table. - for specific portGroupId
*
* @note   APPLICABLE DEVICES:       xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - the port group Id , to support multi-port-group devices that need to access
*                                      specific port group
* @param[in] tableType                - the specific table name
* @param[in] entryIndex               - index in the table
*
* @param[out] entryValuePtr            - (pointer to) the data read from the table
*                                      may be NULL in the case of indirect table.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In the case of entryValuePtr == NULL and indirect table the function
*       just send read entry command to device. And entry is stored in the
*       data registers of a indirect table
*
*/
GT_STATUS prvCpssDxChPortGroupReadTableEntry
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroupId,
    IN CPSS_DXCH_TABLE_ENT tableType,
    IN GT_U32                  entryIndex,
    OUT GT_U32                 *entryValuePtr
)
{
    GT_U32  entryMemoBufArr[PRV_CPSS_DXCH_TABLE_MAX_ENTRY_SIZE_CNS];
    PRV_CPSS_DXCH_TABLES_INFO_STC *tableInfoPtr; /* table info */
    GT_STATUS   status = GT_OK;
    GT_U32  tmpPortGroupId = portGroupId;/* temporary port group id */
    GT_U32  originPortGroupId = portGroupId;
    GT_U32  *orig_entryValuePtr;/* original pointer from the caller */
    GT_U32  startBit;/* start bit in the case of multi entries in single LINE */
    GT_U32  numBitsPerEntry;/*the number of bits that the 'entry' is using in the LINE.*/
    GT_BOOL tableOfMultiEntriesInLine;/* indication of multi entries in single LINE */
    GT_U32  numEntriesPerLine;/*the number of entries in single LINE.
                        NOTE: if value hold FRACTION_INDICATION_CNS meaning that the
                        number in the lower 31 bits is 'fraction' (1/x) and not (x)*/
    GT_U32  numLinesPerEntry = 1;/* the number of lines that the entry occupy.
                        NOTE: value used only if MORE than 1 */
    GT_U32  lineIndex;/* line index when iteration over numLinesPerEntry */
    GT_U32  orig_entryIndex; /* orig entry index as calculate in this function */
    GT_BOOL readFromShadow; /* GT_TRUE - read from Data Integrity Shadow, GT_FALSE - read from HW */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    if(portGroupId == CPSS_PORT_GROUP_UNAWARE_MODE_CNS ||
       (PRV_CPSS_IS_LION_STYLE_MULTI_PORT_GROUPS_DEVICE_MAC(devNum) == 0))
    {
        /* this needed for direct calls to prvCpssDxChPortGroupReadTableEntry with
           portGroupId = CPSS_PORT_GROUP_UNAWARE_MODE_CNS , and still need convert
           of the entryIndex.
           for example :
           CPSS_DXCH_LION_TABLE_TXQ_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG_E
        */

        /* convert if needed the entry index , get specific portGroupId (or all port groups) */
        prvCpssDxChIndexAsPortNumConvert(devNum,tableType,entryIndex, &tmpPortGroupId , &entryIndex,&tableType);
    }

    /* validity check */
    if((GT_U32)tableType >=  PRV_CPSS_DXCH_TABLE_SIZE_GET_MAC(devNum))
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);

    tableInfoPtr = PRV_TABLE_INFO_PTR_GET_MAC(devNum,tableType);

    orig_entryIndex = entryIndex;
    orig_entryValuePtr = entryValuePtr;
    startBit = 0;
    numBitsPerEntry = 0;/*dont care*/
    numEntriesPerLine = 0;/*dont care*/
    /* check if the table entry is actually implemented as
       'several entries' in single 'line' */
    if(GT_TRUE == prvCpssDxChTableEngineMultiEntriesInLineIndexAndGlobalBitConvert(devNum,tableType,
                INOUT &entryIndex,INOUT &startBit ,
                OUT &numBitsPerEntry, OUT &numEntriesPerLine))
    {
        if(numEntriesPerLine & FRACTION_HALF_TABLE_INDICATION_CNS)
        {
            GT_U32 divisionMode = TABLE_MODE_TO_DIVISION_NUMBER_MAC(devNum);
            if(orig_entryIndex >= (tableInfoPtr->maxNumOfEntries/divisionMode))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "index[%d] is out of range [0..%d]",
                    orig_entryIndex,
                    (tableInfoPtr->maxNumOfEntries/divisionMode)-1);
            }

            /* indication that the entry uses multiple lines but not consecutive */
            status = prvCpssDxChPortGroupReadTableEntry_splitOn2entries(
                devNum,
                portGroupId,
                tableType,
                entryIndex,
                entryValuePtr,
                numBitsPerEntry                      /* halfEntryBitOffset   */
            );
            return status;
        }

        if(numEntriesPerLine & FRACTION_INDICATION_CNS)
        {
            /* indication that the entry uses multiple lines */
            numLinesPerEntry = numEntriesPerLine - FRACTION_INDICATION_CNS;
        }

        /*
           point the function to use the 'local' buffer that should support the
           multi entries in single LINE , because the caller is not aware to
           the actual size of the entry.
        */
        entryValuePtr = &entryMemoBufArr[0];

        tableOfMultiEntriesInLine = GT_TRUE;
    }
    else
    {
        tableOfMultiEntriesInLine = GT_FALSE;
    }

    if(entryIndex >= tableInfoPtr->maxNumOfEntries)
    {
        if(tableInfoPtr->maxNumOfEntries)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "tableType[%d] index[%d] is out of range [0..%d]. orig entry index %d",
                tableType,
                entryIndex,
                (tableInfoPtr->maxNumOfEntries-1),
                orig_entryIndex);
        }
        else/* 0 entries --> table not valid */
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "tableType[%d] is not valid for any access !",
                tableType);
        }
    }

    if(PRV_SHAREG_GLOBAL_TABLE_ACCESS_OBJECT.hwAccessTableEntryReadFunc != NULL)
    {
       status = PRV_SHAREG_GLOBAL_TABLE_ACCESS_OBJECT.hwAccessTableEntryReadFunc(
                                                  devNum, tmpPortGroupId, tableType,
                                                  orig_entryIndex, entryValuePtr, originPortGroupId,
                                                  CPSS_DRV_HW_ACCESS_STAGE_PRE_E, status);
       if (status != GT_OK)
       {
           return (GT_ABORTED == status) ? GT_OK : status;
       }
    }

    readFromShadow = GT_FALSE;
    status = prvCpssDxChTableReadFromShadowEnableGet(
        devNum, tableType, &readFromShadow);
    if ((status == GT_OK) && (readFromShadow == GT_TRUE))
    {
        status = prvCpssDxChPortGroupReadTableEntry_fromShadow(
            devNum, portGroupId, tableType, entryIndex, entryValuePtr);
        if (status != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(
                status, "tableType[%d] read from shadow error", tableType, entryIndex);
        }
    }

    if (readFromShadow == GT_FALSE)
    {
        for (lineIndex = 0; lineIndex < numLinesPerEntry; lineIndex++, entryIndex++)
        {
            if(tableInfoPtr->readAccessType == PRV_CPSS_DXCH_INDIRECT_ACCESS_E)
            {
                /* Read a whole entry from table by indirect access method */
                status = indirectReadTableEntry(
                    devNum, portGroupId, tableType, tableInfoPtr, entryIndex,
                    (PRV_CPSS_DXCH_TABLES_INFO_INDIRECT_STC*)(tableInfoPtr->readTablePtr),
                    entryValuePtr);
            }
            else
            {    /* Read a whole entry from table by direct access method */
                status = directReadTableHwEntries(devNum, portGroupId, tableInfoPtr,
                                                  entryIndex, 1, entryValuePtr);
            }
            if (status != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(status, "failed table [%d] entry [%d] portGroup [%d]", tableType, entryIndex, portGroupId);
            }

            if(tableOfMultiEntriesInLine == GT_TRUE)
            {
                /* we have done reading the info from HW , but we need to prepare it for
                   the 'caller' */
                copyBits(orig_entryValuePtr ,/* target */
                         numBitsPerEntry * lineIndex,/* start bit in target */
                         entryValuePtr,/*source*/
                         startBit, /* start bit In the source */
                         numBitsPerEntry);/* number of bits in the entry */
            }
        }
        entryIndex -= numLinesPerEntry;/* restore entryIndex */

        if(tableOfMultiEntriesInLine == GT_TRUE)
        {
            entryValuePtr = orig_entryValuePtr;
            if((numBitsPerEntry * numLinesPerEntry) & 0x1f)
            {
                /* reset the last words of the 'entry' that we read */
                resetBitsInMemory(orig_entryValuePtr,/*target*/
                    numBitsPerEntry * numLinesPerEntry, /* start bit in target */
                    32 - ((numBitsPerEntry * numLinesPerEntry) & 0x1f));
            }
        }
    }

    if(PRV_SHAREG_GLOBAL_TABLE_ACCESS_OBJECT.hwAccessTableEntryReadFunc != NULL)
    {
       PRV_SHAREG_GLOBAL_TABLE_ACCESS_OBJECT.hwAccessTableEntryReadFunc(
                                                  devNum, tmpPortGroupId, tableType,
                                                  orig_entryIndex, entryValuePtr, originPortGroupId,
                                                  CPSS_DRV_HW_ACCESS_STAGE_POST_E, status);
    }

    return status;
}

/**
* @internal prvCpssDxChPortGroupReadTableEntryField function
* @endinternal
*
* @brief   Read a field from the table. - for specific portGroupId
*
* @note   APPLICABLE DEVICES:       xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - the port group Id , to support multi-port-group devices that need to access
*                                      specific port group
* @param[in] tableType                - the specific table name
* @param[in] entryIndex               - index in the table
* @param[in] fieldWordNum             - field word number
*                                      use PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS
*                                      if need global offset in the field of fieldOffset
*                                      use PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS
*                                      if need 'field name' in the field of fieldOffset
*                                      use PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS
*                                      if need 'field name' in the field of fieldOffset
* @param[in] fieldOffset              - field offset
* @param[in] fieldLength              - field length
*                                      when fieldWordNum == PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS
*                                      the field 'fieldLength' is used ONLY if  it's value != 0
*                                      and then it means 'Mumber of consecutive fields' !!!
*
* @param[out] fieldValuePtr            - (pointer to) the data read from the table
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortGroupReadTableEntryField
(
    IN GT_U8                  devNum,
    IN GT_U32                  portGroupId,
    IN CPSS_DXCH_TABLE_ENT tableType,
    IN GT_U32                 entryIndex,
    IN GT_U32                 fieldWordNum,
    IN GT_U32                 fieldOffset,
    IN GT_U32                 fieldLength,
    OUT GT_U32                *fieldValuePtr
)
{
    GT_U32  entryMemoBufArr[PRV_CPSS_DXCH_TABLE_MAX_ENTRY_SIZE_CNS];
    PRV_CPSS_DXCH_TABLES_INFO_STC *tableInfoPtr; /* pointer to table info */
    GT_U32 address; /* address to read from */
    /* pointer to direct table info */
    PRV_CPSS_DXCH_TABLES_INFO_DIRECT_STC *tablePtr;
    GT_STATUS status = GT_OK;
    GT_U32  tmpPortGroupId = portGroupId;/* temporary port group id */
    GT_U32  originPortGroupId = portGroupId;
    GT_U32  subFieldOffset;/* the sub field offset */
    GT_U32  subFieldLength;/* the sub field length */
    PRV_CPSS_DXCH_PP_TABLE_FORMAT_INFO_STC *  fieldsInfoPtr;
    GT_U32  orig_entryIndex; /* orig entry index as calculate in this function */
    GT_U32  orig_fieldOffset;/* orig entry field Offset as calculate in this function */
    GT_BOOL readFromShadow; /* GT_TRUE - read from Data Integrity Shadow, GT_FALSE - read from HW */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    if(portGroupId == CPSS_PORT_GROUP_UNAWARE_MODE_CNS ||
       (PRV_CPSS_IS_LION_STYLE_MULTI_PORT_GROUPS_DEVICE_MAC(devNum) == 0))
    {
        /* this needed for direct calls to prvCpssDxChPortGroupReadTableEntry with
           portGroupId = CPSS_PORT_GROUP_UNAWARE_MODE_CNS , and still need convert
           of the entryIndex.
           for example :
           CPSS_DXCH_LION_TABLE_TXQ_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG_E
        */

        /* convert if needed the entry index , get specific portGroupId (or all port groups) */
        prvCpssDxChIndexAsPortNumConvert(devNum,tableType,entryIndex, &tmpPortGroupId , &entryIndex,&tableType);
    }

    /* validity check */
    if((GT_U32)tableType >= PRV_CPSS_DXCH_TABLE_SIZE_GET_MAC(devNum))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);

    tableInfoPtr = PRV_TABLE_INFO_PTR_GET_MAC(devNum,tableType);

    if(fieldWordNum == PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS)
    {
        fieldsInfoPtr = tableFieldsFormatInfoGet(devNum, tableType);
        if(fieldsInfoPtr == NULL)
        {
            /* the table is not bound to fields format */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        if (fieldsInfoPtr->patternNum == 0)
        {
            /* convert field info to global bit position */
            status = convertFieldInfoToGlobalBit(devNum,tableType,&fieldWordNum,&fieldOffset,&fieldLength,0);
            if(status != GT_OK)
            {
                return status;
            }
        }
        else    /* pattern entry */
        {
            /* convert index and pattern field info to new index and global bit position */
            status = convertPatternFieldInfoToGlobalBit(devNum, tableType, &entryIndex, &fieldWordNum, &fieldOffset, &fieldLength, 0);
            if(status != GT_OK)
            {
                return status;
            }
        }
    }
    else if(fieldWordNum == PRV_CPSS_DXCH_TABLE_WORD_INDICATE_SUB_FIELD_IN_FIELD_NAME_CNS)
    {
        /* reverse the operation of PRV_CPSS_DXCH_TABLE_SUB_FIELD_OFFSET_AND_LENGTH_MAC */
        subFieldOffset = fieldLength & 0xFFFF;
        subFieldLength = fieldLength >> 16;

        /* convert field info to global bit position */
        status = convertSubFieldInfoToGlobalBit(devNum,tableType,subFieldOffset,subFieldLength,&fieldWordNum,&fieldOffset,&fieldLength,0);
        if(status != GT_OK)
        {
            return status;
        }
    }

    orig_entryIndex  = entryIndex;
    orig_fieldOffset = fieldOffset;

    if(fieldWordNum == PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS)
    {
        /* check if the table entry is actually implemented as
           'several entries' in single 'line' */
        prvCpssDxChTableEngineMultiEntriesInLineIndexAndGlobalBitConvert(devNum,tableType,
                    INOUT &entryIndex,INOUT &fieldOffset,NULL,NULL);

        /* the field of fieldOffset is used as 'Global offset' in the entry */
        if((entryIndex >= tableInfoPtr->maxNumOfEntries)    ||
           ((fieldOffset + fieldLength) > (tableInfoPtr->entrySize * 32)) ||
           ((fieldLength == 0) || (fieldLength > 32)))
        {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }

        /* check if the write is to single word or more */
        if(/*(fieldLength > 1) && */OFFSET_TO_WORD_MAC(fieldOffset) != OFFSET_TO_WORD_MAC(fieldOffset + fieldLength))
        {
            /* writing to more than single word in the entry */

            if(tableInfoPtr->entrySize > PRV_CPSS_DXCH_TABLE_MAX_ENTRY_SIZE_CNS)
            {
                /* need to set PRV_CPSS_DXCH_TABLE_MAX_ENTRY_SIZE_CNS with larger value ?! */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }

            return prvCpssDxChPortGroupReadTableEntryFieldList(devNum,portGroupId ,
                tableType,orig_entryIndex,entryMemoBufArr,1,
                &orig_fieldOffset,&fieldLength,fieldValuePtr);
        }

        fieldWordNum = OFFSET_TO_WORD_MAC(fieldOffset);
        fieldOffset  = OFFSET_TO_BIT_MAC(fieldOffset);
    }
    else
    if((entryIndex >= tableInfoPtr->maxNumOfEntries)    ||
       ((fieldOffset + fieldLength) > 32)               ||
       (fieldLength == 0)                               ||
       (fieldWordNum >= tableInfoPtr->entrySize))
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);


    /* Check if HW access required. */
    if(PRV_SHAREG_GLOBAL_TABLE_ACCESS_OBJECT.hwAccessTableEntryFieldReadFunc != NULL)
    {
       status = PRV_SHAREG_GLOBAL_TABLE_ACCESS_OBJECT.hwAccessTableEntryFieldReadFunc(
                                                  devNum, tmpPortGroupId, tableType,
                                                  entryIndex, fieldWordNum, fieldOffset,
                                                  fieldLength,fieldValuePtr, originPortGroupId,
                                                  CPSS_DRV_HW_ACCESS_STAGE_PRE_E, status);
       if(status == GT_ABORTED)
        return GT_OK;

       if(status != GT_OK)
        return status;
    }

    readFromShadow = GT_FALSE;
    status = prvCpssDxChTableReadFromShadowEnableGet(
        devNum, tableType, &readFromShadow);
    if ((status == GT_OK) && (readFromShadow == GT_TRUE))
    {
        status = prvCpssDxChPortGroupReadTableEntry_fromShadow(
            devNum, portGroupId, tableType, entryIndex, entryMemoBufArr);
        if (status != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(
                status, "tableType[%d] read from shadow error", tableType, entryIndex);
        }
    }
    else
    {
        /* fix warning - 'entryMemoBufArr' array elements might be used uninitialized */
        readFromShadow = GT_FALSE;
    }

    if (readFromShadow == GT_FALSE)
    {
        if (tableInfoPtr->readAccessType == PRV_CPSS_DXCH_INDIRECT_ACCESS_E)
            /*Read a field of entry from table by indirect access method. */
            status = indirectReadTableEntryField(devNum, portGroupId,tableType, tableInfoPtr,
                   entryIndex,fieldWordNum, fieldOffset, fieldLength, fieldValuePtr);
        else
        {
            /*Read a field of entry from table by direct access method. */
            tablePtr =
                 (PRV_CPSS_DXCH_TABLES_INFO_DIRECT_STC*)(tableInfoPtr->readTablePtr);

            if(tablePtr->baseAddress == SPECIAL_ADDRESS_NOT_EXISTS_E)
            {
                /* the table not supported */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }

            address = tablePtr->baseAddress + entryIndex * tablePtr->step +
                                                                   fieldWordNum * tablePtr->nextWordOffset;

            status = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId, address,fieldOffset, fieldLength,
                                                                    fieldValuePtr);
        }
    }
    else
    {
        /* from shadow */
        GT_U32  mask;
        GT_U32  value;

        mask   = BIT_MASK_MAC(fieldLength) << fieldOffset;
        value  = (entryMemoBufArr[fieldWordNum] & mask);
        *fieldValuePtr = value >> fieldOffset;
    }

    /* Check if HW access required. */
    if(PRV_SHAREG_GLOBAL_TABLE_ACCESS_OBJECT.hwAccessTableEntryFieldReadFunc != NULL)
    {
       PRV_SHAREG_GLOBAL_TABLE_ACCESS_OBJECT.hwAccessTableEntryFieldReadFunc(
                                                  devNum, tmpPortGroupId, tableType,
                                                  entryIndex, fieldWordNum, fieldOffset,
                                                  fieldLength,fieldValuePtr, originPortGroupId,
                                                  CPSS_DRV_HW_ACCESS_STAGE_POST_E, status);
    }

    return status;

}

/**
* @internal prvCpssDxChPortGroupWriteTableEntryField function
* @endinternal
*
* @brief   Write a field to the table. - for specific portGroupId
*
* @note   APPLICABLE DEVICES:       xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - the port group Id , to support multi-port-group devices that need to access
*                                      specific port group
* @param[in] tableType                - the specific table name
* @param[in] entryIndex               - index in the table
* @param[in] fieldWordNum             - field word number
*                                      use PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS
*                                      if need global offset in the field of fieldOffset
*                                      use PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS
*                                      if need 'field name' in the field of fieldOffset
*                                      use PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS
*                                      if need 'field name' in the field of fieldOffset
* @param[in] fieldOffset              - field offset
* @param[in] fieldLength              - field length
*                                      when fieldWordNum == PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS
*                                      the field 'fieldLength' is used ONLY if  it's value != 0
*                                      and then it means 'Mumber of consecutive fields' !!!
* @param[in] fieldValue               - the data write to the table
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In xCat A1 and above devices the data is updated only when the last
*       word in the entry was written.
*
*/
GT_STATUS prvCpssDxChPortGroupWriteTableEntryField
(
    IN GT_U8                  devNum,
    IN GT_U32                 portGroupId,
    IN CPSS_DXCH_TABLE_ENT    tableType,
    IN GT_U32                 entryIndex,
    IN GT_U32                 fieldWordNum,
    IN GT_U32                 fieldOffset,
    IN GT_U32                 fieldLength,
    IN GT_U32                 fieldValue
)
{
    GT_U32  entryMemoBufArr[PRV_CPSS_DXCH_TABLE_MAX_ENTRY_SIZE_CNS];
    GT_U32  entryMaskBuf[PRV_CPSS_DXCH_TABLE_MAX_ENTRY_SIZE_CNS];
    GT_STATUS   status = GT_OK;  /* return code */
    PRV_CPSS_DXCH_TABLES_INFO_STC *tableInfoPtr; /* pointer to table info */
    GT_U32 address; /* address to write to */
    /* pointer to direct table info */
    PRV_CPSS_DXCH_TABLES_INFO_DIRECT_STC *tablePtr;
    GT_U32      entrySize;  /* table entry size in words */
    GT_U32  tmpPortGroupId = portGroupId;/* temporary port group id */
    GT_U32  originPortGroupId = portGroupId;
    GT_U32  contWriteTable = 1;
    GT_U32  subFieldOffset;/* the sub field offset */
    GT_U32  subFieldLength;/* the sub field length */
    PRV_CPSS_DXCH_PP_TABLE_FORMAT_INFO_STC *  fieldsInfoPtr;
    GT_U32  orig_entryIndex; /* orig entry index as calculate in this function */
    GT_U32  orig_fieldOffset;/* orig entry field Offset as calculate in this function */
    GT_U32  orig_fieldWordNum;/* orig entry field wordNum as calculate in this function */
    GT_U32  numEntriesPerLine;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    if(portGroupId == CPSS_PORT_GROUP_UNAWARE_MODE_CNS ||
       (PRV_CPSS_IS_LION_STYLE_MULTI_PORT_GROUPS_DEVICE_MAC(devNum) == 0))
    {
        /* this needed for direct calls to prvCpssDxChPortGroupReadTableEntry with
           portGroupId = CPSS_PORT_GROUP_UNAWARE_MODE_CNS , and still need convert
           of the entryIndex.
           for example :
           CPSS_DXCH_LION_TABLE_TXQ_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG_E
        */

        /* convert if needed the entry index , get specific portGroupId (or all port groups) */
        prvCpssDxChIndexAsPortNumConvert(devNum,tableType,entryIndex, &tmpPortGroupId , &entryIndex,&tableType);
    }

    /* validity check */
    if((GT_U32)tableType >= PRV_CPSS_DXCH_TABLE_SIZE_GET_MAC(devNum))
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);

    tableInfoPtr = PRV_TABLE_INFO_PTR_GET_MAC(devNum,tableType);

    entrySize = tableInfoPtr->entrySize;

    if(fieldWordNum == PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS)
    {
        fieldsInfoPtr = tableFieldsFormatInfoGet(devNum, tableType);
        if(fieldsInfoPtr == NULL)
        {
            /* the table is not bound to fields format */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
        if (fieldsInfoPtr->patternNum == 0)
        {
            /* convert field info to global bit position */
            status = convertFieldInfoToGlobalBit(devNum,tableType,&fieldWordNum,&fieldOffset,&fieldLength,fieldValue);
            if(status != GT_OK)
            {
                return status;
            }
        }
        else    /* pattern entry */
        {
            /* convert index and pattern field info to new index and global bit position */
            status = convertPatternFieldInfoToGlobalBit(devNum, tableType, &entryIndex, &fieldWordNum, &fieldOffset, &fieldLength, fieldValue);
            if(status != GT_OK)
            {
                return status;
            }
        }
    }
    else if(fieldWordNum == PRV_CPSS_DXCH_TABLE_WORD_INDICATE_SUB_FIELD_IN_FIELD_NAME_CNS)
    {
        /* reverse the operation of PRV_CPSS_DXCH_TABLE_SUB_FIELD_OFFSET_AND_LENGTH_MAC */
        subFieldOffset = fieldLength & 0xFFFF;
        subFieldLength = fieldLength >> 16;

        /* convert field info to global bit position */
        status = convertSubFieldInfoToGlobalBit(devNum,tableType,subFieldOffset,subFieldLength,&fieldWordNum,&fieldOffset,&fieldLength,fieldValue);
        if(status != GT_OK)
        {
            return status;
        }
    }

    orig_entryIndex  = entryIndex;
    orig_fieldOffset = fieldOffset;
    orig_fieldWordNum = fieldWordNum;
    numEntriesPerLine = 0;


    if(fieldWordNum == PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS)
    {
        /* check if the table entry is actually implemented as
           'several entries' in single 'line' */
        prvCpssDxChTableEngineMultiEntriesInLineIndexAndGlobalBitConvert(devNum,tableType,
                    INOUT &entryIndex,INOUT &fieldOffset, NULL , OUT &numEntriesPerLine);

        /* the field of fieldOffset is used as 'Global offset' in the entry */
        if((entryIndex >= tableInfoPtr->maxNumOfEntries)    ||
           ((fieldOffset + fieldLength) > (entrySize * 32)) ||
           ((fieldLength == 0) || (fieldLength > 32)))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }

        /* check if the write is to single word or more */
        if(/*(fieldLength > 1) && */OFFSET_TO_WORD_MAC(fieldOffset) != OFFSET_TO_WORD_MAC(fieldOffset + fieldLength-1))
        {
            /* writing to more than single word in the entry */
            if(entrySize > PRV_CPSS_DXCH_TABLE_MAX_ENTRY_SIZE_CNS)
            {
                /* need to set PRV_CPSS_DXCH_TABLE_MAX_ENTRY_SIZE_CNS with larger value ?! */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }

            return prvCpssDxChPortGroupWriteTableEntryFieldList(devNum, portGroupId,
                tableType,orig_entryIndex,entryMemoBufArr,1,
                &orig_fieldOffset,&fieldLength,&fieldValue);
        }

        fieldWordNum = OFFSET_TO_WORD_MAC(fieldOffset);
        fieldOffset  = OFFSET_TO_BIT_MAC(fieldOffset);
    }
    else
    if((entryIndex >= tableInfoPtr->maxNumOfEntries)    ||
       ((fieldOffset + fieldLength) > 32)               ||
       (fieldLength == 0)                               ||
       (fieldWordNum >= entrySize))
    {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    /* Check if HW access required. */
    if(PRV_SHAREG_GLOBAL_TABLE_ACCESS_OBJECT.hwAccessTableEntryFieldWriteFunc != NULL)
    {
       status = PRV_SHAREG_GLOBAL_TABLE_ACCESS_OBJECT.hwAccessTableEntryFieldWriteFunc(
                                                  devNum, tmpPortGroupId, tableType,
                                                  entryIndex, fieldWordNum, fieldOffset,
                                                  fieldLength,fieldValue, originPortGroupId,
                                                  CPSS_DRV_HW_ACCESS_STAGE_PRE_E, status);
       if(status == GT_ABORTED)
        return GT_OK;

       if(status != GT_OK)
        return status;
    }

    if(tableInfoPtr->writeAccessType == PRV_CPSS_DXCH_INDIRECT_ACCESS_E)
    {
        /* Write a field to table by indirect access method. */
        status = prvCpssDxChWriteTableEntryField_indirect(devNum, portGroupId, tableType, tableInfoPtr,
                 entryIndex,fieldWordNum, fieldOffset, fieldLength, fieldValue);
    }
    else
    {
        /* Write a field to table by direct access method. */
        tablePtr =
             (PRV_CPSS_DXCH_TABLES_INFO_DIRECT_STC*)(tableInfoPtr->writeTablePtr);

        if(tablePtr->baseAddress == SPECIAL_ADDRESS_NOT_EXISTS_E)
        {
            /* the table not supported */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        /* read / write tableEntryField per port group */
        if(portGroupId == CPSS_PORT_GROUP_UNAWARE_MODE_CNS && PRV_CPSS_IS_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))
        {
            GT_PORT_GROUPS_BMP  portGroupsBmp;/* bmp of port groups */
            if(GT_TRUE ==
                prvCpssDuplicatedMultiPortGroupsGet(devNum,portGroupId,tablePtr->baseAddress,
                    &portGroupsBmp,NULL,NULL))
            {
                /* when the entry involve 'per port info' and 'global info'
                 (like vlan entry) between hemispheres , need to update each port group
                 according to it's existing info and not according to 'representative'
                 port group */
                PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum, portGroupsBmp , portGroupId)
                {

                    status = prvCpssDxChPortGroupWriteTableEntryField(
                        devNum, portGroupId, tableType,
                        orig_entryIndex, orig_fieldWordNum,
                        orig_fieldOffset, fieldLength, fieldValue);
                    if (status != GT_OK)
                    {
                        return status;
                    }
                }
                PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum, portGroupsBmp , portGroupId)

                contWriteTable = 0;
                status = GT_OK;
            }
        }
        if (contWriteTable)
        {
            /* In xCat3 and above devices the data is updated only when the last */
            /* word in the entry was written. */
            if(entrySize > PRV_CPSS_DXCH_TABLE_MAX_ENTRY_SIZE_CNS)
            {
                /* need to set PRV_CPSS_DXCH_TABLE_MAX_ENTRY_SIZE_CNS with larger value ?! */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
            address = tablePtr->baseAddress + entryIndex * tablePtr->step;
            /* read whole entry */
            status = prvCpssHwPpPortGroupReadRam(devNum, portGroupId, address, entrySize,
                                                 entryMemoBufArr);
            if (status != GT_OK)
            {
                return status;
            }

            /* update field */
            U32_SET_FIELD_MAC(entryMemoBufArr[fieldWordNum], fieldOffset, fieldLength, fieldValue);

            /* To stay shadow DB independent from possibly erroneous bits in HW,
               update only required part of shadow line instead of
               overriding full shadow line with value read from HW table */

            /* prepare mask for updating shadow table */
            cpssOsMemSet(entryMaskBuf, 0, sizeof(entryMaskBuf));
            raiseBitsInMemory(&entryMaskBuf[fieldWordNum], fieldOffset, fieldLength);


            /* update shadow table entry (if supported and requested) */
            status = prvCpssDxChPortGroupShadowLineUpdateMasked(devNum, portGroupId,
                                                                tableType,
                                                                entryIndex,
                                                                entryMemoBufArr,
                                                                entryMaskBuf);
            if (status != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(status, "update hw table shadow failed");
            }

            if(tableInfoPtr->writeAccessType == PRV_CPSS_DXCH_MGCAM_ACCESS_E)
            {
                status = prvCpssHwPpPortGroupMgCamWriteRam(devNum, portGroupId, address, entrySize,
                                                             entryMemoBufArr);
            }
            else
            {
                /* write whole HW entry */
                status = prvCpssHwPpPortGroupWriteRam(devNum, portGroupId, address, entrySize,
                                                                 entryMemoBufArr);
            }

            if (status != GT_OK)
            {
                return status;
            }
        } /*contWriteTable*/
    }

    if(PRV_SHAREG_GLOBAL_TABLE_ACCESS_OBJECT.hwAccessTableEntryFieldWriteFunc != NULL)
    {
       PRV_SHAREG_GLOBAL_TABLE_ACCESS_OBJECT.hwAccessTableEntryFieldWriteFunc(
                                                      devNum, tmpPortGroupId, tableType,
                                                      entryIndex, fieldWordNum, fieldOffset,
                                                      fieldLength,fieldValue, originPortGroupId,
                                                      CPSS_DRV_HW_ACCESS_STAGE_POST_E, status);

    }

    return status;
}

/**
* @internal prvCpssDxChHwTblAddrStcInfoGet function
* @endinternal
*
* @brief   This function return the address of the tables struct of cheetah devices.
*
* @note   APPLICABLE DEVICES:       xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*
* @param[out] tblAddrPtrPtr            - (pointer to) address of the tables struct
* @param[out] sizePtr                  - (pointer to) size of tables struct
*
* @retval GT_OK                    - on success, or
* @retval GT_BAD_PARAM             - wrong device type to operate
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChHwTblAddrStcInfoGet
(
    IN  GT_U8     devNum,
    OUT GT_U32    **tblAddrPtrPtr,
    OUT GT_U32    *sizePtr
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    *tblAddrPtrPtr = (GT_U32 *)PRV_CPSS_DXCH_DEV_TBLS_MAC(devNum);
    *sizePtr = PRV_CPSS_DXCH_TABLE_SIZE_GET_MAC(devNum);

    return GT_OK;
}

/**
* @internal prvCpssDxChWriteTableMultiEntry function
* @endinternal
*
* @brief   Write number of entries to the table in consecutive indexes.
*
* @param[in] devNum                   - the device number
* @param[in] tableType                - the specific table name
* @param[in] startIndex               - index to the first table entry
* @param[in] numOfEntries             - the number of consecutive entries to write
* @param[in] entryValueArrayPtr       - (pointer to) the data that will be written to the table
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChWriteTableMultiEntry
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_TABLE_ENT              tableType,
    IN GT_U32                           startIndex,
    IN GT_U32                           numOfEntries,
    IN GT_U32                           *entryValueArrayPtr
)
{
    GT_U32  ii;         /* loop iterator            */
    GT_U32  entrySize;  /* entry size in words      */
    GT_U32  rc;         /* return code              */
    PRV_CPSS_DXCH_TABLES_INFO_STC   *tableInfoPtr;

    CPSS_NULL_PTR_CHECK_MAC(entryValueArrayPtr);

    if( 0 == numOfEntries )
        return GT_OK;

    /* calculate entry size in words*/
    tableInfoPtr = PRV_TABLE_INFO_PTR_GET_MAC(devNum,tableType);
    entrySize = tableInfoPtr->entrySize;

    for( ii = startIndex ; ii < startIndex + numOfEntries ; ii++ )
    {
        rc = prvCpssDxChWriteTableEntry(devNum,
                                        tableType,
                                        ii,
                                        entryValueArrayPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
        entryValueArrayPtr+=entrySize;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChReadTableMultiEntry function
* @endinternal
*
* @brief   Read number of entries from the table in consecutive indexes.
*
* @param[in] devNum                   - the device number
* @param[in] tableType                - the specific table name
* @param[in] startIndex               - index to the first table entry
* @param[in] numOfEntries             - the number of consecutive entries to read
*
* @param[out] entryValueArrayPtr       - (pointer to) the data that will be read from the table
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChReadTableMultiEntry
(
    IN  GT_U8                            devNum,
    IN  CPSS_DXCH_TABLE_ENT              tableType,
    IN  GT_U32                           startIndex,
    IN  GT_U32                           numOfEntries,
    OUT GT_U32                           *entryValueArrayPtr
)
{
    GT_U32  ii;         /* loop iterator            */
    GT_U32  entrySize;  /* entry size in words      */
    GT_U32  rc;         /* return code              */
    PRV_CPSS_DXCH_TABLES_INFO_STC   *tableInfoPtr;

    CPSS_NULL_PTR_CHECK_MAC(entryValueArrayPtr);

    if( 0 == numOfEntries )
        return GT_OK;

    /* calculate entry size in words*/
    tableInfoPtr = PRV_TABLE_INFO_PTR_GET_MAC(devNum,tableType);
    entrySize = tableInfoPtr->entrySize;

    for( ii = startIndex ; ii < startIndex + numOfEntries ; ii++ )
    {
        rc = prvCpssDxChReadTableEntry(devNum,
                                       tableType,
                                       ii,
                                       entryValueArrayPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
        entryValueArrayPtr+=entrySize;
    }

    return GT_OK;
}

/*#define DXCH_TABLES_DEBUG*/
#ifdef DXCH_TABLES_DEBUG
#include <stdio.h>
GT_VOID debugPrintTable
(
     IN GT_U8  devNum
)
{
    const PRV_CPSS_DXCH_TABLES_INFO_STC   *tableInfoPtr;
    const PRV_CPSS_DXCH_TABLES_INFO_DIRECT_UNIT_STC * directAccessTableInfoPtr;
    FILE    *memObjFilePtr;             /* Dump memory file descriptor */
    char buffer[128];                  /* Read/Write buffer */
    GT_U32 ii;
    GT_U32 index;
    char *caption[]    = {"Index", "EntrySize", "EntryNums", "AccessType", "Address", "WordOffset", "Step"};
    char *directCaption[] =   {"Index", "EntrySize", "EntryNums", "Read/Write", "Address", "WordOffset", "Step"};
    char *indirectCaption[] = {"Index", "EntrySize", "EntryNums", "Read/Write", "CtrlolReg", "DataReg", "TriggerBit", "IndexBit", "specTblVal", "specTblBit", "ActionBit"};
    char *device[] = {"Bobcat2", "xCat3", "xCat","Bobcat3"};    /* Bobcat2/Caelum, Bobcat3, xCat3 or xCat device name */
    GT_U32   directAccessBobcat2TableInfoNumEntries = directAccessBobcat2TableInfoSize;

    GT_U32 tableInfoSize = 0;

    PRV_CPSS_DXCH_TABLES_INFO_DIRECT_STC *directAccessInfoPtr;
    PRV_CPSS_DXCH_TABLES_INFO_INDIRECT_STC *indirectAccessInfoPtr;

    /* Open file for write */
    memObjFilePtr = fopen("table_info.txt", "w+");
    if (memObjFilePtr == NULL)
    {
        cpssOsPrintf("File open error!!!\n");
    }

    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5_E)
    {
        tableInfoPtr = dxChXcat3TablesInfo;
        tableInfoSize = dxChXcat3TableInfoSize;
        ii = 1;
    }
    else if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
    {
        tableInfoPtr = dxChXcat3TablesInfo;
        tableInfoSize = dxChXcat3TableInfoSize;
        ii = 1;
    }
    else if ((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E) ||
             (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E))
    {
        CPSS_TBD_BOOKMARK_BOBCAT3
        directAccessTableInfoPtr = directAccessBobcat2TableInfo;
        tableInfoPtr = bobcat2TablesInfo;
        ii = 2;
    }
    else if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
    {
        tableInfoPtr = lion2TablesInfo;
        tableInfoSize = lion2TableInfoSize;
        ii = 3;
    }
    else
    {
        fprintf(memObjFilePtr, "Running device does not support <debugPrintTable> utility\n");
        fclose(memObjFilePtr);
        return;
    }

    cpssOsSprintf(buffer, "Device: %10s\n", device[ii]);
    fprintf(memObjFilePtr, buffer);

    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E ||
        PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E ||
        PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E ||
        PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E ||
        PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
    {
        cpssOsSprintf(buffer, "%10s %10s %10s %10s %10s %10s %10s\n",
                  caption[0], caption[1], caption[2], caption[3], caption[4],
                  caption[5], caption[6]);

        fprintf(memObjFilePtr, buffer);

        for(ii = 0 ; ii < directAccessBobcat2TableInfoNumEntries ; ii++)
        {
            if((directAccessTableInfoPtr[ii].directAccessInfo.baseAddress ==
                SPECIAL_ADDRESS_NOT_MODIFIED_E) ||
                ( directAccessTableInfoPtr[ii].directAccessInfo.baseAddress ==
                SPECIAL_ADDRESS_NOT_EXISTS_E))
            {
                /* no need to modify the entry (same as Lion2) */
                continue;
            }
            index = directAccessTableInfoPtr[ii].globalIndex;

            cpssOsSprintf(buffer, "%10d %10d %10d %10d 0x%08x %10d %10d\n",
                          index,
                          tableInfoPtr[index].entrySize,
                          tableInfoPtr[index].maxNumOfEntries,
                          tableInfoPtr[index].writeAccessType,
                          directAccessTableInfoPtr[ii].directAccessInfo.baseAddress,
                          directAccessTableInfoPtr[ii].directAccessInfo.nextWordOffset,
                          directAccessTableInfoPtr[ii].directAccessInfo.step);

            fprintf(memObjFilePtr, buffer);
        }
    }
    else /* CPSS_PP_FAMILY_DXCH_XCAT3_E, CPSS_PP_FAMILY_DXCH_XCAT_E */
    {
        fprintf(memObjFilePtr, "Direct Access Tables info:\n");

        cpssOsSprintf(buffer, "%10s %10s %10s %10s %10s %10s %10s\n",
                      directCaption[0], directCaption[1], directCaption[2],
                      directCaption[3], directCaption[4], directCaption[5],
                      directCaption[6]);

        fprintf(memObjFilePtr, buffer);

        fprintf(memObjFilePtr, "Indirect Access Tables info:\n");

        cpssOsSprintf(buffer, "%10s %10s %10s %10s %10s %10s %10s %10s %10s %10s %10s\n\n",
                      indirectCaption[0], indirectCaption[1], indirectCaption[2],
                      indirectCaption[3], indirectCaption[4], indirectCaption[5],
                      indirectCaption[6], indirectCaption[7], indirectCaption[8],
                      indirectCaption[9], indirectCaption[10]);

        fprintf(memObjFilePtr, buffer);

        for(ii = 0 ; ii < tableInfoSize ; ii++)
        {
            if( tableInfoPtr[ii].readTablePtr != NULL )
            {
                if( tableInfoPtr[ii].readAccessType ==
                                                PRV_CPSS_DXCH_DIRECT_ACCESS_E )
                {
                    directAccessInfoPtr = (PRV_CPSS_DXCH_TABLES_INFO_DIRECT_STC *)
                                                     tableInfoPtr[ii].readTablePtr;
                    cpssOsSprintf(buffer, "%10d %10d %10d %10s 0x%08x %10d %10d\n",
                              ii,
                              tableInfoPtr[ii].entrySize,
                              tableInfoPtr[ii].maxNumOfEntries,
                              "Read",
                              directAccessInfoPtr->baseAddress,
                              directAccessInfoPtr->nextWordOffset,
                              directAccessInfoPtr->step);

                    fprintf(memObjFilePtr, buffer);
                }

                if( tableInfoPtr[ii].readAccessType ==
                                                PRV_CPSS_DXCH_INDIRECT_ACCESS_E )
                {
                    indirectAccessInfoPtr = (PRV_CPSS_DXCH_TABLES_INFO_INDIRECT_STC *)
                                                     tableInfoPtr[ii].readTablePtr;
                    cpssOsSprintf(buffer, "%10d %10d %10d %10s 0x%08x 0x%08x %10d %10d %10d %10d %10d\n",
                              ii,
                              tableInfoPtr[ii].entrySize,
                              tableInfoPtr[ii].maxNumOfEntries,
                              "I-Read",
                              indirectAccessInfoPtr->controlReg,
                              indirectAccessInfoPtr->dataReg,
                              indirectAccessInfoPtr->trigBit,
                              indirectAccessInfoPtr->indexBit,
                              indirectAccessInfoPtr->specificTableValue,
                              indirectAccessInfoPtr->specificTableBit,
                              indirectAccessInfoPtr->actionBit);

                    fprintf(memObjFilePtr, buffer);
                }
            }

            if( tableInfoPtr[ii].writeTablePtr != NULL )
            {
                if( tableInfoPtr[ii].writeAccessType ==
                                                PRV_CPSS_DXCH_DIRECT_ACCESS_E )
                {
                    directAccessInfoPtr = (PRV_CPSS_DXCH_TABLES_INFO_DIRECT_STC *)
                                                     tableInfoPtr[ii].writeTablePtr;
                    cpssOsSprintf(buffer, "%10d %10d %10d %10s 0x%08x %10d %10d\n",
                              ii,
                              tableInfoPtr[ii].entrySize,
                              tableInfoPtr[ii].maxNumOfEntries,
                              "Write",
                              directAccessInfoPtr->baseAddress,
                              directAccessInfoPtr->nextWordOffset,
                              directAccessInfoPtr->step);

                    fprintf(memObjFilePtr, buffer);
                }



                if( tableInfoPtr[ii].writeAccessType ==
                                                PRV_CPSS_DXCH_INDIRECT_ACCESS_E )
                {
                    indirectAccessInfoPtr = (PRV_CPSS_DXCH_TABLES_INFO_INDIRECT_STC *)
                                                     tableInfoPtr[ii].writeTablePtr;
                    cpssOsSprintf(buffer, "%10d %10d %10d %10s 0x%08x 0x%08x %10d %10d %10d %10d %10d\n",
                              ii,
                              tableInfoPtr[ii].entrySize,
                              tableInfoPtr[ii].maxNumOfEntries,
                              "I-Write",
                              indirectAccessInfoPtr->controlReg,
                              indirectAccessInfoPtr->dataReg,
                              indirectAccessInfoPtr->trigBit,
                              indirectAccessInfoPtr->indexBit,
                              indirectAccessInfoPtr->specificTableValue,
                              indirectAccessInfoPtr->specificTableBit,
                              indirectAccessInfoPtr->actionBit);

                    fprintf(memObjFilePtr, buffer);
                }
            }
        }
    }

    fclose(memObjFilePtr);
}
#endif

/**
* @internal prvCpssDxChTableBaseAddrGet function
* @endinternal
*
* @brief   This function retrieves base address of table.(for 'Direct access' tables)
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] devNum                   - the device number
* @param[in] tableType                - the specific table name
*
* @param[out] errorPtr                 - (pointer to) indication that function did error. (can be NULL)
*
* @retval on success               - return the base address
* @retval on error                 - return address that ((address % 4) != 0)
*/
GT_U32  prvCpssDxChTableBaseAddrGet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_TABLE_ENT          tableType,
    OUT GT_BOOL                         *errorPtr
)
{
    PRV_CPSS_DXCH_TABLES_INFO_STC *tableInfoPtr; /* pointer to table info */
    PRV_CPSS_DXCH_TABLES_INFO_DIRECT_STC *tablePtr;/* pointer to direct table info */
    GT_U32      address;

    tableInfoPtr = PRV_TABLE_INFO_PTR_GET_MAC(devNum,tableType);
    if(tableInfoPtr->writeAccessType == PRV_CPSS_DXCH_INDIRECT_ACCESS_E)
    {
        if(errorPtr)
        {
            *errorPtr = GT_TRUE;
        }
        return SPECIAL_ADDRESS_NOT_EXISTS_E;
    }
    tablePtr =
         (PRV_CPSS_DXCH_TABLES_INFO_DIRECT_STC*)(tableInfoPtr->writeTablePtr);

    address = tablePtr->baseAddress;
    if(errorPtr)
    {
        if(address == SPECIAL_ADDRESS_NOT_EXISTS_E)
        {
            *errorPtr = GT_TRUE;
        }
        else
        {
            *errorPtr = GT_FALSE;
        }
    }

    return address;
}

/**
* @internal prvCpssDxChTablePortsBmpCheck function
* @endinternal
*
* @brief   This function checks that the bmp of ports can be supported by the device.
*         assumption that it is bmp of port of 'this' device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portsBmpPtr              - pointer to the bmp of ports
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - when bmp hold ports that not supported by the device
*/
GT_STATUS   prvCpssDxChTablePortsBmpCheck
(
    IN GT_U8                     devNum,
    IN CPSS_PORTS_BMP_STC       *portsBmpPtr
)
{
    return prvCpssDxChTablePortsBmpLimitedNumCheck(devNum,0xFFFFFFFF,portsBmpPtr);
}

/**
* @internal prvCpssDxChTablePortsBmpLimitedNumCheck function
* @endinternal
*
* @brief   This function checks that the bmp of ports can be supported by the device.
*         assumption that it is bmp of port of 'this' device.
*         with limited number of ports that supported
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] limitedNum               - limited number of ports in the BMP that allowed
*                                      to be different from zero.
* @param[in] portsBmpPtr              - pointer to the bmp of ports
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - when bmp hold ports that not supported by the device or
*                                       bit is set over 'limitedNum'
*/
GT_STATUS   prvCpssDxChTablePortsBmpLimitedNumCheck
(
    IN GT_U8                     devNum,
    IN GT_U32                    limitedNum,
    IN CPSS_PORTS_BMP_STC       *portsBmpPtr
)
{
    /* assume that device is DX and exists */
    CPSS_PORTS_BMP_STC          existingPorts;
    CPSS_PORTS_BMP_STC          tmpPortsBmp;
    GT_U32  numOfPorts = MIN(limitedNum,(PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_PHY_PORT_MAC(devNum)+1));

    /* check that pointer is valid */
    CPSS_NULL_PTR_CHECK_MAC(portsBmpPtr);

    /* get the bmp of supported port on this device */
    prvCpssDrvPortsFullMaskGet(
            numOfPorts ,
            &existingPorts);

    /* check if the IN bmp hold ports that are not in the supported ports */
    CPSS_PORTS_BMP_BITWISE_AND_NOT_MAC(&tmpPortsBmp,portsBmpPtr,&existingPorts);

    if(PRV_CPSS_PORTS_BMP_IS_ZERO_MAC(&tmpPortsBmp))
    {
        return GT_OK;
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
        "The bitmap of ports hold 'set' bits over the supported amount[%d]",
        numOfPorts);
}

/**
* @internal prvCpssDxChTablePortsBmpAdjustToDevice function
* @endinternal
*
* @brief   This function removes none exists ports from bmp of ports.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] IN_portsBmpPtr           - pointer to the bmp of ports
*
* @param[out] OUT_portsBmpPtr          - pointer to the bmp of ports , that hold no ports
*                                      that not supported by the device
*
* @retval GT_OK                    - on success
*/
GT_STATUS   prvCpssDxChTablePortsBmpAdjustToDevice
(
    IN GT_U8                     devNum,
    IN  CPSS_PORTS_BMP_STC       *IN_portsBmpPtr,
    OUT CPSS_PORTS_BMP_STC       *OUT_portsBmpPtr
)
{
    return prvCpssDxChTablePortsBmpAdjustToDeviceLimitedNum(devNum,0xFFFFFFFF,IN_portsBmpPtr,OUT_portsBmpPtr);
}

/**
* @internal prvCpssDxChTablePortsBmpAdjustToDeviceLimitedNum function
* @endinternal
*
* @brief   This function removes none exists ports from bmp of ports.
*         with limited number of ports that supported
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] limitedNum               - limited number of ports in the BMP that allowed
*                                      to be different from zero.
* @param[in] IN_portsBmpPtr           - pointer to the bmp of ports
*
* @param[out] OUT_portsBmpPtr          - pointer to the bmp of ports , that hold no ports
*                                      that not supported by the device
*
* @retval GT_OK                    - on success
*/
GT_STATUS   prvCpssDxChTablePortsBmpAdjustToDeviceLimitedNum
(
    IN GT_U8                     devNum,
    IN GT_U32                    limitedNum,
    IN  CPSS_PORTS_BMP_STC       *IN_portsBmpPtr,
    OUT CPSS_PORTS_BMP_STC       *OUT_portsBmpPtr
)
{
    /* assume that device is DX and exists */
    CPSS_PORTS_BMP_STC          existingPorts;
    GT_U32  numOfPorts = MIN(limitedNum,(PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum)));

    /* get the bmp of supported port on this device */
    prvCpssDrvPortsFullMaskGet(
        numOfPorts ,
            &existingPorts);

    /* remove from IN bmp ports that are not in the supported ports ..
        and set result into 'OUT' bmp */
    CPSS_PORTS_BMP_BITWISE_AND_MAC(OUT_portsBmpPtr,IN_portsBmpPtr,&existingPorts);

    return GT_OK;
}


/**
* @internal tablePlrMeterFieldInfoGet function
* @endinternal
*
* @brief   This function convert field for the 'generic meter' entry to :
*         1. the exact table type to use
*         2. the exact field name to use
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] fieldName                - name of the field to set
*
* @param[out] tableIdPtr               - pointer to table id of the actual table:
*                                      0 - meter entry (token bucket in sip 5_15)
*                                      1 - meter config entry (sip5_15 only)
* @param[out] convertedFieldNamePtr    - pointer to the field name in the actual table
*
* @retval GT_OK                    - on success
*/
static GT_STATUS   tablePlrMeterFieldInfoGet
(
    IN GT_U8                    devNum,
    IN SIP5_PLR_METERING_TABLE_FIELDS_ENT      fieldName,
    OUT GT_U32                  *tableIdPtr,
    OUT GT_U32                  *convertedFieldNamePtr
)
{
    GT_U32  converted_fieldName = 0xFFFFFFFF; /* dummy initialization */

    *tableIdPtr = 0; /* the 'meter' entry */
    *convertedFieldNamePtr = fieldName;

    switch(fieldName)
    {
        case SIP5_PLR_METERING_TABLE_FIELDS_LAST_UPDATE_TIME0_E          :
        case SIP5_PLR_METERING_TABLE_FIELDS_LAST_UPDATE_TIME1_E          :
        case SIP5_PLR_METERING_TABLE_FIELDS_WRAP_AROUND_INDICATOR0_E     :
        case SIP5_PLR_METERING_TABLE_FIELDS_WRAP_AROUND_INDICATOR1_E     :
        case SIP5_PLR_METERING_TABLE_FIELDS_BUCKET_SIZE0_E               :
        case SIP5_PLR_METERING_TABLE_FIELDS_BUCKET_SIZE1_E               :
        case SIP5_PLR_METERING_TABLE_FIELDS_RATE_TYPE0_E                 :
        case SIP5_PLR_METERING_TABLE_FIELDS_RATE_TYPE1_E                 :
        case SIP5_PLR_METERING_TABLE_FIELDS_RATE0_E                      :
        case SIP5_PLR_METERING_TABLE_FIELDS_RATE1_E                      :
        case SIP5_PLR_METERING_TABLE_FIELDS_MAX_BURST_SIZE0_E            :
        case SIP5_PLR_METERING_TABLE_FIELDS_MAX_BURST_SIZE1_E            :
            return GT_OK;
        /* sip5_15 only fields */
        case SIP5_15_PLR_METERING_TABLE_FIELDS_BUCKET_0_PRIORITY_E        :
        case SIP5_15_PLR_METERING_TABLE_FIELDS_BUCKET_1_PRIORITY_E        :
        case SIP5_15_PLR_METERING_TABLE_FIELDS_BUCKET_0_COLOR_E           :
        case SIP5_15_PLR_METERING_TABLE_FIELDS_BUCKET_1_COLOR_E           :
        case SIP5_15_PLR_METERING_TABLE_FIELDS_COUPLING_FLAG_E            :
        case SIP5_15_PLR_METERING_TABLE_FIELDS_COUPLING_FLAG_0_E          :
        case SIP5_15_PLR_METERING_TABLE_FIELDS_EIR_AND_CIR_MAX_INDEX_E    :
            if(! PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
            }
            return GT_OK;

        case SIP5_PLR_METERING_TABLE_FIELDS_COLOR_MODE_E                          :     /* in sip5_15 moved to 'Metering Configuration Entry' */
            converted_fieldName =    SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_COLOR_MODE_E                           ;
            break;
        case SIP5_PLR_METERING_TABLE_FIELDS_POLICER_MODE_E                        :     /* in sip5_15 moved to 'Metering Configuration Entry' */
            converted_fieldName =    SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_POLICER_MODE_E                         ;
            break;
        case SIP5_PLR_METERING_TABLE_FIELDS_MG_COUNTERS_SET_EN_E                  :     /* in sip5_15 moved to 'Metering Configuration Entry' */
            converted_fieldName =    SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_MG_COUNTERS_SET_EN_E                   ;
            break;
        case SIP5_PLR_METERING_TABLE_FIELDS_BILLING_PTR_E                         :     /* in sip5_15 moved to 'Metering Configuration Entry' */
            converted_fieldName =    SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_BILLING_PTR_E                          ;
            break;
        case SIP5_PLR_METERING_TABLE_FIELDS_BYTE_OR_PACKET_COUNTING_MODE_E        :     /* in sip5_15 moved to 'Metering Configuration Entry' */
            converted_fieldName =    SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_BYTE_OR_PACKET_COUNTING_MODE_E         ;
            break;
        case SIP5_PLR_METERING_TABLE_FIELDS_PACKET_SIZE_MODE_E                    :     /* in sip5_15 moved to 'Metering Configuration Entry' */
            converted_fieldName =    SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_PACKET_SIZE_MODE_E                     ;
            break;
        case SIP5_PLR_METERING_TABLE_FIELDS_TUNNEL_TERMINATION_PACKET_SIZE_MODE_E :     /* in sip5_15 moved to 'Metering Configuration Entry' */
            converted_fieldName =    SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_TUNNEL_TERMINATION_PACKET_SIZE_MODE_E  ;
            break;
        case SIP5_PLR_METERING_TABLE_FIELDS_INCLUDE_LAYER1_OVERHEAD_E             :     /* in sip5_15 moved to 'Metering Configuration Entry' */
            converted_fieldName =    SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_INCLUDE_LAYER1_OVERHEAD_E              ;
            break;
        case SIP5_PLR_METERING_TABLE_FIELDS_DSA_TAG_COUNTING_MODE_E               :     /* in sip5_15 moved to 'Metering Configuration Entry' */
            converted_fieldName =    SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_DSA_TAG_COUNTING_MODE_E                ;
            break;
        case SIP5_PLR_METERING_TABLE_FIELDS_TIMESTAMP_TAG_COUNTING_MODE_E         :     /* in sip5_15 moved to 'Metering Configuration Entry' */
            converted_fieldName =    SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_TIMESTAMP_TAG_COUNTING_MODE_E          ;
            break;
        /* ingress only fields*/
        case SIP5_PLR_METERING_TABLE_FIELDS_INGRESS_RED_CMD_E                     :     /* in sip5_15 moved to 'Metering Configuration Entry' */
            converted_fieldName =    SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_INGRESS_RED_CMD_E                      ;
            break;
        case SIP5_PLR_METERING_TABLE_FIELDS_INGRESS_YELLOW_CMD_E                  :     /* in sip5_15 moved to 'Metering Configuration Entry' */
            converted_fieldName =    SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_INGRESS_YELLOW_CMD_E                   ;
            break;
        case SIP5_PLR_METERING_TABLE_FIELDS_INGRESS_GREEN_CMD_E                   :     /* in sip5_15 moved to 'Metering Configuration Entry' */
            converted_fieldName =    SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_INGRESS_GREEN_CMD_E                    ;
            break;
        case SIP5_PLR_METERING_TABLE_FIELDS_INGRESS_MODIFY_DSCP_E                 :     /* in sip5_15 moved to 'Metering Configuration Entry' */
            converted_fieldName =    SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_INGRESS_MODIFY_DSCP_E                  ;
            break;
        case SIP5_PLR_METERING_TABLE_FIELDS_INGRESS_MODIFY_UP_E                   :     /* in sip5_15 moved to 'Metering Configuration Entry' */
            converted_fieldName =    SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_INGRESS_MODIFY_UP_E                    ;
            break;
        case SIP5_PLR_METERING_TABLE_FIELDS_INGRESS_QOS_PROFILE_E                 :     /* in sip5_15 moved to 'Metering Configuration Entry' */
            converted_fieldName =    SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_INGRESS_QOS_PROFILE_E                  ;
            break;
        /* egress only fields*/                                                                                           /* egress only fields*/
        case SIP5_PLR_METERING_TABLE_FIELDS_EGRESS_REMARK_MODE_E                  :     /* in sip5_15 moved to 'Metering Configuration Entry' */
            converted_fieldName =    SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_EGRESS_REMARK_MODE_E                   ;
            break;
        case SIP5_PLR_METERING_TABLE_FIELDS_EGRESS_DROP_RED_E                     :     /* in sip5_15 moved to 'Metering Configuration Entry' */
            converted_fieldName =    SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_EGRESS_DROP_RED_E                      ;
            break;
        case SIP5_PLR_METERING_TABLE_FIELDS_EGRESS_EN_MODIFY_EXP_E                :     /* in sip5_15 moved to 'Metering Configuration Entry' */
            converted_fieldName =    SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_EGRESS_EN_MODIFY_EXP_E                 ;
            break;
        case SIP5_PLR_METERING_TABLE_FIELDS_EGRESS_EN_MODIFY_DSCP_E               :     /* in sip5_15 moved to 'Metering Configuration Entry' */
            converted_fieldName =    SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_EGRESS_EN_MODIFY_DSCP_E                ;
            break;
        case SIP5_PLR_METERING_TABLE_FIELDS_EGRESS_EN_MODIFY_TC_E                 :     /* in sip5_15 moved to 'Metering Configuration Entry' */
            converted_fieldName =    SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_EGRESS_EN_MODIFY_TC_E                  ;
            break;
        case SIP5_PLR_METERING_TABLE_FIELDS_EGRESS_EN_MODIFY_UP_E                 :     /* in sip5_15 moved to 'Metering Configuration Entry' */
            converted_fieldName =    SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_EGRESS_EN_MODIFY_UP_E                  ;
            break;
        case SIP5_PLR_METERING_TABLE_FIELDS_EGRESS_EN_MODIFY_DP_E                 :     /* in sip5_15 moved to 'Metering Configuration Entry' */
            converted_fieldName =    SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_EGRESS_EN_MODIFY_DP_E                  ;
            break;
        case SIP5_PLR_METERING_TABLE_FIELDS_EGRESS_EN_YELLOW_ECN_MARKING_E        :     /* in sip5_15 moved to 'Metering Configuration Entry' */
            converted_fieldName =    SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_EGRESS_EN_YELLOW_ECN_MARKING_E         ;
            break;

        case SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_INGRESS_MEF_10_3_ENV_SIZE_E___EMULATION_ONLY_E:
            if(! PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
            }
            converted_fieldName =    SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_INGRESS_MEF_10_3_ENV_SIZE_E;
            break;

        case SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_EGRESS_MEF_10_3_ENV_SIZE_E___EMULATION_ONLY_E:
            if(! PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
            }
            converted_fieldName =    SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_EGRESS_MEF_10_3_ENV_SIZE_E;
            break;

        case SIP_6_PLR_METERING_CONFIG_TABLE_FIELDS_INGRESS_GREEN_MIRROR_EN___EMULATION_ONLY_E:
            /* field form Metering Configuration Entry */
            if(! PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
            }
            converted_fieldName = SIP_6_PLR_METERING_CONFIG_TABLE_FIELDS_INGRESS_GREEN_MIRROR_EN_E;
            break;

        case SIP_6_PLR_METERING_CONFIG_TABLE_FIELDS_EGRESS_GREEN_MIRROR_EN___EMULATION_ONLY_E:
            /* field form Metering Configuration Entry */
            if(! PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
            }
            converted_fieldName = SIP_6_PLR_METERING_CONFIG_TABLE_FIELDS_EGRESS_GREEN_MIRROR_EN_E;
            break;

        case SIP_6_PLR_METERING_CONFIG_TABLE_FIELDS_INGRESS_YELLOW_MIRROR_EN___EMULATION_ONLY_E:
            if(! PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
            }
            converted_fieldName = SIP_6_PLR_METERING_CONFIG_TABLE_FIELDS_INGRESS_YELLOW_MIRROR_EN_E;
            break;

        case SIP_6_PLR_METERING_CONFIG_TABLE_FIELDS_EGRESS_YELLOW_MIRROR_EN___EMULATION_ONLY_E:
            if(! PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
            }
            converted_fieldName = SIP_6_PLR_METERING_CONFIG_TABLE_FIELDS_EGRESS_YELLOW_MIRROR_EN_E;
            break;

        case SIP_6_PLR_METERING_CONFIG_TABLE_FIELDS_INGRESS_RED_MIRROR_EN___EMULATION_ONLY_E:
            if(! PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
            }
            converted_fieldName = SIP_6_PLR_METERING_CONFIG_TABLE_FIELDS_INGRESS_RED_MIRROR_EN_E;
            break;

        case SIP_6_PLR_METERING_CONFIG_TABLE_FIELDS_EGRESS_RED_MIRROR_EN___EMULATION_ONLY_E:
            if(! PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
            }
            converted_fieldName = SIP_6_PLR_METERING_CONFIG_TABLE_FIELDS_EGRESS_RED_MIRROR_EN_E;
            break;

        default:    /* should not get here ... which field is it ?! */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        *tableIdPtr = 1;/* 'meter config' table */
        *convertedFieldNamePtr = converted_fieldName;
    }
    else
    {
        /* we already stated , at start of the function:
            *tableIdPtr = 0;
            *convertedFieldNamePtr = fieldName;
        */
    }


    return GT_OK;
}


/**
* @internal prvCpssDxChTablePlrMeterFieldBuild function
* @endinternal
*
* @brief   This function build the value into needed field in the proper buffer.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] hwMeterArr[]             - pointer to 'meter' entry (in 5_15 'token bucket' entry)
* @param[in] hwMeterConfigArr[]       - pointer to 'meter config' entry (relevant only to 5_15)
* @param[in] fieldName                - name of the field to set
* @param[in] value                    -  to set
*
* @retval GT_OK                    - on success
* @retval GT_NOT_IMPLEMENTED       - on non implemented case
* @retval GT_NOT_APPLICABLE_DEVICE - on non relevant field to the device
* @retval GT_BAD_PARAM             - on value that not fit into field
*/
GT_STATUS   prvCpssDxChTablePlrMeterFieldBuild
(
    IN GT_U8                    devNum,
    IN GT_U32                   hwMeterArr[],
    IN GT_U32                   hwMeterConfigArr[],
    IN SIP5_PLR_METERING_TABLE_FIELDS_ENT      fieldName,
    IN GT_U32                   value
)
{
    GT_STATUS   rc;
    GT_U32  converted_fieldName = 0;
    GT_U32  tableId = 0;

    /* convert field into proper table and proper field */
    rc = tablePlrMeterFieldInfoGet(devNum,fieldName,&tableId,&converted_fieldName);
    if(rc != GT_OK)
    {
        return rc;
    }

    switch(tableId)
    {
        case 0:
            /* field in 'meter' entry (in 5_15 'token bucket' entry) */
            __SIP5_PLR_METER_FIELD_SET_MAC(devNum,hwMeterArr,converted_fieldName,value);
            break;
        case 1:
            /* field in 'meter config' entry (relevant only to 5_15) */
            __SIP5_15_PLR_METER_CONFIG_FIELD_SET_MAC(devNum,hwMeterConfigArr,converted_fieldName,value);
            break;
        default: /* which table is it ?*/
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}


/**
* @internal prvCpssDxChTablePlrMeterFieldParse function
* @endinternal
*
* @brief   This function parse the value for needed field from the proper buffer.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] hwMeterArr[]             - pointer to 'meter' entry (in 5_15 'token bucket' entry)
* @param[in] hwMeterConfigArr[]       - pointer to 'meter config' entry (relevant only to 5_15)
* @param[in] fieldName                - name of the field to set
*
* @retval GT_OK                    - on success
* @retval GT_NOT_IMPLEMENTED       - on non implemented case
* @retval GT_NOT_APPLICABLE_DEVICE - on non relevant field to the device
* @retval GT_BAD_PARAM             - on value that not fit into field
*/
GT_STATUS   prvCpssDxChTablePlrMeterFieldParse
(
    IN GT_U8                    devNum,
    IN GT_U32                   hwMeterArr[],
    IN GT_U32                   hwMeterConfigArr[],
    IN SIP5_PLR_METERING_TABLE_FIELDS_ENT      fieldName,
    IN GT_U32                   *valuePtr
)
{
    GT_STATUS   rc;
    GT_U32  converted_fieldName = 0;
    GT_U32  tableId = 0;
    GT_U32  value;

    /* convert field into proper table and proper field */
    rc = tablePlrMeterFieldInfoGet(devNum,fieldName,&tableId,&converted_fieldName);
    if(rc != GT_OK)
    {
        return rc;
    }

    switch(tableId)
    {
        case 0:
            /* field in 'meter' entry (in 5_15 'token bucket' entry) */
            __SIP5_PLR_METER_FIELD_GET_MAC(devNum,hwMeterArr,converted_fieldName,value);
            break;
        case 1:
            /* field in 'meter config' entry (relevant only to 5_15) */
            __SIP5_15_PLR_METER_CONFIG_FIELD_GET_MAC(devNum,hwMeterConfigArr,converted_fieldName,value);
            break;
        default: /* which table is it ?*/
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);
    }

    *valuePtr = value;

    return GT_OK;
}

/**
* @internal prvCpssDxChTableEngineMultiEntriesInLineIndexAndGlobalBitConvert function
* @endinternal
*
* @brief   check if the table entry is actually implemented as
*         'several entries' in single 'line'
*         NOTE: if the table is not of type 'multi entries' in line , the function
*         not update the INOUT parameters
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] tableType                - the specific table name
* @param[in,out] entryIndexPtr            - (pointer to) entry index , as 'unaware' that there are
*                                      multi entries in single line.
* @param[in,out] fieldOffsetPtr           - (pointer to) offset of the 'field' in the entry ,
*                                      as 'unaware' that there are multi entries in single line.
* @param[in,out] entryIndexPtr            - (pointer to) LINE index , as 'aware' that there are
*                                      multi entries in single line.
*                                      0x0FFFFFFF means impossible value!
* @param[in,out] fieldOffsetPtr           - (pointer to) offset of the 'field' in the LINE ,
*                                      as 'aware' that there are multi entries in single line.
*
* @param[out] numBitsPerEntryPtr       - (pointer to) the number of bits that the 'entry' is
*                                      using in the LINE.
*                                      NOTE: can be NULL
* @param[out] numEntriesPerLinePtr     - (pointer to) the number of entries in single LINE.
*                                      NOTE: can be NULL
*                                      NOTE: if value holds FRACTION_INDICATION_CNS meaning that the
*                                      number in the lower 31 bits is 'fraction' (1/x) and not (x)
*
* @retval GT_TRUE                  - convert done
* @retval GT_FALSE                 - no convert done
*
* @note Note that numBitsPerEntry numEntriesPerLine is not necessary
*       equal to table line length. For example, in bobcat3
*       CPSS_DXCH_SIP5_TABLE_ERMRK_PTP_LOCAL_ACTION_E table line's length is 30
*       bits: 5 entries (profiles 0..4) by 6 bits.
*       Whereas the function returns numEntriesPerLinePtr = 8.
*
*/
GT_BOOL prvCpssDxChTableEngineMultiEntriesInLineIndexAndGlobalBitConvert
(
    IN GT_U8                   devNum,
    IN CPSS_DXCH_TABLE_ENT tableType,
    INOUT GT_U32               *entryIndexPtr,
    INOUT GT_U32               *fieldOffsetPtr,
    OUT GT_U32                 *numBitsPerEntryPtr,
    OUT GT_U32                 *numEntriesPerLinePtr
)
{
    GT_U32  numBitsPerEntry;/* number of bits per entry in the LINE */
    GT_U32  numEntriesPerLine;/* number of entries per line */
    GT_U32  fractionValue;/*fraction value*/
    GT_U32  fieldOffset;/* field offset */
    GT_U32  entryIndex; /* entry index */
    GT_BOOL reverseOrder = GT_FALSE;
    GT_U32  sip6_chunkSize = 1;

    switch(tableType)
    {
        case CPSS_DXCH_SIP5_TABLE_IPVX_INGRESS_EPORT_E:
            if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
            {
                numBitsPerEntry = 7;
            }
            else
            {
                numBitsPerEntry = 8;
            }
            numEntriesPerLine = 16;
            break;

        case CPSS_DXCH_SIP5_TABLE_IPVX_ROUTER_NEXTHOP_AGE_BITS_E:
            /* the API return manipulate only FULL LINE and not the 'NH age bit'
            numBitsPerEntry = 1;
            numEntriesPerLine = 32;
            */
            return GT_FALSE;

        case CPSS_DXCH_SIP5_TABLE_EQ_TX_PROTECTION_E:
            numBitsPerEntry = 1;
            numEntriesPerLine = 32;
            break;
        case CPSS_DXCH_SIP5_TABLE_EQ_EPORT_TO_LOC_MAPPING_E:
            numBitsPerEntry = ( PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) ? 11 :
                                PRV_CPSS_SIP_6_CHECK_MAC(devNum)    ? 10 :
                                PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) ? 13 : 11);
            numEntriesPerLine = 2;
            break;
        case CPSS_DXCH_SIP5_TABLE_EQ_PROTECTION_LOC_E:
            numBitsPerEntry = 1;
            numEntriesPerLine = 32;
            break;
        case CPSS_DXCH_SIP5_20_TABLE_EQ_TX_PROTECTION_LOC_E:
            numBitsPerEntry = 1;
            numEntriesPerLine = 32;
            break;

        case CPSS_DXCH_SIP5_TABLE_EQ_INGRESS_EPORT_E:
            if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
            {
                numBitsPerEntry = 7;
            }
            else
            {
                numBitsPerEntry = 8;
            }
            numEntriesPerLine = 4;
            break;

        case CPSS_DXCH_SIP5_TABLE_BRIDGE_INGRESS_EPORT_LEARN_PRIO_E:
            numBitsPerEntry = 5;
            numEntriesPerLine = 8;
            break;

        case CPSS_DXCH_SIP5_TABLE_INGRESS_BRIDGE_PORT_MEMBERS_E:
        case CPSS_DXCH_LION_TABLE_STG_INGRESS_E:
            if(!PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
            {
                return GT_FALSE;
            }

            /* ironman supports only 64 ports */
            if(CPSS_PP_FAMILY_DXCH_IRONMAN_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
            {
                /* so no multi entries in line issues */
                return GT_FALSE;
            }


            if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) &&
               (CPSS_DXCH_LION_TABLE_STG_INGRESS_E == tableType))
            {
                /* the table became usual one in Hawk */
                return GT_FALSE;
            }

            if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                numBitsPerEntry = BMP_128_PORTS_CNS;

                numEntriesPerLine = 8 | VERTICAL_INDICATION_CNS;/* 8 entries of 128 bit in line of 1024 bit */
            }
            else if (PRV_CPSS_DXCH_ALDRIN2_CHECK_MAC(devNum))/* Aldrin2 */
            {
                numBitsPerEntry = BMP_128_PORTS_CNS;
                numEntriesPerLine = 1;
            }
            else
            {
                numBitsPerEntry = BMP_256_PORTS_CNS;
                numEntriesPerLine = 2 | VERTICAL_INDICATION_CNS;
            }

            if(CPSS_DXCH_LION_TABLE_STG_INGRESS_E == tableType)
            {
                numBitsPerEntry *= 2;/*2 bit per port*/
            }

            break;
        case CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_EGRESS_VLAN_E:
            if(!PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
            {
                return GT_FALSE;
            }

            /* we have only 16 ports per line ... which is actually 1/16 of the entry */
            /* so this is special entry that takes more than single line */
            numBitsPerEntry = SIP5_20_EGF_QAG_EGRESS_EVLAN_BITS_SIZE_CNS;

            /* ironman supports only 64 port so only 4 lines (each 16 ports) needed
               for single vlan */
            if(CPSS_PP_FAMILY_DXCH_IRONMAN_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
            {
                /* indicate that the 4 is actually 'fraction' (1/4) */
                numEntriesPerLine  = FRACTION_INDICATION_CNS | 4;
            }
            else
            /* falcon: no half mode is 128 ports mode, (128 /(48/3) ) = 128/16 = 8,
               so for table mode 128 ports, each entry (with logical index evlan) required 8 lines */
            if(PRV_CPSS_SIP_5_25_CHECK_MAC(devNum))
            {
                /* indicate that the 8 is actually 'fraction' (1/8) */
                numEntriesPerLine  = FRACTION_INDICATION_CNS | 8;
            }
            else
            {
                /* indicate that the 16 is actually 'fraction' (1/16) */
                numEntriesPerLine  = FRACTION_INDICATION_CNS | 16;
            }
            break;

        case CPSS_DXCH_SIP5_TABLE_ERMRK_PTP_LOCAL_ACTION_E:
            if(!PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
            {
                return GT_FALSE;
            }
            /* the actual 5 domains (out of 8 range) are in single line */
            numEntriesPerLine = 8;
            numBitsPerEntry = 6;
            break;

        case CPSS_DXCH_LION_TABLE_STG_EGRESS_E:
        case CPSS_DXCH_LION_TABLE_VLAN_EGRESS_E:
        case CPSS_DXCH_LION_TABLE_TXQ_SOURCE_ID_MEMBERS_E:
        case CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_SHT_NON_TRUNK_MEMBERS2_E:
        case CPSS_DXCH_TABLE_MULTICAST_E:
            if(0 == PRV_CPSS_DXCH_BMP_OF_TABLES_IN_HALF_SIZE_MODE_GET_MAC(devNum,tableType))
            {
                /* port mode 256 in BC3  */
                /* port mode 128 in sip6 */
                /* all other devices     */
                return GT_FALSE;
            }

            if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                GT_U32 tableMode = PRV_CPSS_DXCH_TABLES_SIZE_MODE_GET_MAC(devNum);
                GT_U32 factorMode = tableMode - 2; /* mode 64 is FRACTION_INDICATION_CNS , not FRACTION_HALF_TABLE_INDICATION_CNS !*/

                if (tableMode == 0)
                {
                    /* port mode is 64. there are 2 entries in line*/
                    numBitsPerEntry = BMP_64_PORTS_CNS;/*1 bit per port*/
                    numEntriesPerLine = 2 ; /* No fraction indication !!! */
                    break;
                }

                /* Falcon: those 5 tables on 'HALF_SIZE_MODE' implemented as x ports
                   spread on (x/128) lines of 128 ports each , with 'sip6_chunkSize'
                   between the entries */
                sip6_chunkSize = (tableType == CPSS_DXCH_LION_TABLE_VLAN_EGRESS_E ? 1024 : 256);

                numBitsPerEntry = BMP_128_PORTS_CNS; /* 128 bits regardless to 'mode' */

                numEntriesPerLine = (2<<factorMode) | FRACTION_HALF_TABLE_INDICATION_CNS;
            }
            else
            {
                /* bobcat3: those 4 tables on 'HALF_SIZE_MODE' implemented as 512 ports
                   spread on 2 lines of 256 ports each , with 'half table' between
                   the entries */
                numBitsPerEntry = BMP_256_PORTS_CNS;/*1 bit per port*/
                numEntriesPerLine = 2 | FRACTION_HALF_TABLE_INDICATION_CNS;
            }
            break;

        case CPSS_DXCH_SIP5_TABLE_TM_FCU_PORT_TO_PHYSICAL_PORT_MAPPING_E:
            numBitsPerEntry = 8;/*8 bits per port*/
            numEntriesPerLine = 4;/*4 ports in line*/
            break;

        case CPSS_DXCH_SIP6_TABLE_INGRESS_POLICER_0_QOS_ATTRIBUTE_TABLE_E:
        case CPSS_DXCH_SIP6_TABLE_INGRESS_POLICER_1_QOS_ATTRIBUTE_TABLE_E:
        case CPSS_DXCH_SIP6_TABLE_EGRESS_POLICER_QOS_ATTRIBUTE_TABLE_E:
            numBitsPerEntry = 5;/*8 bits per port*/
            numEntriesPerLine = 4;/*4 QOS profiles in entry*/
            break;

        case CPSS_DXCH_SIP5_25_TABLE_IPVX_ROUTER_ECMP_POINTER_E:
            if(!PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                /* single entry in line */
                return GT_FALSE;
            }
            numBitsPerEntry   = 15;/* 15 bits per entry */
            numEntriesPerLine = 4; /* 4 entries in line */
            break;

        case CPSS_DXCH_SIP6_TABLE_IPVX_ECMP_E:
            if(!PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                /* single entry in line */
                return GT_FALSE;
            }
            numBitsPerEntry   = 29;/* 29 bits per entry */
            numEntriesPerLine = 2; /* 2 entries in line */
            break;
        case CPSS_DXCH_SIP6_TABLE_PBR_E:
            if(!PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                return GT_FALSE;
            }
            numBitsPerEntry = 23;   /* 23 bits per entry */
            numEntriesPerLine = 5;  /* 5 entries in line */
            reverseOrder = GT_TRUE;
            break;
        default:
            return GT_FALSE;
    }

    if(PRV_CPSS_DXCH_BMP_OF_TABLES_IN_HALF_SIZE_MODE_GET_MAC(devNum,tableType))
    {
        /* required for CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_EGRESS_VLAN_E table in BC3 and Falcon*/
        GT_U32 tableMode = PRV_CPSS_DXCH_TABLES_SIZE_MODE_GET_MAC(devNum);
        GT_U32 divisionMode = TABLE_MODE_TO_DIVISION_NUMBER_MAC(devNum);

        if(numEntriesPerLine & FRACTION_INDICATION_CNS)
        {
            numEntriesPerLine = (numEntriesPerLine - FRACTION_INDICATION_CNS) * divisionMode ;
            if (tableMode == 0 /* 64 ports*/)
            {
                numEntriesPerLine /= 2;
            }
            /* still need indication */
            numEntriesPerLine |= FRACTION_INDICATION_CNS;
        }
        /* required for tables CPSS_DXCH_SIP5_TABLE_INGRESS_BRIDGE_PORT_MEMBERS_E and CPSS_DXCH_LION_TABLE_STG_INGRESS_E in BC3 and Falcon*/
        else
        if(numEntriesPerLine & VERTICAL_INDICATION_CNS)
        {
            numEntriesPerLine = (numEntriesPerLine - VERTICAL_INDICATION_CNS) / divisionMode;
            numBitsPerEntry *= divisionMode;
            if (PRV_CPSS_SIP_6_CHECK_MAC(devNum) &&
                tableMode == 0 /* 64 ports */    &&
                /* for index calculations this table hold 128 bits event for the 64 ports mode */
                tableType != CPSS_DXCH_SIP5_TABLE_INGRESS_BRIDGE_PORT_MEMBERS_E)
            {
                numBitsPerEntry   /= 2; /* 64 bit */
                numEntriesPerLine *= 2; /* 2 entries in the 128 bits */
            }

            if(numEntriesPerLine > 1)
            {
                /* still need indication */
                numEntriesPerLine |= VERTICAL_INDICATION_CNS;
            }
        }
    }

    if(numEntriesPerLine & FRACTION_HALF_TABLE_INDICATION_CNS)
    {
        if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            GT_U32 divisionMode = TABLE_MODE_TO_DIVISION_NUMBER_MAC(devNum);

            /* choose the table part (0.125 in case of modes 64 or 128, 0.25 in case of mode 256, 0.5 in case of mode 512:
            ((entryIndex/chunkSize)*(chunkSize * divisionMode))
            choose the line inside the chunk:
            (entryIndex%chunkSize)*/
            (*entryIndexPtr)  = ((((*entryIndexPtr)/sip6_chunkSize)* (sip6_chunkSize * divisionMode)) + ((*entryIndexPtr)%sip6_chunkSize));
        }

        if((*fieldOffsetPtr) >= numBitsPerEntry)
        {
            if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                (*entryIndexPtr) += sip6_chunkSize * ((*fieldOffsetPtr)/numBitsPerEntry);
                (*fieldOffsetPtr) %= numBitsPerEntry;
            }
            else
            {
                PRV_CPSS_DXCH_TABLES_INFO_STC *tableInfoPtr = /* pointer to table info */
                                PRV_TABLE_INFO_PTR_GET_MAC(devNum,tableType);
                GT_U32 maxLines = tableInfoPtr->maxNumOfEntries;

                (*fieldOffsetPtr) -= numBitsPerEntry;
                (*entryIndexPtr)  += (maxLines / 2);
            }
        }
    }
    else
    if(numEntriesPerLine & FRACTION_INDICATION_CNS)
    {
        /* the entry from the caller is taking more than single line */
        fractionValue = numEntriesPerLine - FRACTION_INDICATION_CNS;

        fieldOffset = (*fieldOffsetPtr) % numBitsPerEntry;
        entryIndex  = ((*entryIndexPtr) * fractionValue) + ((*fieldOffsetPtr) / numBitsPerEntry);

        *fieldOffsetPtr = fieldOffset;
        *entryIndexPtr  = entryIndex;
    }
    else if(numEntriesPerLine & VERTICAL_INDICATION_CNS)
    {
        PRV_CPSS_DXCH_TABLES_INFO_STC *tableInfoPtr = /* pointer to table info */
                        PRV_TABLE_INFO_PTR_GET_MAC(devNum,tableType);
        GT_U32 maxLines = tableInfoPtr->maxNumOfEntries;

        numEntriesPerLine -= VERTICAL_INDICATION_CNS;

        if((*entryIndexPtr) >= (maxLines*numEntriesPerLine))
        {
            (*entryIndexPtr) = 0x0FFFFFFF;/* cause error on the caller function */
            *fieldOffsetPtr = 0;
        }
        else
        {
            *fieldOffsetPtr += (((*entryIndexPtr)/maxLines) % numEntriesPerLine) * numBitsPerEntry;
            (*entryIndexPtr) %= maxLines;
        }
        /* restore indication */
        numEntriesPerLine |= VERTICAL_INDICATION_CNS;
    }
    else
    {
        if (reverseOrder)
        {
            *fieldOffsetPtr += (numBitsPerEntry*(numEntriesPerLine - 1))-((*entryIndexPtr) % numEntriesPerLine) * numBitsPerEntry;
        }
        else
        {
            *fieldOffsetPtr += ((*entryIndexPtr) % numEntriesPerLine) * numBitsPerEntry;
        }
        *entryIndexPtr  /= numEntriesPerLine;
    }

    if(numBitsPerEntryPtr)
    {
        *numBitsPerEntryPtr = numBitsPerEntry;
    }

    if(numEntriesPerLinePtr)
    {
        *numEntriesPerLinePtr = numEntriesPerLine;
    }

    return GT_TRUE;

}

/**
* @internal prvCpssDxChTableEngineToHwRatioGet function
* @endinternal
*
* @brief   return the ratio between the index that the 'table engine' gets from the
*         'cpss API' to the 'HW index' in the table.
*         NOTE: this to support 'multi entries' in line or fraction of entry in line.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] tableType                - the specific table name
*
* @param[out] ratioPtr                 - (pointer to) number of entries for in single line or
*                                      number of lines  for in single entry.
* @param[out] isMultiplePtr            - (pointer to) indication that ratio is 'multiple' or 'fraction'.
*                                      GT_TRUE  - ratio is 'multiple' (number of entries for in single line)
*                                      GT_FALSE - ratio is 'fraction' (number of lines  for in single entry)
*                                       GT_OK
*/
GT_STATUS prvCpssDxChTableEngineToHwRatioGet
(
    IN GT_U8                   devNum,
    IN CPSS_DXCH_TABLE_ENT      tableType,
    OUT GT_U32                 *ratioPtr,
    OUT GT_BOOL                *isMultiplePtr
)
{
    GT_U32  numBitsPerEntry;
    GT_U32  numEntriesPerLine;
    GT_U32  entryIndex = 0;
    GT_U32  startBit = 0;

    if(GT_TRUE == prvCpssDxChTableEngineMultiEntriesInLineIndexAndGlobalBitConvert(devNum,tableType,
                INOUT &entryIndex,INOUT &startBit ,
                OUT &numBitsPerEntry , OUT &numEntriesPerLine))
    {
        if(numEntriesPerLine & FRACTION_INDICATION_CNS ||
           numEntriesPerLine & FRACTION_HALF_TABLE_INDICATION_CNS )
        {
            /* the entry uses multiple lines - either consecutive or not */
            *isMultiplePtr = GT_FALSE;
        }
        else
        {
            *isMultiplePtr = GT_TRUE;
        }

        /* assign ratio with numEntriesPerLine without 'special' bits */
        *ratioPtr = numEntriesPerLine & ~( FRACTION_HALF_TABLE_INDICATION_CNS
                                           | FRACTION_INDICATION_CNS
                                           | VERTICAL_INDICATION_CNS );
    }
    else
    {
        *ratioPtr = 1;
        *isMultiplePtr = GT_TRUE;/* Multiple */
    }

    return GT_OK;

}


/**
* @internal directReadTableHwEntries function
* @endinternal
*
* @brief   Read specified number of HW entry from the table into single buffer.
*         In spite of prvCpssDxChPortGroupReadTableEntry the function
*         treats incoming "entry index" and outgoing "entries" as values
*         in HW format! I.e. entry index - is line number in HW table
*         and the "entry" is value of some number of 32-bit words
*         storing single line in HW table.
*         portGroupId can't be CPSS_PORT_GROUP_UNAWARE_MODE_CNS!
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - the port group Id
* @param[in] tableInfoPtr             - (pointer to) the structure with a table access info
* @param[in] entryIndex               - index of first hw entrie to be copied
* @param[in] entriesNum               - numbers of entries to be copied
*
* @param[out] entryValuePtr            - (pointer to) the data read from the table
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS directReadTableHwEntries
(
    IN GT_U8                          devNum,
    IN GT_U32                         portGroupId,
    IN PRV_CPSS_DXCH_TABLES_INFO_STC *tableInfoPtr,
    IN GT_U32                         entryIndex,
    IN GT_U32                         entriesNum,
    OUT GT_U32                       *entryValuePtr
)
{
    GT_STATUS status;
    GT_U32 address;
    GT_U32 i;
    GT_U32 readPerWord;
    PRV_CPSS_DXCH_TABLES_INFO_DIRECT_STC *tablePtr =
        (PRV_CPSS_DXCH_TABLES_INFO_DIRECT_STC*)(tableInfoPtr->readTablePtr);

    if(tablePtr->baseAddress == SPECIAL_ADDRESS_NOT_EXISTS_E)
    {
        /* the table not supported */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    readPerWord = (tablePtr->nextWordOffset !=
                   PRV_CPSS_DXCH_DEFAULT_NEXT_WORD_OFFSET_CNS);

    for (++entriesNum; entriesNum >1; entriesNum--, entryIndex++)
    {
        address = tablePtr->baseAddress + entryIndex * tablePtr->step;
        if (readPerWord)
        {
            for (i=0; i<tableInfoPtr->entrySize; i++)
            {
                status = prvCpssHwPpPortGroupReadRam(devNum, portGroupId,
                                                     address, 1,
                                                     &entryValuePtr[i]);
                if (status != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }
                address += tablePtr->nextWordOffset;
            }

        }
        else
        {
            status = prvCpssHwPpPortGroupReadRam(
                devNum, portGroupId,address, tableInfoPtr->entrySize,
                entryValuePtr);
            if (status != GT_OK)
            {
                return status;
            }
        }
        entryValuePtr += tableInfoPtr->entrySize;
    }

    return GT_OK;
}

#ifdef CPSS_LOG_ENABLE
extern PRV_CPSS_ENUM_STRING_VALUE_PAIR_STC prvCpssLogEnum_map_CPSS_DXCH_TABLE_ENT[];
extern GT_U32 prvCpssLogEnum_size_CPSS_DXCH_TABLE_ENT;
/* utility function to get the 'id' of 'string of enumeration from CPSS_DXCH_TABLE_ENT' ,
   for example convert : "CPSS_DXCH_SIP5_TABLE_TCAM_E" to 150 (the value of CPSS_DXCH_SIP5_TABLE_TCAM_E)

   this function allow to call prvCpssDxChTablePerformanceRead_debugTest(...) with the 'tableType' value.
*/
void debug_printTableEnumForTableName(IN char* tableName)
{
    GT_U32  ii;

    if(tableName == NULL)
    {
        /* NULL pointer */
        cpssOsPrintf("NULL pointer \n");
        return;
    }

    for(ii = 0 ; ii < prvCpssLogEnum_size_CPSS_DXCH_TABLE_ENT; ii++)
    {
        if(0 != cpssOsStrCmp(prvCpssLogEnum_map_CPSS_DXCH_TABLE_ENT[ii].namePtr,tableName))
        {
            continue;
        }

        cpssOsPrintf("table : [%s] is [%d] in the enum \n",
            tableName,
            prvCpssLogEnum_map_CPSS_DXCH_TABLE_ENT[ii].enumValue);
        return;
    }

    cpssOsPrintf("table : [%s] not found in the enum CPSS_DXCH_TABLE_ENT \n",
        tableName);

}
#endif /* CPSS_LOG_ENABLE */

/**
* @internal prvCpssDxChTablePerformanceRead_debugTest function
* @endinternal
*
* @brief   debug function to check performance of table (read operation) in the device.
*         (to allow compare to performance of other device)
*         NOTE: the function prints the results.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] tableType                - the specific table name
* @param[in] startIndex               - start index in the table
* @param[in] numOfEntries             - numbers of entries read (allow to overlap and re-start from index 0)
*                                      NOTE: value 0xFFFFFFFF means 'until end of table'
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTablePerformanceRead_debugTest
(
    IN GT_U8                   devNum,
    IN CPSS_DXCH_TABLE_ENT      tableType,
    IN GT_U32                   startIndex,
    IN GT_U32                   numOfEntries
)
{
    GT_STATUS                rc = GT_OK;
    GT_U32  secondsStart, secondsEnd,
            nanoSecondsStart, nanoSecondsEnd,
            seconds, nanoSec;
    GT_U32                   ii;
    GT_U32                   lastIndex;
    GT_U32                   numEntriesDone = 0;
    PRV_CPSS_DXCH_TABLES_INFO_STC *tableInfoPtr; /* pointer to table info */
    GT_U32  entryMemoBufArr[PRV_CPSS_DXCH_TABLE_MAX_ENTRY_SIZE_CNS];
#ifdef CPSS_LOG_ENABLE
    for(ii = 0 ; ii < prvCpssLogEnum_size_CPSS_DXCH_TABLE_ENT; ii++)
    {
        if(prvCpssLogEnum_map_CPSS_DXCH_TABLE_ENT[ii].enumValue != (GT_32)tableType)
        {
            continue;
        }

        cpssOsPrintf("testing table : [%s] \n",
            prvCpssLogEnum_map_CPSS_DXCH_TABLE_ENT[ii].namePtr);
        break;
    }

#endif /* CPSS_LOG_ENABLE  */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    /* validity check */
    if((GT_U32)tableType >=  PRV_CPSS_DXCH_TABLE_SIZE_GET_MAC(devNum))
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);

    tableInfoPtr = PRV_TABLE_INFO_PTR_GET_MAC(devNum,tableType);

    lastIndex = tableInfoPtr->maxNumOfEntries - 1;

    if(numOfEntries == 0xFFFFFFFF)
    {
        /* means 'until end of table' */
        numOfEntries = (lastIndex + 1) - startIndex;
    }

    if(startIndex > lastIndex)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* take the 'start time' */

    rc = cpssOsTimeRT(&secondsStart,&nanoSecondsStart);
    if(rc != GT_OK)
    {
        return rc;
    }

    for(ii = startIndex ; ii <= lastIndex; /*ii++ not in here */)
    {
        if(numEntriesDone >= numOfEntries)
        {
            /* we are done with the entries */
            break;
        }

        if((numEntriesDone < numOfEntries) &&
           (ii == lastIndex))
        {
            /* we need to wraparound */
            ii = startIndex;
        }

        if(GT_FALSE ==
            prvCpssDxChDiagDataIntegrityTableCheckIsDevSupportHwTable(devNum,tableType,
            ii))
        {
            ii++;
            /* the device not supports this index in table */
            continue;
        }

        /* read the entry */
        rc = prvCpssDxChPortGroupReadTableEntry(devNum,0,tableType,
            ii,entryMemoBufArr);
        if(rc != GT_OK)
        {
            break;
        }

        numEntriesDone ++;
        ii++;

    }

    rc = cpssOsTimeRT(&secondsEnd,&nanoSecondsEnd);
    if(rc != GT_OK)
    {
        return rc;
    }

    seconds = secondsEnd-secondsStart;
    if(nanoSecondsEnd >= nanoSecondsStart)
    {
        nanoSec = nanoSecondsEnd-nanoSecondsStart;
    }
    else
    {
        nanoSec = (1000000000 - nanoSecondsStart) + nanoSecondsEnd;
        seconds--;
    }
    cpssOsPrintf("time: total time : numEntries[%ld] : %ld sec., %ld nanosec.\n",
        numOfEntries , seconds, nanoSec);

    return GT_OK;
}

/**
* @internal prvCpssDxChPbrTableConfigSet function
* @endinternal
*
* @brief   This function set PBR virtual table configuration
*
* @note   APPLICABLE DEVICES:           Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                          PP's device number.
* @param[in] baseAddress                     base address.
* @param[in] maxNumOfEntries                 maximum number of PBR entries.
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_NOT_FOUND         - PBR table entry not found.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvCpssDxChPbrTableConfigSet
(
    IN GT_U8    devNum,
    IN GT_U32   baseAddress,
    IN GT_U32   maxNumOfEntries
)
{
    PRV_CPSS_DXCH_TABLES_INFO_STC          *tableInfoPtr;      /* pointer to table info */
    PRV_CPSS_DXCH_TABLES_INFO_DIRECT_STC   *directTablePtr;    /* pointer to direct access table entry info.*/

    PRV_CPSS_DXCH_PP_HW_INFO_SIP6_SUPPORTED_CHECK_MAC(devNum);

    /* NOTE: fixed code from update of falconTablesInfo[] , to support Hawk */
    tableInfoPtr = PRV_TABLE_INFO_PTR_GET_MAC(devNum,CPSS_DXCH_SIP6_TABLE_PBR_E);
    /* update the number of entries */
    tableInfoPtr->maxNumOfEntries = maxNumOfEntries;
    /* update the baseAddr */
    directTablePtr = (PRV_CPSS_DXCH_TABLES_INFO_DIRECT_STC*)(tableInfoPtr->readTablePtr);
    /* NOTE: fixed bug in old code that updated falcon_directAccessTableInfo_new[]
             1. for current device is was not relevant any more after the 'table init'
             2. for other devices that may init later ... it would impact !
    */
    directTablePtr->baseAddress = baseAddress +
        prvCpssDxChHwUnitBaseAddrGet(devNum,PRV_CPSS_DXCH_UNIT_LPM_E,NULL);/* LPM baseline */

    return GT_OK;
}

/**
* @internal prvCpssDxChTableMgcamEnableSet function
* @endinternal
*
* @brief   Enable/Disable using MGCAM for table access.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] tableType                - TCAM tabel type.
* @param[in] enable                   - GT_TRUE: MGCAM engine access enabled.
*                                      GT_FALSE: MGCAM engine access disabled.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTableMgcamEnableSet
(
    IN GT_U8                  devNum,
    IN CPSS_DXCH_TABLE_ENT    tableType,
    IN GT_BOOL                enable
)
{
    GT_U32 regAddr;
    GT_STATUS rc = GT_OK;
    PRV_CPSS_DXCH_TABLES_INFO_STC *tableInfoPtr; /* pointer to table info */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    /* validity check */
    if((GT_U32)tableType >= PRV_CPSS_DXCH_TABLE_SIZE_GET_MAC(devNum))
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);

    tableInfoPtr = PRV_TABLE_INFO_PTR_GET_MAC(devNum,tableType);

    if(tableInfoPtr->writeAccessType == PRV_CPSS_DXCH_INDIRECT_ACCESS_E)
    {
        /* it's not possible to use MGCAM for tables with indirect access */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    if (GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(
        devNum,PRV_CPSS_DXCH_BOBCAT2_TXQ_TAIL_DROP_TABLES_R_W_PENDS_BUS_WA_E))
    {
        switch(tableType)
        {
            case CPSS_DXCH_SIP5_TABLE_TAIL_DROP_MAX_QUEUE_LIMITS_E:
            case CPSS_DXCH_SIP5_TABLE_TAIL_DROP_EQ_QUEUE_LIMITS_DP0_E:
            case CPSS_DXCH_SIP5_TABLE_TAIL_DROP_DQ_QUEUE_BUF_LIMITS_E:
            case CPSS_DXCH_SIP5_TABLE_TAIL_DROP_DQ_QUEUE_DESC_LIMITS_E:
            case CPSS_DXCH_SIP5_TABLE_TAIL_DROP_EQ_QUEUE_LIMITS_DP1_E:
            case CPSS_DXCH_SIP5_TABLE_TAIL_DROP_EQ_QUEUE_LIMITS_DP2_E:
                /* it's not possible to use MGCAM for tables related to the erratum */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            default: break;
        }
    }

    if(enable != GT_FALSE)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->MG.tableAccessControl.mgcamControl;

        rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 0, 1, 1);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /* update DB for write access type */
    tableInfoPtr->writeAccessType = (enable == GT_FALSE) ? PRV_CPSS_DXCH_DIRECT_ACCESS_E : PRV_CPSS_DXCH_MGCAM_ACCESS_E;
    return rc;
}

/**
* @internal prvCpssDxChTableReadFromShadowEnableSet function
* @endinternal
*
* @brief   Set enable reading table from shadow instead of HW
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  none.
*
* @param[in] devNum                   - The PP device number to write to.
* @param[in] tableType                - the 'HW table'
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad param value
* @retval GT_BAD_STATE             - shadow for the table not supported
*/
GT_STATUS prvCpssDxChTableReadFromShadowEnableSet
(
    IN GT_U8                 devNum,
    IN CPSS_DXCH_TABLE_ENT   tableType,
    IN GT_BOOL               enable
)
{
    GT_U32 *wordPtr;
    GT_U32 bitIndex;
    PRV_CPSS_DXCH_HW_TABLES_SHADOW_STC *shadowInfoPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (tableType >= CPSS_DXCH_TABLE_LAST_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    shadowInfoPtr = &(PRV_CPSS_DXCH_PP_MAC(devNum)->shadowInfoArr[tableType]);
    if (shadowInfoPtr->shadowType != CPSS_DXCH_SHADOW_TYPE_CPSS_E)
    {
        return /* it's OK to get this code */ GT_BAD_STATE;
    }

    wordPtr  = &(PRV_CPSS_DXCH_PP_MAC(devNum)->readFromShadowTablesBitmap[tableType / 32]);
    bitIndex = (tableType % 32);

    if (enable == GT_FALSE)
    {
        *wordPtr &= (~ (1 << bitIndex));
    }
    else
    {
        *wordPtr |= (1 << bitIndex);
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChTableReadFromShadowEnableGet function
* @endinternal
*
* @brief   Get enable reading table from shadow instead of HW
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  none.
*
* @param[in] devNum                   - The PP device number to write to.
* @param[in] tableType                - the 'HW table'
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad param value
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvCpssDxChTableReadFromShadowEnableGet
(
    IN  GT_U8                 devNum,
    IN  CPSS_DXCH_TABLE_ENT   tableType,
    OUT GT_BOOL               *enablePtr
)
{
    GT_U32 *wordPtr;
    GT_U32 bitIndex;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (tableType >= CPSS_DXCH_TABLE_LAST_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    /* the bitmap below cannot contain ones when shadow type is not CPSS_DXCH_SHADOW_TYPE_CPSS_E */
    /* the "set" function checks it.                                                             */

    wordPtr  = &(PRV_CPSS_DXCH_PP_MAC(devNum)->readFromShadowTablesBitmap[tableType / 32]);
    bitIndex = (tableType % 32);

    *enablePtr = (((*wordPtr) >> bitIndex) & 1) ? GT_TRUE : GT_FALSE;
    return GT_OK;
}

