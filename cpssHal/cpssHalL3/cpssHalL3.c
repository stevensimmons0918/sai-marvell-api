/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

#include "xpsL3.h"
#include "xpsInternal.h"
#include <cpss/dxCh/dxChxGen/ip/cpssDxChIpCtrl.h>
#include "cpssHalUtil.h"
#include "cpssHalDevice.h"
#include "cpssDxChIpTypes.h"
#include "cpssHalVlan.h"
#include "cpssHalL3.h"
#include "cpssHalTunnel.h"
#include "cpssHalCounter.h"
#include <cpss/dxCh/dxChxGen/tti/cpssDxChTtiTypes.h>
#include <cpss/dxCh/dxChxGen/ip/cpssDxChIp.h>
#include <cpss/dxCh/dxChxGen/tti/cpssDxChTti.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgFdbRouting.h>
#include <cpss/dxCh/dxChxGen/lpm/cpssDxChLpm.h>

#define CPSS_HAL_L3_MAX_MTU_INDEX 7
#define CPSS_HAL_L3_MAX_MTU_VALUE 0x3FFF

extern int GetL3CounterIdBase(int pos);

/**
 *
 * \brief unmask an IP header error due to SIP=DIP
 *
 *
 *
 * \param [in] devId
 *
 * \return GT_STATUS
 */
GT_STATUS cpssHalL3IpHeaderErrorMaskSet(int devId)
{
    GT_STATUS cpss_status = GT_OK;
    GT_U8               cpssDevNum;

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {

        cpss_status = cpssDxChIpHeaderErrorMaskSet(cpssDevNum,
                                                   CPSS_DXCH_IP_HEADER_ERROR_SIP_DIP_ENT, CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E,
                                                   GT_TRUE);
        if (cpss_status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChIpHeaderErrorMaskSet failed ");
            return (cpss_status);
        }

        cpss_status = cpssDxChIpHeaderErrorMaskSet(cpssDevNum,
                                                   CPSS_DXCH_IP_HEADER_ERROR_SIP_DIP_ENT, CPSS_IP_PROTOCOL_IPV6_E, CPSS_UNICAST_E,
                                                   GT_TRUE);
        if (cpss_status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChIpHeaderErrorMaskSet failed ");
            return (cpss_status);
        }

    }
    return cpss_status;
}
/**
 * \private
 * \brief internal API to write a ARP MAC address to the router ARP / Tunnel start / NAT table
 *
 * This API may cover multipe device support in future.
 *
 *
 * \param [in] devId
 * \param [in] routerArpIndex
 * \param [int] arpDaMac
 *
 * \return GT_STATUS
 */
GT_STATUS cpssHalL3IpRouterArpAddWrite(int devId,
                                       xpsArpPointer_t routerArpIndex, macAddr_t arpDaMac)
{
    GT_ETHERADDR arpMacAddr;
    GT_STATUS cpss_status;
    /* write a ARP MAC address to the Router ARP Table */
    memcpy(arpMacAddr.arEther, arpDaMac, sizeof(macAddr_t));

    /* call device specific API */
    cpss_status = cpssDxChIpRouterArpAddrWrite(devId, routerArpIndex, &arpMacAddr);
    return (cpss_status);
}

GT_STATUS cpssHalL3UpdtIntfIpUcRoutingEn(GT_U8 cpssDevId, GT_U32 cpssPortNum,
                                         CPSS_IP_PROTOCOL_STACK_ENT protocol,
                                         GT_BOOL enable, GT_BOOL isSVI)
{
    GT_STATUS           cpssStatus;
    GT_BOOL v4Enable = GT_FALSE;
    GT_BOOL v6Enable = GT_FALSE;
    GT_BOOL cncEnable = GT_TRUE;
    /* Enable Unicast IPv4 Routing on port */
    cpssStatus = cpssDxChIpPortRoutingEnable(cpssDevId, cpssPortNum,
                                             CPSS_IP_UNICAST_E, protocol, enable);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to enable v4 UC routing on port %d\n", cpssPortNum);
        return (cpssStatus);
    }

    // Enable the FDB based UC routing
    cpssStatus =  cpssDxChBrgFdbRoutingPortIpUcEnableSet(cpssDevId, cpssPortNum,
                                                         protocol, enable);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set the FDB based UC routing enable for port %d\n", cpssPortNum);
        return (cpssStatus);
    }

    /*
     * TTI IPv4 configurations. Enable/disable based on IPv4 capability of
     * the interface. It is assumed that user will enable L3 routing capability
     * on the interface before configuring tunnel termination on that interface.
     */
    if (protocol == CPSS_IP_PROTOCOL_IPV4_E ||
        protocol == CPSS_IP_PROTOCOL_IPV4V6_E)
    {
        cpssStatus = cpssHalTtiPortLookupEnableSet(cpssDevId, cpssPortNum,
                                                   CPSS_DXCH_TTI_KEY_IPV4_E, enable);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " cpssHalTtiPortLookupEnableSet failed with err : %d\n for port :%d ",
                  cpssStatus, cpssPortNum);
            return (cpssStatus);
        }

        /* Vxlan v4 TTI on NNI.
        To configure this dynmically on tunnel creation we need
        refCnt at port-level for the tunnels, since same port can
        hold multiple tunnels. Make sure IP Decap works with this. */
        cpssStatus = cpssHalTtiPortLookupEnableSet(cpssDevId, cpssPortNum,
                                                   CPSS_DXCH_TTI_KEY_UDB_IPV4_UDP_E, enable);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " cpssHalTtiPortLookupEnableSet failed with err : %d\n for port :%d ",
                  cpssStatus, cpssPortNum);
            return (cpssStatus);
        }

        cpssStatus = cpssHalTtiPortIpv4OnlyTunneledEnableSet(cpssDevId, cpssPortNum,
                                                             enable);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " cpssHalTtiPortIpv4OnlyTunneledEnableSet failed with err : %d\n for port :%d ",
                  cpssStatus, cpssPortNum);
            return (cpssStatus);
        }

        cpssStatus = cpssHalTtiPortIpv4OnlyMacToMeEnableSet(cpssDevId, cpssPortNum,
                                                            enable);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " cpssHalTtiPortIpv4OnlyMacToMeEnableSet failed with err : %d\n for port :%d ",
                  cpssStatus, cpssPortNum);
            return (cpssStatus);
        }

    }
    else if (protocol == CPSS_IP_PROTOCOL_IPV6_E ||
             protocol == CPSS_IP_PROTOCOL_IPV4V6_E)
    {
        cpssStatus = cpssHalTtiPortLookupEnableSet(cpssDevId, cpssPortNum,
                                                   CPSS_DXCH_TTI_KEY_UDB_IPV6_E, enable);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " cpssHalTtiPortLookupEnableSet failed with err : %d\n for port :%d ",
                  cpssStatus, cpssPortNum);
            return (cpssStatus);
        }

        /*Vxlan v6 TTI on NNI*/
        cpssStatus = cpssHalTtiPortLookupEnableSet(cpssDevId, cpssPortNum,
                                                   CPSS_DXCH_TTI_KEY_UDB_IPV6_UDP_E, enable);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " cpssHalTtiPortLookupEnableSet failed with err : %d\n for port :%d ",
                  cpssStatus, cpssPortNum);
            return (cpssStatus);
        }
    }
    else
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, " Invalid Protocol :%d ",
              cpssPortNum);
        return GT_ERROR;
    }

    /* Disable CNC counting when both v4 and v6 is disabled */
    cpssStatus = cpssDxChIpPortRoutingEnableGet(cpssDevId, cpssPortNum,
                                                CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, &v4Enable);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to enable v4 UC routing on port %d\n", cpssPortNum);
        return (cpssStatus);
    }

    cpssStatus = cpssDxChIpPortRoutingEnableGet(cpssDevId, cpssPortNum,
                                                CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV6_E, &v6Enable);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to enable v4 UC routing on port %d\n", cpssPortNum);
        return (cpssStatus);
    }
    if ((v4Enable == GT_FALSE) && (v6Enable == GT_FALSE))
    {
        cncEnable = GT_FALSE;
    }

    cpssStatus = cpssDxChCncPortClientEnableSet(cpssDevId, cpssPortNum,
                                                CPSS_DXCH_CNC_CLIENT_L2L3_INGRESS_VLAN_E, cncEnable);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " cpssDxChCncPortClientEnableSet failed with err : %d\n for port :%d ",
              cpssStatus, cpssPortNum);
        return (cpssStatus);
    }

    // Disable  FDB NA AU msg : DONT DO THIS for L3 VLAN member Interface. Had to be done only for L3 Physical Port
    if (isSVI == GT_FALSE)
    {
        cpssStatus = cpssDxChBrgFdbNaToCpuPerPortSet(cpssDevId, cpssPortNum,
                                                     enable? GT_FALSE:GT_TRUE);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set the FDB NA to CPU disable for port %d\n", cpssPortNum);
            return (cpssStatus);
        }
    }
