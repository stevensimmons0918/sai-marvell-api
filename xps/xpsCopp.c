// xpsCopp.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpsCopp.h"
#include "xpsLock.h"
#include "xpsUtil.h"
#include "xpsCommon.h"
#include "xpsPacketDrv.h"
#include "cpssHalCtrlPkt.h"
#include "cpssHalUtil.h"
#include "cpssHalVlan.h"
#include "cpssHalCopp.h"
#include "cpssHalHostIf.h"
#include "cpssHalQos.h"
#include "xpsAllocator.h"
#include "xpsAcl.h"

#define XP_COPP_QUEUE_NUM_INBAND_MGMT       0
#define XP_COPP_RATELIMIT_INDEX_INBAND_MGMT 255 /* using last cpuCode ratelimiter index for inband mgmt */

#ifdef __cplusplus
extern "C"
{
#endif

static xpsDbHandle_t cpuCodeTableEntryDbHndl = XPS_STATE_INVALID_DB_HANDLE;

/* Acl table for Cpu port */
extern uint32_t ctrlPacketForCpuEgressIpv4AclTableId;
extern uint32_t ctrlPacketForCpuEgressIpv6AclTableId;


extern void cpssHalDumpUtlCPUCodesDump(uint32_t devId,
                                       CPSS_DXCH_TABLE_ENT tableType, uint32_t startIndex, uint32_t endIndex);

XP_STATUS xpsCoppInit(void)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

/* compare function for table Entry DB
*/
static int32_t cpuCodeTableEntryCompare(void *key, void *data)
{
    uint32_t value1 = *((uint32_t *)key);
    xpsCpuCodeTableContextDbEntry* value2 = (xpsCpuCodeTableContextDbEntry*)data;
    return ((value1) - (value2-> cpuCode));
}

XP_STATUS xpsCoppInitScope(xpsScope_t scopeId)
{
    GT_STATUS status;
    XP_STATUS ret = XP_NO_ERR;
    xpDevice_t devId = 0;

    XPS_FUNC_ENTRY_LOG();

    cpuCodeTableEntryDbHndl = XPSAI_HOST_INTF_ACL_DB_HNDL;
    ret = xpsStateRegisterDb(scopeId, "Cpu Code table Entry Db", XPS_GLOBAL,
                             &cpuCodeTableEntryCompare, cpuCodeTableEntryDbHndl);
    if (ret != XP_NO_ERR)
    {
        return ret;
    }

    status = cpssHalCtrlPktInit(devId);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssHalCtrlPktInit Failed with status: %d", status);
        return xpsConvertCpssStatusToXPStatus(status);
    }

    XPS_FUNC_EXIT_LOG();
    return XP_NO_ERR;
}

XP_STATUS xpsCoppDeInit(void)
{
    XPS_FUNC_ENTRY_LOG();

    return xpsCoppDeInitScope(XP_SCOPE_DEFAULT);

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsCoppDeInitScope(xpsScope_t scopeId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS ret = XP_NO_ERR;

    ret = xpsStateDeRegisterDb(scopeId, &cpuCodeTableEntryDbHndl);
    if (ret)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " Failed to deregister cpuCodeTableEntryDbHndl with err : %d\n ", ret);
        return ret;
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsCoppAddDevice(xpsDevice_t devId, xpsInitType_t initType)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsCoppRemoveDevice(xpsDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}



XP_STATUS xpsCoppRemoveEgressAclTableEntry(xpDevice_t devId,
                                           xpsCoppCtrlPkt_e reasonCode,
                                           xpsCpuCodeTableContextDbEntry_t *tableEntryDbEntry)
{
    uint32_t i, ruleNum = 0;
    uint32_t tableId, aclRuleId;
    XP_STATUS xpsRetVal = XP_NO_ERR;

    if (tableEntryDbEntry->ruleNum == 0)
    {
        return XP_NO_ERR;
    }

    ruleNum = tableEntryDbEntry->ruleNum;

    /* When the same dsa code is pointed to by different
     * types (IPv4 or IPv6) cpu code, we only need to remove
     * the EACL policer rule for one of the types.*/
    if (tableEntryDbEntry->cpuCode == (CPSS_NET_RX_CPU_CODE_ENT)
        CPSS_HAL_CTRLPKT_CPU_CODE_DHCPv6 ||
        tableEntryDbEntry->cpuCode == (CPSS_NET_RX_CPU_CODE_ENT)
        CPSS_HAL_CTRLPKT_CPU_CODE_BGPv4v6  ||
        tableEntryDbEntry->cpuCode == CPSS_NET_UDP_BC_MIRROR_TRAP0_E ||
        tableEntryDbEntry->cpuCode == CPSS_NET_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_1_E
        ||
        tableEntryDbEntry->cpuCode == CPSS_NET_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_2_E)
    {
        uint32_t dstTableId = ctrlPacketForCpuEgressIpv6AclTableId;
        if ((reasonCode == XPS_COPP_CTRL_PKT_DHCP) ||
            (reasonCode == XPS_COPP_CTRL_PKT_BGP)  ||
            (reasonCode == XPS_COPP_CTRL_PKT_OSPF) ||
            (reasonCode == XPS_COPP_CTRL_PKT_VRRP))
        {
            dstTableId = ctrlPacketForCpuEgressIpv4AclTableId;
        }
        for (i = 0; i < ruleNum; i++)
        {
            tableId = tableEntryDbEntry->ruleInfo[i].tableId;
            aclRuleId = tableEntryDbEntry->ruleInfo[i].ruleId;
            /* Delete a rule of the specified type */
            if (tableId == dstTableId)
            {
                xpsRetVal =  xpsAclDeleteEntry(devId, tableId, aclRuleId, false);
                if (xpsRetVal != XP_NO_ERR)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Error: xpsCoppRemoveEgressAclTableEntry failed for ruleId: %d \n", aclRuleId);
                    return xpsRetVal;
                }

                xpsRetVal = xpsAllocatorReleaseId(XP_SCOPE_DEFAULT, XP_SAI_ALLOC_ACL_ENTRY,
                                                  aclRuleId);
                if (xpsRetVal != XP_NO_ERR)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Unable to release RuleId :%d | error: %d.\n",
                          aclRuleId, xpsRetVal);
                    return xpsRetVal;
                }
                if (i == 0)
                {
                    tableEntryDbEntry->ruleInfo[i].ruleId =  tableEntryDbEntry->ruleInfo[i +
                                                                                         1].ruleId;
                    tableEntryDbEntry->ruleInfo[i].tableId =  tableEntryDbEntry->ruleInfo[i +
                                                                                          1].tableId;

                    tableEntryDbEntry->ruleInfo[i + 1].ruleId =  XPS_ACL_INVALID_TABLE_ID;
                    tableEntryDbEntry->ruleInfo[i + 1].tableId =  XPS_ACL_INVALID_TABLE_ID;
                }
                else
                {
                    tableEntryDbEntry->ruleInfo[i].ruleId =  XPS_ACL_INVALID_TABLE_ID;
                    tableEntryDbEntry->ruleInfo[i].tableId =  XPS_ACL_INVALID_TABLE_ID;
                }
                tableEntryDbEntry->ruleNum --;
                break;
            }
        }
    }
    /* Remove all of the EACL policer rules pointed to the dsa code.*/
    else
    {
        for (i = 0; i < ruleNum; i++)
        {
            tableId = tableEntryDbEntry->ruleInfo[i].tableId;
            aclRuleId = tableEntryDbEntry->ruleInfo[i].ruleId;
            xpsRetVal =  xpsAclDeleteEntry(devId, tableId, aclRuleId, false);
            if (xpsRetVal != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Error: xpsCoppRemoveEgressAclTableEntry failed for ruleId: %d \n", aclRuleId);
                return xpsRetVal;
            }

            xpsRetVal = xpsAllocatorReleaseId(XP_SCOPE_DEFAULT, XP_SAI_ALLOC_ACL_ENTRY,
                                              aclRuleId);
            if (xpsRetVal != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Unable to release RuleId :%d | error: %d.\n",
                      aclRuleId, xpsRetVal);
                return xpsRetVal;
            }
            tableEntryDbEntry->ruleInfo[i].ruleId =  XPS_ACL_INVALID_TABLE_ID;
            tableEntryDbEntry->ruleInfo[i].tableId =  XPS_ACL_INVALID_TABLE_ID;
            tableEntryDbEntry->ruleNum --;
        }
    }
    return XP_NO_ERR;
}

