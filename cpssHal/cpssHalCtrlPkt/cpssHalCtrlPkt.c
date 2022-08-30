/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/**
********************************************************************************
* @file cpssHalCtrlPkt.c
*
* @brief Private API implementation for CPSS CPU Code Table Read, Write, Update
* which can be used in XPS layer.
* @version   01
********************************************************************************
*/

#include "xpsCopp.h"
#include "cpssHalFdb.h"
#include "cpssHalUtil.h"
#include "xpsCommon.h"
#include "xpsInternal.h"
#include "openXpsEnums.h"
#include "openXpsInterface.h"
#include "openXpsCopp.h"
#include "cpssHalDevice.h"
#include "xpsPort.h"
#include "xpsScope.h"
#include "cpssDxChBrgGen.h"
#include "xpsGlobalSwitchControl.h"
#include "cpssDxChNetIf.h"
#include "cpssHalCopp.h"
#include "cpssHalCtrlPkt.h"
#include "cpssDxChIpCtrl.h"
#include "xpsAcl.h"
#include "xpsVlan.h"
#include "xpsAllocator.h"
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include "cpssHalPort.h"
#include "cpssHalAcl.h"
#include <cpss/dxCh/dxChxGen/policer/cpssDxChPolicer.h>

/*
 *  CPSS_HAL_CTRLPKT_TYPE_ARP - REQUEST, RESPONSE
 *      cpssHalCtrlPktArpOnPort
 *      cpssHalCtrlPktArpOnSwitch
 *
 *  CPSS_HAL_CTRLPKT_TYPE_IEEE_RESERVED_MCAST - STP, LACP, LLDP
 *      cpssHalCtrlPktIeeeReservedMcastProtoOnPort
 *      cpssHalCtrlPktIeeeReservedMcastProtoOnSwitch
 *
 *  CPSS_HAL_CTRLPKT_TYPE_IGMP - QUERY, LEAVE, V1_REPORT, V2_REPORT, V3_REPORT
 *      cpssHalCtrlPktIgmpOnPort
 *      cpssHalCtrlPktIgmpOnVlan
 *      cpssHalCtrlPktIgmpOnSwitch
 *
 *  CPSS_HAL_CTRLPKT_TYPE_ICMPV6 - MLD_V1_V2, MLD_V1_REPORT, MLD_V1_DONE, MLD_V2_REPORT, NEIGHBOR_DISCOVERY
 *      cpssHalCtrlPktIcmpV6OnPort - not spported
 *      cpssHalCtrlPktIcmpV6OnVlan
 *      cpssHalCtrlPktIcmpV6OnSwitch
 *
 *  CPSS_HAL_CTRLPKT_TYPE_UDP_PORT - DHCP, SNMP
 *      cpssHalCtrlPktUdpPortOnPort
 *      cpssHalCtrlPktUdpPortOnVlan
 *      cpssHalCtrlPktUdpPortOnSwitch
 *
 *  CPSS_HAL_CTRLPKT_TYPE_IP_LINK_LOCAL - OSPFV6, OSPF, PIM, VRRP, VRRPV6
 *      cpssHalCtrlPktIpLinkLocalOnPort
 *      cpssHalCtrlPktIpLinkLocalOnVlan
 *      cpssHalCtrlPktIpLinkLocalOnSwitch
 *
 *  CPSS_HAL_CTRLPKT_TYPE_CISCO_MULTICAST
 *      cpssHalCtrlPktCiscoL2ProtOnSwitch
 *
 *  CTRL TCAM ACL - BGPV6, BGP,DHCPV6,SSH
 *  TBD - SAMPLEPACKET, EAPOL
 */
#define CPSS_HAL_CTRL_PKT_ARP_MAX_KEY_FLDS 1
#define CPSS_HAL_CTRL_PKT_BGP_MAX_KEY_FLDS 3
#define CPSS_HAL_CTRL_PKT_BGP_RSVD_TCP_PORT 179
#define CPSS_HAL_CTRL_PKT_SSH_RSVD_TCP_PORT 22
#define CPSS_HAL_CTRL_PKT_DHCPV6_SERVER_KEY_FLDS  5
#define CPSS_HAL_CTRL_PKT_DHCPV6_CLIENT_KEY_FLDS  4
#define CPSS_HAL_CTRL_PKT_DHCPV6_SERVER_RSVD_UDP_PORT 547
#define CPSS_HAL_CTRL_PKT_DHCPV6_CLIENT_RSVD_UDP_PORT 546


uint32_t ctrlPacketIpv4AclTableId = 0xFFFFFFFF;
uint32_t ctrlPacketIpv6AclTableId = 0xFFFFFFFF;
uint32_t ctrlPacketPFCAclTableId = 0xFFFFFFFF;
uint32_t ctrlPacketL3Ipv4CommonDropAclTableId = 0xFFFFFFFF;
uint32_t ctrlPacketL3Ipv6CommonDropAclTableId = 0xFFFFFFFF;

uint32_t ctrlPacketEgressIpv4AclTableId = 0xFFFFFFFF;
uint32_t ctrlPacketEgressIpv6AclTableId = 0xFFFFFFFF;

/* Acl table for Cpu port */
uint32_t ctrlPacketForCpuEgressIpv4AclTableId = 0xFFFFFFFF;
uint32_t ctrlPacketForCpuEgressIpv6AclTableId = 0xFFFFFFFF;

cpssHalCtrlPktIcmpV6MsgTypeCmd_t sIcmpV6MsgTypeCmdArray [MAX_ICMPV6_IDX] =
{
    /*ICMP IPv6 REDIRECT will be UC Routed*/
    /* msgType                                       pktCmd                    */
    {CPSS_HAL_CTRLPKT_MESSAGE_TYPE_MLD_V1_V2, CPSS_PACKET_CMD_FORWARD_E},
    {CPSS_HAL_CTRLPKT_MESSAGE_TYPE_MLD_V1_REPORT, CPSS_PACKET_CMD_FORWARD_E},
    {CPSS_HAL_CTRLPKT_MESSAGE_TYPE_MLD_V1_DONE,   CPSS_PACKET_CMD_FORWARD_E},
    {CPSS_HAL_CTRLPKT_MESSAGE_TYPE_MLD_V2_REPORT,   CPSS_PACKET_CMD_FORWARD_E},
    {CPSS_HAL_CTRLPKT_MESSAGE_TYPE_NDP_ROUTER_SOLICITATION,  CPSS_PACKET_CMD_FORWARD_E},
    {CPSS_HAL_CTRLPKT_MESSAGE_TYPE_NDP_ROUTER_ADVERTISEMENT, CPSS_PACKET_CMD_FORWARD_E},
    {CPSS_HAL_CTRLPKT_MESSAGE_TYPE_NDP_NEIGHBOUR_SOLICITATION, CPSS_PACKET_CMD_FORWARD_E},
    {CPSS_HAL_CTRLPKT_MESSAGE_TYPE_NDP_NEIGHBOUR_ADVERTISEMENT, CPSS_PACKET_CMD_FORWARD_E},
};

typedef enum cpssHalCtrlPktResIeeeMCastProtoPortProfile_e
{
    CPSS_HAL_CTRLPKT_RES_IEEE_MCAST_PORT_PROFILE_DEF   = 0,
    CPSS_HAL_CTRLPKT_RES_IEEE_MCAST_PORT_PROFILE_STP   = 1,
    CPSS_HAL_CTRLPKT_RES_IEEE_MCAST_PORT_PROFILE_LACP  = 2,
    CPSS_HAL_CTRLPKT_RES_IEEE_MCAST_PORT_PROFILE_LLDP  = 3,
} cpssHalCtrlPktResIeeeMCastProtoPortProfile_e;

typedef struct cpssHalCtrlPktResIeeeMCastCpuCodeMap_t
{
    cpssHalCtrlPktResIeeeMCastProto_e ieeeMcProto;
    cpssHalCtrlPktResIeeeMCastProtoPortProfile_e profile;
    CPSS_NET_RX_CPU_CODE_ENT         cpuCode;
} cpssHalCtrlPktResIeeeMCastCpuCodeMap_t;

/**
 * \brief This type Array lists the port and cpuCod mapping for
 *        IEEE Reserved Multicast (01:08:C2:00:00:XX)
 */
cpssHalCtrlPktResIeeeMCastCpuCodeMap_t ieeeMcCpuCodeArray[] =
{
    {CPSS_HAL_CTRLPKT_RES_IEEE_MCAST_PROTO_STP,  CPSS_HAL_CTRLPKT_RES_IEEE_MCAST_PORT_PROFILE_STP,  CPSS_NET_IEEE_RSRVD_MULTICAST_ADDR_1_E},
    {CPSS_HAL_CTRLPKT_RES_IEEE_MCAST_PROTO_LACP, CPSS_HAL_CTRLPKT_RES_IEEE_MCAST_PORT_PROFILE_LACP, CPSS_NET_IEEE_RSRVD_MULTICAST_ADDR_2_E},
    {CPSS_HAL_CTRLPKT_RES_IEEE_MCAST_PROTO_LLDP, CPSS_HAL_CTRLPKT_RES_IEEE_MCAST_PORT_PROFILE_LLDP, CPSS_NET_IEEE_RSRVD_MULTICAST_ADDR_3_E},
};


typedef struct cpssHalCtrlPktUdpPortCpuCodePktCmd_t
{
    cpssHalCtrlPktUdpPort_e udpPort;
    CPSS_NET_RX_CPU_CODE_ENT cpuCode;
    CPSS_PACKET_CMD_ENT pktCmd;
} cpssHalCtrlPktUdpPortCpuCodePktCmd_t;

/* HW allows to configure 12 UDP idx*/
/*
 * UDP Broadcast table supports IPv4 traffic only. So DHCPv6 UDP ports are NOT
 * included in this array.
 */
cpssHalCtrlPktUdpPortCpuCodePktCmd_t sUdpPortPktCmd[] =
{
    /* udpPort                                   cpuCode                       pktCmd                    */
    {CPSS_HAL_CTRLPKT_UDP_PORT_DHCP_SERVER, CPSS_NET_UDP_BC_MIRROR_TRAP0_E, CPSS_PACKET_CMD_MIRROR_TO_CPU_E},
    {CPSS_HAL_CTRLPKT_UDP_PORT_DHCP_CLIENT, CPSS_NET_UDP_BC_MIRROR_TRAP0_E, CPSS_PACKET_CMD_MIRROR_TO_CPU_E},
    {CPSS_HAL_CTRLPKT_UDP_PORT_SNMP, CPSS_NET_UDP_BC_MIRROR_TRAP2_E, CPSS_PACKET_CMD_TRAP_TO_CPU_E},
    {CPSS_HAL_CTRLPKT_UDP_PORT_SNMP_TRAP, CPSS_NET_UDP_BC_MIRROR_TRAP2_E, CPSS_PACKET_CMD_TRAP_TO_CPU_E},
};


typedef struct cpssHalCtrlPktIpLinkLocalProtoCpuCode_t
{
    IN CPSS_IP_PROTOCOL_STACK_ENT    ipVer;
    /* LSB of link local mcast add 224.0.0.xx */
    cpssHalCtrlPktIpLinkLocalProto_e ipLLProto;
    CPSS_NET_RX_CPU_CODE_ENT         cpuCode;
} cpssHalCtrlPktIpLinkLocalProtoCpuCode_t;

cpssHalCtrlPktIpLinkLocalProtoCpuCode_t sIpLLCpuCode[] =
{
    /* ipVer                    ipLLProto                               cpuCode                                 */
    {CPSS_IP_PROTOCOL_IPV4_E, CPSS_HAL_CTRLPKT_IP_LL_PROTO_OSPF, CPSS_NET_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_1_E},
    {CPSS_IP_PROTOCOL_IPV4_E, CPSS_HAL_CTRLPKT_IP_LL_PROTO_OSPF_DR, CPSS_NET_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_1_E},
    {CPSS_IP_PROTOCOL_IPV6_E, CPSS_HAL_CTRLPKT_IP_LL_PROTO_OSPFV6, CPSS_NET_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_1_E},
    {CPSS_IP_PROTOCOL_IPV6_E, CPSS_HAL_CTRLPKT_IP_LL_PROTO_OSPFV6_DR, CPSS_NET_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_1_E},
    {CPSS_IP_PROTOCOL_IPV4_E, CPSS_HAL_CTRLPKT_IP_LL_PROTO_VRRP, CPSS_NET_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_2_E},
    {CPSS_IP_PROTOCOL_IPV4_E, CPSS_HAL_CTRLPKT_IP_LL_PROTO_PIMV2, CPSS_NET_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_3_E},
    {CPSS_IP_PROTOCOL_IPV6_E, CPSS_HAL_CTRLPKT_IP_LL_PROTO_PIMV6, CPSS_NET_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_3_E},
    {CPSS_IP_PROTOCOL_IPV4_E, CPSS_HAL_CTRLPKT_IP_LL_PROTO_VRRPV6, CPSS_NET_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_2_E},
};


/*******************************************************************************
   NAME
   cpssHalCtrlPktGetIeeeReservedMCastProtoPortProfile

   DESCRIPTION

******************************************************************************/

GT_U8 cpssHalCtrlPktGetIeeeReservedMCastProtoPortProfile
(
    cpssHalCtrlPktResIeeeMCastProto_e proto
)
{
    switch (proto)
    {
        case CPSS_HAL_CTRLPKT_RES_IEEE_MCAST_PROTO_STP:
            return CPSS_HAL_CTRLPKT_RES_IEEE_MCAST_PORT_PROFILE_STP;
        case CPSS_HAL_CTRLPKT_RES_IEEE_MCAST_PROTO_LACP:
            return CPSS_HAL_CTRLPKT_RES_IEEE_MCAST_PORT_PROFILE_LACP;
        case CPSS_HAL_CTRLPKT_RES_IEEE_MCAST_PROTO_LLDP:
            return CPSS_HAL_CTRLPKT_RES_IEEE_MCAST_PORT_PROFILE_LLDP;
        default:
            return CPSS_HAL_CTRLPKT_RES_IEEE_MCAST_PORT_PROFILE_DEF;
    }
}

static GT_STATUS cpssHalCtrlPktArpInit
(
    uint32_t devId,
    CPSS_DXCH_ARP_BCAST_CMD_MODE_ENT cmdMode,
    CPSS_PACKET_CMD_ENT cmd
)
{
    GT_U8           cpssDevNum;
    GT_STATUS       rc = GT_OK;

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        if (cmd == CPSS_PACKET_CMD_DROP_HARD_E)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChBrgGenArpBcastToCpuCmdSet CMD_DROP_HARD_E is not supported");
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
                  "Default packet action forward is set for ctrl pkt ARP\n");
            return rc;
        }
        rc = cpssDxChBrgGenArpBcastToCpuCmdSet(cpssDevNum,
                                               cmdMode, cmd);
        if (GT_OK != rc)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChBrgGenArpBcastToCpuCmdSet with port mode and mirror failed  %d\n", rc);
            return rc;
        }
    }
    return GT_OK;
}

static XP_STATUS cpssHalInitializePclForCtrlPkt
(
    uint32_t devId
)
{
    XP_STATUS retVal = XP_NO_ERR;
    GT_STATUS rc = GT_OK;
    xpAclTableInfo_t     tableInfo;
    uint32_t tableId = 0;

    memset(&tableInfo, 0x00, sizeof(xpAclTableInfo_t));

    tableInfo.tableType = XPS_ACL_TABLE_TYPE_IPV4;
    retVal = xpsAclCreateTable(devId, tableInfo, &tableId);
    if (retVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: Failed to create table for the device %u \n", devId);
        return retVal;
    }
    ctrlPacketIpv4AclTableId = tableId;

    tableInfo.tableType = XPS_ACL_TABLE_TYPE_IPV6;
    retVal = xpsAclCreateTable(devId, tableInfo, &tableId);
    if (retVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: Failed to create table for the device %u \n", devId);
        return retVal;
    }
    ctrlPacketIpv6AclTableId = tableId;

    if (cpssHalDevPPFamilyGet(devId) == CPSS_PP_FAMILY_DXCH_FALCON_E ||
        cpssHalDevPPFamilyGet(devId) == CPSS_PP_FAMILY_DXCH_AC5X_E)
    {
        tableInfo.tableType = XPS_ACL_TABLE_TYPE_NON_IP;
        retVal = xpsAclCreateTable(devId, tableInfo, &tableId);
        if (retVal != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Error: Failed to create table for the device %u \n", devId);
            return retVal;
        }
        ctrlPacketPFCAclTableId = tableId;

        /* Add ACL rule to drop 802_3x pause frames at switch level.*/
        uint16_t opCode = 0x0001;
        rc = cpssHalCtrlPktFlowControl(devId, ctrlPacketIpv4AclTableId, opCode,
                                       CPSS_PACKET_CMD_DROP_HARD_E,
                                       CPSS_HAL_CTRLPKT_ACL_PRIO_802_3_FLOW_CONTROL,
                                       true);
        if (rc != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssHalCtrlPktFlowControl failed %d\n", rc);
            return GT_FAIL;
        }
    }

    tableInfo.tableType = XPS_ACL_TABLE_TYPE_IPV4;
    retVal = xpsAclCreateTable(devId, tableInfo, &tableId);
    if (retVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: Failed to create table for the device %u \n", devId);
        return retVal;
    }
    ctrlPacketL3Ipv4CommonDropAclTableId = tableId;

    tableInfo.tableType = XPS_ACL_TABLE_TYPE_IPV6;
    retVal = xpsAclCreateTable(devId, tableInfo, &tableId);
    if (retVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: Failed to create table for the device %u \n", devId);
        return retVal;
    }
    ctrlPacketL3Ipv6CommonDropAclTableId = tableId;

    if (cpssHalIsEbofDevice(devId))
    {
        /* allocating table id for IPv4 & IPv6 egress drop rules due to fabrico error HW issue */
        tableInfo.tableType = XPS_ACL_TABLE_TYPE_IPV4;
        tableInfo.stage = XPS_PCL_STAGE_EGRESS;
        retVal = xpsAclCreateTable(devId, tableInfo, &tableId);
        if (retVal != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Error: Failed to create table for the device %u \n", devId);
            return retVal;
        }
        ctrlPacketEgressIpv4AclTableId = tableId;

        tableInfo.tableType = XPS_ACL_TABLE_TYPE_IPV6;
        tableInfo.stage = XPS_PCL_STAGE_EGRESS;
        retVal = xpsAclCreateTable(devId, tableInfo, &tableId);
        if (retVal != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Error: Failed to create table for the device %u \n", devId);
            return retVal;
        }
        ctrlPacketEgressIpv6AclTableId = tableId;
    }

    /* Create ACL table for CPU port to handle policer */
    uint32_t cpuPortNum;
    rc = cpssHalGlobalSwitchControlGetCpuPhysicalPortNum(devId, &cpuPortNum);
    if (rc != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssHalGlobalSwitchControlGetCpuPhysicalPortNum failed %d\n", rc);
        return GT_FAIL;
    }

    /* Create IPv4 egress ACL and bind to cpu port */
    tableInfo.tableType = XPS_ACL_TABLE_TYPE_IPV4;
    tableInfo.stage = XPS_PCL_STAGE_EGRESS;
    retVal = xpsAclCreateTable(devId, tableInfo, &tableId);
    if (retVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: Failed to create table for the device %u \n", devId);
        return retVal;
    }
    ctrlPacketForCpuEgressIpv4AclTableId = tableId;
    retVal = xpsAclSetPortAcl(devId, cpuPortNum,
                              ctrlPacketForCpuEgressIpv4AclTableId, 0);
    if (retVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: Failed to set IPv4 acl for egress port at device %u \n", devId);
        return retVal;
    }

    /* Create IPv6 egress ACL and bind to cpu port */
    tableInfo.tableType = XPS_ACL_TABLE_TYPE_IPV6;
    tableInfo.stage = XPS_PCL_STAGE_EGRESS;
    retVal = xpsAclCreateTable(devId, tableInfo, &tableId);
    if (retVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: Failed to create table for the device %u \n", devId);
        return retVal;
    }
    ctrlPacketForCpuEgressIpv6AclTableId = tableId;
    retVal = xpsAclSetPortAcl(devId, cpuPortNum,
                              ctrlPacketForCpuEgressIpv6AclTableId, 0);
    if (retVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: Failed to set IPv6 acl for egress port at device %u \n", devId);
        return retVal;
    }

    if (cpssHalDevPPFamilyGet(devId) == CPSS_PP_FAMILY_DXCH_FALCON_E)
    {
        /* Enable policer metering on trap pkts */
        rc = cpssDxChPolicerMeteringOnTrappedPktsEnableSet(devId,
                                                           CPSS_DXCH_POLICER_STAGE_EGRESS_E,
                                                           true);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChPolicerMeteringOnTrappedPktsEnableSet egress enable failed  %d \n",
                  rc);
            return rc;
        }
    }

