// xpsIpinIp.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpsIpinIp.h"
#include "xpsInternal.h"
#include "xpsLock.h"
#include "xpsGlobalSwitchControl.h"
#include "cpssHalUtil.h"
#include "cpssHalDevice.h"
#include "cpssHalTunnel.h"
#include "cpssHalL3.h"
#include "cpss/dxCh/dxChxGen/tti/cpssDxChTti.h"


#ifdef __cplusplus
extern "C" {
#endif

XP_STATUS xpsIpinIpInit(xpsScope_t scopeId)
{
    XPS_LOCK(xpsIpinIpInitTermTable);
    uint32_t devId = 0;
    uint8_t devNum = 0;

    XP_STATUS rt = XP_NO_ERR;
    GT_STATUS   rc = GT_OK;

    CPSS_DXCH_TTI_MAC_MODE_ENT      dxChMacMode;
    CPSS_DXCH_TTI_KEY_TYPE_ENT      dxChKeyType = CPSS_DXCH_TTI_KEY_IPV4_E;

    /*
     * Since P2MP tunnels is not associated with any port,
     * enable below configurations by default on all devices.
     */

    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        rc = cpssHalTtiMacModeGet(devNum, dxChKeyType, &dxChMacMode);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " cpssHalTtiMacModeGet failed with err : %d\n for Device :%d ", rc, devNum);
            return xpsConvertCpssStatusToXPStatus(rc);
        }

        if (dxChMacMode != CPSS_DXCH_TTI_MAC_MODE_DA_E)
        {
            rc = cpssHalTtiMacModeSet(devNum, dxChKeyType, CPSS_DXCH_TTI_MAC_MODE_DA_E);
            if (rc != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      " cpssHalTtiMacModeSet failed with err : %d\n for Device :%d ", rc, devNum);
                return xpsConvertCpssStatusToXPStatus(rc);
            }
        }

        rc = cpssHalTunnelTerminationIPv6KeySet(devNum);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " cpssHalTunnelTerminationIPv6KeySet failed with err : %d\n for Device :%d ",
                  rc, devNum);
            return xpsConvertCpssStatusToXPStatus(rc);
        }

    }
    return rt;
}

XP_STATUS xpsIpinIpCreateTunnelInterface(xpsIpTunnelData_t *ipTnlData,
                                         xpsInterfaceId_t *tnlIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    return xpsIpinIpCreateTunnelInterfaceScope(XP_SCOPE_DEFAULT, ipTnlData,
                                               tnlIntfId);
}

XP_STATUS xpsIpinIpCreateTunnelInterfaceScope(xpsScope_t scopeId,
                                              xpsIpTunnelData_t *ipTnlData, xpsInterfaceId_t *tnlIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsIpinIpCreateTunnelInterfaceScope);

    XP_STATUS rt = XP_NO_ERR;

    rt = xpsIpTunnelCreate(scopeId, XPS_TUNNEL_IP_IN_IP, ipTnlData, tnlIntfId);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Ip tunnel Creation failed");
        return rt;
    }

    XPS_FUNC_EXIT_LOG();
    return rt;
}

XP_STATUS xpsIpinIpAddTunnelEntry(xpsDevice_t devId, xpsInterfaceId_t tnlIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsIpinIpAddTunnelEntry);

    XP_STATUS rt = XP_NO_ERR;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        return XP_ERR_INVALID_DEV_ID;
    }

    rt = xpsIpTunnelAdd(devId, tnlIntfId);

    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Ip tunnel Add failed");
    }
#if 0
    rt = xpsIpTunnelAddTermination(devId, tnlIntfId, 0xFFFFFFFF);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Ip tunnel Add failed");
    }

#endif
    XPS_FUNC_EXIT_LOG();

    return rt;
}

XP_STATUS xpsIpinIpRemoveTunnelEntry(xpsDevice_t devId,
                                     xpsInterfaceId_t tnlIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS rt = XP_NO_ERR;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        return XP_ERR_INVALID_DEV_ID;
    }

    rt = xpIpTunnelRemove(devId, XP_IP_OVER_IP_TUNNEL, tnlIntfId);

    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Ip tunnel Remove failed");
        return rt;
    }

#if 0
    rt = xpsIpTunnelDeleteTermination(devId, tnlIntfId, 0xFFFFFFFF);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Ip tunnel Delete Termination failed");
        return rt;
    }
#endif
    XPS_FUNC_EXIT_LOG();
    return rt;
}

XP_STATUS xpsIpinIpDestroyTunnelInterface(xpsInterfaceId_t tnlIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    return xpsIpinIpDestroyTunnelInterfaceScope(XP_SCOPE_DEFAULT, tnlIntfId);
}

XP_STATUS xpsIpinIpDestroyTunnelInterfaceScope(xpsScope_t scopeId,
                                               xpsInterfaceId_t tnlIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsIpinIpDestroyTunnelInterfaceScope);

    XP_STATUS rt = XP_NO_ERR;

    rt = xpsIpTunnelDelete(scopeId, tnlIntfId);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Ip tunnel delete failed");
        return rt;
    }

    XPS_FUNC_EXIT_LOG();
    return rt;
}

XP_STATUS xpsIpinIpSetTunnelNextHopData(xpsDevice_t devId,
                                        xpsInterfaceId_t tnlIntfId, uint32_t nhId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsIpinIpUpdateTunnelNextHopData(xpsDevice_t devId,
                                           xpsInterfaceId_t tnlIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsIpinIpSetTunnelConfig(xpsDevice_t devId,
                                   xpsInterfaceId_t tnlIntfId, xpsIpinIpTunnelConfig_t *tunnelConfig)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsIpinIpGetTunnelConfig(xpsDevice_t devId,
                                   xpsInterfaceId_t tnlIntfId, xpsIpinIpTunnelConfig_t *tunnelConfig)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsIpinIpTunnelAttachL3Intf(xpsDevice_t devId,
                                      xpsInterfaceId_t tnlIntfId, xpsInterfaceId_t l3IntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsIpinIpTunnelDetachL3Intf(xpsDevice_t devId,
                                      xpsInterfaceId_t tnlIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsIpinIpGetTunnelRemoteIp(xpsDevice_t devId,
                                     xpsInterfaceId_t tnlIntfId, ipv4Addr_t *rmtEpIpAddr)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

#ifdef __cplusplus
}
#endif
