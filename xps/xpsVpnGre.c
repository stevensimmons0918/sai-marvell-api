// xpsVpnGre.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpsVpnGre.h"
#include "xpsTunnel.h"
#include "xpsInternal.h"
#include "xpsScope.h"
#include "xpsLock.h"


#ifdef __cplusplus
extern "C" {
#endif

XP_STATUS xpsVpnGreCreateLooseModeIpTunnelInterface(ipv4Addr_t lclEpIpAddr,
                                                    ipv4Addr_t rmtEpIpAddr, xpsInterfaceId_t *tnlIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVpnGreCreateLooseModeIpTunnelInterfaceScope(xpsScope_t scopeId,
                                                         ipv4Addr_t lclEpIpAddr, ipv4Addr_t rmtEpIpAddr, xpsInterfaceId_t *tnlIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVpnGreCreateStrictModeTunnelInterface(ipv4Addr_t lclEpIpAddr,
                                                   ipv4Addr_t rmtEpIpAddr, uint32_t vpnLabel, xpsInterfaceId_t *tnlIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVpnGreCreateStrictModeTunnelInterfaceScope(xpsScope_t scopeId,
                                                        ipv4Addr_t lclEpIpAddr, ipv4Addr_t rmtEpIpAddr, uint32_t vpnLabel,
                                                        xpsInterfaceId_t *tnlIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVpnGreAddTunnelEntry(xpsDevice_t devId, xpsInterfaceId_t tnlIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVpnGreRemoveTunnelEntry(xpsDevice_t devId,
                                     xpsInterfaceId_t tnlIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVpnGreDestroyTunnelInterface(xpsInterfaceId_t tnlIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVpnGreDestroyTunnelInterfaceScope(xpsScope_t scopeId,
                                               xpsInterfaceId_t tnlIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVpnGreSetTunnelNextHopData(xpsDevice_t devId,
                                        xpsInterfaceId_t tnlIntfId, uint32_t nhId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVpnGreUpdateTunnelNextHopData(xpsDevice_t devId,
                                           xpsInterfaceId_t tnlIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVpnGreSetTunnelConfig(xpsDevice_t devId,
                                   xpsInterfaceId_t tnlIntfId, xpsVpnGreTunnelConfig_t *tunnelConfig)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVpnGreGetTunnelConfig(xpsDevice_t devId,
                                   xpsInterfaceId_t tnlIntfId, xpsVpnGreTunnelConfig_t *tunnelConfig)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVpnGreStrictModeTunnelAttachL3Intf(xpsDevice_t devId,
                                                xpsInterfaceId_t tnlIntfId, xpsInterfaceId_t l3IntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVpnGreStrictModeTunnelDetachL3Intf(xpsDevice_t devId,
                                                xpsInterfaceId_t tnlIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVpnGreGetTunnelRemoteIp(xpsDevice_t devId,
                                     xpsInterfaceId_t tnlIntfId, ipv4Addr_t *rmtEpIpAddr)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

#ifdef __cplusplus
}
#endif
