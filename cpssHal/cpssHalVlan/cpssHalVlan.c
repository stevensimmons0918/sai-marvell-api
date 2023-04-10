/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/**
********************************************************************************
* @file cpssHalVlan.c
*
* @brief Private API implementation for CPSS Vlan functions that can be used in XPS layer
*
* @version   01
********************************************************************************
*/

#include "cpssHalVlan.h"
#include "cpssHalUtil.h"
#include "cpssDxChBrgVlan.h"
#include "cpssHalDevice.h"
#include "xpsCommon.h"
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

GT_STATUS cpssHalUpdateMaxVlanCountInHw(int devId, GT_U16 maxVlanId)
{
    int         devNum;
    GT_STATUS   status = GT_OK;

    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        PRV_CPSS_DXCH_PP_HW_INFO_LIMITED_RESOURCES_FIELD_MAC(devNum).vid = maxVlanId;

        if ((status = cpssDxChBrgFdb16BitFidHashEnableSet(devNum, GT_TRUE)!= GT_OK))
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Enabling 16 bit hash failed");
            return status;
        }
    }
    return GT_OK;
}

GT_STATUS cpssHalWriteBrgVlanEntry
(
    int                                   devId,
    GT_U16                                vlanId,
    CPSS_PORTS_BMP_STC                    *portsMembersPtr,
    CPSS_PORTS_BMP_STC                    *portsTaggingPtr,
    CPSS_DXCH_BRG_VLAN_INFO_STC           *vlanInfoPtr,
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC  *portsTaggingCmdPtr
)

{
    GT_STATUS           status = GT_OK;
    int devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        if ((status = cpssDxChBrgVlanEntryWrite(devNum, vlanId, portsMembersPtr,
                                                portsTaggingPtr, vlanInfoPtr, portsTaggingCmdPtr)) != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Vlan %d create failed",
                  vlanId);
            return status;
        }
    }
    return status;
}

GT_STATUS cpssHalReadBrgVlanEntry
(
    int                                  devId,
    GT_U16                               vlanId,
    CPSS_PORTS_BMP_STC                   *portsMembersPtr,
    CPSS_PORTS_BMP_STC                   *portsTaggingPtr,
    CPSS_DXCH_BRG_VLAN_INFO_STC          *vlanInfoPtr,
    GT_BOOL                              *isValidPtr,
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC *portsTaggingCmdPtr,
    CPSS_PORTS_BMP_STC                   *globalPortMembers
)
{
    GT_STATUS           status = GT_OK;
    int devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(portsMembersPtr);
        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(portsTaggingPtr);

        if ((status = cpssDxChBrgVlanEntryRead(devNum, vlanId, portsMembersPtr,
                                               portsTaggingPtr, vlanInfoPtr, isValidPtr, portsTaggingCmdPtr)) != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Read Vlan %d failed", vlanId);
            return status;
        }

        if (!(*isValidPtr))
        {
            cpssOsPrintf("Invalid Vlan\n");
            return XP_ERR_INVALID_VLAN_ID;
        }
        CPSS_PORTS_BMP_BITWISE_OR_MAC(globalPortMembers, globalPortMembers,
                                      portsMembersPtr);
    }
    return status;
}

GT_STATUS cpssHalSetBrgVlanMruProfileIdx
(
    int       devId,
    GT_U16    vlanId,
    GT_U32    mruIndex
)
{
    GT_STATUS           status = GT_OK;
    int devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        if ((status = cpssDxChBrgVlanMruProfileIdxSet(devNum, vlanId,
                                                      mruIndex)) != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Mru profile ID 0 set failed");
            return status;
        }
    }
    return status;
}

GT_STATUS cpssHalSetBrgVlanMruProfileValue
(
    int       devId,
    GT_U32    mruIndex,
    IN GT_U32    mruValue
)
{
    GT_STATUS           status = GT_OK;
    int devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        if ((status = cpssDxChBrgVlanMruProfileValueSet(devNum, mruIndex,
                                                        mruValue)) != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Mru set profile for VLAN failed");
            return status;
        }
    }
    return status;
}