#if 0
    rc = cpssHalCtrlPktArpTcamOnSwitch(devId, CPSS_PACKET_CMD_TRAP_TO_CPU_E);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssHalCtrlPktArpTcamOnSwitch failed with for pktCmd trap status %d \n", rc);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    //Default ctrl ACL rules(BGP, BGPV6, DHCPV6, SSH)
    rc = cpssHalCtrlPktBgpOnSwitch(devId, CPSS_HAL_BGP_IPV4,
                                   CPSS_PACKET_CMD_DROP_HARD_E);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssHalCtrlPktBgpOnSwitch failed  for ipv4 pktCmd drop status %d \n", rc);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    rc = cpssHalCtrlPktBgpOnSwitch(devId, CPSS_HAL_BGP_IPV6,
                                   CPSS_PACKET_CMD_DROP_HARD_E);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssHalCtrlPktBgpOnSwitch failed with for ipv6 pktCmd drop status %d \n", rc);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    rc = cpssHalCtrlPktDhcpV6OnSwitch(devId, CPSS_PACKET_CMD_FORWARD_E);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssHalCtrlPktDhcpV6OnSwitch failed with for pktCmd fwd status %d \n", rc);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    rc = cpssHalCtrlPktSshOnSwitch(devId, CPSS_PACKET_CMD_DROP_HARD_E);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssHalCtrlPktSshOnSwitch failed with for pktCmd drop status %d \n", rc);
        return xpsConvertCpssStatusToXPStatus(rc);
    }
#endif
    return retVal;

}
GT_STATUS cpssHalCtrlPktIeeeReservedMcastInit
(
    uint32_t devId
)
{
    GT_PORT_NUM     cpssPortNum;
    GT_U8           cpssDevNum;
    GT_STATUS       rc = GT_OK;
    GT_U32          portId;
    GT_U8           maxTotalPorts, ieeeMcIdx;

    if (xpsGlobalSwitchControlGetMaxPorts(devId, &maxTotalPorts) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Can't get max physical port number\n");
        return GT_FAIL;
    }


    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        /* 1.Enable feature to handle IEEE Reserved Multicast (01:08:C2:00:00:XX) */
        if ((rc = cpssDxChBrgGenIeeeReservedMcastTrapEnable(cpssDevNum,
                                                            GT_TRUE)) != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChBrgGenIeeeReservedMcastTrapEnable failed  %d \n", rc);
            return rc;
        }
        /* 2.Configure CPU code corresponding to IEEE Reserved Multicast control packets*/
        for (ieeeMcIdx = 0;
             ieeeMcIdx < sizeof(ieeeMcCpuCodeArray)/sizeof(
                 cpssHalCtrlPktResIeeeMCastCpuCodeMap_t); ieeeMcIdx++)
        {
            if ((rc = cpssDxChNetIfCpuCodeIeeeReservedMcastProtSet(cpssDevNum,
                                                                   ieeeMcCpuCodeArray[ieeeMcIdx].ieeeMcProto,
                                                                   ieeeMcCpuCodeArray[ieeeMcIdx].cpuCode)) != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssDxChNetIfCpuCodeIeeeReservedMcastProtSet: ret[%d] cpssDevNum[%d] proto [%d]",
                      rc, cpssDevNum, ieeeMcCpuCodeArray[ieeeMcIdx].ieeeMcProto);
                return rc;
            }
        }

    }

    /* 3.Create a profileIndex to handle ieee reserved mac behavior on all the ports */
    XPS_GLOBAL_PORT_ITER(portId, maxTotalPorts)
    {
        cpssDevNum = xpsGlobalIdToDevId(devId, portId);
        cpssPortNum = xpsGlobalPortToPortnum(devId, portId);

        if ((rc = cpssDxChBrgGenPortIeeeReservedMcastProfileIndexSet(cpssDevNum,
                                                                     cpssPortNum, CPSS_HAL_CTRLPKT_RES_IEEE_MCAST_PORT_PROFILE_DEF)) != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChBrgGenPortIeeeReservedMcastProfileIndexSet port %d failed  %d \n",
                  cpssPortNum, rc);
            return rc;
        }
    }

    return GT_OK;
}

GT_STATUS cpssHalCtrlPktIgmpInit
(
    uint32_t devId
)
{
    GT_U8           cpssDevNum;
    GT_STATUS       rc = GT_OK;

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        /** All IGMP packets are mirrored to CPU if the vlan level snoop is enabled */
        rc = cpssDxChBrgGenIgmpSnoopModeSet(cpssDevNum, CPSS_IGMP_ROUTER_MIRROR_MODE_E);
        if (GT_OK != rc)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChBrgGenIgmpSnoopModeSet with ROUTER_MIRROR_MODE_E failed  %d\n", rc);
            return rc;
        }
    }
    return GT_OK;
}

GT_STATUS cpssHalCtrlPktIcmpV6Init
(
    uint32_t devId
)
{
    GT_U8           cpssDevNum;
    GT_STATUS       rc = GT_OK;
    GT_U8           msgIdx;

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        /* Initialize all IcmpV6 message types as CPSS_PACKET_FORWARD_E */
        for (msgIdx = 0;
             msgIdx < sizeof(sIcmpV6MsgTypeCmdArray)/sizeof(
                 cpssHalCtrlPktIcmpV6MsgTypeCmd_t); msgIdx++)
        {
            /* reserving index 0..7 for IcmpV6 msgTypes */
            rc = cpssDxChBrgGenIcmpv6MsgTypeSet(cpssDevNum, msgIdx,
                                                sIcmpV6MsgTypeCmdArray[msgIdx].msgType, sIcmpV6MsgTypeCmdArray[msgIdx].pktCmd);
            if (GT_OK != rc)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssDxChBrgGenIcmpv6MsgTypeSet failed for %d rc %d\n", msgIdx, rc);
                return rc;
            }
        }

    }
    return GT_OK;
}

GT_STATUS cpssHalCtrlPktUdpPortInit
(
    uint32_t devId
)
{
    GT_U8           cpssDevNum;
    GT_STATUS       rc = GT_OK;
    GT_U8           udpIdx;
    GT_PORT_NUM     cpssPortNum;
    GT_U32          portId;
    GT_U8           maxTotalPorts;
    GT_BOOL         enable = GT_TRUE;

    if (xpsGlobalSwitchControlGetMaxPorts(devId, &maxTotalPorts) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Can't get max physical port number\n");
        return GT_FAIL;
    }

    XPS_GLOBAL_PORT_ITER(portId, maxTotalPorts)
    {
        cpssDevNum = xpsGlobalIdToDevId(devId, portId);
        cpssPortNum = xpsGlobalPortToPortnum(devId, portId);

        /* required udp port and command mapping is done in init */
        rc = cpssDxChBrgGenPortBcUdpTrapMirrorEnableSet(cpssDevNum, cpssPortNum,
                                                        enable);
        if (GT_OK != rc)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChBrgGenPortBcUdpTrapMirrorEnableSet FAILED, rc = [%d]", rc);
            return rc;
        }
    }

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        /* udp port to cpuCode and pktCmd mapping */
        for (udpIdx = 0;
             udpIdx <  sizeof(sUdpPortPktCmd)/sizeof(cpssHalCtrlPktUdpPortCpuCodePktCmd_t);
             udpIdx++)
        {
            /*
             * UDP BC table is configured for DHCP V4 when the DHCP_L2 trap is added.
             * This is done from xpsCoppEnableCtrlPktTrapOnSwitch.
             * ACL is programmed for DHCP V6 L2 packets when the DHCPV6_L2 V6 trap is added.
             * This is done from xpsCoppEnableCtrlPktTrapOnSwitch.
             */
            if ((sUdpPortPktCmd[udpIdx].udpPort == CPSS_HAL_CTRLPKT_UDP_PORT_SNMP) ||
                (sUdpPortPktCmd[udpIdx].udpPort == CPSS_HAL_CTRLPKT_UDP_PORT_SNMP_TRAP))
            {
                rc = cpssDxChBrgGenUdpBcDestPortCfgSet(cpssDevNum, udpIdx,
                                                       sUdpPortPktCmd[udpIdx].udpPort,
                                                       sUdpPortPktCmd[udpIdx].cpuCode,
                                                       sUdpPortPktCmd[udpIdx].pktCmd);
                if (GT_OK != rc)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "cpssDxChBrgGenUdpBcDestPortCfgSet failed for udpPort %d, error  %d\n",
                          sUdpPortPktCmd[udpIdx].udpPort, rc);
                    return rc;
                }
            }
        }
    }

    return GT_OK;
}

GT_STATUS cpssHalCtrlPktIpLinkLocalInit
(
    uint32_t devId
)
{
    GT_U8           cpssDevNum;
    GT_STATUS       rc = GT_OK;
    GT_U8           ipLLIdx;

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        /* 1.Enable the link local handling for v4 and v6 */
        rc = cpssDxChBrgGenIpLinkLocalMirrorToCpuEnable(cpssDevNum,
                                                        CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
        if (GT_OK != rc)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChBrgGenIpLinkLocalMirrorToCpuEnable failed  %d \n", rc);
            return rc;
        }

        rc = cpssDxChBrgGenIpLinkLocalMirrorToCpuEnable(cpssDevNum,
                                                        CPSS_IP_PROTOCOL_IPV6_E, GT_TRUE);
        if (GT_OK != rc)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChBrgGenIpLinkLocalMirrorToCpuEnable failed  %d \n", rc);
            return rc;
        }

        /* 2.Initializing the linklocal 224.0.0.1 protocol to cpuCode mapping */
        for (ipLLIdx = 0;
             ipLLIdx < sizeof(sIpLLCpuCode)/sizeof(cpssHalCtrlPktIpLinkLocalProtoCpuCode_t);
             ipLLIdx++)
        {
            rc = cpssDxChBrgGenIpLinkLocalProtMirrorToCpuEnable(cpssDevNum,
                                                                sIpLLCpuCode[ipLLIdx].ipVer, sIpLLCpuCode[ipLLIdx].ipLLProto, GT_TRUE);
            if (GT_OK != rc)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssDxChBrgGenIpLinkLocalProtMirrorToCpuEnable failed  %d \n", rc);
                return rc;
            }

            rc = cpssDxChNetIfCpuCodeIpLinkLocalProtSet(cpssDevNum,
                                                        sIpLLCpuCode[ipLLIdx].ipVer, sIpLLCpuCode[ipLLIdx].ipLLProto,
                                                        sIpLLCpuCode[ipLLIdx].cpuCode);
            if (GT_OK != rc)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssDxChNetIfCpuCodeIpLinkLocalProtSet failed for ipLLProto %d, error  %d\n",
                      sIpLLCpuCode[ipLLIdx].ipLLProto, rc);
                return rc;
            }
        }
    }

    return GT_OK;
}

/*******************************************************************************
NAME
   cpssHalCtrlPktInit

DESCRIPTION
   Set default ControlPkt rules

*******************************************************************************/
GT_STATUS cpssHalCtrlPktInit
(
    uint32_t devId
)
{
    GT_STATUS       rc = GT_OK;
    XP_STATUS       status = XP_NO_ERR;
    CPSS_PACKET_CMD_ENT cmd = CPSS_PACKET_CMD_FORWARD_E;

    /* ARP Init Default Pkt Cmd set to Forward*/
    {
        cmd = CPSS_PACKET_CMD_FORWARD_E;
        rc = cpssHalCtrlPktArpInit(devId, CPSS_DXCH_ARP_BCAST_CMD_MODE_PORT_E, cmd);
        if (GT_OK != rc)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssHalCtrlPktArpInit  failed %d\n", rc);
            return rc;
        }

        /*
         * By default set the Vlan ARP BC pkt action to MIRROR_TO_CPU.
         * Pkts will not be trapped unless cpssDxChBrgVlanIpCntlToCpuSet is set.
        */
        cmd = CPSS_PACKET_CMD_FORWARD_E;
        rc = cpssHalCtrlPktArpInit(devId, CPSS_DXCH_ARP_BCAST_CMD_MODE_VLAN_E, cmd);
        if (GT_OK != rc)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssHalCtrlPktArpInit  failed %d\n", rc);
            return rc;
        }
    }
    /* Reserved Multicast (01:08:C2:00:00:XX) Init
     * Enables feature to handle IEEE Reserved Multicast (01:08:C2:00:00:XX)
     */
    rc = cpssHalCtrlPktIeeeReservedMcastInit(devId);
    if (GT_OK != rc)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssHalCtrlPktIeeeReservedMcastInit failed %d\n", rc);
        return rc;
    }

    /* IGMP Init ,only mirror mode is set*/
    rc = cpssHalCtrlPktIgmpInit(devId);
    if (GT_OK != rc)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssHalCtrlPktIgmpInit failed %d\n", rc);
        return rc;
    }


    /* IcmpV6/MLD Init default Pkt Cmd set to Forward*/
    rc = cpssHalCtrlPktIcmpV6Init(devId);
    if (GT_OK != rc)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssHalCtrlPktIcmpV6Init failed %d\n", rc);
        return rc;
    }

    /* UDP port based Init default Pkt Cmd set to Mirror to CPU*/
    rc = cpssHalCtrlPktUdpPortInit(devId);
    if (GT_OK != rc)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssHalCtrlPktUdpPortInit failed %d\n", rc);
        return rc;
    }

    /* IP Link Local Multicast 224.0.0.x init
     * Enable and Initialize handling of IP link local ctrlPkt
     */
    rc = cpssHalCtrlPktIpLinkLocalInit(devId);
    if (GT_OK != rc)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssHalCtrlPktIpLinkLocalInit failed %d\n", rc);
        return rc;
    }

    status = cpssHalInitializePclForCtrlPkt(devId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssHalInitializePclForCtrlPkt failed %d\n", status);
        return GT_FAIL;
    }
    /* Debug API to dump ctrlPkt settings
     * xpsShell Copp features dump command can also be used to invoke below API
     */
    //cpssHalCtrlPktDump(devId, CPSS_HAL_CTRLPKT_TYPE_ALL);

    return GT_OK;
}

/*******************************************************************************
  NAME
  cpssHalCtrlPktArpOnPort

  DESCRIPTION
  ARP request/reply handling
 *******************************************************************************/
GT_STATUS cpssHalCtrlPktArpOnPort
(
    uint32_t devId,
    uint32_t portId,
    GT_BOOL enable
)
{
    GT_STATUS       rc;
    GT_PORT_NUM     cpssPortNum;
    GT_U8           cpssDevNum;

    cpssDevNum = xpsGlobalIdToDevId(devId, portId);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portId);

    rc = cpssDxChBrgGenArpTrapEnable(cpssDevNum, cpssPortNum, enable);
    if (GT_OK != rc)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChBrgGenArpTrapEnable FAILED, rc = [%d]", rc);
        return rc;
    }

    return GT_OK;
}

/*******************************************************************************
  NAME
  cpssHalCtrlPktArpOnSwitch

  Implemented using port interation.
  DESCRIPTION
  ARP request/reply handling
 *******************************************************************************/
GT_STATUS cpssHalCtrlPktArpOnSwitch
(
    uint32_t devId,
    CPSS_PACKET_CMD_ENT cmd
)
{
    GT_STATUS       rc;
    GT_PORT_NUM     cpssPortNum;
    GT_U8           cpssDevNum;
    GT_U32          portId;
    GT_U8           maxTotalPorts;
    GT_BOOL         enable = GT_FALSE;

    if (cmd == CPSS_PACKET_CMD_DROP_HARD_E)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChBrgGenArpBcastToCpuCmdSet CMD_DROP_HARD_E is not supported");
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
              "Default packet action forward is set for ctrl pkt ARP\n");
        cmd = CPSS_PACKET_CMD_FORWARD_E;
    }
    if (xpsGlobalSwitchControlGetMaxPorts(devId, &maxTotalPorts) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Can't get max physical port number\n");
        return GT_FAIL;
    }

    XPS_GLOBAL_PORT_ITER(portId, maxTotalPorts)
    {
        cpssDevNum = xpsGlobalIdToDevId(devId, portId);
        cpssPortNum = xpsGlobalPortToPortnum(devId, portId);

        /* Set ARP in port mode and command mode as cmd */
        rc = cpssDxChBrgGenArpBcastToCpuCmdSet(cpssDevNum,
                                               CPSS_DXCH_ARP_BCAST_CMD_MODE_PORT_E, cmd);
        if (GT_OK != rc)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChBrgGenArpBcastToCpuCmdSet with port mode and cmd %d failed  %d\n", cmd,
                  rc);
            return rc;
        }

        /*Enable/Disable Trap based on pktCmd for the port*/
        if (cmd == CPSS_PACKET_CMD_MIRROR_TO_CPU_E ||
            cmd == CPSS_PACKET_CMD_TRAP_TO_CPU_E)
        {
            enable = GT_TRUE;
        }
        rc = cpssDxChBrgGenArpTrapEnable(cpssDevNum, cpssPortNum, enable);
        if (GT_OK != rc)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChBrgGenArpTrapEnable FAILED, rc = [%d]", rc);
            return rc;
        }
    }
    return GT_OK;
}

