/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/**
********************************************************************************
* @file cpssHalOsNetworkStackLinux.c
*
* @brief OS TCP/IP stack API
* Linux BM implementation
*
* @version   2
********************************************************************************
*/

#ifndef LINUX
# error "Linux only"
#endif
#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_arp.h>
#include <linux/if_tun.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <cpss/common/cpssTypes.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include "cpssHalUtilOsNetworkStack.h"
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
/* interface file descriptor */
//static int tnFd = -1;
/* interface name */
//static char tnName[IFNAMSIZ];

/* forward declaration */
GT_STATUS cpssHalOsNetworkStackIfconfig(
    const char *config,
    char* tnName
);

/**
* @internal cpssHalOsNetworkStackDeviceOpen function
* @endinternal
*
* @brief   Open and initialize OS network device (usually tap0)
*/
GT_STATUS cpssHalOsNetworkStackDeviceOpen(char* devName, int32_t* tnFd)
{
    struct ifreq ifr;
    if (*tnFd >= 0)
    {
        return GT_OK;
    }

    /* open the clone device */
    if ((*tnFd = open("/dev/net/tun", O_RDWR)) < 0)
    {
        cpssOsPrintf("cpssHalOsNetworkStackDeviceOpen(): "
                     "Failed to open /dev/net/tun: %s\r\n",
                     strerror(errno));
        return GT_FAIL;
    }

    /* preparation of the struct ifr, of type "struct ifreq" */
    memset(&ifr, 0, sizeof(ifr));

    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
    /* the kernel will try to allocate the "next" device of the TAP type */
    if (*devName)
    {
        memcpy(ifr.ifr_name, devName, IFNAMSIZ);
    }
    /* try to create the device */
    if (ioctl(*tnFd, TUNSETIFF, (void *) &ifr) < 0)
    {
        cpssOsPrintf("cpssHalOsNetworkStackDeviceOpen(): "
                     "Failed to clone TAP device (TUNSETIFF) %s: %s\r\n",
                     devName, strerror(errno));
        close(*tnFd);
        *tnFd = -1;
        return GT_FAIL;
    }
    /* if the operation was successful, write back the name of the
     * interface to the variable tnName, so we will know it
     */
    strncpy(devName, ifr.ifr_name, IFNAMSIZ);

    cpssOsPrintf("cpssHalOsNetworkStackDeviceOpen: Creating tuntap %s fd %d\n",
                 devName, *tnFd);

    cpssHalOsNetworkStackIfconfig("up mtu 1500", devName);
    return GT_OK;
}

/**
* @internal cpssHalOsNetworkStackDeviceClose function
* @endinternal
*
* @brief   Close OS network device (usually tap0)
*/
GT_STATUS cpssHalOsNetworkStackDeviceClose(int32_t tnFd)
{
    if (tnFd < 0)
    {
        return GT_OK;
    }
    //cpssHalOsNetworkStackIfconfig("down");
    close(tnFd);
    tnFd = -1;
    return GT_OK;
}

/**
* @internal cpssHalOsNetworkStackDeviceRead function
* @endinternal
*
* @brief   Read packet from OS netwrok stack
*
* @param[in,out] pktLen                   - length of pktBuf
*
* @param[out] pktBuf                   - pointer for packet data
* @param[in,out] pktLen                   - actual length of packet data
*                                       GT_STATUS
*/
GT_STATUS cpssHalOsNetworkStackDeviceRead(
    int32_t tnFd,
    GT_U8  *pktBuf,
    INOUT GT_U32 *pktLen
)
{
    int nread;
    if (tnFd < 0)
    {
        return GT_NOT_INITIALIZED;
    }
    while (1)
    {
        nread = read(tnFd, pktBuf, (size_t)(*pktLen));
        if (nread > 0)
        {
            break;
        }
        if (errno != EINTR)
        {
            return GT_FAIL;
        }
    }
    *pktLen = (GT_U32)nread;
    return GT_OK;
}

/**
* @internal cpssHalOsNetworkStackDeviceWrite function
* @endinternal
*
* @brief   Write packet to OS netwrok stack
*
* @param[in] pktBuf                   - pointer for packet data
* @param[in] pktLen                   - length of pktBuf
*                                       GT_STATUS
*/
GT_STATUS cpssHalOsNetworkStackDeviceWrite(
    int32_t tnFd,
    GT_U8   *pktBuf,
    GT_U32  pktLen
)
{
    int nwrite;
    if (tnFd < 0)
    {
        return GT_NOT_INITIALIZED;
    }
    nwrite = write(tnFd, pktBuf, (size_t)pktLen);
    return (nwrite < 0) ? GT_FAIL : GT_OK;
}

/**
* @internal cpssHalOsNetworkStackMacGet function
* @endinternal
*
* @brief   Get mac address of OS network device (usually tap0)
*
* @param[out] etherPtr                 - pointer to store mac address
*                                       GT_STATUS
*/
GT_STATUS cpssHalOsNetworkStackMacGet(
    int32_t tnFd,
    GT_ETHERADDR *etherPtr
)
{
    struct ifreq ifr;

    if (tnFd < 0)
    {
        return GT_NOT_INITIALIZED;
    }

    memset(&ifr, 0, sizeof(ifr));
    if (ioctl(tnFd, SIOCGIFHWADDR, (void *) &ifr) < 0)
    {
        return GT_FAIL;
    }
    memcpy(etherPtr->arEther, ifr.ifr_hwaddr.sa_data, 6);
    return GT_OK;
}

/**
* @internal cpssHalOsNetworkStackMacSet function
* @endinternal
*
* @brief   Get mac address of OS network device (usually tap0)
*
* @param[in] etherPtr                 - mac address to set
*                                       GT_STATUS
*/
GT_STATUS cpssHalOsNetworkStackMacSet(
    GT_ETHERADDR *etherPtr,
    int32_t tnFd,
    char* tnName
)
{
    struct ifreq ifr;

    if (tnFd < 0)
    {
        return GT_NOT_INITIALIZED;
    }

    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, tnName);
    memcpy(ifr.ifr_hwaddr.sa_data, etherPtr->arEther, 6);
    ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;

    if (ioctl(tnFd, SIOCSIFHWADDR, (void *) &ifr) < 0)
    {
        return GT_FAIL;
    }
    return GT_OK;

}

/**
* @internal cpssHalOsNetworkStackIfconfig function
* @endinternal
*
* @brief   Configure OS network interface
*
* @param[in] config                   - Configuration string
*                                       GT_STATUS
*
* @note Example:
*       cpssHalOsNetworkStackIfconfig("up inet 10.1.2.3 netmask 255.255.255.0")
*
*/
GT_STATUS cpssHalOsNetworkStackIfconfig(
    const char *config,
    char* tnName
)
{
    char cmd[BUFSIZ];

    if (BUFSIZ <= (strlen("ifconfig ") + strlen(tnName) + strlen(config)))
    {
        return GT_BAD_PARAM;
    }
    cpssOsSprintf(cmd, "ifconfig %s %s", tnName, config);
#define ignore_unused_result(_cmd) { int rc = _cmd; (void)sizeof rc; }
    ignore_unused_result(system(cmd));
    return GT_OK;
}