#if 0
    cpssStatus = cpssDxChBrgFdbPortLearnStatusSet(cpssDevId, cpssPortNum, GT_FALSE,
                                                  enable? CPSS_LOCK_DROP_E:CPSS_LOCK_FRWRD_E);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "failed to disable learning of new SMAC on  port %d\n", cpssPortNum);
        return (cpssStatus);
    }
#endif


    return GT_OK;
}
GT_STATUS cpssHalL3BindPortIntf(GT_U8 cpssDevId,    GT_U32 cpssPortNum,
                                GT_U16 vlanId)
{

    GT_STATUS           cpssStatus;

    // Set the accepted frame type to only untagged for this port
    cpssStatus = cpssDxChBrgVlanPortAccFrameTypeSet(cpssDevId, cpssPortNum,
                                                    CPSS_PORT_ACCEPT_FRAME_UNTAGGED_E);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set accepted frame type for port id %d\n", cpssPortNum);
        return (cpssStatus);
    }

    // Add the port to the router reserved vlan
    cpssStatus = cpssDxChBrgVlanMemberAdd(cpssDevId, vlanId, cpssPortNum,
                                          GT_FALSE, CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to add port %d to router reserved vlan\n", cpssPortNum);
        return (cpssStatus);
    }

    cpssStatus = cpssDxChBrgVlanPortVidSet(cpssDevId, cpssPortNum,
                                           CPSS_DIRECTION_INGRESS_E, vlanId);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set port vid for port %d\n", cpssPortNum);
        return (cpssStatus);
    }
    return GT_OK;
}

#define MAC_SA_TABLE_INDEX_MAX     256

typedef struct macSaTableIndexMap_t
{
    GT_ETHERADDR macAddr;
    GT_BOOL      refCount;
} macSaTableIndexMap_t;

static macSaTableIndexMap_t macSaTblIdxMap[MAC_SA_TABLE_INDEX_MAX];

GT_STATUS cpssHalL3GetMacSaTableIndex(GT_ETHERADDR* macAddrPtr, GT_U32* index)
{
    GT_U32 i = 0;
    for (i=1; i<MAC_SA_TABLE_INDEX_MAX; i++)
    {
        if (macSaTblIdxMap[i].refCount == 0)
        {
            continue;
        }
        if (!memcmp(macSaTblIdxMap[i].macAddr.arEther, macAddrPtr,
                    sizeof(GT_ETHERADDR)))
        {
            /* match found */
            *index = i;
            macSaTblIdxMap[i].refCount++;
            return GT_OK;
        }
    }
    /* find a free index */
    for (i=1; i<MAC_SA_TABLE_INDEX_MAX; i++)
    {
        if (macSaTblIdxMap[i].refCount == 0)
        {
            memcpy(macSaTblIdxMap[i].macAddr.arEther, macAddrPtr, sizeof(GT_ETHERADDR));
            macSaTblIdxMap[i].refCount = 1;
            *index = i;
            return GT_OK;
        }
    }
    return GT_FAIL;
}

GT_STATUS cpssHalL3FreeMacSaTableIndex(GT_U32 index)
{
    if (index >= MAC_SA_TABLE_INDEX_MAX)
    {
        return GT_FAIL;
    }

    if (macSaTblIdxMap[index].refCount == 0)
    {
        return GT_FAIL;
    }

    macSaTblIdxMap[index].refCount--;

    return GT_OK;
}

GT_STATUS cpssHalL3SetPortEgressMac(GT_U8 cpssDevId,  GT_U32 cpssPortNum,
                                    macAddr_t egressMac)
{
    GT_STATUS                            cpssStatus;
    uint32_t                             macSaTableIndex = 0;
    GT_ETHERADDR                         cpssMacAddr;

    memset(&cpssMacAddr, 0, sizeof(GT_ETHERADDR));

    // Set the MAC SA modification enable
    cpssStatus = cpssDxChIpRouterMacSaModifyEnable(cpssDevId, cpssPortNum, GT_TRUE);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to enable MAC SA modify on port %d\n", cpssPortNum);
        return (cpssStatus);
    }

    // Set the SA MAC address to the Global MAC SA table
    memcpy(cpssMacAddr.arEther, egressMac, sizeof(cpssMacAddr.arEther));

    cpssStatus = cpssHalL3GetMacSaTableIndex(&cpssMacAddr, &macSaTableIndex);

    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get MAC SA table index to port id %d\n", cpssPortNum);
        return (cpssStatus);
    }

    // Set the allocated index to the egress port
    cpssStatus = cpssDxChIpRouterPortGlobalMacSaIndexSet(cpssDevId, cpssPortNum,
                                                         macSaTableIndex);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set MAC SA table index to port id %d\n", cpssPortNum);
        return (cpssStatus);
    }

    cpssStatus = cpssDxChIpRouterGlobalMacSaSet(cpssDevId, macSaTableIndex,
                                                &cpssMacAddr);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set the mac address into MAC SA table index %d\n", macSaTableIndex);
        return (cpssStatus);
    }

    return GT_OK;
}