/*******************************************************************************
 * NAME
 * cpssHalCtrlPktIeeeReservedMcastProtoOnPort
 *
 * DESCRIPTION:
 *  Supports
 *      PROTO_STP
 *      PROTO_LACP
 *      PROTO_LLDP
 *
 *******************************************************************************/
GT_STATUS cpssHalCtrlPktIeeeReservedMcastProtoOnPort
(
    uint32_t devId,
    cpssHalCtrlPktResIeeeMCastProto_e proto,
    uint32_t portId,
    GT_BOOL enable
)
{
    GT_STATUS           rc;
    GT_PORT_NUM         cpssPortNum;
    GT_U8               cpssDevNum;
    GT_U8               profileIndex;
    CPSS_PACKET_CMD_ENT pktCmd = CPSS_PACKET_CMD_FORWARD_E;

    if (enable)
    {
        pktCmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    }

    profileIndex = cpssHalCtrlPktGetIeeeReservedMCastProtoPortProfile(proto);

    cpssDevNum = xpsGlobalIdToDevId(devId, portId);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portId);

    if ((rc = cpssDxChBrgGenPortIeeeReservedMcastProfileIndexSet(cpssDevNum,
                                                                 cpssPortNum, profileIndex)) != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChBrgGenPortIeeeReservedMcastProfileIndexSet port %d failed  %d \n",
              cpssPortNum, rc);
        return rc;
    }

    if ((rc = cpssDxChBrgGenIeeeReservedMcastProtCmdSet(cpssDevNum, profileIndex,
                                                        proto, pktCmd)) != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChBrgGenIeeeReservedMcastProtCmdSet: ret[%d] asic[%d] protocol[%d]", rc,
              cpssDevNum, proto);
        return rc;
    }

    return (GT_OK);
}

/*******************************************************************************
 * NAME
 * cpssHalCtrlPktIeeeReservedMcastProtoOnSwitch
 *
 * DESCRIPTION:
 *  Supports
 *      PROTO_STP
 *      PROTO_LACP
 *      PROTO_LLDP
 *
 *******************************************************************************/
GT_STATUS cpssHalCtrlPktIeeeReservedMcastProtoOnSwitch
(
    uint32_t devId,
    cpssHalCtrlPktResIeeeMCastProto_e proto,
    CPSS_PACKET_CMD_ENT pktCmd
)
{
    GT_STATUS           rc;
    GT_U8               cpssDevNum;
    GT_U8               profileIndex;

    /* Only SOFT drop is supported for IeeeReservedMcast control packets*/
    if (pktCmd == CPSS_PACKET_CMD_DROP_HARD_E)
    {
        pktCmd = CPSS_PACKET_CMD_DROP_SOFT_E;
    }

    profileIndex = CPSS_HAL_CTRLPKT_RES_IEEE_MCAST_PORT_PROFILE_DEF;

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        if ((rc = cpssDxChBrgGenIeeeReservedMcastProtCmdSet(cpssDevNum, profileIndex,
                                                            proto, pktCmd)) != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChBrgGenIeeeReservedMcastProtCmdSet: ret[%d] asic[%d] protocol[%d]", rc,
                  cpssDevNum, proto);
            return rc;
        }
    }
    return GT_OK;
}

/*******************************************************************************
   NAME
   cpssHalCtrlPktIgmpOnPort

   DESCRIPTION
   IPv4 IGMP snooping on port
        IGMP_TYPE_QUERY
        IGMP_TYPE_LEAVE
        IGMP_TYPE_V1_REPORT
        IGMP_TYPE_V2_REPORT
        IGMP_TYPE_V3_REPORT
*******************************************************************************/
GT_STATUS cpssHalCtrlPktIgmpOnPort
(
    uint32_t devId,
    uint32_t portId,
    GT_BOOL enable
)
{
    GT_STATUS           rc;
    GT_PORT_NUM         cpssPortNum;
    GT_U8               cpssDevNum;

    cpssDevNum = xpsGlobalIdToDevId(devId, portId);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portId);

    rc = cpssDxChBrgGenIgmpSnoopEnable(cpssDevNum, cpssPortNum, enable);
    if (GT_OK != rc)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChBrgGenIgmpSnoopEnable FAILED, rc = [%d]", rc);
        return rc;
    }

    return GT_OK;
}

/*******************************************************************************
   NAME
   cpssHalCtrlPktIgmpOnVlan

   DESCRIPTION
   IPv4 IGMP snooping on port supports:
        IGMP_TYPE_QUERY
        IGMP_TYPE_LEAVE
        IGMP_TYPE_V1_REPORT
        IGMP_TYPE_V2_REPORT
        IGMP_TYPE_V3_REPORT
*******************************************************************************/
GT_STATUS cpssHalCtrlPktIgmpOnVlan
(
    uint32_t            devId,
    uint16_t            vlanId,
    GT_BOOL             enable
)
{
    GT_STATUS                   rc;
    GT_U8                       cpssDevNum;

    /** By default All IGMP packets are mirrored to CPU. */

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        rc = cpssDxChBrgVlanIgmpSnoopingEnable(cpssDevNum, vlanId, enable);
        if (GT_OK != rc)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChBrgVlanIgmpSnoopingEnable with vlan %d failed  %d\n", vlanId, rc);
            return rc;
        }
    }

    return GT_OK;
}

/*******************************************************************************
   NAME
   cpssHalCtrlPktIgmpOnSwitch

   DESCRIPTION
   Implemented using port interation.
   IPv4 IGMP snooping globally
        IGMP_TYPE_QUERY
        IGMP_TYPE_LEAVE
        IGMP_TYPE_V1_REPORT
        IGMP_TYPE_V2_REPORT
        IGMP_TYPE_V3_REPORT
*******************************************************************************/
GT_STATUS cpssHalCtrlPktIgmpOnSwitch
(
    uint32_t devId,
    CPSS_PACKET_CMD_ENT cmd
)
{
    GT_STATUS       rc;
    GT_U8           cpssDevNum;
    CPSS_IGMP_SNOOP_MODE_ENT mode;


    if (cmd == CPSS_PACKET_CMD_FORWARD_E ||
        cmd == CPSS_PACKET_CMD_MIRROR_TO_CPU_E)
    {
        mode = CPSS_IGMP_ROUTER_MIRROR_MODE_E;
    }
    else if (cmd == CPSS_PACKET_CMD_TRAP_TO_CPU_E)
    {
        mode = CPSS_IGMP_SNOOP_TRAP_MODE_E;
    }
    else if (cmd == CPSS_PACKET_CMD_DROP_HARD_E)
    {
        mode = CPSS_IGMP_ALL_TRAP_MODE_E;
    }
    else
    {
        return GT_FAIL;
    }
    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        /** All IGMP packets are mirrored to CPU if the vlan level snoop is enabled */
        rc = cpssDxChBrgGenIgmpSnoopModeSet(cpssDevNum, mode);
        if (GT_OK != rc)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChBrgGenIgmpSnoopModeSet with ROUTER_MIRROR_MODE_E failed  %d\n", rc);
            return rc;
        }
    }
    return GT_OK;
}

/*******************************************************************************
  NAME
  cpssHalCtrlPktIcmpV6OnPort

  DESCRIPTION
  Icmp snooping
    TYPE_IPV6_NDP
    TYPE_IPV6_MLD_V1_V2
    TYPE_IPV6_MLD_V1_REPORT
    TYPE_IPV6_MLD_V1_DONE
    TYPE_MLD_V2_REPORT
 *******************************************************************************/
GT_STATUS cpssHalCtrlPktIcmpV6OnPort
(
    uint32_t devId,
    uint32_t portId,
    GT_BOOL enable
)
{
    /*
     * Not supported for Aldrin
     */
    /*
        GT_STATUS       rc;
        GT_PORT_NUM     cpssPortNum;
        GT_U8           cpssDevNum;

        cpssDevNum = xpsGlobalIdToDevId(devId, portId);
        cpssPortNum = xpsGlobalPortToPortnum(devId, portId);

        rc = cpssDxChBrgGenIpV6IcmpTrapEnable(cpssDevNum, cpssPortNum, enable);
        if (GT_OK != rc)
        {
            LOGFN(xpLogModXps,XP_SUBMOD_MAIN,XP_LOG_ERROR,
                    "cpssDxChBrgGenIpV6IcmpTrapEnable FAILED, rc = [%d]", rc);
            return rc;
        }
    */
    return GT_OK;
}

/*******************************************************************************
  NAME
  cpssHalCtrlPktIcmpV6OnVlan

  DESCRIPTION
  Icmp snooping
    TYPE_IPV6_NDP
    TYPE_IPV6_MLD_V1_V2
    TYPE_IPV6_MLD_V1_REPORT
    TYPE_IPV6_MLD_V1_DONE
    TYPE_MLD_V2_REPORT
 *******************************************************************************/
GT_STATUS cpssHalCtrlPktIcmpV6OnVlan
(
    uint32_t devId,
    uint32_t vlanId,
    GT_BOOL enable
)
{
    GT_STATUS       rc;
    GT_U8           cpssDevNum;

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        /* Supported types and pkt command is set during Init*/
        rc = cpssDxChBrgVlanIpV6IcmpToCpuEnable(cpssDevNum, vlanId, enable);
        if (GT_OK != rc)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChBrgVlanIpV6IcmpToCpuEnable with vlan %d failed  %d\n", vlanId, rc);
            return rc;
        }
    }

    return GT_OK;
}

/*******************************************************************************
  NAME
  cpssHalCtrlPktIcmpV6OnSwitch

  Implemented using the vlan iteration.
  DESCRIPTION
  Icmp snooping
    TYPE_IPV6_NDP
    TYPE_IPV6_MLD_V1_V2
    TYPE_IPV6_MLD_V1_REPORT
    TYPE_IPV6_MLD_V1_DONE
    TYPE_MLD_V2_REPORT
Packet Action:
    TRAP - Set global action as TRAP
    COPY_TO_CPU - Set global action as MIRROR_TO_CPU
    FWD - Set global action as FWD.
    DROP - Set global action as TRAP and DROP it in CPU
    Drop is handled by trapping the packets and setting
    the statistical index value to 1 for CPSS_NET_IPV6_ICMP_PACKET_E cpu code.
NOTE: IPV6 MLD pkts are associated to same trap. Hence setting drop, will
impact on MLD pkts. NDP/MLD must have same pkt action.
Revisit MLD COPP implementation, when support is enabled, as 5 out 8 msgIdx are
used by NDP. MLD_v1 types are enabled now.

 *******************************************************************************/
GT_STATUS cpssHalCtrlPktIcmpV6OnSwitch
(
    uint32_t devId,
    CPSS_PACKET_CMD_ENT pktCmd,
    cpssHalCtrlPktIcmpV6MsgType_e msgType
)
{
    GT_STATUS       rc;
    GT_U8           cpssDevNum;
    GT_U8           msgIdx = 0;

    if (pktCmd == CPSS_PACKET_CMD_DROP_HARD_E)
    {
        pktCmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    }

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        for (msgIdx = 0;
             msgIdx < MAX_ICMPV6_IDX; msgIdx++)
        {
            if (sIcmpV6MsgTypeCmdArray[msgIdx].msgType == msgType)
            {
                /* reserving index 0..7 for IcmpV6 msgTypes */
                rc = cpssDxChBrgGenIcmpv6MsgTypeSet(cpssDevNum, msgIdx, msgType, pktCmd);
                if (GT_OK != rc)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "cpssDxChBrgGenIcmpv6MsgTypeSet failed for %d rc %d\n", msgIdx, rc);
                    return rc;
                }
                else if (GT_OK == rc)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
                          "cpssDxChBrgGenIcmpv6MsgTypeSet success for msgType %d pktCmd %d\n", msgType,
                          pktCmd);
                }
            }
        }
    }
    return GT_OK;
}

/*******************************************************************************
   NAME
   cpssHalCtrlPktUdpPortOnPort

   DESCRIPTION
   Some UDP applications (DHCP, ): client send L2 Brdcst packet to server.
   Up to 12 UDP ports are user configurable. Same action for same UDP port must be used in all entries.
   port - Set BC UDP Trap Mirror enable Per Port
*******************************************************************************/
GT_STATUS cpssHalCtrlPktUdpPortOnPort
(
    uint32_t devId,
    uint32_t portId,
    GT_BOOL enable
)
{
    GT_STATUS       rc;
    GT_PORT_NUM     cpssPortNum;
    GT_U8           cpssDevNum;

    cpssDevNum = xpsGlobalIdToDevId(devId, portId);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portId);

    /* required udp port and command mapping is done in init */
    rc = cpssDxChBrgGenPortBcUdpTrapMirrorEnableSet(cpssDevNum, cpssPortNum,
                                                    enable);
    if (GT_OK != rc)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChBrgGenPortBcUdpTrapMirrorEnableSet FAILED, rc = [%d]", rc);
        return rc;
    }

    return GT_OK;
}

/*******************************************************************************
   NAME
   cpssHalCtrlPktUdpPortOnVlan

   DESCRIPTION
   Some UDP applications (DHCP, ): client send L2 Brdcst packet to server.
   Up to 12 UDP ports are user configurable. Same action for same UDB port must be used in all entries.
   vlan - Enable or disable per Vlan the Broadcast UDP packets Mirror/Trap to the
      CPU based on their destination UDP port
*******************************************************************************/
GT_STATUS cpssHalCtrlPktUdpPortOnVlan
(
    uint32_t devId,
    uint32_t vlanId,
    GT_BOOL enable
)
{
    GT_STATUS       rc;
    GT_U8           cpssDevNum;

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        /* Supported types and pkt command is set during Init*/
        rc = cpssDxChBrgVlanUdpBcPktsToCpuEnable(cpssDevNum, vlanId, enable);
        if (GT_OK != rc)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChBrgVlanUdpBcPktsToCpuEnable with vlan %d failed  %d\n", vlanId, rc);
            return rc;
        }
    }

    return GT_OK;
}

/*******************************************************************************
   NAME
   cpssHalCtrlPktUdpPortOnSwitch
   Implemented using port interation.

   DESCRIPTION
   Some UDP applications (DHCP, ): client send L2 Brdcst packet to server.
   Up to 12 UDP ports are user configurable. Same action for same UDP port must be used in all entries.
   port - Set BC UDP Trap Mirror enable Per Port
*******************************************************************************/
GT_STATUS cpssHalCtrlPktUdpPortOnSwitch
(
    uint32_t devId,
    CPSS_PACKET_CMD_ENT pktCmd,
    cpssHalCtrlPktUdpPort_e udpPort
)
{
    GT_STATUS       rc;
    GT_U8           cpssDevNum;
    GT_U8           udpIdx = 0;


    if (pktCmd == CPSS_PACKET_CMD_DROP_HARD_E)
    {
        pktCmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    }
    else if (pktCmd == CPSS_PACKET_CMD_FORWARD_E)
    {
        pktCmd = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
    }


    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        /* udp port to cpuCode and pktCmd mapping */
        for (udpIdx = 0;
             udpIdx <  sizeof(sUdpPortPktCmd)/sizeof(cpssHalCtrlPktUdpPortCpuCodePktCmd_t);
             udpIdx++)
        {
            if (sUdpPortPktCmd[udpIdx].udpPort == udpPort)
            {
                /* HW allows to configure 12 UDP idx*/
                rc = cpssDxChBrgGenUdpBcDestPortCfgSet(cpssDevNum, udpIdx, udpPort,
                                                       sUdpPortPktCmd[udpIdx].cpuCode, pktCmd);
                if (GT_OK != rc)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "cpssDxChBrgGenUdpBcDestPortCfgSet failed for udpPort %d, error  %d\n", udpPort,
                          rc);
                    return rc;
                }
                else if (GT_OK == rc)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
                          "cpssDxChBrgGenUdpBcDestPortCfgSet success for udpPort %d Cmd %d\n", udpPort,
                          pktCmd);
                }
            }
        }

    }
    return GT_OK;
}

/*******************************************************************************
   NAME
   cpssHalCtrlPktIpLinkLocalOnPort

   DESCRIPTION
*******************************************************************************/
GT_STATUS cpssHalCtrlPktIpLinkLocalOnPort
(
    uint32_t devId,
    uint32_t portId,
    GT_BOOL enable
)
{
    GT_STATUS           rc;
    GT_PORT_NUM         cpssPortNum;
    GT_U8               cpssDevNum;
    CPSS_DXCH_BRG_IP_CTRL_TYPE_ENT ipCntrlType = CPSS_DXCH_BRG_IP_CTRL_NONE_E;

    cpssDevNum = xpsGlobalIdToDevId(devId, portId);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portId);

    if (enable)
    {
        ipCntrlType = CPSS_DXCH_BRG_IP_CTRL_IPV4_IPV6_E;
    }

    rc = cpssDxChBrgGenPortIpControlTrapEnableSet(cpssDevNum, cpssPortNum,
                                                  ipCntrlType);
    if (GT_OK != rc)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChBrgGenPortIpControlTrapEnableSet with port %d failed  %d\n",
              cpssPortNum, rc);
        return rc;
    }

    return GT_OK;
}


/*******************************************************************************
   NAME
   cpssHalCtrlPktIpLinkLocalOnVlan

   DESCRIPTION
*******************************************************************************/
GT_STATUS cpssHalCtrlPktIpLinkLocalOnVlan
(
    uint32_t devId,
    uint32_t vlanId,
    GT_BOOL enable
)
{
    GT_STATUS       rc;
    GT_U8           cpssDevNum;
    CPSS_DXCH_BRG_IP_CTRL_TYPE_ENT ipCntrlType = CPSS_DXCH_BRG_IP_CTRL_NONE_E;

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        if (enable)
        {
            ipCntrlType = CPSS_DXCH_BRG_IP_CTRL_IPV4_IPV6_E;
        }

        rc = cpssDxChBrgVlanIpCntlToCpuSet(cpssDevNum, vlanId, ipCntrlType);
        if (GT_OK != rc)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChBrgVlanIpCntlToCpuSet with vlan %d failed  %d\n", vlanId, rc);
            return rc;
        }
    }

    return GT_OK;
}