XP_STATUS xpsCoppWriteEgressAclTableEntryWithAlloc(xpDevice_t devId,
                                                   uint32_t tableId,
                                                   xpsCoppRuleInfo_t *ruleInfo, uint32_t keyType, CPSS_PACKET_CMD_ENT pktCmd,
                                                   CPSS_NET_RX_CPU_CODE_ENT cpuCode, uint32_t policerId)
{
    GT_STATUS status;
    XP_STATUS xpsRetVal = XP_NO_ERR;

    if (!ruleInfo)
    {
        return XP_ERR_VALUE_OUT_OF_RANGE;
    }

    if (ruleInfo->ruleId == XPS_ACL_INVALID_TABLE_ID)
    {
        xpsRetVal = xpsAllocatorAllocateId(XP_SCOPE_DEFAULT, XP_SAI_ALLOC_ACL_ENTRY,
                                           &(ruleInfo->ruleId));
        if (xpsRetVal != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Could not allocate ACL ruleId id, error %d\n", xpsRetVal);
            return xpsRetVal;
        }
    }
    status = cpssHalCoppWriteEgressAclTableEntry(devId, tableId, ruleInfo->ruleId,
                                                 keyType, pktCmd, cpuCode, policerId);
    if (status != GT_OK)
    {
        return xpsConvertCpssStatusToXPStatus(status);
    }
    ruleInfo->tableId = tableId;

    return XP_NO_ERR;
}

XP_STATUS xpsCoppWriteEgressAclTableEntry(xpDevice_t devId,
                                          xpsCoppCtrlPkt_e reasonCode,
                                          xpsCpuCodeTableContextDbEntry_t *tableEntryDbEntry)
{
    // GT_STATUS status;
    XP_STATUS xpsRetVal = XP_NO_ERR;
    uint32_t tableId;
    CPSS_NET_RX_CPU_CODE_ENT cpuCode = tableEntryDbEntry->cpuCode;
    uint32_t policerId = tableEntryDbEntry->policerId;
    CPSS_PACKET_CMD_ENT pktCmd = CPSS_PACKET_CMD_FORWARD_E;

    switch (cpuCode)
    {
        /* ARP */
        case CPSS_NET_INTERVENTION_ARP_E:
        case CPSS_NET_ARP_REPLY_TO_ME_E:
            {
                /* Create Non IP - ARP acl table entry */
                tableId = ctrlPacketForCpuEgressIpv4AclTableId;
                xpsRetVal = xpsCoppWriteEgressAclTableEntryWithAlloc(devId, tableId,
                                                                     &(tableEntryDbEntry ->ruleInfo[0]),
                                                                     XPS_PCL_ARP_KEY, pktCmd, cpuCode, policerId);
                if (xpsRetVal != XP_NO_ERR)
                {
                    return xpsRetVal;
                }
                tableEntryDbEntry ->ruleNum = 1;

                break;
            }
        /* IPv4 or IPv6 */
        case CPSS_HAL_CTRLPKT_CPU_CODE_DHCPv6:
        case CPSS_HAL_CTRLPKT_CPU_CODE_BGPv4v6:
        case CPSS_NET_UDP_BC_MIRROR_TRAP0_E:
        case CPSS_NET_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_1_E: //OSPFv4/v6
        case CPSS_NET_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_2_E: //VRRPv4/v6
            {
                bool isIpv4 = false;

                if ((reasonCode == XPS_COPP_CTRL_PKT_DHCP) ||
                    (reasonCode == XPS_COPP_CTRL_PKT_BGP)  ||
                    (reasonCode == XPS_COPP_CTRL_PKT_OSPF) ||
                    (reasonCode == XPS_COPP_CTRL_PKT_VRRP))
                {
                    isIpv4 = true;
                }
                /* Created IPv4 rule or IPv6 rule for the first time */
                if (tableEntryDbEntry ->ruleNum == 0)
                {
                    if (isIpv4)
                    {
                        /* Create IPv4 acl table entry */
                        tableId = ctrlPacketForCpuEgressIpv4AclTableId;
                        xpsRetVal = xpsCoppWriteEgressAclTableEntryWithAlloc(devId, tableId,
                                                                             &(tableEntryDbEntry ->ruleInfo[0]),
                                                                             XPS_PCL_IPV4_L4_KEY, pktCmd, cpuCode, policerId);
                        if (xpsRetVal != XP_NO_ERR)
                        {
                            return xpsRetVal;
                        }
                        tableEntryDbEntry ->ruleNum = 1;
                    }
                    else
                    {
                        /* Create IPv6 acl table entry */
                        tableId = ctrlPacketForCpuEgressIpv6AclTableId;
                        xpsRetVal = xpsCoppWriteEgressAclTableEntryWithAlloc(devId, tableId,
                                                                             &(tableEntryDbEntry ->ruleInfo[0]),
                                                                             XPS_PCL_IPV6_L4_KEY, pktCmd, cpuCode, policerId);
                        if (xpsRetVal != XP_NO_ERR)
                        {
                            return xpsRetVal;
                        }
                        tableEntryDbEntry ->ruleNum = 1;
                    }
                }
                /* Update or Create */
                else if (tableEntryDbEntry ->ruleNum == 1)
                {
                    /* rule[0] is an IPv4 type rule, so update the IPv4 rule to rule[0]
                     * or create an IPv6 rule to rule[1] */
                    if (tableEntryDbEntry ->ruleInfo[0].tableId ==
                        ctrlPacketForCpuEgressIpv4AclTableId)
                    {
                        if (isIpv4)
                        {
                            tableId = ctrlPacketForCpuEgressIpv4AclTableId;
                            xpsRetVal = xpsCoppWriteEgressAclTableEntryWithAlloc(devId, tableId,
                                                                                 &(tableEntryDbEntry ->ruleInfo[0]),
                                                                                 XPS_PCL_IPV4_L4_KEY, pktCmd, cpuCode, policerId);
                            if (xpsRetVal != XP_NO_ERR)
                            {
                                return xpsRetVal;
                            }
                        }
                        else
                        {
                            /* Create IPv6 acl table entry */
                            tableId = ctrlPacketForCpuEgressIpv6AclTableId;
                            xpsRetVal = xpsCoppWriteEgressAclTableEntryWithAlloc(devId, tableId,
                                                                                 &(tableEntryDbEntry ->ruleInfo[1]),
                                                                                 XPS_PCL_IPV6_L4_KEY, pktCmd, cpuCode, policerId);
                            if (xpsRetVal != XP_NO_ERR)
                            {
                                return xpsRetVal;
                            }
                            tableEntryDbEntry ->ruleNum ++;
                        }
                    }
                    /* rule[0] is an IPv6 type rule, so update the IPv6 rule to rule[0]
                     * or create an IPv4 rule to rule[1] */
                    else
                    {
                        if (isIpv4)
                        {
                            tableId = ctrlPacketForCpuEgressIpv4AclTableId;
                            xpsRetVal = xpsCoppWriteEgressAclTableEntryWithAlloc(devId, tableId,
                                                                                 &(tableEntryDbEntry ->ruleInfo[1]),
                                                                                 XPS_PCL_IPV4_L4_KEY, pktCmd, cpuCode, policerId);
                            if (xpsRetVal != XP_NO_ERR)
                            {
                                return xpsRetVal;
                            }
                            tableEntryDbEntry ->ruleNum ++;
                        }
                        else
                        {
                            /* Create IPv6 acl table entry */
                            tableId = ctrlPacketForCpuEgressIpv6AclTableId;
                            xpsRetVal = xpsCoppWriteEgressAclTableEntryWithAlloc(devId, tableId,
                                                                                 &(tableEntryDbEntry ->ruleInfo[0]),
                                                                                 XPS_PCL_IPV6_L4_KEY, pktCmd, cpuCode, policerId);
                            if (xpsRetVal != XP_NO_ERR)
                            {
                                return xpsRetVal;
                            }
                        }
                    }
                }
                /* Update only */
                else if (tableEntryDbEntry ->ruleNum == 2)
                {
                    uint32_t ruleIndex_4, ruleIndex_6;

                    /* Update IPv4 or IPv6 rules to the corresponding index */
                    if (tableEntryDbEntry ->ruleInfo[0].tableId ==
                        ctrlPacketForCpuEgressIpv4AclTableId)
                    {
                        ruleIndex_4 = 0;
                        ruleIndex_6 = 1;
                    }
                    else
                    {
                        ruleIndex_4 = 1;
                        ruleIndex_6 = 0;
                    }
                    if (isIpv4)
                    {
                        tableId = ctrlPacketForCpuEgressIpv4AclTableId;
                        xpsRetVal = xpsCoppWriteEgressAclTableEntryWithAlloc(devId, tableId,
                                                                             &(tableEntryDbEntry ->ruleInfo[ruleIndex_4]),
                                                                             XPS_PCL_IPV4_L4_KEY, pktCmd, cpuCode, policerId);
                        if (xpsRetVal != XP_NO_ERR)
                        {
                            return xpsRetVal;
                        }
                    }
                    else
                    {
                        /* Create IPv6 acl table entry */
                        tableId = ctrlPacketForCpuEgressIpv6AclTableId;
                        xpsRetVal = xpsCoppWriteEgressAclTableEntryWithAlloc(devId, tableId,
                                                                             &(tableEntryDbEntry ->ruleInfo[ruleIndex_6]),
                                                                             XPS_PCL_IPV6_L4_KEY, pktCmd, cpuCode, policerId);
                        if (xpsRetVal != XP_NO_ERR)
                        {
                            return xpsRetVal;
                        }
                    }
                }

                break;
            }
        /* IPv4 */
        case CPSS_NET_INTERVENTION_IGMP_E:
        case CPSS_NET_UDP_BC_MIRROR_TRAP2_E:
        case CPSS_NET_IP_ZERO_TTL_TRAP_E:
        case CPSS_NET_ROUTE_ENTRY_TRAP_E:
            {
                tableId = ctrlPacketForCpuEgressIpv4AclTableId;
                xpsRetVal = xpsCoppWriteEgressAclTableEntryWithAlloc(devId, tableId,
                                                                     &(tableEntryDbEntry ->ruleInfo[0]),
                                                                     XPS_PCL_IPV4_L4_KEY, pktCmd, cpuCode, policerId);
                if (xpsRetVal != XP_NO_ERR)
                {
                    return xpsRetVal;
                }
                tableEntryDbEntry ->ruleNum = 1;

                break;
            }
        /* IPv6 */
        case CPSS_NET_IPV6_ICMP_PACKET_E:
        case CPSS_NET_IPV6_TTL_TRAP_E:
        case CPSS_NET_IPV6_ROUTE_TRAP_E:
            {
                /* Create IPv6 acl table entry */
                tableId = ctrlPacketForCpuEgressIpv6AclTableId;
                xpsRetVal = xpsCoppWriteEgressAclTableEntryWithAlloc(devId, tableId,
                                                                     &(tableEntryDbEntry ->ruleInfo[0]),
                                                                     XPS_PCL_IPV6_L4_KEY, pktCmd, cpuCode, policerId);
                if (xpsRetVal != XP_NO_ERR)
                {
                    return xpsRetVal;
                }
                tableEntryDbEntry ->ruleNum = 1;

                break;
            }
        /* NON IP */
        case CPSS_NET_IEEE_RSRVD_MULTICAST_ADDR_2_E:
        case CPSS_NET_IEEE_RSRVD_MULTICAST_ADDR_3_E:
        case CPSS_NET_IEEE_RSRVD_MULTICAST_ADDR_1_E:
        case CPSS_NET_CISCO_MULTICAST_MAC_RANGE_E:
            {
                /* Create Non IP acl table entry */
                tableId = ctrlPacketForCpuEgressIpv4AclTableId;
                xpsRetVal = xpsCoppWriteEgressAclTableEntryWithAlloc(devId, tableId,
                                                                     &(tableEntryDbEntry ->ruleInfo[0]),
                                                                     XPS_PCL_NON_IP_KEY, pktCmd, cpuCode, policerId);
                if (xpsRetVal != XP_NO_ERR)
                {
                    return xpsRetVal;
                }
                tableEntryDbEntry ->ruleNum = 1;

                break;
            }
        /* IPv4 + IPv6 */
        case CPSS_NET_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_3_E:
        case CPSS_NET_IP_MTU_EXCEED_E:
        case CPSS_NET_ROUTED_PACKET_FORWARD_E:
        case CPSS_HAL_CTRLPKT_CPU_CODE_SSH:
            {
                /* Create IPv4 acl table entry */
                tableId = ctrlPacketForCpuEgressIpv4AclTableId;
                xpsRetVal = xpsCoppWriteEgressAclTableEntryWithAlloc(devId, tableId,
                                                                     &(tableEntryDbEntry ->ruleInfo[0]),
                                                                     XPS_PCL_IPV4_L4_KEY, pktCmd, cpuCode, policerId);
                if (xpsRetVal != XP_NO_ERR)
                {
                    return xpsRetVal;
                }
                tableEntryDbEntry ->ruleNum = 1;

                /* Create IPv6 acl table entry */
                tableId = ctrlPacketForCpuEgressIpv6AclTableId;
                xpsRetVal = xpsCoppWriteEgressAclTableEntryWithAlloc(devId, tableId,
                                                                     &(tableEntryDbEntry ->ruleInfo[1]),
                                                                     XPS_PCL_IPV6_L4_KEY, pktCmd, cpuCode, policerId);
                if (xpsRetVal != XP_NO_ERR)
                {
                    return xpsRetVal;
                }
                tableEntryDbEntry ->ruleNum = 2;

                break;
            }
        /* NON IP + ARP + IPv4 + IPv6 */
        case CPSS_NET_INGRESS_MIRRORED_TO_ANLYZER_E:
        case CPSS_NET_EGRESS_MIRRORED_TO_ANLYZER_E:
        case CPSS_NET_STATIC_ADDR_MOVED_E:
            {
                /* Create Non IP - ARP acl table entry */
                tableId = ctrlPacketForCpuEgressIpv4AclTableId;
                xpsRetVal = xpsCoppWriteEgressAclTableEntryWithAlloc(devId, tableId,
                                                                     &(tableEntryDbEntry ->ruleInfo[0]),
                                                                     XPS_PCL_ARP_KEY, pktCmd, cpuCode, policerId);
                if (xpsRetVal != XP_NO_ERR)
                {
                    return xpsRetVal;
                }
                tableEntryDbEntry ->ruleNum = 1;

                /* Create IPv4 acl table entry */
                tableId = ctrlPacketForCpuEgressIpv4AclTableId;
                xpsRetVal = xpsCoppWriteEgressAclTableEntryWithAlloc(devId, tableId,
                                                                     &(tableEntryDbEntry ->ruleInfo[1]),
                                                                     XPS_PCL_IPV4_L4_KEY, pktCmd, cpuCode, policerId);
                if (xpsRetVal != XP_NO_ERR)
                {
                    return xpsRetVal;
                }
                tableEntryDbEntry ->ruleNum = 2;

                /* Create IPv6 acl table entry */
                tableId = ctrlPacketForCpuEgressIpv6AclTableId;
                xpsRetVal = xpsCoppWriteEgressAclTableEntryWithAlloc(devId, tableId,
                                                                     &(tableEntryDbEntry ->ruleInfo[2]),
                                                                     XPS_PCL_IPV6_L4_KEY, pktCmd, cpuCode, policerId);
                if (xpsRetVal != XP_NO_ERR)
                {
                    return xpsRetVal;
                }
                tableEntryDbEntry ->ruleNum = 3;

                /* Create Non IP acl table entry */
                tableId = ctrlPacketForCpuEgressIpv4AclTableId;
                xpsRetVal = xpsCoppWriteEgressAclTableEntryWithAlloc(devId, tableId,
                                                                     &(tableEntryDbEntry ->ruleInfo[3]),
                                                                     XPS_PCL_NON_IP_KEY, pktCmd, cpuCode, policerId);
                if (xpsRetVal != XP_NO_ERR)
                {
                    return xpsRetVal;
                }
                tableEntryDbEntry ->ruleNum = 4;

                break;
            }
        default:
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpuCode type %d for switch is not supported\n", cpuCode);
            return XP_ERR_NOT_SUPPORTED;
    }
    return XP_NO_ERR;
}


