/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/**
********************************************************************************
* @file cpssHalCopp.c
*
* @brief Private API implementation for CPSS CPU Code Table Read, Write, Update which can be used in XPS layer.
*
* @version   01
********************************************************************************
*/

#include "xpsCopp.h"
#include "cpssHalFdb.h"
#include "cpssHalUtil.h"
#include "xpsL3.h"
#include "xpsFdb.h"
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
#include "xpsAcl.h"
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>

//TODO what is index and how to get it.
GT_STATUS cpssHalDefaultRateLimitSet(uint32_t devId,
                                     CPSS_NET_RX_CPU_CODE_ENT cpuCode, uint32_t rateLimit, uint32_t index)
{
    GT_STATUS rc;
    GT_U32    rateLimitPerWindow;
    GT_U8     cpssDevId;
    CPSS_DXCH_NET_CPU_CODE_TABLE_ENTRY_STC entryInfo;

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevId)
    {
        /* Configure TO CPU rate limiter window size to 100us = 100,000ns */
        /* This assumed initialized value */
        rc = cpssDxChNetIfCpuCodeRateLimiterWindowResolutionSet(cpssDevId, 100000);
        if (GT_OK != rc)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "[XPS]: cpssDxChNetIfCpuCodeRateLimiterWindowResolutionSet FAILED, rc = [%d]",
                  rc);
            return XP_ERR_INVALID_VALUE;
        }

        /* Calculate rate limit per window */
        rateLimitPerWindow = rateLimit/DEFAULT_RATE_LIMITER_WINDOW_SIZE;

        rc = cpssDxChNetIfCpuCodeRateLimiterTableSet(cpssDevId, index,
                                                     DEFAULT_RATE_LIMITER_WINDOW_SIZE, rateLimitPerWindow);
        if (GT_OK != rc)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "[XPS]: cpssDxChNetIfCpuCodeRateLimiterTableSet FAILED, rc = [%d]", rc);
            return XP_ERR_INVALID_VALUE;
        }

        /* Get the cpu code entry */
        rc = cpssDxChNetIfCpuCodeTableGet(cpssDevId, cpuCode, &entryInfo);
        if (GT_OK != rc)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "[XPS]: cpssDxChNetIfCpuCodeTableGet FAILED, rc = [%d]", rc);
            return XP_ERR_INVALID_VALUE;
        }

        entryInfo.cpuCodeRateLimiterIndex = index;

        /* Set the cpu code with rate limiter */
        rc = cpssDxChNetIfCpuCodeTableSet(cpssDevId, cpuCode, &entryInfo);
        if (GT_OK != rc)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "[XPS]: cpssDxChNetIfCpuCodeTableSet FAILED, rc = [%d]", rc);
            return XP_ERR_INVALID_VALUE;
        }
    }
    return GT_OK;
}



GT_STATUS cpssHalCoppSetCpuRateLimit(uint32_t devId, uint32_t index,
                                     uint32_t rateLimit)
{
    GT_STATUS rc;
    GT_U32    rateLimitPerWindow;
    GT_U8     cpssDevId;

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevId)
    {
        /* Configure to CPU rate limiter window size to 100us = 100,000ns */
        rc = cpssDxChNetIfCpuCodeRateLimiterWindowResolutionSet(cpssDevId, 100000);
        if (GT_OK != rc)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set cpu code rate limiter window resolution, rc = [%d]", rc);
            return GT_FAIL;
        }

        /* Calculate rate limit per window */
        rateLimitPerWindow = rateLimit/DEFAULT_RATE_LIMITER_WINDOW_SIZE;

        rc = cpssDxChNetIfCpuCodeRateLimiterTableSet(cpssDevId, index,
                                                     DEFAULT_RATE_LIMITER_WINDOW_SIZE, rateLimitPerWindow);
        if (GT_OK != rc)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set cpu code rate limiter table, rc = [%d]", rc);
            return GT_FAIL;
        }
    }

    return GT_OK;
}

