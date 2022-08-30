// xpsAc.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpsLock.h"
#include "xpsInterface.h"
#include "xpsState.h"
#include "xpsVlan.h"
#include "xpTypes.h"
#include "xpsUtil.h"
#include "xpsInternal.h"
#include "xpsScope.h"
#include "xpsPort.h"
#include "xpsAc.h"

#ifdef __cplusplus
extern "C" {
#endif

#define XPS_AC_GBL_DB_NAME                  ("XPS AC Global DB")
#define XPS_AC_INDEX_DB_NAME                ("XPS AC index DB")
#define XPS_AC_INTF_PVID_DB_NAME            ("XPS AC Inteface PVID DB")

#define IS_SPECIFIC_VID_MATCH(matchType)    (((matchType) == XPS_AC_OUTER_VID) || ((matchType) == XPS_AC_OUTER_VID_ONLY_TAGGED))

/*
 * AC index DB entry:
 * This is per-device entry that holds HW index information for AC.
 * For VID match ACs, tableIdx holds port-vlan index.
 * For catchAll ACs, tableIdx holds port AC table index
 */

typedef struct xpsAcIndexDbEntry_t
{
    xpsInterfaceId_t    acIntfId;
    uint32_t
    tableIdx; // portVlan table index or TT table index based on AC type
} xpsAcIndexDbEntry_t;

/*
 * Global AC info DB entry:
 * Holds information ons key and allocated Ids
 */
typedef struct xpsAcGblDbEntry_t
{
    xpsInterfaceId_t    acIntfId; // AC inteface Id
    xpsDeviceMap        deviceMap;
    xpsAcInfo_t         acInfo;
} xpsAcGblDbEntry_t;


typedef struct xpsAcIntfPvidDbEntry_t
{
    xpsInterfaceId_t interfaceId;
    xpsVlan_t pvid;
    xpsDeviceMap deviceMap;
} xpsAcIntfPvidDbEntry_t;

/*
 * Database handles
 */





/* Static  forward declarations */




















































/* Find the AC which uses the given interface and vlan as match criteria */








XP_STATUS xpsAcConfigurePvidOnInterface(xpsDevice_t devId,
                                        xpsInterfaceId_t interfaceId, xpsVlan_t pvid)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsAcRemovePvidFromInterface(xpsDevice_t devId,
                                       xpsInterfaceId_t interfaceId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsAcIsPvidExistForInterface(xpsDevice_t devId,
                                       xpsInterfaceId_t interfaceId, uint8_t *exists)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}




XP_STATUS xpsAcInit()
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsAcInitScope(xpsScope_t scope)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsAcDeinit()
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsAcDeinitScope(xpsScope_t scope)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


XP_STATUS xpsAcAddDevice(xpsDevice_t devId, xpsInitType_t initType)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsAcRemoveDevice(xpsDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsAcCreate(xpsAcInfo_t *acInfo, xpsInterfaceId_t *acIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsAcCreateScope(xpsScope_t scopeId, xpsAcInfo_t *acInfo,
                           xpsInterfaceId_t *acIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsAcDestroy(xpsInterfaceId_t acIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsAcDestroyScope(xpsScope_t scope, xpsInterfaceId_t acIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsAcSetConfig(xpsDevice_t devId, xpsInterfaceId_t acIntfId,
                         xpsAcConfig_t *config)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsAcGetConfig(xpsDevice_t devId, xpsInterfaceId_t acIntfId,
                         xpsAcConfig_t *config)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsAcGetAcMatchType(xpsScope_t scopeId, xpsInterfaceId_t acIntfId,
                              xpsAcMatchType_e *matchType)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsAcGetPhysInterface(xpsScope_t scopeId, xpsInterfaceId_t acIntfId,
                                xpsInterfaceId_t *physIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsAcGetAcVid(xpsScope_t scopeId, xpsInterfaceId_t acIntfId,
                        xpsVlan_t *vid)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsAcBindWithVsi(xpsDevice_t devId, xpsInterfaceId_t acIntfId,
                           xpsVsi_t vsiId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsAcUnbindWithVsi(xpsDevice_t devId, xpsInterfaceId_t acIntfId,
                             xpsVsi_t vsiId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsAcHandlePortVlanTableRehash(xpsDevice_t devId,
                                         xpsHashIndexList_t *indexList)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsAcHandleTunnelIvifTableRehash(xpsDevice_t devId,
                                           xpsHashIndexList_t *indexList)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsAcResolveIngressAc(xpsDevice_t devId, xpsInterfaceId_t physIntf,
                                uint8_t isTagged, xpVlan_t tagVlanId, xpsInterfaceId_t *acIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

#ifdef __cplusplus
}
#endif