XP_STATUS xpsCoppAddEntry(xpDevice_t devId, uint32_t sourcePort,
                          uint32_t reasonCode, xpCoppEntryData_t entry, xpsHashIndexList_t *indexList)
{
    XPS_FUNC_ENTRY_LOG();

    GT_STATUS status;
    XP_STATUS  xpsRetVal = XP_NO_ERR;
    uint8_t    count     = 0; //Max number of CPU codes
    xpsScope_t scopeId   = 0;

    CPSS_NET_RX_CPU_CODE_ENT cpuCode[MAX_CPU_CODE_COUNT];
    xpsCpuCodeTableContextDbEntry_t   tableEntryDbKey;
    xpsCpuCodeTableContextDbEntry_t   *tableEntryDbEntry = NULL;
    CPSS_DXCH_NET_CPU_CODE_TABLE_ENTRY_STC entryInfo;

    XPS_UNUSED(*indexList);
    XPS_UNUSED(sourcePort);

    if (reasonCode >= XPS_COPP_CTRL_PKT_MAX)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Invalid reason code %d, " "supported number of reason codes %d\n", reasonCode,
              XPS_COPP_CTRL_PKT_MAX);
        return XP_ERR_VALUE_OUT_OF_RANGE;
    }

    /* Convert xps reason code to cpss cpu code */
    (void)xpsUtilConvertReasonCodeToCpssCpuCode((xpsCoppCtrlPkt_e)reasonCode,
                                                cpuCode, &count);

    for (int i =0; i < count; i++)
    {
        /* Read cpu code table entry */
        status = cpssHalCoppReadCpuCodeTableEntry(devId, cpuCode[i], &entryInfo);
        if (status != GT_OK)
        {
            return xpsConvertCpssStatusToXPStatus(status);
        }

        entryInfo.cpuCodeRateLimiterIndex = 0;
        /*Set Statistical rate limit index 1 to drop and 0 to trap all the packets sent to CPU */
        if (entry.pktCmd == XP_PKTCMD_DROP)
        {
            entryInfo.cpuCodeStatRateLimitIndex = XPS_STAT_RATE_INDEX_DROP_ALL;
        }
        else
        {
            entryInfo.cpuCodeStatRateLimitIndex = XPS_STAT_RATE_INDEX_FWD_ALL;
        }

        /* Write cpu code table entry */
        status = cpssHalCoppWriteCpuCodeTableEntry(devId, cpuCode[i], &entryInfo);
        if (status != GT_OK)
        {
            return xpsConvertCpssStatusToXPStatus(status);
        }

        tableEntryDbKey.cpuCode = cpuCode[i];
        if (((xpsRetVal = xpsStateSearchData(scopeId, cpuCodeTableEntryDbHndl,
                                             (xpsDbKey_t)&tableEntryDbKey, (void**)&tableEntryDbEntry)) != XP_NO_ERR))
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "xpsStateSearchData failed with err : %d \n ", xpsRetVal);
            return xpsRetVal;
        }

        if (tableEntryDbEntry == NULL)
        {
            /* Alloc cpu code table entry Node */
            if ((xpsRetVal = xpsStateHeapMalloc(sizeof(xpsCpuCodeTableContextDbEntry_t),
                                                (void**)&tableEntryDbEntry)) == XP_NO_ERR)
            {
                memset(tableEntryDbEntry, 0, sizeof(xpsCpuCodeTableContextDbEntry_t));

                tableEntryDbEntry->cpuCode = cpuCode[i];
                for (int j = 0; j < XPS_COPP_RULE_ID_MAX; j++)
                {
                    tableEntryDbEntry ->ruleInfo[j].ruleId = XPS_ACL_INVALID_TABLE_ID;
                    tableEntryDbEntry ->ruleInfo[j].tableId = XPS_ACL_INVALID_TABLE_ID;
                }
                tableEntryDbEntry->ruleNum = 0;
                if ((xpsRetVal = xpsStateInsertData(scopeId, cpuCodeTableEntryDbHndl,
                                                    (void*)tableEntryDbEntry)) != XP_NO_ERR)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Insert Cpu Code table entry failed, devId(%d)", devId);
                    /* Free Allocated memory */
                    xpsStateHeapFree((void*)tableEntryDbEntry);
                    return xpsRetVal;
                }
            }
        }

        if ((entry.enPolicer) && (entry.pktCmd != XP_PKTCMD_DROP))
        {
            /* When the same dsa code is pointed to by different
             * types (IPv4 or IPv6) cpu code, we need to enter twice
             * to create different types of EACL policer rules. */
            if ((tableEntryDbEntry->policerId != entry.policerId) ||
                tableEntryDbEntry->cpuCode == (CPSS_NET_RX_CPU_CODE_ENT)
                CPSS_HAL_CTRLPKT_CPU_CODE_DHCPv6 ||
                tableEntryDbEntry->cpuCode == (CPSS_NET_RX_CPU_CODE_ENT)
                CPSS_HAL_CTRLPKT_CPU_CODE_BGPv4v6  ||
                tableEntryDbEntry->cpuCode == CPSS_NET_UDP_BC_MIRROR_TRAP0_E ||
                tableEntryDbEntry->cpuCode == CPSS_NET_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_1_E
                ||
                tableEntryDbEntry->cpuCode == CPSS_NET_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_2_E)
            {
                tableEntryDbEntry->pktCmd = (xpsPktCmd_e)entry.pktCmd;
                tableEntryDbEntry->policerId = entry.policerId;

                /* Write Egress acl table entry */
                xpsRetVal = xpsCoppWriteEgressAclTableEntry(devId,
                                                            (xpsCoppCtrlPkt_e)reasonCode, tableEntryDbEntry);
                if (xpsRetVal != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Insert Cpu Code table entry failed, devId(%d)", devId);
                    return xpsRetVal;
                }
            }
        }
        else
        {
            /* Remove Egress acl table entry */
            xpsRetVal = xpsCoppRemoveEgressAclTableEntry(devId,
                                                         (xpsCoppCtrlPkt_e)reasonCode, tableEntryDbEntry);
            if (xpsRetVal != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Insert Cpu Code table entry failed, devId(%d)", devId);
                return xpsRetVal;
            }
            if (tableEntryDbEntry->ruleNum == 0)
            {
                tableEntryDbEntry->policerId = 0;
            }
        }
    }

    XPS_FUNC_EXIT_LOG();
    return XP_NO_ERR;
}