GT_STATUS cpssHalCoppGetCpuRateLimit(uint32_t devId, uint32_t index,
                                     uint32_t *rateLimit)
{
    GT_STATUS rc;
    GT_U32    rateLimitPerWindow;
    GT_U32    windowSize;
    rc = cpssDxChNetIfCpuCodeRateLimiterTableGet(devId, index, &windowSize,
                                                 &rateLimitPerWindow);
    if (GT_OK != rc)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set cpu code rate limiter table, rc = [%d]", rc);
        return GT_FAIL;
    }
    *rateLimit = (rateLimitPerWindow) * (windowSize);
    return GT_OK;
}

GT_STATUS cpssHalCoppReadCpuCodeTableEntry(uint32_t devId,
                                           CPSS_NET_RX_CPU_CODE_ENT cpuCode, CPSS_DXCH_NET_CPU_CODE_TABLE_ENTRY_STC *entry)
{
    GT_STATUS rc;
    GT_U8     cpssDevId = 0 ;
    rc = cpssDxChNetIfCpuCodeTableGet(cpssDevId, cpuCode, entry);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get cpu code table entry, "
              "rc:%d, dev:%d, cpuCode:%d\n", rc, devId, cpuCode);
        return rc;
    }

    return GT_OK;
}

GT_STATUS cpssHalCoppWriteCpuCodeTableEntry(uint32_t devId,
                                            CPSS_NET_RX_CPU_CODE_ENT cpuCode, CPSS_DXCH_NET_CPU_CODE_TABLE_ENTRY_STC *entry)
{
    GT_STATUS rc;
    GT_U8     cpssDevId;

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevId)
    {
        rc = cpssDxChNetIfCpuCodeTableSet(cpssDevId, cpuCode, entry);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set cpu code table entry, "
                  "rc:%d, dev:%d, cpuCode:%d\n", rc, devId, cpuCode);
            return rc;
        }
    }

    return GT_OK;
}

