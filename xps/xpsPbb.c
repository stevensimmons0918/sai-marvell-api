// xpsPbb.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpsPbb.h"
#include "xpsInternal.h"
#include "xpsScope.h"
#include "xpsLock.h"


#ifdef __cplusplus
extern "C" {
#endif


XP_STATUS xpsPbbCreateTunnelInterface(macAddr_t bSa, macAddr_t bDa,
                                      uint32_t bTag, xpsInterfaceId_t *pbbTnlId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPbbCreateTunnelInterfaceScope(xpsScope_t scopeId, macAddr_t bSa,
                                           macAddr_t bDa, uint32_t bTag, xpsInterfaceId_t *pbbTnlId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPbbAddTunnelEntry(xpsDevice_t devId, xpsInterfaceId_t pbbTnlId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPbbBindPortToTunnel(xpsDevice_t devId, xpsInterfaceId_t pbbTnlId,
                                 xpsPort_t port)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


XP_STATUS xpsPbbRemoveTunnelEntry(xpsDevice_t devId, xpsInterfaceId_t pbbIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPbbDestroyTunnelInterface(xpsInterfaceId_t pbbTnlId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPbbDestroyTunnelInterfaceScope(xpsScope_t scopeId,
                                            xpsInterfaceId_t pbbTnlId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPbbAddIsid(xpsDevice_t devId, uint32_t isid, xpsVlan_t vlanId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPbbRemoveIsid(xpsDevice_t devId, uint32_t isid)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

#ifdef __cplusplus
}
#endif