GT_STATUS cpssHalL3SetEgressRouterMac(GT_U8 cpssDevId,  GT_BOOL isVlan,
                                      GT_U32 cpssPortNum, macAddr_t egressMac)
{
    GT_STATUS                            cpssStatus;
    uint32_t                             macSaTableIndex = 0;
    int                                  maxTotalPorts = 0;
    uint8_t                              port = 0;
    GT_ETHERADDR                         cpssMacAddr;
    CPSS_PORTS_BMP_STC                   globalPortMembers;
    CPSS_PORTS_BMP_STC                   portsTagging;
    CPSS_DXCH_BRG_VLAN_INFO_STC          vlanInfo;
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd;
    GT_BOOL                              isValid = GT_FALSE;
    GT_PORT_NUM     cpssPort;

    memset(&portsTagging, 0, sizeof(portsTagging));
    memset(&vlanInfo, 0, sizeof(vlanInfo));
    memset(&portsTaggingCmd, 0, sizeof(portsTaggingCmd));

    cpssHalGetMaxGlobalPorts(&maxTotalPorts);
    memset(&cpssMacAddr, 0, sizeof(GT_ETHERADDR));

    if (isVlan)
    {

        /* Retrieving member ports of the vlan */
        memset(&globalPortMembers, 0, sizeof(globalPortMembers));
        cpssStatus = cpssDxChBrgVlanEntryRead(cpssDevId, (GT_U16)cpssPortNum,
                                              &globalPortMembers,
                                              &portsTagging, &vlanInfo, &isValid,
                                              &portsTaggingCmd);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Read vlan %d failed .",
                  cpssPortNum);
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }


        /* Construct MAC SA Entry */
        cpssMacAddr.arEther[0] = egressMac[0];
        cpssMacAddr.arEther[1] = egressMac[1];
        cpssMacAddr.arEther[2] = egressMac[2];
        cpssMacAddr.arEther[3] = egressMac[3];
        cpssMacAddr.arEther[4] = egressMac[4];
        cpssMacAddr.arEther[5] = egressMac[5];

        /* Enabling UC routing on all member ports of the vlan*/
        XPS_GLOBAL_PORT_ITER(port, maxTotalPorts)
        {
            cpssPort = xpsGlobalPortToPortnum(cpssDevId, port);
            if (cpssPort == 0xffff)
            {
                return GT_FAIL;
            }
            if (CPSS_PORTS_BMP_IS_PORT_SET_MAC(&globalPortMembers, cpssPort))
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
                      "cpssDevNum = %d Member port : %d\n", cpssDevId, cpssPort);
                // Set the egress port mode to take use full 48 bits
                cpssStatus = cpssDxChIpPortRouterMacSaLsbModeSet(cpssDevId, cpssPort,
                                                                 CPSS_SA_LSB_FULL_48_BIT_GLOBAL);
                if (cpssStatus != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Failed to set the egress mac SA mode to port %d\n", cpssPort);
                    return (cpssStatus);
                }

                // Set the MAC SA modification enable
                cpssStatus = cpssDxChIpRouterMacSaModifyEnable(cpssDevId, cpssPort, GT_TRUE);
                if (cpssStatus != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Failed to enable MAC SA modify on port %d\n", cpssPort);
                    return (cpssStatus);
                }

                cpssStatus = cpssHalL3GetMacSaTableIndex(&cpssMacAddr, &macSaTableIndex);

                if (cpssStatus != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Failed to get MAC SA table index to port id %d\n", cpssPort);
                    return (cpssStatus);
                }

                // Set the allocated index to the egress port
                cpssStatus = cpssDxChIpRouterPortGlobalMacSaIndexSet(cpssDevId, cpssPort,
                                                                     macSaTableIndex);
                if (cpssStatus != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Failed to set MAC SA table index to port id %d\n", cpssPortNum);
                    return (cpssStatus);
                }


                cpssStatus = cpssDxChIpRouterGlobalMacSaSet(cpssDevId, macSaTableIndex,
                                                            &cpssMacAddr);
                if (cpssStatus != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Failed to set the mac address into MAC SA table index %d\n", macSaTableIndex);
                    return (cpssStatus);
                }
            }
        }
    }

    else /* Port interface */
    {
        // Set the egress port mode to take all 48 bits from global mac SA table
        cpssStatus = cpssDxChIpPortRouterMacSaLsbModeSet(cpssDevId, cpssPortNum,
                                                         CPSS_SA_LSB_FULL_48_BIT_GLOBAL);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set the egress mac SA mode to port %d\n", cpssPortNum);
            return (cpssStatus);
        }

        cpssStatus = cpssHalL3SetPortEgressMac(cpssDevId, cpssPortNum, egressMac);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set the mac address to port %d\n", cpssPortNum);
            return (cpssStatus);
        }
    }
    return GT_OK;
}
GT_STATUS cpssHalL3UnBindPortIntf(GT_U8 cpssDevId,    GT_U32 cpssPortNum)
{
    GT_STATUS           cpssStatus;
    GT_U16              vlanId;

    cpssStatus = cpssDxChBrgVlanPortVidGet(cpssDevId, cpssPortNum,
                                           CPSS_DIRECTION_INGRESS_E, &vlanId);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Failed to get pvid\n");
        return (cpssStatus);
    }
    // Remove the port from the router reserved vlan
    cpssStatus = cpssDxChBrgVlanPortDelete(cpssDevId, vlanId, cpssPortNum);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to remove port %d from router reserved vlan. DevId : %d\n", cpssPortNum,
              cpssDevId);
        return (cpssStatus);
    }

    // Set accepted frame type back to ALL
    cpssStatus = cpssDxChBrgVlanPortAccFrameTypeSet(cpssDevId, cpssPortNum,
                                                    CPSS_PORT_ACCEPT_FRAME_ALL_E);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set accepted frame type for port id %d\n", cpssPortNum);
        return (cpssStatus);
    }
    return GT_OK;
}
GT_STATUS cpssHalL3RemoveEgressRouterMac(GT_U8 cpssDevId, GT_BOOL isVlan,
                                         GT_U32 cpssPortNum)
{
    GT_STATUS           cpssStatus;
    uint32_t            macSaTableIndex;
    GT_ETHERADDR        cpssMacAddr;
    macAddr_t           egressMac;
    int                 port = 0;
    int                 maxTotalPorts = 0;
    GT_BOOL             isValid = GT_FALSE;
    CPSS_PORTS_BMP_STC                   globalPortMembers;
    CPSS_PORTS_BMP_STC                   portsMembers;
    CPSS_PORTS_BMP_STC                   portsTagging;
    CPSS_DXCH_BRG_VLAN_INFO_STC          vlanInfo;
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd;
    GT_PORT_NUM     cpssPort;

    memset(&portsMembers, 0, sizeof(portsMembers));
    memset(&portsTagging, 0, sizeof(portsTagging));
    memset(&vlanInfo, 0, sizeof(vlanInfo));
    memset(&portsTaggingCmd, 0, sizeof(portsTaggingCmd));

    cpssHalGetMaxGlobalPorts(&maxTotalPorts);
    memset(&cpssMacAddr, 0, sizeof(GT_ETHERADDR));
    memset(&egressMac, 0, sizeof(macAddr_t));

    if (isVlan)
    {
        /* Retrieving member ports of the vlan */
        memset(&globalPortMembers, 0, sizeof(globalPortMembers));
        cpssStatus = cpssHalReadBrgVlanEntry(cpssDevId, (GT_U16)cpssPortNum,
                                             &portsMembers,
                                             &portsTagging, &vlanInfo, &isValid,
                                             &portsTaggingCmd, &globalPortMembers);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Read vlan %d failed .",
                  cpssPortNum);
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }

        /* Enabling UC routing on all member ports of the vlan*/
        XPS_GLOBAL_PORT_ITER(port, maxTotalPorts)
        {
            cpssPort = xpsGlobalPortToPortnum(cpssDevId, port);
            if (cpssPort == 0xffff)
            {
                return GT_FAIL;
            }

            if (CPSS_PORTS_BMP_IS_PORT_SET_MAC(&globalPortMembers, cpssPort))
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
                      "cpssDevNum = %d Member port : %d\n", cpssDevId, port);
                // Set the MAC SA modification enable
                cpssStatus = cpssDxChIpRouterMacSaModifyEnable(cpssDevId, port, GT_FALSE);
                if (cpssStatus != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Failed to enable MAC SA modify on port %d\n", port);
                    return (cpssStatus);
                }
            }
        }

        /* Construct MAC SA Entry */
        cpssMacAddr.arEther[0] = egressMac[0];
        cpssMacAddr.arEther[1] = egressMac[1];
        cpssMacAddr.arEther[2] = egressMac[2];
        cpssMacAddr.arEther[3] = egressMac[3];
        cpssMacAddr.arEther[4] = egressMac[4];
        cpssMacAddr.arEther[5] = egressMac[5];

        /* Clear the 8 LSB to Vlan Mac LSB Register */
        cpssStatus = cpssDxChIpRouterVlanMacSaLsbSet(cpssDevId, cpssPortNum,
                                                     cpssMacAddr.arEther[5]);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to enable MAC SA Lsb modify %d\n", cpssPortNum);
            return (cpssStatus);
        }
    }
    else /* Port interface */
    {
        /*remove egress mac*/
        // Disable the MAC SA modification on port. All other configs will get invalidated
        cpssStatus = cpssDxChIpRouterMacSaModifyEnable(cpssDevId, cpssPortNum,
                                                       GT_FALSE);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to disable MAC SA modify on port %d\n", cpssPortNum);
            return (cpssStatus);
        }

        cpssStatus = cpssDxChIpRouterPortGlobalMacSaIndexGet(cpssDevId, cpssPortNum,
                                                             &macSaTableIndex);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to get MAC SA table index to port id %d\n", cpssPortNum);
            return (cpssStatus);
        }

        /* Free mac sa table index */
        cpssStatus = cpssHalL3FreeMacSaTableIndex(macSaTableIndex);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set free MAC SA table index %d\n", macSaTableIndex);
            return (cpssStatus);
        }

        macSaTableIndex = DEFAULT_MAC_SA_TABLE_INDEX;

        // Set the allocated index to the egress port
        cpssStatus = cpssDxChIpRouterPortGlobalMacSaIndexSet(cpssDevId, cpssPortNum,
                                                             macSaTableIndex);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set MAC SA table index to port id %d\n", cpssPortNum);
            return (cpssStatus);
        }

        cpssStatus = cpssDxChIpRouterGlobalMacSaSet(cpssDevId, macSaTableIndex,
                                                    &cpssMacAddr);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set the mac address into MAC SA table index %d\n", macSaTableIndex);
            return (cpssStatus);
        }
    }

    return GT_OK;
}