XP_STATUS xpsCoppUpdateEntry(xpDevice_t devId, uint32_t index,
                             xpCoppEntryData_t entry)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsCoppGetIndex(xpDevice_t devId, uint32_t sourcePort,
                          uint32_t reasonCode, int *index)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsCoppGetEntry(xpDevice_t devId, uint32_t sourcePort,
                          uint32_t reasonCode, xpCoppEntryData_t *entry)
{
    XPS_FUNC_ENTRY_LOG();
    CPSS_DXCH_NET_CPU_CODE_TABLE_ENTRY_STC entryInfo;
    uint8_t count = 0;//Max number of CPU codes
    CPSS_NET_RX_CPU_CODE_ENT cpuCode[MAX_CPU_CODE_COUNT];
    GT_STATUS status;
    memset(entry, 0, sizeof(xpCoppEntryData_t));
    memset(&entryInfo, 0, sizeof(CPSS_DXCH_NET_CPU_CODE_TABLE_ENTRY_STC));
    if (reasonCode >= XPS_COPP_CTRL_PKT_MAX)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Invalid reason code %d, " "supported number of reason codes %d\n", reasonCode,
              XPS_COPP_CTRL_PKT_MAX);
        return XP_ERR_VALUE_OUT_OF_RANGE;
    }
    /* Convert xps reason code to cpss cpu code */
    (void)xpsUtilConvertReasonCodeToCpssCpuCode((xpsCoppCtrlPkt_e)reasonCode,
                                                cpuCode, &count);

    for (int i =0; i < count; i++)
    {

        /* Read cpu code table entry */
        status = cpssHalCoppReadCpuCodeTableEntry(devId, cpuCode[i], &entryInfo);
        if (status != GT_OK)
        {
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }

    /* Populate policer id in case of enable */
    entry -> policerId = entryInfo.cpuCodeRateLimiterIndex;
    entry -> reasonCode = reasonCode;


    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsCoppGetEntryByIndex(xpDevice_t devId, uint32_t index,
                                 xpCoppEntryData_t* entry)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsCoppRemoveEntry(xpDevice_t devId, uint32_t sourcePort,
                             uint32_t reasonCode)
{
    XPS_FUNC_ENTRY_LOG();

    CPSS_DXCH_NET_CPU_CODE_TABLE_ENTRY_STC entryInfo;
    uint8_t count = 0;//Max number of CPU codes
    CPSS_NET_RX_CPU_CODE_ENT cpuCode[MAX_CPU_CODE_COUNT];
    GT_STATUS status;

    XPS_UNUSED(sourcePort);

    if (reasonCode >= XPS_COPP_CTRL_PKT_MAX)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Invalid reason code %d, " "supported number of reason codes %d\n", reasonCode,
              XPS_COPP_CTRL_PKT_MAX);
        return XP_ERR_VALUE_OUT_OF_RANGE;
    }
    /* Conver xps reason code to cpss cpu code */
    (void)xpsUtilConvertReasonCodeToCpssCpuCode((xpsCoppCtrlPkt_e)reasonCode,
                                                cpuCode, &count);

    for (int i = 0; i < count; i++)
    {
        /* Read cpu code table entry */
        status = cpssHalCoppReadCpuCodeTableEntry(devId, cpuCode[i], &entryInfo);
        if (status != GT_OK)
        {
            return xpsConvertCpssStatusToXPStatus(status);
        }

        /* Set policer id to 0 to disable policer */
        entryInfo.cpuCodeRateLimiterIndex = 0;
        entryInfo.tc = 0;

        /* Write cpu code table entry */
        status = cpssHalCoppWriteCpuCodeTableEntry(devId, cpuCode[i], &entryInfo);
        if (status != GT_OK)
        {
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }
    XPS_FUNC_EXIT_LOG();
    return XP_NO_ERR;
}

XP_STATUS xpsCoppRemoveEntryByIndex(xpDevice_t devId, uint32_t index)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsCoppSetRehashLevel(xpDevice_t devId, uint8_t numOfRehashLevels)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsCoppGetRehashLevel(xpDevice_t devId, uint8_t* numOfRehashLevels)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


XP_STATUS xpsCoppDisplayTable(xpsDevice_t devId, uint32_t *numOfValidEntries,
                              uint32_t startIndex, uint32_t endIndex, char * logFile, uint32_t detailFormat,
                              uint32_t silentMode, uint32_t tblecpyIndx)
{
    XPS_FUNC_ENTRY_LOG();

    cpssHalDumpUtlCPUCodesDump(devId, CPSS_DXCH_TABLE_CPU_CODE_E, startIndex,
                               endIndex);

    return XP_NO_ERR;
}


