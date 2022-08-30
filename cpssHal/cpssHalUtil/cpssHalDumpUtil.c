/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/********************************************************************************
* utils.c
*
* DESCRIPTION:
*       OpenFlow FPA Library APIs.
*
* FILE REVISION NUMBER:
*       $Revision: 04 $
*
*******************************************************************************/
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include "cpssCommon/private/prvCpssMisc.h"
#include "cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h"
#include "cpss/dxCh/dxChxGen/port/cpssDxChPortStat.h"
#include <cpss/common/port/private/prvCpssPortTypes.h>
#include "cpss/dxCh/dxChxGen/phy/cpssDxChPhySmi.h"
#include <cpss/dxCh/dxChxGen/networkIf/private/prvCpssDxChNetIf.h>
#include "cpss/dxCh/dxChxGen/bridge/cpssDxChBrgVlan.h"
#include "cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwTables.h"
#include "cpss/dxCh/dxChxGen/tti/cpssDxChTtiTypes.h"
#include "cpss/dxCh/dxChxGen/tti/cpssDxChTti.h"
#include "cpss/dxCh/dxChxGen/config/cpssDxChCfgInit.h"
#include "cpss/generic/cpssTypes.h"
#include "cpss/common/port/cpssPortStat.h"
#include "cpssDriver/pp/hardware/cpssDriverPpHw.h"
#include "cpssDriver/pp/hardware/prvCpssDrvHwPpPortGroupCntl.h"
#include "cpss/dxCh/dxChxGen/diag/cpssDxChDiag.h"
#include "cpss/dxCh/dxChxGen/ipLpmEngine/cpssDxChIpLpmTypes.h"
#include "cpss/dxCh/dxChxGen/ipLpmEngine/cpssDxChIpLpm.h"
#include "cpss/dxCh/dxChxGen/bridge/cpssDxChBrgMc.h"
#include "cpss/dxCh/dxChxGen/ip/cpssDxChIp.h"
#include "cpss/generic/log/cpssLog.h"

#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


#include <cpssHalDumpUtil.h>


#define CPSS_HAL_DUMPMAX_NUM_OF_PORTS        32
#define MAX_IP_PREFIX               32
#define MAX_IPV6_PREFIX             128
#define IPV6_BYTE_PREFIX            8
#define CPSS_HAL_DUMPUTL_STRING_MAX_LENGTH   40
#define CPSS_HAL_DUMPUTL_MAX_DBGS            20
#define CPSS_HAL_DUMPIP_STR_MAX_LEN          128

#define CPSS_HAL_DUMPIP_TYPE_TO_AF_TYPE(type) (type==CPSS_HAL_DUMPLIB_ETH_TYPE_IPv4?AF_INET:AF_INET6)

typedef enum
{
    CPSS_HAL_DUMPUTL_DBG_SYSTEM_E,
    CPSS_HAL_DUMPUTL_DBG_LOG_E,
    CPSS_HAL_DUMPUTL_DBG_HW_E,
    CPSS_HAL_DUMPUTL_DBG_FLOW_COMMON_E,
    CPSS_HAL_DUMPUTL_DBG_FLOW_VLAN_E,
    CPSS_HAL_DUMPUTL_DBG_FLOW_TERMINATION_E,
    CPSS_HAL_DUMPUTL_DBG_FLOW_IPCL_E,
    CPSS_HAL_DUMPUTL_DBG_FLOW_BRIDGE_E,
    CPSS_HAL_DUMPUTL_DBG_FLOW_L3UNICAST_E,
    CPSS_HAL_DUMPUTL_DBG_METER_E,
    CPSS_HAL_DUMPUTL_DBG_GROUP_COMMON_E,
    CPSS_HAL_DUMPUTL_DBG_GROUP_L2INTERFACE_E,
    CPSS_HAL_DUMPUTL_DBG_GROUP_L2MULTICAST_E,
    CPSS_HAL_DUMPUTL_DBG_GROUP_L2REWRITE_E,
    CPSS_HAL_DUMPUTL_DBG_GROUP_L3UNICAST_E,
    CPSS_HAL_DUMPUTL_DBG_LAST
} CPSS_HAL_DUMPUTL_DBG_ENT;

struct cpssHalDumpUtlDbgFunction
{
    char name[100];
    char description[400];
};
void *cpssHalDumpUtlComnCpuRegAddr = 0;

#define CPSS_HAL_DUMPARP_TBL_SIZE                (16*1024)
#define CPSS_HAL_DUMPNEXT_HOP_TBL_SIZE           (8*1024)

#define CPSS_HAL_DUMPHASH_CONTROL_PKTS_TABLE_SIZE        1024
/* Flow vlan table defines */
#define CPSS_HAL_DUMPHASH_VLAN_TABLE_SIZE        1024/*(128 * 1024)*/
#define CPSS_HAL_DUMPFLOW_VLAN_MAX_NUMBER        4095
/*TODO This is changed as per aldrin FS. When profiling is done in future, this needs to be taken care */
#define CPSS_HAL_DUMPFLOW_VRF_TABLE_SIZE         128
#define CPSS_HAL_DUMPGROUP_VIDX_MAX_SIZE (4 * 1024)
#define CPSS_HAL_DUMPHASH_TERMINATION_TABLE_SIZE 128
#define     CPSS_HAL_DUMPHAL_L3_LPM_DB_ID        1
#define CPSS_HAL_DUMPLIB_ETH_TYPE_IPv4           0x0800
#define CPSS_HAL_DUMPLIB_ETH_TYPE_IPv6           0x86dd
#define CPSS_HAL_DUMPOUTPUT_CPU_PORT             63
#define     CPSS_HAL_DUMPHAL_L3_DEFAULT_VRID     0
uint32_t cpssHalDumpSysArpTblSize = CPSS_HAL_DUMPARP_TBL_SIZE;
uint32_t cpssHalDumpSysNextHopTblSize = CPSS_HAL_DUMPNEXT_HOP_TBL_SIZE;
uint32_t cpssHalDumpSysVlanTblSize = CPSS_HAL_DUMPHASH_VLAN_TABLE_SIZE;
uint32_t cpssHalDumpSysVrfTblSize = CPSS_HAL_DUMPFLOW_VRF_TABLE_SIZE;

uint32_t cpssHalDumpPortToHalPort[CPSS_HAL_DUMPMAX_NUM_OF_PORTS];
uint32_t cpssHalDumpHalPortToPort[CPSS_HAL_DUMPMAX_NUM_OF_PORTS];
uint32_t cpssHalDumpSystemNumOfPorts = 0;



void cpssHalDumpUtlPrintMacAddress
(
    CPSS_HAL_DUMPMAC_ADDRESS_STC  *macAddrPtr
)
{
    cpssOsPrintf("%02x:%02x:%02x:%02x:%02x:%02x",
                 macAddrPtr->addr[0], macAddrPtr->addr[1],
                 macAddrPtr->addr[2], macAddrPtr->addr[3],
                 macAddrPtr->addr[4], macAddrPtr->addr[5]);
}

static char * cpssHalDumpUtlMacAddress2String
(
    CPSS_HAL_DUMPMAC_ADDRESS_STC  *macAddrPtr
)
{
    static char str[20];
    sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x",
            macAddrPtr->addr[0], macAddrPtr->addr[1],
            macAddrPtr->addr[2], macAddrPtr->addr[3],
            macAddrPtr->addr[4], macAddrPtr->addr[5]);
    return str;
}

/******** debug functions *******************/
CPSS_HAL_DUMPSTATUS cpssHalDumpUtlGetDeviceNumber
(
    GT_U8 *cpssDevNum
)
{
    *cpssDevNum = 0;
    return CPSS_HAL_DUMPOK;
}

int cpssHalDumpUtlReadPpReg(uint32_t regAddr)
{
    int          rc;
    uint32_t     value;

    rc = cpssDrvPpHwRegisterRead(0, 0, regAddr, &value);
    if (rc == GT_OK)
    {
        cpssOsPrintf("value: 0x%x\n", (unsigned int)value);
    }
    else
    {
        cpssOsPrintf("failed\n");
        return CPSS_HAL_DUMPFAIL;
    }

    return CPSS_HAL_DUMPOK;
}

int cpssHalDumpUtlWritePpReg(GT_U32 regAddr, uint32_t value)
{
    GT_STATUS   rc;
    rc = cpssDrvPpHwRegisterWrite(0, 0, regAddr, value);
    if (rc != GT_OK)
    {
        return CPSS_HAL_DUMPFAIL;
    }
    else
    {
        return CPSS_HAL_DUMPOK;
    }

}

int cpssHalDumpUtlReadPpRange(uint32_t regAddr, uint32_t size)
{
    int   rc;
    uint32_t      buff[64], i, len;

    while (size > 0)
    {
        len = size;
        if ((len > 64) || (len == 0))
        {
            len = 64;
        }
        rc = prvCpssDrvHwPpPortGroupReadRam(0, 0, regAddr, len, buff);

        if (rc == GT_OK)
        {
            for (i = 0; i < len; i += 4)
            {
                cpssOsPrintf("%08x:  %08x %08x %08x %08x\n", regAddr+i*4, buff[i], buff[i+1],
                             buff[i+2], buff[i+3]);
            }
        }
        else
        {
            cpssOsPrintf("failed\n");
        }

        if (size > 64)
        {
            size -= 64;
        }
        else
        {
            size = 0;
        }

        regAddr += 256;
    }
    return CPSS_HAL_DUMPOK;
}

int cpssHalDumpUtlReadPhyReg(uint16_t port, uint16_t page, uint16_t regAddr)
{
    int   rc;
    uint16_t      regVal;     /* Register value */

    rc = cpssDxChPhyPortSmiRegisterWrite(0, port, 22, page);
    if (rc != GT_OK)
    {
        return CPSS_HAL_DUMPFAIL;
    }
    rc = cpssDxChPhyPortSmiRegisterRead(0, port, regAddr, &regVal);
    if (rc != GT_OK)
    {
        return CPSS_HAL_DUMPFAIL;
    }
    /*change back page to 0 */
    rc = cpssDxChPhyPortSmiRegisterWrite(0, port, 22, 0);
    if (rc != GT_OK)
    {
        return CPSS_HAL_DUMPFAIL;
    }
    cpssOsPrintf("read phy: port:%d page:%d address:0x%x value: 0x%x \n", port,
                 page, regAddr, regVal);

    return CPSS_HAL_DUMPOK;
}

int cpssHalDumpUtlWritePhyReg(uint16_t port, uint16_t page, uint16_t regAddr,
                              uint16_t value)
{
    int   rc;

    rc = cpssDxChPhyPortSmiRegisterWrite(0, port, 22, page);
    if (rc != GT_OK)
    {
        return CPSS_HAL_DUMPFAIL;
    }

    rc = cpssDxChPhyPortSmiRegisterWrite(0, port, regAddr, value);
    if (rc != GT_OK)
    {
        return CPSS_HAL_DUMPFAIL;
    }
    /*change back page to 0 */
    rc = cpssDxChPhyPortSmiRegisterWrite(0, port, 22, 0);
    if (rc != GT_OK)
    {
        return CPSS_HAL_DUMPFAIL;
    }
    cpssOsPrintf("write phy: port:%d page:%d address:0x%x value: 0x%x\n", port,
                 page, regAddr, value);

    return CPSS_HAL_DUMPOK;
}

int cpssHalDumpUtlReadCpuReg(uint32_t regAddr)
{
    int      value;
    int  volatile *paddr;
    paddr = (int *)(((void *) cpssHalDumpUtlComnCpuRegAddr) + regAddr);
    value =   *paddr;
    cpssOsPrintf("base %p address %p value: 0x%x\n", cpssHalDumpUtlComnCpuRegAddr,
                 paddr, value);

    return CPSS_HAL_DUMPOK;
}

int cpssHalDumpUtlWriteCpuReg(uint32_t regAddr, uint32_t value)
{
    int  volatile *paddr;
    paddr = (int *)(((void *) cpssHalDumpUtlComnCpuRegAddr) + regAddr);
    *paddr = value;
    return CPSS_HAL_DUMPOK;
}

int cpssHalDumpUtlReadCpuRange(uint32_t regAddr, uint32_t len)
{
    uint32_t      buff[64], i, offset = 0;

    while (len > 0)
    {
        for (i = 0; i < 64; i++)
        {
            buff[i] = *(GT_U32 *)(((volatile void *) cpssHalDumpUtlComnCpuRegAddr) + regAddr
                                  + i*4 + offset);
        }

        for (i = 0; i < 64; i += 4)
        {
            cpssOsPrintf("%08x:  %08x %08x %08x %08x\n", regAddr + i*4 + offset, buff[i],
                         buff[i + 1], buff[i + 2], buff[i + 3]);
        }
        if (len > 256)
        {
            len -= 256;
        }
        else
        {
            len = 0;
        }
        offset += 256;
    }
    return CPSS_HAL_DUMPOK;
}

