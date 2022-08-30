/* xpSaiHostIfGenetlink.c TODO This file is Linux Specific. Should use SAL APIs.*/

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include <sys/socket.h>
#include <net/if_arp.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <linux/genetlink.h>
#include "xpSaiHostInterface.h"
#include "xpsPacketDrv.h"


XP_SAI_LOG_REGISTER_API(SAI_API_HOSTIF);

sai_status_t xpSaiGenetlinkObjCreate(uint8_t *genetlinkName,
                                     uint8_t *mcgrpName, uint32_t *pFd,
                                     uint16_t *familyId, uint32_t *gid)
{
    int fd  = 0;
    int err = 0;
    struct sockaddr_nl recvSock= {0};

    if (pFd == NULL)
    {
        XP_SAI_LOG_ERR("A null pointer\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
    if (fd < 0)
    {
        XP_SAI_LOG_ERR("Error: socket create failed\n");
        return SAI_STATUS_FAILURE;
    }

    recvSock.nl_family = AF_NETLINK;
    recvSock.nl_pid    = 0;
    recvSock.nl_groups = 0;

    if ((err = bind(fd, (struct sockaddr *)&recvSock, sizeof(recvSock))) < 0)
    {
        XP_SAI_LOG_ERR("Error: bind failed \n");
        close(fd);
        return SAI_STATUS_FAILURE;
    }

    int ret;
    uint32_t addr_len = sizeof(recvSock);
    ret = getsockname(fd, (struct sockaddr *)&recvSock, &addr_len);
    if ((ret < 0) || (addr_len != sizeof(recvSock))
        || (recvSock.nl_family != AF_NETLINK))
    {
        XP_SAI_LOG_ERR("Error: socket is invalid \n");
        close(fd);
        return SAI_STATUS_FAILURE;
    }

    XP_SAI_LOG_DBG("DBG:: Created Genetlink: %d pid %d\n", fd, recvSock.nl_pid);

    /* Set the fds as non-blocking. Select can be used if reqd. */
    if (0 != fcntl(fd, F_SETFL, O_NONBLOCK))
    {
        XP_SAI_LOG_ERR("Error: file descriptor manipulation failed \n");
        close(fd);
        return SAI_STATUS_FAILURE;
    }

    if ((xpsHostIfGenetlinkGetFamilyId(genetlinkName, fd, familyId, mcgrpName,
                                       gid)) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Get family %s failed \n", genetlinkName);
        close(fd);
        return SAI_STATUS_FAILURE;
    }

    if ((*familyId == 0) || (*gid == 0))
    {
        XP_SAI_LOG_ERR("Error: Get family %s mcgrp %s failed \n", genetlinkName,
                       mcgrpName);
        close(fd);
        return SAI_STATUS_FAILURE;
    }

    XP_SAI_LOG_DBG("DBG:: Get Family %d gid %d\n", *familyId, *gid);

    if ((err = setsockopt(fd, SOL_NETLINK, NETLINK_ADD_MEMBERSHIP, gid,
                          sizeof(*gid))) < 0)
    {
        XP_SAI_LOG_ERR("Error: Add member group %d failed \n", *gid);
        close(fd);
        return SAI_STATUS_FAILURE;
    }

    *pFd = fd;

    XP_SAI_LOG_DBG("DBG:: Created Genetlink: %d\n", fd);

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiGenetlinkObjDelete(uint32_t fd)
{
    close(fd);

    return SAI_STATUS_SUCCESS;
}

