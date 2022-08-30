/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file gtAppNetDevDemo.c
*
* @brief This file includes APIs for netDev demo.
*        It should be used with appropriate linux driver.
*
* @version   1
********************************************************************************
*/
#include <appDemo/boardConfig/appDemoBoardConfig.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgGen.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgVlan.h>
#include <extUtils/common/cpssEnablerUtils.h>
#include <cmdShell/common/cmdCommon.h>
#include <gtOs/gtOsIo.h>

#define NET_DEV_DEBUG_PRINT /* Flag to print ... */
/* max ports in device in falcon  */
#define APP_DEMO_NETDEV_MAX_PORT_NUM             128

/* max entry count for cpssDxChBrgGenUdpBcDestPortCfgSet() */
#define BRG_GEN_UDP_BC_DEST_PORT_CFG_MAX_CNS     12

static GT_BOOL appDemoNetDevPortIsValid(GT_U8 devNum, GT_PORT_NUM portNum)
{
    GT_BOOL   isCpu, isValid;
    GT_STATUS rc, rc1;

    if (PRV_CPSS_DXCH_PP_HW_INFO_PORT_MAP_CHECK_SUPPORTED_MAC(devNum) != GT_TRUE)
    {
        return GT_FALSE;
    }

    rc = cpssDxChPortPhysicalPortMapIsCpuGet(devNum, portNum, &isCpu);
    rc1 = cpssDxChPortPhysicalPortMapIsValidGet(devNum, portNum, &isValid);
    if ((rc != GT_OK) || (isCpu == GT_TRUE) || (rc1 != GT_OK) || (isValid != GT_TRUE))
    {
        return GT_FALSE;
    }

    return GT_TRUE;
}

/**
* @internal appDemoNetDevDemoPortCfg function
* @endinternal
*
* @brief    This function is used to manul configure specific MAC address and VLAN ID for NetDev.
*           User can set "ignorePortCfg=1" in appDemoNetDevDemo to disable port configuration via the for loop.
*           And then apply their own configuration with this API.
*           Configurations - per portNum:
*            1. Create vlan id
*            2. Add port to vlan with egress untagged
*            3. Port pvid = vlan id
*            4. Enable ip control to trap (ipcntl, arp bc ... ) :
*            5. Add FDB entry -
*               Rule:   port MAC DA + vlan id
*               Action: da route trap to cpu + egress queue number
*
* @note   APPLICABLE DEVICES:      Falcon
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] macAddrStr            - MAC bytes 1-6 in format "XX:XX:XX:XX:XX:XX"
* @param[in] vid                   - VLAN ID which is used to set untagged memberset and port PVID
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS appDemoNetDevDemoPortCfg
(
    GT_U8       devNum,
    GT_PORT_NUM portNum,
    GT_U8       macAddrStr[18],
    GT_U16      vid
)
{
    GT_STATUS                                       rc = GT_OK;
    CPSS_PORTS_BMP_STC                              portsMembers;
    CPSS_PORTS_BMP_STC                              portsTagging;
    CPSS_DXCH_BRG_VLAN_INFO_STC                     vlanInfo;
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC            portsTaggingCmd;
    GT_U8                                           idx, macIdx;
    CPSS_MAC_ENTRY_EXT_STC                          macEntry;
    GT_ETHERADDR                                    baseDstMac;
    GT_CHAR hexcode[] = "0123456789ABCDEF"; /* for converting chars to hex */

#ifdef NET_DEV_DEBUG_PRINT
    cpssOsPrintf("[%s:%d] entered\n",__FUNCNAME__, __LINE__);