int cpssHalDumpUtlReadMibCnt(uint32_t port)
{
    CPSS_PORT_MAC_COUNTER_SET_STC   port_counters;
    int                   rc;

    rc = cpssDxChPortMacCountersOnPortGet((GT_U8)0, (GT_PHYSICAL_PORT_NUM)port,
                                          &port_counters);
    if (rc != GT_OK)
    {
        return (CPSS_HAL_DUMPERROR);
    }
    cpssOsPrintf("ucPktsRcv           = %d\n", port_counters.ucPktsRcv.l[0]);
    cpssOsPrintf("brdcPktsRcv         = %d\n", port_counters.brdcPktsRcv.l[0]);
    cpssOsPrintf("mcPktsRcv           = %d\n", port_counters.mcPktsRcv.l[0]);
    cpssOsPrintf("ucPktsSent          = %d\n", port_counters.ucPktsSent.l[0]);
    cpssOsPrintf("brdcPktsSent        = %d\n", port_counters.brdcPktsSent.l[0]);
    cpssOsPrintf("mcPktsSent          = %d\n", port_counters.mcPktsSent.l[0]);
    cpssOsPrintf("goodOctetsRcv       = %d\n", port_counters.goodOctetsRcv.l[0]);
    cpssOsPrintf("goodOctetsSent      = %d\n", port_counters.goodOctetsSent.l[0]);
    cpssOsPrintf("badPktsRcv          = %d\n", port_counters.badPktsRcv.l[0]);
    cpssOsPrintf("collisions          = %d\n", port_counters.collisions.l[0]);
    cpssOsPrintf("excessiveCollisions = %d\n",
                 port_counters.excessiveCollisions.l[0]);
    cpssOsPrintf("lateCollisions      = %d\n", port_counters.lateCollisions.l[0]);
    cpssOsPrintf("badCrc              = %d\n", port_counters.badCrc.l[0]);
    cpssOsPrintf("dropEvents          = %d\n", port_counters.dropEvents.l[0]);
    cpssOsPrintf("macTransmitErr      = %d\n", port_counters.macTransmitErr.l[0]);
    cpssOsPrintf("undersizePkts       = %d\n", port_counters.undersizePkts.l[0]);
    cpssOsPrintf("oversizePkts        = %d\n", port_counters.oversizePkts.l[0]);

    return CPSS_HAL_DUMPOK;
}



int cpssHalDumpUtlDbg(void)
{
    cpssOsPrintf("List of hw debug commands:\n");
    cpssOsPrintf("cpssHalDumpUtlReadPpReg    - read hw reg (regAddr)\n");
    cpssOsPrintf("cpssHalDumpUtlWritePpReg   - write hw reg (regAddr, value)\n");
    cpssOsPrintf("cpssHalDumpUtlReadPpRange   - read hw regs (regAddr, len)\n");
    cpssOsPrintf("cpssHalDumpUtlReadCpuReg   - read cpu hw reg (regAddr)\n");
    cpssOsPrintf("cpssHalDumpUtlReadCpuRange - read cpu hw regs (regAddr, len)\n");
    cpssOsPrintf("cpssHalDumpUtlWriteCpuReg  - write cpu hw reg (regAddr, value)\n");
    cpssOsPrintf("cpssHalDumpUtlReadPhyReg   - read phy hw reg (port, page, address)\n");
    cpssOsPrintf("cpssHalDumpUtlWritePhyReg  - write phy hw reg (port, page, address, value)\n");
    cpssOsPrintf("cpssHalDumpUtlReadMibCnt   - read port mib counters( port)\n");

    return CPSS_HAL_DUMPOK;
}

/**********************************************************************************************/


void cpssHalDumpUtlCPUCodesDump(uint32_t devId, CPSS_DXCH_TABLE_ENT tableType,
                                uint32_t startIndex, uint32_t endIndex)
{

    GT_STATUS rc = GT_OK;
    GT_U32     hwData[6];
    CPUCodeDisplayTable CPUCodeInfo;
    CPSS_NET_RX_CPU_CODE_ENT reasonCode;
    GT_U32 numEntries = 0;

    rc = prvCpssDxChTableNumEntriesGet(devId, tableType, &numEntries);
    cpssOsPrintf("%d\n", numEntries);
    if (rc != GT_OK)
    {
        return;
    }
    if (endIndex > numEntries)
    {
        endIndex = numEntries;
    }

    if (startIndex >= endIndex)
    {
        endIndex = numEntries;
    }
    cpssOsPrintf("\nSize of structure is %d\n", (uint32_t)sizeof(CPUCodeInfo));
    cpssOsPrintf("========================================================================================================\n");
    cpssOsPrintf("CPU Code Table\n");
    cpssOsPrintf("========================================================================================================\n");
    cpssOsPrintf("%4s %4s %4s %10s %13s %18s %26s %15s\n", "RC", "TC", "DP",
                 "Truncated", "TargetDevId", "rateLimitingMode", "statisticalRateLimiterId",
                 "rateLimiterId");
    cpssOsPrintf("--------------------------------------------------------------------------------------------------------\n");
    for (; startIndex < endIndex; startIndex++)
    {
        rc = prvCpssDxChReadTableEntry(devId, tableType, (GT_U32)startIndex, hwData);
        if (rc != GT_OK)
        {
            cpssOsPrintf(" cheetah FDB read ERROR : index [%5.5d] \n", startIndex);
            continue;
        }

        memcpy(&CPUCodeInfo, hwData, sizeof(CPUCodeInfo));
        rc=prvCpssDxChNetIfDsaToCpuCode((PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT)
                                        startIndex, &reasonCode);

        cpssOsPrintf("%4d %4d %4d %10d %13d %18d %26d %15d\n", reasonCode,
                     CPUCodeInfo.trafficClass, CPUCodeInfo.dropPrecedence, CPUCodeInfo.truncated,
                     CPUCodeInfo.targetDeviceIndex, CPUCodeInfo.rateLimitingMode,
                     CPUCodeInfo.statisticalRateLimiterIndex, CPUCodeInfo.rateLimiterIndex);
    }
    cpssOsPrintf("\n======================================================================================================\n");

}

void cpssHalDumpUtlRateLimiterDump(uint32_t devId,
                                   CPSS_DXCH_TABLE_ENT tableType, uint32_t startIndex, uint32_t endIndex)
{

    GT_STATUS rc = GT_OK;
    GT_U32     hwData[6];
    rateLimiterConfigDisplayTable rateLimiterInfo;
    GT_U32 numEntries = 0;

    rc = prvCpssDxChTableNumEntriesGet(devId, tableType, &numEntries);
    cpssOsPrintf("%d\n", numEntries);
    if (rc != GT_OK)
    {
        return;
    }
    if (endIndex > numEntries)
    {
        endIndex = numEntries;
    }

    if (startIndex >= endIndex)
    {
        endIndex = numEntries;
    }
    cpssOsPrintf("\nSize of structure is %d\n", (uint32_t)sizeof(rateLimiterInfo));
    cpssOsPrintf("===================================\n");
    cpssOsPrintf("Rate Limiter Table\n");
    cpssOsPrintf("===================================\n");
    cpssOsPrintf("%12s %15s %19s\n", "RateLimIdx", "LimiterPktNum",
                 "LimiterWindowSize");
    cpssOsPrintf("---------------------------------------------------------------------------\n");
    for (; startIndex <= endIndex; startIndex++)
    {
        rc = prvCpssDxChReadTableEntry(devId, tableType, (GT_U32)startIndex, hwData);
        if (rc != GT_OK)
        {
            cpssOsPrintf(" cheetah FDB read ERROR : index [%5.5d] \n", startIndex);
            continue;
        }

        memcpy(&rateLimiterInfo, hwData, sizeof(rateLimiterInfo));

        cpssOsPrintf("%10d %15d %19d  \n", startIndex, rateLimiterInfo.limiterPktNum,
                     rateLimiterInfo.limiterWindowSize);
    }
    cpssOsPrintf("\n=================================\n");

}
int cpssHalDumpUtlDumpVlan(int vlan)
{
    uint8_t  device_number;
    int                                ret;
    CPSS_PORTS_BMP_STC                   portsMembers;
    CPSS_PORTS_BMP_STC                   portsTagging;
    CPSS_DXCH_BRG_VLAN_INFO_STC          vlanInfo;
    GT_BOOL                              isValid;
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd;

    GT_U32     port                                 = 0;  /* current port number */
    CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT currTagCmd = 0 ;

    memset(&portsMembers, 0, sizeof(portsMembers));
    memset(&portsTagging, 0, sizeof(portsTagging));
    memset(&vlanInfo, 0, sizeof(vlanInfo));
    memset(&portsTaggingCmd, 0, sizeof(portsTaggingCmd));

    ret = cpssHalDumpUtlGetDeviceNumber(&device_number);
    if (ret != CPSS_HAL_DUMPOK)
    {
        cpssOsPrintf("%s:%d:%d Invalid dev \n",
                     __func__, __LINE__, ret);
        return CPSS_HAL_DUMPERROR;
    }

    if ((ret = cpssDxChBrgVlanEntryRead((GT_U8) device_number, (GT_U16)vlan,
                                        &portsMembers,
                                        &portsTagging, &vlanInfo, &isValid,
                                        &portsTaggingCmd)) != GT_OK)
    {
        return CPSS_HAL_DUMPERROR;
    }
    if (!isValid)
    {
        cpssOsPrintf("Invalid Vlan\n");
        return CPSS_HAL_DUMPERROR;
    }
    cpssOsPrintf("\n------------------------------------------------------------------");
    cpssOsPrintf("\n802.1Q Tag             Port members                    ");
    cpssOsPrintf("\n------------------------------------------------------------------");
    cpssOsPrintf("\n%9d\t", vlan);
    for (port = 0; port < CPSS_MAX_PORTS_BMP_NUM_CNS; port++)
    {
        if (CPSS_PORTS_BMP_IS_PORT_SET_MAC(&portsMembers, port))
        {
            cpssOsPrintf("%d,", port);
        }
    }
    /* cpssOsPrintf("\n------------------------------------------------------------------");
     cpssOsPrintf("\n  Port tagged                    ");
     cpssOsPrintf("\n------------------------------------------------------------------\n");
     for (port = 0; port < CPSS_MAX_PORTS_BMP_NUM_CNS; port++) {
      if (CPSS_PORTS_BMP_IS_PORT_SET_MAC(&portsTagging, port))
        cpssOsPrintf("%d,",port);
     }*/

    /* cpssOsPrintf("\n%9d\t\t%s\t\t\t%s\n", vlan, str_tpbmp, str_upbmp);*/

    cpssOsPrintf("\n-----------------------------------------------------\n");
    cpssOsPrintf("vlan Info\n");
    cpssOsPrintf("-----------------------------------------------------\n");


    cpssOsPrintf("unkSrcAddrSecBreach  =%d  ", vlanInfo.unkSrcAddrSecBreach);
    cpssOsPrintf("  ipv4UcastRouteEn     =%d\n", vlanInfo.ipv4UcastRouteEn);
    cpssOsPrintf("unregNonIpMcastCmd   =%d  ", vlanInfo.unregNonIpMcastCmd);
    cpssOsPrintf("  ipv4McastRouteEn     =%d\n", vlanInfo.ipv4McastRouteEn);
    cpssOsPrintf("unregIpv4McastCmd    =%d  ", vlanInfo.unregIpv4McastCmd);
    cpssOsPrintf("  ipv6UcastRouteEn     =%d\n", vlanInfo.ipv6UcastRouteEn);
    cpssOsPrintf("unregIpv6McastCmd    =%d  ", vlanInfo.unregIpv6McastCmd);
    cpssOsPrintf("  ipv6McastRouteEn     =%d\n", vlanInfo.ipv6McastRouteEn);
    cpssOsPrintf("unkUcastCmd          =%d  ", vlanInfo.unkUcastCmd);
    cpssOsPrintf("  stgId                =%d\n", vlanInfo.stgId);
    cpssOsPrintf("unregIpv4BcastCmd    =%d  ", vlanInfo.unregIpv4BcastCmd);
    cpssOsPrintf("  autoLearnDisable     =%d\n", vlanInfo.autoLearnDisable);
    cpssOsPrintf("unregNonIpv4BcastCmd =%d  ", vlanInfo.unregNonIpv4BcastCmd);
    cpssOsPrintf("  naMsgToCpuEn         =%d\n", vlanInfo.naMsgToCpuEn);
    cpssOsPrintf("ipv4IgmpToCpuEn      =%d  ", vlanInfo.ipv4IgmpToCpuEn);
    cpssOsPrintf("  mruIdx               =%d\n",
                 (int)vlanInfo.mruIdx);
    cpssOsPrintf("mirrToRxAnalyzerEn   =%d  ", vlanInfo.mirrToRxAnalyzerEn);
    cpssOsPrintf("  bcastUdpTrapMirrEn   =%d\n", vlanInfo.bcastUdpTrapMirrEn);
    cpssOsPrintf("ipv6IcmpToCpuEn      =%d  ", vlanInfo.ipv6IcmpToCpuEn);
    cpssOsPrintf("  vrfId                =%d\n",
                 (int)vlanInfo.vrfId);
    cpssOsPrintf("ipCtrlToCpuEn        =%d  ", vlanInfo.ipCtrlToCpuEn);
    cpssOsPrintf("  floodVidx            =%d\n", vlanInfo.floodVidx);
    cpssOsPrintf("ipv4IpmBrgMode       =%d  ", vlanInfo.ipv4IpmBrgMode);
    cpssOsPrintf("  floodVidxMode        =%d\n", vlanInfo.floodVidxMode);
    cpssOsPrintf("ipv6IpmBrgMode       =%d  ", vlanInfo.ipv6IpmBrgMode);
    cpssOsPrintf("  portIsolationMode    =%d\n", vlanInfo.portIsolationMode);
    cpssOsPrintf("ipv4IpmBrgEn         =%d  ", vlanInfo.ipv4IpmBrgEn);
    cpssOsPrintf("  ucastLocalSwitchingEn=%d\n", vlanInfo.ucastLocalSwitchingEn);
    cpssOsPrintf("ipv6IpmBrgEn         =%d  ", vlanInfo.ipv6IpmBrgEn);
    cpssOsPrintf("  mcastLocalSwitchingEn=%d\n", vlanInfo.mcastLocalSwitchingEn);
    cpssOsPrintf("ipv6SiteIdMode       =%d\n", vlanInfo.ipv6SiteIdMode);

    cpssOsPrintf("\n-----------------------------------------------------\n");

    cpssOsPrintf("Usage:\n");
    cpssOsPrintf("\n");
    cpssOsPrintf("%d Untagged     - ifTag0 and/or Tag1 were classified in the incoming packet,      \n",
                 CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E);
    cpssOsPrintf("                  they are removed from the packet.                               \n");
    cpssOsPrintf("%d Tag0         - Packet egress with Tag0 (Tag0 as defined in ingress pipe).      \n",
                 CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E);
    cpssOsPrintf("%d Tag1         - Packet egress with Tag1 (Tag1 as defined in ingress pipe).      \n",
                 CPSS_DXCH_BRG_VLAN_PORT_TAG1_CMD_E);
    cpssOsPrintf("%d Out_T0_in_T1 - Outer Tag0, Inner Tag1 (tag swap).                              \n",
                 CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E);
    cpssOsPrintf("%d Out_T1_in_T0 - Outer Tag1, Inner Tag0 (tag swap).                              \n",
                 CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG1_INNER_TAG0_CMD_E);
    cpssOsPrintf("%d Push_Tag0    - TAG0 is added to the packet regardless of whether Tag0 and TAG1 \n",
                 CPSS_DXCH_BRG_VLAN_PORT_PUSH_TAG0_CMD_E);
    cpssOsPrintf("                  were classified in the incoming packet.                         \n");
    cpssOsPrintf("%d Pop_Out_Tag  - The incoming packet outer tag is removed,                       \n",
                 CPSS_DXCH_BRG_VLAN_PORT_POP_OUTER_TAG_CMD_E);
    cpssOsPrintf("                  regardless of whether it is Tag0 or Tag1.                       \n");
    cpssOsPrintf("                  This operation is a NOP ifthe packet arrived with neither       \n");
    cpssOsPrintf("                  Tag0 nor Tag1 classified.                                       \n");
    cpssOsPrintf("\n-------------------------\n");
    cpssOsPrintf("Dev/Port Tagging commands\n");
    cpssOsPrintf("-------------------------\n");
    /*Macro iterates through all the ports on that device*/
    for (port = 0; port < 24; port++)
    {
        currTagCmd = portsTaggingCmd.portsCmd[port];
        cpssOsPrintf("  %d/%02d   %d\n", device_number, port, currTagCmd);
    }

    cpssOsPrintf("\n-------------------------\n");

    return CPSS_HAL_DUMPOK;
}

