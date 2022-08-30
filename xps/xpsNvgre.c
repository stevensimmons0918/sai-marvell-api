// xpsNvgre.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpsNvgre.h"
#include "xpsVxlan.h"
#include "xpsCommon.h"
#include "xpsInternal.h"
#include "xpsScope.h"


#ifdef __cplusplus
extern "C" {
#endif

XP_STATUS xpsNvgreCreateTunnelInterface(ipv4Addr_t lclEpIpAddr,
                                        ipv4Addr_t rmtEpIpAddr, xpsInterfaceId_t *tnlIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}
XP_STATUS xpsNvgreCreateTunnelInterfaceScope(xpsScope_t scopeId,
                                             ipv4Addr_t lclEpIpAddr, ipv4Addr_t rmtEpIpAddr, xpsInterfaceId_t *tnlIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsNvgreAddTunnelEntry(xpsDevice_t devId, xpsInterfaceId_t tnlIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsNvgreRemoveTunnelEntry(xpsDevice_t devId,
                                    xpsInterfaceId_t tnlIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsNvgreDestroyTunnelInterface(xpsInterfaceId_t tnlIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}
XP_STATUS xpsNvgreDestroyTunnelInterfaceScope(xpsScope_t scopeId,
                                              xpsInterfaceId_t tnlIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsNvgreSetTunnelNextHopData(xpsDevice_t devId,
                                       xpsInterfaceId_t tnlIntfId, uint32_t nhId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsNvgreUpdateTunnelNextHopData(xpsDevice_t devId,
                                          xpsInterfaceId_t tnlIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


XP_STATUS xpsNvgreAddLocalNve(xpsDevice_t devId, ipv4Addr_t localIp)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsNvgreRemoveLocalNve(xpsDevice_t devId, ipv4Addr_t localIp)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsNvgreSetTunnelConfig(xpsDevice_t devId, xpsInterfaceId_t tnlIntfId,
                                  xpsNvgreTunnelConfig_t *tunnelConfig)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsNvgreGetTunnelConfig(xpsDevice_t devId, xpsInterfaceId_t tnlIntfId,
                                  xpsNvgreTunnelConfig_t *tunnelConfig)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsNvgreGetTunnelRemoteIp(xpsDevice_t devId,
                                    xpsInterfaceId_t tnlIntfId, ipv4Addr_t *rmtEpIpAddr)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsNvgreAddTni(xpsDevice_t devId, uint32_t tni, xpsVlan_t vlanId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsNvgreRemoveTni(xpsDevice_t devId, uint32_t tni)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsNvgreAddMcTunnelEntry(xpsDevice_t devId,
                                   xpsInterfaceId_t tnlIntfId, ipv4Addr_t lclEpIpAddr, ipv4Addr_t rmtEpIpAddr,
                                   xpsInterfaceId_t l3IntfId, xpsInterfaceId_t portIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

#ifdef __cplusplus
}
#endif