#endif

    /* Validate macAddrStr: expected format "XX:XX:XX:XX:XX:XX" */
    if (cpssOsStrlen(macAddrStr) != 17) {
        cpssOsPrintf("[%s:%d] cpssOsStrlen mac length != 17, devNum [%d] portNum [%d]\n",
                     __FUNCNAME__, __LINE__, devNum, portNum);
        return GT_BAD_PARAM;
    }

    /* Process macAddrStr ("XX:XX:XX:XX:XX:XX" format) into MAC array bytes 1-6
     * The rest will be addes later on a per port basis */
    cpssOsMemSet(&baseDstMac, 0, sizeof(baseDstMac));
    macIdx = 0;
    for (idx = 0; idx < cpssOsStrlen(macAddrStr); idx++) {
        if (macAddrStr[idx] == ':')
            continue;

        baseDstMac.arEther[macIdx] = (int)(cmdOsStrChr(hexcode, cmdOsToUpper(macAddrStr[idx++])) - hexcode) << 4;
        baseDstMac.arEther[macIdx] += (int)(cmdOsStrChr(hexcode, cmdOsToUpper(macAddrStr[idx])) - hexcode);
        macIdx++;
    }

    /* prepare VLAN table entry struct to use as input to cpssDxChBrgVlanEntryWrite() per-port */
    cpssOsMemSet(&vlanInfo, 0, sizeof(vlanInfo));
    vlanInfo.naMsgToCpuEn                = GT_TRUE;
    vlanInfo.autoLearnDisable            = GT_TRUE; /* Disable auto learn on VLAN */
    vlanInfo.unkUcastCmd                 = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unregIpv6McastCmd           = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unregIpv4McastCmd           = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unregNonIpMcastCmd          = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unregNonIpv4BcastCmd        = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unregIpv4BcastCmd           = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.floodVidx                   = 0xFFF;
    vlanInfo.mirrToRxAnalyzerIndex       = 0;
    vlanInfo.mirrToTxAnalyzerEn          = GT_FALSE;
    vlanInfo.mirrToTxAnalyzerIndex       = 0;
    vlanInfo.unknownMacSaCmd             = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.ipv4McBcMirrToAnalyzerEn    = GT_FALSE;
    vlanInfo.ipv4McBcMirrToAnalyzerIndex = 0;
    vlanInfo.ipv6McMirrToAnalyzerEn      = GT_FALSE;
    vlanInfo.ipv6McMirrToAnalyzerIndex   = 0;

    /* prepare FDB table entry struct to use as input to cpssDxChBrgFdbMacEntrySet() per-port */
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    macEntry.key.entryType      = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E;
    macEntry.isStatic           = GT_TRUE;
    macEntry.daCommand          = CPSS_MAC_TABLE_CNTL_E; /* trap to cpu */
    macEntry.saCommand          = CPSS_MAC_TABLE_FRWRD_E;
    macEntry.daRoute            = GT_TRUE;
    macEntry.appSpecificCpuCode = GT_TRUE;

    /* Create vlan id and Add port to vlan with egress untagged */
    vlanInfo.fidValue = vid;
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTagging);
    cpssOsMemSet(&portsTaggingCmd, 0, sizeof(CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC));
    CPSS_PORTS_BMP_PORT_SET_MAC(&portsMembers, portNum);
    CPSS_PORTS_BMP_PORT_SET_MAC(&portsTagging, portNum);
    portsTaggingCmd.portsCmd[portNum] = CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E;
    rc = cpssDxChBrgVlanEntryWrite(devNum,
                                   vid,
                                   &portsMembers,
                                   &portsTagging,
                                   &vlanInfo,
                                   &portsTaggingCmd);
    if (GT_OK != rc)
    {
        cpssOsPrintf("[%s:%d] cpssDxChBrgVlanEntryWrite FAILED, rc [%d] devNum [%d] portNum [%d]\n",
                      __FUNCNAME__, __LINE__, rc, devNum, portNum);
        return rc;
    }

    /* Set pvid of port to vlan id */
    rc = cpssDxChBrgVlanPortVidSet(devNum, portNum, CPSS_DIRECTION_INGRESS_E, vid);
    if (GT_OK != rc)
    {
        cpssOsPrintf("[%s:%d] cpssDxChBrgVlanPortVidSet FAILED, rc [%d] devNum [%d] portNum [%d]\n",
                      __FUNCNAME__, __LINE__, rc, devNum, portNum);
        return rc;
    }

    /* Enable ip control to trap */
    rc = cpssDxChBrgVlanIpCntlToCpuSet(devNum, vid, CPSS_DXCH_BRG_IP_CTRL_IPV4_IPV6_E);
    if (GT_OK != rc)
    {
        cpssOsPrintf("[%s:%d] cpssDxChBrgVlanIpCntlToCpuSet FAILED, rc [%d] devNum [%d] vid [%d]\n",
                      __FUNCNAME__, __LINE__, rc, devNum, vid);
        return rc;
    }

    rc = cpssDxChBrgVlanUdpBcPktsToCpuEnable(devNum, vid, GT_TRUE);
    if (GT_OK != rc)
    {
        cpssOsPrintf("[%s:%d] cpssDxChBrgVlanUdpBcPktsToCpuEnable FAILED, rc [%d] devNum [%d] vid [%d]\n",
                      __FUNCNAME__, __LINE__, rc, devNum, vid);
        return rc;
    }

    rc = cpssDxChBrgGenArpTrapEnable(devNum, portNum, GT_TRUE);
    if (GT_OK != rc)
    {
        cpssOsPrintf("[%s:%d] cpssDxChBrgGenArpTrapEnable FAILED, rc [%d] devNum [%d] portNum [%d]\n",
                      __FUNCNAME__, __LINE__, rc, devNum, portNum);
        return rc;
    }

    rc = cpssDxChBrgGenPortIpControlTrapEnableSet(devNum, portNum, CPSS_DXCH_BRG_IP_CTRL_IPV4_IPV6_E);
    if (GT_OK != rc)
    {
        cpssOsPrintf("[%s:%d] cpssDxChBrgGenPortIpControlTrapEnableSet FAILED, rc [%d] devNum [%d] portNum [%d]\n",
                      __FUNCNAME__, __LINE__, rc, devNum, portNum);
        return rc;
    }

    /* Add FDB entry to trap all packet (and da route) with port MAC DA and vlan id */
    macEntry.key.key.macVlan.vlanId = vid;

    /* Increase MAC address */
    cpssOsMemCpy(&macEntry.key.key.macVlan.macAddr.arEther, &baseDstMac, sizeof(GT_ETHERADDR));

    rc = cpssDxChBrgFdbMacEntrySet(devNum, &macEntry);
    if (GT_OK != rc)
    {
        cpssOsPrintf("[%s:%d] cpssDxChBrgFdbMacEntrySet FAILED, rc [%d] devNum [%d] portNum [%d]\n",
                      __FUNCNAME__, __LINE__, rc, devNum, portNum);
        return rc;
    }