GT_STATUS cpssHalL3SetMtuProfileLimitValue(uint32_t devId,
                                           GT_U32 mtuProfileIndex, GT_U32 mtu)
{
    GT_STATUS           cpssStatus = GT_OK;
    GT_U8               cpssDevNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        if (mtuProfileIndex > CPSS_HAL_L3_MAX_MTU_INDEX)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid mtu profile index");
            return GT_FAIL;
        }
        else if (mtu > CPSS_HAL_L3_MAX_MTU_VALUE)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid mtu value");
            return GT_FAIL;
        }
        cpssStatus = cpssDxChIpMtuProfileSet(cpssDevNum, mtuProfileIndex, mtu);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed cpssDxChIpMtuProfileSet on cpssDevNum %d\n", cpssDevNum);
            return (cpssStatus);
        }
    }
    return cpssStatus;
}

GT_STATUS cpssHalWriteL3IpRouterArpAddress
(
    int      devId,
    xpsArpPointer_t routerArpIndex,
    macAddr_t arpDaMac
)

{
    GT_STATUS           status = GT_OK;
    int devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssHalL3IpRouterArpAddWrite(devNum, routerArpIndex, arpDaMac);
        if (status!= GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "writing ARP MAC address to the router ARP failed");
            return status;
        }
    }

    return status;
}