XP_STATUS xpsCoppEnableCtrlPktTrapOnPort(xpDevice_t devId,
                                         xpsCoppCtrlPkt_e type, xpsPort_t portId, uint8_t enable)
{
    GT_STATUS status;
    GT_BOOL   gtEnable = GT_TRUE;
    cpssHalCtrlPktResIeeeMCastProto_e proto;
    XPS_FUNC_ENTRY_LOG();

    if (!enable)
    {
        gtEnable = GT_FALSE;
    }

    switch (type)
    {
        case XPS_COPP_CTRL_PKT_ARP_REQUEST:
        case XPS_COPP_CTRL_PKT_ARP_RESPONSE:
            status = cpssHalCtrlPktArpOnPort(devId, portId, gtEnable);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssHalCtrlPktArpOnPort failed with for portId %d enable %d status %d \n",
                      portId, gtEnable, status);
                return xpsConvertCpssStatusToXPStatus(status);
            }
            break;
        case XPS_COPP_CTRL_PKT_STP:
            proto = CPSS_HAL_CTRLPKT_RES_IEEE_MCAST_PROTO_STP;
            status = cpssHalCtrlPktIeeeReservedMcastProtoOnPort(devId, proto, portId,
                                                                gtEnable);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssHalCtrlPktIeeeReservedMcastProtoOnPort failed with for portId %d enable %d status %d \n",
                      portId, gtEnable, status);
                return xpsConvertCpssStatusToXPStatus(status);
            }
            break;
        case XPS_COPP_CTRL_PKT_LACP:
            proto = CPSS_HAL_CTRLPKT_RES_IEEE_MCAST_PROTO_LACP;
            status = cpssHalCtrlPktIeeeReservedMcastProtoOnPort(devId, proto, portId,
                                                                gtEnable);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssHalCtrlPktIeeeReservedMcastProtoOnPort failed with for portId %d enable %d status %d \n",
                      portId, gtEnable, status);
                return xpsConvertCpssStatusToXPStatus(status);
            }
            break;
        case XPS_COPP_CTRL_PKT_LLDP:
            proto = CPSS_HAL_CTRLPKT_RES_IEEE_MCAST_PROTO_LLDP;
            status = cpssHalCtrlPktIeeeReservedMcastProtoOnPort(devId, proto, portId,
                                                                gtEnable);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssHalCtrlPktIeeeReservedMcastProtoOnPort failed with for portId %d enable %d status %d \n",
                      portId, gtEnable, status);
                return xpsConvertCpssStatusToXPStatus(status);
            }
            break;
        case XPS_COPP_CTRL_PKT_IGMP_TYPE_LEAVE:
        case XPS_COPP_CTRL_PKT_IGMP_TYPE_QUERY:
        case XPS_COPP_CTRL_PKT_IGMP_TYPE_V1_REPORT:
        case XPS_COPP_CTRL_PKT_IGMP_TYPE_V2_REPORT:
        case XPS_COPP_CTRL_PKT_IGMP_TYPE_V3_REPORT:
            status = cpssHalCtrlPktIgmpOnPort(devId, portId, gtEnable);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssHalCtrlPktIgmpOnPort failed with for portId %d enable %d status %d \n",
                      portId, gtEnable, status);
                return xpsConvertCpssStatusToXPStatus(status);
            }
            break;
        case XPS_COPP_CTRL_PKT_IPV6_MLD_V1_DONE:
        case XPS_COPP_CTRL_PKT_IPV6_MLD_V1_REPORT:
        case XPS_COPP_CTRL_PKT_IPV6_MLD_V1_V2:
        case XPS_COPP_CTRL_PKT_IPV6_NEIGHBOR_DISCOVERY:
        case XPS_COPP_CTRL_PKT_MLD_V2_REPORT:
            status = cpssHalCtrlPktIcmpV6OnPort(devId, portId, gtEnable);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssHalCtrlPktIcmpV6OnPort failed with for portId %d enable %d status %d \n",
                      portId, gtEnable, status);
                return xpsConvertCpssStatusToXPStatus(status);
            }
            break;
        case XPS_COPP_CTRL_PKT_DHCP:
        case XPS_COPP_CTRL_PKT_DHCPV6:
        case XPS_COPP_CTRL_PKT_SNMP:
            status = cpssHalCtrlPktUdpPortOnPort(devId, portId, gtEnable);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssHalCtrlPktUdpPortOnPort failed with for portId %d enable %d status %d \n",
                      portId, gtEnable, status);
                return xpsConvertCpssStatusToXPStatus(status);
            }
            break;
        case XPS_COPP_CTRL_PKT_OSPF:
        case XPS_COPP_CTRL_PKT_OSPFV6:
        case XPS_COPP_CTRL_PKT_PIM:
            status = cpssHalCtrlPktIpLinkLocalOnPort(devId, portId, gtEnable);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssHalCtrlPktIpLinkLocalOnPort failed with for portId %d enable %d status %d \n",
                      portId, gtEnable, status);
                return xpsConvertCpssStatusToXPStatus(status);
            }
            break;
        case XPS_COPP_CTRL_PKT_BGP:
        case XPS_COPP_CTRL_PKT_BGPV6:
        case XPS_COPP_CTRL_PKT_EAPOL:
        case XPS_COPP_CTRL_PKT_IP2ME:
        case XPS_COPP_CTRL_PKT_L3_MTU_ERROR:
        case XPS_COPP_CTRL_PKT_PVRST:
        case XPS_COPP_CTRL_PKT_SAMPLEPACKET:
        case XPS_COPP_CTRL_PKT_STATIC_FDB_MOVE:
        case XPS_COPP_CTRL_PKT_SSH:
        case XPS_COPP_CTRL_PKT_TTL_ERROR:
        case XPS_COPP_CTRL_PKT_UDLD:
        case XPS_COPP_CTRL_PKT_VRRP:
        case XPS_COPP_CTRL_PKT_VRRPV6:
        default:
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "xpsCoppCtrlPkt type %d is not supported on port\n", type);
            return XP_ERR_NOT_SUPPORTED;
    }
    XPS_FUNC_EXIT_LOG();
    return XP_NO_ERR;
}

XP_STATUS xpsCoppEnableCtrlPktTrapOnVlan(xpDevice_t devId,
                                         xpsCoppCtrlPkt_e type,
                                         xpsVlan_t vlanId, macAddr_t mac,
                                         xpsPktCmd_e pktCmd, uint8_t enable)
{
    GT_STATUS status;
    GT_BOOL   gtEnable = GT_TRUE;
    XPS_FUNC_ENTRY_LOG();

    if (!enable)
    {
        gtEnable = GT_FALSE;
    }

    switch (type)
    {
        case XPS_COPP_CTRL_PKT_STP:
        case XPS_COPP_CTRL_PKT_LACP:
        case XPS_COPP_CTRL_PKT_LLDP:
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "xpsCoppCtrlPkt type for vlan is not supported\n", type);
            return XP_ERR_NOT_SUPPORTED;
        case XPS_COPP_CTRL_PKT_IGMP_TYPE_LEAVE:
        case XPS_COPP_CTRL_PKT_IGMP_TYPE_QUERY:
        case XPS_COPP_CTRL_PKT_IGMP_TYPE_V1_REPORT:
        case XPS_COPP_CTRL_PKT_IGMP_TYPE_V2_REPORT:
        case XPS_COPP_CTRL_PKT_IGMP_TYPE_V3_REPORT:
            status = cpssHalCtrlPktIgmpOnVlan(devId, vlanId, gtEnable);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssHalCtrlPktIgmpOnVlan failed with for vlanId %d enable %d status %d \n",
                      vlanId, gtEnable, status);
                return xpsConvertCpssStatusToXPStatus(status);
            }
            break;
        case XPS_COPP_CTRL_PKT_IPV6_MLD_V1_DONE:
        case XPS_COPP_CTRL_PKT_IPV6_MLD_V1_REPORT:
        case XPS_COPP_CTRL_PKT_IPV6_MLD_V1_V2:
        case XPS_COPP_CTRL_PKT_IPV6_NEIGHBOR_DISCOVERY:
        case XPS_COPP_CTRL_PKT_MLD_V2_REPORT:
            status = cpssHalCtrlPktIcmpV6OnVlan(devId, vlanId, gtEnable);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssHalCtrlPktIcmpV6OnVlan failed with for vlanId %d enable %d status %d \n",
                      vlanId, gtEnable, status);
                return xpsConvertCpssStatusToXPStatus(status);
            }
            break;
        case XPS_COPP_CTRL_PKT_DHCP:
        case XPS_COPP_CTRL_PKT_DHCPV6:
        case XPS_COPP_CTRL_PKT_SNMP:
            status = cpssHalCtrlPktUdpPortOnVlan(devId, vlanId, gtEnable);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssHalCtrlPktUdpPortOnVlan failed with for vlanId %d enable %d status %d \n",
                      vlanId, gtEnable, status);
                return xpsConvertCpssStatusToXPStatus(status);
            }
            break;
        case XPS_COPP_CTRL_PKT_OSPF:
        case XPS_COPP_CTRL_PKT_OSPFV6:
        case XPS_COPP_CTRL_PKT_PIM:
        case XPS_COPP_CTRL_PKT_ARP_REQUEST:
            status = cpssHalCtrlPktIpLinkLocalOnVlan(devId, vlanId, gtEnable);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssHalCtrlPktIpLinkLocalOnVlan failed with for vlanId %d enable %d status %d \n",
                      vlanId, gtEnable, status);
                return xpsConvertCpssStatusToXPStatus(status);
            }
            break;
        case XPS_COPP_CTRL_PKT_ARP_RESPONSE:
            status = cpssHalHostIfSetPktCmdFdbEntry(devId, vlanId,
                                                    mac, pktCmd);
            if (status != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssHalHostIfSetPktCmdFdbEntry, failed for port %d\n", vlanId);
                return xpsConvertCpssStatusToXPStatus(status);
            }

            break;
        case XPS_COPP_CTRL_PKT_BGP:
        case XPS_COPP_CTRL_PKT_BGPV6:
        case XPS_COPP_CTRL_PKT_EAPOL:
        case XPS_COPP_CTRL_PKT_IP2ME:
        case XPS_COPP_CTRL_PKT_L3_MTU_ERROR:
        case XPS_COPP_CTRL_PKT_PVRST:
        case XPS_COPP_CTRL_PKT_SAMPLEPACKET:
        case XPS_COPP_CTRL_PKT_STATIC_FDB_MOVE:
        case XPS_COPP_CTRL_PKT_SSH:
        case XPS_COPP_CTRL_PKT_TTL_ERROR:
        case XPS_COPP_CTRL_PKT_UDLD:
        case XPS_COPP_CTRL_PKT_VRRP:
        case XPS_COPP_CTRL_PKT_VRRPV6:
        default:
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "xpsCoppCtrlPkt type %d for vlan is not supported\n", type);
            return XP_ERR_NOT_SUPPORTED;
    }
    XPS_FUNC_EXIT_LOG();
    return XP_NO_ERR;
}