#ifdef NET_DEV_DEBUG_PRINT
    cpssOsPrintf("[%s:%d] finished\n",__FUNCNAME__, __LINE__);
#endif

    return GT_OK;
}

/**
* @internal appDemoNetDevDemo function
* @endinternal
*
* @brief    Configure the switch to trap packet to CPU.
*           CPSS Netdev Linux driver will process trapped packets from the queue and
*           forward them to Linux kernel network stack.
*           Generate file with list of MAC address per port in format "XX:XX:XX:XX:XX:XX".
*           MAC is generated by macPrefix[3] + random[2] + portNum[1].
*           Or specific full 6 bytes based MAC address win format "XX:XX:XX:XX:XX:XX".
*           Configurations - per portNum:
*            1. Create vlan id = portnum
*            2. Add port to vlan with egress untagged
*            3. Port pvid = vlan id
*            4. Enable ip control to trap (ipcntl, arp bc ... ) :
*            5. Add FDB entry -
*               Rule:   port MAC DA + vlan id
*               Action: da route trap to cpu + egress queue number
*
* @note   APPLICABLE DEVICES:      Falcon
*
* @param[in] fileNameStr           - Pathname for MAC list generated file
* @param[in] macPrefix             - MAC bytes 1-3 in format "XX:XX:XX" or bytes 1-6 in format "XX:XX:XX:XX:XX:XX".
* @param[in] cpuQueueNum           - RX queue number to use (APPLICABLE RANGES : 0..7)
* @param[in] ignorePortCfg         - only appy the global configured setting.
*                                    Customer can apply the port configured setting individually by calling appDemoNetDevDemoPortCfg.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*/