GT_STATUS cpssHalSetBrgVlanForwardingId
(
    int     devId,
    GT_U16  vlanId,
    GT_U32  fidValue
)
{
    GT_STATUS           status = GT_OK;
    int devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        if ((status = cpssDxChBrgVlanForwardingIdSet(devNum, vlanId,
                                                     fidValue)) != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "VlanForwardingIdSet failed");
            return status;
        }
    }
    return status;
}

GT_STATUS cpssHalBindBrgVlanToStpId
(
    int      devId,
    GT_U16   vlanId,
    GT_U16   stpId
)

{
    GT_STATUS           status = GT_OK;
    int devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        if ((status = cpssDxChBrgVlanToStpIdBind(devNum, vlanId, stpId)) != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Vlan to Stp bind failed");
            return status;
        }
    }
    return status;
}

GT_STATUS cpssHalEnableBrgVlanPortIngFlt
(
    int          devId,
    GT_PORT_NUM  portNum,
    GT_BOOL      enable
)

{
    GT_STATUS status = GT_OK;
    int devNum;
    GT_PORT_NUM     cpssPortNum;
    GT_U8           cpssDevNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        cpssDevNum = xpsGlobalIdToDevId(devNum, portNum);
        cpssPortNum = xpsGlobalPortToPortnum(devNum, portNum);
        status = cpssDxChBrgVlanPortIngFltEnable(cpssDevNum, cpssPortNum, enable);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Ingress filtering for the dev (%d) port (%d)  cpssDev (%d) cpssPort (%d) failed",
                  devId, portNum, cpssDevNum, cpssPortNum);
            return status;
        }
    }
    return status;
}

GT_STATUS cpssHalInvalidateBrgVlanEntry
(
    int      devId,
    GT_U16   vlanId
)

{
    GT_STATUS status = GT_OK;
    int devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChBrgVlanEntryInvalidate(devNum, vlanId);

        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Destroy vlan failed");
            return status;
        }

    }
    return status;
}

GT_STATUS cpssHalSetBrgVlanUnkUnregFilter
(
    int                                  devId,
    GT_U16                               vlanId,
    CPSS_DXCH_BRG_VLAN_PACKET_TYPE_ENT   packetType,
    CPSS_PACKET_CMD_ENT                  cmd

)
{
    GT_STATUS status = GT_OK;
    int devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChBrgVlanUnkUnregFilterSet(devNum, vlanId, packetType, cmd);
        if (status != GT_OK)
        {

            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Set Unregistered IPv4  multicast Cmd failed");
            return status;
        }
    }
    return  status;
}

GT_STATUS cpssHalSetBrgGenArpBcastToCpuCmd
(
    int                              devId,
    CPSS_DXCH_ARP_BCAST_CMD_MODE_ENT cmdMode,
    CPSS_PACKET_CMD_ENT              cmd
)
{
    GT_STATUS status = GT_OK;
    int devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChBrgGenArpBcastToCpuCmdSet(devNum, cmdMode, cmd);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Set arp broadcast cmd failed");
            return  status;
        }
    }
    return  status;
}

GT_STATUS cpssHalSetBrgVlanUnknownMacSaCommand
(
    int                  devId,
    GT_U16               vlanId,
    CPSS_PACKET_CMD_ENT  cmd
)
{
    GT_STATUS status = GT_OK;
    int devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChBrgVlanUnknownMacSaCommandSet(devNum, vlanId, cmd);
        if (GT_OK != status)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Set unknown SA cmd failed");
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }
    return  status;
}