/**
* @internal cpssHalSetTtiPortGroupMacToMe function
* @endinternal
*
* @brief   This function sets a TTI MacToMe entry.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devId                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Aldrin2; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] entryIndex               - Index of mac and vlan in MacToMe table.
*                                      (APPLICABLE RANGES:
*                                      xCat3; Lion2: 0..7
*                                      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2: 0..127)
* @param[in] valuePtr                 - points to Mac To Me and Vlan To Me
* @param[in] maskPtr                  - points to mac and vlan's masks
* @param[in] interfaceInfoPtr         - points to source interface info (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id or wrong vlan/mac values
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/


GT_STATUS cpssHalSetTtiPortGroupMacToMe
(
    int                                             devId,
    GT_PORT_GROUPS_BMP                              portGroupsBmp,
    GT_U32                                          entryIndex,
    CPSS_DXCH_TTI_MAC_VLAN_STC                      *valuePtr,
    CPSS_DXCH_TTI_MAC_VLAN_STC                      *maskPtr,
    CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC  *interfaceInfoPtr
)

{
    GT_STATUS           status = GT_OK;
    int devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChTtiPortGroupMacToMeSet(devNum, portGroupsBmp, entryIndex,
                                                valuePtr, maskPtr, interfaceInfoPtr);
        if (status!= GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to add MAC2ME table entry at index %d\n", entryIndex);
            return status;
        }
    }

    return status;
}


/**
* @internal cpssHalWriteIpUcRouteEntries function
* @endinternal
*
* @brief   Writes an array of uc route entries to hw.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devId                   - the device number
* @param[in] baseRouteEntryIndex      - the index from which to write the array
* @param[in] routeEntriesArray        - the uc route entries array
* @param[in] numOfRouteEntries        - the number route entries in the array (= the
*                                      number of route entries to write)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on devNum not active.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In case uRPF ECMP/QOS is globally enabled, then for ECMP/QOS block
*       with x route entries, additional route entry should be included after
*       the block with the uRPF route entry information.
*
*/


GT_STATUS cpssHalWriteIpUcRouteEntries
(
    int                             devId,
    GT_U32                          baseRouteEntryIndex,
    CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC *routeEntriesArray,
    GT_U32                          numOfRouteEntries
)

{
    GT_STATUS           status = GT_OK;
    int devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChIpUcRouteEntriesWrite(devNum, baseRouteEntryIndex,
                                               routeEntriesArray, numOfRouteEntries);
        if (status!= GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to add route nexthop entry for NH : %d", baseRouteEntryIndex);
            return status;
        }
    }

    return status;
}


/**
* @internal cpssHalEnableIpRouting function
* @endinternal
*
* @brief   globally enable/disable routing.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devId                   - the device number
* @param[in] enableRouting            - enable /disable global routing
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - wrong devNum
*
* @note the ASIC defualt is routing enabled.
*
*/



GT_STATUS cpssHalEnableIpRouting
(
    int      devId,
    GT_BOOL  enableRouting
)

{
    GT_STATUS           status = GT_OK;
    int devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChIpRoutingEnable(devNum, enableRouting);
        if (status!= GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "enabling routing failed ");
            return status;
        }
    }

    return status;
}

GT_STATUS cpssHalBrgVlanIpUcRouteEnable
(
    int devId,
    xpsVlan_t vlanId
)

{
    GT_STATUS status = GT_OK;
    int devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChBrgVlanIpUcRouteEnable(devNum, vlanId, CPSS_IP_PROTOCOL_IPV4_E,
                                                GT_TRUE);
        if (status!= GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "enabling ipv4 routing on vlan failed ");
            return status;
        }

        status = cpssDxChBrgVlanIpUcRouteEnable(devNum, vlanId, CPSS_IP_PROTOCOL_IPV6_E,
                                                GT_TRUE);
        if (status!= GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "enabling ipv6 routing on vlan failed ");
            return status;
        }
    }
    return status;
}

/**
* @internal cpssHalL3SetIPMtuUcRouteEntries function
* @endinternal
*
* @brief   set the mtu profile index for entire unicast nexthop
*
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devId                   - the device number
* @param[in] mtuProfileIndex            - MTU profile index
* @param[in] cpssIntfType         -     interface type (vlan,
*       trunk, physical port
* @param[in] cpssIfNum          -  interface Id (trunk ID, vlan
*       ID, Phsyical port ID)
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - wrong devNum
*
* @note
*
*/

GT_STATUS cpssHalL3SetIPMtuUcRouteEntries
(
    GT_U8      devId,
    GT_U32 mtuProfileIndex,
    CPSS_INTERFACE_TYPE_ENT cpssIntfType,
    GT_PORT_NUM     cpssIfNum
)

{
    GT_U32 numEntries = 0, entryIndex;
    CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC cpssNhEntry;
    GT_STATUS cpssStatus = GT_OK;
    int  cpssDevNum;

    memset(&cpssNhEntry, 0, sizeof(cpssNhEntry));

    /* get NextHop entries count*/
    cpssStatus = cpssDxChCfgTableNumEntriesGet(devId,
                                               CPSS_DXCH_CFG_TABLE_ROUTER_NEXT_HOP_E, &numEntries);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get Router Next Hop count \n");
        return cpssStatus;
    }

    GT_BOOL foundEntry = GT_FALSE;
    /* go through NextHop table and update the nexthop with egress interfaces matches L3 Interface*/
    for (entryIndex = 0; entryIndex < numEntries; entryIndex++)
    {

        /* Read the CPSS Route Nexthop entrddy from HW */
        cpssStatus = cpssDxChIpUcRouteEntriesRead(devId, entryIndex, &cpssNhEntry, 1);

        /* skip entry if could not read or not same as requested input interface type*/
        if (cpssStatus != GT_OK ||
            cpssNhEntry.entry.regularEntry.nextHopInterface.type != cpssIntfType)
        {
            continue;
        }

        switch (cpssIntfType)
        {
            case CPSS_INTERFACE_TRUNK_E:
                {
                    if (cpssNhEntry.entry.regularEntry.nextHopInterface.trunkId == cpssIfNum)
                    {
                        foundEntry = GT_TRUE;
                    }
                    break;
                }
            case CPSS_INTERFACE_PORT_E:
                {
                    if (cpssNhEntry.entry.regularEntry.nextHopInterface.devPort.portNum ==
                        cpssIfNum)
                    {
                        foundEntry = GT_TRUE;
                    }
                    break;
                }
            case CPSS_INTERFACE_VID_E:
                {
                    /* nextHopInterface.vlanId field is not updated by CPSS,
                        hence check on nextHopVlanId for SVI */
                    if (cpssNhEntry.entry.regularEntry.nextHopVlanId == cpssIfNum)
                    {
                        foundEntry = GT_TRUE;
                    }
                    break;
                }
            default:
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Faild to set MTU profile index due to unsupported cpss egressInterface %u type: %d",
                          cpssIfNum, cpssIntfType);
                }
        }

        if (foundEntry == GT_TRUE)
        {
            foundEntry = GT_FALSE;  /* reset found entry to find next matched entry */
            cpssNhEntry.entry.regularEntry.mtuProfileIndex = mtuProfileIndex;
            /* Supporting B2B device to update MTU for corrosponding Nexthop in that device*/
            XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
            {
                /* Read the CPSS Route Nexthop entry from HW */
                cpssStatus = cpssDxChIpUcRouteEntriesWrite(cpssDevNum, entryIndex, &cpssNhEntry,
                                                           1);
                if (cpssStatus != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Failed to update MTU profile index for Unicast Nexthop entry at index: %d",
                          entryIndex);
                    return cpssStatus;
                }
            }
        }

    }


    return GT_OK;
}

