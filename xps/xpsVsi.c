// xpsVsi.c

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
#include "xpsUtil.h"
#include "xpsInternal.h"
#include "xpsScope.h"
#include "xpsStp.h"
#include "xpsVsi.h"


#ifdef __cplusplus
extern "C" {
#endif

#define XPS_VSI_DEFAULT_INTFS       (32)
#define XPS_VSI_DB_NAME             "VSI Database"
#define XPS_VSI_ENCAP_INFO_DB_NAME  "VSI Encap info database"
#define XPS_VSI_ACLIST_DB_NAME      "VSI AC List database"

typedef struct xpsVsiDbEntry_t
{
    xpsVsi_t vsiId;
    xpL2DomainCtx_t l2DomainCtx;
    xpsDeviceMap deviceMap;
    xpsVsiAccessMode_e accessMode;
} xpsVsiDbEntry_t;

/* Per device */
typedef struct xpsVsiAcList_t
{
    xpsVsi_t            vsiId;  /* Key for database */
    uint32_t            numAcs;
    xpsInterfaceId_t    acIntfList[XPS_VSI_DEFAULT_INTFS];
} xpsVsiAcList_t;

/* Global */
typedef struct xpsVsiEncapInfo_t
{
    xpsVsi_t            vsiId;
    xpL2EncapType_e     encapType;
    xpL2Encapdata_t     encapData;
    xpsInterfaceId_t    intefaceId; /* Only for tunnel interfaces */

    xpsDeviceMap        isPresentBitMap;
} xpsVsiEncapInfo_t;

/* Global DB */
/* Global DB */
/* Per-device DB */




























XP_STATUS xpsVsiInit()
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVsiInitScope(xpsScope_t scopeId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVsiDeinit()
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVsiDeinitScope(xpsScope_t scopeId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVsiAddDevice(xpsDevice_t devId, xpsInitType_t initType)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVsiRemoveDevice(xpsDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVsiCreate(xpsVsiAccessMode_e accessMode, xpsVsi_t *vsiId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVsiCreateScope(xpsScope_t scopeId, xpsVsiAccessMode_e accessMode,
                            xpsVsi_t *vsiId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVsiDestroy(xpsVsi_t vsiId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}































/* XXX: Dynamic array related code for AC list is derived from xpsVlan. TODO: Make the code common for both */


















XP_STATUS xpsVsiDestroyScope(xpsScope_t scopeId, xpsVsi_t vsiId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVsiAddAc(xpsDevice_t devId, xpsVsi_t vsiId,
                      xpsInterfaceId_t acIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVsiRemoveAc(xpsDevice_t devId, xpsVsi_t vsiId,
                         xpsInterfaceId_t acIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVsiAddTunnel(xpsDevice_t devId, xpsVsi_t vsiId,
                          xpsInterfaceId_t tnlIntfId, uint32_t vniId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVsiRemoveTunnel(xpsDevice_t devId, xpsVsi_t vsiId,
                             xpsInterfaceId_t tnlIntfId, uint32_t vniId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVsiAdd(xpsDevice_t devId, xpsVsi_t vsiId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVsiRemove(xpsDevice_t devId, xpsVsi_t vsiId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVsiGetBdId(xpsScope_t scopeId, xpsVsi_t vsiId, uint32_t *bdId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVsiGetAccessMode(xpsScope_t scopeId, xpsVsi_t vsiId,
                              xpsVsiAccessMode_e *accessMode)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVsiServiceInstanceCreate(xpsDevice_t devId, uint32_t serviceId,
                                      xpsServiceInstanceType_e serviceType, xpsVsi_t vsiId, xpsServiceIdData_t *data)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVsiGetVsiIdFromBdId(xpsScope_t scopeId, uint32_t bdId,
                                 xpsVsi_t *vsiId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVsiIsVsiBd(xpsScope_t scopeId, uint32_t bdId, uint8_t *isVsi)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVsiSetUnknownSaCmd(xpsDevice_t devId, xpsVsi_t vsiId,
                                xpsPktCmd_e saMissCmd)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVsiGetUnknownSaCmd(xpsDevice_t devId, xpsVsi_t vsiId,
                                xpsPktCmd_e *saMissCmd)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

#ifdef __cplusplus
}
#endif
