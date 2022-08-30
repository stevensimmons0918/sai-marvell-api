/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/**
********************************************************************************
* @file cpssHalUtilOsNetworkStack.c
*
* @brief CPSS <=> OS TCP/IP stack
* Linux BM only now
* @ref
*   1.osNetworkStack.c --- cpss sdk extUtils library
*   2.prvOsNetworkStackDeviceOpen.c --- cpss sdk extUtils library
* @version   1
********************************************************************************
*/
#include <sys/ioctl.h>
#include <net/if.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "cpssHalUtilOsNetworkStack.h"
#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <extUtils/rxEventHandler/rxEventHandler.h>
#include <extUtils/iterators/extUtilDevPort.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#ifdef CHX_FAMILY
#  include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgFdb.h>
#  include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgVlan.h>
#  include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIfTypes.h>
#  include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#endif

static GT_BOOL  nsInitialized = GT_FALSE;
static GT_BOOL  nsConnected = GT_FALSE;
CPSS_OS_MUTEX nsMtx = (CPSS_OS_MUTEX)0;
/* os depended */
GT_STATUS cpssHalOsNetworkStackDeviceOpen(char* devName, int32_t* tnFd);
GT_STATUS cpssHalOsNetworkStackDeviceClose(int32_t tnFd);
GT_STATUS cpssHalOsNetworkStackDeviceRead(
    int32_t tnFd,
    GT_U8  *pktBuf,
    INOUT GT_U32 *pktLen
);
GT_STATUS cpssHalOsNetworkStackDeviceWrite(
    int32_t tnFd,
    GT_U8   *pktBuf,
    GT_U32  pktLen
);

GT_STATUS cpssHalOsNetworkStackMacGet(
    int32_t tnFd,
    GT_ETHERADDR *etherPtr
);
GT_STATUS cpssHalOsNetworkStackMacSet(
    GT_ETHERADDR *etherPtr,
    int32_t tnFd,
    char* tnName
);
GT_STATUS cpssHalOsNetworkStackIfconfig(
    const char *config,
    char* tnName
);

