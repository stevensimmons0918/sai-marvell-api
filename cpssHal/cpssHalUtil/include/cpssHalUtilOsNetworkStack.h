/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/**
********************************************************************************
* @file cpssHalUtilOsNetworkStack.h
*
* @brief CPSS <=> OS TCP/IP stack
*
* @version   1
********************************************************************************
*/
#ifndef __cpssHalUtilOsNetworkStack_h__
#define __cpssHalUtilOsNetworkStack_h__

#include <cpss/common/cpssTypes.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <extUtils/rxEventHandler/rxEventHandler.h>
#include <extUtils/iterators/extUtilDevPort.h>

extern CPSS_OS_MUTEX nsMtx;
#define NSLOCK() cpssOsMutexLock(nsMtx)
#define NSUNLOCK() cpssOsMutexUnlock(nsMtx)


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
);

/**
* @internal cpssHalUtilOsNetworkStackDeInit function
* @endinternal
*
* @brief   De Initialize library
*
* @param[in] devId
* @param[in] rxHandlerCb
*/
GT_STATUS cpssHalUtilOsNetworkStackDeInit
(
    GT_U8 devId,
    RX_EV_PACKET_RECEIVE_CB_FUN    rxHandlerCb
);

/**
* @internal cpssHalUtilOsNetworkStackConnect function
* @endinternal
*
* @brief   Connect device/vlan to OS network stack (unicast and broadcast)
*         Only one OS network interface can be confugured
* @param[in] devId                    - device ID
* @param[in] netDevName               - netdevice name
* @param[out] netDevFd
*                                       GT_STATUS
*/
GT_STATUS cpssHalUtilOsNetworkStackConnect(
    GT_U8         devId,
    GT_U8         *netDevName,
    GT_32        *netDevFd
);

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
);

/**
* @internal cpssHalUtilOsNetworkStackMacSet function
* @endinternal
*
* @brief   Change MAC address of OS network interface, update FDB
*/
GT_STATUS cpssHalUtilOsNetworkStackMacSet(
    GT_ETHERADDR *etherPtr,
    int32_t fd,
    char* tnName
);

/**
* @internal cpssHalUtilOsNetworkStackMacGet function
* @endinternal
*
* @brief   Query for MAC address of OS network interface
*/
GT_STATUS cpssHalUtilOsNetworkStackMacGet(
    int32_t  fd,
    GT_ETHERADDR *etherPtr
);

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
GT_STATUS cpssHalUtilOsNetworkStackIfconfig
(
    const char *config,
    char* tnName
);

GT_STATUS cpssHalUtilOsNetworkStackDeviceWrite
(
    int32_t tnFd,
    GT_U8   *pktBuf,
    GT_U32  pktLen
);

GT_STATUS cpssHalUtilOsNetworkStackDeviceRead
(
    int32_t tnFd,
    GT_U8  *pktBuf,
    INOUT GT_U32 *pktLen
);

GT_STATUS cpssHalUtilOsNetworkStackLinkUp
(
    char* tnName
);

GT_STATUS cpssHalUtilOsNetworkSetLinkState(char* ifName, int isLinkUp);
GT_STATUS cpssHalUtilOsNetworkGetLinkState(char* ifName, int *isLinkUp);

#endif /* __cpssHalUtilOsNetworkStack_h__ */