/*******************************************************************************
   NAME
   cpssHalCtrlPktIpLinkLocalOnSwitch

   DESCRIPTION
   Implemented using port interation.
*******************************************************************************/
GT_STATUS cpssHalCtrlPktIpLinkLocalOnSwitch
(
    uint32_t devId,
    CPSS_PACKET_CMD_ENT pktCmd,
    cpssHalCtrlPktIpLinkLocalProto_e proto
)
{
    GT_STATUS       rc;
    GT_U8           cpssDevNum;
    GT_BOOL         enable = GT_TRUE;

    /* FWD is supported by settting MIRROR_TO_CPU and dropping at CPU code table.
       Since there is no TRAP ONLY behavior for these proto, DROP cannot be supported.*/

    if ((pktCmd == CPSS_PACKET_CMD_TRAP_TO_CPU_E) ||
        (pktCmd == CPSS_PACKET_CMD_MIRROR_TO_CPU_E) ||
        (pktCmd == CPSS_PACKET_CMD_FORWARD_E))
    {
        enable = GT_TRUE;
    }
    else if (pktCmd == CPSS_PACKET_CMD_DROP_HARD_E)
    {
        enable = GT_FALSE;
        if (pktCmd == CPSS_PACKET_CMD_DROP_HARD_E)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEFAULT,
                  "pktCmd Drop is not Supported default pktCmd Forward is set \n");
        }
    }

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        GT_U8  ipLLIdx = 0;

        for (ipLLIdx = 0;
             ipLLIdx < sizeof(sIpLLCpuCode)/sizeof(cpssHalCtrlPktIpLinkLocalProtoCpuCode_t);
             ipLLIdx++)
        {
            if (sIpLLCpuCode[ipLLIdx].ipLLProto == proto)
            {
                rc = cpssDxChBrgGenIpLinkLocalProtMirrorToCpuEnable(cpssDevNum, \
                                                                    sIpLLCpuCode[ipLLIdx].ipVer, proto, enable);
                if (GT_OK != rc)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "cpssDxChBrgGenIpLinkLocalProtMirrorToCpuEnable failed  %d \n", rc);
                    return rc;
                }
                else
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
                          "cpssDxChBrgGenIpLinkLocalProtMirrorToCpuEnable success \
                            for proto %d enable %d ipVer %d\n", proto, enable,
                          sIpLLCpuCode[ipLLIdx].ipVer);
                }
            }
        }

    }

    return GT_OK;
}
GT_STATUS cpssHalCtrlPktArpTcamOnSwitch
(
    uint32_t devId,
    uint32_t cpuCode,
    CPSS_PACKET_CMD_ENT pktCmd
)
{
    XP_STATUS            ret = XP_NO_ERR;
    uint8_t              i = 0;
    xpsAclkeyFieldList_t aclFieldData;
    xpsAclkeyField_t     aclFieldList[CPSS_HAL_CTRL_PKT_ARP_MAX_KEY_FLDS];
    uint8_t             iacl_value[CPSS_HAL_CTRL_PKT_ARP_MAX_KEY_FLDS];
    uint8_t             iacl_mask[CPSS_HAL_CTRL_PKT_ARP_MAX_KEY_FLDS];
    xpsPclAction_t       aclEntryData;
    GT_U32               maxKeyFlds = CPSS_HAL_CTRL_PKT_ARP_MAX_KEY_FLDS;
    GT_U32               tableId = ctrlPacketIpv4AclTableId;
    xpsAclKeyFormat      keyFormat = XPS_PCL_ARP_KEY;

    memset(&aclFieldData, 0x00, sizeof(aclFieldData));
    memset(&aclEntryData, 0x00, sizeof(aclEntryData));
    memset(&aclFieldList, 0x00, sizeof(aclFieldList));
    memset(iacl_value, 0x00, sizeof(iacl_value));
    memset(iacl_mask, 0x00, sizeof(iacl_mask));

    aclEntryData.pktCmd = pktCmd;
    aclEntryData.mirror.cpuCode = cpuCode;
    aclEntryData.actionStop = GT_TRUE;

    for (i =0; i < maxKeyFlds; i++)
    {
        aclFieldList[i].value = &iacl_value[i];
        aclFieldList[i].mask  = &iacl_mask[i];
    }

    aclFieldData.fldList = aclFieldList;
    aclFieldData.numFlds = CPSS_HAL_CTRL_PKT_ARP_MAX_KEY_FLDS;
    aclFieldData.isValid = 1;

    uint8_t value = 1;
    uint8_t mask = 0x1;
    aclFieldData.fldList[0].keyFlds = XPS_PCL_IS_ARP;
    memcpy(aclFieldData.fldList[0].value, &value, sizeof(uint8_t));
    memcpy(aclFieldData.fldList[0].mask, &mask, sizeof(uint8_t));

    ret =  xpsAclWriteEntry(devId, tableId, CPSS_HAL_CTRLPKT_ACL_PRIO_ARP_MATCH,
                            CPSS_HAL_CTRLPKT_ACL_PRIO_ARP_MATCH, &aclFieldData, aclEntryData,
                            keyFormat, true);
    if (ret != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: xpsAclWriteEntry failed\n");
        return GT_FAIL;
    }
    //Read Entry
#if 0 // UNIT_TEST
    xpsPclAction_t action_ptr;
    xpsVirtualTcamRuleData_t ruleData;
    xpsPclRuleFormat_t         mask1;
    xpsPclRuleFormat_t         pattern1;

    memset(&action_ptr, 0x00, sizeof(xpsPclAction_t));
    memset(&mask1, 0x0, sizeof(xpsPclRuleFormat_t));
    memset(&pattern1, 0x0, sizeof(xpsPclRuleFormat_t));
    ruleData.valid               = (GT_BOOL)true;
    ruleData.rule.pcl.patternPtr = &pattern1;
    ruleData.rule.pcl.maskPtr    = &mask1;
    ruleData.rule.pcl.actionPtr  = &action_ptr;

    ret =  xpsAclReadEntry(devId, 0, 10, &ruleData);
#endif // UNIT_TEST
    return GT_OK;
}
XP_STATUS cpssHalCtrlPktInbandMgmt
(
    uint32_t devId, uint32_t port
)
{
    XP_STATUS ret = XP_NO_ERR;

    xpAclTableInfo_t tableInfo;
    uint32_t         tableId = 0;
    memset(&tableInfo, 0x00, sizeof(xpAclTableInfo_t));

    tableInfo.vTcamInfo.clientGroup = 0;
    tableInfo.vTcamInfo.hitNumber = 0;
    tableInfo.vTcamInfo.ruleSize = XPS_VIRTUAL_TCAM_RULE_SIZE_STD_E;
    tableInfo.vTcamInfo.autoResize = true;
    tableInfo.vTcamInfo.guaranteedNumOfRules = 3;
    tableInfo.vTcamInfo.ruleAdditionMethod =
        XPS_VIRTUAL_TCAM_RULE_ADDITION_METHOD_PRIORITY_E;
    tableInfo.vTcamInfo.ruleFormat = XPS_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E;

    ret = xpsAclCreateTable(devId, tableInfo, &tableId);
    if (ret != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: Failed to create table for the device %u \n", devId);
        return ret;
    }

    ret = xpsAclSetPortAcl(devId, port, tableId, 0);
    if (ret != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: Failed to set ingress port acl for the device %u \n", devId);
        return ret;
    }

    uint32_t             maxFields = 1;
    xpsAclkeyFieldList_t aclFieldData;
    xpsAclkeyField_t     aclFieldList[maxFields];
    uint8_t              iacl_value[maxFields];
    uint8_t              iacl_mask[maxFields];
    xpsPclAction_t       aclEntryData;

    memset(&aclFieldData, 0x00, sizeof(aclFieldData));
    memset(&aclEntryData, 0x00, sizeof(aclEntryData));
    memset(&aclFieldList, 0x00, sizeof(aclFieldList));
    memset(iacl_value, 0x00, sizeof(iacl_value));
    memset(iacl_mask, 0x00, sizeof(iacl_mask));

    aclEntryData.pktCmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    aclEntryData.mirror.cpuCode = CPSS_HAL_CTRLPKT_CPU_CODE_INBAND_MGMT;
    aclEntryData.actionStop = GT_TRUE;

    aclFieldList[0].value = iacl_value;
    aclFieldList[0].mask  = iacl_mask;

    aclFieldData.fldList = aclFieldList;
    aclFieldData.isValid = 1;
    aclFieldData.numFlds = maxFields;

    cpssOsPrintf("Trap IPv4 & IPv6 pkts on mgmt port:%d\n", port);

    uint32_t pclId = tableId;
    uint32_t pclIdMask  = 0x3FF;
    aclFieldData.fldList[0].keyFlds = XPS_PCL_PCLID;
    memcpy(aclFieldData.fldList[0].value, &pclId, sizeof(uint32_t));
    memcpy(aclFieldData.fldList[0].mask, &pclIdMask, sizeof(uint32_t));

    ret =  xpsAclWriteEntry(devId, tableId,
                            CPSS_HAL_CTRLPKT_ACL_PRIO_INBAND_MGMT_IPV4_MATCH,
                            CPSS_HAL_CTRLPKT_ACL_PRIO_INBAND_MGMT_IPV4_MATCH,
                            &aclFieldData, aclEntryData, XPS_PCL_IPV4_L4_KEY, false);
    if (ret != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: Inband Mgmt - IPv4 xpsAclWriteEntry failed\n");
        return ret;
    }

    /* Create separate table for V6. */

    tableInfo.vTcamInfo.ruleSize = XPS_VIRTUAL_TCAM_RULE_SIZE_EXT_E;
    tableInfo.vTcamInfo.ruleFormat = XPS_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E;
    tableId = 0;

    ret = xpsAclCreateTable(devId, tableInfo, &tableId);
    if (ret != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: Failed to create table for the device %u \n", devId);
        return ret;
    }

    /* Note PCL-ID of v4 table-id is set as key for V6 rules.
     * This is because, we can set one PCL-id for look-up on
     * the port. Hence, same PCL-id used for V4 and V6.
     */

    ret =  xpsAclWriteEntry(devId, tableId,
                            CPSS_HAL_CTRLPKT_ACL_PRIO_INBAND_MGMT_IPV6_MATCH,
                            CPSS_HAL_CTRLPKT_ACL_PRIO_INBAND_MGMT_IPV6_MATCH,
                            &aclFieldData, aclEntryData, XPS_PCL_IPV6_L4_KEY, false);
    if (ret != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: Inband Mgmt - IPv6 xpsAclWriteEntry failed\n");
        return ret;
    }

    return ret;
}

GT_STATUS cpssHalCtrlPktDhcpV4OnSwitch
(
    uint32_t devId,
    CPSS_PACKET_CMD_ENT pktCmd
)
{
    GT_STATUS       rc;
    GT_U8           cpssDevNum;

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        rc = cpssHalNetIfAppSpecificCpuCodeTcpUdpPortRangeSet(cpssDevNum,
                                                              CPSS_HAL_L4_PORT_DHCPV4_IDX,
                                                              CPSS_HAL_CTRLPKT_UDP_PORT_DHCP_SERVER,
                                                              CPSS_HAL_CTRLPKT_UDP_PORT_DHCP_CLIENT,
                                                              CPSS_NET_PROT_UDP_E,
                                                              CPSS_NET_UDP_BC_MIRROR_TRAP0_E);

        if (GT_OK != rc)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "TcpUdpPortRangeSet failed for V4, error  %d\n", rc);
            return rc;
        }
    }
    return GT_OK;
}


GT_STATUS cpssHalCtrlPktDhcpV6OnSwitch
(
    uint32_t devId,
    CPSS_PACKET_CMD_ENT pktCmd
)
{
    GT_STATUS       rc;
    GT_U8           cpssDevNum;

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        rc = cpssHalNetIfAppSpecificCpuCodeTcpUdpPortRangeSet(cpssDevNum,
                                                              CPSS_HAL_L4_PORT_DHCPV6_IDX,
                                                              CPSS_HAL_CTRLPKT_UDP_PORT_DHCPv6_CLIENT,
                                                              CPSS_HAL_CTRLPKT_UDP_PORT_DHCPv6_SERVER,
                                                              CPSS_NET_PROT_UDP_E,
                                                              CPSS_HAL_CTRLPKT_CPU_CODE_DHCPv6);

        if (GT_OK != rc)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "TcpUdpPortRangeSet failed for V4, error  %d\n", rc);
            return rc;
        }
    }
    return GT_OK;
}


GT_STATUS cpssHalCtrlPktDhcpV6L2OnSwitch
(
    uint32_t devId,
    CPSS_PACKET_CMD_ENT pktCmd
)
{
    XP_STATUS ret = XP_NO_ERR;
    uint16_t  value = 0;
    uint16_t  mask = 0;
    uint8_t   i = 0;
    GT_U32    maxKeyFlds = (CPSS_HAL_CTRL_PKT_DHCPV6_SERVER_KEY_FLDS >
                            CPSS_HAL_CTRL_PKT_DHCPV6_CLIENT_KEY_FLDS) ?
                           CPSS_HAL_CTRL_PKT_DHCPV6_SERVER_KEY_FLDS :
                           CPSS_HAL_CTRL_PKT_DHCPV6_CLIENT_KEY_FLDS ;
    GT_U32    tableId = ctrlPacketIpv6AclTableId;

    xpsAclkeyFieldList_t aclFieldData;
    xpsAclkeyField_t     aclFieldList[maxKeyFlds];
    uint8_t              iacl_value[maxKeyFlds];
    uint8_t              iacl_mask[maxKeyFlds];
    xpsPclAction_t       aclEntryData;

    memset(&aclFieldData, 0x00, sizeof(aclFieldData));
    memset(&aclEntryData, 0x00, sizeof(aclEntryData));
    memset(&aclFieldList, 0x00, sizeof(aclFieldList));
    memset(iacl_value, 0x00, sizeof(iacl_value));
    memset(iacl_mask, 0x00, sizeof(iacl_mask));

    aclEntryData.pktCmd = pktCmd;
    aclEntryData.mirror.cpuCode = CPSS_HAL_CTRLPKT_CPU_CODE_DHCPv6;

    /*
     * Program the ACL entry for server bound packets.
     * The DIP is expected to be link-local multicast.
     */
    aclEntryData.actionStop = GT_TRUE;

    for (i =0; i < maxKeyFlds; i++)
    {
        aclFieldList[i].value = &iacl_value[i];
        aclFieldList[i].mask  = &iacl_mask[i];
    }

    aclFieldData.fldList = aclFieldList;
    aclFieldData.numFlds = CPSS_HAL_CTRL_PKT_DHCPV6_SERVER_KEY_FLDS;
    aclFieldData.isValid = 1;

    value = CPSS_HAL_CTRL_PKT_DHCPV6_SERVER_RSVD_UDP_PORT;    //Reserved UDP server port for DHCPv6
    mask  = 0xFFFF;
    uint16_t valuebyte0 = ((value & 0xFF00) >> 8);
    uint16_t maskbyte0  = ((mask & 0xFF00) >> 8);
    aclFieldData.fldList[0].keyFlds = XPS_PCL_L4_BYTE2;
    memcpy(aclFieldData.fldList[0].value, &valuebyte0, sizeof(uint8_t));
    memcpy(aclFieldData.fldList[0].mask, &maskbyte0, sizeof(uint8_t));

    uint8_t valuebyte1 = (value & 0x00FF);
    uint8_t maskbyte1  = (mask & 0x00FF);
    aclFieldData.fldList[1].keyFlds = XPS_PCL_L4_BYTE3;
    memcpy(aclFieldData.fldList[1].value, &valuebyte1, sizeof(uint8_t));
    memcpy(aclFieldData.fldList[1].mask, &maskbyte1, sizeof(uint8_t));

    uint8_t isL4Value = 1;
    uint8_t isL4Mask = 0x1;
    aclFieldData.fldList[2].keyFlds = XPS_PCL_IS_L4_VALID;
    memcpy(aclFieldData.fldList[2].value, &isL4Value, sizeof(uint8_t));
    memcpy(aclFieldData.fldList[2].mask, &isL4Mask, sizeof(uint8_t));

    value = 1;
    mask = 0x1;
    aclFieldData.fldList[3].keyFlds = XPS_PCL_IS_IPV6_UDP;
    memcpy(aclFieldData.fldList[3].value, &value, sizeof(uint8_t));
    memcpy(aclFieldData.fldList[3].mask, &mask, sizeof(uint8_t));

    /* Link local Multi cast address reserved for DHCPv6 */
    ipv6Addr_t v6_dip;
    ipv6Addr_t v6_mask;
    memset(v6_dip, 0, sizeof(v6_dip));
    v6_dip[0]  = 0xFF;
    v6_dip[1]  = 0x02;
    v6_dip[13] = 0x1;
    v6_dip[15] = 0x2;

    memset(v6_mask, 0xFF, sizeof(v6_mask));
    aclFieldData.fldList[4].keyFlds = XPS_PCL_IPV6_DIP;
    aclFieldData.fldList[4].value = v6_dip;
    aclFieldData.fldList[4].mask = v6_mask;

    ret =  xpsAclWriteEntry(devId, tableId,
                            CPSS_HAL_CTRLPKT_ACL_PRIO_DHCPv6_UDP1_PORT_MATCH,
                            CPSS_HAL_CTRLPKT_ACL_PRIO_DHCPv6_UDP1_PORT_MATCH, &aclFieldData, aclEntryData,
                            XPS_PCL_IPV6_L4_KEY, true);
    if (ret != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: xpsAclWriteEntry failed\n");
        return GT_FAIL;
    }

    /*
     * Program the ACL entry for client bound packets.
     * The qualifiers are just the UDP dest port.
     * If the incoming interface has L3 configured then the routing engine's
     * lookup result would potentially override the ACL Entry's cpuCode if
     * DHCPV6 trap (L3) is configured.
     */
    memset(&aclFieldData, 0x00, sizeof(aclFieldData));
    memset(&aclEntryData, 0x00, sizeof(aclEntryData));
    memset(&aclFieldList, 0x00, sizeof(aclFieldList));
    memset(iacl_value, 0x00, sizeof(iacl_value));
    memset(iacl_mask, 0x00, sizeof(iacl_mask));

    aclEntryData.pktCmd = pktCmd;
    aclEntryData.mirror.cpuCode = CPSS_HAL_CTRLPKT_CPU_CODE_DHCPv6;
    aclEntryData.actionStop = GT_TRUE;

    for (i =0; i < maxKeyFlds; i++)
    {
        aclFieldList[i].value = &iacl_value[i];
        aclFieldList[i].mask  = &iacl_mask[i];
    }

    aclFieldData.fldList = aclFieldList;
    aclFieldData.numFlds = CPSS_HAL_CTRL_PKT_DHCPV6_CLIENT_KEY_FLDS;
    aclFieldData.isValid = 1;

    value = CPSS_HAL_CTRL_PKT_DHCPV6_CLIENT_RSVD_UDP_PORT;    //Reserved UDP client port for DHCPv6
    mask  = 0xFFFF;
    valuebyte0 = ((value & 0xFF00) >> 8);
    maskbyte0  = ((mask & 0xFF00) >> 8);
    aclFieldData.fldList[0].keyFlds = XPS_PCL_L4_BYTE2;
    memcpy(aclFieldData.fldList[0].value, &valuebyte0, sizeof(uint8_t));
    memcpy(aclFieldData.fldList[0].mask, &maskbyte0, sizeof(uint8_t));

    valuebyte1 = (value & 0x00FF);
    maskbyte1  = (mask & 0x00FF);
    aclFieldData.fldList[1].keyFlds = XPS_PCL_L4_BYTE3;
    memcpy(aclFieldData.fldList[1].value, &valuebyte1, sizeof(uint8_t));
    memcpy(aclFieldData.fldList[1].mask, &maskbyte1, sizeof(uint8_t));

    isL4Value = 1;
    isL4Mask = 0x1;
    aclFieldData.fldList[2].keyFlds = XPS_PCL_IS_L4_VALID;
    memcpy(aclFieldData.fldList[2].value, &isL4Value, sizeof(uint8_t));
    memcpy(aclFieldData.fldList[2].mask, &isL4Mask, sizeof(uint8_t));

    value = 1;
    mask = 0x1;
    aclFieldData.fldList[3].keyFlds = XPS_PCL_IS_IPV6_UDP;
    memcpy(aclFieldData.fldList[3].value, &value, sizeof(uint8_t));
    memcpy(aclFieldData.fldList[3].mask, &mask, sizeof(uint8_t));

    ret =  xpsAclWriteEntry(devId, tableId,
                            CPSS_HAL_CTRLPKT_ACL_PRIO_DHCPv6_UDP2_PORT_MATCH,
                            CPSS_HAL_CTRLPKT_ACL_PRIO_DHCPv6_UDP2_PORT_MATCH, &aclFieldData, aclEntryData,
                            XPS_PCL_IPV6_L4_KEY, true);
    if (ret != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: xpsAclWriteEntry failed\n");
        return GT_FAIL;
    }

    //Read Entry
#if 0 // UNIT_TEST
    xpsPclAction_t action_ptr;
    xpsVirtualTcamRuleData_t ruleData;
    xpsPclRuleFormat_t         mask1;
    xpsPclRuleFormat_t         pattern1;

    memset(&action_ptr, 0x00, sizeof(xpsPclAction_t));
    memset(&mask1, 0x0, sizeof(xpsPclRuleFormat_t));
    memset(&pattern1, 0x0, sizeof(xpsPclRuleFormat_t));
    ruleData.valid               = (GT_BOOL)true;
    ruleData.rule.pcl.patternPtr = &pattern1;
    ruleData.rule.pcl.maskPtr    = &mask1;
    ruleData.rule.pcl.actionPtr  = &action_ptr;

    ret =  xpsAclReadEntry(devId, 0, 10, &ruleData);
#endif // UNIT_TEST
    return GT_OK;
}

