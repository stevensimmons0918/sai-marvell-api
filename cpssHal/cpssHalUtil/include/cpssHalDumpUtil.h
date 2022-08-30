/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/*******************************************************************************
* cpssHalDumpUtil.h
*
* DESCRIPTION:
*       OpenFlow FPA Library datatypes.
*
* FILE REVISION NUMBER:
*       $Revision: 04 $
*
*******************************************************************************/
#ifndef CPSS_HAL_DUMP_UTIL_H
#define CPSS_HAL_DUMP_UTIL_H

#include <stdbool.h>
#include <syslog.h>
#include <cpss/generic/cpssTypes.h>
#include "cpss/dxCh/dxChxGen/config/cpssDxChCfgInit.h"


#define CPSS_HAL_DUMPMAC_ADDRESS_SIZE    6
#define CPSS_HAL_DUMPIPV6_ADDRESS_SIZE   16


/* Macro to check for a mac multicast address   */
/* GT_U8 mac[6]                                 */
#define IS_MAC_MULTICAST(mac)        ((mac[0] & 0x1) == 0x1)

/* Macro to check for a mac broadcast address   */
/* GT_U8 mac[6]                                 */
#define IS_MAC_BROADCAST(mac)                                           \
        ((mac[0] == 0xFF) && (mac[1] == 0xFF) && (mac[2] == 0xFF) &&    \
         (mac[3] == 0xFF) && (mac[4] == 0xFF) && (mac[5] == 0xFF))

#define IS_MAC_EMPTY(mac)                                      \
        ((mac[0] == 0) && (mac[1] == 0) && (mac[2] == 0) &&    \
         (mac[3] == 0) && (mac[4] == 0) && (mac[5] == 0))

#define CPSS_HAL_DUMPUTILS_DEV_TYPE_BC2        0
#define CPSS_HAL_DUMPUTILS_DEV_TYPE_BOBK       1
#define CPSS_HAL_DUMPUTILS_DEV_TYPE_BC3        2
#define CPSS_HAL_DUMPUTILS_DEV_TYPE_ARMSTRONG  3

/* Curently constant definition for switch_id value instead of using "0" hard coded */
#define CPSS_HAL_DUMPSWITCH_ID 0

typedef struct
{
    uint8_t     addr[CPSS_HAL_DUMPMAC_ADDRESS_SIZE];
} CPSS_HAL_DUMPMAC_ADDRESS_STC;

#define CPSS_HAL_DUMPMAC_ADDRESS_IS_BC(macAddr)  ((macAddr.addr[0] == 0xFF) && (macAddr.addr[1] == 0xFF) && (macAddr.addr[2] == 0xFF) && (macAddr.addr[3] == 0xFF) && (macAddr.addr[4] == 0xFF) && (macAddr.addr[5] == 0xFF))
#define CPSS_HAL_DUMPMAC_ADDRESS_IS_EMPTY(macAddr)  ((macAddr.addr[0] == 0) && (macAddr.addr[1] == 0) && (macAddr.addr[2] == 0) && (macAddr.addr[3] == 0) && (macAddr.addr[4] == 0) && (macAddr.addr[5] == 0))
#define CPSS_HAL_DUMPMAC_ADDRESS_IS_INVALID(macAddr) (CPSS_HAL_DUMPMAC_ADDRESS_IS_BC(macAddr) || CPSS_HAL_DUMPMAC_ADDRESS_IS_EMPTY(macAddr))

/*
 *  * Typedef: enum CPSS_HAL_DUMPSTATUS
 *   *
 *    * Description: Values to be returned by all FPA functions
 *     *
 *      */
typedef enum
{
    CPSS_HAL_DUMPERROR                 = (-1),
    CPSS_HAL_DUMPOK                    = (0x00), /* Operation succeeded */
    CPSS_HAL_DUMPFAIL                  = (0x01), /* Operation failed    */
} CPSS_HAL_DUMPSTATUS;


typedef struct
{
    GT_U32 trafficClass : 3;
    GT_U32 dropPrecedence : 2;
    GT_U32 truncated : 1;
    GT_U32 reserved1 : 5;
    GT_U32 targetDeviceIndex : 3;
    GT_U32 reserved2 : 1;
    GT_U32 rateLimitingMode : 1;
    GT_U32 statisticalRateLimiterIndex : 8;
    GT_U32 rateLimiterIndex : 8;

} CPUCodeDisplayTable;

