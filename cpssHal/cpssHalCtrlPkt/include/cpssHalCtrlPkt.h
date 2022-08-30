/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/**
********************************************************************************
* @file cpssHalCtrlPkt.h
*
* @brief Internal header which defines API for helpers functions
* which are specific for XPS CntrlTrap Table .
*
* @version   01
********************************************************************************
*/
#ifndef __cpssHalCtrlPkth
#define __cpssHalCtrlPkth

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <gtOs/gtGenTypes.h>
#include <cpss/common/cpssTypes.h>
#include <cpss/generic/networkIf/cpssGenNetIfTypes.h>
#include <cpss/generic/cos/cpssCosTypes.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgVlan.h>

#define CPSS_HAL_CTRL_PKT_IP_PROTOCOL_TYPE_ICMPV6     0x3A
#define CPSS_HAL_CNP_BTH_HEADER_OPCODE_CNS           (0x81)
#define CPSS_HAL_CNP_ROCEV2_UDP_DESTINATION_PORT_CNS (4791)

typedef enum cpssHalCtrlPktUserDefinedCpuCode
{
    CPSS_HAL_CTRLPKT_CPU_CODE_BGPv4v6 = CPSS_NET_FIRST_USER_DEFINED_E + 1,
    //CPSS_NET_FIRST_USER_DEFINED_E used by UT fremework
    CPSS_HAL_CTRLPKT_CPU_CODE_DHCPv6,
    CPSS_HAL_CTRLPKT_CPU_CODE_SSH,
    //    CPSS_HAL_CTRLPKT_CPU_CODE_ARP,
    CPSS_HAL_CTRLPKT_CPU_CODE_ACL_LOG,  //not used
    CPSS_HAL_CTRLPKT_CPU_CODE_ACL_TRAP, //not used
    CPSS_HAL_CTRLPKT_CPU_CODE_CONTROL_QUEUE0, //not used
    CPSS_HAL_CTRLPKT_CPU_CODE_CONTROL_QUEUE1, //not used
    CPSS_HAL_CTRLPKT_CPU_CODE_CONTROL_QUEUE2, //not used
    CPSS_HAL_CTRLPKT_CPU_CODE_CONTROL_QUEUE3, //not used
    CPSS_HAL_CTRLPKT_CPU_CODE_CONTROL_QUEUE4, //not used
    CPSS_HAL_CTRLPKT_CPU_CODE_CONTROL_QUEUE5, //not used
    CPSS_HAL_CTRLPKT_CPU_CODE_CONTROL_QUEUE6, //not used
    CPSS_HAL_CTRLPKT_CPU_CODE_CONTROL_QUEUE7, //not used
    CPSS_HAL_CTRLPKT_CPU_CODE_INBAND_MGMT,
    CPSS_HAL_CTRLPKT_CPU_CODE_CNP,
    CPSS_HAL_CTRLPKT_CPU_CODE_FABRICO_ERROR,
    CPSS_HAL_CTRLPKT_CPU_CODE_EAPOL_TRAP,

    CPSS_HAL_CTRLPKT_CPU_CODE_LAST_NOT_USED

} cpssHalCtrlPktUserDefinedCpuCode_e;

