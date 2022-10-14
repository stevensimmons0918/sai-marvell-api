/* xpSaiHostIfNetdev.c TODO This file is Linux Specific. Should use SAL APIs.*/

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#ifndef MAC
#include <sys/socket.h>
#include <net/if_arp.h>
#include <linux/netlink.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include "xpSaiHostInterface.h"
#include "xpsPacketDrv.h"

extern XP_STATUS xpsPacketDriverUpdateMacNetDevDb(uint32_t xpnetId);
XP_SAI_LOG_REGISTER_API(SAI_API_HOSTIF);

xp_nl_hostif_trap_channel_t
xpSaiTrapChannelToXpnet(sai_hostif_table_entry_channel_type_t trapChannel)
{
    switch (trapChannel)
    {
        case SAI_HOSTIF_TABLE_ENTRY_CHANNEL_TYPE_CB:
            return XP_NL_HOSTIF_TRAP_CB;
        case SAI_HOSTIF_TABLE_ENTRY_CHANNEL_TYPE_FD:
            return XP_NL_HOSTIF_TRAP_FD;
        case SAI_HOSTIF_TABLE_ENTRY_CHANNEL_TYPE_NETDEV_PHYSICAL_PORT:
        case SAI_HOSTIF_TABLE_ENTRY_CHANNEL_TYPE_NETDEV_LOGICAL_PORT:
        case SAI_HOSTIF_TABLE_ENTRY_CHANNEL_TYPE_NETDEV_L3:
            return XP_NL_HOSTIF_TRAP_NETDEV;
        default:
            return XP_NL_HOSTIF_TRAP_CB;
    }
}

XP_STATUS xpSaiHostIfNetdevCreate(xpsDevice_t devId, xpsInterfaceId_t intfId,
                                  uint8_t *intfName)
{
    uint32_t xpnetId = intfId;
    xpsInterfaceType_e ifType = XPS_PORT;
    XP_STATUS rc = XP_NO_ERR;
    sai_status_t        saiRetVal   = SAI_STATUS_FAILURE;

    rc = xpsNetdevXpnetIdAllocateWith(devId, xpnetId);
    if (XP_NO_ERR != rc)
    {
        XP_SAI_LOG_ERR("Unable to allocate xpnetId for interface: %u, %s\n", intfId,
                       intfName);
        return rc;
    }
    rc = xpsNetdevIfCreate(devId, xpnetId, (char *)intfName);
    if (XP_NO_ERR != rc)
    {
        XP_SAI_LOG_ERR("Unable to create interface: %u, %s\n", intfId, intfName);
        return rc;
    }

    rc = xpsInterfaceGetType(intfId, &ifType);
    if (XP_NO_ERR != rc)
    {
        XP_SAI_LOG_ERR("Failed to get %u interface type\n", intfId);
        return rc;
    }

    XP_SAI_LOG_DBG("Setting MAC for intf id = %d, interface name = %s\n", intfId,
                   intfName);
    saiRetVal = xpSaiSetSwitchMac((uint8_t *)intfName);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not set MAC address, retVal %d", saiRetVal);
    }
    if (XP_NO_ERR != (rc = xpsPacketDriverUpdateMacNetDevDb(intfId)))
    {
        XP_SAI_LOG_ERR("Could not update MAC address in DB, retVal %d", rc);
    }

    return rc;
}

XP_STATUS xpSaiHostIfNetdevDelete(xpsDevice_t devId, xpsInterfaceId_t intfId)
{
    uint32_t xpnetId = intfId;
    XP_STATUS rc = XP_NO_ERR;

    rc = xpsNetdevIfDelete(devId, xpnetId);
    if (XP_NO_ERR != rc)
    {
        XP_SAI_LOG_ERR("Unable to delete interface: %u.\n", intfId);
        return rc;
    }

    rc = xpsNetdevXpnetIdFree(devId, xpnetId);
    if (XP_NO_ERR != rc)
    {
        XP_SAI_LOG_ERR("Unable to delete interface: %u.\n", intfId);
        return rc;
    }
    return rc;
}

XP_STATUS xpSaiHostIfLinkStatusSet(xpsDevice_t devId, xpsInterfaceId_t intfId,
                                   bool status)
{
    uint32_t xpnetId = intfId;
    XP_STATUS rc = XP_NO_ERR;

    if (XP_SAL_HW_TYPE != xpGetSalType())
    {
        return XP_NO_ERR;
    }

    rc = xpsNetdevLinkStatusSet(devId, xpnetId, status);
    if (XP_NO_ERR != rc)
    {
#if 0
        XP_SAI_LOG_WARNING("Unable to set operational status for interface: %u.\n",
                           intfId);
#endif
    }

    return rc;
}