XP_STATUS xpsCoppEnableCtrlPktTrapOnSwitch(xpDevice_t devId,
                                           xpsCoppCtrlPkt_e type, xpsPktCmd_e cmd)
{
    GT_STATUS status = GT_OK;
    GT_BOOL   gtEnable = GT_TRUE;
    cpssHalCtrlPktResIeeeMCastProto_e proto;
    cpssHalCtrlPktIcmpV6MsgType_e msgType;
    uint8_t  ndpMsgType;
    cpssHalCtrlPktUdpPort_e udpPort;
    cpssHalCtrlPktIpLinkLocalProto_e ipProto;

    XPS_FUNC_ENTRY_LOG();
    CPSS_PACKET_CMD_ENT pktCmd = CPSS_PACKET_CMD_FORWARD_E;
    pktCmd = xpsConvertPktCmdToCpssPktCmd(cmd);
    switch (type)
    {
        case XPS_COPP_CTRL_PKT_ARP_REQUEST:
            /*
               TRAP - Set global action as TRAP
               COPY_TO_CPU - Set global action as MIRROR_TO_CPU
               FWD - Set global action as FWD.
               DROP - Set global action as TRAP and DROP it in CPU
               (1) Drop is handled by trapping the packets and setting
               the statistical index value to 0 for INTERVENTION_ARP cpu code.

               (2) cpssHalSetBrgGenArpBcastToCpuCmd is global configuration.
               This should be updated according to host-if-trap group settings.
               Avoid this for delete, so other L3 Intf are not affected.
             */


            if (pktCmd == CPSS_PACKET_CMD_DROP_HARD_E)
            {
                pktCmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
            }
            status = cpssHalSetBrgGenArpBcastToCpuCmd(devId,
                                                      CPSS_DXCH_ARP_BCAST_CMD_MODE_VLAN_E, pktCmd);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Set arp broadcast cmd failed");
                return  xpsConvertCpssStatusToXPStatus(status);
            }

            break;

        case XPS_COPP_CTRL_PKT_ARP_RESPONSE:
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
                  " ARP-RESP pktCmd :%d \n ", pktCmd);
            break;
        case XPS_COPP_CTRL_PKT_STP:
            proto = CPSS_HAL_CTRLPKT_RES_IEEE_MCAST_PROTO_STP;
            status = cpssHalCtrlPktIeeeReservedMcastProtoOnSwitch(devId, proto, pktCmd);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssHalCtrlPktIeeeReservedMcastProtoOnSwitch failed with for enable %d status %d \n",
                      gtEnable, status);
                return xpsConvertCpssStatusToXPStatus(status);
            }
            break;
        case XPS_COPP_CTRL_PKT_LACP:
            status = cpssHalCtrlPktLACPOnSwitch(devId, pktCmd);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssHalCtrlPktIeeeReservedMcastProtoOnSwitch failed with for enable %d status %d \n",
                      gtEnable, status);
                return xpsConvertCpssStatusToXPStatus(status);
            }
            break;
        case XPS_COPP_CTRL_PKT_LLDP:
            status = cpssHalCtrlPktLLDPOnSwitch(devId, pktCmd);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssHalCtrlPktIeeeReservedMcastProtoOnSwitch failed with for enable %d status %d \n",
                      gtEnable, status);
                return xpsConvertCpssStatusToXPStatus(status);
            }
            break;
        case XPS_COPP_CTRL_PKT_IGMP_TYPE_LEAVE:
        case XPS_COPP_CTRL_PKT_IGMP_TYPE_QUERY:
        case XPS_COPP_CTRL_PKT_IGMP_TYPE_V1_REPORT:
        case XPS_COPP_CTRL_PKT_IGMP_TYPE_V2_REPORT:
        case XPS_COPP_CTRL_PKT_IGMP_TYPE_V3_REPORT:
            /* All IGMP Types will have same behavior on HW. */
            status = cpssHalCtrlPktIgmpOnSwitch(devId, pktCmd);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssHalCtrlPktIgmpOnSwitch failed with for enable %d status %d \n", gtEnable,
                      status);
                return xpsConvertCpssStatusToXPStatus(status);
            }
            break;
        case XPS_COPP_CTRL_PKT_IPV6_MLD_V1_DONE:
            msgType = CPSS_HAL_CTRLPKT_MESSAGE_TYPE_MLD_V1_DONE;
            status = cpssHalCtrlPktIcmpV6OnSwitch(devId, pktCmd, msgType);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssHalCtrlPktIcmpV6OnSwitch failed with for pktCmd %d status %d \n", pktCmd,
                      status);
                return xpsConvertCpssStatusToXPStatus(status);
            }
            break;
        case XPS_COPP_CTRL_PKT_IPV6_MLD_V1_REPORT:
            msgType = CPSS_HAL_CTRLPKT_MESSAGE_TYPE_MLD_V1_REPORT;
            status = cpssHalCtrlPktIcmpV6OnSwitch(devId, pktCmd, msgType);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssHalCtrlPktIcmpV6OnSwitch failed with for pktCmd %d status %d \n", pktCmd,
                      status);
                return xpsConvertCpssStatusToXPStatus(status);
            }
            break;
        case XPS_COPP_CTRL_PKT_IPV6_MLD_V1_V2:
            msgType = CPSS_HAL_CTRLPKT_MESSAGE_TYPE_MLD_V1_V2;
            status = cpssHalCtrlPktIcmpV6OnSwitch(devId, pktCmd, msgType);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssHalCtrlPktIcmpV6OnSwitch failed with for pktCmd %d status %d \n", pktCmd,
                      status);
                return xpsConvertCpssStatusToXPStatus(status);
            }
            break;
        case XPS_COPP_CTRL_PKT_IPV6_NEIGHBOR_DISCOVERY:
            {
                for (ndpMsgType = CPSS_HAL_CTRLPKT_MESSAGE_TYPE_NDP_ROUTER_SOLICITATION;
                     ndpMsgType <= CPSS_HAL_CTRLPKT_MESSAGE_TYPE_NDP_NEIGHBOUR_ADVERTISEMENT;
                     ndpMsgType++)
                {
                    /* reserving index 0..7 for IcmpV6 msgTypes */
                    status = cpssHalCtrlPktIcmpV6OnSwitch(devId, pktCmd,
                                                          (cpssHalCtrlPktIcmpV6MsgType_e)ndpMsgType);
                    if (status != GT_OK)
                    {
                        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                              "Set NDP cmd failed Msg :%d status : %d ", ndpMsgType, status);
                        return  xpsConvertCpssStatusToXPStatus(status);
                    }
                }

                /* IPv6 Neibhor Adv and Redirect will be UC packets.
                   Hence set the IP proto CPU code table to override trap with
                   cpu code CPSS_NET_IPV6_ICMP_PACKET_E.
                 */
                status = cpssHalNetIfAppSpecificCpuCodeIpProtocolSet(devId,
                                                                     CPSS_HAL_IP_ICMPV6_IDX,
                                                                     CPSS_HAL_CTRL_PKT_IP_PROTOCOL_TYPE_ICMPV6,
                                                                     CPSS_NET_IPV6_ICMP_PACKET_E
                                                                    );
                if (GT_OK != status)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Ip proto Range  failed, error  %d\n", status);
                    return  xpsConvertCpssStatusToXPStatus(status);
                }
            }

            break;
        case XPS_COPP_CTRL_PKT_MLD_V2_REPORT:
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "MLDV2 Not Supported !!!");

                return XP_ERR_NOT_SUPPORTED;
            }