#define BC2_MAC_ENTRY_MAC_ADDR(arEther , macTablePtr) \
    arEther[5] = (GT_U8)((*macTablePtr     >> 19) & 0xFF);     \
    arEther[4] = (GT_U8)(((*macTablePtr     >> 27) & 0x1F)  |    \
                       (((*(macTablePtr+1) >> 0)  & 0x7)<<5));\
    arEther[3] = (GT_U8)((*(macTablePtr+1) >>  3) & 0xFF);     \
    arEther[2] = (GT_U8)((*(macTablePtr+1) >>  11) & 0xFF);     \
    arEther[1] = (GT_U8)((*(macTablePtr+1) >> 19) & 0xFF);     \
    arEther[0] = (GT_U8)(((*(macTablePtr+1) >> 27) & 0x1F) |    \
                         (((*(macTablePtr+2) >> 0)  & 0x3)<<5));

#define BC2_MAC_ENTRY_VALID(p)       (((*(((GT_U32 *)p)+0)        & 0x0001     ))      )
#define BC2_MAC_ENTRY_VID(p)         (((*(((GT_U32 *)p)+0) >> 6)  & 0x1fff     ))
#define BC2_MAC_ENTRY_SKIP_BIT(p)    (((*(((GT_U32 *)p)+0) >> 1)  & 0x00000001 ))
#define BC2_MAC_ENTRY_AGE_BIT(p)     (((*(((GT_U32 *)p)+0)        & 0x0004     )) >> 2 )

#define BC2_MAC_ENTRY_TRUNK(p)       (((*(((GT_U32 *)p)+2)        & (1<<19)    )) >> 19)
#define BC2_MAC_ENTRY_PORT_TRUNK(p)  (((*(((GT_U32 *)p)+2)        & (0xFFF<<20) )) >> 20)
#define BC2_MAC_ENTRY_USER_DEF(p)    (((*(((GT_U32 *)p)+3) >> 3) & 0x1f ))
#define BC2_MAC_ENTRY_VIDX(p)        (((*(((GT_U32 *)p)+2) >> 19) & 0x1fff) |\
                                             (((*(((GT_U32 *)p)+3) >>  0) & 0x7 )<< 13))
#define BC2_MAC_ENTRY_STATIC(p)      (((*(((GT_U32 *)p)+3)        & (1<<23)    )) >> 23)

#define BC2_MAC_ENTRY_SRC_TC(p)      -1
#define BC2_MAC_ENTRY_DST_TC(p)      -1
#define BC2_MAC_ENTRY_MULTIPLE(p)    (((*(((GT_U32 *)p)+3) >> 24) & 0x1 ))
#define BC2_MAC_ENTRY_DA_CMD(p)      (((*(((GT_U32 *)p)+3) >> 25) & 0x7 ))
#define BC2_MAC_ENTRY_SA_CMD(p)      (((*(((GT_U32 *)p)+3) >> 28) & 0x7 ))

#define BC2_FDB_MAC_ENTRY_WORDS_SIZE_CNS   5
#define U32_GET_FIELD(data,offset,length)           \
        (((data) >> (offset)) & ((1 << (length)) - 1))

void cpssHalDumpUtlFdbDump
(
    GT_BOOL full
)
{
    GT_STATUS  rc;
    GT_U32     hwData[BC2_FDB_MAC_ENTRY_WORDS_SIZE_CNS];
    GT_U32     entryIndex;
    GT_BOOL    tableEmpty = GT_TRUE;
    GT_U32     numOfEntry = 0;
    GT_U32     numOfSkip = 0;
    GT_U32     numOfValid = 0;
    GT_U32     numOfAged = 0;
    GT_U32     numOfStormPrevention = 0;
    static     GT_BOOL printHelp = GT_TRUE;
    /*GT_U32     macEntryType;*/
    GT_U32     valid;
    GT_U32     vid;
    GT_U32     skip;
    GT_U32     age;
    GT_U32     isTrunk;
    GT_U32     portTrunk;
    GT_U32     userDefined;
    GT_U32     vidx;
    GT_U32     isStatic;
    GT_U32     multi;
    GT_U32     daCmd;
    GT_U32     saCmd;
    GT_U32     spUnknown;
    GT_U32     saQosProfileIndex;
    GT_U32     daQosProfileIndex;
    GT_U32     mirrorToAnalyzerPort;
    GT_U32     daAccessLevel;
    GT_U32     saAccessLevel;
    GT_U32     daRoute;
    GT_U32     devId;
    GT_U32     srcId;
    GT_U8      mac[6];
    GT_BOOL    useVidx;
    GT_U8      cpssDevNum;
    int        ret;
    CPSS_MAC_ENTRY_EXT_STC  entry;
    GT_HW_DEV_NUM           hwDevNum;
    GT_U32     tblSize;

    ret = cpssHalDumpUtlGetDeviceNumber(&cpssDevNum);
    if (ret != CPSS_HAL_DUMPOK)
    {
        cpssOsPrintf("%s:%d:%d: Invalid dev \n",
                     __func__, __LINE__, ret);
        return;
    }

    cpssOsPrintf(" === HW MAC TABLE READ dev %d (address N/A) ===\n", cpssDevNum);

    rc = cpssDxChCfgTableNumEntriesGet(cpssDevNum, CPSS_DXCH_CFG_TABLE_FDB_E,
                                       &tblSize);
    if (rc != GT_OK)
    {
        cpssOsPrintf(" FDB TABLE Entries Read ERROR, ret Val = %d\n", rc);
        return;
    }

    for (entryIndex = 0 ;
         entryIndex < tblSize;
         entryIndex++)
    {
        rc = cpssDxChBrgFdbMacEntryRead(cpssDevNum, entryIndex, &valid, &skip, &age,
                                        &hwDevNum,  &entry);
        if (rc != GT_OK)
        {
            break;
        }

        /*Ignore entries which are not MAC address type */
        if (entry.key.entryType != CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E)
        {
            continue;
        }

        rc = prvCpssDxChReadTableEntry(cpssDevNum,
                                       CPSS_DXCH_TABLE_FDB_E,
                                       entryIndex,
                                       &hwData[0]);
        if (rc != GT_OK)
        {
            cpssOsPrintf(" cheetah FDB read ERROR : index [%5.5d] \n", entryIndex);
            continue;
        }

        if (full == GT_TRUE)
        {
            cpssOsPrintf("%5.5d)  %8.8x  %8.8x  %8.8x  %8.8x \n"
                         , entryIndex
                         , hwData[0]
                         , hwData[1]
                         , hwData[2]
                         , hwData[3]);
        }

        valid  = BC2_MAC_ENTRY_VALID(&hwData[0]);
        vid    = BC2_MAC_ENTRY_VID(&hwData[0]);
        skip   = BC2_MAC_ENTRY_SKIP_BIT(&hwData[0]);
        age    = BC2_MAC_ENTRY_AGE_BIT(&hwData[0]);
        spUnknown = U32_GET_FIELD(hwData[4], 0, 1);


        if (valid == 0x1 && age == 0x0)
        {
            ++numOfAged ;
        }

        if (valid == 0x1)
        {
            if (skip == 0x0)
            {
                ++numOfEntry;

                if (spUnknown  == 0x1)
                {
                    ++numOfStormPrevention;
                }
            }
            else
            {
                ++numOfSkip ;
            }
            ++numOfValid ;
        }

        if (valid == 0x1 &&
            skip == 0x0 &&
            numOfEntry < 1000)
        {
            /*macEntryType = U32_GET_FIELD(hwData[0],3,3);*/

            daCmd        = BC2_MAC_ENTRY_DA_CMD(&hwData[0]);
            saCmd        = BC2_MAC_ENTRY_SA_CMD(&hwData[0]);
            isStatic     = BC2_MAC_ENTRY_STATIC(&hwData[0]);

            isTrunk      = BC2_MAC_ENTRY_TRUNK(&hwData[0]);
            portTrunk    = BC2_MAC_ENTRY_PORT_TRUNK(&hwData[0]);
            userDefined  = BC2_MAC_ENTRY_USER_DEF(&hwData[0]);
            vidx         = BC2_MAC_ENTRY_VIDX(&hwData[0]);
            isStatic     = BC2_MAC_ENTRY_STATIC(&hwData[0]);

            multi        = BC2_MAC_ENTRY_MULTIPLE(&hwData[0]);
            daCmd        = BC2_MAC_ENTRY_DA_CMD(&hwData[0]);
            saCmd        = BC2_MAC_ENTRY_SA_CMD(&hwData[0]);

            BC2_MAC_ENTRY_MAC_ADDR(mac, &hwData[0]);

            devId        = U32_GET_FIELD(hwData[2], 3, 10);
            srcId        = U32_GET_FIELD(hwData[2], 13, 6);

            saQosProfileIndex = U32_GET_FIELD(hwData[4], 1, 3);
            daQosProfileIndex = U32_GET_FIELD(hwData[4], 4, 3);
            mirrorToAnalyzerPort = U32_GET_FIELD(hwData[4], 9, 1);
            daAccessLevel = U32_GET_FIELD(hwData[3], 11, 3);
            saAccessLevel = U32_GET_FIELD(hwData[3], 14, 3);

            daRoute = U32_GET_FIELD(hwData[3], 31, 1);

            useVidx = ((mac[0] & 1) || multi) ? GT_TRUE : GT_FALSE;

            if (useVidx == GT_TRUE)
            {
                portTrunk = 999;
                userDefined = 0;
            }
            else
            {
                vidx = 0;
            }

            if (tableEmpty == GT_TRUE)
            {
                cpssOsPrintf("      )v sk ag  Vid     MAC             dv sId tr p_t sp dQ sQ Vidx  S m dC sC A dL sL U R\n");
            }
            tableEmpty = GT_FALSE;

            cpssOsPrintf("%6.6d)%d  %d  %d  0x%3.3x "          /* last vid */
                         "%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x "/* last mac */
                         "%2.2d %2.2d  %s  %3.3d %d  "         /* last spUnknown */
                         "%d  %d  0x%3.3x %d "                 /* last st */
                         "%s %d  %d  %d %d  %d  %d %d\n"
                         , entryIndex
                         , valid
                         , skip
                         , age
                         , vid
                         , mac[0]
                         , mac[1]
                         , mac[2]
                         , mac[3]
                         , mac[4]
                         , mac[5]
                         , devId
                         , srcId
                         , useVidx == GT_FALSE ? (isTrunk ? "T" : "P") : "X"
                         , portTrunk
                         , spUnknown
                         , saQosProfileIndex
                         , daQosProfileIndex
                         , vidx
                         , isStatic
                         , multi ? "M" : "-"
                         , daCmd
                         , saCmd
                         , mirrorToAnalyzerPort
                         , daAccessLevel
                         , saAccessLevel
                         , userDefined
                         , daRoute);
        }
    }

    if (tableEmpty == GT_TRUE)
    {
        cpssOsPrintf("MAC table Empty. \n");
    }

    cpssOsPrintf("End of table - num of entries : %d , %d valid ,%d skip ,%d Aged , %d sp\n"
                 , numOfEntry
                 , numOfValid
                 , numOfSkip
                 , numOfAged
                 , numOfStormPrevention);

    if (numOfEntry > 1000)
    {
        cpssOsPrintf("=== Display only the first 1000 entries ===\n");
    }

    if (printHelp == GT_TRUE)
    {
        printHelp = GT_FALSE;
        cpssOsPrintf("v    :  valid bit\n");
        cpssOsPrintf("sk   :  skip bit\n");
        cpssOsPrintf("ag   :  aging bit\n");
        cpssOsPrintf("Vid  :  vid \n");
        cpssOsPrintf("MAC  :  macAddress \n");
        cpssOsPrintf("dv   :  device number \n");
        cpssOsPrintf("sId  :  source id \n");
        cpssOsPrintf("tr   :  is trunk --- P port , T trunk , X - vidx \n");
        cpssOsPrintf("p_t  :  port or Trunk number (999 when vidx used) \n");
        cpssOsPrintf("sp   :  storm prevention \n");
        cpssOsPrintf("sQ   :  source QoS profile index \n");
        cpssOsPrintf("dQ   :  destination QoS profile index \n");
        cpssOsPrintf("Vidx :  vidx \n");
        cpssOsPrintf("S    :  static \n");
        cpssOsPrintf("m    :  multiple \n");
        cpssOsPrintf("dC   :  daCmd \n");
        cpssOsPrintf("sC   :  saCmd \n");
        cpssOsPrintf("A    :  mirror to Rx Analyzer port \n");
        cpssOsPrintf("dL   :  DA AccessLevel \n");
        cpssOsPrintf("sL   :  SA AccessLevel \n");
        cpssOsPrintf("U    :  user defined value \n");
        cpssOsPrintf("R    :  DA Routing \n");
        cpssOsPrintf("\n");
    }
    return;
}

