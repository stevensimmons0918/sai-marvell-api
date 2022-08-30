// xpReasonCodeTable.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#ifndef _xpReasonCodeTable_h_
#define _xpReasonCodeTable_h_

#include "openXpsReasonCodeTable.h"

#define XP_IVIF_PORT_VLAN_MISS                  10  ///< Miss in Ingress VIF Port VLAN Table control

#define XP_BRIDGE_MAC_SA_HIT                    13  ///< FDB MAC learning: MAC SA hit in the FDB indication
#define XP_ROUTE_RC_NH_TABLE_HIT_SUB_1          61  ///< NextHope table hit + User Defined Reason 1 indication
#define XP_ROUTE_RC_NH_TABLE_HIT_SUB_2          62  ///< NextHope table hit + User Defined Reason 2 indication
#define XP_ROUTE_RC_NH_TABLE_HIT_SUB_3          63  ///< NextHope table hit + User Defined Reason 3 indication
#define XP_IVIF_RC_INVALID_HDR                  101 ///< Invalid IP Header
#define XP_IVIF_RC_PORT_TAG_POLICY              102 ///< Port frame type acceptance policy 

#define XP_IVIF_RC_PKT_TMPLT_UNKNOWN            104 ///< Unknown Header Template
#define XP_IVIF_RC_MARTIAN_IPADDR               105 ///< A Martian packet contain IP in IANA range indication
#define XP_IVIF_RC_NO_BD                        106 ///< No Bridge Domain assignment
#define XP_TNL_ID_MISS                          107 ///< Tunnel Table ID miss
#define XP_TNL_RVTEP_NEW                        108 ///< New MAC Learning on a Remote VTEP tunnel
#define XP_PTP_TRAP_TO_CPU                      109 ///< PTP Trap
#define XP_BRIDGE_RC_BC_FLOOD_CMD               201 ///< Ingress VIF Flooding for bcast packets control
#define XP_BRIDGE_RC_FLOOD_CMD                  202 ///< Ingress VIF Flooding for unknown ucast/mcast packets control
#define XP_BRIDGE_RC_FRAME_TYPE_INVALID         203 ///< Ingress VIF frame type acceptance policy
#define XP_BRIDGE_RC_FDB_DA_CMD                 204 ///< FDB MAC DA Control
#define XP_BRIDGE_RC_FDB_STATIC_SA_VIOLATION    205 ///< Static MAC SA appearing on Ingress VIF different from FDB entry

#define XP_BRIDGE_RC_IVIF_IGMP_CMD              207 ///< IGMP Broadcast Ingress VIF control
#define XP_BRIDGE_RC_IVIF_ICMP6_CMD             208 ///< ICMP Broadcast Ingress VIF control
#define XP_BRIDGE_RC_IVIF_MAC_SAMODE            209 ///< MAC SA Mode control
#define XP_BRIDGE_RC_IVIF_SPAN_BLOCKED          210 ///< Blocked Span State

#define XP_BRIDGE_RC_MC_BRIDGE_CMD              212 ///< FDB MAC DA Control
#define XP_RC_PBB_REDIRECT                      213 ///< Tunnel Redirection control
#define XP_BRIDGE_RC_UNKNOWN_L2_PACKET          214 ///< Unknown UC packets control
#define XP_BRIDGE_BD_TABLE_MISS                 215 ///< Miss in Bridge Domain table
#define XP_BRIDGE_RC_MC_UNREG_CMD               216 ///< Unregistered MC packets control
#define XP_ROUTE_RC_TTL0                        301 ///< IP TTL expiration

#define XP_ROUTE_RC_ROUTE_TABLE_HIT             303 ///< Route Table hit indication
#define XP_ROUTE_RC_MC_BRDG_TABLE_HIT           304 ///< MC Bridging Table hit indication
#define XP_ROUTE_RC_MC_ROUTE_TABLE_HIT          305 ///< MC Routing Table hit indication 
#define XP_ROUTE_RC_MC_ROUTE_MISS               306 ///< Miss when Multicast Routing is enabled
#define XP_ROUTE_RC_PKT_TYPE_UNKNOWN            308 ///< Unknown Packet Type
#define XP_ROUTE_RC_MC_INVALID_DA               309 ///< MAC DA is Multicast for Multicast Routing
#define XP_ROUTE_RC_IP_OPTIONS                  310 ///< Routing IP Options indication
#define XP_ROUTE_RC_MC_RPF_FAIL                 311 ///< IP Routing RFP check failure 
#define XP_IACL_EVIF_REDIRECT_CODE              315 ///< Reason code for Iacl redirect case

#define XP_ROUTE_RC_URPF_FAIL                   314 ///< uRPF Routing failure
#define XP_IACL_ACTION                          401 ///< Ingress ACL control
#define XP_NAT_TABLE_MISS                       512 ///< NAT Table miss
#define XP_NAT_TRAP_TO_CPU                      513 ///< NAT TRAP to CPU control 
#define XP_SFLOW_RC_SAMPLE                      601 ///< SFlow
#define XP_POLICING_RC_PKT_DROP                 602 ///< DROP due to Policing
#define XP_MTU_CHECK_PKT_CMD                    703 ///< Packet Command updated with MTU check
#define XP_OPENFLOW_RC_TABLE_MISS               801 ///< OpenFlow Table miss
#define XP_OPENFLOW_RC_OFPP_LOCAL               802 ///< OpenFlow local processing indication
#define XP_OPENFLOW_RC_OFPP_CONTROLLER          803 ///< OpenFlow controller processing indication
#define XP_USER_ROUTE_ICMPv4                    891
#define XP_USER_ROUTE_ICMPv6_NS                 892
#define XP_USER_ROUTE_ICMPv6_NA                 893
#define XP_USER_VRRP_CONTROL                    897

#endif
