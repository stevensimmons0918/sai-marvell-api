// xpsErspanGre.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpsErspanGre.h"
#include "xpsTunnel.h"
#include "xpsInternal.h"
#include "xpsIpGre.h"
#ifdef __cplusplus
extern "C" {
#endif

XP_STATUS xpsErspanGreCreateTunnelInterface(inetAddr_t *lclEpIpAddr,
                                            inetAddr_t *rmtEpIpAddr, xpsInterfaceId_t *tnlIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status;

    status = xpsErspanGreCreateTunnelInterfaceScope(XP_SCOPE_DEFAULT, lclEpIpAddr,
                                                    rmtEpIpAddr, tnlIntfId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to create Erspan GRE Tunnel Interface with error code %d.\n", status);
        return status;
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsErspanGreCreateTunnelInterfaceScope(xpsScope_t scopeId,
                                                 inetAddr_t *lclEpIpAddr, inetAddr_t *rmtEpIpAddr, xpsInterfaceId_t *tnlIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    xpsIpTunnelData_t ipData;
    XP_STATUS status;

    memset(&ipData, 0, sizeof(xpsIpTunnelData_t));

    ipData.type = XP_IP_GRE_ERSPAN2_TUNNEL;

    ipData.lclEpIpAddr = *lclEpIpAddr;
    ipData.rmtEpIpAddr = *rmtEpIpAddr;

    status = xpsIpTunnelCreate(scopeId, XPS_TUNNEL_GRE_ERSPAN2, &ipData, tnlIntfId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to create Erspan GRE Tunnel with error code %d.\n", status);
        return status;
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsErspanGreAddTunnelEntry(xpsDevice_t devId,
                                     xpsInterfaceId_t tnlIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        XPS_FUNC_EXIT_LOG();
        return XP_ERR_INVALID_DEV_ID;
    }

    status = xpsErspanTunnelAdd(devId, tnlIntfId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to add Erspan GRE Tunnel Entry with error code %d.\n", status);
        return status;
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsErspanGreRemoveTunnelEntry(xpsDevice_t devId,
                                        xpsInterfaceId_t tnlIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        XPS_FUNC_EXIT_LOG();
        return XP_ERR_INVALID_DEV_ID;
    }

    status = xpIpTunnelRemove(devId, XP_IP_GRE_ERSPAN2_TUNNEL, tnlIntfId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to remove Erspan GRE Tunnel Entry with error code %d.\n", status);
        return status;
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsErspanGreDestroyTunnelInterface(xpsInterfaceId_t tnlIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status;

    status = xpsErspanGreDestroyTunnelInterfaceScope(XP_SCOPE_DEFAULT, tnlIntfId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to destroy Erspan GRE Tunnel Interface with error code %d.\n", status);
        return status;
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsErspanGreDestroyTunnelInterfaceScope(xpsScope_t scopeId,
                                                  xpsInterfaceId_t tnlIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status;

    status = xpsIpTunnelDelete(scopeId, tnlIntfId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to destroy Erspan GRE Tunnel Interface with error code %d.\n", status);
        return status;
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsErspanGreTunnelCfgSet(xpsDevice_t devId,
                                   xpsInterfaceId_t tnlIntfId, xpsIpGreTunnelConfig_t * data_PTR)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        XPS_FUNC_EXIT_LOG();
        return XP_ERR_INVALID_DEV_ID;
    }

    status = xpsIpGreSetTunnelConfig(devId, tnlIntfId, data_PTR);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set Erspan GRE Tunnel config with error code %d.\n", status);
        return status;
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

#ifdef __cplusplus
}
#endif