#define CPSS_HAL_CTRLPKT_ACL_PORTS_OFFSET (30)
typedef enum cpssHalCtrlPktAclPriority
{
    CPSS_HAL_CTRLPKT_ACL_PRIO_ARP_MATCH = 0,
    CPSS_HAL_CTRLPKT_ACL_PRIO_BGPv4_TCP_SRC_PORT_MATCH = 1,
    CPSS_HAL_CTRLPKT_ACL_PRIO_BGPv4_TCP_DEST_PORT_MATCH = 2,
    CPSS_HAL_CTRLPKT_ACL_PRIO_BGPv6_TCP_SRC_PORT_MATCH = 3,
    CPSS_HAL_CTRLPKT_ACL_PRIO_BGPv6_TCP_DEST_PORT_MATCH = 4,
    CPSS_HAL_CTRLPKT_ACL_PRIO_DHCPv4_UDP1_PORT_MATCH = 5,
    CPSS_HAL_CTRLPKT_ACL_PRIO_DHCPv4_UDP2_PORT_MATCH = 6,
    CPSS_HAL_CTRLPKT_ACL_PRIO_DHCPv6_UDP1_PORT_MATCH = 7,
    CPSS_HAL_CTRLPKT_ACL_PRIO_DHCPv6_UDP2_PORT_MATCH = 8,
    CPSS_HAL_CTRLPKT_ACL_PRIO_SSH_TCP_SRC_PORT_MATCH = 9,
    CPSS_HAL_CTRLPKT_ACL_PRIO_SSH_TCP_DEST_PORT_MATCH = 10,
    CPSS_HAL_CTRLPKT_ACL_PRIO_PACKET_TYPE_UNK_ETH_TYPE_LACP_MATCH = 11,
    CPSS_HAL_CTRLPKT_ACL_PRIO_PACKET_TYPE_UNK_ETH_TYPE_LLDP_MATCH = 12,
    CPSS_HAL_CTRLPKT_ACL_PRIO_802_3_FLOW_CONTROL     = 13,
    CPSS_HAL_CTRLPKT_ACL_PRIO_INBAND_MGMT_IPV4_MATCH = 14,
    CPSS_HAL_CTRLPKT_ACL_PRIO_INBAND_MGMT_IPV6_MATCH = 15,
    CPSS_HAL_CTRLPKT_ACL_PRIO_EGRESS_ERSPAN_IPV4_0_MATCH = 16,
    CPSS_HAL_CTRLPKT_ACL_PRIO_CNP_IPv4_MATCH        = 17 + CPSS_HAL_CTRLPKT_ACL_PORTS_OFFSET,
    CPSS_HAL_CTRLPKT_ACL_PRIO_CNP_IPv6_MATCH        = CPSS_HAL_CTRLPKT_ACL_PRIO_CNP_IPv4_MATCH + CPSS_HAL_CTRLPKT_ACL_PORTS_OFFSET + 1,
    CPSS_HAL_CTRLPKT_ACL_PRIO_PACKET_TYPE_UNK_ETH_TYPE_EAP_MATCH  = CPSS_HAL_CTRLPKT_ACL_PRIO_CNP_IPv6_MATCH + 1,
    CPSS_HAL_CTRLPKT_ACL_PRIO_LAST_NOT_USED
}   cpssHalCtrlPktAclPriority_e;

typedef enum cpssHalBgpType
{
    CPSS_HAL_BGP_IPV4,
    CPSS_HAL_BGP_IPV6,
    CPSS_HAL_BGP_MAX
} cpssHalBgpType;

typedef enum
{
    CPSS_HAL_IP_IPV4,
    CPSS_HAL_IP_IPV6,
    CPSS_HAL_IP_MAX
} cpssHalIpType;

/*
 *  CPSS_HAL_CTRLPKT_TYPE_Arp - REQUEST, RESPONSE
 *  CPSS_HAL_CTRLPKT_TYPE_IEEE_RESERVED_MCAST - STP, LACP, LLDP
 *  CPSS_HAL_CTRLPKT_TYPE_IGMP - QUERY, LEAVE, V1_REPORT, V2_REPORT, V3_REPORT
 *  CPSS_HAL_CTRLPKT_TYPE_ICMPV6 - MLD_V1_V2, MLD_V1_REPORT, MLD_V1_DONE, MLD_V2_REPORT, NEIGHBOR_DISCOVERY
 *  CPSS_HAL_CTRLPKT_TYPE_UDP_PORT - DHCPV6, DHCP, SNMP
 *  CPSS_HAL_CTRLPKT_TYPE_IP_LINK_LOCAL - OSPFV6, OSPF, PIM, VRRP
 *  TBD - VRRPV6, BGPV6, BGP, SAMPLEPACKET, EAPOL, PVRST, L3_MTU_ERROR, TTL_ERROR
 */
typedef enum cpssHalCtrlPktTypes_e
{
    CPSS_HAL_CTRLPKT_TYPE_ARP,
    CPSS_HAL_CTRLPKT_TYPE_IEEE_RESERVED_MCAST,
    CPSS_HAL_CTRLPKT_TYPE_IGMP,
    CPSS_HAL_CTRLPKT_TYPE_ICMPV6,
    CPSS_HAL_CTRLPKT_TYPE_UDP_PORT,
    CPSS_HAL_CTRLPKT_TYPE_IP_LINK_LOCAL,
    CPSS_HAL_CTRLPKT_TYPE_ALL,
    CPSS_HAL_CTRLPKT_TYPE_MAX = CPSS_HAL_CTRLPKT_TYPE_ALL,
} cpssHalCtrlPktTypes_e;

/**
 * \brief This type (enum) lists the protocols for (MAC LSB)
 *        IEEE Reserved Multicast (01:08:C2:00:00:XX)
 */