GT_STATUS cpssHalCtrlPktSshOnSwitch
(
    uint32_t devId,
    CPSS_PACKET_CMD_ENT pktCmd
)
{
    XP_STATUS ret = XP_NO_ERR;
    uint16_t  value = 0;
    uint16_t  mask  = 0;
    uint8_t   i     = 0;
    GT_U32    maxKeyFlds = CPSS_HAL_CTRL_PKT_BGP_MAX_KEY_FLDS+1;
    GT_U32    tableId = ctrlPacketIpv4AclTableId;

    xpsAclkeyFieldList_t aclFieldData;
    xpsAclkeyField_t     aclFieldList[maxKeyFlds];
    uint8_t              iacl_value[maxKeyFlds];
    uint8_t              iacl_mask[maxKeyFlds];
    xpsPclAction_t       aclEntryData;

    memset(&aclFieldData, 0x00, sizeof(aclFieldData));
    memset(&aclEntryData, 0x00, sizeof(aclEntryData));
    memset(&aclFieldList, 0x00, sizeof(aclFieldList));
    memset(iacl_value, 0x00, sizeof(iacl_value));
    memset(iacl_mask, 0x00, sizeof(iacl_mask));

    aclEntryData.pktCmd = pktCmd;
    aclEntryData.mirror.cpuCode = CPSS_HAL_CTRLPKT_CPU_CODE_SSH;
    aclEntryData.actionStop = GT_TRUE;


    for (i =0; i < maxKeyFlds; i++)
    {
        aclFieldList[i].value = &iacl_value[i];
        aclFieldList[i].mask  = &iacl_mask[i];
    }

    aclFieldData.fldList = aclFieldList;
    aclFieldData.numFlds = maxKeyFlds;
    aclFieldData.isValid = 1;

    value = CPSS_HAL_CTRL_PKT_SSH_RSVD_TCP_PORT;    //Reserved TCP port for SSH 22
    mask  = 0xFFFF;
    uint16_t valuebyte0 = 0;
    uint16_t maskbyte0 = 0;
    valuebyte0 = ((value & 0xFF00) >> 8);
    maskbyte0  = ((mask & 0xFF00) >> 8);
    aclFieldData.fldList[0].keyFlds = XPS_PCL_L4_BYTE0;
    memcpy(aclFieldData.fldList[0].value, &valuebyte0, sizeof(uint8_t));
    memcpy(aclFieldData.fldList[0].mask, &maskbyte0, sizeof(uint8_t));

    uint8_t valuebyte1 = 0;
    uint8_t maskbyte1 = 0;
    valuebyte1 = (value & 0x00FF);
    maskbyte1  = (mask & 0x00FF);
    aclFieldData.fldList[1].keyFlds = XPS_PCL_L4_BYTE1;
    memcpy(aclFieldData.fldList[1].value, &valuebyte1, sizeof(uint8_t));
    memcpy(aclFieldData.fldList[1].mask, &maskbyte1, sizeof(uint8_t));

    uint8_t isL4Value = 1;
    uint8_t isL4Mask = 0x1;

    aclFieldData.fldList[2].keyFlds = XPS_PCL_IS_L4_VALID;
    memcpy(aclFieldData.fldList[2].value, &isL4Value, sizeof(uint8_t));
    memcpy(aclFieldData.fldList[2].mask, &isL4Mask, sizeof(uint8_t));

    value = 1;
    mask = 0x1;
    aclFieldData.fldList[3].keyFlds = XPS_PCL_IS_IPV4_TCP;
    memcpy(aclFieldData.fldList[3].value, &value, sizeof(uint8_t));
    memcpy(aclFieldData.fldList[3].mask, &mask, sizeof(uint8_t));

    ret =  xpsAclWriteEntry(devId, tableId,
                            CPSS_HAL_CTRLPKT_ACL_PRIO_SSH_TCP_SRC_PORT_MATCH,
                            CPSS_HAL_CTRLPKT_ACL_PRIO_SSH_TCP_SRC_PORT_MATCH, &aclFieldData, aclEntryData,
                            XPS_PCL_IPV4_L4_KEY, true);
    if (ret != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: xpsAclWriteEntry failed\n");
        return GT_FAIL;
    }

    memset(&aclFieldData, 0x00, sizeof(aclFieldData));
    memset(&aclEntryData, 0x00, sizeof(aclEntryData));
    memset(&aclFieldList, 0x00, sizeof(aclFieldList));
    memset(iacl_value, 0x00, sizeof(iacl_value));
    memset(iacl_mask, 0x00, sizeof(iacl_mask));

    for (i =0; i < maxKeyFlds; i++)
    {
        aclFieldList[i].value = &iacl_value[i];
        aclFieldList[i].mask  = &iacl_mask[i];
    }

    aclEntryData.pktCmd = pktCmd;
    aclEntryData.mirror.cpuCode = CPSS_HAL_CTRLPKT_CPU_CODE_SSH;
    aclEntryData.actionStop = GT_TRUE;

    aclFieldData.fldList = aclFieldList;
    aclFieldData.numFlds = maxKeyFlds;
    aclFieldData.isValid = 1;

    //ACL entry to match TCP dest port 22
    value = CPSS_HAL_CTRL_PKT_SSH_RSVD_TCP_PORT;    //Reserved TCP port for SSH 22
    mask  = 0xFFFF;
    valuebyte0 = 0;
    maskbyte0 = 0;
    valuebyte0 = ((value & 0xFF00) >> 8);
    maskbyte0  = ((mask & 0xFF00) >> 8);
    aclFieldData.fldList[0].keyFlds = XPS_PCL_L4_BYTE2;
    memcpy(aclFieldData.fldList[0].value, &valuebyte0, sizeof(uint8_t));
    memcpy(aclFieldData.fldList[0].mask, &maskbyte0, sizeof(uint8_t));

    valuebyte1 = 0;
    maskbyte1 = 0;
    valuebyte1 = (value & 0x00FF);
    maskbyte1  = (mask & 0x00FF);
    aclFieldData.fldList[1].keyFlds = XPS_PCL_L4_BYTE3;
    memcpy(aclFieldData.fldList[1].value, &valuebyte1, sizeof(uint8_t));
    memcpy(aclFieldData.fldList[1].mask, &maskbyte1, sizeof(uint8_t));

    isL4Value = 1;
    isL4Mask = 0x1;
    aclFieldData.fldList[2].keyFlds = XPS_PCL_IS_L4_VALID;
    memcpy(aclFieldData.fldList[2].value, &isL4Value, sizeof(uint8_t));
    memcpy(aclFieldData.fldList[2].mask, &isL4Mask, sizeof(uint8_t));

    value = 1;
    mask = 0x1;
    aclFieldData.fldList[3].keyFlds = XPS_PCL_IS_IPV4_TCP;
    memcpy(aclFieldData.fldList[3].value, &value, sizeof(uint8_t));
    memcpy(aclFieldData.fldList[3].mask, &mask, sizeof(uint8_t));

    ret =  xpsAclWriteEntry(devId, tableId,
                            CPSS_HAL_CTRLPKT_ACL_PRIO_SSH_TCP_DEST_PORT_MATCH,
                            CPSS_HAL_CTRLPKT_ACL_PRIO_SSH_TCP_DEST_PORT_MATCH, &aclFieldData, aclEntryData,
                            XPS_PCL_IPV4_L4_KEY, true);
    if (ret != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: xpsAclWriteEntry failed\n");
        return GT_FAIL;
    }

    //Read Entry
#if 0 // UNIT_TEST
    xpsPclAction_t action_ptr;
    xpsVirtualTcamRuleData_t ruleData;
    xpsPclRuleFormat_t         mask1;
    xpsPclRuleFormat_t         pattern1;

    memset(&action_ptr, 0x00, sizeof(xpsPclAction_t));
    memset(&mask1, 0x0, sizeof(xpsPclRuleFormat_t));
    memset(&pattern1, 0x0, sizeof(xpsPclRuleFormat_t));
    ruleData.valid               = (GT_BOOL)true;
    ruleData.rule.pcl.patternPtr = &pattern1;
    ruleData.rule.pcl.maskPtr    = &mask1;
    ruleData.rule.pcl.actionPtr  = &action_ptr;

    ret =  xpsAclReadEntry(devId, 0, 10, &ruleData);
#endif // UNIT_TEST
    return GT_OK;
}

#if 0

GT_STATUS cpssHalCtrlPktBgpOnSwitch
(
    uint32_t devId,
    cpssHalBgpType bgpType,
    CPSS_PACKET_CMD_ENT pktCmd
)
{
    GT_STATUS       status = GT_OK;
    CPSS_DXCH_NETIF_APP_SPECIFIC_CPUCODE_TCP_UDP_PORT_RANGE_STC l4TypeInfo;

    l4TypeInfo.minL4Port    = CPSS_HAL_CTRL_PKT_BGP_RSVD_TCP_PORT;
    l4TypeInfo.maxL4Port    = CPSS_HAL_CTRL_PKT_BGP_RSVD_TCP_PORT;
    l4TypeInfo.packetType   = CPSS_NET_TCP_UDP_PACKET_BOTH_UC_MC_E;
    l4TypeInfo.protocol     = CPSS_NET_PROT_TCP_E;
    l4TypeInfo.cpuCode      = (CPSS_NET_RX_CPU_CODE_ENT)(
                                  CPSS_HAL_CTRLPKT_CPU_CODE_BGPv4v6);
    l4TypeInfo.l4PortMode   = CPSS_NET_CPU_CODE_L4_BOTH_DEST_SRC_PORT_E;

    cpssDxChNetIfAppSpecificCpuCodeTcpUdpPortRangeSet(devId, 1, &l4TypeInfo);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Failed to set port range");
        return xpsConvertCpssStatusToXPStatus(status);
    }
    return GT_OK;
}

#else

GT_STATUS cpssHalCtrlPktBgpOnSwitch
(
    uint32_t devId,
    cpssHalBgpType bgpType,
    CPSS_PACKET_CMD_ENT pktCmd
)
{
    XP_STATUS ret = XP_NO_ERR;
    uint16_t  value = 0;
    uint16_t  mask  = 0;
    uint8_t   i     = 0;
    GT_U32    maxKeyFlds = CPSS_HAL_CTRL_PKT_BGP_MAX_KEY_FLDS+1;
    GT_U32    tableId = ctrlPacketIpv4AclTableId;

    xpsAclkeyFieldList_t aclFieldData;
    xpsPclAction_t       aclEntryData;
    xpsAclkeyField_t     aclFieldList[maxKeyFlds];
    uint8_t              iacl_value[maxKeyFlds];
    uint8_t              iacl_mask[maxKeyFlds];

    memset(&aclFieldData, 0x00, sizeof(aclFieldData));
    memset(&aclEntryData, 0x00, sizeof(aclEntryData));
    memset(&aclFieldList, 0x00, sizeof(aclFieldList));
    memset(iacl_value, 0x00, sizeof(iacl_value));
    memset(iacl_mask, 0x00, sizeof(iacl_mask));

    aclEntryData.pktCmd = pktCmd;
    aclEntryData.mirror.cpuCode = CPSS_HAL_CTRLPKT_CPU_CODE_BGPv4v6;
    aclEntryData.actionStop = GT_TRUE;

    for (i =0; i < maxKeyFlds; i++)
    {
        aclFieldList[i].value = &iacl_value[i];
        aclFieldList[i].mask  = &iacl_mask[i];
    }

    aclFieldData.fldList = aclFieldList;
    aclFieldData.numFlds = maxKeyFlds;
    aclFieldData.isValid = 1;

    value = CPSS_HAL_CTRL_PKT_BGP_RSVD_TCP_PORT;    //Reserved TCP port for BGPv4 and BGPv6
    mask  = 0xFFFF;
    uint16_t valuebyte0 = 0;
    uint16_t maskbyte0 = 0;
    valuebyte0 = ((value & 0xFF00) >> 8);
    maskbyte0  = ((mask & 0xFF00) >> 8);
    aclFieldData.fldList[0].keyFlds = XPS_PCL_L4_BYTE0;
    memcpy(aclFieldData.fldList[0].value, &valuebyte0, sizeof(uint8_t));
    memcpy(aclFieldData.fldList[0].mask, &maskbyte0, sizeof(uint8_t));

    uint8_t valuebyte1 = 0;
    uint8_t maskbyte1 = 0;
    valuebyte1 = (value & 0x00FF);
    maskbyte1  = (mask & 0x00FF);
    aclFieldData.fldList[1].keyFlds = XPS_PCL_L4_BYTE1;
    memcpy(aclFieldData.fldList[1].value, &valuebyte1, sizeof(uint8_t));
    memcpy(aclFieldData.fldList[1].mask, &maskbyte1, sizeof(uint8_t));

    uint8_t isL4Value = 1;
    uint8_t isL4Mask = 0x1;

    aclFieldData.fldList[2].keyFlds = XPS_PCL_IS_L4_VALID;
    memcpy(aclFieldData.fldList[2].value, &isL4Value, sizeof(uint8_t));
    memcpy(aclFieldData.fldList[2].mask, &isL4Mask, sizeof(uint8_t));


    if (bgpType == CPSS_HAL_BGP_IPV4)
    {
        value = 1;
        mask = 0x1;
        aclFieldData.fldList[3].keyFlds = XPS_PCL_IS_IPV4_TCP;
        memcpy(aclFieldData.fldList[3].value, &value, sizeof(uint8_t));
        memcpy(aclFieldData.fldList[3].mask, &mask, sizeof(uint8_t));
        tableId = ctrlPacketIpv4AclTableId;

        ret =  xpsAclWriteEntry(devId, tableId,
                                CPSS_HAL_CTRLPKT_ACL_PRIO_BGPv4_TCP_SRC_PORT_MATCH,
                                CPSS_HAL_CTRLPKT_ACL_PRIO_BGPv4_TCP_SRC_PORT_MATCH, &aclFieldData, aclEntryData,
                                XPS_PCL_IPV4_L4_KEY, true);
        if (ret != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Error: xpsAclWriteEntry failed\n");
            return GT_FAIL;
        }
    }
    else if (bgpType == CPSS_HAL_BGP_IPV6)
    {
        value = 1;
        mask = 0x1;
        aclFieldData.fldList[3].keyFlds = XPS_PCL_IS_IPV6_TCP;
        memcpy(aclFieldData.fldList[3].value, &value, sizeof(uint8_t));
        memcpy(aclFieldData.fldList[3].mask, &mask, sizeof(uint8_t));

        tableId = ctrlPacketIpv6AclTableId;

        ret =  xpsAclWriteEntry(devId, tableId,
                                CPSS_HAL_CTRLPKT_ACL_PRIO_BGPv6_TCP_SRC_PORT_MATCH,
                                CPSS_HAL_CTRLPKT_ACL_PRIO_BGPv6_TCP_SRC_PORT_MATCH, &aclFieldData, aclEntryData,
                                XPS_PCL_IPV6_L4_KEY, true);
        if (ret != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Error: xpsAclWriteEntry failed\n");
            return GT_FAIL;
        }
    }

    memset(&aclFieldData, 0x00, sizeof(aclFieldData));
    memset(&aclEntryData, 0x00, sizeof(aclEntryData));
    memset(&aclFieldList, 0x00, sizeof(aclFieldList));
    memset(iacl_value, 0x00, sizeof(iacl_value));
    memset(iacl_mask, 0x00, sizeof(iacl_mask));

    for (i =0; i < maxKeyFlds; i++)
    {
        aclFieldList[i].value = &iacl_value[i];
        aclFieldList[i].mask  = &iacl_mask[i];
    }

    aclFieldData.fldList = aclFieldList;
    aclFieldData.numFlds = maxKeyFlds;
    aclFieldData.isValid = 1;

    //ACL entry to match TCP dest port 179
    value = CPSS_HAL_CTRL_PKT_BGP_RSVD_TCP_PORT;    //Reserved TCP port for BGPv4 and BGPv6
    mask  = 0xFFFF;
    valuebyte0 = 0;
    maskbyte0 = 0;
    valuebyte0 = ((value & 0xFF00) >> 8);
    maskbyte0  = ((mask & 0xFF00) >> 8);
    aclFieldData.fldList[0].keyFlds = XPS_PCL_L4_BYTE2;
    memcpy(aclFieldData.fldList[0].value, &valuebyte0, sizeof(uint8_t));
    memcpy(aclFieldData.fldList[0].mask, &maskbyte0, sizeof(uint8_t));

    valuebyte1 = 0;
    maskbyte1 = 0;
    valuebyte1 = (value & 0x00FF);
    maskbyte1  = (mask & 0x00FF);
    aclFieldData.fldList[1].keyFlds = XPS_PCL_L4_BYTE3;
    memcpy(aclFieldData.fldList[1].value, &valuebyte1, sizeof(uint8_t));
    memcpy(aclFieldData.fldList[1].mask, &maskbyte1, sizeof(uint8_t));

    isL4Value = 1;
    isL4Mask = 0x1;
    aclFieldData.fldList[2].keyFlds = XPS_PCL_IS_L4_VALID;
    memcpy(aclFieldData.fldList[2].value, &isL4Value, sizeof(uint8_t));
    memcpy(aclFieldData.fldList[2].mask, &isL4Mask, sizeof(uint8_t));

    aclEntryData.pktCmd = pktCmd;
    aclEntryData.mirror.cpuCode = CPSS_HAL_CTRLPKT_CPU_CODE_BGPv4v6;
    aclEntryData.actionStop = GT_TRUE;

    if (bgpType == CPSS_HAL_BGP_IPV4)
    {
        value = 1;
        mask = 0x1;
        aclFieldData.fldList[3].keyFlds = XPS_PCL_IS_IPV4_TCP;
        memcpy(aclFieldData.fldList[3].value, &value, sizeof(uint8_t));
        memcpy(aclFieldData.fldList[3].mask, &mask, sizeof(uint8_t));

        tableId = ctrlPacketIpv4AclTableId;
        ret =  xpsAclWriteEntry(devId, tableId,
                                CPSS_HAL_CTRLPKT_ACL_PRIO_BGPv4_TCP_DEST_PORT_MATCH,
                                CPSS_HAL_CTRLPKT_ACL_PRIO_BGPv4_TCP_DEST_PORT_MATCH, &aclFieldData,
                                aclEntryData, XPS_PCL_IPV4_L4_KEY, true);
        if (ret != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Error: xpsAclWriteEntry failed\n");
            return GT_FAIL;
        }
    }
    else if (bgpType == CPSS_HAL_BGP_IPV6)
    {
        value = 1;
        mask = 0x1;
        aclFieldData.fldList[3].keyFlds = XPS_PCL_IS_IPV6_TCP;
        memcpy(aclFieldData.fldList[3].value, &value, sizeof(uint8_t));
        memcpy(aclFieldData.fldList[3].mask, &mask, sizeof(uint8_t));
        tableId = ctrlPacketIpv6AclTableId;

        ret =  xpsAclWriteEntry(devId, tableId,
                                CPSS_HAL_CTRLPKT_ACL_PRIO_BGPv6_TCP_DEST_PORT_MATCH,
                                CPSS_HAL_CTRLPKT_ACL_PRIO_BGPv6_TCP_DEST_PORT_MATCH, &aclFieldData,
                                aclEntryData, XPS_PCL_IPV6_L4_KEY, true);
        if (ret != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Error: xpsAclWriteEntry failed\n");
            return GT_FAIL;
        }
    }


    //Read Entry
#if 0 // UNIT_TEST
    xpsPclAction_t action_ptr;
    xpsVirtualTcamRuleData_t ruleData;
    xpsPclRuleFormat_t         mask1;
    xpsPclRuleFormat_t         pattern1;

    memset(&action_ptr, 0x00, sizeof(xpsPclAction_t));
    memset(&mask1, 0x0, sizeof(xpsPclRuleFormat_t));
    memset(&pattern1, 0x0, sizeof(xpsPclRuleFormat_t));
    ruleData.valid               = (GT_BOOL)true;
    ruleData.rule.pcl.patternPtr = &pattern1;
    ruleData.rule.pcl.maskPtr    = &mask1;
    ruleData.rule.pcl.actionPtr  = &action_ptr;

    ret =  xpsAclReadEntry(devId, 0, 10, &ruleData);
#endif // UNIT_TEST
    return GT_OK;
}