GT_STATUS cpssHalCncL3InitNon64BitBlock(GT_U8 devId, uint32_t blockStart)
{
    GT_STATUS rc = GT_OK;
    int pos = 0;
    int eVlanIdxBase = 0;
    uint32_t ingPassCntIdx = 0;
    uint32_t ingDropCntIdx = 0;
    uint32_t egrPassCntIdx = 0;
    uint32_t egrDropCntIdx = 0;
    CPSS_PP_FAMILY_TYPE_ENT devFamily = cpssHalDevPPFamilyGet(devId);

    if (devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E)
    {
        /* "There are new debug/monitor features in AC5P and above devices.
           These features require that dropped packets got to end of processing pipe.
           Need to disable drop in ingress part of pipe for that. "
           Currently for AC5X enable Ingress Drop.
        */
        rc =  cpssDxChCfgIngressDropEnableSet(devId, GT_TRUE);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    for (pos = 0; pos < 5; pos++)
    {
        /* For AC3X only 1 eVlan base is valid. Take the first one. */
        if ((pos > 0) && (devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E))
        {
            break;
        }

        eVlanIdxBase = GetL3CounterIdBase(pos);
        if (eVlanIdxBase == -1)
        {
            continue;
        }
        ingPassCntIdx = (eVlanIdxBase/XPS_CPSS_CNC_COUNTER_PER_BLK);
        ingDropCntIdx = (((1<<14) | eVlanIdxBase)/XPS_CPSS_CNC_COUNTER_PER_BLK);

        if (devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E ||
            devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E)
        {
            /*Pass,Drop(Bit 0) falls on same block.
              0-511 uses CNC_CLIENT_EGRESS_VLAN_PASS_BLK_OFFSET
              512-1023 uses CNC_CLIENT_EGRESS_VLAN_DROP_BLK_OFFSET*/
            egrPassCntIdx = ((eVlanIdxBase << 1)/XPS_CPSS_CNC_COUNTER_PER_BLK);
            egrDropCntIdx = (((eVlanIdxBase+512) << 1)/XPS_CPSS_CNC_COUNTER_PER_BLK);
        }
        else
        {
            egrPassCntIdx = ingPassCntIdx;
            egrDropCntIdx = ingDropCntIdx;
        }

        /* Ingress L2/L3 eVlan Client */
        rc = cpssHalCncBlockClientEnableAndBindSet(devId,
                                                   (blockStart + (pos*MAX_RIF_CLINETS_NON_64_BIT_MODE) +
                                                    CNC_CLIENT_L2L3_INGRESS_VLAN_BLK_OFFSET),
                                                   CPSS_DXCH_CNC_CLIENT_L2L3_INGRESS_VLAN_E,
                                                   GT_TRUE, ingPassCntIdx,
                                                   CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_1_E);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to enable Ingress eValn client rc %d\n", rc);
            return rc;
        }

        /* Ingress Vlan Drop Client */
        rc = cpssHalCncBlockClientEnableAndBindSet(devId,
                                                   (blockStart + (pos*MAX_RIF_CLINETS_NON_64_BIT_MODE) +
                                                    CNC_CLIENT_INGRESS_VLAN_DROP_BLK_OFFSET),
                                                   CPSS_DXCH_CNC_CLIENT_INGRESS_VLAN_PASS_DROP_E,
                                                   GT_TRUE, ingDropCntIdx,
                                                   CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_1_E);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to enable Ingress Drop Valn client rc %d\n", rc);
            return rc;
        }

        /* Egress Vlan Pass Client */
        rc = cpssHalCncBlockClientEnableAndBindSet(devId,
                                                   (blockStart + (pos*MAX_RIF_CLINETS_NON_64_BIT_MODE) +
                                                    CNC_CLIENT_EGRESS_VLAN_PASS_BLK_OFFSET),
                                                   CPSS_DXCH_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E,
                                                   GT_TRUE, egrPassCntIdx,
                                                   CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_1_E);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to enable Egress Pass Valn client rc %d\n", rc);
            return rc;
        }

        /* Egress Vlan Drop Client */
        rc = cpssHalCncBlockClientEnableAndBindSet(devId,
                                                   (blockStart + (pos*MAX_RIF_CLINETS_NON_64_BIT_MODE) +
                                                    CNC_CLIENT_EGRESS_VLAN_DROP_BLK_OFFSET),
                                                   CPSS_DXCH_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E,
                                                   GT_TRUE, egrDropCntIdx,
                                                   CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_1_E);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to enable Egress Drop Valn client rc %d\n", rc);
            return rc;
        }

    }
    return rc;
}

