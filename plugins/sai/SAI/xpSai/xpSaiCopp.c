// xpSaiCopp.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpSaiCopp.h"
#include "xpsCopp.h"
#include "cpssHalDevice.h"

XP_SAI_LOG_REGISTER_API(SAI_API_HOSTIF);

//Func: xpSaiCoppBumPolicerSet

sai_status_t xpSaiCoppBumPolicerSet(xpsDevice_t xpsDevId, bool enable)
{
    XP_STATUS           xpStatus    = XP_NO_ERR;
    xpsInterfaceId_t    intfId      = XPS_INTF_INVALID_ID;
    uint8_t             port        = 0;
    uint8_t             maxPortNum  = 0;

    xpStatus = xpsGlobalSwitchControlGetMaxPorts(xpsDevId, &maxPortNum);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not get maximum ports number, xpStatus %d\n", xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    XPS_GLOBAL_PORT_ITER(port, maxPortNum)
    {
        xpStatus = xpsPortGetPortIntfId(xpsDevId, port, &intfId);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to get interface ID by port %d, dev %d, error %d\n",
                           port, xpsDevId, xpStatus);
            return xpsStatus2SaiStatus(xpStatus);
        }

        xpStatus = xpsPortSetField(xpsDevId, intfId, XPS_PORT_BUM_POLICER_EN, enable);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Could not set XPS_PORT_BUM_POLICER_EN port field, error %d\n",
                           xpStatus);
            continue;
        }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiCoppQueueMappingSet

sai_status_t xpSaiCoppQueueMappingSet(xpsDevice_t xpsDevId, uint32_t queue,
                                      uint32_t reasonCode)
{
    XP_STATUS xpStatus = XP_NO_ERR;

    XP_SAI_LOG_DBG("Map reason code %d into queue %d\n", reasonCode, queue);

    xpStatus = xpsQosCpuScAddReasonCodeToCpuQueueMap(xpsDevId, XP_MCPU, reasonCode,
                                                     queue);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not map reason code %d to queue %d, error %d",
                       reasonCode, queue, xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    return SAI_STATUS_SUCCESS;
}

/**
 * \brief CoPP table entry create or update
 *
 * This API creates new CoPP table entries or updates existing entries. Single API call
 * creates or updates exactly maxPortNum entries - one entry for each HW port.
 * If the port is marked as valid in @validPort array then the packet's command
 * will be set to @pktCmd. In case @validPort is NULL or the port is marked as invalid
 * in @validPort array then the packet command will be set to @defaultPktCmd.
 *
 * \param[in] xpsDevice_t xpsDevId
 * \param[in] bool *validPort
 * \param[in] uint32_t policerId
 * \param[in] xpPktCmd_e pktCmd
 * \param[in] uint32_t reasonCode
 * \param[in] xpPktCmd_e defaultPktCmd
 *
 * \return sai_status_t
 */
sai_status_t xpSaiCoppEntryAdd(xpsDevice_t xpsDevId, const bool *validPort,
                               uint32_t policerId, xpsPktCmd_e pktCmd, uint32_t reasonCode,
                               xpsPktCmd_e defaultPktCmd)
{
    XP_STATUS           xpStatus    = XP_NO_ERR;
    xpsHashIndexList_t  indexList   = { 0 };
    xpsPktCmd_e         portPktCmd;
    xpCoppEntryData_t   coppEntry;
    uint32_t             maxPortNum  = 0;
    uint32_t srcPort = 0;

    XP_SAI_LOG_DBG("Calling %s: policer %d, command %d, reason code %d\n",
                   __FUNCNAME__, policerId, pktCmd, reasonCode);
    /*
      ARP_REPLY FWD should be dropped as it is invlalid to flood known UC pkt.
      FDB rule will trap it to CPU.
      Over-ride the pktCmd to drop, so that it will be dropped by CPU code table.
      Handle the same for different protocols.
    */

    switch ((xpsCoppCtrlPkt_e)reasonCode)
    {
        case XPS_COPP_CTRL_PKT_DHCP_L2:
        case XPS_COPP_CTRL_PKT_DHCP:
        case XPS_COPP_CTRL_PKT_DHCPV6:
        case XPS_COPP_CTRL_PKT_OSPF:
        case XPS_COPP_CTRL_PKT_OSPFV6:
        case XPS_COPP_CTRL_PKT_PIM:
        case XPS_COPP_CTRL_PKT_VRRP:
        case XPS_COPP_CTRL_PKT_VRRPV6:
        case XPS_COPP_CTRL_PKT_IGMP_TYPE_LEAVE:
        case XPS_COPP_CTRL_PKT_IGMP_TYPE_QUERY:
        case XPS_COPP_CTRL_PKT_IGMP_TYPE_V1_REPORT:
        case XPS_COPP_CTRL_PKT_IGMP_TYPE_V2_REPORT:
        case XPS_COPP_CTRL_PKT_IGMP_TYPE_V3_REPORT:
        case XPS_COPP_CTRL_PKT_ARP_RESPONSE:
            if (pktCmd == XP_PKTCMD_FWD)
            {
                pktCmd = XP_PKTCMD_DROP;
            }
            break;
        default:
            break;
    }

    portPktCmd  = pktCmd;
    xpStatus = xpsPortGetMaxNum(xpsDevId, &maxPortNum);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not get maximum ports number, xpStatus %d\n", xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    // Source Port not applicable for Copp Rules . Only Applicable for Bum Policer
    maxPortNum = 0x1;
    memset(&coppEntry, 0, sizeof(xpCoppEntryData_t));
    coppEntry.enPolicer = (XPSAI_INVALID_POLICER_ID != policerId);
    coppEntry.policerId = policerId;
    coppEntry.updateReasonCode = false;
    coppEntry.reasonCode = reasonCode;

    XPS_GLOBAL_PORT_ITER(srcPort, maxPortNum)
    {
        if (validPort != NULL)
        {
            portPktCmd = validPort[srcPort] ? pktCmd : defaultPktCmd;
        }
        coppEntry.updatePktCmd = (XP_PKTCMD_MAX != portPktCmd);
        coppEntry.pktCmd = (XP_PKTCMD_MAX != portPktCmd) ? portPktCmd : XP_PKTCMD_DROP;
        xpStatus = xpsCoppAddEntry(xpsDevId, srcPort, reasonCode, coppEntry,
                                   &indexList);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Could not add copp entry for reason code %d, error %d",
                           reasonCode, xpStatus);
            continue;
        }
    }

    return SAI_STATUS_SUCCESS;
}

