// xpsState.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpsCommon.h"
#include "xpsUtil.h"
#include "xpsInternal.h"
#include "xpsLink.h"
#include "xpsMac.h"
#include "xpsGlobalSwitchControl.h"
#include "openXpsTypes.h"
#include "gtEnvDep.h"
#include "cpssHalCtrlPkt.h"
#include "cpssHalDevice.h"
#include "cpssHalShell.h"

#define XPLOG_SIZE 100

static const char* xpLogLevelNames[XP_LOG_MAX_LEVEL+1] =
{
    "DEBUG",
    "TRACE",
    "DEFAULT",
    "WARNING",
    "ERROR",
    "CRITICAL",
    "UNKNOWN"
};

static int exit_parent;

xpTimeStamp_t xpTimeStamp[XP_MAX_FUNCTION];

xpDevConfigStruct devDefaultConfig =
{
    XP_TWO_PIPE,                          // pipe line no (1,2 or 4 and default mode is 1)
    XP_DEFAULT_TWO_PIPE_PROFILE,          // Select Profile
    0,                                    // performanceMode, default 128
    XP_HARDWARE,
    1,                                    // shadowNeeded
    0,                                    // shadowWm
    1,                                    // statictablesNeeded
    0,                                    // isRemote
    0,
    0,                                    // Emulation configuration mode
    1,                                    // Initialize ports by xpLinkManager
    0,                                    // Keep ports down after init by xpLinkManager
    0,                                    // Initail Port Vif value
};
#ifdef __cplusplus
extern "C" {
#endif
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
uint32_t xpsIsIPv4MulticastAddress(ipv4Addr_t addr)
{
    return ((addr[3] & 0xF0) == 0xE0);
}

uint8_t xpsIsVlanIdValid(xpsVlan_t vlanId)
{

    return (!((vlanId <= XPS_VLANID_MIN) || (vlanId > XPS_VLANID_MAX)));
}
/*
 * DLL utility
 */

void xpsDllLinkAtTheEnd(xpsDllHead *head, xpsDllLink *node)
{
    // If this is not the first node
    if (head->last != NULL)
    {
        // make the last node point to thie node
        head->last->next = node;
    }
    else
    {
        // Adding the first node
        head->first = node;
    }

    // The previous node is what last was pointing to before
    node->prev = head->last;

    // Update last to point to this node
    head->last = node;
}

void xpsDllUnlink(xpsDllHead *head, xpsDllLink *node)
{
    // If this is not the first node
    if (node->prev != NULL)
    {
        // Update the previous node to point to the next node
        node->prev->next = node->next;
    }
    else
    {
        // Update first to point to the next node
        head->first = node->next;
    }

    // If this is not the last node
    if (node->next != NULL)
    {
        // Update the next node to point to the previous node
        node->next->prev = node->prev;
    }
    else
    {
        // Update last to point to the previous node
        head->last = node->prev;
    }
}

/*
 * Device map utility
 */

void xpsDeviceMapSetDeviceBit(xpsDeviceMap *deviceMap, xpsDevice_t devId)
{
    // Set the bit corresponding to device devId.
    deviceMap->bits[devId / SIZEOF_BYTE] |= (0x01 << (devId % SIZEOF_BYTE));
}

void xpsDeviceMapClrDeviceBit(xpsDeviceMap *deviceMap, xpsDevice_t devId)
{
    // Clear the bit corresponding to device devId.
    deviceMap->bits[devId / SIZEOF_BYTE] &= ~(0x01 << (devId % SIZEOF_BYTE));
}

uint32_t xpsDeviceMapTestDeviceBit(xpsDeviceMap *deviceMap, xpsDevice_t devId)
{
    // Check the bit corresponding to device devId.
    return ((uint32_t)((deviceMap->bits[devId / SIZEOF_BYTE]) & (0x01 <<
                                                                 (devId % SIZEOF_BYTE))));
}

uint32_t xpsDeviceMapIsEmpty(xpsDeviceMap *deviceMap)
{
    // Check if any of the bits is set.
    for (uint32_t i=0; i<(XP_MAX_DEVICES / SIZEOF_BYTE); i++)
    {
        if (deviceMap->bits[i] != 0)
        {
            return 0;
        }
    }

    return 1;
}

/*
 * Math utility
 */

uint32_t xpsIsPowerOfTwo(uint32_t number)
{
    return ((number != 0) && ((number & (number - 1)) == 0));
}

/* Dynamic Array (DA) utility apis.
The dynamic array grows exponentially with base 2 ie.
every time it doubles when the size exceeds the current array size.

But the array shrinks at a slower pace to provide a small window
of relief for flaps at the 2^n array boundaries. If size falls to 1/3 then
array is resized to 1/2 of its size.
*/
uint32_t xpsDAIsCtxGrowthNeeded(uint16_t numOfArrayElements,
                                uint16_t defaultArraySize)
{
    return (xpsIsPowerOfTwo(numOfArrayElements) &&
            (numOfArrayElements >= defaultArraySize));
}

uint32_t xpsDAGetCtxSizeWhileGrow(uint16_t sizeOfContext,
                                  uint16_t sizeOfElement, uint16_t numOfArrayElements, uint16_t defaultArraySize)
{
    uint16_t sizeNewArray = numOfArrayElements*2;

    return ((sizeOfContext + sizeOfElement *(sizeNewArray - defaultArraySize)));
}

XP_STATUS xpsDynamicArrayGrow(void **newContext, void *oldContext,
                              uint32_t sizeOfContext,
                              uint16_t sizeOfElement, uint32_t numOfElements, uint32_t defaultSize)
{
    XP_STATUS result = XP_NO_ERR;

    //Allocate a new context with double interface array size

    //Subtraction accounts for already added default dynamic array size in the structure type xpsLagVlanListDbEntry
    uint16_t numOfBytes = xpsDAGetCtxSizeWhileGrow(sizeOfContext, sizeOfElement,
                                                   numOfElements, defaultSize);

    if ((result = xpsStateHeapMalloc(numOfBytes, (void**)newContext)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "XPS state Heap Allocation failed");
        return result;
    }
    if ((*newContext) == NULL)
    {
        return XP_ERR_NULL_POINTER;
    }

    memset((*newContext), 0, numOfBytes);

    //Copy old context into new context (should copy old context size - smaller one)
    numOfBytes = sizeOfContext+ sizeOfElement*(numOfElements - defaultSize);
    memcpy((*newContext), oldContext, numOfBytes);

    return result;
}
/*The number of elements reaches 1/3 or the integer just less than 1/3 of the dynamic array size
then shrink the size to 1/2 or original.

A power of 2 can never be divisible by 3. The remainder will be either 1 or 2.
So if the 3n+1 or 3n+2 is a power of 2 then we should shrink the array to 3n/2 +1 size

The two APIs below implement this logic*/

uint32_t xpsDAIsCtxShrinkNeeded(uint16_t numOfArrayElements,
                                uint16_t defaultArraySize)
{
    return ((xpsIsPowerOfTwo(numOfArrayElements*3 + 1) ||
             xpsIsPowerOfTwo(numOfArrayElements*3 + 2))
            && (numOfArrayElements >= defaultArraySize));
}

uint32_t xpsDAGetCtxSizeWhileShrink(uint16_t sizeOfContext,
                                    uint16_t sizeOfElement, uint16_t numOfArrayElements, uint16_t defaultArraySize)
{
    uint16_t sizeNewArray = (numOfArrayElements*3)/2 + 1;

    return ((sizeOfContext + sizeOfElement *(sizeNewArray - defaultArraySize)));
}

XP_STATUS xpsDynamicArrayShrink(void **newContext, void *oldContext,
                                uint32_t sizeOfContext,
                                uint16_t sizeOfElement, uint32_t numOfElements, uint32_t defaultSize)
{
    XP_STATUS result = XP_NO_ERR;

    uint16_t numOfBytes = xpsDAGetCtxSizeWhileShrink(sizeOfContext, sizeOfElement,
                                                     numOfElements, defaultSize);

    if ((result = xpsStateHeapMalloc(numOfBytes, (void**)newContext)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "XPS state Heap Allocation failed");
        return result;
    }
    if ((*newContext) == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Recieved null pointer");
        return XP_ERR_NULL_POINTER;
    }

    memset((*newContext), 0, numOfBytes);

    //Copy old context into new context (should copy new context size - smaller)
    memcpy((*newContext), oldContext, numOfBytes);

    return result;

}
void xpsPrintBootTimeAnalysis(xpsDevice_t devId)
{
    uint32_t portNumber = 0,
             totalInitedPortCount = 0, totalSerdesCount = 0;
    XP_STATUS ret;
    uint16_t maxPortNum = 0;


    ret = xpsGlobalSwitchControlGetMaxNumPhysicalPorts(devId, &maxPortNum);
    if (XP_NO_ERR != ret)
    {
        return;
    }
    for (portNumber = 0; portNumber < maxPortNum; portNumber++)
    {
    }
    /* Check invalid port and serdes count*/
    if (0 == totalInitedPortCount || 0 == totalSerdesCount)
    {
    }

    if (((xpTimeStamp[XP_BOOTUP_ALL].endTime -
          xpTimeStamp[XP_BOOTUP_ALL].startTime)/1000000) > 0)
    {
        if (xpTimeStamp[XP_BOOTUP_ALL].startTime != 0)
        {
        }
        else
        {
        }

        if (xpTimeStamp[XP_APP_NEW_DEVICE_INIT].startTime != 0)
        {
        }
        else
        {
        }
        if (xpTimeStamp[XP_INIT_ADD_DEVICE].startTime != 0)
        {
        }
        else
        {
        }
        if (xpTimeStamp[XP_DEVICE_INIT].startTime != 0)
        {
        }
        else
        {
        }
        if (xpTimeStamp[XP_TXQMGR_INIT].startTime != 0)
        {
        }
        else
        {
        }
        if (xpTimeStamp[XP_VLAN_ADD_DEVICE].startTime != 0)
        {
        }
        else
        {
        }
        if (xpTimeStamp[XP_TUNNEL_ADD_DEVICE].startTime != 0)
        {
        }
        else
        {
        }
        if (xpTimeStamp[XP_LINK_ADD_DEVICE].startTime != 0)
        {
        }
        else
        {
        }

        if (xpTimeStamp[XP_PORT_INIT].startTime != 0)
        {
        }
        else
        {
        }

        if (xpTimeStamp[XP_SINGLE_PORT_INIT].startTime != 0)
        {
        }
        else
        {
        }
        if (xpTimeStamp[XP_MAC_INIT].startTime != 0)
        {
        }
        else
        {
        }
        if (xpTimeStamp[XP_SERDES_ILB].startTime != 0)
        {
        }
        else
        {
        }
        if (xpTimeStamp[XP_SERDES_SETUP].startTime != 0)
        {
        }
        else
        {
        }
        if (xpTimeStamp[XP_SERDES_FW_UPLOAD].startTime != 0)
        {
        }
        else
        {
        }
    }
    else
    {
    }

    return;
}

XP_STATUS xpsHandlePortVlanTableRehash(xpsDevice_t devId,
                                       xpsHashIndexList_t *indexList)
{
    XP_STATUS status = XP_NO_ERR;

    /* If no rehahsing had happened, it is fine */
    if (indexList->size == XP_INDEX_LIST_SIZE_FOR_NO_REHASH)
    {
        return XP_NO_ERR;
    }
    /*
     * Following XPS modules use port vlan table:
     *      - xpsVlan
     *      - xpsAc
     */
    status = xpsVlanHandlePortVlanTableRehash(devId, indexList);
    if (status != XP_NO_ERR)
    {
        return status;
    }


    return XP_NO_ERR;
}

XP_STATUS xpsHandleTunnelIvifTableRehash(xpsDevice_t devId,
                                         xpsHashIndexList_t *indexList)
{
    XP_STATUS status = XP_NO_ERR;

    /* If no rehahsing had happened, it is fine */
    if (indexList->size == XP_INDEX_LIST_SIZE_FOR_NO_REHASH)
    {
        return XP_NO_ERR;
    }

    /*
     * Following XPS modules use tunnel Ivif table:
     *      - xpsTunnel
     *      - xpsL3
     *      - xpsAc
     */
    status = xpsTunnelHandleTunnelIvifTableRehash(devId, indexList);
    if (status != XP_NO_ERR)
    {
        return status;
    }


    return XP_NO_ERR;
}

int xpsGetExitStatus()
{
    //XPS_FUNC_ENTRY_LOG();

    return exit_parent;

    //XPS_FUNC_EXIT_LOG();

    return 0;
}

void xpsSetExitStatus()
{
    //XPS_FUNC_ENTRY_LOG();

    exit_parent++;

    //XPS_FUNC_EXIT_LOG();

}

static xpLogLevel logLevel = XP_LOG_ERROR;

/**
* \brief Set the log level
*
*
* \param [in] xpLoglevel
*
* \return XP_STATUS
*/

GT_STATUS xpsSetLogLevel(xpLogLevel l)
{
    logLevel = l;
    return XP_NO_ERR;
}

/**
* \brief Gets the log level
*
*
* \param [in] void
*
* \return xpLogLevel
*/

xpLogLevel xpsGetLogLevel(void)
{
    return logLevel;
}


/**
* \brief Gets the log level name
*
*
* \param [in] xpLogLevel
*
* \return char*
*/

char* xpsGetLogLevelName(xpLogLevel l)
{
    return (char*) xpLogLevelNames[l];
}


/**
* \brief Gets the number of log levels
*
*
* \param [in] void
*
* \return uint8_t
*/

uint8_t xpsGetNumLogLevels(void)
{
    return XP_LOG_MAX_LEVEL;
}

void writeLog(char* fileName, int lineNum, xpLogLevel msgLogLevel,
              const char* msg, ...)
{

    int size;
    va_list vl;
    char *buffer;

    xpLogLevel currentLogLevel = xpsGetLogLevel();

    if (msgLogLevel < 0 || msgLogLevel >= XP_LOG_MAX_LEVEL)
    {
        msgLogLevel=XP_LOG_MAX_LEVEL;
    }

    if (msgLogLevel >= currentLogLevel)
    {
        va_start(vl, msg);
        buffer=(char*)cpssOsMalloc(XPLOG_SIZE);
        if (buffer==NULL)
        {
            cpssOsPrintf("LOGGING FAILED ! BUFFER NOT ALLOCATED \n");
            va_end(vl);
            return;
        }
        size=cpssOsVsnprintf(buffer, XPLOG_SIZE, msg, vl);
        va_end(vl);
        if (size >= XPLOG_SIZE)
        {
            size=size+1;
            va_start(vl, msg);
            buffer=(char*)cpssOsRealloc(buffer, size);
            if (buffer==NULL)
            {
                cpssOsPrintf("LOGGING FAILED ! BUFFER NOT ALLOCATED \n");
                va_end(vl);
                return;
            }
            size=cpssOsVsnprintf(buffer, size, msg, vl);
            va_end(vl);
        }

        if (lineNum <= 0)
        {
            cpssOsPrintf("%s", buffer);
            syslog(LOG_INFO, "%s \n", buffer);
        }
        else
        {
            cpssOsPrintf("%s:%1d, %s: %s \n", fileName, lineNum,
                         xpLogLevelNames[msgLogLevel], buffer);
            syslog(LOG_INFO, "%s:%1d, %s: %s \n", fileName, lineNum,
                   xpLogLevelNames[msgLogLevel], buffer);
        }
        cpssOsFree(buffer);
    }
}


XP_STATUS xpsUtilConvertReasonCodeToCpssCpuCode(xpsCoppCtrlPkt_e pXpsTrap,
                                                CPSS_NET_RX_CPU_CODE_ENT reasonCode[], uint8_t* count)
{
    switch (pXpsTrap)
    {
        case XPS_COPP_CTRL_PKT_ARP_REQUEST:
            reasonCode[0] = CPSS_NET_INTERVENTION_ARP_E;
            *count = 1;
            break;
        case XPS_COPP_CTRL_PKT_ARP_RESPONSE:
            reasonCode[0] = CPSS_NET_ARP_REPLY_TO_ME_E;
            *count = 1;
            break;
        case XPS_COPP_CTRL_PKT_DHCP:
        case XPS_COPP_CTRL_PKT_DHCP_L2:
            reasonCode[0] = CPSS_NET_UDP_BC_MIRROR_TRAP0_E;
            *count = 1;
            break;
        case XPS_COPP_CTRL_PKT_DHCPV6:
        case XPS_COPP_CTRL_PKT_DHCPV6_L2:
            reasonCode[0] = (CPSS_NET_RX_CPU_CODE_ENT)CPSS_HAL_CTRLPKT_CPU_CODE_DHCPv6;
            *count = 1;
            break;
        case XPS_COPP_CTRL_PKT_IGMP_TYPE_LEAVE:
        case XPS_COPP_CTRL_PKT_IGMP_TYPE_QUERY:
        case XPS_COPP_CTRL_PKT_IGMP_TYPE_V1_REPORT:
        case XPS_COPP_CTRL_PKT_IGMP_TYPE_V2_REPORT:
        case XPS_COPP_CTRL_PKT_IGMP_TYPE_V3_REPORT:
            reasonCode[0] = CPSS_NET_INTERVENTION_IGMP_E;
            *count = 1;
            break;
        case XPS_COPP_CTRL_PKT_IPV6_MLD_V1_DONE:
        case XPS_COPP_CTRL_PKT_IPV6_MLD_V1_REPORT:
        case XPS_COPP_CTRL_PKT_IPV6_MLD_V1_V2:
        case XPS_COPP_CTRL_PKT_IPV6_NEIGHBOR_DISCOVERY:
        case XPS_COPP_CTRL_PKT_MLD_V2_REPORT:
            reasonCode[0] = CPSS_NET_IPV6_ICMP_PACKET_E;
            *count = 1;
            break;
        case XPS_COPP_CTRL_PKT_LACP:
            reasonCode[0] = CPSS_NET_IEEE_RSRVD_MULTICAST_ADDR_2_E;
            *count = 1;
            break;
        case XPS_COPP_CTRL_PKT_LLDP:
            reasonCode[0] =  CPSS_NET_IEEE_RSRVD_MULTICAST_ADDR_3_E;
            *count = 1;
            break;
        case XPS_COPP_CTRL_PKT_OSPF:
            reasonCode[0] = CPSS_NET_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_1_E;
            *count = 1;
            break;
        case XPS_COPP_CTRL_PKT_OSPFV6:
            reasonCode[0] = CPSS_NET_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_1_E ;
            *count = 1;
            break;
        case XPS_COPP_CTRL_PKT_PIM:
            reasonCode[0] = CPSS_NET_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_3_E;
            *count = 1;
            break;
        case XPS_COPP_CTRL_PKT_VRRP:
        case XPS_COPP_CTRL_PKT_VRRPV6: //rfc5798
            reasonCode[0] = CPSS_NET_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_2_E;
            *count = 1;
            break;
        case XPS_COPP_CTRL_PKT_SNMP:
            reasonCode[0] = CPSS_NET_UDP_BC_MIRROR_TRAP2_E;
            *count = 1;
            break;
        case XPS_COPP_CTRL_PKT_STP:
            reasonCode[0] = CPSS_NET_IEEE_RSRVD_MULTICAST_ADDR_1_E;
            *count = 1;
            break;
        case XPS_COPP_CTRL_PKT_TTL_ERROR:
            reasonCode[0] = CPSS_NET_IP_ZERO_TTL_TRAP_E;
            reasonCode[1] = CPSS_NET_IPV6_TTL_TRAP_E;
            *count = 2;
            break;
        case XPS_COPP_CTRL_PKT_L3_MTU_ERROR:
            reasonCode[0] = CPSS_NET_IP_MTU_EXCEED_E;
            *count = 1;
            break;
        case XPS_COPP_CTRL_PKT_BGP:
        case XPS_COPP_CTRL_PKT_BGPV6:
            reasonCode[0] = (CPSS_NET_RX_CPU_CODE_ENT)CPSS_HAL_CTRLPKT_CPU_CODE_BGPv4v6;
            *count = 1;
            break;
        case XPS_COPP_CTRL_PKT_UDLD:
        case XPS_COPP_CTRL_PKT_PVRST:
            reasonCode[0] = CPSS_NET_CISCO_MULTICAST_MAC_RANGE_E;
            *count = 1;
            break;
        case XPS_COPP_CTRL_PKT_IP2ME:
            reasonCode[0] = CPSS_NET_ROUTED_PACKET_FORWARD_E;
            reasonCode[1] = CPSS_NET_ROUTE_ENTRY_TRAP_E;
            reasonCode[2] = CPSS_NET_IPV6_ROUTE_TRAP_E;
            *count = 3;
            break;
        case XPS_COPP_CTRL_PKT_SSH:
            reasonCode[0] = (CPSS_NET_RX_CPU_CODE_ENT)CPSS_HAL_CTRLPKT_CPU_CODE_SSH;
            *count = 1;
            break;
        case XPS_COPP_CTRL_PKT_SAMPLEPACKET:
            reasonCode[0] = CPSS_NET_INGRESS_MIRRORED_TO_ANLYZER_E;
            reasonCode[1] = CPSS_NET_EGRESS_MIRRORED_TO_ANLYZER_E;
            *count = 2;
            break;
        case XPS_COPP_CTRL_PKT_STATIC_FDB_MOVE:
            reasonCode[0] = CPSS_NET_STATIC_ADDR_MOVED_E;
            *count = 1;
            break;
        case XPS_COPP_CTRL_PKT_EAPOL:
            reasonCode[0] = (CPSS_NET_RX_CPU_CODE_ENT) CPSS_HAL_CTRLPKT_CPU_CODE_EAPOL_TRAP;
            *count = 1;
            break;
        default:
            *count = 0;
            break;
    }

    return XP_NO_ERR;
}

uint8_t xpsIsDeviceFalcon(xpsDevice_t devId)
{
    XP_DEV_TYPE_T devType;
    XP_STATUS ret;

    if ((ret = cpssHalGetDeviceType((xpsDevice_t)devId, &devType)) != XP_NO_ERR)
    {
        return 0;
    }
    if (IS_DEVICE_FALCON(devType))
    {
        return 1;
    }
    // Default
    return 0;
}

XP_STATUS xpsUtilCpssShellEnable(bool enable)
{
#ifdef SHELL_ENABLE
    GT_STATUS rc;
    if ((rc = cpssHalCmdShellEnable((GT_BOOL)enable)==GT_OK))
    {
        return XP_NO_ERR;
    }
    else
    {
        return XP_ERR_INIT;
    }
#endif
    return XP_NO_ERR;


}
#ifdef __cplusplus
}
#endif