int cpssHalDumpUtlFdbCount
(
    uint32_t *usedEntries
)
{
    GT_STATUS  rc;
    GT_U8      cpssDevNum;
    int        ret;
    GT_U32     bankIndex;
    GT_U32     value, count = 0;

    ret = cpssHalDumpUtlGetDeviceNumber(&cpssDevNum);
    if (ret != CPSS_HAL_DUMPOK)
    {
        cpssOsPrintf("%s:%d:%d Invalid dev \n", __func__, __LINE__, ret);
        return CPSS_HAL_DUMPERROR;
    }
    for (bankIndex = 0; bankIndex < 15; bankIndex++)
    {
        rc = cpssDxChBrgFdbBankCounterValueGet(cpssDevNum, bankIndex, &value);
        if (rc != GT_OK)
        {
            cpssOsPrintf(" FDB TABLE Entries Read ERROR, ret Val = %d\n", rc);
            return rc;
        }
        count = count + value;
    }
    *usedEntries = count;
    //cpssOsPrintf("FDB table num of entries : %d \n", count);
    return CPSS_HAL_DUMPOK;
}

CPSS_HAL_DUMPSTATUS cpssHalDumpHalGroupVidxGet(
    GT_U8               cpssDevNum,
    GT_U16              vidx,
    CPSS_PORTS_BMP_STC  *portBitmapPtr
)
{
    GT_STATUS rc;

    rc= cpssDxChBrgMcEntryRead(cpssDevNum, vidx, portBitmapPtr);
    if (rc != GT_OK)
    {
        cpssOsPrintf("%s:%d: Read vidx %d failed \n",
                     __func__, __LINE__, vidx);
        return CPSS_HAL_DUMPFAIL;
    }
    return CPSS_HAL_DUMPOK;
}


CPSS_HAL_DUMPSTATUS cpssHalDumpUtlVidxDump
(
    void
)
{
    GT_U16      vidx, port;
    GT_U8       cpssDevNum;
    CPSS_HAL_DUMPSTATUS status;
    CPSS_PORTS_BMP_STC  portBitmap;

    status = cpssHalDumpUtlGetDeviceNumber(&cpssDevNum);
    if (status != CPSS_HAL_DUMPOK)
    {
        cpssOsPrintf("%s:%d: Invalid dev \n",
                     __func__, __LINE__);
        return CPSS_HAL_DUMPERROR;
    }
    cpssOsPrintf("|  vidx  | ports \n");
    cpssOsPrintf("-----------------\n");

    for (vidx = 0; vidx < CPSS_HAL_DUMPGROUP_VIDX_MAX_SIZE - 1; vidx++)
    {
        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portBitmap);
        status = cpssHalDumpHalGroupVidxGet(cpssDevNum, vidx, &portBitmap);
        if (status != CPSS_HAL_DUMPOK)
        {
            return CPSS_HAL_DUMPFAIL;
        }
        if (CPSS_PORTS_BMP_IS_ZERO_MAC(&portBitmap))
        {
            continue;
        }
        cpssOsPrintf("| %6d | ", vidx);
        for (port = 0 ; port < CPSS_MAX_PORTS_NUM_CNS; port++)
        {
            if (CPSS_PORTS_BMP_IS_PORT_SET_MAC(&portBitmap, port))
            {
                cpssOsPrintf("%d, ", port);
            }
        }
        cpssOsPrintf("\n");
    }
    return CPSS_HAL_DUMPOK;
}

CPSS_HAL_DUMPSTATUS cpssHalDumpUtlMacToMeDump
(
    void
)
{
    GT_U8                                           cpssDevNum;
    GT_U32                                          entryIndex;
    CPSS_DXCH_TTI_MAC_VLAN_STC                      value;
    CPSS_DXCH_TTI_MAC_VLAN_STC                      mask, empty;
    CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC  interfaceInfo;
    GT_STATUS   rc;
    CPSS_HAL_DUMPSTATUS  status;

    status = cpssHalDumpUtlGetDeviceNumber(&cpssDevNum);
    if (status != CPSS_HAL_DUMPOK)
    {
        cpssOsPrintf("%s:%d: Invalid dev \n",
                     __func__, __LINE__);
        return CPSS_HAL_DUMPERROR;
    }
    memset(&empty, 0, sizeof(CPSS_DXCH_TTI_MAC_VLAN_STC));
    cpssOsPrintf("| idx |    mac address    |      mac mask     |  vlan | mask |   interface\n");
    cpssOsPrintf("---------------------------------------------------------------------------------------\n");
    for (entryIndex = 0; entryIndex < CPSS_HAL_DUMPHASH_TERMINATION_TABLE_SIZE;
         entryIndex++)
    {
        rc = cpssDxChTtiMacToMeGet(cpssDevNum, entryIndex, &value, &mask,
                                   &interfaceInfo);
        if (rc != GT_OK)
        {
            return CPSS_HAL_DUMPFAIL;
        }
        if (memcmp(&value.mac, &empty.mac, sizeof(GT_ETHERADDR)) != 0)
        {
            /* entry is not empty */
            cpssOsPrintf("| %3d | %02x:%02x:%02x:%02x:%02x:%02x | %02x:%02x:%02x:%02x:%02x:%02x | %5d | %04x |",
                         entryIndex,
                         value.mac.arEther[0], value.mac.arEther[1], value.mac.arEther[2],
                         value.mac.arEther[3], value.mac.arEther[4], value.mac.arEther[5],
                         mask.mac.arEther[0], mask.mac.arEther[1], mask.mac.arEther[2],
                         mask.mac.arEther[3], mask.mac.arEther[4], mask.mac.arEther[5],
                         value.vlanId, mask.vlanId);
            if (interfaceInfo.includeSrcInterface ==
                CPSS_DXCH_TTI_DONT_USE_SRC_INTERFACE_FIELDS_E)
            {
                cpssOsPrintf(" none\n");
            }
            else
            {
                cpssOsPrintf(" dev:%d port:%d isTrunk:%d\n",
                             interfaceInfo.srcHwDevice, interfaceInfo.srcPortTrunk,
                             interfaceInfo.srcIsTrunk);
            }
        }
    }
    return CPSS_HAL_DUMPOK;
}


CPSS_HAL_DUMPSTATUS cpssHalDumpUtlGetFirstZeroBit(uint32_t word,
                                                  uint32_t *bitIndex)
{
    uint32_t val = ~word;

    val = ffs((int)val);
    if (!val)
    {
        return CPSS_HAL_DUMPFAIL;
    }
    else
    {
        *bitIndex = val -1;
        return CPSS_HAL_DUMPOK;
    }

}


#if BYTE_ORDER == BIG_ENDIAN_MEM_FORMAT
#define RSG_swap32l_MAC(val)   ( RSG_swap(val) )
#else
#define RSG_swap32l_MAC(val)   (val)
#endif

bool cpssHalDumpUtlIsIpUnicast(in_addr_t ipAddr)
{
    unsigned char *ip_addr_vec = (unsigned char*)&ipAddr;

    return (
               (ip_addr_vec[0] != 127) &&
               (ip_addr_vec[0] > 0) &&
               (ip_addr_vec[0] < 224)
           );
}

bool cpssHalDumpUtlIsIpv6Unicast(struct in6_addr *ip6Addr)
{

    if (((ip6Addr->s6_addr[0] | ip6Addr->s6_addr[1] | ip6Addr->s6_addr[2] |
          ip6Addr->s6_addr[3]) == 0) ||
        (ip6Addr->s6_addr[0] == 0xFF))
    {
        return false;
    }
    return true;
}

/* Not re-entrant !! */
char* cpssHalDumpUtlIp2Str(in_addr_t ipAddr)
{
    static char str[16];
    unsigned char *ip_addr_vec = (unsigned char*)&ipAddr;

    sprintf(str, "%d.%d.%d.%d", ip_addr_vec[0], ip_addr_vec[1], ip_addr_vec[2],
            ip_addr_vec[3]);
    return str;
}