typedef enum cpssHalCtrlPktResIeeeMCastProto_e
{
    CPSS_HAL_CTRLPKT_RES_IEEE_MCAST_PROTO_STP   = 0x00,
    CPSS_HAL_CTRLPKT_RES_IEEE_MCAST_PROTO_LACP  = 0x02,
    CPSS_HAL_CTRLPKT_RES_IEEE_MCAST_PROTO_LLDP  = 0x0E,
} cpssHalCtrlPktResIeeeMCastProto_e;

/**
 * \brief This type (enum) lists the messageTypes for ICMPv6
 *
 */

/* CPSS Supports only 8 ICMPv6 msg types. */
#define MAX_ICMPV6_IDX 8
typedef enum cpssHalCtrlPktIcmpV6MsgType_e
{
    CPSS_HAL_CTRLPKT_MESSAGE_TYPE_MLD_V1_V2                   = 130, //Need to check
    CPSS_HAL_CTRLPKT_MESSAGE_TYPE_MLD_V1_REPORT               = 131,
    CPSS_HAL_CTRLPKT_MESSAGE_TYPE_MLD_V1_DONE                 = 132,
    CPSS_HAL_CTRLPKT_MESSAGE_TYPE_MLD_V2_REPORT               = 143,
    CPSS_HAL_CTRLPKT_MESSAGE_TYPE_NDP_ROUTER_SOLICITATION     = 133,
    CPSS_HAL_CTRLPKT_MESSAGE_TYPE_NDP_ROUTER_ADVERTISEMENT    = 134,
    CPSS_HAL_CTRLPKT_MESSAGE_TYPE_NDP_NEIGHBOUR_SOLICITATION  = 135,
    CPSS_HAL_CTRLPKT_MESSAGE_TYPE_NDP_NEIGHBOUR_ADVERTISEMENT = 136,
    //CPSS_HAL_CTRLPKT_MESSAGE_TYPE_NDP_RIDERECT_MESSAGE        = 137,
} cpssHalCtrlPktIcmpV6MsgType_e;

typedef struct cpssHalCtrlPktIcmpV6MsgTypeCmd_t
{
    cpssHalCtrlPktIcmpV6MsgType_e msgType;
    CPSS_PACKET_CMD_ENT pktCmd;
} cpssHalCtrlPktIcmpV6MsgTypeCmd_t;

typedef enum cpssHalL4PortRangeIndex_e
{
    CPSS_HAL_L4_PORT_DHCPV4_IDX = 0,
    CPSS_HAL_L4_PORT_BGP_IDX,
    CPSS_HAL_L4_PORT_DHCPV6_IDX
} cpssHalL4PortRangeIndex_e;

/* Max 8 indices possible */
typedef enum cpssHalIPProtoIndex_e
{
    CPSS_HAL_IP_ICMPV6_IDX = 0
} cpssHalIPProtoIndex_t;


/**
 * \brief This type (enum) lists the some wellknown udp ports
 *        for ctrl pkts
 *
 */
typedef enum cpssHalCtrlPktUdpPort_e
{
    CPSS_HAL_CTRLPKT_UDP_PORT_DHCP_SERVER   = 67,
    CPSS_HAL_CTRLPKT_UDP_PORT_DHCP_CLIENT   = 68,
    CPSS_HAL_CTRLPKT_UDP_PORT_DHCPv6_SERVER = 547,
    CPSS_HAL_CTRLPKT_UDP_PORT_DHCPv6_CLIENT = 546,
    CPSS_HAL_CTRLPKT_UDP_PORT_SNMP          = 161,
    CPSS_HAL_CTRLPKT_UDP_PORT_SNMP_TRAP     = 162
} cpssHalCtrlPktUdpPort_e;

/**
 * \brief This type (enum) lists the LSB of IP link local mcast
 *        address 224.0.0.xx
 */