GT_STATUS cpssHalEnableBrgVlanIgmpSnooping
(
    int     devId,
    GT_U16  vlanId,
    GT_BOOL enable
)
{
    GT_STATUS status = GT_OK;
    int devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChBrgVlanIgmpSnoopingEnable(devNum, vlanId, enable);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Setting IGMP Cmd failed");
            return status;
        }
    }
    return status;
}

GT_STATUS cpssHalEnableBrgVlanIpV6IcmpToCpu
(
    int        devId,
    GT_U16     vlanId,
    GT_BOOL    enable
)
{
    GT_STATUS status = GT_OK;
    int devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChBrgVlanIpV6IcmpToCpuEnable(devNum, vlanId, enable);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Program ICMPv6 Cmd failed");
            return status;
        }

    }
    return status;
}

GT_STATUS cpssHalEnableBrgVlanNaToCpu
(
    int       devId,
    GT_U16    vlanId,
    GT_BOOL   enable
)
{
    GT_STATUS status = GT_OK;
    int devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChBrgVlanNaToCpuEnable(devNum, vlanId, enable);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Vlan to CPU NA update enable failed");
            return status;
        }

    }
    return status;
}

GT_STATUS cpssHalBrgVlanIngressTpidProfileSet
(
    int       devId,
    GT_U32               profile,
    CPSS_ETHER_MODE_ENT  ethMode,
    GT_U32               tpidBmp
)
{
    GT_STATUS status = GT_OK;
    int devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChBrgVlanIngressTpidProfileSet(devNum, profile, ethMode,
                                                      tpidBmp);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChBrgVlanIngressTpidProfileSet Failed : %d \n", status);
            return status;
        }

    }
    return status;
}

GT_STATUS cpssHalVlanUnkUnregFilterSet
(
    int       devId,
    GT_U16                               vlanId,
    CPSS_DXCH_BRG_VLAN_PACKET_TYPE_ENT   packetType,
    CPSS_PACKET_CMD_ENT                  cmd
)
{
    GT_STATUS status = GT_OK;
    int devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChBrgVlanUnkUnregFilterSet(devNum, vlanId, packetType, cmd);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChBrgVlanUnkUnregFilterSet Failed Vlan: %d rc %d\n", vlanId, status);
            return status;
        }

    }
    return status;
}

GT_STATUS cpssHalBrgVlanIpmBridgingModeSet(int       devId,
                                           GT_U16    vlanId,
                                           CPSS_IP_PROTOCOL_STACK_ENT  ipVer,
                                           CPSS_BRG_IPM_MODE_ENT       ipmMode)
{
    GT_STATUS status = GT_OK;
    int devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChBrgVlanIpmBridgingModeSet(devNum, vlanId, ipVer, ipmMode);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChBrgVlanIpmBridgingModeSet Failed Vlan: %d rc %d\n", vlanId, status);
            return status;
        }

    }
    return status;
}

GT_STATUS cpssHalBrgVlanIpmBridgingEnable(int       devId,
                                          GT_U16    vlanId,
                                          CPSS_IP_PROTOCOL_STACK_ENT  ipVer,
                                          GT_BOOL enable)
{
    GT_STATUS status = GT_OK;
    int devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChBrgVlanIpmBridgingEnable(devNum, vlanId, ipVer, enable);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChBrgVlanIpmBridgingEnable Failed Vlan: %d rc %d\n", vlanId, status);
            return status;
        }
    }
    return status;
}

GT_STATUS cpssHalBrgVlanIpMcRouteEnable(int       devId,
                                        GT_U16    vlanId,
                                        CPSS_IP_PROTOCOL_STACK_ENT  ipVer,
                                        GT_BOOL enable)
{
    GT_STATUS status = GT_OK;
    int devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChBrgVlanIpMcRouteEnable(devNum, vlanId, ipVer, enable);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChBrgVlanIpMcRouteEnable Failed Vlan: %d rc %d\n", vlanId, status);
            return status;
        }
    }
    return status;
}