char *cpssHalDumpUtlIpPrefix2Str(uint32_t type, uint8_t *value)
{
    static char str[CPSS_HAL_DUMPIP_STR_MAX_LEN];
    str[0] = '\0';

    inet_ntop(CPSS_HAL_DUMPIP_TYPE_TO_AF_TYPE(type),
              value,
              str,
              CPSS_HAL_DUMPIP_STR_MAX_LEN);

    return str;
}

uint32_t cpssHalDumpUtlIpmask2Prefixlen(in_addr_t ipMask)
{
    uint32_t prefixLen;

    prefixLen = (uint32_t)ffs(htonl((int)ipMask));
    /* if no set bit found - prefixLen remains 0 */
    if (prefixLen)
    {
        prefixLen = MAX_IP_PREFIX +1 -prefixLen;
    }

    return prefixLen;
}

in_addr_t cpssHalDumpUtlPrefixlen2Ipmask(uint32_t prefix)
{
    in_addr_t ip = 0;
    uint32_t i;

    for (i = 0; i < prefix; i++)
    {
        ip |= 1<<(31-i);
    }

    return ip;
}

uint32_t cpssHalDumpUtlIpv6mask2Prefixlen(struct in6_addr *ip6Mask)
{
    uint32_t prefixLen = 0, currPrefixLen;
    uint32_t i = 0;
    for (i=0; i<CPSS_HAL_DUMPIPV6_ADDRESS_SIZE; i++)
    {
        currPrefixLen = (uint32_t)ffs(ip6Mask->s6_addr[i]);
        if (currPrefixLen != 0)
        {
            prefixLen += (IPV6_BYTE_PREFIX + 1 - currPrefixLen);
        }
        if (currPrefixLen != 1)
        {
            break;
        }
    }
    return prefixLen;
}

void cpssHalDumpUtlPrefixlen2Ipv6mask(uint32_t prefix, struct in6_addr *ip6Mask)
{
    uint32_t bytes = prefix/8;
    uint32_t remBits = prefix%8;
    uint32_t i, j;

    for (i = 0; i < bytes; i++)
    {
        ip6Mask->s6_addr[i] = 0xff;
    }

    for (j = 0; j < remBits; j++)
    {
        ip6Mask->s6_addr[i] |= 1<<(7-j);
    }
}