GT_STATUS cpssHalCncL3Init64BitBlock(GT_U8 devId, uint32_t blockStart)
{
    GT_STATUS rc = GT_OK;
    int pos = 0;
    int eVlanIdxBase = 0;
    uint32_t ingPassCntIdx = 0;
    uint32_t ingDropCntIdx = 0;
    uint32_t egrPassCntIdx = 0;
    uint32_t egrDropCntIdx = 0;
    CPSS_PP_FAMILY_TYPE_ENT devFamily = cpssHalDevPPFamilyGet(devId);

    if (devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E)
    {
        /* "There are new debug/monitor features in AC5P and above devices.
           These features require that dropped packets got to end of processing pipe.
           Need to disable drop in ingress part of pipe for that. "
           Currently for AC5X enable Ingress Drop.
        */
        rc =  cpssDxChCfgIngressDropEnableSet(devId, GT_TRUE);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    for (pos = 0; pos < 5; pos++)
    {
        eVlanIdxBase = GetL3CounterIdBase(pos);
        if (eVlanIdxBase == -1)
        {
            continue;
        }

        ingPassCntIdx = (eVlanIdxBase/XPS_CPSS_CNC_COUNTER_PER_BLK);
        ingDropCntIdx = (((1<<14) | eVlanIdxBase)/XPS_CPSS_CNC_COUNTER_PER_BLK);

        if (devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E ||
            devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E)
        {
            /* For Falcon Pass/Drop is 0th bit. Same block will be used to count
               pass and drop. Like index0=pass,index1=drop,index2=pass,...
               Hence allocate 512 eVlan per block.
             */
            egrPassCntIdx = ((eVlanIdxBase << 1)/XPS_CPSS_CNC_COUNTER_PER_BLK);
            egrDropCntIdx = (((eVlanIdxBase+512) << 1)/XPS_CPSS_CNC_COUNTER_PER_BLK);
        }
        else
        {
            egrPassCntIdx = ingPassCntIdx;
            egrDropCntIdx = ingDropCntIdx;
        }

        /* Ingress L2/L3 eVlan Client for pkt block */
        rc = cpssHalCncBlockClientEnableAndBindSet(devId,
                                                   (blockStart + (pos*MAX_RIF_CLINETS_64_BIT_MODE) +
                                                    CNC_CLIENT_L2L3_INGRESS_VLAN_PKT_BLK_OFFSET),
                                                   CPSS_DXCH_CNC_CLIENT_L2L3_INGRESS_VLAN_E,
                                                   GT_TRUE, ingPassCntIdx,
                                                   CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_3_E);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to enable Ingress eValn client rc %d\n", rc);
            return rc;
        }

        /* Ingress L2/L3 eVlan Client for Byte block */
        rc = cpssHalCncBlockClientEnableAndBindSet(devId,
                                                   (blockStart + (pos*MAX_RIF_CLINETS_64_BIT_MODE) +
                                                    CNC_CLIENT_L2L3_INGRESS_VLAN_BYTE_BLK_OFFSET),
                                                   CPSS_DXCH_CNC_CLIENT_L2L3_INGRESS_VLAN_E,
                                                   GT_TRUE, ingPassCntIdx,
                                                   CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_4_E);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to enable Ingress eValn client rc %d\n", rc);
            return rc;
        }

        /* Ingress Vlan Drop Client for PKT block */
        rc = cpssHalCncBlockClientEnableAndBindSet(devId,
                                                   (blockStart + (pos*MAX_RIF_CLINETS_64_BIT_MODE) +
                                                    CNC_CLIENT_INGRESS_VLAN_DROP_PKT_BLK_OFFSET),
                                                   CPSS_DXCH_CNC_CLIENT_INGRESS_VLAN_PASS_DROP_E,
                                                   GT_TRUE, ingDropCntIdx,
                                                   CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_3_E);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to enable Ingress Drop Valn client rc %d\n", rc);
            return rc;
        }

        /* Ingress Vlan Drop Client for Byte block */
        rc = cpssHalCncBlockClientEnableAndBindSet(devId,
                                                   (blockStart + (pos*MAX_RIF_CLINETS_64_BIT_MODE) +
                                                    CNC_CLIENT_INGRESS_VLAN_DROP_BYTE_BLK_OFFSET),
                                                   CPSS_DXCH_CNC_CLIENT_INGRESS_VLAN_PASS_DROP_E,
                                                   GT_TRUE, ingDropCntIdx,
                                                   CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_4_E);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to enable Ingress Drop Valn client rc %d\n", rc);
            return rc;
        }

        /* Egress Vlan Pass Client for Pkt Block*/
        rc = cpssHalCncBlockClientEnableAndBindSet(devId,
                                                   (blockStart + (pos*MAX_RIF_CLINETS_64_BIT_MODE) +
                                                    CNC_CLIENT_EGRESS_VLAN_PASS_PKT_BLK_OFFSET),
                                                   CPSS_DXCH_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E,
                                                   GT_TRUE, egrPassCntIdx,
                                                   CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_3_E);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to enable Egress Pass Valn client rc %d\n", rc);
            return rc;
        }
        /* Egress Vlan Drop Client for Pkt Block */
        rc = cpssHalCncBlockClientEnableAndBindSet(devId,
                                                   (blockStart + (pos*MAX_RIF_CLINETS_64_BIT_MODE) +
                                                    CNC_CLIENT_EGRESS_VLAN_DROP_PKT_BLK_OFFSET),
                                                   CPSS_DXCH_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E,
                                                   GT_TRUE, egrDropCntIdx,
                                                   CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_3_E);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to enable Egress Drop Valn client rc %d\n", rc);
            return rc;
        }

        /* Egress Vlan Pass Client for Byte Block*/
        rc = cpssHalCncBlockClientEnableAndBindSet(devId,
                                                   (blockStart + (pos*MAX_RIF_CLINETS_64_BIT_MODE) +
                                                    CNC_CLIENT_EGRESS_VLAN_PASS_BYTE_BLK_OFFSET),
                                                   CPSS_DXCH_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E,
                                                   GT_TRUE, egrPassCntIdx,
                                                   CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_4_E);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to enable Egress Pass Valn client rc %d\n", rc);
            return rc;
        }

        /* Egress Vlan Drop Client for Byte Block */
        rc = cpssHalCncBlockClientEnableAndBindSet(devId,
                                                   (blockStart + (pos*MAX_RIF_CLINETS_64_BIT_MODE) +
                                                    CNC_CLIENT_EGRESS_VLAN_DROP_BYTE_BLK_OFFSET),
                                                   CPSS_DXCH_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E,
                                                   GT_TRUE, egrDropCntIdx,
                                                   CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_4_E);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to enable Egress Drop Valn client rc %d\n", rc);
            return rc;
        }

    }
    return rc;
}

/* Init CNC for L3 counters. By design, only 1K RIFs can
   support counters. RIF range is selected from profile.ini.
   For M0 devices, 0 to 4K-1 is for Vlan id based RIFs and
   4K to max is for Port/LAG RIFs.
 */