typedef enum cpssHalCtrlPktIpLinkLocalProto_e
{
    CPSS_HAL_CTRLPKT_IP_LL_PROTO_ALL_NODES  =  1,
    CPSS_HAL_CTRLPKT_IP_LL_PROTO_ALL_ROUTERS=  2,
    CPSS_HAL_CTRLPKT_IP_LL_PROTO_OSPF       =  5,
    CPSS_HAL_CTRLPKT_IP_LL_PROTO_OSPF_DR    =  6,
    CPSS_HAL_CTRLPKT_IP_LL_PROTO_OSPFV6     =  5,
    CPSS_HAL_CTRLPKT_IP_LL_PROTO_OSPFV6_DR  =  6,
    CPSS_HAL_CTRLPKT_IP_LL_PROTO_VRRPV6     =  12,
    CPSS_HAL_CTRLPKT_IP_LL_PROTO_PIMV2      =  13,
    CPSS_HAL_CTRLPKT_IP_LL_PROTO_PIMV6      =  13,
    CPSS_HAL_CTRLPKT_IP_LL_PROTO_VRRP       =  18,
} cpssHalCtrlPktIpLinkLocalProto_e;


/**
 * \brief API for initializing PP to handle all supported
 *        wellknown ctrl packets
 *
 * \param [in] devId
 *
 * \return GT_STATUS
*/
GT_STATUS cpssHalCtrlPktInit
(
    uint32_t devId
);

/**
 * \brief API for enabling ARP ctrl pkt handling at port level
 *
 * \param [in] devId
 * \param [in] portId
 * \param [in] enable
 *
 * \return GT_STATUS
*/
GT_STATUS cpssHalCtrlPktArpOnPort
(
    uint32_t devId,
    uint32_t portId,
    GT_BOOL enable
);

/**
 * \brief API for enabling ARP ctrl pkt handling at switch level
 *
 * \param [in] devId
 * \param [in] cmd
 *
 * \return GT_STATUS
*/
GT_STATUS cpssHalCtrlPktArpOnSwitch
(
    uint32_t devId,
    CPSS_PACKET_CMD_ENT cmd
);

/**
 * \brief API for enabling IEEE rsvd mcast ctrl pkt handling
 *        at port level
 *
 * \param [in] devId
 * \param [in] proto
 * \param [in] portId
 * \param [in] enable
 *
 * \return GT_STATUS
*/
GT_STATUS cpssHalCtrlPktIeeeReservedMcastProtoOnPort
(
    uint32_t devId,
    cpssHalCtrlPktResIeeeMCastProto_e proto,
    uint32_t portId,
    GT_BOOL enable
);

/**
 * \brief API for handling IEEE rsvd mcast ctrl pkt
 *        at switch level
 *
 * \param [in] devId
 * \param [in] proto
 * \param [in] cmd
 *
 * \return GT_STATUS
*/
GT_STATUS cpssHalCtrlPktIeeeReservedMcastProtoOnSwitch
(
    uint32_t devId,
    cpssHalCtrlPktResIeeeMCastProto_e proto,
    CPSS_PACKET_CMD_ENT cmd
);

/**
 * \brief API for enabling IGMP ctrl pkt handling at port level
 *
 * \param [in] devId
 * \param [in] portId
 * \param [in] enable
 *
 * \return GT_STATUS
*/
GT_STATUS cpssHalCtrlPktIgmpOnPort
(
    uint32_t devId,
    uint32_t portId,
    GT_BOOL enable
);


/**
 * \brief API for enabling IGMP ctrl pkt handling at vlan level
 *
 * \param [in] devId
 * \param [in] vlanId
 * \param [in] enable
 *
 * \return GT_STATUS
*/
GT_STATUS cpssHalCtrlPktIgmpOnVlan
(
    uint32_t devId,
    uint16_t vlanId,
    GT_BOOL enable
);

/**
 * \brief API for handling IGMP ctrl pkt at switch level
 *
 * \param [in] devId
 * \param [in] cmd
 *
 * \return GT_STATUS
*/
GT_STATUS cpssHalCtrlPktIgmpOnSwitch
(
    uint32_t devId,
    CPSS_PACKET_CMD_ENT cmd
);

/**
 * \brief API for enabling ICMPv6 ctrl pkt handling at port level
 *        This is not supported on the Aldrin device
 *
 * \param [in] devId
 * \param [in] portId
 * \param [in] enable
 *
 * \return GT_STATUS
*/
GT_STATUS cpssHalCtrlPktIcmpV6OnPort
(
    uint32_t devId,
    uint32_t portId,
    GT_BOOL enable
);

/**
 * \brief API for enabling ICMPv6 ctrl pkt handling at vlan level
 *
 * \param [in] devId
 * \param [in] vlanId
 * \param [in] enable
 *
 * \return GT_STATUS
*/
GT_STATUS cpssHalCtrlPktIcmpV6OnVlan
(
    uint32_t devId,
    uint32_t vlanId,
    GT_BOOL enable
);