typedef struct
{
    GT_U32 limiterPktNum : 16;
    GT_U32 limiterWindowSize : 12;

} rateLimiterConfigDisplayTable;

bool cpssHalDumpUtlIsIpUnicast(in_addr_t);
bool cpssHalDumpUtlIsIpv6Unicast(struct in6_addr *ip6Addr);

void cpssHalDumpUtlInit(void);
void cpssHalDumpUtlCPUCodesDump(uint32_t devId, CPSS_DXCH_TABLE_ENT tableType,
                                uint32_t startIndex, uint32_t endIndex);

void cpssHalDumpUtlMsgFlowDelete(uint32_t  flowTableNo,
                                 uint64_t  cookie);


void cpssHalDumpUtlPrintMacAddress
(
    CPSS_HAL_DUMPMAC_ADDRESS_STC    *macAddrPtr
);

CPSS_HAL_DUMPSTATUS cpssHalDumpUtlGetFirstZeroBit(uint32_t word,
                                                  uint32_t *bitIndex);


char* cpssHalDumpUtlIp2Str(in_addr_t ipAddr);

char* cpssHalDumpUtlIpPrefix2Str(uint32_t type, uint8_t *value);

bool cpssHalDumpUtlGetDeviceType(uint8_t dev, uint32_t *devTypePtr);

bool cpssHalDumpUtlGetSwitchNumOfPorts(uint32_t switchId,
                                       uint32_t *numOfPorts);

bool cpssHalDumpUtlConvertSwitchPortToDevPort(uint32_t switchId,
                                              uint32_t port,
                                              uint8_t  *devId,
                                              uint32_t *devPort);

bool cpssHalDumpUtlConvertDevPortToSwitchPort(uint8_t devId,
                                              uint32_t devPort,
                                              uint32_t *switchId,
                                              uint32_t *port);


uint32_t cpssHalDumpUtlIpmask2Prefixlen(in_addr_t ipMask);
in_addr_t cpssHalDumpUtlPrefixlen2Ipmask(uint32_t prefix);

uint32_t cpssHalDumpUtlIpv6mask2Prefixlen(struct in6_addr *ip6Mask);
void cpssHalDumpUtlPrefixlen2Ipv6mask(uint32_t prefix,
                                      struct in6_addr *ip6Mask);

void cpssHalDumpUtlRateLimiterDump(uint32_t devId,
                                   CPSS_DXCH_TABLE_ENT tableType, uint32_t startIndex, uint32_t endIndex);
int cpssHalDumpUtlReadPpReg(uint32_t regAddr);
int cpssHalDumpUtlWritePpReg(GT_U32 regAddr, uint32_t value);
int cpssHalDumpUtlReadPpRange(uint32_t regAddr, uint32_t size);
int cpssHalDumpUtlDumpVlan(int vlan);
void cpssHalDumpUtlFdbDump(GT_BOOL full);
int cpssHalDumpUtlFdbCount(uint32_t *usedEntries);
CPSS_HAL_DUMPSTATUS cpssHalDumpUtlVidxDump(void);
CPSS_HAL_DUMPSTATUS cpssHalDumpUtlMacToMeDump
(
    void
);
void cpssHalDumputilsLpmDump
(
    uint8_t                dev,
    uint32_t               ip_ver
);

void cpssHalDumputilsVlanDump
(
    uint8_t                dev,
    uint32_t               vlan_id
);

void cpssHalDumputilsVlanListDump
(
    uint8_t                dev
);

void cpssHalDumputilsPortDump
(
    uint8_t                devId,
    uint32_t               portNum,
    bool                   detail
);
GT_STATUS cpssHalDumpUtlRegDump(int devNum);
GT_STATUS cpssHalDumputilsFdbDump
(
    IN GT_U8                devNum
);

extern void mrvlExtOsLogFunc
(
    CPSS_LOG_LIB_ENT      lib,
    CPSS_LOG_TYPE_ENT     type,
    const char*           format,
    ...
);
#endif /* CPSS_HAL_DUMP_UTIL_H */