#if 0
            msgType = CPSS_HAL_CTRLPKT_MESSAGE_TYPE_MLD_V2_REPORT;
            status = cpssHalCtrlPktIcmpV6OnSwitch(devId, pktCmd, msgType);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssHalCtrlPktIcmpV6OnSwitch failed with for pktCmd %d status %d \n", pktCmd,
                      status);
                return xpsConvertCpssStatusToXPStatus(status);
            }
            break;
#endif
        case XPS_COPP_CTRL_PKT_DHCP_L2:
            /*
             * Program the UDP Broadcast table to capture UDP BC Packets
             * sent to DHCP UDP Ports.
             */
            udpPort = CPSS_HAL_CTRLPKT_UDP_PORT_DHCP_SERVER;
            status = cpssHalCtrlPktUdpPortOnSwitch(devId, pktCmd, udpPort);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssHalCtrlPktUdpPortOnSwitch failed with \
                        for udpPort %d status %d \n", udpPort, status);
                return xpsConvertCpssStatusToXPStatus(status);
            }
            udpPort = CPSS_HAL_CTRLPKT_UDP_PORT_DHCP_CLIENT;
            status = cpssHalCtrlPktUdpPortOnSwitch(devId, pktCmd, udpPort);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssHalCtrlPktUdpPortOnSwitch failed with \
                        for udpPort %d status %d \n", udpPort, status);
                return xpsConvertCpssStatusToXPStatus(status);
            }
            break;
        case XPS_COPP_CTRL_PKT_DHCPV6_L2:
            /*
             * Configure ACL to capture DHCP packest sent to link-local
             * multicast DIP assigned for all DHCP servers and relay agents.
             */
            status  = cpssHalCtrlPktDhcpV6L2OnSwitch(devId, pktCmd);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssHalCtrlPktDhcpV6OnSwitch failed with for pktCmd %d status %d \n", pktCmd,
                      status);
                return xpsConvertCpssStatusToXPStatus(status);
            }

            break;
        case XPS_COPP_CTRL_PKT_DHCP:
            /*
             * UC pkt config. Additionally LPM must have a hit.
             * Program the App specific CPU code table.
             */
            status  = cpssHalCtrlPktDhcpV4OnSwitch(devId, pktCmd);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssHalCtrlPktDhcpV4OnSwitch failed with for pktCmd %d status %d \n", pktCmd,
                      status);
                return xpsConvertCpssStatusToXPStatus(status);
            }

            /* BC pkt configuration for DHCP trap. Note that both L2/L3 must have
               same pakcet cmd, else last configured cmd will be updated in UDP
               BC table and the same will reflect for both L2 and L3. */
            udpPort = CPSS_HAL_CTRLPKT_UDP_PORT_DHCP_SERVER;
            status = cpssHalCtrlPktUdpPortOnSwitch(devId, pktCmd, udpPort);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssHalCtrlPktUdpPortOnSwitch failed with \
                        for udpPort %d status %d \n", udpPort, status);
                return xpsConvertCpssStatusToXPStatus(status);
            }

            udpPort = CPSS_HAL_CTRLPKT_UDP_PORT_DHCP_CLIENT;
            status = cpssHalCtrlPktUdpPortOnSwitch(devId, pktCmd, udpPort);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssHalCtrlPktUdpPortOnSwitch failed with \
                        for udpPort %d status %d \n", udpPort, status);
                return xpsConvertCpssStatusToXPStatus(status);
            }

            break;
        case XPS_COPP_CTRL_PKT_DHCPV6:
            /*
             * Program the App specific CPU code table.
             */
            status  = cpssHalCtrlPktDhcpV6OnSwitch(devId, pktCmd);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssHalCtrlPktDhcpV6OnSwitch failed with for pktCmd %d status %d \n", pktCmd,
                      status);
                return xpsConvertCpssStatusToXPStatus(status);
            }

            break;

        case XPS_COPP_CTRL_PKT_SNMP:
            udpPort = CPSS_HAL_CTRLPKT_UDP_PORT_SNMP;
            status = cpssHalCtrlPktUdpPortOnSwitch(devId, pktCmd, udpPort);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssHalCtrlPktUdpPortOnSwitch failed with \
                        for udpPort %d status %d \n", udpPort, status);
                return xpsConvertCpssStatusToXPStatus(status);
            }
            udpPort = CPSS_HAL_CTRLPKT_UDP_PORT_SNMP_TRAP;
            status = cpssHalCtrlPktUdpPortOnSwitch(devId, pktCmd, udpPort);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssHalCtrlPktUdpPortOnSwitch failed with \
                        for udpPort %d status %d \n", udpPort, status);
                return xpsConvertCpssStatusToXPStatus(status);
            }
            break;
        case XPS_COPP_CTRL_PKT_OSPF:
            ipProto = CPSS_HAL_CTRLPKT_IP_LL_PROTO_OSPF;
            status = cpssHalCtrlPktIpLinkLocalOnSwitch(devId, pktCmd, ipProto);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssHalCtrlPktIpLinkLocalOnSwitch failed with for pktCmd %d status %d \n",
                      pktCmd, status);
                return xpsConvertCpssStatusToXPStatus(status);
            }
            ipProto = CPSS_HAL_CTRLPKT_IP_LL_PROTO_OSPF_DR;
            status = cpssHalCtrlPktIpLinkLocalOnSwitch(devId, pktCmd, ipProto);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssHalCtrlPktIpLinkLocalOnSwitch failed with for pktCmd %d status %d \n",
                      pktCmd, status);
                return xpsConvertCpssStatusToXPStatus(status);
            }
            break;
        case XPS_COPP_CTRL_PKT_OSPFV6:
            ipProto = CPSS_HAL_CTRLPKT_IP_LL_PROTO_OSPFV6;
            status = cpssHalCtrlPktIpLinkLocalOnSwitch(devId, pktCmd, ipProto);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssHalCtrlPktIpLinkLocalOnSwitch failed with for pktCmd %d status %d \n",
                      pktCmd, status);
                return xpsConvertCpssStatusToXPStatus(status);
            }
            ipProto = CPSS_HAL_CTRLPKT_IP_LL_PROTO_OSPFV6_DR;
            status = cpssHalCtrlPktIpLinkLocalOnSwitch(devId, pktCmd, ipProto);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssHalCtrlPktIpLinkLocalOnSwitch failed with for pktCmd %d status %d \n",
                      pktCmd, status);
                return xpsConvertCpssStatusToXPStatus(status);
            }
            break;
        case XPS_COPP_CTRL_PKT_PIM:
            ipProto = CPSS_HAL_CTRLPKT_IP_LL_PROTO_PIMV2;
            status = cpssHalCtrlPktIpLinkLocalOnSwitch(devId, pktCmd, ipProto);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssHalCtrlPktIpLinkLocalOnSwitch failed with for pktCmd %d status %d \n",
                      pktCmd, status);
                return xpsConvertCpssStatusToXPStatus(status);
            }
            ipProto = CPSS_HAL_CTRLPKT_IP_LL_PROTO_PIMV6;
            status = cpssHalCtrlPktIpLinkLocalOnSwitch(devId, pktCmd, ipProto);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssHalCtrlPktIpLinkLocalOnSwitch failed with for pktCmd %d status %d \n",
                      pktCmd, status);
                return xpsConvertCpssStatusToXPStatus(status);
            }
            break;
        case XPS_COPP_CTRL_PKT_VRRP:
        case XPS_COPP_CTRL_PKT_VRRPV6:
            ipProto = CPSS_HAL_CTRLPKT_IP_LL_PROTO_VRRP;
            status = cpssHalCtrlPktIpLinkLocalOnSwitch(devId, pktCmd, ipProto);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssHalCtrlPktIpLinkLocalOnSwitch failed with for pktCmd %d status %d \n",
                      pktCmd, status);
                return xpsConvertCpssStatusToXPStatus(status);
            }
            ipProto = CPSS_HAL_CTRLPKT_IP_LL_PROTO_VRRPV6;
            status = cpssHalCtrlPktIpLinkLocalOnSwitch(devId, pktCmd, ipProto);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssHalCtrlPktIpLinkLocalOnSwitch failed with for pktCmd %d status %d \n",
                      pktCmd, status);
                return xpsConvertCpssStatusToXPStatus(status);
            }
            break;
        case XPS_COPP_CTRL_PKT_L3_MTU_ERROR:
            status = cpssHalCtrlPktIpMtuExceedOnSwitch(devId, pktCmd);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssHalCtrlPktIpMtuExceedOnSwitch failed with for pktCmd %d status %d \n",
                      pktCmd, status);
                return xpsConvertCpssStatusToXPStatus(status);
            }
            break;
        case XPS_COPP_CTRL_PKT_TTL_ERROR:
            status = cpssHalCtrlPktIpTtlErrorOnSwitch(devId, pktCmd);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssHalCtrlPktIpTtlErrorOnSwitch failed with for pktCmd %d status %d \n",
                      pktCmd, status);
                return xpsConvertCpssStatusToXPStatus(status);
            }
            break;
        case XPS_COPP_CTRL_PKT_BGP:
            status = cpssHalCtrlPktBgpOnSwitch(devId, CPSS_HAL_BGP_IPV4, pktCmd);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssHalCtrlPktBgpOnSwitch failed with for pktCmd %d status %d \n", pktCmd,
                      status);
                return xpsConvertCpssStatusToXPStatus(status);
            }
            break;
        case XPS_COPP_CTRL_PKT_BGPV6:
            status = cpssHalCtrlPktBgpOnSwitch(devId, CPSS_HAL_BGP_IPV6, pktCmd);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssHalCtrlPktBgpOnSwitch failed with for pktCmd %d status %d \n", pktCmd,
                      status);
                return xpsConvertCpssStatusToXPStatus(status);
            }
            break;
        case XPS_COPP_CTRL_PKT_UDLD:
        case XPS_COPP_CTRL_PKT_PVRST:
            status = cpssHalCtrlPktCiscoL2ProtOnSwitch(devId, pktCmd);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssHalCtrlPktCiscoL2ProtOnSwitch failed with for pktCmd %d status %d \n",
                      pktCmd, status);
                return xpsConvertCpssStatusToXPStatus(status);
            }
            break;
        case XPS_COPP_CTRL_PKT_SSH:
            status = cpssHalCtrlPktSshOnSwitch(devId, pktCmd);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssHalCtrlPktSshOnSwitch failed with for pktCmd %d status %d \n", pktCmd,
                      status);
                return xpsConvertCpssStatusToXPStatus(status);
            }
            break;
        case XPS_COPP_CTRL_PKT_EAPOL:
            status = cpssHalCtrlPktEAPOLOnSwitch(devId, pktCmd);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssHalCtrlPktSshOnSwitch failed with for pktCmd %d status %d \n", pktCmd,
                      status);
                return xpsConvertCpssStatusToXPStatus(status);
            }
            break;
        case XPS_COPP_CTRL_PKT_IP2ME:
        case XPS_COPP_CTRL_PKT_SAMPLEPACKET:
        case XPS_COPP_CTRL_PKT_STATIC_FDB_MOVE:
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
                  "xpsCoppCtrlPkt type for switch is not supported. Returning success\n", type);
            break;
        default:
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
                  "xpsCoppCtrlPkt type for switch is not supported\n", type);
            return XP_ERR_NOT_SUPPORTED;
    }
    XPS_FUNC_EXIT_LOG();
    return XP_NO_ERR;
}