/**
 * \brief CoPP table entry remove
 *
 * This API removes CoPP table entries with @reasonCode. Single API call
 * removes exactly maxPortNum entries - one entry for each HW port.
 *
 * \param[in] xpsDevice_t xpsDevId
 * \param[in] uint32_t reasonCode
 *
 * \return sai_status_t
 */
sai_status_t xpSaiCoppEntryRemove(xpsDevice_t xpsDevId, uint32_t reasonCode)
{
    XP_STATUS xpStatus = XP_NO_ERR;
    uint32_t maxPortNum = 0;
    uint32_t srcPort = 0;

    XP_SAI_LOG_DBG("Remove copp entry: reason code %d\n", reasonCode);

    xpStatus = xpsPortGetMaxNum(xpsDevId, &maxPortNum);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not get maximum ports number, xpStatus %d\n", xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }
    // Source Port not applicable for Copp Rules . Only Applicable for Bum Policer
    maxPortNum = 0x1;

    XPS_GLOBAL_PORT_ITER(srcPort, maxPortNum)
    {
        xpStatus = xpsCoppRemoveEntry(xpsDevId, srcPort, reasonCode);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to remove copp entry : port %d, reason code %d, error %d",
                           srcPort, reasonCode, xpStatus);
            continue;
        }
    }

    return SAI_STATUS_SUCCESS;
}

/**
 * \brief CoPP table entry get on scpecific port
 *
 * This API retrieves CoPP table entry with @reasonCode. Single API call
 * gets one entry, which was applied earlier on HW port.
 *
 * \param[in] xpsDevice_t xpsDevId
 * \param[in] uint32_t reasonCode
 * \param[in] uint8_t xpPortIdx
 *
 * \return sai_status_t
 */
sai_status_t xpSaiCoppEntryGetPolicerIdFromPort(xpsDevice_t xpsDevId,
                                                uint32_t reasonCode, uint32_t xpPortIdx, uint32_t *policerId)
{
    XP_STATUS xpStatus = XP_NO_ERR;
    xpCoppEntryData_t coppEntry;

    if (policerId == NULL)
    {
        XP_SAI_LOG_ERR("NULL ptr provided!\n");
        return SAI_STATUS_FAILURE;
    }

    memset(&coppEntry, 0, sizeof(coppEntry));
    // Source Port not applicable for Copp Rules . Only Applicable for Bum Policer
    xpPortIdx = 0x0;

    xpStatus = xpsCoppGetEntry(xpsDevId, xpPortIdx, reasonCode, &coppEntry);
    if (xpStatus != XP_NO_ERR)
    {
        return xpsStatus2SaiStatus(xpStatus);
    }

    *policerId = coppEntry.policerId;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiCoppInit

sai_status_t xpSaiCoppInit(xpsDevice_t xpsDevId)
{
    XP_STATUS           xpStatus    = XP_NO_ERR;
    sai_status_t        saiStatus   = SAI_STATUS_SUCCESS;
    static const int    clearOnRead = 0;
    static const int    wrapAround  = 0;
    static const int    countOffset = 15;

    xpStatus = xpsSetCountMode(xpsDevId, XP_ACM_COPP_POLICER, XP_ACM_POLICING,
                               XP_ANA_BANK_MODE_P, clearOnRead, wrapAround, countOffset);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not set count mode, error %d\n", xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    saiStatus = xpSaiCoppBumPolicerSet(xpsDevId, true);
    if (saiStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not enable BUM policer on the ports, error %d\n",
                       saiStatus);
        return saiStatus;
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiCoppDeInit

sai_status_t xpSaiCoppDeInit(xpsDevice_t xpsDevId)
{
    XP_STATUS       xpStatus    = XP_NO_ERR;
    sai_status_t    saiStatus   = SAI_STATUS_SUCCESS;

    saiStatus = xpSaiCoppBumPolicerSet(xpsDevId, false);
    if (saiStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not disable BUM policer on the ports, error %d",
                       saiStatus);
        return saiStatus;
    }

    xpStatus = xpsSetCountMode(xpsDevId, XP_ACM_COPP_POLICER, XP_ACM_NONE, 0, 0, 0,
                               0);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not set count mode, error %d", xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    return SAI_STATUS_SUCCESS;
}
