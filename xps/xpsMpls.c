// xpsMpls.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpsMpls.h"
#include "xpsInternal.h"
#include "xpsScope.h"
#include "xpsLock.h"


#ifdef __cplusplus
extern "C" {
#endif

/* Static global handle to the mpls label entry db */


/* Static global handle to the mpls vpn db */


/* Static global handle to the mpls tunnel termination db */


typedef struct xpsMplsVpnDbEntry_t
{
    uint32_t label;
    uint32_t tableIndex;
} xpsMplsVpnDbEntry_t;

typedef struct xpsMplsVpnGreLooseModeDbEntry_t
{
    uint32_t label;
    uint32_t tableIndex;
} xpsMplsVpnGreLooseModeDbEntry_t;

typedef struct xpsMplsLabelDbEntry_t
{
    uint32_t label;
    xpMplsNhEncapType mplsNhType;
    uint32_t lblTableIndex;
} xpsMplsLabelDbEntry_t;

/* Per device MPLS tunnel termination state. */
typedef struct xpsMplsTTDbEntry_t
{
    uint32_t label1;
    uint32_t label2;
    uint32_t tnlTableIndex;
    xpMplsTunnelType_t tnlType;
} xpsMplsTTDbEntry_t;


/* This API is defined as static, but will be registered via
 * function pointer to be used internally by State manager to do
 * comparisons for the interface manager database
 */


/* This API is defined as static, but will be registered via
 * function pointer to be used internally by State manager to do
 * comparisons for the interface manager database
 */


/* This API is defined as static, but will be registered via
 * function pointer to be used internally by State manager to do
 * comparisons for the interface manager database
 */


XP_STATUS xpsMplsInit(void)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMplsInitScope(xpsScope_t scopeId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMplsDeInit(void)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMplsDeInitScope(xpsScope_t scopeId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMplsAddDevice(xpsDevice_t devId, xpsInitType_t initType)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMplsRemoveDevice(xpsDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMplsCreateTunnelInterface(xpsInterfaceId_t *mplsTnlId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMplsCreateTunnelInterfaceScope(xpsScope_t scopeId,
                                            xpsInterfaceId_t *mplsTnlId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMplsAddTunnelOriginationEntry(xpsDevice_t devId,
                                           xpsMplsTunnelKey_t *mplsTnlKey, xpsInterfaceId_t mplsTnlId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMplsAddTunnelTerminationEntry(xpsDevice_t devId,
                                           xpsMplsTunnelKey_t *mplsTnlKey)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMplsRemoveTunnelOriginationEntry(xpsDevice_t devId,
                                              xpsInterfaceId_t mplsTnlId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMplsRemoveTunnelTerminationEntry(xpsDevice_t devId,
                                              xpsMplsTunnelKey_t *mplsTnlKey)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMplsDestroyTunnelInterface(xpsInterfaceId_t mplsTnlId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMplsDestroyTunnelInterfaceScope(xpsScope_t scopeId,
                                             xpsInterfaceId_t mplsTnlId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMplsSetTunnelConfig(xpsDevice_t devId,
                                 xpsMplsTunnelKey_t *mplsTnlKey, xpsMplsTunnelParams_t *tunnelParams)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMplsGetTunnelConfig(xpsDevice_t devId,
                                 xpsMplsTunnelKey_t *mplsTnlKey, xpsMplsTunnelParams_t *tunnelParams)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMplsSetTunnelNextHopData(xpsDevice_t devId,
                                      xpsInterfaceId_t mplsTnlId, uint32_t nextHopId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMplsUpdateTunnelNextHopData(xpsDevice_t devId,
                                         xpsInterfaceId_t mplsTnlId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


XP_STATUS xpsMplsAddVpnEntry(xpsDevice_t devId, uint32_t vpnLabel,
                             uint32_t l3IntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMplsRemoveVpnEntry(xpsDevice_t devId, uint32_t vpnLabel)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMplsSetVpnConfig(xpsDevice_t devId, uint32_t vpnLabel,
                              xpsMplsVpnParams_t *vpnParams)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMplsGetVpnConfig(xpsDevice_t devId, uint32_t vpnLabel,
                              xpsMplsVpnParams_t *vpnParams)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMplsAddVpnGreLooseModeEntry(xpsDevice_t devId, uint32_t vpnLabel,
                                         xpsMplsVpnGreLooseModeParams_t *vpnParams)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMplsRemoveVpnGreLooseModeEntry(xpsDevice_t devId,
                                            uint32_t vpnLabel)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMplsSetVpnGreLooseModeConfig(xpsDevice_t devId, uint32_t vpnLabel,
                                          xpsMplsVpnGreLooseModeParams_t *vpnParams)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMplsGetVpnGreLooseModeConfig(xpsDevice_t devId, uint32_t vpnLabel,
                                          xpsMplsVpnGreLooseModeParams_t *vpnParams)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMplsAddLabelEntry(xpsDevice_t devId,
                               xpsMplsLabelEntry_t *mplsLblEntry)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMplsRemoveLabelEntry(xpsDevice_t devId, uint32_t mplsLabel)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMplsGetLabelEntry(xpsDevice_t devId, uint32_t mplsLabel,
                               xpsMplsLabelEntry_t *mplsLblEntry)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMplsSetLabelEntryRehashLevel(xpDevice_t devId,
                                          uint8_t numOfRehashLevels)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMplsGetLabelEntryRehashLevel(xpDevice_t devId,
                                          uint8_t* numOfRehashLevels)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

#ifdef __cplusplus
}
#endif