#endif

GT_STATUS cpssHalCtrlCommonPacketDrop
(
    uint32_t devId, bool isIpv4Type, bool isSip,
    ipv4Addr_t ipv4AddressPattern, ipv4Addr_t ipv4AddressMask,
    ipv6Addr_t ipv6AddressPattern,
    ipv6Addr_t ipv6AddressMask,
    uint32_t ruleId
)
{
    XP_STATUS ret =XP_NO_ERR;
    bool            is_control_acl = false;
    uint8_t         i     = 0;
    GT_U32 maxKeyFlds;
    GT_U32 tableId;
    xpsAclkeyFieldList_t       aclFieldData;
    xpsPclAction_t             aclEntryData;
    xpsAclKeyFormat              keyFormat;
    maxKeyFlds = 6+1+1;

    xpsAclkeyField_t           aclFieldList[maxKeyFlds];
    uint8_t                    iacl_value[16];
    uint8_t                    iacl_mask[16];

    if (isIpv4Type)
    {
        tableId = ctrlPacketL3Ipv4CommonDropAclTableId;
        keyFormat = XPS_PCL_IPV4_L4_KEY;
    }
    else
    {
        tableId = ctrlPacketL3Ipv6CommonDropAclTableId;
        keyFormat = XPS_PCL_IPV6_L4_KEY;
    }

    memset(&aclFieldData, 0x00, sizeof(aclFieldData));
    memset(&aclEntryData, 0x00, sizeof(aclEntryData));
    memset(&aclFieldList, 0x00, sizeof(aclFieldList));
    memset(iacl_value, 0x00, sizeof(iacl_value));
    memset(iacl_mask, 0x00, sizeof(iacl_mask));
    /* check if needed soft drop */
    aclEntryData.pktCmd = CPSS_PACKET_CMD_DROP_HARD_E;
    aclEntryData.actionStop = GT_FALSE;
    aclEntryData.mirror.cpuCode = XPS_HARD_DROP_REASON_CODE;

    for (i =0; i < maxKeyFlds; i++)
    {
        aclFieldList[i].value = &iacl_value[i];
        aclFieldList[i].mask  = &iacl_mask[i];
    }
    /* needed for place holder to update ports bit map */
    maxKeyFlds--;

    aclFieldData.fldList = aclFieldList;
    aclFieldData.numFlds = maxKeyFlds;
    aclFieldData.isValid = 1;
    uint8_t isL4Value = 1;
    uint8_t isL4Mask = 0x1;
    i = 0;
    aclFieldData.fldList[i].keyFlds = XPS_PCL_IS_L4_VALID;
    memcpy(aclFieldData.fldList[i].value, &isL4Value, sizeof(uint8_t));
    memcpy(aclFieldData.fldList[i].mask, &isL4Mask, sizeof(uint8_t));
    i++;

    if (isIpv4Type)
    {
        if (isSip)
        {
            aclFieldData.fldList[i].keyFlds = XPS_PCL_SIP;
        }
        else
        {
            aclFieldData.fldList[i].keyFlds = XPS_PCL_DIP;
        }

        COPY_IPV4_ADDR_T(aclFieldData.fldList[i].value, ipv4AddressPattern);
        COPY_IPV4_ADDR_T(aclFieldData.fldList[i].mask, ipv4AddressMask);
    }
    else
    {
        if (isSip)
        {
            aclFieldData.fldList[i].keyFlds = XPS_PCL_IPV6_SIP;
        }
        else
        {
            aclFieldData.fldList[i].keyFlds = XPS_PCL_IPV6_DIP;
        }

        COPY_IPV6_ADDR_T(aclFieldData.fldList[i].value, ipv6AddressPattern);
        COPY_IPV6_ADDR_T(aclFieldData.fldList[i].mask, ipv6AddressMask);
    }

    ret =  xpsAclWriteEntry(devId, tableId,
                            0,
                            ruleId, &aclFieldData, aclEntryData,
                            keyFormat, is_control_acl);

    if (ret != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: xpsAclWriteEntry %s failed ruleId %d dropType %d ret %d\n",
              (keyFormat == XPS_PCL_IPV4_L4_KEY) ? "IPV4" : "IPV6", ruleId,
              ret);
        return GT_FAIL;
    }

    return GT_OK;
}

GT_STATUS cpssHalCtrlPktCnpUpdatePolicerOnPort
(
    uint32_t devId,
    uint32_t port,
    GT_U32  cir,
    GT_U32  cbs

)
{
    CPSS_DXCH3_POLICER_METERING_ENTRY_STC policerEntry;
    CPSS_DXCH3_POLICER_METER_TB_PARAMS_UNT  tbParams;
    GT_STATUS rc;

    rc = cpssDxCh3PolicerMeteringEntryGet(devId,
                                          CPSS_DXCH_POLICER_STAGE_INGRESS_1_E, port, &policerEntry);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed cpssDxCh3PolicerMeteringEntryGet, "
              "rc:%d, dev:%d, index:%d\n", rc, devId, port);
        return rc;
    }

    policerEntry.tokenBucketParams.srTcmParams.cir = cir;
    policerEntry.tokenBucketParams.srTcmParams.cbs =
        policerEntry.tokenBucketParams.srTcmParams.ebs = cbs;

    rc = cpssDxCh3PolicerMeteringEntrySet(devId,
                                          CPSS_DXCH_POLICER_STAGE_INGRESS_1_E, port, &policerEntry, &tbParams);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed cpssDxCh3PolicerMeteringEntrySet, "
              "rc:%d, dev:%d, index:%d\n", rc, devId, port);
        return rc;
    }

    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
          "cpssDxCh3PolicerMeteringEntrySet dev:%d, index:%d, cbs %d cir %d\n", devId,
          port, cbs, cir);
    return GT_OK;
}

GT_STATUS cpssHalCtrlPktCnpOnPort
(
    uint32_t devId,
    uint32_t port,
    cpssHalIpType ipType
)
{
    XP_STATUS ret =XP_NO_ERR;
    bool            is_control_acl = true;
    uint16_t        value = 0;
    uint16_t        mask  = 0;
    uint8_t         i     = 0;
    GT_U32 maxKeyFlds;
    GT_U32 tableId;
    xpsAclkeyFieldList_t       aclFieldData;
    xpsPclAction_t             aclEntryData;
    CPSS_DXCH_PCL_ACTION_POLICER_STC cnpPolicer;
    CPSS_DXCH3_POLICER_METERING_ENTRY_STC policerEntry;
    CPSS_DXCH3_POLICER_METER_TB_PARAMS_UNT  tbParams;
    CPSS_DXCH3_POLICER_BILLING_ENTRY_STC   billingCntr;
    GT_STATUS rc;

    maxKeyFlds = 5+1;
    is_control_acl = true;

    xpsAclkeyField_t           aclFieldList[maxKeyFlds];
    uint8_t                    iacl_value[maxKeyFlds];
    uint8_t                    iacl_mask[maxKeyFlds];

    if (ipType == CPSS_HAL_IP_IPV4)
    {
        memset(&policerEntry, 0, sizeof(CPSS_DXCH3_POLICER_METERING_ENTRY_STC));
        policerEntry.modifyUp = CPSS_DXCH_POLICER_MODIFY_UP_DISABLE_E;
        policerEntry.modifyDscp = CPSS_DXCH_POLICER_MODIFY_DSCP_DISABLE_E;
        policerEntry.modifyDp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
        policerEntry.mngCounterSet = CPSS_DXCH3_POLICER_MNG_CNTR_SET0_E;
        policerEntry.byteOrPacketCountingMode =
            CPSS_DXCH3_POLICER_METER_RESOLUTION_BYTES_E;
        policerEntry.meterMode = CPSS_DXCH3_POLICER_METER_MODE_SR_TCM_E;
        policerEntry.tokenBucketParams.srTcmParams.cir =150400; /* 1000 * 150.4Mbit */
        policerEntry.tokenBucketParams.srTcmParams.cbs =
            policerEntry.tokenBucketParams.srTcmParams.ebs = 0x19000 +
                                                             940; /*940 + policer MRU size */
        policerEntry.yellowPcktCmd = CPSS_DXCH3_POLICER_NON_CONFORM_CMD_DROP_E;
        policerEntry.greenPcktCmd = CPSS_DXCH3_POLICER_NON_CONFORM_CMD_NO_CHANGE_E;
        policerEntry.redPcktCmd = CPSS_DXCH3_POLICER_NON_CONFORM_CMD_DROP_E;
        policerEntry.packetSizeMode = CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E;
        policerEntry.meterColorMode = CPSS_POLICER_COLOR_BLIND_E;
        policerEntry.countingEntryIndex = port;
        rc = cpssDxCh3PolicerMeteringEntrySet(devId,
                                              CPSS_DXCH_POLICER_STAGE_INGRESS_1_E, port, &policerEntry, &tbParams);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed cpssDxCh3PolicerMeteringEntrySet, "
                  "rc:%d, dev:%d, index:%d\n", rc, devId, port);
            return rc;
        }

        memset(&billingCntr, 0, sizeof(CPSS_DXCH3_POLICER_BILLING_ENTRY_STC));
        billingCntr.billingCntrMode = CPSS_DXCH3_POLICER_BILLING_CNTR_PACKET_E;
        billingCntr.packetSizeMode = CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E;
        rc = cpssDxCh3PolicerBillingEntrySet(devId, CPSS_DXCH_POLICER_STAGE_INGRESS_1_E,
                                             port, &billingCntr);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed cpssDxCh3PolicerMeteringEntrySet, "
                  "rc:%d, dev:%d, index:%d\n", rc, devId, port);
            return rc;
        }
    }

    memset(&aclFieldData, 0x00, sizeof(aclFieldData));
    memset(&aclEntryData, 0x00, sizeof(aclEntryData));
    memset(&aclFieldList, 0x00, sizeof(aclFieldList));
    memset(iacl_value, 0x00, sizeof(iacl_value));
    memset(iacl_mask, 0x00, sizeof(iacl_mask));
    memset(&cnpPolicer, 0x00, sizeof(cnpPolicer));

    aclEntryData.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
    aclEntryData.actionStop = GT_FALSE;
    aclEntryData.mirror.cpuCode = CPSS_HAL_CTRLPKT_CPU_CODE_CNP;
    cnpPolicer.policerEnable = CPSS_DXCH_PCL_POLICER_ENABLE_METER_AND_COUNTER_E;
    cnpPolicer.policerId = port;
    memcpy(&aclEntryData.policer, &cnpPolicer,
           sizeof(CPSS_DXCH_PCL_ACTION_POLICER_STC));
    for (i =0; i < maxKeyFlds; i++)
    {
        aclFieldList[i].value = &iacl_value[i];
        aclFieldList[i].mask  = &iacl_mask[i];
    }

    aclFieldData.fldList = aclFieldList;
    aclFieldData.numFlds = maxKeyFlds;
    aclFieldData.isValid = 1;

    value = CPSS_HAL_CNP_ROCEV2_UDP_DESTINATION_PORT_CNS;    //UDP destination port 4791 (ROCEv2)
    mask  = 0xFFFF;
    uint16_t valuebyte0 = 0;
    uint16_t maskbyte0 = 0;
    valuebyte0 = ((value & 0xFF00) >> 8);
    maskbyte0  = ((mask & 0xFF00) >> 8);
    i = 0;
    aclFieldData.fldList[i].keyFlds = XPS_PCL_L4_BYTE2;
    memcpy(aclFieldData.fldList[i].value, &valuebyte0, sizeof(uint8_t));
    memcpy(aclFieldData.fldList[i].mask, &maskbyte0, sizeof(uint8_t));

    uint8_t valuebyte1 = 0;
    uint8_t maskbyte1 = 0;
    valuebyte1 = (value & 0x00FF);
    maskbyte1  = (mask & 0x00FF);
    i++;
    aclFieldData.fldList[i].keyFlds = XPS_PCL_L4_BYTE3;
    memcpy(aclFieldData.fldList[i].value, &valuebyte1, sizeof(uint8_t));
    memcpy(aclFieldData.fldList[i].mask, &maskbyte1, sizeof(uint8_t));

    uint8_t isL4Value = 1;
    uint8_t isL4Mask = 0x1;
    i++;
    aclFieldData.fldList[i].keyFlds = XPS_PCL_IS_L4_VALID;
    memcpy(aclFieldData.fldList[i].value, &isL4Value, sizeof(uint8_t));
    memcpy(aclFieldData.fldList[i].mask, &isL4Mask, sizeof(uint8_t));

    value = (uint16_t)port;    //port num
    mask  = 0x1FFF;
    i++;
    aclFieldData.fldList[i].keyFlds = XP_PCL_SRC_PORT;
    memcpy(aclFieldData.fldList[i].value, &value, sizeof(uint16_t));
    memcpy(aclFieldData.fldList[i].mask, &mask, sizeof(uint16_t));

    i++;
    if (ipType == CPSS_HAL_IP_IPV4)
    {
        value = 1;
        mask = 0x1;
        aclFieldData.fldList[i].keyFlds = XPS_PCL_IS_IPV4_UDP;
        memcpy(aclFieldData.fldList[i].value, &value, sizeof(uint8_t));
        memcpy(aclFieldData.fldList[i].mask, &mask, sizeof(uint8_t));

        value = CPSS_HAL_CNP_BTH_HEADER_OPCODE_CNS;
        mask = 0xFF;
        i++;
        aclFieldData.fldList[i].keyFlds = XPS_PCL_ICMP_MSG_TYPE;
        memcpy(aclFieldData.fldList[i].value, &value, sizeof(uint8_t));
        memcpy(aclFieldData.fldList[i].mask, &mask, sizeof(uint8_t));

        tableId = ctrlPacketIpv4AclTableId;
        ret =  xpsAclWriteEntry(devId, tableId,
                                CPSS_HAL_CTRLPKT_ACL_PRIO_CNP_IPv4_MATCH,
                                CPSS_HAL_CTRLPKT_ACL_PRIO_CNP_IPv4_MATCH + port, &aclFieldData, aclEntryData,
                                XPS_PCL_IPV4_L4_KEY, is_control_acl);

        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
              "cpssHalCtrlPktCnpOnPort IPv4 port %d ret %d \n", port, ret);

        if (ret != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Error: xpsAclWriteEntry failed\n");
            return GT_FAIL;
        }
    }
    else if (ipType == CPSS_HAL_IP_IPV6)
    {
        value = 1;
        mask = 0x1;
        aclFieldData.fldList[i].keyFlds = XPS_PCL_IS_IPV6_UDP;
        memcpy(aclFieldData.fldList[i].value, &value, sizeof(uint8_t));
        memcpy(aclFieldData.fldList[i].mask, &mask, sizeof(uint8_t));

        value = CPSS_HAL_CNP_BTH_HEADER_OPCODE_CNS;
        mask = 0xFF;
        i++;
        aclFieldData.fldList[i].keyFlds = XPS_PCL_ICMPV6_MSG_TYPE;
        memcpy(aclFieldData.fldList[i].value, &value, sizeof(uint8_t));
        memcpy(aclFieldData.fldList[i].mask, &mask, sizeof(uint8_t));

        tableId = ctrlPacketIpv6AclTableId;
        ret =  xpsAclWriteEntry(devId, tableId,
                                CPSS_HAL_CTRLPKT_ACL_PRIO_CNP_IPv6_MATCH,
                                CPSS_HAL_CTRLPKT_ACL_PRIO_CNP_IPv6_MATCH + port, &aclFieldData, aclEntryData,
                                XPS_PCL_IPV6_L4_KEY, is_control_acl);

        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
              "cpssHalCtrlPktCnpOnPort IPv6 port %d ret %d \n", port, ret);

        if (ret != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Error: xpsAclWriteEntry failed\n");
            return GT_FAIL;
        }
    }

    return GT_OK;
}

