/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssGenNetworkIfLog.c
*       WARNING!!! this is a generated file, please don't edit it manually
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/

/* disable deprecation warnings (if one) */
#ifdef __GNUC__
#if  (__GNUC__*100+__GNUC_MINOR__) >= 406
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
#endif

#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/generic/networkIf/cpssGenNetIfTypes.h>
#include <cpss/generic/networkIf/private/prvCpssGenNetworkIfLog.h>


/********* enums *********/

const PRV_CPSS_ENUM_STRING_VALUE_PAIR_STC prvCpssLogEnum_map_CPSS_NET_RX_CPU_CODE_ENT[]  =
{PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_UNDEFINED_CPU_CODE_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_CONTROL_OBSOLETE_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_UN_KNOWN_UC_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_UN_REGISTERD_MC_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_CONTROL_BPDU_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_CONTROL_DEST_MAC_TRAP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_CONTROL_SRC_MAC_TRAP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_CONTROL_SRC_DST_MAC_TRAP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_CONTROL_MAC_RANGE_TRAP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_RX_SNIFFER_TRAP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_INTERVENTION_ARP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_INTERVENTION_IGMP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_INTERVENTION_SA_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_INTERVENTION_DA_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_INTERVENTION_SA_DA_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_INTERVENTION_PORT_LOCK_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_EXP_TNL_BAD_IPV4_HDR_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_EXP_TNL_BAD_VLAN_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_STATIC_ADDR_MOVED_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_MAC_SPOOF_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_MLL_RPF_TRAP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_RESERVED_SIP_TRAP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_INTERNAL_SIP_TRAP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_SIP_SPOOF_TRAP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_DEF_KEY_TRAP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_IP_CLASS_TRAP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_CLASS_KEY_TRAP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_TCP_RST_FIN_TRAP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_CLASS_KEY_MIRROR_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_TRAP_BY_DEFAULT_ENTRY0_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_TRAP_BY_DEFAULT_ENTRY1_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_TRAP_UD_INVALID_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_RESERVED_DIP_TRAP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_MC_BOUNDARY_TRAP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_INTERNAL_DIP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_IP_ZERO_TTL_TRAP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_BAD_IP_HDR_CHECKSUM_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_RPF_CHECK_FAILED_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_OPTIONS_IN_IP_HDR_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_END_OF_IP_TUNNEL_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_BAD_TUNNEL_HDR_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_IP_HDR_ERROR_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_ROUTE_ENTRY_TRAP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_DIP_CHECK_ERROR_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_ILLEGAL_DIP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_POLICY_TRAP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_DEFAULT_ROUTE_TRAP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_IP_MTU_EXCEED_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_MPLS_MTU_EXCEED_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_CLASS_MTU_EXCEED_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_MPLS_ZERO_TTL_TRAP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_NHLFE_ENTRY_TRAP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_ILLEGAL_POP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_INVALID_MPLS_IF_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_IPV6_ICMP_MLD_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_IPV6_IGMP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_IPV6_MC_PCL_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_IPV6_HOP_BY_HOP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_IPV6_EXT_HEADER_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_IPV6_BAD_HEADER_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_IPV6_ILLIGAL_DIP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_IPV6_DIP_ERROR_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_IPV6_ROUTE_TRAP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_IPV6_ROUTER_RPF_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_IPV6_SCOP_FAIL_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_IPV6_TTL_TRAP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_IPV6_DEFAULT_ROUTE_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_MIRROR_PCL_RST_FIN_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_MIRROR_PCL_PCE_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_MIRROR_BRIDGE_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_MIRROR_MLD_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_MIRROR_IGMP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_MIRROR_RIPV1_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_MIRROR_TTL_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_MIRROR_RPF_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_MIRROR_IP_OPTIONS_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_MIRROR_ROUTE_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_MIRROR_ICMP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_MIRROR_HOP_BY_HOP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_MIRROR_EXT_HEADER_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_MIRROR_HEADER_ERROR_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_MIRROR_ILLIGAL_IP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_MIRROR_SPECIAL_DIP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_MIRROR_IP_SCOP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_MIRROR_IP_MTU_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_MIRROR_INLIF_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_MIRROR_PCL_MTU_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_MIRROR_DEFAULT_ROUTE_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_MIRROR_MC_ROUTE1_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_MIRROR_MC_ROUTE2_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_MIRROR_MC_ROUTE3_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_MIRROR_IPV4_BC_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_ETH_BRIDGED_LLT_OBSOLETE_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_IPV4_ROUTED_LLT_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_UC_MPLS_LLT_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_MC_MPLS_LLT_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_IPV6_ROUTED_LLT_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_L2CE_LLT_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_EGRESS_MIRROR_TO_CPU_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_UNKNOWN_UC_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_UNREG_MC_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_LOCK_PORT_MIRROR_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_IEEE_RSRVD_MULTICAST_ADDR_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_IPV6_ICMP_PACKET_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_MIRROR_IPV4_RIPV1_TO_CPU_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_IPV6_NEIGHBOR_SOLICITATION_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_IPV4_BROADCAST_PACKET_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_NON_IPV4_BROADCAST_PACKET_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_CISCO_MULTICAST_MAC_RANGE_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_UNREGISTERED_MULTICAST_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_IPV4_UNREGISTERED_MULTICAST_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_IPV6_UNREGISTERED_MULTICAST_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_UNKNOWN_UNICAST_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_UDP_BC_MIRROR_TRAP0_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_UDP_BC_MIRROR_TRAP1_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_UDP_BC_MIRROR_TRAP2_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_UDP_BC_MIRROR_TRAP3_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_IEEE_RSRVD_MULTICAST_ADDR_1_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_IEEE_RSRVD_MULTICAST_ADDR_2_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_IEEE_RSRVD_MULTICAST_ADDR_3_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_BRIDGED_PACKET_FORWARD_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_INGRESS_MIRRORED_TO_ANLYZER_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_EGRESS_MIRRORED_TO_ANLYZER_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_MAIL_FROM_NEIGHBOR_CPU_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_CPU_TO_CPU_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_EGRESS_SAMPLED_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_INGRESS_SAMPLED_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_INVALID_PCL_KEY_TRAP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_PACKET_TO_VIRTUAL_ROUTER_PORT_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_MIRROR_IPV4_UC_ICMP_REDIRECT_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_MIRROR_IPV6_UC_ICMP_REDIRECT_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_ROUTED_PACKET_FORWARD_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_IP_DIP_DA_MISMATCH_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_IP_UC_SIP_SA_MISMATCH_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_IPV4_UC_ROUTE1_TRAP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_IPV4_UC_ROUTE2_TRAP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_IPV4_UC_ROUTE3_TRAP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_IPV4_MC_ROUTE0_TRAP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_IPV4_MC_ROUTE1_TRAP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_IPV4_MC_ROUTE2_TRAP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_IPV4_MC_ROUTE3_TRAP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_IPV6_UC_ROUTE1_TRAP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_IPV6_UC_ROUTE2_TRAP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_IPV6_UC_ROUTE3_TRAP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_IPV6_MC_ROUTE0_TRAP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_IPV6_MC_ROUTE1_TRAP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_IPV6_MC_ROUTE2_TRAP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_IPV6_MC_ROUTE3_TRAP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_IP_UC_RPF_FAIL_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_ARP_BC_TO_ME_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_1_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_2_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_3_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_SEC_AUTO_LEARN_UNK_SRC_TRAP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_IPV4_TT_HEADER_ERROR_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_IPV4_TT_OPTION_FRAG_ERROR_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_IPV4_TT_UNSUP_GRE_ERROR_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_ARP_REPLY_TO_ME_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_CPU_TO_ALL_CPUS_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_TCP_SYN_TO_CPU_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_MC_BRIDGED_PACKET_FORWARD_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_MC_ROUTED_PACKET_FORWARD_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_L2VPN_PACKET_FORWARD_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_MPLS_PACKET_FORWARD_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_ARP_BC_EGRESS_MIRROR_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_VPLS_UNREGISTERED_MC_EGRESS_FILTER_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_VPLS_UNKWONW_UC_EGRESS_FILTER_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_VPLS_BC_EGRESS_FILTER_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_MC_CPU_TO_CPU_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_PCL_MIRRORED_TO_ANALYZER_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_VPLS_UNTAGGED_MRU_FILTER_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_TT_MPLS_HEADER_CHECK_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_TT_MPLS_TTL_EXCEED_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_TTI_MIRROR_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_MPLS_SRC_FILTERING_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_IPV4_TTL1_EXCEEDED_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_IPV6_HOPLIMIT1_EXCEED_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_OAM_PDU_TRAP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_IPCL_MIRROR_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_MPLS_TTL1_EXCEEDED_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_MPLS_ROUTE_ENTRY_1_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_MPLS_ROUTE_ENTRY_2_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_MPLS_ROUTE_ENTRY_3_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_PTP_HEADER_ERROR_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_IPV4_6_SIP_FILTERING_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_IPV4_6_SIP_IS_ZERO_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_ACCESS_MATRIX_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_FCOE_DIP_LOOKUP_NOT_FOUND_OR_FCOE_EXCEPTION_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_FCOE_SIP_NOT_FOUND_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_IP_MC_ROUTE_BIDIR_RPF_FAIL_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_MET_RED_DROP_CODE_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_ECC_DROP_CODE_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_BAD_ANALYZER_INDEX_DROP_ERROR_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_RXDMA_DROP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_TARGET_NULL_PORT_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_TARGET_EPORT_MTU_EXCEEDED_OR_RX_PROTECTION_SWITCHING_OR_INGRESS_FORWARDING_RESTRICTIONS_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_UNTAGGED_OVERSIZE_DROP_CODE_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_IPV6_TT_UNSUP_GRE_ERROR_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_CCFC_PKT_DROP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_BAD_FC_PKT_DROP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_PHA_FW_DROP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_FIRST_USER_DEFINED_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_0_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_1_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_2_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_3_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_4_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_5_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_6_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_7_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_8_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_9_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_10_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_11_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_12_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_13_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_14_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_15_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_16_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_17_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_18_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_19_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_20_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_21_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_22_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_23_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_24_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_25_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_26_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_27_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_28_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_29_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_30_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_31_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_32_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_33_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_34_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_35_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_36_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_37_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_38_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_39_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_40_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_41_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_42_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_43_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_44_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_45_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_46_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_47_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_48_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_49_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_50_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_51_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_52_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_53_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_54_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_55_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_56_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_57_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_58_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_59_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_60_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_61_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_62_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_USER_DEFINED_63_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_LAST_USER_DEFINED_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_LAST_UNKNOWN_HW_CPU_CODE_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_NET_ALL_CPU_OPCODES_E)
};
PRV_CPSS_LOG_STC_ENUM_MAP_ARRAY_SIZE_MAC(CPSS_NET_RX_CPU_CODE_ENT);
const char * const prvCpssLogEnum_CPSS_NET_CPU_CODE_L4_PORT_MODE_ENT[]  =
{
    "CPSS_NET_CPU_CODE_L4_DEST_PORT_E",
    "CPSS_NET_CPU_CODE_L4_SRC_PORT_E",
    "CPSS_NET_CPU_CODE_L4_BOTH_DEST_SRC_PORT_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_NET_CPU_CODE_L4_PORT_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_NET_CPU_CODE_RATE_LIMIT_MODE_ENT[]  =
{
    "CPSS_NET_CPU_CODE_RATE_LIMIT_LOCAL_E",
    "CPSS_NET_CPU_CODE_RATE_LIMIT_AGGREGATE_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_NET_CPU_CODE_RATE_LIMIT_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_NET_PROT_ENT[]  =
{
    "CPSS_NET_PROT_UDP_E",
    "CPSS_NET_PROT_TCP_E",
    "CPSS_NET_PROT_BOTH_UDP_TCP_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_NET_PROT_ENT);
const char * const prvCpssLogEnum_CPSS_NET_TCP_UDP_PACKET_TYPE_ENT[]  =
{
    "CPSS_NET_TCP_UDP_PACKET_UC_E",
    "CPSS_NET_TCP_UDP_PACKET_MC_E",
    "CPSS_NET_TCP_UDP_PACKET_BOTH_UC_MC_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_NET_TCP_UDP_PACKET_TYPE_ENT);


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_NET_RX_CPU_CODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_NET_RX_CPU_CODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAP_MAC(namePtr, paramVal, CPSS_NET_RX_CPU_CODE_ENT);
}
void prvCpssLogParamFunc_CPSS_NET_RX_CPU_CODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_NET_RX_CPU_CODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAP_MAC(namePtr, *paramVal, CPSS_NET_RX_CPU_CODE_ENT);
}