uint32_t cpssHalCoppGetAclEntryPriority(uint32_t ruleId, uint32_t keyType,
                                        CPSS_NET_RX_CPU_CODE_ENT cpuCode)
{
    switch (cpuCode)
    {
        case CPSS_NET_INTERVENTION_ARP_E:
            return CPSS_HAL_COPP_ACL_PRIO_INTERVENTION_ARP_E;
        case CPSS_NET_ARP_REPLY_TO_ME_E:
            return CPSS_HAL_COPP_ACL_PRIO_ARP_REPLY_TO_ME_E;
        case CPSS_HAL_CTRLPKT_CPU_CODE_DHCPv6: //DHCPv6 L2 and L3 gets same CPU code
            return CPSS_HAL_COPP_ACL_PRIO_CPU_CODE_DHCPv6_L2_L3;
        case CPSS_NET_UDP_BC_MIRROR_TRAP0_E: //DHCPv4 L2 and L3 gets same CPU code
            return CPSS_HAL_COPP_ACL_PRIO_CPU_CODE_DHCPv4_L2_L3;
        case CPSS_NET_INTERVENTION_IGMP_E:
            return CPSS_HAL_COPP_ACL_PRIO_INTERVENTION_IGMP_E;
        case CPSS_NET_IPV6_ICMP_PACKET_E:
            return CPSS_HAL_COPP_ACL_PRIO_IPV6_ICMP_PACKET_E;
        case CPSS_NET_IEEE_RSRVD_MULTICAST_ADDR_2_E:
            return CPSS_HAL_COPP_ACL_PRIO_IEEE_RSRVD_MULTICAST_ADDR_2_E;
        case CPSS_NET_IEEE_RSRVD_MULTICAST_ADDR_3_E:
            return CPSS_HAL_COPP_ACL_PRIO_IEEE_RSRVD_MULTICAST_ADDR_3_E;
        case CPSS_NET_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_1_E:
            if (keyType == XPS_PCL_IPV6_L4_KEY)
            {
                return CPSS_HAL_COPP_ACL_PRIO_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_1_E;
            }
            else
            {
                return CPSS_HAL_COPP_ACL_PRIO_IPV4_LINK_LOCAL_MC_DIP_TRP_MRR_1_E;
            }
        case CPSS_NET_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_3_E:
            if (keyType == XPS_PCL_IPV6_L4_KEY)
            {
                return CPSS_HAL_COPP_ACL_PRIO_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_3_E;
            }
            else
            {
                return CPSS_HAL_COPP_ACL_PRIO_IPV4_LINK_LOCAL_MC_DIP_TRP_MRR_3_E;
            }
        case CPSS_NET_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_2_E:
            if (keyType == XPS_PCL_IPV6_L4_KEY)
            {
                return CPSS_HAL_COPP_ACL_PRIO_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_2_E;
            }
            else
            {
                return CPSS_HAL_COPP_ACL_PRIO_IPV4_LINK_LOCAL_MC_DIP_TRP_MRR_2_E;
            }
        case CPSS_NET_UDP_BC_MIRROR_TRAP2_E:
            if (keyType == XPS_PCL_IPV6_L4_KEY)
            {
                return CPSS_HAL_COPP_ACL_PRIO_IPV6_UDP_BC_MIRROR_TRAP2_E;
            }
            else
            {
                return CPSS_HAL_COPP_ACL_PRIO_IPV4_UDP_BC_MIRROR_TRAP2_E;
            }
        case CPSS_NET_IEEE_RSRVD_MULTICAST_ADDR_1_E:
            return CPSS_HAL_COPP_ACL_PRIO_IEEE_RSRVD_MULTICAST_ADDR_1_E;
        case CPSS_NET_IP_ZERO_TTL_TRAP_E:
            if (keyType == XPS_PCL_IPV6_L4_KEY)
            {
                return CPSS_HAL_COPP_ACL_PRIO_IPV6_ZERO_TTL_TRAP_E;
            }
            else
            {
                return CPSS_HAL_COPP_ACL_PRIO_IPV4_ZERO_TTL_TRAP_E;
            }
        case CPSS_NET_IPV6_TTL_TRAP_E:
            return CPSS_HAL_COPP_ACL_PRIO_IPV6_TTL_TRAP_E;
        case CPSS_NET_IP_MTU_EXCEED_E:
            if (keyType == XPS_PCL_IPV6_L4_KEY)
            {
                return CPSS_HAL_COPP_ACL_PRIO_IPV6_MTU_EXCEED_E;
            }
            else
            {
                return CPSS_HAL_COPP_ACL_PRIO_IPV4_MTU_EXCEED_E;
            }
        case CPSS_HAL_CTRLPKT_CPU_CODE_BGPv4v6:
            if (keyType == XPS_PCL_IPV6_L4_KEY)
            {
                return CPSS_HAL_COPP_ACL_PRIO_CPU_CODE_BGPv6;
            }
            else
            {
                return CPSS_HAL_COPP_ACL_PRIO_CPU_CODE_BGPv4;
            }
        case CPSS_NET_CISCO_MULTICAST_MAC_RANGE_E:
            return CPSS_HAL_COPP_ACL_PRIO_CISCO_MULTICAST_MAC_RANGE_E;
        case CPSS_NET_ROUTED_PACKET_FORWARD_E:
            if (keyType == XPS_PCL_IPV6_L4_KEY)
            {
                return CPSS_HAL_COPP_ACL_PRIO_IPV6_ROUTED_PACKET_FORWARD_E;
            }
            else
            {
                return CPSS_HAL_COPP_ACL_PRIO_IPV4_ROUTED_PACKET_FORWARD_E;
            }
        case CPSS_NET_ROUTE_ENTRY_TRAP_E:
            if (keyType == XPS_PCL_IPV6_L4_KEY)
            {
                return CPSS_HAL_COPP_ACL_PRIO_IPV6_ROUTE_ENTRY_TRAP_E;
            }
            else
            {
                return CPSS_HAL_COPP_ACL_PRIO_IPV4_ROUTE_ENTRY_TRAP_E;
            }
        case CPSS_NET_IPV6_ROUTE_TRAP_E:
            return CPSS_HAL_COPP_ACL_PRIO_IPV6_ROUTE_TRAP_E;
        case CPSS_HAL_CTRLPKT_CPU_CODE_SSH:
            if (keyType == XPS_PCL_IPV6_L4_KEY)
            {
                return CPSS_HAL_COPP_ACL_PRIO_IPV6_CPU_CODE_SSH;
            }
            else
            {
                return CPSS_HAL_COPP_ACL_PRIO_IPV4_CPU_CODE_SSH;
            }
        case CPSS_NET_INGRESS_MIRRORED_TO_ANLYZER_E:
            if (keyType == XPS_PCL_NON_IP_KEY)
            {
                return CPSS_HAL_COPP_ACL_PRIO_NONIP_INGRESS_MIRRORED_TO_ANLYZER_E;
            }
            else if (keyType == XPS_PCL_ARP_KEY)
            {
                return CPSS_HAL_COPP_ACL_PRIO_ARP_INGRESS_MIRRORED_TO_ANLYZER_E;
            }
            else if (keyType == XPS_PCL_IPV6_L4_KEY)
            {
                return CPSS_HAL_COPP_ACL_PRIO_IPV6_INGRESS_MIRRORED_TO_ANLYZER_E;
            }
            else
            {
                return CPSS_HAL_COPP_ACL_PRIO_IPV4_INGRESS_MIRRORED_TO_ANLYZER_E;
            }
        case CPSS_NET_EGRESS_MIRRORED_TO_ANLYZER_E:
            if (keyType == XPS_PCL_NON_IP_KEY)
            {
                return CPSS_HAL_COPP_ACL_PRIO_NONIP_EGRESS_MIRRORED_TO_ANLYZER_E;
            }
            else if (keyType == XPS_PCL_ARP_KEY)
            {
                return CPSS_HAL_COPP_ACL_PRIO_ARP_EGRESS_MIRRORED_TO_ANLYZER_E;
            }
            else if (keyType == XPS_PCL_IPV6_L4_KEY)
            {
                return CPSS_HAL_COPP_ACL_PRIO_IPV6_EGRESS_MIRRORED_TO_ANLYZER_E;
            }
            else
            {
                return CPSS_HAL_COPP_ACL_PRIO_IPV4_EGRESS_MIRRORED_TO_ANLYZER_E;
            }
        case CPSS_NET_STATIC_ADDR_MOVED_E:
            if (keyType == XPS_PCL_NON_IP_KEY)
            {
                return CPSS_HAL_COPP_ACL_PRIO_NONIP_STATIC_ADDR_MOVED_E;
            }
            else if (keyType == XPS_PCL_ARP_KEY)
            {
                return CPSS_HAL_COPP_ACL_PRIO_ARP_STATIC_ADDR_MOVED_E;
            }
            else if (keyType == XPS_PCL_IPV6_L4_KEY)
            {
                return CPSS_HAL_COPP_ACL_PRIO_IPV6_STATIC_ADDR_MOVED_E;
            }
            else
            {
                return CPSS_HAL_COPP_ACL_PRIO_IPV4_STATIC_ADDR_MOVED_E;
            }
        case CPSS_HAL_CTRLPKT_CPU_CODE_EAPOL_TRAP:
            return CPSS_HAL_COPP_ACL_PRIO_EAPOL_ADDR_E;
        default:
            break;
    }

    return ruleId;
}