GT_STATUS cpssHalCncL3Init(GT_U8 devId)
{
    GT_STATUS rc = GT_OK;
    XP_DEV_TYPE_T devType;
    cpssHalGetDeviceType(devId, &devType);

    switch (cpssHalDevPPFamilyGet(devId))
    {
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            {
                if (IS_DEVICE_FALCON_10G_ONLY_PROFILE(devType) ||
                    IS_DEVICE_FALCON_25G_ONLY_PROFILE(devType) ||
                    IS_DEVICE_FALCON_10G_25G_ONLY_PROFILE(devType))
                {
                    rc = cpssHalCncL3InitNon64BitBlock(devId, FALCON_RIF_CNC_BLOCK_START);
                    if (rc != GT_OK)
                    {
                        return xpsConvertCpssStatusToXPStatus(rc);
                    }
                }
                else
                {
                    rc = cpssHalCncL3Init64BitBlock(devId, FALCON_RIF_CNC_BLOCK_START);
                    if (rc != GT_OK)
                    {
                        return xpsConvertCpssStatusToXPStatus(rc);
                    }
                }
            }
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            {
                rc = cpssHalCncL3InitNon64BitBlock(devId, ALDRIN2_RIF_CNC_BLOCK_START);
                if (rc != GT_OK)
                {
                    return xpsConvertCpssStatusToXPStatus(rc);
                }
            }
            break;
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
            {
                rc = cpssHalCncL3InitNon64BitBlock(devId, AC5X_RIF_CNC_BLOCK_START);
                if (rc != GT_OK)
                {
                    return xpsConvertCpssStatusToXPStatus(rc);
                }
            }
            break;
        default:
            {
                rc = cpssHalCncL3InitNon64BitBlock(devId, CPSS_CNC_BLOCK_1);
                if (rc != GT_OK)
                {
                    return xpsConvertCpssStatusToXPStatus(rc);
                }
            }
            break;
    }

    /* Set Vlan index mode to EVID */
    rc = cpssHalCncVlanClientIndexModeSet(devId,
                                          CPSS_DXCH_CNC_CLIENT_INGRESS_VLAN_PASS_DROP_E,
                                          CPSS_DXCH_CNC_VLAN_INDEX_MODE_EVID_E);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed Vid Mode rc %d\n", rc);
        return rc;
    }

    rc = cpssHalCncVlanClientIndexModeSet(devId,
                                          CPSS_DXCH_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E,
                                          CPSS_DXCH_CNC_VLAN_INDEX_MODE_EVID_E);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed Vid Mode rc %d\n", rc);
        return rc;
    }

    rc = cpssHalCncClientByteCountModeSet(devId,
                                          CPSS_DXCH_CNC_CLIENT_L2L3_INGRESS_VLAN_E,
                                          CPSS_DXCH_CNC_BYTE_COUNT_MODE_L3_E);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed Count Mode rc %d\n", rc);
        return rc;
    }
    rc = cpssHalCncClientByteCountModeSet(devId,
                                          CPSS_DXCH_CNC_CLIENT_INGRESS_VLAN_PASS_DROP_E,
                                          CPSS_DXCH_CNC_BYTE_COUNT_MODE_L3_E);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed Count Mode rc %d\n", rc);
        return rc;
    }
    rc = cpssHalCncClientByteCountModeSet(devId,
                                          CPSS_DXCH_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E,
                                          CPSS_DXCH_CNC_BYTE_COUNT_MODE_L3_E);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed Count Mode rc %d\n", rc);
        return rc;
    }

    return rc;
}

GT_STATUS cpssHalIpExceptionCommandSet(GT_U32 devId,
                                       CPSS_DXCH_IP_EXCEPTION_TYPE_ENT excep,
                                       CPSS_IP_PROTOCOL_STACK_ENT proto,
                                       CPSS_PACKET_CMD_ENT pktCmd)
{
    GT_STATUS       rc = GT_OK;
    GT_U8           cpssDevNum;

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        rc = cpssDxChIpExceptionCommandSet(cpssDevNum,
                                           excep, proto, pktCmd);
        if (GT_OK != rc)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChIpExceptionCommandSet failed execp %d err %d \n", excep, rc);
            return rc;
        }
    }

    return rc;
}

GT_STATUS cpssHalIpLpmVirtualRouterAdd
(
    GT_U32                                 lpmDBId,
    GT_U32                                 vrId,
    CPSS_DXCH_IP_LPM_VR_CONFIG_STC        *vrConfigPtr
)
{
    GT_STATUS       rc = GT_OK;
    rc = cpssDxChIpLpmVirtualRouterAdd(lpmDBId, vrId, vrConfigPtr);
    if (GT_OK != rc)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChIpLpmVirtualRouterAdd failed Vrf %d err %d \n", vrId, rc);
        return rc;
    }
    return rc;
}

GT_STATUS cpssHalIpLpmVirtualRouterDel(GT_U32 lpmDBId, GT_U32 vrId)
{
    GT_STATUS       rc = GT_OK;
    rc = cpssDxChIpLpmVirtualRouterDel(lpmDBId, vrId);
    if (GT_OK != rc)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChIpLpmVirtualRouterDel failed Vrf %d err %d \n", vrId, rc);
        return rc;
    }
    return rc;
}

GT_STATUS cpssHalBrgVlanVrfIdSet(GT_U32 devId, GT_U16 vlanId, GT_U32 vrfId)
{
    GT_STATUS       rc = GT_OK;
    GT_U8           cpssDevNum;

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        rc = cpssDxChBrgVlanVrfIdSet(cpssDevNum, vlanId, vrfId);
        if (GT_OK != rc)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChBrgVlanVrfIdSet failed vlanId %d Vrf %d err %d \n", vlanId, vrfId, rc);
            return rc;
        }
    }

    return rc;
}

GT_STATUS cpssHalLpmLeafEntryWrite(GT_U32  devId, GT_U32 leafIndex,
                                   CPSS_DXCH_LPM_LEAF_ENTRY_STC *leafPtr)
{
    GT_STATUS       rc = GT_OK;
    GT_U8           cpssDevNum;

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        rc = cpssDxChLpmLeafEntryWrite(cpssDevNum, leafIndex, leafPtr);
        if (GT_OK != rc)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChLpmLeafEntryWrite failed leafIdx %d err %d \n", leafIndex, rc);
            return rc;
        }
    }

    return rc;
}

GT_STATUS cpssHalIpPbrBypassRouterTriggerRequirementsEnableSet(GT_U32  devId,
                                                               GT_BOOL enable)
{
    GT_STATUS       rc = GT_OK;
    GT_U8           cpssDevNum;

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        rc = cpssDxChIpPbrBypassRouterTriggerRequirementsEnableSet(cpssDevNum, enable);
        if (GT_OK != rc)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "IpPbrBypass failed enable %d err %d \n", enable, rc);
            return rc;
        }
    }

    return rc;
}

GT_STATUS cpssHalL3EnableUcRoutingOnPort(GT_U8 cpssDevId, GT_U32 cpssPortNum,
                                         CPSS_IP_PROTOCOL_STACK_ENT protocol, GT_BOOL enable)
{
    GT_STATUS       rc = GT_OK;

    rc = cpssDxChIpPortRoutingEnable(cpssDevId, cpssPortNum,
                                     CPSS_IP_UNICAST_E, protocol, enable);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to enable UC routing on port %d\n", cpssPortNum);
        return (rc);
    }

    return rc;
}

GT_STATUS cpssHalL3EnableBridgeFdbRoutingOnPort(GT_U8 cpssDevId,
                                                GT_U32 cpssPortNum,
                                                CPSS_IP_PROTOCOL_STACK_ENT protocol, GT_BOOL enable)
{
    GT_STATUS       rc = GT_OK;

    rc = cpssDxChBrgFdbRoutingPortIpUcEnableSet(cpssDevId, cpssPortNum,
                                                protocol, enable);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to enable UC routing on port %d\n", cpssPortNum);
        return (rc);
    }

    return rc;
}
