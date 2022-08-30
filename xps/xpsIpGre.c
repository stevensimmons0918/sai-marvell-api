// xpsIpGre.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpsIpGre.h"
#include "xpsTunnel.h"
#include "xpsInternal.h"
#include "xpsLock.h"


#ifdef __cplusplus
extern "C" {
#endif

XP_STATUS xpsIpGreCreateTunnelInterface(ipv4Addr_t lclEpIpAddr,
                                        ipv4Addr_t rmtEpIpAddr, xpsInterfaceId_t *tnlIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsIpGreCreateTunnelInterfaceScope(xpsScope_t scopeId,
                                             ipv4Addr_t lclEpIpAddr, ipv4Addr_t rmtEpIpAddr, xpsInterfaceId_t *tnlIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsIpGreAddTunnelEntry(xpsDevice_t devId, xpsInterfaceId_t tnlIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsIpGreRemoveTunnelEntry(xpsDevice_t devId,
                                    xpsInterfaceId_t tnlIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsIpGreDestroyTunnelInterface(xpsInterfaceId_t tnlIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsIpGreDestroyTunnelInterfaceScope(xpsScope_t scopeId,
                                              xpsInterfaceId_t tnlIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsIpGreSetTunnelNextHopData(xpsDevice_t devId,
                                       xpsInterfaceId_t tnlIntfId, uint32_t nhId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsIpGreUpdateTunnelNextHopData(xpsDevice_t devId,
                                          xpsInterfaceId_t tnlIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsIpGreSetTunnelConfig(xpsDevice_t devId, xpsInterfaceId_t tnlIntfId,
                                  xpsIpGreTunnelConfig_t *tunnelConfig)
{
    XP_STATUS rt = XP_NO_ERR;
    xpsIpTunnelConfig_t cfg;

    XPS_FUNC_ENTRY_LOG();

    if (IS_DEVICE_VALID(devId) == 0)
    {
        XPS_FUNC_EXIT_LOG();
        return XP_ERR_INVALID_DEV_ID;
    }

    cfg.ipgreCfg = *tunnelConfig;

    rt = xpsIpTunnelSetConfig(devId, tnlIntfId, XP_IP_GRE_ERSPAN2_TUNNEL, &cfg);
    if (rt)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Ip tunnel Set GRE config failed, tunnel interface(%d)", tnlIntfId);
        XPS_FUNC_EXIT_LOG();
        return rt;
    }

    XPS_FUNC_EXIT_LOG();
    return rt;
}

XP_STATUS xpsIpGreGetTunnelConfig(xpsDevice_t devId, xpsInterfaceId_t tnlIntfId,
                                  xpsIpGreTunnelConfig_t *tunnelConfig)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsIpGreTunnelAttachL3Intf(xpsDevice_t devId,
                                     xpsInterfaceId_t tnlIntfId, xpsInterfaceId_t l3IntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsIpGreTunnelDetachL3Intf(xpsDevice_t devId,
                                     xpsInterfaceId_t tnlIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsIpGreGetTunnelRemoteIp(xpsDevice_t devId,
                                    xpsInterfaceId_t tnlIntfId, ipv4Addr_t *rmtEpIpAddr)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

#ifdef __cplusplus
}
#endif