XP_STATUS xpsCoppDisplayCtrlPktTraps(xpDevice_t devId, xpsCoppCtrlPkt_e type)
{
    GT_STATUS status;
    cpssHalCtrlPktTypes_e cpssHalType = CPSS_HAL_CTRLPKT_TYPE_ALL;
    XPS_FUNC_ENTRY_LOG();

    switch (type)
    {
        case XPS_COPP_CTRL_PKT_ARP_REQUEST:
        case XPS_COPP_CTRL_PKT_ARP_RESPONSE:
            cpssHalType = CPSS_HAL_CTRLPKT_TYPE_ARP;
            break;
        case XPS_COPP_CTRL_PKT_STP:
        case XPS_COPP_CTRL_PKT_LACP:
        case XPS_COPP_CTRL_PKT_LLDP:
        case XPS_COPP_CTRL_PKT_EAPOL:
            cpssHalType = CPSS_HAL_CTRLPKT_TYPE_IEEE_RESERVED_MCAST;
            break;
        case XPS_COPP_CTRL_PKT_IGMP_TYPE_LEAVE:
        case XPS_COPP_CTRL_PKT_IGMP_TYPE_QUERY:
        case XPS_COPP_CTRL_PKT_IGMP_TYPE_V1_REPORT:
        case XPS_COPP_CTRL_PKT_IGMP_TYPE_V2_REPORT:
        case XPS_COPP_CTRL_PKT_IGMP_TYPE_V3_REPORT:
            cpssHalType = CPSS_HAL_CTRLPKT_TYPE_IGMP;
            break;
        case XPS_COPP_CTRL_PKT_IPV6_MLD_V1_DONE:
        case XPS_COPP_CTRL_PKT_IPV6_MLD_V1_REPORT:
        case XPS_COPP_CTRL_PKT_IPV6_MLD_V1_V2:
        case XPS_COPP_CTRL_PKT_IPV6_NEIGHBOR_DISCOVERY:
        case XPS_COPP_CTRL_PKT_MLD_V2_REPORT:
            cpssHalType = CPSS_HAL_CTRLPKT_TYPE_ICMPV6;
            break;
        case XPS_COPP_CTRL_PKT_DHCP:
        case XPS_COPP_CTRL_PKT_DHCPV6:
        case XPS_COPP_CTRL_PKT_SNMP:
            cpssHalType = CPSS_HAL_CTRLPKT_TYPE_UDP_PORT;
            break;
        case XPS_COPP_CTRL_PKT_OSPF:
        case XPS_COPP_CTRL_PKT_OSPFV6:
        case XPS_COPP_CTRL_PKT_PIM:
        case XPS_COPP_CTRL_PKT_VRRP:
        case XPS_COPP_CTRL_PKT_VRRPV6:
            cpssHalType = CPSS_HAL_CTRLPKT_TYPE_IP_LINK_LOCAL;
            break;
        case XPS_COPP_CTRL_PKT_MAX:
            cpssHalType = CPSS_HAL_CTRLPKT_TYPE_ALL;
            break;
        case XPS_COPP_CTRL_PKT_BGP:
        case XPS_COPP_CTRL_PKT_BGPV6:
        case XPS_COPP_CTRL_PKT_IP2ME:
        case XPS_COPP_CTRL_PKT_L3_MTU_ERROR:
        case XPS_COPP_CTRL_PKT_PVRST:
        case XPS_COPP_CTRL_PKT_SAMPLEPACKET:
        case XPS_COPP_CTRL_PKT_STATIC_FDB_MOVE:
        case XPS_COPP_CTRL_PKT_SSH:
        case XPS_COPP_CTRL_PKT_TTL_ERROR:
        case XPS_COPP_CTRL_PKT_UDLD:
        default:
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "xpsCoppCtrlPkt type for switch is not supported\n", type);
            return XP_ERR_NOT_SUPPORTED;
    }
    status = cpssHalCtrlPktDump(devId, cpssHalType);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssHalCtrlPktDump failed for type %d error %d\n", cpssHalType, status);
    }
    XPS_FUNC_EXIT_LOG();
    return XP_NO_ERR;
}

XP_STATUS xpsCoppInbandMgmtRatelimitSet(xpDevice_t devId, uint32_t rate)
{
    GT_STATUS rc = GT_OK;
    XP_STATUS ret = XP_NO_ERR;

    XPS_FUNC_ENTRY_LOG();

    uint32_t index = XP_COPP_RATELIMIT_INDEX_INBAND_MGMT;
    if ((ret = xpsAllocatorAllocateWithId(XP_SCOPE_DEFAULT, XP_SAI_ALLOC_POLICER,
                                          index)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: xpsAllocator failed\n");
        return ret;
    }

    /* set ratelimit for mgmt traffic */
    rc = cpssHalDefaultRateLimitSet(devId,
                                    (CPSS_NET_RX_CPU_CODE_ENT)CPSS_HAL_CTRLPKT_CPU_CODE_INBAND_MGMT, rate, index);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: rate limit for mgmt traffic failed\n");
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    /* set low priority cpu queue for mgmt traffic */
    rc = cpssHalQosBindReasonCodeToCpuQueue(devId,
                                            (CPSS_NET_RX_CPU_CODE_ENT)CPSS_HAL_CTRLPKT_CPU_CODE_INBAND_MGMT,
                                            XP_COPP_QUEUE_NUM_INBAND_MGMT);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: cpu code to queue bind failed\n");
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    XPS_FUNC_EXIT_LOG();
    return XP_NO_ERR;
}

#ifdef __cplusplus
}
#endif