GT_STATUS cpssHalCtrlPktLACPOnSwitch
(
    uint32_t devId,
    CPSS_PACKET_CMD_ENT pktCmd
)
{
    CPSS_NET_RX_CPU_CODE_ENT cpuCode = CPSS_NET_IEEE_RSRVD_MULTICAST_ADDR_2_E;
    macAddr_t macAddr = {0x01, 0x80, 0xC2, 0x00, 0x00, 0x02};
    return cpssHalCtrlPktRsvdMcastMacOnSwitch(devId, pktCmd, cpuCode, macAddr,
                                              CPSS_HAL_CTRLPKT_ACL_PRIO_PACKET_TYPE_UNK_ETH_TYPE_LACP_MATCH, 0);
}
GT_STATUS cpssHalCtrlPktLLDPOnSwitch
(
    uint32_t devId,
    CPSS_PACKET_CMD_ENT pktCmd
)
{
    macAddr_t macAddr = {0x01, 0x80, 0xC2, 0x00, 0x00, 0x0e};
    CPSS_NET_RX_CPU_CODE_ENT cpuCode = CPSS_NET_IEEE_RSRVD_MULTICAST_ADDR_3_E;
    return cpssHalCtrlPktRsvdMcastMacOnSwitch(devId, pktCmd, cpuCode, macAddr,
                                              CPSS_HAL_CTRLPKT_ACL_PRIO_PACKET_TYPE_UNK_ETH_TYPE_LLDP_MATCH, 0);
}

GT_STATUS cpssHalCtrlPktEAPOLOnSwitch
(
    uint32_t devId,
    CPSS_PACKET_CMD_ENT pktCmd
)
{
    macAddr_t macAddr   = {0x01, 0x80, 0xC2, 0x00, 0x00, 0x03};
    uint16_t  etherType = 0x888E;
    CPSS_NET_RX_CPU_CODE_ENT cpuCode = CPSS_HAL_CTRLPKT_CPU_CODE_EAPOL_TRAP;
    return cpssHalCtrlPktRsvdMcastMacOnSwitch(devId, pktCmd, cpuCode, macAddr,
                                              CPSS_HAL_CTRLPKT_ACL_PRIO_PACKET_TYPE_UNK_ETH_TYPE_EAP_MATCH,
                                              etherType);
}

GT_STATUS cpssHalCtrlPktRsvdMcastMacOnSwitch
(
    uint32_t devId,
    CPSS_PACKET_CMD_ENT pktCmd,
    CPSS_NET_RX_CPU_CODE_ENT cpuCode,
    macAddr_t macAddr,
    cpssHalCtrlPktAclPriority_e ruleId,
    uint16_t etherType
)
{
    XP_STATUS            ret = XP_NO_ERR;
    macAddr_t            mask = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    xpsAclkeyFieldList_t aclFieldData;
    xpsAclkeyField_t     aclFieldList[2];
    uint8_t              iacl_value[6];
    uint8_t              iacl_mask[6];
    uint8_t              etherTypeVal[2];
    uint8_t              etherTypeMask[2];
    xpsPclAction_t       aclEntryData;
    uint32_t             tableId = ctrlPacketIpv4AclTableId;

    memset(&aclFieldData, 0x00, sizeof(aclFieldData));
    memset(&aclEntryData, 0x00, sizeof(aclEntryData));
    memset(&aclFieldList, 0x00, sizeof(aclFieldList));
    memset(iacl_value, 0x00, sizeof(iacl_value));
    memset(iacl_mask, 0x00, sizeof(iacl_mask));
    memset(etherTypeVal, 0x00, sizeof(etherTypeVal));
    memset(etherTypeMask, 0x00, sizeof(etherTypeMask));

    aclEntryData.pktCmd = pktCmd;
    aclEntryData.mirror.cpuCode = cpuCode;
    aclEntryData.actionStop = GT_TRUE;

    aclFieldList[0].value = iacl_value;
    aclFieldList[0].mask  = iacl_mask;

    aclFieldData.fldList = aclFieldList;
    aclFieldData.numFlds = (etherType == 0)? 1 : 2;
    aclFieldData.isValid = 1;

    aclFieldData.fldList[0].keyFlds = XPS_PCL_MAC_DA;
    memcpy(aclFieldData.fldList[0].value, macAddr, sizeof(macAddr_t));
    memcpy(aclFieldData.fldList[0].mask, mask, sizeof(macAddr_t));

    if (etherType != 0)
    {
        aclFieldList[1].value = etherTypeVal;
        aclFieldList[1].mask  = etherTypeMask;
        aclFieldData.fldList[1].keyFlds = XPS_PCL_ETHER_TYPE;
        memcpy(aclFieldData.fldList[1].value, &etherType, sizeof(uint16_t));
        memcpy(aclFieldData.fldList[1].mask, mask, sizeof(uint16_t));
    }

    ret =  xpsAclWriteEntry(devId, tableId, ruleId, ruleId,
                            &aclFieldData, aclEntryData, XPS_PCL_NON_IP_KEY, true);
    if (ret != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: xpsAclWriteEntry failed for ruleId: %d \n", ruleId);
        return GT_FAIL;
    }

    return GT_OK;
}

GT_STATUS cpssHalCtrlPktCiscoL2ProtOnSwitch
(
    uint32_t devId,
    CPSS_PACKET_CMD_ENT pktCmd
)
{
    GT_STATUS       rc;
    GT_U8           cpssDevNum;

    if (pktCmd == CPSS_PACKET_CMD_DROP_HARD_E)
    {
        pktCmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    }
    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        /* NOTE: This configuration  will trap to the CPU all packets, with
         *       MAC DA = 0x01-00-0C-xx-xx-xx (CISCO Layer 2 proprietary protocols) */
        if ((rc = cpssDxChBrgGenCiscoL2ProtCmdSet(cpssDevNum, pktCmd)) != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChBrgGenCiscoL2ProtCmdSet: ret[%d] asic[%d]"
                  , rc, cpssDevNum);
            return rc;
        }
    }
    return GT_OK;
}

GT_STATUS cpssHalCtrlPktIpTtlErrorOnSwitch
(
    uint32_t devId,
    CPSS_PACKET_CMD_ENT pktCmd
)
{
    GT_STATUS       rc;
    GT_U8           cpssDevNum;

    if ((pktCmd == CPSS_PACKET_CMD_FORWARD_E) ||
        (pktCmd == CPSS_PACKET_CMD_MIRROR_TO_CPU_E))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEFAULT,
              "pktCmd mirror to cpu and forward not supported default pktCmd trap to cpu is set\n");
        pktCmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    }

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        rc = cpssDxChIpExceptionCommandSet(cpssDevNum,
                                           CPSS_DXCH_IP_EXCP_UC_TTL_EXCEED_E, CPSS_IP_PROTOCOL_IPV4V6_E, pktCmd);
        if (GT_OK != rc)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChIpExceptionCommandSet IPv4/v6 TTL err failed  %d \n", rc);
            return rc;
        }
    }
    return GT_OK;
}

GT_STATUS cpssHalCtrlPktIpMtuExceedOnSwitch
(
    uint32_t devId,
    CPSS_PACKET_CMD_ENT pktCmd
)
{
    GT_STATUS       rc;
    GT_U8           cpssDevNum;

    if ((pktCmd == CPSS_PACKET_CMD_FORWARD_E) ||
        (pktCmd == CPSS_PACKET_CMD_MIRROR_TO_CPU_E))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEFAULT,
              "pktCmd mirror to cpu and forward not supported default pktCmd trap to cpu is set\n");
        pktCmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    }

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        rc = cpssDxChIpExceptionCommandSet(cpssDevNum,
                                           CPSS_DXCH_IP_EXCP_UC_MTU_EXCEEDED_E, CPSS_IP_PROTOCOL_IPV4V6_E, pktCmd);
        if (GT_OK != rc)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChIpExceptionCommandSet IPv4/6 UC mtu err failed  %d \n", rc);
            return rc;
        }
        rc = cpssDxChIpExceptionCommandSet(cpssDevNum,
                                           CPSS_DXCH_IP_EXCP_MC_MTU_EXCEEDED_E, CPSS_IP_PROTOCOL_IPV4V6_E, pktCmd);
        if (GT_OK != rc)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChIpExceptionCommandSet IPv4/6 UC mtu err failed  %d \n", rc);
            return rc;
        }
    }
    return GT_OK;
}

/************************************* DUMP COMMANDS **************************************/
GT_STATUS cpssHalCtrlPktArpDump
(
    uint32_t devId
)
{
    GT_STATUS       rc;
    CPSS_PACKET_CMD_ENT pktCmd;
    GT_U32          portId;
    GT_U8           maxTotalPorts;
    GT_U32          cpssPortNum;
    GT_U8           cpssDevNum, cpssPortDevNum;
    GT_BOOL         enable;

    if (xpsGlobalSwitchControlGetMaxPorts(devId, &maxTotalPorts) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Can't get max physical port number\n");
        return GT_FAIL;
    }

    printf("\n\nARP BC Control\n");

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        printf("--- cpssDevNum %d ------\n", cpssDevNum);
        rc = cpssDxChBrgGenArpBcastToCpuCmdGet(cpssDevNum,
                                               CPSS_DXCH_ARP_BCAST_CMD_MODE_PORT_E, &pktCmd);
        if (rc != GT_OK)
        {
            printf("cpssDxChBrgGenArpBcastToCpuCmdGet port command failed  %d \n", rc);
            //return rc;
        }
        printf("    ARP command: port %d \n", pktCmd);
        printf("    port list (enabled ports):");
        XPS_GLOBAL_PORT_ITER(portId, maxTotalPorts)
        {
            cpssPortDevNum = xpsGlobalIdToDevId(devId, portId);
            cpssPortNum = xpsGlobalPortToPortnum(devId, portId);
            if (cpssPortDevNum != cpssDevNum)
            {
                continue;
            }

            rc = cpssDxChBrgGenArpTrapEnableGet(cpssDevNum, cpssPortNum, &enable);
            if (rc != GT_OK)
            {
                printf("cpssDxChBrgGenArpTrapEnableGet port %d failed  %d \n", cpssPortNum, rc);
                //return rc;
            }
            if (enable)
            {
                printf("%d ", cpssPortNum);
            }
        }

        rc = cpssDxChBrgGenArpBcastToCpuCmdGet(cpssDevNum,
                                               CPSS_DXCH_ARP_BCAST_CMD_MODE_VLAN_E, &pktCmd);
        if (rc != GT_OK)
        {
            printf("cpssDxChBrgGenArpBcastToCpuCmdGet vlan command failed  %d \n", rc);
            //return rc;
        }
        printf("\n    ARP command: vlan %d\n", pktCmd);
    }
    return GT_OK;
}

GT_STATUS cpssHalCtrlPktIeeeReservedMcastDump
(
    uint32_t devId
)
{
    GT_STATUS       rc;
    CPSS_PACKET_CMD_ENT pktCmd;
    GT_U32          portId;
    GT_U8           maxTotalPorts;
    GT_U32          protocol;
    GT_U32          profile;
    GT_U32          cpssPortNum;
    GT_U8           cpssDevNum, cpssPortDevNum;
    GT_BOOL         enable;
    OUT CPSS_NET_RX_CPU_CODE_ENT cpuCode;

    if (xpsGlobalSwitchControlGetMaxPorts(devId, &maxTotalPorts) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Can't get max physical port number\n");
        return GT_FAIL;
    }

    printf("\n\nIeee Reserved Mcast Control:\n");

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        printf("--- cpssDevNum %d ------\n", cpssDevNum);
        rc = cpssDxChBrgGenIeeeReservedMcastTrapEnableGet(cpssDevNum, &enable);
        if (rc != GT_OK)
        {
            printf("cpssDxChBrgGenIeeeReservedMcastTrapEnableGet dev %d failed  %d \n",
                   cpssDevNum, rc);
            //return rc;
        }

        printf("\nIeeeReservedMcast: enable %d\n", enable);

        printf("IeeeReservedMcast: CPU Code mapping:\n");
        for (profile = 0;
             profile < sizeof(ieeeMcCpuCodeArray)/sizeof(
                 cpssHalCtrlPktResIeeeMCastCpuCodeMap_t); profile++)
        {
            rc = cpssDxChNetIfCpuCodeIeeeReservedMcastProtGet(cpssDevNum,
                                                              ieeeMcCpuCodeArray[profile].ieeeMcProto, &cpuCode);
            if ((rc == GT_OK) && (cpuCode != 0))
            {
                printf("    protocol %d ==> cpuCode %d\n",
                       ieeeMcCpuCodeArray[profile].ieeeMcProto, cpuCode);
            }
        }

        printf("IeeeReservedMcast: Profile mapping:\n");
        for (profile = 0; profile < 3; profile++)
        {
            for (protocol = 0;
                 protocol < sizeof(ieeeMcCpuCodeArray)/sizeof(
                     cpssHalCtrlPktResIeeeMCastCpuCodeMap_t); protocol++)
            {
                rc = cpssDxChBrgGenIeeeReservedMcastProtCmdGet(cpssDevNum, profile,
                                                               ieeeMcCpuCodeArray[protocol].ieeeMcProto, &pktCmd);
                if ((rc == GT_OK) && (pktCmd != 0))
                {
                    printf("    profile %d:  protocol %d ==> cmd %d\n", profile,
                           ieeeMcCpuCodeArray[protocol].ieeeMcProto,  pktCmd);
                }
            }
        }

        printf("IeeeReservedMcast: Profile-> Port mapping:\n");
        printf("    portList [port]:profile:");
        XPS_GLOBAL_PORT_ITER(portId, maxTotalPorts)
        {
            cpssPortDevNum = xpsGlobalIdToDevId(devId, portId);
            cpssPortNum = xpsGlobalPortToPortnum(devId, portId);
            if (cpssPortDevNum != cpssDevNum)
            {
                continue;
            }

            rc = cpssDxChBrgGenPortIeeeReservedMcastProfileIndexGet(cpssDevNum, cpssPortNum,
                                                                    &profile);
            if (rc != GT_OK)
            {
                printf("cpssDxChBrgGenPortIeeeReservedMcastProfileIndexGet port %d failed  %d \n",
                       cpssPortNum, rc);
                //return rc;
            }
            printf("[%d]:%d ", cpssPortNum, profile);
        }
        printf("\n");
    }
    return GT_OK;
}

/*******************************************************************************
   NAME
   cpssHalCtrlPktIgmpDump

   DESCRIPTION
*******************************************************************************/
GT_STATUS cpssHalCtrlPktIgmpDump
(
    uint32_t devId
)
{
    GT_STATUS       rc;
    GT_U32          portId;
    GT_U8           maxTotalPorts;
    GT_U32          cpssPortNum;
    GT_U8           cpssDevNum, cpssPortDevNum;
    GT_U32          cntr;
    GT_BOOL         enable;
    GT_U16          vlan, maxVlan = XPS_VLAN_MAX_IDS;
    CPSS_IGMP_SNOOP_MODE_ENT mode;

    if (xpsGlobalSwitchControlGetMaxPorts(devId, &maxTotalPorts) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Can't get max physical port number\n");
        return GT_FAIL;
    }

    printf("\n\nIGMP Contorl:\n");
    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        printf("--- cpssDevNum %d ------\n", cpssDevNum);
        rc = cpssDxChBrgGenIgmpSnoopModeGet(cpssDevNum, &mode);
        if (rc != GT_OK)
        {
            printf("cpssDxChBrgGenCiscoL2ProtCmdGet  failed  %d \n", rc);
            //return rc;
        }

        printf("\n\nIGMP: mode %d\n", mode);
        printf("    port list (enabled ports):");
        XPS_GLOBAL_PORT_ITER(portId, maxTotalPorts)
        {
            cpssPortDevNum = xpsGlobalIdToDevId(devId, portId);
            cpssPortNum = xpsGlobalPortToPortnum(devId, portId);
            if (cpssPortDevNum != cpssDevNum)
            {
                continue;
            }

            rc = cpssDxChBrgGenIgmpSnoopEnableGet(cpssDevNum, cpssPortNum, &enable);
            if (rc != GT_OK)
            {
                printf("cpssDxChBrgGenIgmpSnoopEnableGet port %d failed  %d \n", cpssPortNum,
                       rc);
                //return rc;
            }
            if (enable)
            {
                printf("%d ", cpssPortNum);
            }
        }
        printf("\n    vlan list (enabled vlans):");
        cntr = 0;
        for (vlan = 0; vlan < maxVlan; vlan++)
        {
            rc = cpssDxChBrgVlanIgmpSnoopingEnableGet(cpssDevNum, vlan, &enable);
            if (rc != GT_OK)
            {
                printf("cpssDxChBrgGenIgmpSnoopEnableGet vlan %d failed  %d \n", vlan,
                       rc);
                //return rc;
            }
            if (enable)
            {
                cntr++;
                printf("%d ", vlan);
            }
            if (cntr > 50)
            {
                printf("....");
                break;
            }
        }
        printf("\n");
    }
    return GT_OK;
}