GT_STATUS cpssHalCoppWriteEgressAclTableEntry(uint32_t devId,
                                              uint32_t tableId, uint32_t ruleId, uint32_t keyType, CPSS_PACKET_CMD_ENT pktCmd,
                                              CPSS_NET_RX_CPU_CODE_ENT cpuCode, uint32_t policerId)
{
    XP_STATUS ret = XP_NO_ERR;
    GT_STATUS rc;
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT dsaCode = 0;
    uint32_t             maxFields = 2;
    xpsAclkeyFieldList_t aclFieldData;
    xpsPclAction_t       aclEntryData;
    xpsAclkeyField_t     aclFieldList[maxFields + 1];
    uint8_t              acl_value[maxFields];
    uint8_t              acl_mask[maxFields];
    uint32_t             acl_value_for_port_bitmap;
    uint32_t             acl_mask_for_port_bitmap;
    uint32_t             priority = ruleId;

    xpsAclKeyFlds_t      keyFlds;
    int                  i;

    memset(&aclFieldData, 0x00, sizeof(aclFieldData));
    memset(&aclEntryData, 0x00, sizeof(aclEntryData));
    memset(&aclFieldList, 0x00, sizeof(aclFieldList));
    memset(acl_value, 0x00, sizeof(acl_value));
    memset(acl_mask, 0x00, sizeof(acl_mask));
    memset(&acl_value_for_port_bitmap, 0x00, sizeof(acl_value_for_port_bitmap));
    memset(&acl_mask_for_port_bitmap, 0x00, sizeof(acl_mask_for_port_bitmap));

    aclEntryData.pktCmd = pktCmd;
    aclEntryData.actionStop = GT_FALSE;

    for (i = 0; i < maxFields; i++)
    {
        aclFieldList[i].value = &acl_value[i];
        aclFieldList[i].mask  = &acl_mask[i];
    }

    /* Field for port bitmap */
    aclFieldList[i].value = (uint8_t*)&acl_value_for_port_bitmap;
    aclFieldList[i].mask  = (uint8_t*)&acl_mask_for_port_bitmap;

    aclFieldData.fldList = aclFieldList;
    aclFieldData.isValid = 1;

    rc = prvCpssDxChNetIfCpuToDsaCode(cpuCode, &dsaCode);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get dsa code, "
              "rc:%d, dev:%d, cpuCode:%d\n", rc, devId, cpuCode);
        return rc;
    }

    /* Set ACL field: CPU Code */
    uint8_t dsaCodeMask  = 0xFF;
    aclFieldData.fldList[0].keyFlds = XPS_PCL_CPU_CODE;
    memcpy(aclFieldData.fldList[0].value, &dsaCode, sizeof(uint8_t));
    memcpy(aclFieldData.fldList[0].mask, &dsaCodeMask, sizeof(uint8_t));

    switch (keyType)
    {
        case XPS_PCL_NON_IP_KEY:
            keyFlds = XPS_PCL_IS_NON_IP;
            break;
        case XPS_PCL_ARP_KEY:
            keyFlds = XPS_PCL_IS_ARP;
            break;
        case XPS_PCL_IPV4_L4_KEY:
            keyFlds = XPS_PCL_IS_IPV4;
            break;
        case XPS_PCL_IPV6_L4_KEY:
        case XPS_PCL_VXLAN_IPV6_KEY:
            keyFlds = XPS_PCL_IS_IPV6;
            break;
        default:
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Invalid key Format: %d!\n", keyType);
            return GT_FAIL;
    }

    /* Set ACL field: Key Format */
    uint8_t value = 1;
    uint8_t mask = 0x1;
    aclFieldData.fldList[1].keyFlds = keyFlds;
    memcpy(aclFieldData.fldList[1].value, &value, sizeof(uint8_t));
    memcpy(aclFieldData.fldList[1].mask, &mask, sizeof(uint8_t));

    aclFieldData.numFlds = maxFields;

    aclEntryData.policer.policerEnable = (policerId == 0) ?
                                         CPSS_DXCH_PCL_POLICER_DISABLE_ALL_E :
                                         CPSS_DXCH_PCL_POLICER_ENABLE_METER_AND_COUNTER_E;
    aclEntryData.policer.policerId = policerId;
    aclEntryData.flowId = policerId;

    priority = cpssHalCoppGetAclEntryPriority(ruleId, keyType, cpuCode);
    ret = xpsAclWriteEntry(devId, tableId,
                           priority,
                           ruleId,
                           &aclFieldData, aclEntryData, keyType, false);
    if (ret != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: Add Egress Acl table entry - %d failed\n", keyType);
        return ret;
    }

    return ret;
}