/**
 * \brief API for handling ICMPv6 ctrl pkt at switch level
 *
 * \param [in] devId
 * \param [in] pktCmd
 * \param [in] msgType
 *
 * \return GT_STATUS
*/
GT_STATUS cpssHalCtrlPktIcmpV6OnSwitch
(
    uint32_t devId,
    CPSS_PACKET_CMD_ENT pktCmd,
    cpssHalCtrlPktIcmpV6MsgType_e msgType
);

/**
 * \brief API for enabling UDP ctrl pkt handling at port level
 *
 * \param [in] devId
 * \param [in] portId
 * \param [in] enable
 *
 * \return GT_STATUS
*/
GT_STATUS cpssHalCtrlPktUdpPortOnPort
(
    uint32_t devId,
    uint32_t portId,
    GT_BOOL enable
);

/**
 * \brief API for enabling UDP ctrl pkt handling at vlan level
 *
 * \param [in] devId
 * \param [in] vlanId
 * \param [in] enable
 *
 * \return GT_STATUS
*/
GT_STATUS cpssHalCtrlPktUdpPortOnVlan
(
    uint32_t devId,
    uint32_t vlanId,
    GT_BOOL enable
);

/**
 * \brief API for handling UDP ctrl pkt at switch level
 *
 * \param [in] devId
 * \param [in] pktCmd
 * \param [in] udpPort
 *
 * \return GT_STATUS
*/
GT_STATUS cpssHalCtrlPktUdpPortOnSwitch
(
    uint32_t devId,
    CPSS_PACKET_CMD_ENT pktCmd,
    cpssHalCtrlPktUdpPort_e udpPort
);

/**
 * \brief API for enabling IP link local ctrl pkt handling at port level
 *
 * \param [in] devId
 * \param [in] portId
 * \param [in] enable
 *
 * \return GT_STATUS
*/
GT_STATUS cpssHalCtrlPktIpLinkLocalOnPort
(
    uint32_t devId,
    uint32_t portId,
    GT_BOOL enable
);

/**
 * \brief API for enabling IP link local ctrl pkt handling at vlan level
 *
 * \param [in] devId
 * \param [in] vlanId
 * \param [in] enable
 *
 * \return GT_STATUS
*/
GT_STATUS cpssHalCtrlPktIpLinkLocalOnVlan
(
    uint32_t devId,
    uint32_t vlanId,
    GT_BOOL enable
);

/**
 * \brief API for handling IP link local ctrl pkt at switch level
 *
 * \param [in] devId
 * \param [in] pktCmd
 * \param [in] proto
 *
 * \return GT_STATUS
*/
GT_STATUS cpssHalCtrlPktIpLinkLocalOnSwitch
(
    uint32_t devId,
    CPSS_PACKET_CMD_ENT pktCmd,
    cpssHalCtrlPktIpLinkLocalProto_e proto
);

/**
 * \brief API for handling Ssh ctrl pkt at switch level
 *
 * \param [in] devId
 * \param [in] pktCmd
 *
 * \return GT_STATUS
*/
GT_STATUS cpssHalCtrlPktSshOnSwitch
(
    uint32_t devId,
    CPSS_PACKET_CMD_ENT pktCmd
);

/**
 * \brief API for handling BGPv4 and BGPv6 ctrl pkt at switch level
 *
 * \param [in] devId
 * \param [in] bgpType
 * \param [in] pktCmd
 *
 * \return GT_STATUS
*/
GT_STATUS cpssHalCtrlPktBgpOnSwitch
(
    uint32_t devId,
    cpssHalBgpType bgpType,
    CPSS_PACKET_CMD_ENT pktCmd
);

/**
 * \brief API for handling DHCPv6 ctrl pkt at switch level
 *
 * \param [in] devId
 * \param [in] pktCmd
 *
 * \return GT_STATUS
*/
GT_STATUS cpssHalCtrlPktDhcpV6OnSwitch
(
    uint32_t devId,
    CPSS_PACKET_CMD_ENT pktCmd
);

/**
 * \brief API for handling L2 DHCPv6 ctrl pkt at switch level
 *
 * \param [in] devId
 * \param [in] pktCmd
 *
 * \return GT_STATUS
*/
GT_STATUS cpssHalCtrlPktDhcpV6L2OnSwitch
(
    uint32_t devId,
    CPSS_PACKET_CMD_ENT pktCmd
);