GT_STATUS cpssHalCtrlPktIcmpV6Dump
(
    uint32_t devId
)
{
    GT_STATUS       rc;
    CPSS_PACKET_CMD_ENT pktCmd;
    GT_U8           maxTotalPorts;
    GT_U32          profile;
    GT_U8           cpssDevNum;
    GT_U32          cntr;
    GT_BOOL         enable;
    GT_U16          vlan, maxVlan= XPS_VLAN_MAX_IDS;
    GT_U8           msgTypeInTable;

    if (xpsGlobalSwitchControlGetMaxPorts(devId, &maxTotalPorts) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Can't get max physical port number\n");
        return GT_FAIL;
    }

    printf("\n\nMLD And IcmpV6: \n");
    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        printf("--- cpssDevNum %d ------\n", cpssDevNum);
        for (profile = 0;
             profile < sizeof(sIcmpV6MsgTypeCmdArray)/sizeof(
                 cpssHalCtrlPktIcmpV6MsgTypeCmd_t); profile++)
        {
            rc = cpssDxChBrgGenIcmpv6MsgTypeGet(cpssDevNum, profile, &msgTypeInTable,
                                                &pktCmd);
            if ((rc == GT_OK) && (pktCmd != 0) && (msgTypeInTable != 0))
            {
                printf("    index %d:  msg type %d ==> cmd %d\n", profile, msgTypeInTable,
                       pktCmd);
            }
        }

        printf("    port list (enabled ports):");
        /*
         * Not supported for Aldrin
         */
        /*
                XPS_GLOBAL_PORT_ITER(portId, maxTotalPorts)
                {
                    cpssPortDevNum = xpsGlobalIdToDevId(devId, portId);
                    cpssPortNum = xpsGlobalPortToPortnum(devId, portId);
                    if(cpssPortDevNum != cpssDevNum) continue;
                    printf("\ndevNum %d, portNum %d", cpssPortDevNum, cpssPortNum);
                    rc = cpssDxChBrgGenIpV6IcmpTrapEnableGet(cpssDevNum, cpssPortNum, &enable);
                    if(rc != GT_OK)
                    {
                        printf("cpssDxChBrgGenIpV6IcmpTrapEnableGet port %d failed  %d \n", cpssPortNum, rc);
                        //return rc;
                    }
                    if (enable)
                    {
                        printf("%d ", cpssPortNum);
                    }
                }
        */
        printf("\n    vlan list (enabled vlans):");
        cntr = 0;
        for (vlan = 1; vlan <= maxVlan; vlan++)
        {
            rc = cpssDxChBrgVlanIpV6IcmpToCpuEnableGet(cpssDevNum, vlan, &enable);
            if (rc != GT_OK)
            {
                printf("cpssDxChBrgVlanIpV6IcmpToCpuEnableGet with vlan %d failed %d \n", vlan,
                       rc);
                //return rc;
            }
            if (enable)
            {
                cntr++;
                printf("%d ", vlan);
            }
            if (cntr > 50)
            {
                printf("....");
                break;
            }
        }
        printf("\n");
    }
    return GT_OK;
}

GT_STATUS cpssHalCtrlPktUdpPortDump
(
    uint32_t devId
)
{
    GT_STATUS       rc;
    CPSS_PACKET_CMD_ENT pktCmd;
    GT_U32          portId;
    GT_U8           maxTotalPorts;
    GT_U32          profile;
    GT_U32          cpssPortNum;
    GT_U8           cpssDevNum, cpssPortDevNum;
    GT_U32          cntr;
    GT_BOOL         enable;
    GT_BOOL         valid;
    GT_U16          vlan, maxVlan= XPS_VLAN_MAX_IDS;
    CPSS_NET_RX_CPU_CODE_ENT cpuCode;
    GT_U16          udpPortNum;

    if (xpsGlobalSwitchControlGetMaxPorts(devId, &maxTotalPorts) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Can't get max physical port number\n");
        return GT_FAIL;
    }

    printf("\n\nUDP BC: \n");
    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        printf("--- cpssDevNum %d ------\n", cpssDevNum);
        for (profile = 0;
             profile < sizeof(sUdpPortPktCmd)/sizeof(cpssHalCtrlPktUdpPortCpuCodePktCmd_t);
             profile++)
        {
            rc = cpssDxChBrgGenUdpBcDestPortCfgGet(cpssDevNum, profile, &valid, &udpPortNum,
                                                   &cpuCode, &pktCmd);
            if ((rc == GT_OK) && (valid == GT_TRUE))
            {
                printf("    index %d:  udp port %d ==> cpu code %d cmd %d\n", profile,
                       udpPortNum, cpuCode,  pktCmd);
            }
        }
        printf("    port list (enabled ports):");
        XPS_GLOBAL_PORT_ITER(portId, maxTotalPorts)
        {
            cpssPortDevNum = xpsGlobalIdToDevId(devId, portId);
            cpssPortNum = xpsGlobalPortToPortnum(devId, portId);
            if (cpssPortDevNum != cpssDevNum)
            {
                continue;
            }

            rc = cpssDxChBrgGenPortBcUdpTrapMirrorEnableGet(cpssDevNum, cpssPortNum,
                                                            &enable);
            if (rc != GT_OK)
            {
                printf("cpssDxChBrgGenPortBcUdpTrapMirrorEnableGet port %d failed  %d \n",
                       cpssPortNum, rc);
                //return rc;
            }
            if (enable)
            {
                printf("%d ", cpssPortNum);
            }
        }
        printf("\n    vlan list (enabled vlans):");
        cntr = 0;
        for (vlan = 1; vlan <= maxVlan; vlan++)
        {
            rc = cpssDxChBrgVlanUdpBcPktsToCpuEnableGet(cpssDevNum, vlan, &enable);
            if (rc != GT_OK)
            {
                printf("cpssDxChBrgVlanUdpBcPktsToCpuEnableGet with vlan %d failed %d \n", vlan,
                       rc);
                //return rc;
            }
            if (enable)
            {
                cntr++;
                printf("%d ", vlan);
            }
            if (cntr > 50)
            {
                printf("....");
                break;
            }
        }
        printf("\n");
    }
    return GT_OK;
}

GT_STATUS cpssHalCtrlPktIpLinkLocalDump
(
    uint32_t devId
)
{
    GT_STATUS       rc;
    GT_U32          portId;
    GT_U8           maxTotalPorts;
    GT_U32          protocol;
    GT_U32          cpssPortNum;
    GT_U8           cpssDevNum, cpssPortDevNum;
    GT_U32          cntr = 0;
    GT_BOOL         enable;
    GT_U16          vlan, maxVlan= XPS_VLAN_MAX_IDS;
    CPSS_NET_RX_CPU_CODE_ENT cpuCode;
    CPSS_DXCH_BRG_IP_CTRL_TYPE_ENT currIpCntrlType;

    if (xpsGlobalSwitchControlGetMaxPorts(devId, &maxTotalPorts) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Can't get max physical port number\n");
        return GT_FAIL;
    }

    printf("\n\nIp Control\n");
    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        printf("--- cpssDevNum %d ------\n", cpssDevNum);

        rc = cpssDxChBrgGenIpLinkLocalMirrorToCpuEnableGet(cpssDevNum,
                                                           CPSS_IP_PROTOCOL_IPV4_E, &enable);
        if (GT_OK != rc)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChBrgGenIpLinkLocalMirrorToCpuEnableGet failed for ipv4 error  %d\n",
                  rc);
            //return rc;
        }
        printf("    ipLinkLocal mirror to cpu v4 ==> enable %d\n", enable);
        rc = cpssDxChBrgGenIpLinkLocalMirrorToCpuEnableGet(cpssDevNum,
                                                           CPSS_IP_PROTOCOL_IPV6_E, &enable);
        if (GT_OK != rc)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChBrgGenIpLinkLocalMirrorToCpuEnableGet failed for ipv6 error  %d\n",
                  rc);
            //return rc;
        }
        printf("    ipLinkLocal mirror to cpu v6 ==> enable %d\n", enable);

        /* Initializing the linklocal 224.0.0.1 protocol to cpuCode mapping */
        for (protocol = 0;
             protocol < sizeof(sIpLLCpuCode)/sizeof(cpssHalCtrlPktIpLinkLocalProtoCpuCode_t);
             protocol++)
        {

            rc = cpssDxChNetIfCpuCodeIpLinkLocalProtGet(cpssDevNum,
                                                        sIpLLCpuCode[protocol].ipVer, sIpLLCpuCode[protocol].ipLLProto, &cpuCode);
            if (GT_OK != rc)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssDxChNetIfCpuCodeIpLinkLocalProtGet failed for ipLLProto %d, error  %d\n",
                      sIpLLCpuCode[protocol].ipLLProto, rc);
                //return rc;
            }

            rc = cpssDxChBrgGenIpLinkLocalProtMirrorToCpuEnableGet(cpssDevNum,
                                                                   sIpLLCpuCode[protocol].ipVer, sIpLLCpuCode[protocol].ipLLProto, &enable);
            if (GT_OK != rc)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssDxChBrgGenIpLinkLocalProtMirrorToCpuEnableGet failed  %d \n", rc);
                //return rc;
            }
            printf("    protocol %d version %d ==> enable %d, cpuCode %d\n",
                   sIpLLCpuCode[protocol].ipLLProto, sIpLLCpuCode[protocol].ipVer, enable,
                   cpuCode);
        }
        XPS_GLOBAL_PORT_ITER(portId, maxTotalPorts)
        {
            cpssPortDevNum = xpsGlobalIdToDevId(devId, portId);
            cpssPortNum = xpsGlobalPortToPortnum(devId, portId);
            if (cpssPortDevNum != cpssDevNum)
            {
                continue;
            }

            rc = cpssDxChBrgGenPortIpControlTrapEnableGet(cpssDevNum, cpssPortNum,
                                                          &currIpCntrlType);
            if (rc != GT_OK)
            {
                printf("cpssDxChBrgGenPortIpControlTrapEnableGet port %d failed  %d \n",
                       cpssPortNum, rc);
                //return rc;
            }
            if (currIpCntrlType != CPSS_DXCH_BRG_IP_CTRL_NONE_E)
            {
                printf("\nport %d: HW %d ", cpssPortNum, currIpCntrlType);
            }
        }
        for (vlan = 1; vlan <= maxVlan; vlan++)
        {
            rc = cpssDxChBrgVlanIpCntlToCpuGet(cpssDevNum, vlan, &currIpCntrlType);
            if (rc != GT_OK)
            {
                printf("cpssDxChBrgVlanIpCntlToCpuGet with vlan %d failed %d \n", vlan, rc);
                //return rc;
            }
            if (currIpCntrlType != CPSS_DXCH_BRG_IP_CTRL_NONE_E)
            {
                cntr++;
                printf("\nvlan %d: HW %d ", vlan, currIpCntrlType);
            }
            if (cntr > 50)
            {
                printf("\n....\n");
                break;
            }
        }
        printf("\n");
    }
    printf("---------\n\n");
    return GT_OK;
}

/*******************************************************************************
   NAME
   cpssHalCtrlPktDump

   DESCRIPTION
*******************************************************************************/
GT_STATUS cpssHalCtrlPktDump
(
    uint32_t devId,
    cpssHalCtrlPktTypes_e type
)
{
    GT_STATUS       rc;

    /* ARP Dump */
    if (CPSS_HAL_CTRLPKT_TYPE_ARP == type || CPSS_HAL_CTRLPKT_TYPE_ALL == type)
    {
        rc = cpssHalCtrlPktArpDump(devId);
        if (GT_OK != rc)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssHalCtrlPktArpDump  failed %d\n", rc);
            return rc;
        }
    }

    /* Reserved Multicast (01:08:C2:00:00:XX) Dump */
    if (CPSS_HAL_CTRLPKT_TYPE_IEEE_RESERVED_MCAST == type ||
        CPSS_HAL_CTRLPKT_TYPE_ALL == type)
    {
        rc = cpssHalCtrlPktIeeeReservedMcastDump(devId);
        if (GT_OK != rc)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssHalCtrlPktIeeeReservedMcastDump failed %d\n", rc);
            return rc;
        }
    }
    /* IGMP Dump */
    if (CPSS_HAL_CTRLPKT_TYPE_IGMP == type || CPSS_HAL_CTRLPKT_TYPE_ALL == type)
    {
        rc = cpssHalCtrlPktIgmpDump(devId);
        if (GT_OK != rc)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssHalCtrlPktIgmpDump failed %d\n", rc);
            return rc;
        }
    }

    /* IcmpV6/MLD Dump */
    if (CPSS_HAL_CTRLPKT_TYPE_ICMPV6 == type || CPSS_HAL_CTRLPKT_TYPE_ALL == type)
    {
        rc = cpssHalCtrlPktIcmpV6Dump(devId);
        if (GT_OK != rc)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssHalCtrlPktIcmpV6Dump failed %d\n", rc);
            return rc;
        }
    }

    /* UDP port based Dump */
    if (CPSS_HAL_CTRLPKT_TYPE_UDP_PORT == type || CPSS_HAL_CTRLPKT_TYPE_ALL == type)
    {
        rc = cpssHalCtrlPktUdpPortDump(devId);
        if (GT_OK != rc)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssHalCtrlPktUdpPortDump failed %d\n", rc);
            return rc;
        }
    }

    /* IP Link Local Multicast 224.0.0.x init */
    if (CPSS_HAL_CTRLPKT_TYPE_IP_LINK_LOCAL == type ||
        CPSS_HAL_CTRLPKT_TYPE_ALL == type)
    {
        rc = cpssHalCtrlPktIpLinkLocalDump(devId);
        if (GT_OK != rc)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssHalCtrlPktIpLinkLocalDump failed %d\n", rc);
            return rc;
        }
    }

    return GT_OK;
}

/* PortBitmap should be updated before calling this function for PFC. */

GT_STATUS cpssHalCtrlPktFlowControl
(
    uint32_t devId,
    uint32_t tableId,
    uint16_t opCode,
    CPSS_PACKET_CMD_ENT pktCmd,
    cpssHalCtrlPktAclPriority_e ruleId,
    bool is_control_acl
)
{
    XP_STATUS ret = XP_NO_ERR;
    macAddr_t macAddr = {0x01, 0x80, 0xc2, 0x00, 0x00, 0x01};
    macAddr_t        macAddrMask  = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    GT_U32 maxKeyFlds = 11;
    xpsAclkeyFieldList_t       aclFieldData;
    uint32_t priority = ruleId;

    if (tableId == ctrlPacketPFCAclTableId)
    {
        priority = 0;
        maxKeyFlds += 1;
    }
    xpsAclkeyField_t           aclFieldList[maxKeyFlds];
    uint8_t                    iacl_value[maxKeyFlds];
    uint8_t                    iacl_mask[maxKeyFlds];
    xpsPclAction_t             aclEntryData;
    int i;
    uint16_t etherType = 0x8808;
    uint16_t mask = 0xFFFF;

    memset(&aclFieldData, 0x00, sizeof(aclFieldData));
    memset(&aclEntryData, 0x00, sizeof(aclEntryData));
    memset(&aclFieldList, 0x00, sizeof(aclFieldList));
    memset(iacl_value, 0x00, sizeof(iacl_value));
    memset(iacl_mask, 0x00, sizeof(iacl_mask));

    aclEntryData.pktCmd = pktCmd;
    aclEntryData.mirror.cpuCode = CPSS_HAL_CTRLPKT_CPU_CODE_ACL_LOG;
    aclEntryData.actionStop = GT_TRUE;
    for (i =0; i < maxKeyFlds; i++)
    {
        aclFieldList[i].value = &iacl_value[i];
        aclFieldList[i].mask  = &iacl_mask[i];
    }

    aclFieldData.fldList = aclFieldList;
    if (tableId == ctrlPacketPFCAclTableId)
    {
        aclFieldData.numFlds = (maxKeyFlds-1);
    }
    else
    {
        aclFieldData.numFlds = maxKeyFlds;
    }

    aclFieldData.isValid = 1;

    uint8_t value = 1;
    uint8_t keyMask = 0x1;
    aclFieldData.fldList[0].keyFlds = XPS_PCL_IS_NON_IP;
    memcpy(aclFieldData.fldList[0].value, &value, sizeof(uint8_t));
    memcpy(aclFieldData.fldList[0].mask, &keyMask, sizeof(uint8_t));

    aclFieldData.fldList[1].keyFlds = XPS_PCL_MAC_DA;
    memcpy(aclFieldData.fldList[1].value, macAddr, sizeof(macAddr_t));
    memcpy(aclFieldData.fldList[1].mask, macAddrMask, sizeof(macAddr_t));

    aclFieldData.fldList[7].keyFlds = XPS_PCL_ETHER_TYPE;
    memcpy(aclFieldData.fldList[7].value, &etherType, sizeof(uint16_t));
    memcpy(aclFieldData.fldList[7].mask, &mask, sizeof(uint16_t));

    aclFieldData.fldList[9].keyFlds = XPS_PCL_FC_OPCODE;
    memcpy(aclFieldData.fldList[9].value, &opCode, sizeof(uint16_t));
    memcpy(aclFieldData.fldList[9].mask, &mask, sizeof(uint16_t));

    ret =  xpsAclWriteEntry(devId, tableId, priority, ruleId, &aclFieldData,
                            aclEntryData, XPS_PCL_NON_IP_KEY, is_control_acl);
    if (ret != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: xpsAclWriteEntry failed for ruleId: %d \n", ruleId);
        return GT_FAIL;
    }

    return GT_OK;
}

GT_STATUS cpssHalNetIfAppSpecificCpuCodeTcpUdpPortRangeSet(uint32_t devId,
                                                           uint32_t index,
                                                           uint16_t minL4Port,
                                                           uint16_t maxL4Port,
                                                           CPSS_NET_PROT_ENT proto,
                                                           uint32_t cpuCode
                                                          )
{
    GT_STATUS       status = GT_OK;
    CPSS_DXCH_NETIF_APP_SPECIFIC_CPUCODE_TCP_UDP_PORT_RANGE_STC l4TypeInfo;
    XP_DEV_TYPE_T   devType;
    cpssHalGetDeviceType(devId, &devType);


    l4TypeInfo.minL4Port    = minL4Port;
    l4TypeInfo.maxL4Port    = maxL4Port;
    l4TypeInfo.packetType   = CPSS_NET_TCP_UDP_PACKET_BOTH_UC_MC_E;
    l4TypeInfo.protocol     = proto;
    l4TypeInfo.cpuCode      = (CPSS_NET_RX_CPU_CODE_ENT)(cpuCode);


    /* L4 Port mode is applicaple only for  AC5 devices*/
    if (IS_DEVICE_AC5X(devType))
    {
        l4TypeInfo.l4PortMode   = CPSS_NET_CPU_CODE_L4_BOTH_DEST_SRC_PORT_E;
    }

    status = cpssDxChNetIfAppSpecificCpuCodeTcpUdpPortRangeSet(devId, index,
                                                               &l4TypeInfo);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeSet Failed %d ", status);
        return (status);
    }
    return GT_OK;
}

GT_STATUS cpssHalNetIfAppSpecificCpuCodeIpProtocolSet(uint32_t devId,
                                                      uint32_t index,
                                                      uint8_t protocol,
                                                      uint32_t cpuCode)
{
    GT_STATUS       status = GT_OK;
    GT_U8           cpssDevNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        status = cpssDxChNetIfAppSpecificCpuCodeIpProtocolSet(cpssDevNum, index,
                                                              protocol,
                                                              (CPSS_NET_RX_CPU_CODE_ENT)(cpuCode));

        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChNetIfAppSpecificCpuCodeIpProtocolSet Failed %d ", status);
            return status;
        }
    }
    return GT_OK;
}