GT_STATUS appDemoNetDevDemo
(
    GT_U8   *fileNameStr,
    GT_U8   macPrefix[18],
    GT_U8   cpuQueueNum,
    GT_U8   ignorePortCfg
)
{
    GT_STATUS                                       rc = GT_OK;
    GT_U8                                           devNum = 0;
    GT_PORT_NUM                                     portNum, skipPortCount = 0;
    GT_U8                                           idx, macIdx;
    GT_ETHERADDR                                    baseDstMac, tempDstMac;
    CPSS_OS_FILE_TYPE_STC                           file;
    CPSS_OS_FILE_TYPE_STC                           *filePtr = &file;
    CPSS_DXCH_NET_CPU_CODE_TABLE_ENTRY_STC          cpuCodeEntryInfo;
    GT_CHAR hexcode[] = "0123456789ABCDEF"; /* for converting chars to hex */
    GT_U32                                          bcUdpEntryIndex;
    GT_BOOL                                         validBcUdpEntry, foundExistingBcUdpEntry;
    GT_U16                                          udpPortNum;
    CPSS_NET_RX_CPU_CODE_ENT                        cpuCode;
    CPSS_PACKET_CMD_ENT                             pktCmd;
    GT_U32                                          mac_addr_inc_val;
    char                                            macAddrStr[18];

#ifdef NET_DEV_DEBUG_PRINT
    cpssOsPrintf("[%s:%d] entered\n",__FUNCNAME__, __LINE__);
#endif

    if (!fileNameStr) {
        cpssOsPrintf("[%s:%d] fileNameStr is null, devNum [%d]\n",
                     __FUNCNAME__, __LINE__, devNum);
        return GT_BAD_PARAM;
    }

    /* Open file to write MAC addresses */
    filePtr->type = CPSS_OS_FILE_REGULAR;
    filePtr->fd = cpssOsFopen((const char*)fileNameStr, "w+", filePtr);
    if (filePtr->fd == CPSS_OS_FILE_INVALID) {
        cpssOsPrintf("[%s:%d] error in opening the file, devNum [%d]\n",
                     __FUNCNAME__, __LINE__, devNum);
        return GT_BAD_PARAM;
    }

    /* Validate macPrefix: expected format "XX:XX:XX" (i.e. 8 chars) or "XX:XX:XX:XX:XX:XX" */
    if (cpssOsStrlen(macPrefix) != 8 && cpssOsStrlen(macPrefix) != 17) {
        cpssOsPrintf("[%s:%d] cpssOsStrlen mac length != 8 or length != 17, devNum [%d]\n",
                     __FUNCNAME__, __LINE__, devNum);
        return GT_BAD_PARAM;
    }

    if (cpuQueueNum >= 8) {
        cpssOsPrintf("[%s:%d] cpuQueueNum > 8, devNum [%d]\n",
                     __FUNCNAME__, __LINE__, devNum);
        return GT_BAD_PARAM;
    }

    /* Process macPrefix ("XX:XX:XX" format) into MAC array bytes 1-3
     * or macPrefix ("XX:XX:XX:XX:XX:XX" format) into MAC array bytes 1-6
     * The rest will be addes later on a per port basis */
    cpssOsMemSet(&baseDstMac, 0, sizeof(baseDstMac));
    macIdx = 0;
    for (idx = 0; idx < cpssOsStrlen(macPrefix); idx++) {
        if (macPrefix[idx] == ':')
            continue;

        baseDstMac.arEther[macIdx] = (int)(cmdOsStrChr(hexcode, cmdOsToUpper(macPrefix[idx++])) - hexcode) << 4;
        baseDstMac.arEther[macIdx] += (int)(cmdOsStrChr(hexcode, cmdOsToUpper(macPrefix[idx])) - hexcode);
        macIdx++;
    }

    if (cpssOsStrlen(macPrefix) == 8) {
        /* fill bytes 3 and 4 of baseDstMac with random numbers */
        cpssOsSrand(cpssOsTickGet());
        baseDstMac.arEther[3] = (GT_U8)(cpssOsRand());
        baseDstMac.arEther[4] = (GT_U8)(cpssOsRand());
        baseDstMac.arEther[5] = 0; /* this byte will be updated later in per port loop */
    }

    if (!ignorePortCfg)
    {
        /* per port/vlan confgurations */
        /* port num == vlan id */
        for (portNum = 0; portNum < APP_DEMO_NETDEV_MAX_PORT_NUM; portNum++) {
            if (appDemoNetDevPortIsValid(devNum, portNum) == GT_FALSE)
            {
                skipPortCount++;
            }
        }

        for (portNum = 0; portNum < APP_DEMO_NETDEV_MAX_PORT_NUM + skipPortCount; portNum++) {

            CPSS_ENABLER_PORT_SKIP_CHECK(devNum, portNum);

            /* Increase MAC address */
            cpssOsMemCpy(&tempDstMac, &baseDstMac, sizeof(GT_ETHERADDR));

            /* Ingore port range 60-63, since the range include CPU port 63 */
            mac_addr_inc_val = (portNum < 63) ? portNum : (portNum - 4);

            /* Increase MAC address value from based MAC address */
            for (idx = 6; idx > 0; idx--)
            {
                mac_addr_inc_val += (GT_U32) (tempDstMac.arEther[idx - 1] & 0xFF);
                tempDstMac.arEther[idx - 1] = (GT_U8)(mac_addr_inc_val & 0xFF);
                if (mac_addr_inc_val <= 0xFF)
                {
                    break; /* No overflow of 8-bits, terminal the process. */
                }
                mac_addr_inc_val = mac_addr_inc_val >> 8;
            }

            sprintf(macAddrStr, "%02x:%02x:%02x:%02x:%02x:%02x", tempDstMac.arEther[0], tempDstMac.arEther[1], tempDstMac.arEther[2], tempDstMac.arEther[3], tempDstMac.arEther[4], tempDstMac.arEther[5]);
            rc = appDemoNetDevDemoPortCfg(devNum, portNum, (GT_U8 *)macAddrStr, (GT_U16)portNum); /* Use portNum = VID */
            if (GT_OK != rc)
            {
                cpssOsPrintf("[%s:%d] appDemoNetDevDemoPortCfg FAILED, rc [%d] devNum [%d] portNum [%d] vid[%d]\n",
                              __FUNCNAME__, __LINE__, rc, devNum, portNum, portNum);
                return rc;
            }
        }
    } /* End of if (ignorePortCfg == GT_TRUE) */

    /* Mirror Bootstrap Protocol and DHCP (UDP 68) to CPU. */
    /* Check if the entry with UDP 68 already existing or not.
     * If not found, lookup an available entry index. */
    foundExistingBcUdpEntry = GT_FALSE;
    bcUdpEntryIndex = BRG_GEN_UDP_BC_DEST_PORT_CFG_MAX_CNS;
    for (idx = 0; idx < BRG_GEN_UDP_BC_DEST_PORT_CFG_MAX_CNS; idx++)
    {
        rc = cpssDxChBrgGenUdpBcDestPortCfgGet(devNum, idx, &validBcUdpEntry, &udpPortNum, &cpuCode, &pktCmd);
        if (GT_OK != rc)
        {
            cpssOsPrintf("[%s:%d] cpssDxChBrgGenUdpBcDestPortCfgGet FAILED, rc [%d] devNum [%d] entryIndex [%d]\n",
                          __FUNCNAME__, __LINE__, rc, devNum, idx);
            return rc;
        } else if (validBcUdpEntry == GT_TRUE && udpPortNum == 68 && cpuCode == CPSS_NET_UDP_BC_MIRROR_TRAP0_E && pktCmd == CPSS_PACKET_CMD_MIRROR_TO_CPU_E)
        {
            foundExistingBcUdpEntry = GT_TRUE;
            break;
        } else if (bcUdpEntryIndex == BRG_GEN_UDP_BC_DEST_PORT_CFG_MAX_CNS) {
            bcUdpEntryIndex = idx;
        }
    }

    /* Check if there is available entry for the new entry with UDP 68 */
    if (bcUdpEntryIndex == BRG_GEN_UDP_BC_DEST_PORT_CFG_MAX_CNS && foundExistingBcUdpEntry == GT_FALSE)
    {
        cpssOsPrintf("[%s:%d] No available entry for entrycpssDxChBrgGenUdpBcDestPortCfgSet() devNum [%d]\n",
                      __FUNCNAME__, __LINE__, devNum);
        return GT_HW_ERROR;
    }

    /* Apply the new entry with UDP 68 if the entry is not exiting */
    if (foundExistingBcUdpEntry == GT_FALSE)
    {
        udpPortNum = 68;
        cpuCode = CPSS_NET_UDP_BC_MIRROR_TRAP0_E;
        pktCmd = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
        rc = cpssDxChBrgGenUdpBcDestPortCfgSet(devNum, bcUdpEntryIndex, udpPortNum, cpuCode, pktCmd);
        if (GT_OK != rc)
        {
            cpssOsPrintf("[%s:%d] cpssDxChBrgGenUdpBcDestPortCfgSet FAILED, rc [%d] devNum [%d] udpPortNum[%d] cpuCode [%d] pktCmd [%d]\n",
                          __FUNCNAME__, __LINE__, rc, devNum, udpPortNum, cpuCode, pktCmd);
            return rc;
        }
    }

    /* continue Enable ip control to trap */
    rc = cpssDxChBrgGenIpLinkLocalMirrorToCpuEnable(devNum, CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    if (GT_OK != rc)
    {
        cpssOsPrintf("[%s:%d] cpssDxChBrgGenIpLinkLocalMirrorToCpuEnable FAILED, rc [%d] devNum [%d]\n",
                      __FUNCNAME__, __LINE__, rc, devNum);
        return rc;
    }
    rc = cpssDxChBrgGenIpLinkLocalMirrorToCpuEnable(devNum, CPSS_IP_PROTOCOL_IPV6_E, GT_TRUE);
    if (GT_OK != rc)
    {
        cpssOsPrintf("[%s:%d] cpssDxChBrgGenIpLinkLocalMirrorToCpuEnable FAILED, rc [%d] devNum [%d]\n",
                      __FUNCNAME__, __LINE__, rc, devNum);
        return rc;
    }

    /* configure that icmp types 133-137 will trapped to CPU */
    for (idx = 0; idx < 5; idx++) {
        rc = cpssDxChBrgGenIcmpv6MsgTypeSet(devNum, idx, 133 + idx, CPSS_PACKET_CMD_TRAP_TO_CPU_E);
        if (GT_OK != rc)
        {
            cpssOsPrintf("[%s:%d] cpssDxChBrgGenIcmpv6MsgTypeSet FAILED, rc [%d] devNum [%d]\n",
                          __FUNCNAME__, __LINE__, rc, devNum);
            return rc;
        }
    }

    rc = cpssDxChBrgGenIpV6SolicitedCmdSet(devNum, CPSS_PACKET_CMD_MIRROR_TO_CPU_E);
    if (GT_OK != rc)
    {
        cpssOsPrintf("[%s:%d] cpssDxChBrgGenIpV6SolicitedCmdSet FAILED, rc [%d] devNum [%d]\n",
                      __FUNCNAME__, __LINE__, rc, devNum);
        return rc;
    }

    /* configure the cpu code table to send trapped packets from fdb to input cpuQueueNumber */
    cpssOsMemSet(&cpuCodeEntryInfo, 0, sizeof(cpuCodeEntryInfo));
    cpuCodeEntryInfo.tc                        = cpuQueueNum;
    cpuCodeEntryInfo.dp                        = CPSS_DP_GREEN_E;
    cpuCodeEntryInfo.truncate                  = GT_FALSE;
    cpuCodeEntryInfo.cpuRateLimitMode          = CPSS_NET_CPU_CODE_RATE_LIMIT_LOCAL_E;
    cpuCodeEntryInfo.cpuCodeRateLimiterIndex   = 0;
    cpuCodeEntryInfo.cpuCodeStatRateLimitIndex = 0;
    cpuCodeEntryInfo.designatedDevNumIndex     = 0;
    rc = cpssDxChNetIfCpuCodeTableSet(devNum, CPSS_NET_ALL_CPU_OPCODES_E, &cpuCodeEntryInfo);

    if (GT_OK != rc)
    {
        cpssOsPrintf("[%s:%d] cpssDxChNetIfCpuCodeTableSet FAILED, rc [%d] devNum [%d]\n",
                      __FUNCNAME__, __LINE__, rc, devNum);
        return rc;
    }

#ifdef NET_DEV_DEBUG_PRINT
    if (cpssOsStrlen(macPrefix) == 8)
    {
        cpssOsPrintf("[%s:%d] file name %s\n",__FUNCNAME__, __LINE__, fileNameStr);
        cpssOsPrintf("[%s:%d] cpu queue num %d\n",__FUNCNAME__, __LINE__, cpuQueueNum);
        cpssOsPrintf("[%s:%d] base mac %02x:%02x:%02x:%02x:%02x:%02x\n",__FUNCNAME__, __LINE__,
                     baseDstMac.arEther[0], baseDstMac.arEther[1], baseDstMac.arEther[2],
                     baseDstMac.arEther[3], baseDstMac.arEther[4], baseDstMac.arEther[5]);
        cpssOsPrintf("mac bytes 0,1,2 - from input, bytes 3,4 - random, byte 5 - zero\n");
    }
#endif

    /* write the base mac address bytes 4 and 5 to the file */
    if (cpssOsStrlen(macPrefix) == 8)
    {
        cpssOsFprintf(filePtr->fd, "%02x:%02x\n", baseDstMac.arEther[3], baseDstMac.arEther[4]);
    }
    else
    {
        cpssOsFprintf(filePtr->fd, "%02x:%02x:%02x:%02x:%02x:%02x\n", baseDstMac.arEther[0], baseDstMac.arEther[1], baseDstMac.arEther[2], baseDstMac.arEther[3], baseDstMac.arEther[4], baseDstMac.arEther[5]);
    }
    cpssOsFclose(filePtr);

#ifdef NET_DEV_DEBUG_PRINT
    cpssOsPrintf("[%s:%d] finished\n",__FUNCNAME__, __LINE__);
#endif

    return GT_OK;
}