bool cpssHalDumpUtlGetDeviceType(uint8_t dev, uint32_t *devTypePtr)
{
    CPSS_DXCH_CFG_DEV_INFO_STC  devInfo;
    GT_STATUS rc;

    rc = cpssDxChCfgDevInfoGet(dev, &devInfo);
    if (rc != GT_OK)
    {
        cpssOsPrintf("### cpssDxChCfgDevInfoGet dev %d failed 0x%X\n", dev, rc);
        return false;
    }
    if (devInfo.genDevInfo.devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
    {
        switch (devInfo.genDevInfo.devType)
        {
            case CPSS_ARMSTRONG_ALL_DEVICES_CASES_MAC:
                *devTypePtr = CPSS_HAL_DUMPUTILS_DEV_TYPE_ARMSTRONG;
                break;
            case CPSS_BOBCAT3_ALL_DEVICES_CASES_MAC:
                *devTypePtr = CPSS_HAL_DUMPUTILS_DEV_TYPE_BC3;
                break;
            default:
                return false;
        }
    }
    else if (devInfo.genDevInfo.devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E)
    {
        if (devInfo.genDevInfo.devSubFamily == CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E)
        {
            *devTypePtr = CPSS_HAL_DUMPUTILS_DEV_TYPE_BOBK;
        }
        else
        {
            *devTypePtr = CPSS_HAL_DUMPUTILS_DEV_TYPE_BC2;
        }
    }
    else
    {
        return false;
    }
    return true;
}

#define CPSS_HAL_DUMPACTUAL_NUM_OF_PORTS 12
#define CPSS_HAL_DUMPACTUAL_NUM_OF_PORTS_BC3 48

bool cpssHalDumpUtlGetSwitchNumOfPorts(uint32_t switchId,
                                       uint32_t *numOfPorts)
{
    *numOfPorts = cpssHalDumpSystemNumOfPorts;

    return true;
}

bool cpssHalDumpUtlConvertSwitchPortToDevPort(uint32_t switchId,
                                              uint32_t port,
                                              uint8_t  *devId,
                                              uint32_t *devPort)
{
    if (port >= cpssHalDumpSystemNumOfPorts)
    {
        cpssOsPrintf("%s:%d: CPSS HAL DUMP port number out of range: %d\n",
                     __func__, __LINE__, port);
        return false;
    }

    *devId = /*CPSS_HAL_DUMPDEV_NUM*/0;
    *devPort = cpssHalDumpPortToHalPort[port];

    if (port != cpssHalDumpHalPortToPort[*devPort])
    {
        cpssOsPrintf("%s:%d Incosistency between CPSS HAL DUMP port and HAL port conversion: In CPSS HAL DUMP port [%d], Out CPSS HAL DUMP port [%d], HAL port [%d]\n",
                     __func__, __LINE__, port, cpssHalDumpHalPortToPort[*devPort], *devPort);
        return false;
    }
    /*
        switch (cpssHalDumpDevType) {
        case CPSS_HAL_DUMPUTILS_DEV_TYPE_BOBK:
            if(*devPort >= CPSS_HAL_DUMPACTUAL_NUM_OF_PORTS) {
                return false;
            }
            break;
        case CPSS_HAL_DUMPUTILS_DEV_TYPE_BC3:
            if(*devPort >= CPSS_HAL_DUMPACTUAL_NUM_OF_PORTS_BC3) {
                return false;
            }
            break;
        default:
            cpssOsPrintf(                       "%s:%d: dev type %d not supported\n",
                           __func__, __LINE__, cpssHalDumpDevType);
            return false;
        }
    */
    return true;
}

bool cpssHalDumpUtlConvertDevPortToSwitchPort(uint8_t devId,
                                              uint32_t devPort,
                                              uint32_t *switchId,
                                              uint32_t *port)
{
    *switchId = CPSS_HAL_DUMPSWITCH_ID;
    if (devPort >= CPSS_HAL_DUMPMAX_NUM_OF_PORTS)
    {
        cpssOsPrintf("%s:%d HAL port number out of range: %d\n",
                     __func__, __LINE__, devPort);
        return false;
    }
    if (devPort == CPSS_HAL_DUMPOUTPUT_CPU_PORT)
    {
        *port = CPSS_HAL_DUMPOUTPUT_CPU_PORT;
        return true;
    }
    *port = cpssHalDumpHalPortToPort[devPort];

    if (devPort != cpssHalDumpPortToHalPort[*port])
    {
        cpssOsPrintf("%s:%d: Incosistency between CPSS HAL DUMP port and HAL port conversion: CPSS HAL DUMP port [%d], In HAL port [%d], Out HAL port [%d]\n",
                     __func__, __LINE__, *port, devPort, cpssHalDumpPortToHalPort[*port]);
        return false;
    }
    return true;
}

struct cpssHalDumpUtlDbgFunction
    cpssHalDumpUtlDbgPerModule[CPSS_HAL_DUMPUTL_DBG_LAST][CPSS_HAL_DUMPUTL_MAX_DBGS]
    =
{
    { /*  1 CPSS_HAL_DUMPUTL_DBG_SYSTEM_E */
        {"luaCLI", "starts LUA shell"},
    },
    { /* 2 CPSS_HAL_DUMPUTL_DBG_LOG_E */
        {"cpssHalDumpTraceShowModuleId", "print the modules debug status"},
        {"cpssHalDumpTraceSetModuleLevel", "en/dis debug and level per module (moduleId, enable, level)"},
        {"cpssHalDumpSysEnableCpuCodePrint", "print the cpu code for traped packets (enable)"},
        {"cpssHalDumpSysEnableBufPrint", "print the trapped packets data (enable)"},
    },
    { /* 3 CPSS_HAL_DUMPUTL_DBG_HW_E */
        {"cpssHalDumpUtlReadPpReg", "read hw reg (regAddr)"},
        {"cpssHalDumpUtlWritePpReg", "write hw reg (regAddr, value)"},
        {"cpssHalDumpUtlReadPpRange", "read hw regs range (regAddr, len)"},
        {"cpssHalDumpUtlReadCpuReg", "read cpu hw reg (regAddr)"},
        {"cpssHalDumpUtlReadCpuRange", "read cpu hw regs range(regAddr, len)"},
        {"cpssHalDumpUtlWriteCpuReg", "write cpu hw reg (regAddr, value)"},
        {"cpssHalDumpUtlReadPhyReg", "read PHY reg (port, page, regAddr)"},
        {"cpssHalDumpUtlWritePhyReg", "write PHY reg (port, page, regAddr, value)"},
        {"cpssHalDumpUtlReadMibCnt", "read PP MIB counters(port)"}
    },
    { /* 4 flow common */
        {"cpssHalDumpWrapFlowTableShowName", "show flow table name ()"},
        {"cpssHalDumpWrapFlowTableDump", "print flow table info (table: 0=vlan, 1=termination, 2-4=pcl 5=bridge, 6=L3 unicast)"},
        {"cpssHalDumpWrapFlowEntryDelete", "delete flow (table, cookie, matchingMode)"},
        {"cpssHalDumpWrapFlowTableCookieDelete", "delete flow with cookie (table, cookie)"},
        {"cpssHalDumpWrapFlowTableGetByCookie", "get flow with cookie (table, cookie)"},
        {"cpssHalDumpWrapFlowTableGetNext", "get next flow (Table,firstEntry,cookie)"},
        {"cpssHalDumpWrapFlowEntryStatisticsGet", "get flow statistics (table, cookie)"},
        {"cpssHalDumpWrapFlowTableStatisticsGet", "get table statistics (table)"},
        {"cpssHalDumpLibFlowTimerDump", "show flow timer list ()"},
    },
    { /* 5 flow vlan */
        {"cpssHalDumpWrapFlowEntryAddVlan", "Add flow to vlan table (cookie, port, vlanId, IsTag, hard, pvid, pcp, goto)"},
        {"cpssHalDumpWrapFlowEntryModifyVlan", "Modify flow (cookie, pvid, pcp, matchingMode)"},
        {"cpssHalDumpUtlDumpVlan", "Dump vlan entry (vlanID)"}
    },
    { /* 6 flow termination */
        {"cpssHalDumpWrapFlowEntryAddTerm", "Add flow to termination table (cookie, port, vlan, mac1, mac2, mac3, mac4, gotoTableNo)"},
        {"cpssHalDumpWrapFlowEntryModifyTerm", "Modify flow (cookie, metadataValue, metadataMask, matchingMode)"},
        {"cpssHalDumpUtlMacToMeDump", "Dump MacToMe table ()"}
    },
    { /* 7 flow ipcl */
        {"cpssHalDumpWrapFlowEntryBuildIpclPattern", "build ipcl flow pattern (cookie, field, value)"},
        {"cpssHalDumpWrapFlowBuildIpclActionAndSend", "set pcl flow action and send (stage, meterId, cntrl, group, queue, pcp, dscp, metadata, clear, gotoTbl)"},
        {"cpssHalDumpWrapFlowEntryModifyIpcl", "Modify flow (cookie, stage, meterId, controller, group, queue, vlanPcp, dscp, metadata, clear)"},
        {"cpssHalDumpHalIpclHwListDump", "print all pcl hw list (client 0-2)"},
        {"cpssHalDumpHalIpclHwRuleDump", "print pcl info (vTcamId,ruleId)"},
        {"cpssHalDumpHalQosDump", "print qos profile table(first, last) -1= all"},
        {"cpssHalDumpHalCncDebug", "print cnc table() "},
        {"cpssHalDumpHalCncDebugGetCounter", "Get cnc counter (dev, counterIndex)"}

    },

    { /* 8 flow bridge */
        {"cpssHalDumpWrapFlowEntryAddBridging", "Add flow to bridge table (cookie, vlanId, mac1, mac2, mac3, mac4, cntrl, group, clear, gotoTableNo)"},
        {"cpssHalDumpWrapFlowEntryModifyBridging", "Modify flow (cookie,cntrl,group,clear,matchingMode)"},
        {"cpssHalDumpUtlFdbDump", "dump fdb hw (full)"},
        {"cpssHalDumpUtlFdbCount", "count fdb entries in table ()"},
    },
    { /* 9 flow l3 */
        {"cpssHalDumpHalL3TableDump", "dump fdb hw ()"},
        {"cpssDxChIpLpmDump", "dumo lpm table (dev, vrId, protocol, prefixType)"},
    },
    { /* 10 meter */
        {"cpssHalDumpWrapMeterStatisticsGet", "Print meter statistics (meterId)"},
        {"cpssHalDumpMeterTableDump", "dump the meter table()"},
        {"cpssHalDumpMeterHwEntryPrint", "Print meter entry from hw(dev, meterId)"},
        {"cpssHalDumpHalPolicerMngCounterDump", "Print meter mng counters (dev, stage)"},
    },
    { /* 11 group common */
        {"cpssHalDumpGroupTableDump", "dump all group tables ()"},
        {"cpssHalDumpWrapGroupIdentifierParse", "Parse the 32 bit group identifier (groupId)"},
        {"cpssHalDumpWrapGroupIdentifierBuild", "Build the 32 bit group identifier (groupType, vlanId, portNum, multicastId, index)"},
        {"cpssHalDumpWrapGroupTableEntryAdd", "Add group to table (groupId, groupSemantics) 0-all 2- direct"},
        {"cpssHalDumpWrapGroupTableEntryDelete", "Delete group (groupId)"},
        {"cpssHalDumpWrapGroupTableGetNext", "Get next group (groupId)"},
        {"cpssHalDumpWrapGroupEntryStatisticsGet", "Get group statistics (groupId)"},
        {"cpssHalDumpWrapGroupEntryBucketDelete", "Delete bucket from group (groupId, bucketIndex) bucketIndex == -1 - delete all buckets"},
        {"cpssHalDumpWrapbGroupEntryBucketGet", "Get bucket info (groupId, bucketIndex)"},
    },
    { /* 12 group l2 interface */
        {"cpssHalDumpWrapGroupEntryL2InterfaceBucketAdd", "Add L2 interface bucket  (groupId, popVlan)"},
        {"cpssHalDumpWrapGroupEntryL2InterfaceBucketModify", "Modify L2 interface bucket (groupId, popVlan)"},
        {"cpssHalDumpWrapGroupEntryL2InterfaceAdd", "Create L2 interface group+bucket (groupId, popVlan)"},
    },
    { /* 13 group l2 multicast */
        {"cpssHalDumpWrapGroupEntryL2MulticastBucketAdd", "Add L2 multicast bucket  (groupId, index, refGroup)"},
        {"cpssHalDumpWrapGroupEntryL2MulticastBucketModify", "Modify L2 multicast bucket (groupId, index, refGroup)"},
        {"cpssHalDumpUtlVidxDump", "Dump vidx table ()"},
    },
    { /* 14 group l2 rewrite */
        {"cpssHalDumpWrapGroupEntryL2RewriteBucketAdd", "Add L2 rewrite bucket  (groupId, refGroup, vlan, srcMac, dstMac)"},
        {"cpssHalDumpWrapGroupEntryL2RewriteAdd", "Create L2 rewrite group+bucket (groupId, refGroup, vlan, srcMac, dstMac)"},
    },
    { /* 15 group l3 */
        {"cpssHalDumpWrapGroupEntryL3InterfaceAdd", "Add flow to l3 table (groupId, refGroup, vlan)"},
        {"cpssHalDumpWrapGroupEntryL3UnicastBucketAdd", "add flow bucket(groupId, refGroup, vlan)"},
    }
};


int debug(void)
{
    char line[100];
    int  num = 0;
    int  module_id = 0;
    int  i = 0;
    char * rc;


    while (1)
    {
        cpssOsPrintf("0.  Exit\n");
        cpssOsPrintf("1.  System\n");
        cpssOsPrintf("2.  Log\n");
        cpssOsPrintf("3.  HW\n");
        cpssOsPrintf("4.  Flow - common\n");
        cpssOsPrintf("5.  Flow - vlan\n");
        cpssOsPrintf("6.  Flow - termination\n");
        cpssOsPrintf("7.  Flow - ipcl\n");
        cpssOsPrintf("8.  Flow - bridge\n");
        cpssOsPrintf("9.  Flow - l3 unicast\n");
        cpssOsPrintf("10. Meter\n");
        cpssOsPrintf("11. Group - common\n");
        cpssOsPrintf("12. Group - L2 interface\n");
        cpssOsPrintf("13. Group - L2 multicast\n");
        cpssOsPrintf("14. Group - L2 rewrite\n");
        cpssOsPrintf("15. Group - L3 unicast\n");
        cpssOsPrintf("> ");


        do
        {
            rc = fgets(line, sizeof(line), stdin);
            if (rc)
            {
                module_id = atoi(line);
                if (module_id == 0 || module_id >= CPSS_HAL_DUMPUTL_DBG_LAST)
                {
                    cpssOsPrintf("Module number not found ortoo big\n");
                    continue;
                }
            }
            else
            {
                module_id = 0;
                break;
            }
        }
        while (module_id >= CPSS_HAL_DUMPUTL_DBG_LAST);

        if (module_id == 0)
        {
            return 0;
        }

        module_id--;

        do
        {
            cpssOsPrintf("\n0. exit\n");
            for (i=0; (i < CPSS_HAL_DUMPUTL_MAX_DBGS) &&
                 (cpssHalDumpUtlDbgPerModule[module_id][i].name[0] != 0) ; i++)
            {
                cpssOsPrintf("%d. %s\n", i+1, cpssHalDumpUtlDbgPerModule[module_id][i].name);
            }
            cpssOsPrintf("> ");

            rc = fgets(line, sizeof(line), stdin);
            if (rc)
            {
                num = atoi(line);
                if (num >= CPSS_HAL_DUMPUTL_MAX_DBGS)
                {
                    printf("Wrong choice\n");
                }
                continue;
            }
            else
            {
                num = 0;
            }
            if (num == 0)
            {
                break;
            }
            if (cpssHalDumpUtlDbgPerModule[module_id][num-1].name[0] == 0)
            {
                cpssOsPrintf("Wrong choice\n");
            }
            else
            {
                cpssOsPrintf("%s - %s\n", cpssHalDumpUtlDbgPerModule[module_id][num-1].name,
                             cpssHalDumpUtlDbgPerModule[module_id][num-1].description);
            }
        }
        while (1);
    }
    return 0;
}

GT_STATUS cpssHalDumpUtlRegDump(int cpssDevNum)
{
    GT_U32 *regAddrPtr,
           *regDataPtr; /* pointers to arrays of reg addresses and data*/
    GT_U32 regNum, i; /* number of regs */
    GT_STATUS       result;

    /* call cpss api function */
    result = cpssDxChDiagRegsNumGet((GT_U8)cpssDevNum, &regNum);
    if (result != GT_OK)
    {
        return (int)result;
    }
    cpssOsPrintf("registers number %d\n\n", regNum);
    regAddrPtr = cpssOsMalloc(regNum * sizeof(GT_U32));
    regDataPtr = cpssOsMalloc(regNum * sizeof(GT_U32));


    result = cpssDxChDiagRegsDump((GT_U8)cpssDevNum, &regNum, 0, regAddrPtr,
                                  regDataPtr);


    if (result != GT_OK)
    {
        return result;
    }

    cpssOsPrintf("registers number %d\n\n", regNum);
    for (i = 0; i < regNum; i++)
    {
        cpssOsPrintf("register addr 0x%08X value 0x%08X\n", regAddrPtr[i],
                     regDataPtr[i]);
    }

    cpssOsFree(regAddrPtr);
    cpssOsFree(regDataPtr);

    return GT_OK;
}

void cpssHalDumputilsPortDump
(
    uint8_t                devId,
    uint32_t               portNum,
    bool                   detail
)
{
    CPSS_PORT_MAC_COUNTER_SET_STC   portMacCounterSetArray;
    GT_STATUS rc;
    GT_BOOL     isLinkUp;
    GT_BOOL signal_detect;
    rc = cpssDxChPortLinkStatusGet(devId, portNum, &isLinkUp);
    if (rc == GT_OK)
    {
        cpssOsPrintf("Link status port %d link %s\n", portNum,
                     (isLinkUp==GT_TRUE) ? "UP" : "DOWN");
    }
    rc = cpssDxChPortSerdesSignalDetectGet(devId, portNum, &signal_detect);
    if (rc == GT_OK)
    {
        cpssOsPrintf("Link partner Detected port %d %s\n", portNum,
                     (signal_detect==GT_TRUE) ? "Yes" : "No");
    }
    if (detail)
    {
        rc = cpssDxChPortMacCountersOnPortGet(devId, portNum, &portMacCounterSetArray);
        if (rc == GT_OK)
        {
            cpssOsPrintf("Port %d stats:\n", portNum);

            cpssOsPrintf("ucPktsRcv           = 0x%X%X\n",
                         portMacCounterSetArray.ucPktsRcv.l[1], portMacCounterSetArray.ucPktsRcv.l[0]);
            cpssOsPrintf("brdcPktsRcv         = 0x%X%X\n",
                         portMacCounterSetArray.brdcPktsRcv.l[1],
                         portMacCounterSetArray.brdcPktsRcv.l[0]);
            cpssOsPrintf("mcPktsRcv           = 0x%X%X\n",
                         portMacCounterSetArray.mcPktsRcv.l[1], portMacCounterSetArray.mcPktsRcv.l[0]);
            cpssOsPrintf("ucPktsSent          = 0x%X%X\n",
                         portMacCounterSetArray.ucPktsSent.l[1], portMacCounterSetArray.ucPktsSent.l[0]);
            cpssOsPrintf("brdcPktsSent        = 0x%X%X\n",
                         portMacCounterSetArray.brdcPktsSent.l[1],
                         portMacCounterSetArray.brdcPktsSent.l[0]);
            cpssOsPrintf("mcPktsSent          = 0x%X%X\n",
                         portMacCounterSetArray.mcPktsSent.l[1], portMacCounterSetArray.mcPktsSent.l[0]);
            cpssOsPrintf("goodOctetsRcv       = 0x%X%X\n",
                         portMacCounterSetArray.goodOctetsRcv.l[1],
                         portMacCounterSetArray.goodOctetsRcv.l[0]);
            cpssOsPrintf("goodOctetsSent      = 0x%X%X\n",
                         portMacCounterSetArray.goodOctetsSent.l[1],
                         portMacCounterSetArray.goodOctetsSent.l[0]);
            cpssOsPrintf("badPktsRcv          = 0x%X%X\n",
                         portMacCounterSetArray.badPktsRcv.l[1], portMacCounterSetArray.badPktsRcv.l[0]);
            cpssOsPrintf("collisions          = 0x%X%X\n",
                         portMacCounterSetArray.collisions.l[1], portMacCounterSetArray.collisions.l[0]);
            cpssOsPrintf("excessiveCollisions = 0x%X%X\n",
                         portMacCounterSetArray.excessiveCollisions.l[1],
                         portMacCounterSetArray.excessiveCollisions.l[0]);
            cpssOsPrintf("lateCollisions      = 0x%X%X\n",
                         portMacCounterSetArray.lateCollisions.l[1],
                         portMacCounterSetArray.lateCollisions.l[0]);
            cpssOsPrintf("badCrc              = 0x%X%X\n",
                         portMacCounterSetArray.badCrc.l[1], portMacCounterSetArray.badCrc.l[0]);
            cpssOsPrintf("dropEvents          = 0x%X%X\n",
                         portMacCounterSetArray.dropEvents.l[1], portMacCounterSetArray.dropEvents.l[0]);
            cpssOsPrintf("macTransmitErr      = 0x%X%X\n",
                         portMacCounterSetArray.macTransmitErr.l[1],
                         portMacCounterSetArray.macTransmitErr.l[0]);
            cpssOsPrintf("undersizePkts       = 0x%X%X\n",
                         portMacCounterSetArray.undersizePkts.l[1],
                         portMacCounterSetArray.undersizePkts.l[0]);
            cpssOsPrintf("oversizePkts        = 0x%X%X\n",
                         portMacCounterSetArray.oversizePkts.l[1],
                         portMacCounterSetArray.oversizePkts.l[0]);
        }
    }

    return;
}

GT_STATUS cpssHalDumputilsFdbDump
(
    IN GT_U8                cpssDevNum
)
{
    CPSS_MAC_ENTRY_EXT_STC  entry;
    GT_HW_DEV_NUM           associatedHwDevNum;
    GT_U32                  index;
    GT_BOOL                 valid;
    GT_BOOL                 skip;
    GT_BOOL                 aged;
    GT_STATUS               status;
    index = 0;
    cpssOsPrintf("|  index |   type  |    Mode    |           Address           | vlan | dev_list_bitmap\n");
    cpssOsPrintf("--------------------------------------------------------------------------------------\n");
    while (1)
    {
        status = cpssDxChBrgFdbMacEntryRead
                 (
                     cpssDevNum, index, &valid, &skip, &aged, &associatedHwDevNum,  &entry
                 );
        if (status != GT_OK)
        {
            break;
        }
        index ++;
        if (valid != GT_TRUE || skip == GT_TRUE)
        {
            continue;
        }

        cpssOsPrintf("%06d) ", index-1);
        cpssOsPrintf("vid1 %d isAged %d assochwDev %d", entry.key.vid1, aged,
                     associatedHwDevNum);
        if (entry.dstInterface.type == CPSS_INTERFACE_PORT_E)
        {
            cpssOsPrintf(" dev %d port %d", entry.dstInterface.devPort.hwDevNum,
                         entry.dstInterface.devPort.portNum);
        }
        else if (entry.dstInterface.type == CPSS_INTERFACE_VID_E)
        {
            cpssOsPrintf(" vlanid %d", entry.dstInterface.vlanId);
        }
        else if (entry.dstInterface.type == CPSS_INTERFACE_VIDX_E)
        {
            cpssOsPrintf(" vidx %d", entry.dstInterface.vidx);
        }
        cpssOsPrintf(" isStatic %d daCommand %d saCommand %d daRoute %d ",
                     entry.isStatic, entry.daCommand, entry.saCommand, entry.daRoute);
        switch (entry.key.entryType)
        {
            case CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E:
                cpssOsPrintf("CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E ");
                cpssOsPrintf("%02x:%02x:%02x:%02x:%02x:%02x vlanId %d",
                             entry.key.key.macVlan.macAddr.arEther[0],
                             entry.key.key.macVlan.macAddr.arEther[1],
                             entry.key.key.macVlan.macAddr.arEther[2],
                             entry.key.key.macVlan.macAddr.arEther[3],
                             entry.key.key.macVlan.macAddr.arEther[4],
                             entry.key.key.macVlan.macAddr.arEther[5],
                             entry.key.key.macVlan.vlanId);
                break;
            case CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E:
                cpssOsPrintf("CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E sip %d.%d.%d.%d dip %d.%d.%d.%d vlanId %d",
                             entry.key.key.ipMcast.sip[0], entry.key.key.ipMcast.sip[1],
                             entry.key.key.ipMcast.sip[2], entry.key.key.ipMcast.sip[3],
                             entry.key.key.ipMcast.dip[0], entry.key.key.ipMcast.dip[1],
                             entry.key.key.ipMcast.dip[2], entry.key.key.ipMcast.dip[3],
                             entry.key.key.ipMcast.vlanId);
                break;
            case CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_E:
                cpssOsPrintf("CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_E sip %02x%02x:%02x%02x dip %02x%02x:%02x%02x vlanId %d",
                             entry.key.key.ipMcast.sip[0], entry.key.key.ipMcast.sip[1],
                             entry.key.key.ipMcast.sip[2], entry.key.key.ipMcast.sip[3],
                             entry.key.key.ipMcast.dip[0], entry.key.key.ipMcast.dip[1],
                             entry.key.key.ipMcast.dip[2], entry.key.key.ipMcast.dip[3],
                             entry.key.key.ipMcast.vlanId);
                break;
            case CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E:
                cpssOsPrintf("CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E ");
                cpssOsPrintf("dip %d.%d.%d.%d vrfId %d",
                             entry.key.key.ipv4Unicast.dip[0], entry.key.key.ipv4Unicast.dip[1],
                             entry.key.key.ipv4Unicast.dip[2], entry.key.key.ipv4Unicast.dip[3],
                             entry.key.key.ipv4Unicast.vrfId);
                break;
            case CPSS_MAC_ENTRY_EXT_TYPE_IPV6_UC_ADDR_ENTRY_E:
                cpssOsPrintf("CPSS_MAC_ENTRY_EXT_TYPE_IPV6_UC_ADDR_ENTRY_E ");
                cpssOsPrintf("dip %s vrfId %d",
                             cpssHalDumpUtlIpPrefix2Str(CPSS_HAL_DUMPLIB_ETH_TYPE_IPv6,
                                                        &entry.key.key.ipv6Unicast.dip[0]),
                             entry.key.key.ipv6Unicast.vrfId);
                break;
            case CPSS_MAC_ENTRY_EXT_TYPE_IPV6_UC_DATA_ENTRY_E:
                cpssOsPrintf("CPSS_MAC_ENTRY_EXT_TYPE_IPV6_UC_DATA_ENTRY_E ");
                cpssOsPrintf("dip %s vrfId %d",
                             cpssHalDumpUtlIpPrefix2Str(CPSS_HAL_DUMPLIB_ETH_TYPE_IPv6,
                                                        &entry.key.key.ipv6Unicast.dip[0]),
                             entry.key.key.ipv6Unicast.vrfId);
                break;
            case CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_FID_VID1_E:
                cpssOsPrintf("CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_FID_VID1_E");
                break;
            case CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_FID_VID1_E:
                cpssOsPrintf("CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_FID_VID1_E");
                break;
            case CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_FID_VID1_E:
                cpssOsPrintf("CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_FID_VID1_E");
                break;
            default:
                cpssOsPrintf(" unknown ");
                break;
        }
        cpssOsPrintf("\n");
    }
    return GT_OK;
}
void cpssHalDumputilsMac2MeDump
(
    uint8_t                dev,
    int                 fd
)
{
    GT_U32                              entryIndex = 0;
    CPSS_DXCH_TTI_MAC_VLAN_STC          macValue;
    CPSS_DXCH_TTI_MAC_VLAN_STC                      macMask;
    CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC  interfaceInfo;

    while (GT_OK == cpssDxChTtiMacToMeGet(dev, entryIndex, &macValue, &macMask,
                                          &interfaceInfo))
    {
        cpssOsPrintf("%03d: mac - [%s] ", entryIndex,
                     cpssHalDumpUtlMacAddress2String((CPSS_HAL_DUMPMAC_ADDRESS_STC *)
                                                     &macValue.mac.arEther));
        cpssOsPrintf("mask - [%s] vlan - [%04d]  mask - [%03x]\n",
                     cpssHalDumpUtlMacAddress2String((CPSS_HAL_DUMPMAC_ADDRESS_STC *)
                                                     &macMask.mac.arEther), macValue.vlanId, macMask.vlanId);
        entryIndex ++;
    }
}
#define print_x(x) cpssOsPrintf(#x " %lu\n", vlanInfo.x)
#define print_u(x) cpssOsPrintf(#x " %u\n", vlanInfo.x)
#define print_d(x) cpssOsPrintf(#x " %d\n", vlanInfo.x)
#define print_b(x) cpssOsPrintf(#x " %s\n", vlanInfo.x ? "true" : "false")
void cpssHalDumputilsVlanListDump
(
    uint8_t                dev
)
{
    GT_U32                               entryIndex = 0, count = 0;
    CPSS_PORTS_BMP_STC                   portsMembers;
    CPSS_PORTS_BMP_STC                   portsTagging;
    CPSS_DXCH_BRG_VLAN_INFO_STC          vlanInfo;
    GT_BOOL                              isValid;
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd;

    cpssOsPrintf("VLAN IDs ");
    while (GT_OK == cpssDxChBrgVlanEntryRead(dev, entryIndex, &portsMembers,
                                             &portsTagging, &vlanInfo, &isValid, &portsTaggingCmd))
    {
        if (isValid)
        {
            count ++;
            cpssOsPrintf("%d ", entryIndex);
            if (count % 10 == 0)
            {
                cpssOsPrintf("\n");
            }
        }
        entryIndex ++;
    }
    cpssOsPrintf("VLAN IDs total count  %d\n", count);
}

void cpssHalDumputilsVlanDump
(
    uint8_t                dev,
    uint32_t               vlan_id
)
{
    GT_U32                               entryIndex = (vlan_id) ? vlan_id : 0,
                                         port_index;
    CPSS_PORTS_BMP_STC                   portsMembers;
    CPSS_PORTS_BMP_STC                   portsTagging;
    CPSS_DXCH_BRG_VLAN_INFO_STC          vlanInfo;
    GT_BOOL                              isValid;
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd;

    while (GT_OK == cpssDxChBrgVlanEntryRead(dev, entryIndex, &portsMembers,
                                             &portsTagging, &vlanInfo, &isValid, &portsTaggingCmd))
    {
        if (isValid)
        {
            cpssOsPrintf("VLAN ID %d\n", entryIndex);
            cpssOsPrintf("portsMembers: ");
            for (port_index =0; port_index < CPSS_MAX_PORTS_BMP_NUM_CNS; port_index++)
            {
                cpssOsPrintf("%08d ", portsMembers.ports[port_index]);
            }
            cpssOsPrintf("\n");
            cpssOsPrintf("portsTagging: ");
            for (port_index =0; port_index < CPSS_MAX_PORTS_BMP_NUM_CNS; port_index++)
            {
                cpssOsPrintf("%08d ", portsTagging.ports[port_index]);
            }
            cpssOsPrintf("\n");
            print_u(unregNonIpMcastCmd);
            print_u(unregIpv4McastCmd);
            print_u(unregIpv6McastCmd);
            print_u(unkUcastCmd);
            print_u(unregIpv4BcastCmd);
            print_u(unregNonIpv4BcastCmd);
            print_b(ipv4IgmpToCpuEn);
            print_b(mirrToRxAnalyzerEn);
            print_b(ipv6IcmpToCpuEn);
            print_u(ipCtrlToCpuEn);
            cpssOsPrintf("ipv4IpmBrgMode %s\n",
                         vlanInfo.ipv4IpmBrgMode == CPSS_BRG_IPM_SGV_E ? "sgv" : "gv");
            cpssOsPrintf("ipv6IpmBrgMode %s\n",
                         vlanInfo.ipv6IpmBrgMode == CPSS_BRG_IPM_SGV_E ? "sgv" : "gv");
            print_b(ipv4IpmBrgEn);
            print_b(ipv6IpmBrgEn);
            print_u(ipv6SiteIdMode);
            print_b(ipv4UcastRouteEn);
            print_b(ipv4McastRouteEn);
            print_b(ipv6UcastRouteEn);
            print_b(ipv6McastRouteEn);
            print_d(stgId);
            print_b(autoLearnDisable);
            print_b(naMsgToCpuEn);
            print_d(mruIdx);
            print_b(bcastUdpTrapMirrEn);
            print_d(vrfId);
            print_d(floodVidx);
            print_u(floodVidxMode);
            print_u(portIsolationMode);
            print_b(ucastLocalSwitchingEn);
            print_b(mcastLocalSwitchingEn);
            print_d(mirrToRxAnalyzerIndex);
            print_b(mirrToTxAnalyzerEn);
            print_d(mirrToTxAnalyzerIndex);
            print_d(fidValue);
            print_u(unknownMacSaCmd);
            print_b(ipv4McBcMirrToAnalyzerEn);
            print_d(ipv4McBcMirrToAnalyzerIndex);
            print_b(ipv6McMirrToAnalyzerEn);
            print_d(ipv6McMirrToAnalyzerIndex);
            print_b(fcoeForwardingEn);
            print_u(unregIpmEVidxMode);
            print_d(unregIpmEVidx);
            cpssOsPrintf("fdbLookupKeyMode %s\n",
                         vlanInfo.fdbLookupKeyMode == CPSS_DXCH_BRG_FDB_LOOKUP_KEY_MODE_FID_E ? "fid" :
                         "fid_vid1");
        }
        entryIndex ++;
        break;
    }
}

void cpssHalDumputilsLpmDump
(
    uint8_t                dev,
    uint32_t               ip_ver
)
{
    GT_STATUS   status;
    GT_IPADDR   ipAddr;
    GT_IPV6ADDR ipv6Addr;
    GT_U32      prefixLen, i;
    GT_U32      tcamRowIndex = 0;
    GT_U32      tcamColIndex = 0;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT nextHopInfo;
    GT_BOOL     isTrunk = GT_FALSE;
    GT_TRUNK_ID outTrunkId = 0;
    GT_U8       outDev = 0, outPort = 0;
    GT_U32      outArpId = 0;
    GT_U16      outVid = 0;
    CPSS_PACKET_CMD_ENT outCmd = CPSS_PACKET_CMD_FORWARD_E;
    CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC routeEntry;
    GT_ETHERADDR    arpMacAddr;
    GT_U32      mtu_index = 0;
    GT_BOOL     is_first_run = GT_TRUE;

    if (ip_ver ==4)
    {
        memset(&ipAddr, 0, sizeof(ipAddr));
        prefixLen = 0;
        if (is_first_run)
        {
            status = cpssDxChIpLpmIpv4UcPrefixGet(CPSS_HAL_DUMPHAL_L3_LPM_DB_ID,
                                                  CPSS_HAL_DUMPHAL_L3_DEFAULT_VRID, &ipAddr, &prefixLen, &nextHopInfo,
                                                  &tcamRowIndex, &tcamColIndex);
            is_first_run = GT_FALSE;
        }
        else
        {
            status = cpssDxChIpLpmIpv4UcPrefixGetNext(CPSS_HAL_DUMPHAL_L3_LPM_DB_ID,
                                                      CPSS_HAL_DUMPHAL_L3_DEFAULT_VRID, &ipAddr, &prefixLen, &nextHopInfo,
                                                      &tcamRowIndex, &tcamColIndex);
        }
        while (status == GT_OK)
        {
            cpssOsPrintf("[row:%04d|col:%04d] ip %03d.%03d.%03d.%03d prefix %02d ",
                         tcamRowIndex, tcamColIndex,
                         ipAddr.arIP[0], ipAddr.arIP[1], ipAddr.arIP[2], ipAddr.arIP[3], prefixLen);
            {
                routeEntry.type = CPSS_DXCH_IP_UC_ROUTE_ENTRY_E;
                for (i=0; i < nextHopInfo.ipLttEntry.numOfPaths + 1; i++)
                {
                    status = cpssDxChIpUcRouteEntriesRead(dev,
                                                          nextHopInfo.ipLttEntry.routeEntryBaseIndex + i, &routeEntry, 1);
                    if (status != GT_OK)
                    {
                        cpssOsPrintf(" ---- Couldn't read rest of route entry %d\n",
                                     nextHopInfo.ipLttEntry.routeEntryBaseIndex);
                        status = cpssDxChIpLpmIpv4UcPrefixGetNext(CPSS_HAL_DUMPHAL_L3_LPM_DB_ID,
                                                                  CPSS_HAL_DUMPHAL_L3_DEFAULT_VRID,  &ipAddr, &prefixLen, &nextHopInfo,
                                                                  &tcamRowIndex, &tcamColIndex);
                        break;
                    }

                    if (routeEntry.entry.regularEntry.nextHopInterface.type ==
                        CPSS_INTERFACE_TRUNK_E)
                    {
                        isTrunk = GT_TRUE;
                        outTrunkId = routeEntry.entry.regularEntry.nextHopInterface.trunkId;
                    }
                    else
                    {
                        isTrunk = GT_FALSE;
                        outDev = routeEntry.entry.regularEntry.nextHopInterface.devPort.hwDevNum;
                        outPort = routeEntry.entry.regularEntry.nextHopInterface.devPort.portNum;
                    }

                    outArpId    = routeEntry.entry.regularEntry.nextHopARPPointer;
                    outVid      = routeEntry.entry.regularEntry.nextHopVlanId;
                    outCmd      = routeEntry.entry.regularEntry.cmd;
                    mtu_index   = routeEntry.entry.regularEntry.mtuProfileIndex;

                    cpssOsPrintf("re %04d ", nextHopInfo.ipLttEntry.routeEntryBaseIndex + i);

                    /* if this an ECMP entry - then print last part of line and first part of next line */
                    if (nextHopInfo.ipLttEntry.numOfPaths > i)
                    {
                        cpssDxChIpRouterArpAddrRead(dev, outArpId, &arpMacAddr);
                        cpssOsPrintf(" cmd = %d arp = %03d(%s) vid = %04d ", outCmd, outArpId,
                                     cpssHalDumpUtlMacAddress2String((CPSS_HAL_DUMPMAC_ADDRESS_STC *)&arpMacAddr),
                                     outVid);
                        cpssOsPrintf("mtu = %d ", mtu_index);

                        if (isTrunk)
                        {
                            cpssOsPrintf("trunk %d\n", outTrunkId);
                        }
                        else
                        {
                            cpssOsPrintf("dev %d port %02d\n", outDev, outPort);
                        }

                        cpssOsPrintf("[row:%04d|col:%04d] ip %03d.%03d.%03d.%03d prefix %02d ",
                                     tcamRowIndex, tcamColIndex,
                                     ipAddr.arIP[0], ipAddr.arIP[1], ipAddr.arIP[2], ipAddr.arIP[3], prefixLen);
                    }
                }
            }

            cpssDxChIpRouterArpAddrRead(dev, outArpId, &arpMacAddr);
            cpssOsPrintf(" cmd = %d arp = %03d(%s) vid = %04d ", outCmd, outArpId,
                         cpssHalDumpUtlMacAddress2String((CPSS_HAL_DUMPMAC_ADDRESS_STC *)&arpMacAddr),
                         outVid);
            cpssOsPrintf("mtu = %d ", mtu_index);

            if (isTrunk)
            {
                cpssOsPrintf("trunk %d\n", outTrunkId);
            }
            else
            {
                cpssOsPrintf("dev %d port %02d\n", outDev, outPort);
            }

            status = cpssDxChIpLpmIpv4UcPrefixGetNext(CPSS_HAL_DUMPHAL_L3_LPM_DB_ID,
                                                      CPSS_HAL_DUMPHAL_L3_DEFAULT_VRID,  &ipAddr, &prefixLen, &nextHopInfo,
                                                      &tcamRowIndex, &tcamColIndex);
        }
    }
    else  /*ipv6*/
    {
        memset(&ipv6Addr, 0, sizeof(ipv6Addr));
        prefixLen = 0;
        if (is_first_run)
        {
            status = cpssDxChIpLpmIpv6UcPrefixGet(CPSS_HAL_DUMPHAL_L3_LPM_DB_ID,
                                                  CPSS_HAL_DUMPHAL_L3_DEFAULT_VRID,  &ipv6Addr, &prefixLen, &nextHopInfo,
                                                  &tcamRowIndex, &tcamColIndex);
            is_first_run = GT_FALSE;
        }
        else
        {
            status = cpssDxChIpLpmIpv6UcPrefixGetNext(CPSS_HAL_DUMPHAL_L3_LPM_DB_ID,
                                                      CPSS_HAL_DUMPHAL_L3_DEFAULT_VRID,  &ipv6Addr, &prefixLen, &nextHopInfo,
                                                      &tcamRowIndex, &tcamColIndex);
        }

        while (status == GT_OK)
        {
            cpssOsPrintf("[row:%04d|col:%04d] ip %s prefix %02d ", tcamRowIndex,
                         tcamColIndex, cpssHalDumpUtlIpPrefix2Str(CPSS_HAL_DUMPLIB_ETH_TYPE_IPv6,
                                                                  (uint8_t*)&ipv6Addr), prefixLen);
            {
                routeEntry.type = CPSS_DXCH_IP_UC_ROUTE_ENTRY_E;
                for (i=0; i < nextHopInfo.ipLttEntry.numOfPaths + 1; i++)
                {
                    status = cpssDxChIpUcRouteEntriesRead(dev,
                                                          nextHopInfo.ipLttEntry.routeEntryBaseIndex + i, &routeEntry, 1);
                    if (status != GT_OK)
                    {
                        cpssOsPrintf(" ---- Couldn't read rest of route entry %d\n",
                                     nextHopInfo.ipLttEntry.routeEntryBaseIndex);
                        status = cpssDxChIpLpmIpv6UcPrefixGetNext(CPSS_HAL_DUMPHAL_L3_LPM_DB_ID,
                                                                  CPSS_HAL_DUMPHAL_L3_DEFAULT_VRID, &ipv6Addr, &prefixLen, &nextHopInfo,
                                                                  &tcamRowIndex, &tcamColIndex);
                        break;
                    }

                    if (routeEntry.entry.regularEntry.nextHopInterface.type ==
                        CPSS_INTERFACE_TRUNK_E)
                    {
                        isTrunk = GT_TRUE;
                        outTrunkId = routeEntry.entry.regularEntry.nextHopInterface.trunkId;
                    }
                    else
                    {
                        isTrunk = GT_FALSE;
                        outDev = routeEntry.entry.regularEntry.nextHopInterface.devPort.hwDevNum;
                        outPort = routeEntry.entry.regularEntry.nextHopInterface.devPort.portNum;
                    }

                    outArpId = routeEntry.entry.regularEntry.nextHopARPPointer;
                    outVid = routeEntry.entry.regularEntry.nextHopVlanId;
                    outCmd = routeEntry.entry.regularEntry.cmd;
                    mtu_index = routeEntry.entry.regularEntry.mtuProfileIndex;

                    cpssOsPrintf("re %04d ", nextHopInfo.ipLttEntry.routeEntryBaseIndex + i);

                    /* if this an ECMP entry - then print last part of line and first part of next line */
                    if (nextHopInfo.ipLttEntry.numOfPaths > i)
                    {
                        cpssDxChIpRouterArpAddrRead(dev, outArpId, &arpMacAddr);
                        cpssOsPrintf(" cmd = %d arp = %03d(%s) vid = %04d ", outCmd, outArpId,
                                     cpssHalDumpUtlMacAddress2String((CPSS_HAL_DUMPMAC_ADDRESS_STC *)&arpMacAddr),
                                     outVid);
                        cpssOsPrintf("mtu = %d ", mtu_index);

                        if (isTrunk)
                        {
                            cpssOsPrintf("trunk %d\n", outTrunkId);
                        }
                        else
                        {
                            cpssOsPrintf("dev %d port %02d\n", outDev, outPort);
                        }

                        cpssOsPrintf("[row:%04d|col:%04d] ip %s prefix %02d ", tcamRowIndex,
                                     tcamColIndex, cpssHalDumpUtlIpPrefix2Str(CPSS_HAL_DUMPLIB_ETH_TYPE_IPv6,
                                                                              (uint8_t*)&ipv6Addr), prefixLen);
                    }
                }
            }

            cpssDxChIpRouterArpAddrRead(dev, outArpId, &arpMacAddr);
            cpssOsPrintf(" cmd = %d arp = %03d(%s) vid = %04d ", outCmd, outArpId,
                         cpssHalDumpUtlMacAddress2String((CPSS_HAL_DUMPMAC_ADDRESS_STC *)&arpMacAddr),
                         outVid);
            cpssOsPrintf("mtu = %d ", mtu_index);

            if (isTrunk)
            {
                cpssOsPrintf(" trunk %d\n", outTrunkId);
            }
            else
            {
                cpssOsPrintf(" dev %d port %02d\n", outDev, outPort);
            }
            status = cpssDxChIpLpmIpv6UcPrefixGetNext(CPSS_HAL_DUMPHAL_L3_LPM_DB_ID,
                                                      CPSS_HAL_DUMPHAL_L3_DEFAULT_VRID,  &ipv6Addr, &prefixLen, &nextHopInfo,
                                                      &tcamRowIndex, &tcamColIndex);
        }
    }
    return;
}

GT_STATUS cpssHalEnableLog(int enable)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_LOG_ENABLE

    if (enable)
    {
        /* enable a CPSS API logging for all libs */
        cpssLogEnableSet(GT_TRUE);
        rc = cpssLogLibEnableSet(CPSS_LOG_LIB_ALL_E, CPSS_LOG_TYPE_ALL_E, GT_TRUE);
        if (rc != GT_OK)
        {
            return rc;
        }

        // Disable Driver calls
        rc = cpssLogLibEnableSet(CPSS_LOG_LIB_APP_DRIVER_CALL_E, CPSS_LOG_TYPE_ALL_E,
                                 GT_FALSE);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = cpssLogLibEnableSet(CPSS_LOG_LIB_HW_INIT_E, CPSS_LOG_TYPE_ALL_E, GT_FALSE);
        if (rc != GT_OK)
        {
            return rc;
        }


        /* add prefix to pointer's to distinguish from them usual hex values */
        cpssLogPointerFormatSet(CPSS_LOG_POINTER_FORMAT_PREFIX_E);

        /* don't log line number */
        cpssLogLineNumberEnableSet(GT_TRUE);
    }
    else
    {
        cpssLogEnableSet(GT_FALSE);
        rc = cpssLogLibEnableSet(CPSS_LOG_LIB_ALL_E, CPSS_LOG_TYPE_ALL_E, GT_FALSE);
        if (rc != GT_OK)
        {
            return rc;
        }

    }
#endif //CPSS_LOG_ENABLE
    return rc;
}

void mrvlExtOsLogFunc
(
    CPSS_LOG_LIB_ENT      lib,
    CPSS_LOG_TYPE_ENT     type,
    const char*           format,
    ...
)
{
    va_list args;
    char buffer[2048];

    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    cpssOsPrintf("%s", buffer);
    syslog(LOG_INFO, "%s", buffer);
}