/**
* @internal cpssHalUtilOsNetworkStackInit function
* @endinternal
*
* @brief   Initialize library
*/
GT_STATUS cpssHalUtilOsNetworkStackInit
(
    GT_U8 devId,
    RX_EV_PACKET_RECEIVE_CB_FUN    rxHandlerCb
)
{
    char nm[]="nsMtx";
    GT_STATUS rc;
    if (nsInitialized == GT_TRUE)
    {
        return GT_OK;
    }
    rc = cpssOsMutexCreate(nm, &nsMtx);
    if (rc != GT_OK)
    {
        return rc;
    }

    nsInitialized = GT_TRUE;
    rc = rxEventHandlerAddCallback(
             devId, RX_EV_HANDLER_QUEUE_ALL,
             RX_EV_HANDLER_TYPE_RX_E,
             rxHandlerCb, 0);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

GT_STATUS cpssHalUtilOsNetworkStackDeInit
(
    GT_U8 devId,
    RX_EV_PACKET_RECEIVE_CB_FUN    rxHandlerCb
)
{
    GT_STATUS rc;
    if (nsInitialized == GT_FALSE)
    {
        return GT_OK;
    }
    rc = cpssOsMutexDelete(nsMtx);
    if (rc != GT_OK)
    {
        return rc;
    }

    nsInitialized = GT_FALSE;
    rc = rxEventHandlerRemoveCallback(
             devId, RX_EV_HANDLER_QUEUE_ALL,
             RX_EV_HANDLER_TYPE_RX_E,
             rxHandlerCb, 0);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

GT_STATUS cpssHalUtilOsNetworkStackDeviceWrite
(
    int32_t tnFd,
    GT_U8   *pktBuf,
    GT_U32  pktLen
)
{
    NSLOCK();
    GT_STATUS rc;
    rc = cpssHalOsNetworkStackDeviceWrite(tnFd, pktBuf, pktLen);
    NSUNLOCK();
    return rc;
}

GT_STATUS cpssHalUtilOsNetworkStackDeviceRead
(
    int32_t tnFd,
    GT_U8   *pktBuf,
    IN   GT_U32  *pktLen
)
{
    NSLOCK();
    GT_STATUS rc;
    rc = cpssHalOsNetworkStackDeviceRead(tnFd, pktBuf, pktLen);
    NSUNLOCK();
    return rc;
}

/**
* @internal cpssHalUtilOsNetworkStackDisconnect function
* @endinternal
*
* @brief   Disconnect PP from OS network stack
*/
GT_STATUS cpssHalUtilOsNetworkStackDisconnect
(
    GT_U8   devId,
    int32_t netdevFd
)
{
    NSLOCK();
    cpssHalOsNetworkStackDeviceClose(netdevFd);

    NSUNLOCK();
    return GT_OK;
}

/**
* @internal cpssHalUtilOsNetworkStackMacGet function
* @endinternal
*
* @brief   Query for MAC address of OS network interface
*/
GT_STATUS cpssHalUtilOsNetworkStackMacGet
(
    int32_t fd,
    GT_ETHERADDR *etherPtr
)
{
    GT_STATUS rc;
    if (nsInitialized != GT_TRUE)
    {
        return GT_NOT_INITIALIZED;
    }
    NSLOCK();
    rc = cpssHalOsNetworkStackMacGet(fd, etherPtr);
    if (rc != GT_OK)
    {
        NSUNLOCK();
        return rc;
    }
    NSUNLOCK();
    return rc;
}

/**
* @internal cpssHalUtilOsNetworkStackMacSet function
* @endinternal
*
* @brief   Change MAC address of OS network interface, update FDB
*/
GT_STATUS cpssHalUtilOsNetworkStackMacSet
(
    GT_ETHERADDR *etherPtr,
    int32_t fd,
    char* tnName
)
{
    GT_STATUS rc;
    GT_ETHERADDR addr;
    if (nsInitialized != GT_TRUE)
    {
        return GT_NOT_INITIALIZED;
    }
    NSLOCK();
    if (!nsConnected)
    {
        NSUNLOCK();
        return GT_NOT_INITIALIZED;
    }
    rc = cpssHalOsNetworkStackMacSet(etherPtr, fd, tnName);
    if (rc == GT_OK)
    {
        rc = cpssHalUtilOsNetworkStackMacGet(fd, &addr);
    }
    NSUNLOCK();
    return rc;
}

/**
* @internal cpssHalUtilOsNetworkStackConnect function
* @endinternal
*
* @brief   Connect device/vlan to OS network stack (unicast and broadcast)
*         Only one OS network interface can be confugured
* @param[in] devId                    - device ID
* @param[in] netDevName               - net device name
* @param[out] netDevFd                - netDevFd
*                                       GT_STATUS
*/
GT_STATUS cpssHalUtilOsNetworkStackConnect
(
    GT_U8        devId,
    GT_U8        *netDevName,
    GT_32        *netDevFd
)
{
    GT_STATUS rc;
    NSLOCK();
    if (!PRV_CPSS_IS_DEV_EXISTS_MAC(devId))
    {
        NSUNLOCK();
        return GT_BAD_PARAM;
    }

    /* single pass loop for error handling */
    /* open tunnel ,set admin up and mtu 1500*/
    rc = cpssHalOsNetworkStackDeviceOpen((char *)netDevName, netDevFd);

    if (rc != GT_OK)
    {
        cpssHalOsNetworkStackDeviceClose(*netDevFd);
        NSUNLOCK();
        return rc;
    }
    else
    {
        //Print netdev create success fd , mac and name.
    }
    NSUNLOCK();
    return rc;
}

/**
* @internal cpssHalUtilOsNetworkStackIfconfig function
* @endinternal
*
* @brief   Configure OS network interface
*
* @param[in] config                   - Configuration string
*                                       GT_STATUS
*
* @note Example:
*       cpssHalUtilOsNetworkStackIfconfig("up inet 10.1.2.3 netmask 255.255.255.0")
*
*/
GT_STATUS cpssHalUtilOsNetworkStackIfconfig(
    const char *config,
    char* tnName
)
{
    GT_STATUS rc;
    if (nsInitialized != GT_TRUE)
    {
        return GT_NOT_INITIALIZED;
    }
    NSLOCK();
    rc = cpssHalOsNetworkStackIfconfig(config, tnName);
    NSUNLOCK();
    return rc;
}

GT_STATUS cpssHalUtilOsNetworkStackLinkUp
(
    char* tnName
)
{
    GT_STATUS rc;
    if (nsInitialized != GT_TRUE)
    {
        return GT_NOT_INITIALIZED;
    }
    NSLOCK();
    rc = cpssHalOsNetworkStackIfconfig("up", tnName);
    NSUNLOCK();
    return rc;
}


GT_STATUS cpssHalUtilOsNetworkSetLinkState(char* ifName, int isLinkUp)
{
    struct ifreq ifr;
    int sock;

    if (ifName == NULL)
    {
        return GT_BAD_PARAM;
    }

    memcpy(ifr.ifr_name, ifName, IFNAMSIZ);
    sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock == -1)
    {
        cpssOsPrintf("fail to create socket. errno=%d, %s\n", errno, strerror(errno));
        return GT_NOT_INITIALIZED;
    }
    if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0)
    {
        cpssOsPrintf("failed to get link state for interface %s. errno=%d %s\n",
                     ifName, errno, strerror(errno));
        close(sock);
        return GT_NOT_INITIALIZED;
    }

    memcpy(ifr.ifr_name, ifName, IFNAMSIZ);
    if (isLinkUp)
    {
        ifr.ifr_flags |= IFF_UP;
    }
    else
    {
        ifr.ifr_flags &= ~IFF_UP;
    }
    if (ioctl(sock, SIOCSIFFLAGS, &ifr) < 0)
    {
        cpssOsPrintf("failed to set link state for interface %s. errno=%d %s\n",
                     ifName, errno, strerror(errno));
        close(sock);
        return GT_NOT_INITIALIZED;
    }

    close(sock);

    return GT_OK;
}

GT_STATUS cpssHalUtilOsNetworkGetLinkState(char* ifName, int *isLinkUp)
{
    struct ifreq ifr;
    int sock;

    if (ifName == NULL)
    {
        return GT_BAD_PARAM;
    }

    memcpy(ifr.ifr_name, ifName, IFNAMSIZ);
    sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock == -1)
    {
        cpssOsPrintf("fail to create socket. errno=%d, %s\n", errno, strerror(errno));
        return GT_NOT_INITIALIZED;
    }
    if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0)
    {
        cpssOsPrintf("failed to get link state for interface %s. errno=%d %s\n",
                     ifName, errno, strerror(errno));
        close(sock);
        return GT_NOT_INITIALIZED;
    }

    if (ifr.ifr_flags & IFF_UP)
    {
        *isLinkUp = 1;
    }
    else
    {
        *isLinkUp = 0;
    }

    close(sock);

    return GT_OK;
}