GT_STATUS cpssHalCtrlPktDhcpV4OnSwitch
(
    uint32_t devId,
    CPSS_PACKET_CMD_ENT pktCmd
);

/**
 * \brief API for handling ARP ctrl pkt at switch level with TCAM.
 *
 * \param [in] devId
 * \param [in] pktCmd
 *
 * \return GT_STATUS
*/
GT_STATUS cpssHalCtrlPktArpTcamOnSwitch
(
    uint32_t devId,
    uint32_t cpuCode,
    CPSS_PACKET_CMD_ENT pktCmd
);
/**
 * \brief API for handling CISCO Layer 2 proprietary protocols
 *        ctrl pkt at switch level
 *
 * \param [in] devId
 * \param [in] pktCmd
 *
 * \return iT_STATUS
*/
GT_STATUS cpssHalCtrlPktCiscoL2ProtOnSwitch
(
    uint32_t devId,
    CPSS_PACKET_CMD_ENT pktCmd
);

/**
 * \brief API for handling IPv4 and IPv6 Mtu exceeded at switch level
 *
 * \param [in] devId
 * \param [in] pktCmd
 *
 * \return GT_STATUS
*/
GT_STATUS cpssHalCtrlPktIpMtuExceedOnSwitch
(
    uint32_t devId,
    CPSS_PACKET_CMD_ENT pktCmd
);

/**
 * \brief API for handling IPv4 and IPv6 TTl error at switch level
 *
 * \param [in] devId
 * \param [in] pktCmd
 *
 * \return GT_STATUS
*/
GT_STATUS cpssHalCtrlPktIpTtlErrorOnSwitch
(
    uint32_t devId,
    CPSS_PACKET_CMD_ENT pktCmd
);

/**
 * \brief API for displaying ctrl pkts setting at switch level
 *
 * \param [in] devId
 * \param [in] type
 *
 * \return GT_STATUS
*/
GT_STATUS cpssHalCtrlPktDump
(
    uint32_t devId,
    cpssHalCtrlPktTypes_e type
);
GT_STATUS cpssHalCtrlPktRsvdMcastMacOnSwitch
(
    uint32_t devId,
    CPSS_PACKET_CMD_ENT pktCmd,
    CPSS_NET_RX_CPU_CODE_ENT cpuCode,
    macAddr_t macAddr,
    cpssHalCtrlPktAclPriority_e ruleId,
    uint16_t etherType
);
GT_STATUS cpssHalCtrlPktLLDPOnSwitch
(
    uint32_t devId,
    CPSS_PACKET_CMD_ENT pktCmd
);
GT_STATUS cpssHalCtrlPktEAPOLOnSwitch
(
    uint32_t devId,
    CPSS_PACKET_CMD_ENT pktCmd
);
GT_STATUS cpssHalCtrlPktLACPOnSwitch
(
    uint32_t devId,
    CPSS_PACKET_CMD_ENT pktCmd
);

GT_STATUS cpssHalCtrlPktFlowControl
(
    uint32_t devId,
    uint32_t tableId,
    uint16_t opCode,
    CPSS_PACKET_CMD_ENT pktCmd,
    cpssHalCtrlPktAclPriority_e ruleId,
    bool is_control_acl
);

GT_STATUS cpssHalCtrlPktCnpOnPort
(
    uint32_t devId,
    uint32_t port,
    cpssHalIpType ipType
);

GT_STATUS cpssHalCtrlCommonPacketDrop
(
    uint32_t devId,
    bool isIpv4Type,
    bool isSip,
    ipv4Addr_t ipv4AddressPattern,
    ipv4Addr_t ipv4AddressMask,
    ipv6Addr_t ipv6AddressPattern,
    ipv6Addr_t ipv6AddressMask,
    uint32_t ruleId
);

GT_STATUS cpssHalNetIfAppSpecificCpuCodeTcpUdpPortRangeSet(uint32_t devId,
                                                           uint32_t index,
                                                           uint16_t minL4Port,
                                                           uint16_t maxL4Port,
                                                           CPSS_NET_PROT_ENT proto,
                                                           uint32_t cpuCode
                                                          );
GT_STATUS cpssHalNetIfAppSpecificCpuCodeIpProtocolSet(uint32_t devNum,
                                                      uint32_t index,
                                                      uint8_t protocol,
                                                      uint32_t cpuCode);

XP_STATUS cpssHalCtrlPktInbandMgmt
(
    uint32_t devId, uint32_t port
);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __cpssHalCtrlPkth */