XP_STATUS xpSaiHostIfVlanTagTypeSet(xpsDevice_t devId, xpsInterfaceId_t intfId,
                                    uint32_t tagType)
{
    uint32_t xpnetId = intfId;
    XP_STATUS rc = XP_NO_ERR;

    if (XP_SAL_HW_TYPE != xpGetSalType())
    {
        return XP_NO_ERR;
    }
    rc = xpsPacketDriverNetdevVlanTagTypeSet(devId, xpnetId,
                                             (xpsPacketDrvHostifVlanTagType)tagType);
    if (XP_NO_ERR != rc)
    {
        XP_SAI_LOG_ERR("Unable to set vlan tag type for interface: %u.\n", intfId);
    }
    return rc;
}

sai_status_t xpSaiHostIfTxHeaderGet(xpsDevice_t devId, xpVif_t egressVif,
                                    bool sendToEgress, xphTxHdr *txHeader)
{
    sai_status_t        retVal          = SAI_STATUS_SUCCESS;
    xpVif_t             ingressVif      = 0;
    xpsInterfaceId_t    cpuIfId         = XPS_INTF_INVALID_ID;

    if (txHeader == NULL)
    {
        XP_SAI_LOG_ERR("A null pointer\n");
        return XP_ERR_NULL_POINTER;
    }

    retVal = xpsPortGetCPUPortIntfId(devId, &cpuIfId);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get CPU interface ID\n");
        return retVal;
    }

    ingressVif = XPS_INTF_MAP_INTFID_TO_VIF(cpuIfId);

    memset(txHeader, 0, sizeof(xphTxHdr));
    txHeader->ingressVifLsbByte2 = (ingressVif >> 12) & 0xFF;
    txHeader->ingressVifLsbByte1 = (ingressVif >> 4) & 0xFF;
    txHeader->ingressVifLsbByte0 = ingressVif & 0xF;

    txHeader->egressVifLsbByte2 = (egressVif >> 16) & 0xF;
    txHeader->egressVifLsbByte1 = (egressVif >> 8) & 0xFF;
    txHeader->egressVifLsbByte0 = egressVif & 0xFF;

    if (sendToEgress)
    {
        /* Bypass XP80 datapath to URW block */
        txHeader->nextEngine = XP_ENGINE_URW;
    }
    else
    {
        /* Send to the beginning of XP80 datapath */
        txHeader->nextEngine = 0;
        txHeader->metadata.setIngressVif = 1;
        txHeader->metadata.macSAmissCmd = 1;
    }

    return retVal;
}

/* Encode the trap config message and send it to kernel */
XP_STATUS xpSaiHostIfNetdevTrapSet(xpsDevice_t devId, xpTrapConfig *trapConf,
                                   bool create)
{
    XP_STATUS rc = XP_NO_ERR;

    if (SAI_HOSTIF_TABLE_ENTRY_TYPE_WILDCARD == trapConf->intfType)
    {
        rc = xpsNetdevDefaultTrapSet(devId, create);
    }
    else
    {
        rc = xpsNetdevTrapSet(devId, trapConf->trapId, trapConf->reasonCode,
                              xpSaiTrapChannelToXpnet(trapConf->channel), trapConf->fd, create);
    }

    if (XP_NO_ERR != rc)
    {
        XP_SAI_LOG_ERR("Unable to set trap config.\n");
    }

    return rc;
}

//Func: xpSaiSetSwitchMac

sai_status_t xpSaiSetSwitchMac(uint8_t* ifName)
{
    struct ifreq ifr;
    int sock;
    sai_mac_t switchMac;
    sai_mac_t zero = {0, 0, 0, 0, 0, 0};

    if (ifName == NULL)
    {
        XP_SAI_LOG_ERR("A null pointer\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpSaiGetSwitchSrcMacAddress(switchMac);
    if (0 == memcmp(switchMac, zero, sizeof(sai_mac_t)))
    {
        XP_SAI_LOG_NOTICE("Grobal variable switch src mac is not set \n");
        return SAI_STATUS_SUCCESS;
    }
    memcpy(ifr.ifr_ifrn.ifrn_name, (const char*) ifName,
           sizeof(ifr.ifr_ifrn.ifrn_name));
    sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock == -1)
    {
        XP_SAI_LOG_ERR("fail to create socket. errno=%d, %s\n", errno, strerror(errno));
        return SAI_STATUS_FAILURE;
    }

    ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;
    memcpy(ifr.ifr_hwaddr.sa_data, switchMac, 6);

    if (ioctl(sock, SIOCSIFHWADDR, &ifr) < 0)
    {
        XP_SAI_LOG_ERR("fail to set interface mac Addr for interface %s. errno=%d %s\n",
                       ifr.ifr_ifrn.ifrn_name, errno, strerror(errno));
        close(sock);
        return SAI_STATUS_FAILURE;
    }

    close(sock);

    return SAI_STATUS_SUCCESS;
}
#endif