GT_STATUS cpssHalVlanMemberAdd
(
    uint32_t                cpssDevId,
    GT_U16                  vlanId,
    uint32_t                cpssPortNum,
    GT_BOOL                 isTagged,
    CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT portTaggingCmd
)
{
    GT_STATUS status = GT_OK;

    status = cpssDxChBrgVlanMemberAdd(cpssDevId, vlanId, cpssPortNum, isTagged,
                                      portTaggingCmd);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChBrgVlanMemberAdd Failed Vlan: %d rc %d\n", vlanId, status);
        return status;
    }
    return status;
}

GT_STATUS cpssHalBrgVlanPortDelete
(
    uint32_t                cpssDevId,
    GT_U16                  vlanId,
    uint32_t                cpssPortNum
)
{
    GT_STATUS status = GT_OK;

    status = cpssDxChBrgVlanPortDelete(cpssDevId, vlanId, cpssPortNum);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChBrgVlanPortDelete Failed Vlan: %d rc %d\n", vlanId, status);
        return status;
    }
    return status;
}

GT_STATUS cpssHalVlanTranslationEntryWrite(int devId,
                                           GT_U16 vlanId,
                                           CPSS_DIRECTION_ENT direction,
                                           GT_U16 transVlanId)
{
    GT_STATUS status = GT_OK;
    int devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChBrgVlanTranslationEntryWrite(devNum, vlanId, direction,
                                                      transVlanId);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChBrgVlanTranslationEntryWrite Failed Vlan: %d rc %d\n", vlanId, status);
            return status;
        }
    }
    return status;
}

GT_STATUS cpssHalVlanTranslationEntryRead(int devId,
                                          GT_U16 vlanId,
                                          CPSS_DIRECTION_ENT direction,
                                          GT_U16 *transVlanIdPtr)
{
    GT_STATUS status = GT_OK;
    int devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChBrgVlanTranslationEntryRead(devNum, vlanId, direction,
                                                     transVlanIdPtr);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChBrgVlanTranslationEntryRead Failed Vlan: %d rc %d\n", vlanId, status);
            return status;
        }
    }
    return status;
}

GT_STATUS cpssHalVlanPortTranslationEnableSet
(
    uint32_t                cpssDevId,
    uint32_t                cpssPortNum,
    CPSS_DIRECTION_ENT      direction,
    CPSS_DXCH_BRG_VLAN_TRANSLATION_ENT value
)
{
    GT_STATUS status = GT_OK;

    status = cpssDxChBrgVlanPortTranslationEnableSet(cpssDevId, cpssPortNum,
                                                     direction, value);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChBrgVlanPortTranslationEnableSet Failed port: %d rc %d\n", cpssPortNum,
              status);
        return status;
    }
    return status;
}


GT_STATUS cpssHalBrgFdbNaToCpuPerPortSet
(
    GT_U32    devId,
    GT_PORT_NUM  portNum,
    GT_BOOL  enable
)
{
    GT_STATUS status = GT_OK;
    GT_U8 cpssDevId;

    /*NOTE: This API can be called with tunnel ePort*/

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevId)
    {
        status = cpssDxChBrgFdbNaToCpuPerPortSet(cpssDevId, portNum, enable);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChBrgFdbNaToCpuPerPortSet Failed port: %d rc %d\n", portNum,
                  status);
            return status;
        }
    }
    return status;
}

GT_STATUS cpssHalBrgVlanMemberTagCmdSet
(
    GT_U32    cpssDevId,
    GT_U16 vid,
    GT_PHYSICAL_PORT_NUM  portNum,
    GT_BOOL  tagged,
    CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT tagType
)
{
    GT_STATUS status = GT_OK;

    status = cpssDxChBrgVlanMemberSet(cpssDevId, vid, portNum, GT_TRUE, tagged,
                                      tagType);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChBrgVlanMemberSet Failed port: %d rc %d\n", portNum,
              status);
        return status;
    }
    return status;
}
