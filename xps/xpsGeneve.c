// xpsGeneve.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpsGeneve.h"
#include "xpsCommon.h"
#include "xpsInternal.h"
#include "xpsScope.h"


#ifdef __cplusplus
extern "C" {
#endif

XP_STATUS xpsGeneveCreateTunnelInterface(ipv4Addr_t lclEpIpAddr,
                                         ipv4Addr_t rmtEpIpAddr, xpsGeneveFormatType_t optionFormat,
                                         xpsInterfaceId_t *intfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGeneveCreateTunnelInterfaceScope(xpsScope_t scopeId,
                                              ipv4Addr_t lclEpIpAddr, ipv4Addr_t rmtEpIpAddr,
                                              xpsGeneveFormatType_t optionFormat, xpsInterfaceId_t *intfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGeneveAddTunnelEntry(xpsDevice_t devId, xpsInterfaceId_t intfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGeneveRemoveTunnelEntry(xpsDevice_t devId,
                                     xpsInterfaceId_t tnlIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGeneveDestroyTunnelInterface(xpsInterfaceId_t tnlIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGeneveDestroyTunnelInterfaceScope(xpsScope_t scopeId,
                                               xpsInterfaceId_t tnlIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGeneveSetTunnelNextHopData(xpsDevice_t devId,
                                        xpsInterfaceId_t tnlIntfId, uint32_t nhId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGeneveUpdateTunnelNextHopData(xpsDevice_t devId,
                                           xpsInterfaceId_t tnlIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGeneveAddLocalEndpoint(xpsDevice_t devId, ipv4Addr_t localIp)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGeneveRemoveLocalEndpoint(xpsDevice_t devId, ipv4Addr_t localIp)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


XP_STATUS xpsGeneveTunnelBindOption(xpsDevice_t devId,
                                    xpsInterfaceId_t baseIntfId, xpsInterfaceId_t optIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGeneveSetTunnelConfig(xpsDevice_t devId,
                                   xpsInterfaceId_t tnlIntfId, xpsGeneveTunnelConfig_t *tunnelConfig)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGeneveGetTunnelConfig(xpsDevice_t devId,
                                   xpsInterfaceId_t tnlIntfId, xpsGeneveTunnelConfig_t *tunnelConfig)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGeneveGetTunnelRemoteIp(xpsDevice_t devId,
                                     xpsInterfaceId_t tnlIntfId, ipv4Addr_t *rmtEpIpAddr)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGeneveAddVni(xpsDevice_t devId, uint32_t vni, xpsVlan_t vlanId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGeneveRemoveVni(xpsDevice_t devId, uint32_t vni)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsGeneveAddMcTunnelEntry(xpsDevice_t devId,
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
