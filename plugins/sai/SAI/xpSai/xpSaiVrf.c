// xpSaiVrf.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpSaiVrf.h"
#include "xpSaiUtil.h"
#include "xpsState.h"
#include "xpsL3.h"
#include "xpsAllocator.h"

#ifdef __cplusplus
extern "C" {
#endif

XP_SAI_LOG_REGISTER_API(SAI_API_UNSPECIFIED);


/**
 * \brief VRF Manager State Database Handle
 *
 */
static xpsDbHandle_t vrfDbHandle = XPS_STATE_INVALID_DB_HANDLE;

/**
 * \brief VRF Manager Interface State Database Handle
 *
 */
static xpsDbHandle_t vrfIntfDbHandle = XPS_STATE_INVALID_DB_HANDLE;

/*
 * Static Function Prototypes
 */

static XP_STATUS xpSaiVrfInfoInit(xpSaiVrfDbEntry_t *info);
static int32_t xpSaiVrfKeyCompare(void *key1, void *key2);
static XP_STATUS xpSaiVrfInfoGet(uint32_t vrfId, xpSaiVrfDbEntry_t **ppVrfInfo);
static XP_STATUS xpSaiVrfDestroy(uint32_t vrfId);
static XP_STATUS xpSaiVrfAdminV4StateApply(xpDevice_t devId, uint32_t vrfId);
static XP_STATUS xpSaiVrfAdminV6StateApply(xpDevice_t devId, uint32_t vrfId);
static XP_STATUS xpSaiVrfMacAddressApply(xpDevice_t devId, uint32_t vrfId,
                                         macAddr_t macAddress);
static XP_STATUS xpSaiVrfMacAddressRemove(xpDevice_t devId, uint32_t vrfId);
static int32_t xpSaiVrfIntfKeyCompare(void *key1, void *key2);
static XP_STATUS xpSaiVrfIntfInfoInit(xpSaiVrfIntfDbEntry_t *info);
static int xpSaiVrfIntfExists(xpsInterfaceId_t intfId);
static XP_STATUS xpSaiVrfIntfGetNext(uint32_t vrfId, xpsInterfaceId_t* pIntfId,
                                     xpsInterfaceId_t* pNextIntfId);
static XP_STATUS xpSaiVrfIntfVrfIdSet(xpsInterfaceId_t intfId, uint32_t vrfId);
static XP_STATUS xpSaiVrfIntfAdminV4ReApply(xpDevice_t devId,
                                            xpsInterfaceId_t intfId);
static XP_STATUS xpSaiVrfIntfAdminV6ReApply(xpDevice_t devId,
                                            xpsInterfaceId_t intfId);
static XP_STATUS xpSaiVrfRoutesNumGet(xpDevice_t devId, uint32_t vrfId,
                                      uint32_t* pRoutesNum);
static XP_STATUS xpSaiVrfHostsNumGet(xpDevice_t devId, uint32_t vrfId,
                                     uint32_t* pHostsNum);
static XP_STATUS xpSaiVrfHostsNumSet(xpDevice_t devId, uint32_t vrfId,
                                     uint32_t hostsNum);
static XP_STATUS xpSaiVrfRifsNumInc(xpDevice_t devId, uint32_t vrfId);
static XP_STATUS xpSaiVrfRifsNumDec(xpDevice_t devId, uint32_t vrfId);
static XP_STATUS xpSaiVrfRifsNumGet(xpDevice_t devId, uint32_t vrfId,
                                    uint32_t* pRifsNum);
static XP_STATUS xpSaiVrfGetNext(xpSaiVrfDbEntry_t** pVrfInfo);

/*
 * API Implementation
 */

XP_STATUS xpSaiVrfInit(xpsDevice_t xpSaiDevId)
{
    XP_STATUS status = XP_NO_ERR;
    uint32_t maxVrf = 0;
    status = xpsL3GetMaxVrf(xpSaiDevId, &maxVrf);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsL3GetMaxVrf Failed %d \n", status);
        return status;
    }

    status = xpsAllocatorInitIdAllocator(XP_SCOPE_DEFAULT, XP_SAI_ALLOC_VRF,
                                         maxVrf, XP_SAI_VRF_RANGE_START);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to initialize SAI VRF ID allocator\n");
        return status;
    }

    // Create global VRF DB
    vrfDbHandle = XPSAI_VRF_STATE_DB_HNDLE;
    if ((status = xpsStateRegisterDb(XP_SCOPE_DEFAULT, "VRF DB", XPS_GLOBAL,
                                     &xpSaiVrfKeyCompare, vrfDbHandle)) != XP_NO_ERR)
    {
        vrfDbHandle = XPS_STATE_INVALID_DB_HANDLE;
        XP_SAI_LOG_ERR("Could not create VRF DB\n");
        return status;
    }

    vrfIntfDbHandle = XPSAI_VRF_INTF_STATE_DB_HNDLE;
    // Create global interface VRF DB
    if ((status = xpsStateRegisterDb(XP_SCOPE_DEFAULT, "VRF INTF DB", XPS_GLOBAL,
                                     &xpSaiVrfIntfKeyCompare, vrfIntfDbHandle)) != XP_NO_ERR)
    {
        vrfIntfDbHandle = XPS_STATE_INVALID_DB_HANDLE;
        XP_SAI_LOG_ERR("Could not create interface VRF DB\n");
        return status;
    }

    return status;
}

XP_STATUS xpSaiVrfDeInit(xpsDevice_t xpSaiDevId)
{
    XP_STATUS status = XP_NO_ERR;

    // Purge global VRF DB
    if ((status = xpsStateDeRegisterDb(XP_SCOPE_DEFAULT,
                                       &vrfDbHandle)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not purge VRF DB\n");
        return status;
    }

    // Purge global interface VRF DB
    if ((status = xpsStateDeRegisterDb(XP_SCOPE_DEFAULT,
                                       &vrfIntfDbHandle)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not purge interface VRF DB\n");
        return status;
    }

    return status;
}

XP_STATUS xpSaiVrfCreate(xpDevice_t devId, uint32_t* pVrfId)
{
    XP_STATUS status = XP_NO_ERR;
    xpSaiVrfDbEntry_t *vrfInfo = NULL;
    uint32_t defUcNhId = 0;
    uint32_t defMcNhId = 0;

    if (pVrfId == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer as an argument, devid %d\n", devId);
        return XP_ERR_NULL_POINTER;
    }

    status = xpsAllocatorAllocateId(XP_SCOPE_DEFAULT, XP_SAI_ALLOC_VRF, pVrfId);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not allocate VRF id, devId %d, status %d\n", devId,
                       status);
        return XP_ERR_NOT_FOUND;
    }

    // Create a new VRF Info structure
    if ((status = xpsStateHeapMalloc(sizeof(xpSaiVrfDbEntry_t),
                                     (void**)&vrfInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not allocate VRF structure, devid %d, vrfId %d\n", devId,
                       *pVrfId);
        return status;
    }

    // Initialize the allocated state
    if ((status = xpSaiVrfInfoInit(vrfInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not initialize VRF structure, devid %d, vrfId %d\n",
                       devId, *pVrfId);
        xpsStateHeapFree(vrfInfo);
        return status;
    }

    /* Create VRF in HW */
    status = xpsL3VrfCreate(devId, *pVrfId, &defUcNhId, &defMcNhId);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsL3VrfCreate Failed dev %d, vrfId %d status %d\n",
                       devId, *pVrfId, status);
        xpsStateHeapFree(vrfInfo);
        return status;
    }

    vrfInfo->keyVrfId = *pVrfId;
    vrfInfo->xpsVrfdefUcNhId = defUcNhId;
    vrfInfo->xpsVrfdefMcNhId = defMcNhId;

    // Insert the state into the database
    if ((status = xpsStateInsertData(XP_SCOPE_DEFAULT, vrfDbHandle,
                                     (void*)vrfInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not insert VRF structure to the DB, devid %d, vrfId %d\n",
                       devId, *pVrfId);
        return status;
    }

    return status;
}

XP_STATUS xpSaiVrfRemove(xpDevice_t devId, uint32_t vrfId)
{
    XP_STATUS status    = XP_NO_ERR;
    uint32_t  routesNum = 0;
    uint32_t  hostsNum  = 0;
    uint32_t  rifsNum   = 0;
    xpSaiVrfDbEntry_t* pVrfInfo = NULL;

    status = xpSaiVrfRoutesNumGet(devId, vrfId, &routesNum);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not get number of routes, devId %d, vrfId %d\n", devId,
                       vrfId);
        return status;
    }

    if (routesNum > 0)
    {
        XP_SAI_LOG_ERR("Could not remove VRF from DB as far as %d routes participate, devId %d, vrfId %d\n",
                       routesNum, devId, vrfId);
        return XP_ERR_KEY_NOT_FOUND;
    }

    status = xpSaiVrfHostsNumGet(devId, vrfId, &hostsNum);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not get number of hosts, devId %d, vrfId %d\n", devId,
                       vrfId);
        return status;
    }

    if (hostsNum > 0)
    {
        XP_SAI_LOG_ERR("Could not remove VRF from DB as far as %d hosts participate, devId %d, vrfId %d\n",
                       hostsNum, devId, vrfId);
        return XP_ERR_KEY_NOT_FOUND;
    }

    status = xpSaiVrfRifsNumGet(devId, vrfId, &rifsNum);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not get number of routing interfaces, devId %d, vrfId %d\n",
                       devId, vrfId);
        return status;
    }

    if (rifsNum > 0)
    {
        XP_SAI_LOG_ERR("Could not remove VRF from DB as far as %d routing interfaces participate, devId %d, vrfId %d\n",
                       rifsNum, devId, vrfId);
        return XP_ERR_RESOURCE_BUSY;
    }

    status = xpSaiVrfMacAddressRemove(devId, vrfId);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not remove Router Mac Address. VRF id, devId %d, vrfId %d\n",
                       devId, vrfId);
    }

    /* Remove VRF in HW */

    status = xpSaiVrfInfoGet(vrfId, &pVrfInfo);
    if (status != XP_NO_ERR || pVrfInfo == NULL)
    {
        XP_SAI_LOG_ERR("Could not find VRF, devId %d, vrfId %d\n", devId, vrfId);
        return status;
    }

    status = xpsL3VrfRemove(devId, vrfId, pVrfInfo->xpsVrfdefUcNhId,
                            pVrfInfo->xpsVrfdefMcNhId);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsL3VrfRemove Failed dev %d, vrfId %d status %d\n",
                       devId, vrfId, status);
        return status;
    }
    status = xpsAllocatorReleaseId(XP_SCOPE_DEFAULT, XP_SAI_ALLOC_VRF, vrfId);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not release VRF id, devId %d, vrfId %d\n", devId, vrfId);
        return status;
    }

    status = xpSaiVrfDestroy(vrfId);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not remove VRF from DB, devId %d, vrfId %d\n", devId,
                       vrfId);
        return status;
    }

    return XP_NO_ERR;
}

int xpSaiVrfExists(uint32_t vrfId)
{
    XP_STATUS      status   = XP_NO_ERR;
    xpSaiVrfDbEntry_t *pVrfInfo = NULL;

    status = xpSaiVrfInfoGet(vrfId, &pVrfInfo);
    if (status != XP_NO_ERR)
    {
        return false;
    }

    return (pVrfInfo != NULL);
}

XP_STATUS xpSaiVrfAdminV4StateSet(xpDevice_t devId, uint32_t vrfId,
                                  uint32_t adminV4State)
{
    XP_STATUS       status   = XP_NO_ERR;
    xpSaiVrfDbEntry_t* pVrfInfo = NULL;

    status = xpSaiVrfInfoGet(vrfId, &pVrfInfo);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not find VRF, devId %d, vrfId %d\n", devId, vrfId);
        return XP_ERR_INVALID_ID;
    }

    pVrfInfo->adminV4State = adminV4State;

    xpSaiVrfAdminV4StateApply(devId, vrfId);

    return XP_NO_ERR;
}

XP_STATUS xpSaiVrfAdminV4StateGet(xpDevice_t devId, uint32_t vrfId,
                                  uint32_t* pAdminV4State)
{
    XP_STATUS       status   = XP_NO_ERR;
    xpSaiVrfDbEntry_t* pVrfInfo = NULL;

    if (pAdminV4State == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer as an argument, devId %d, vrfId %d\n", devId,
                       vrfId);
        return XP_ERR_INVALID_ARG;
    }

    status = xpSaiVrfInfoGet(vrfId, &pVrfInfo);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not find VRF, devId %d, vrfId %d\n", devId, vrfId);
        return XP_ERR_INVALID_ID;
    }

    *pAdminV4State = pVrfInfo->adminV4State;

    return XP_NO_ERR;
}

XP_STATUS xpSaiVrfAdminV6StateSet(xpDevice_t devId, uint32_t vrfId,
                                  uint32_t adminV6State)
{
    XP_STATUS       status   = XP_NO_ERR;
    xpSaiVrfDbEntry_t* pVrfInfo = NULL;

    status = (XP_STATUS)xpSaiVrfInfoGet(vrfId, &pVrfInfo);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not find VRF, devId %d, vrfId %d\n", devId, vrfId);
        return XP_ERR_NOT_FOUND;
    }

    pVrfInfo->adminV6State = adminV6State;

    status = xpSaiVrfAdminV6StateApply(devId, vrfId);

    return status;
}

XP_STATUS xpSaiVrfAdminV6StateGet(xpDevice_t devId, uint32_t vrfId,
                                  uint32_t* pAdminV6State)
{
    XP_STATUS       status   = XP_NO_ERR;
    xpSaiVrfDbEntry_t* pVrfInfo = NULL;

    if (pAdminV6State == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer as an argument, devId %d, vrfId %d\n", devId,
                       vrfId);
        return XP_ERR_INVALID_ARG;
    }

    status = xpSaiVrfInfoGet(vrfId, &pVrfInfo);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not find VRF, devId %d, vrfId %d\n", devId, vrfId);
        return XP_ERR_NOT_FOUND;
    }

    *pAdminV6State = pVrfInfo->adminV6State;

    return XP_NO_ERR;
}

XP_STATUS xpSaiVrfMacAddressSet(xpDevice_t devId, uint32_t vrfId,
                                macAddr_t macAddress)
{
    XP_STATUS       status   = XP_NO_ERR;
    xpSaiVrfDbEntry_t* pVrfInfo = NULL;

    status = xpSaiVrfInfoGet(vrfId, &pVrfInfo);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not find VRF, devId %d, vrfId %d\n", devId, vrfId);
        return status;
    }

    if (memcmp(pVrfInfo->macAddress, macAddress, sizeof(sai_mac_t)) == 0)
    {
        XP_SAI_LOG_NOTICE("Existing mac entry!\n");
        return XP_NO_ERR;
    }

    if (!(xpSaiEthAddrIsZero(pVrfInfo->macAddress)))
    {
        status = xpSaiVrfMacAddressRemove(devId, vrfId);
        if (status != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Mac remove failed %d, vrfId %d\n", devId, vrfId);
            return status;
        }
    }

    memcpy(pVrfInfo->macAddress, macAddress, sizeof(macAddr_t));
    status = xpSaiVrfMacAddressApply(devId, vrfId, macAddress);

    return status;
}

sai_status_t xpSaiVrfUnkL3MCPktActionSet(xpDevice_t devId, uint32_t vrfId,
                                         sai_packet_action_t pktAct)
{
    XP_STATUS       status   = XP_NO_ERR;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    xpSaiVrfDbEntry_t* pVrfInfo = NULL;
    xpsPktCmd_e pktCmd;

    status = xpSaiVrfInfoGet(vrfId, &pVrfInfo);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not find VRF, devId %d, vrfId %d\n", devId, vrfId);
        return xpsStatus2SaiStatus(status);
    }

    saiRetVal = xpSaiConvertSaiPacketAction2xps(pktAct, &pktCmd);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to convert vrf Unk MC pkt action, error %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    status = xpsL3UpdateMcNHPktAction(devId, pVrfInfo->xpsVrfdefMcNhId, pktCmd);
    if (status != XP_NO_ERR)
    {
        return xpsStatus2SaiStatus(status);
    }
    pVrfInfo->unkL3MCPktAction = pktAct;

    return saiRetVal;
}

XP_STATUS xpSaiVrfMacAddressRemove(xpDevice_t devId, uint32_t vrfId)
{
    XP_STATUS       status   = XP_NO_ERR;
    xpSaiVrfDbEntry_t* pVrfInfo = NULL;
    sai_mac_t          mac;

    status = xpSaiVrfInfoGet(vrfId, &pVrfInfo);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not find VRF, devId %d, vrfId %d\n", devId, vrfId);
        return status;
    }

    memcpy(mac, pVrfInfo->macAddress, sizeof(macAddr_t));

    status = xpsL3RemoveIngressRouterMac(devId, mac);

    if (status == XP_NO_ERR)
    {
        memset(pVrfInfo->macAddress, 0, sizeof(macAddr_t));
    }

    return status;
}

XP_STATUS xpSaiVrfMacAddressGet(xpDevice_t devId, uint32_t vrfId,
                                macAddr_t macAddress)
{
    XP_STATUS       status   = XP_NO_ERR;
    xpSaiVrfDbEntry_t* pVrfInfo = NULL;

    status = xpSaiVrfInfoGet(vrfId, &pVrfInfo);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not find VRF, devId %d, vrfId %d\n", devId, vrfId);
        return XP_ERR_NOT_FOUND;
    }

    memcpy(macAddress, pVrfInfo->macAddress, sizeof(macAddr_t));

    return XP_NO_ERR;
}

sai_status_t xpSaiVrfUnkL3McPktActionGet(xpDevice_t devId, uint32_t vrfId,
                                         sai_packet_action_t *action)
{
    XP_STATUS       status   = XP_NO_ERR;
    xpSaiVrfDbEntry_t* pVrfInfo = NULL;

    status = xpSaiVrfInfoGet(vrfId, &pVrfInfo);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not find VRF, devId %d, vrfId %d\n", devId, vrfId);
        return xpsStatus2SaiStatus(status);;
    }

    *action = pVrfInfo->unkL3MCPktAction;

    return SAI_STATUS_SUCCESS;
}

XP_STATUS xpSaiVrfRoutesNumInc(xpDevice_t devId, uint32_t vrfId)
{
    XP_STATUS       status   = XP_NO_ERR;
    xpSaiVrfDbEntry_t* pVrfInfo = NULL;

    status = xpSaiVrfInfoGet(vrfId, &pVrfInfo);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not find VRF, devId %d, vrfId %d\n", devId, vrfId);
        return XP_ERR_NOT_FOUND;
    }

    pVrfInfo->routesNum++;

    return XP_NO_ERR;
}

XP_STATUS xpSaiVrfRoutesNumDec(xpDevice_t devId, uint32_t vrfId)
{
    XP_STATUS       status   = XP_NO_ERR;
    xpSaiVrfDbEntry_t* pVrfInfo = NULL;

    status = xpSaiVrfInfoGet(vrfId, &pVrfInfo);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not find VRF, devId %d, vrfId %d\n", devId, vrfId);
        return XP_ERR_NOT_FOUND;
    }

    if (pVrfInfo->routesNum > 0)
    {
        pVrfInfo->routesNum--;
    }

    return XP_NO_ERR;
}

XP_STATUS xpSaiVrfHostsNumInc(xpDevice_t devId, uint32_t vrfId)
{
    XP_STATUS       status   = XP_NO_ERR;
    xpSaiVrfDbEntry_t* pVrfInfo = NULL;

    status = xpSaiVrfInfoGet(vrfId, &pVrfInfo);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not find VRF, devId %d, vrfId %d\n", devId, vrfId);
        return XP_ERR_NOT_FOUND;
    }

    pVrfInfo->hostsNum++;

    return XP_NO_ERR;
}

XP_STATUS xpSaiVrfHostsNumDec(xpDevice_t devId, uint32_t vrfId)
{
    XP_STATUS       status   = XP_NO_ERR;
    xpSaiVrfDbEntry_t* pVrfInfo = NULL;

    status = xpSaiVrfInfoGet(vrfId, &pVrfInfo);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not find VRF, devId %d, vrfId %d\n", devId, vrfId);
        return XP_ERR_NOT_FOUND;
    }

    if (pVrfInfo->hostsNum > 0)
    {
        pVrfInfo->hostsNum--;
    }

    return XP_NO_ERR;
}

XP_STATUS xpSaiVrfHostsClear(xpDevice_t devId)
{
    XP_STATUS           status   = XP_NO_ERR;
    xpSaiVrfDbEntry_t*  pVrf     = NULL;

    while (xpSaiVrfGetNext(&pVrf) == XP_NO_ERR)
    {
        status = xpSaiVrfHostsNumSet(devId, pVrf->keyVrfId, 0);
        if (status != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpSaiVrfHostsNumSet failed, retVal : %d \n", status);
            return status;
        }
    }

    return XP_NO_ERR;
}

//Func: xpSaiIsVlanOrPortRifExist
xpSaiVrfIntfDbEntry_t *xpSaiIsVlanOrPortRifExist(sai_object_id_t oid)
{
    xpSaiVrfIntfDbEntry_t *pVrfIntfInfo = NULL;
    xpSaiVrfIntfDbEntry_t  vrfIntfKey;


    if (xpsStateGetNextData(XP_SCOPE_DEFAULT, vrfIntfDbHandle, NULL,
                            (void **)&pVrfIntfInfo) != XP_NO_ERR)
    {
        return NULL;
    }
    do
    {
        if (pVrfIntfInfo != NULL)
        {
            if (XDK_SAI_OBJID_TYPE_CHECK(oid, SAI_OBJECT_TYPE_PORT) ||
                XDK_SAI_OBJID_TYPE_CHECK(oid, SAI_OBJECT_TYPE_LAG))
            {
                if (pVrfIntfInfo->portOid == oid)
                {
                    return pVrfIntfInfo;
                }
            }
            else
            {
                if (XPS_INTF_MAP_INTF_TO_BD(pVrfIntfInfo->keyIntfId) == xpSaiObjIdValueGet(oid))
                {
                    return pVrfIntfInfo;
                }
            }
            memset(&vrfIntfKey, 0, sizeof(vrfIntfKey));
            vrfIntfKey.keyIntfId = pVrfIntfInfo->keyIntfId;
            if (xpsStateGetNextData(XP_SCOPE_DEFAULT, vrfIntfDbHandle, &vrfIntfKey,
                                    (void **)&pVrfIntfInfo) != XP_NO_ERR)
            {
                return NULL;
            }
        }
    }
    while (pVrfIntfInfo != NULL);

    return NULL;
}

XP_STATUS xpSaiVrfIntfAdd(xpsInterfaceId_t intfId)
{
    XP_STATUS           status      = XP_NO_ERR;
    xpSaiVrfIntfDbEntry_t* vrfIntfInfo = NULL;

    if (xpSaiVrfIntfExists(intfId))
    {
        XP_SAI_LOG_ERR("The interface info already exists, intfId %d\n", intfId);
        return XP_ERR_KEY_EXISTS;
    }

    // Create a new VRF Interface Info structure
    if ((status = xpsStateHeapMalloc(sizeof(xpSaiVrfIntfDbEntry_t),
                                     (void**)&vrfIntfInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not allocate interface VRF structure, intfId %d\n",
                       intfId);
        return status;
    }

    // Initialize the allocated state
    if ((status = xpSaiVrfIntfInfoInit(vrfIntfInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not initialize interface VRF structure, intfId %d\n",
                       intfId);
        return status;
    }

    vrfIntfInfo->keyIntfId = intfId;

    // Insert the state into the database
    if ((status = xpsStateInsertData(XP_SCOPE_DEFAULT, vrfIntfDbHandle,
                                     (void*)vrfIntfInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not insert interface VRF structure, intfId %d\n", intfId);
        return status;
    }

    return status;
}

XP_STATUS xpSaiVrfIntfRemove(xpsInterfaceId_t intfId)
{
    XP_STATUS           status  = XP_NO_ERR;
    xpSaiVrfIntfDbEntry_t* vrfIntfInfo = NULL;
    xpSaiVrfIntfDbEntry_t  vrfIntfKey;
    xpsDevice_t xpsDevId = xpSaiGetDevId();

    vrfIntfKey.keyIntfId = intfId;

    // Remove the corresponding state
    if ((status = xpsStateDeleteData(XP_SCOPE_DEFAULT, vrfIntfDbHandle, &vrfIntfKey,
                                     (void**)&vrfIntfInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not delete interface VRF structure, intfId %d\n", intfId);
        return status;
    }

    if (vrfIntfInfo == NULL)
    {
        XP_SAI_LOG_ERR("Key not found intfId %d\n", intfId);
        return XP_ERR_NOT_FOUND;
    }

    status = xpSaiVrfRifsNumDec(xpsDevId, vrfIntfInfo->vrfId);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVrfRifsNumDec failed, retVal : %d \n\n", status);
        return status;
    }

    // Free the memory allocated for the corresponding state
    if ((status = xpsStateHeapFree(vrfIntfInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not free interface VRF structure, intfId %d\n", intfId);
        return status;
    }

    return status;
}

XP_STATUS xpSaiVrfIntfVrfIdUpdate(xpDevice_t devId, xpsInterfaceId_t intfId,
                                  uint32_t vrfId)
{
    XP_STATUS  status       = XP_NO_ERR;
    uint32_t   oldVrfId     = 0;

    status = xpSaiVrfIntfVrfIdGet(intfId, &oldVrfId);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not get VRF id, devId %d, intfId %d\n", devId, intfId);
        return status;
    }

    status = xpSaiVrfRifsNumDec(devId, oldVrfId);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVrfRifsNumDec failed, retVal : %d \n\n", status);
        return status;
    }

    status = xpSaiVrfIntfVrfIdSet(intfId, vrfId);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not set VRF id, devId %d, intfId %d\n", devId, intfId);
        return status;
    }

    status = xpSaiVrfRifsNumInc(devId, vrfId);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVrfRifsNumInc failed, retVal : %d \n\n", status);
        return status;
    }

    status = xpSaiVrfIntfAdminV4ReApply(devId, intfId);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not apply IPv4 routing admin state to HW, devId %d, intfId %d\n",
                       devId, intfId);
        return status;
    }

    status = xpSaiVrfIntfAdminV6ReApply(devId, intfId);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not apply IPv6 routing admin state to HW, devId %d, intfId %d\n",
                       devId, intfId);
        return status;
    }

    return XP_NO_ERR;
}

XP_STATUS xpSaiVrfIntfAdminV4StateSet(xpsInterfaceId_t intfId,
                                      uint32_t adminV4State)
{
    XP_STATUS           status       = XP_NO_ERR;
    xpSaiVrfIntfDbEntry_t* pVrfIntfInfo = NULL;

    status = xpSaiVrfIntfInfoGet(intfId, &pVrfIntfInfo);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not find VRF interface info, intfId %d\n", intfId);
        return status;
    }

    pVrfIntfInfo->adminV4State = adminV4State;

    return XP_NO_ERR;
}

XP_STATUS xpSaiVrfIntfAdminV4StateGet(xpsInterfaceId_t intfId,
                                      uint32_t* pAdminV4State)
{
    XP_STATUS           status       = XP_NO_ERR;
    xpSaiVrfIntfDbEntry_t* pVrfIntfInfo = NULL;

    if (pAdminV4State == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer as an argument, intfId %d\n", intfId);
        return XP_ERR_INVALID_ARG;
    }

    status = xpSaiVrfIntfInfoGet(intfId, &pVrfIntfInfo);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not find VRF interface info, intfId %d\n", intfId);
        return status;
    }

    *pAdminV4State = pVrfIntfInfo->adminV4State;

    return XP_NO_ERR;
}

XP_STATUS xpSaiVrfIntfAdminV6StateSet(xpsInterfaceId_t intfId,
                                      uint32_t adminV6State)
{
    XP_STATUS           status       = XP_NO_ERR;
    xpSaiVrfIntfDbEntry_t* pVrfIntfInfo = NULL;

    status = xpSaiVrfIntfInfoGet(intfId, &pVrfIntfInfo);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not find VRF interface info, intfId %d\n", intfId);
        return status;
    }

    pVrfIntfInfo->adminV6State = adminV6State;

    return XP_NO_ERR;
}

XP_STATUS xpSaiVrfIntfAdminV6StateGet(xpsInterfaceId_t intfId,
                                      uint32_t* pAdminV6State)
{
    XP_STATUS           status       = XP_NO_ERR;
    xpSaiVrfIntfDbEntry_t* pVrfIntfInfo = NULL;

    if (pAdminV6State == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer as an argument, intfId %d\n", intfId);
        return XP_ERR_INVALID_ARG;
    }

    status = xpSaiVrfIntfInfoGet(intfId, &pVrfIntfInfo);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not find VRF interface info, intfId %d\n", intfId);
        return status;
    }

    *pAdminV6State = pVrfIntfInfo->adminV6State;

    return XP_NO_ERR;
}

XP_STATUS xpSaiVrfIntfMcAdminV4StateSet(xpsInterfaceId_t intfId,
                                        uint32_t mcAdminV4State)
{
    XP_STATUS           status       = XP_NO_ERR;
    xpSaiVrfIntfDbEntry_t* pVrfIntfInfo = NULL;

    status = xpSaiVrfIntfInfoGet(intfId, &pVrfIntfInfo);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not find VRF interface info, intfId %d\n", intfId);
        return status;
    }

    pVrfIntfInfo->mcAdminV4State = mcAdminV4State;

    return XP_NO_ERR;
}

XP_STATUS xpSaiVrfIntfMcAdminV4StateGet(xpsInterfaceId_t intfId,
                                        uint32_t* pMcAdminV4State)
{
    XP_STATUS           status       = XP_NO_ERR;
    xpSaiVrfIntfDbEntry_t* pVrfIntfInfo = NULL;

    if (pMcAdminV4State == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer as an argument, intfId %d\n", intfId);
        return XP_ERR_INVALID_ARG;
    }

    status = xpSaiVrfIntfInfoGet(intfId, &pVrfIntfInfo);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not find VRF interface info, intfId %d\n", intfId);
        return status;
    }

    *pMcAdminV4State = pVrfIntfInfo->mcAdminV4State;

    return XP_NO_ERR;
}

XP_STATUS xpSaiVrfIntfMcAdminV6StateSet(xpsInterfaceId_t intfId,
                                        uint32_t mcAdminV6State)
{
    XP_STATUS           status       = XP_NO_ERR;
    xpSaiVrfIntfDbEntry_t* pVrfIntfInfo = NULL;

    status = xpSaiVrfIntfInfoGet(intfId, &pVrfIntfInfo);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not find VRF interface info, intfId %d\n", intfId);
        return status;
    }

    pVrfIntfInfo->mcAdminV6State = mcAdminV6State;

    return XP_NO_ERR;
}

XP_STATUS xpSaiVrfIntfMcAdminV6StateGet(xpsInterfaceId_t intfId,
                                        uint32_t* pMcAdminV6State)
{
    XP_STATUS           status       = XP_NO_ERR;
    xpSaiVrfIntfDbEntry_t* pVrfIntfInfo = NULL;

    if (pMcAdminV6State == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer as an argument, intfId %d\n", intfId);
        return XP_ERR_INVALID_ARG;
    }

    status = xpSaiVrfIntfInfoGet(intfId, &pVrfIntfInfo);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not find VRF interface info, intfId %d\n", intfId);
        return status;
    }

    *pMcAdminV6State = pVrfIntfInfo->mcAdminV6State;

    return XP_NO_ERR;
}

XP_STATUS xpSaiVrfIntfSrcMacAddrSet(xpsInterfaceId_t intfId,
                                    macAddr_t macAddress)
{
    XP_STATUS           status       = XP_NO_ERR;
    xpSaiVrfIntfDbEntry_t* pVrfIntfInfo = NULL;

    status = xpSaiVrfIntfInfoGet(intfId, &pVrfIntfInfo);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not find VRF interface info, intfId %d\n", intfId);
        return status;
    }

    memcpy(pVrfIntfInfo->mac, macAddress, sizeof(macAddr_t));

    return XP_NO_ERR;
}

XP_STATUS xpSaiVrfIntfSrcMacAddrGet(xpsInterfaceId_t intfId,
                                    macAddr_t macAddress)
{
    XP_STATUS           status       = XP_NO_ERR;
    xpSaiVrfIntfDbEntry_t* pVrfIntfInfo = NULL;

    status = xpSaiVrfIntfInfoGet(intfId, &pVrfIntfInfo);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not find VRF interface info, intfId %d\n", intfId);
        return status;
    }

    memcpy(macAddress, pVrfIntfInfo->mac, sizeof(macAddr_t));

    return XP_NO_ERR;
}

XP_STATUS xpSaiVrfIntfPortSet(xpsInterfaceId_t intfId, sai_object_id_t portOid)
{
    XP_STATUS             status        = XP_NO_ERR;
    xpSaiVrfIntfDbEntry_t *pVrfIntfInfo = NULL;

    status = xpSaiVrfIntfInfoGet(intfId, &pVrfIntfInfo);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not find VRF interface info, intfId %d\n", intfId);
        return status;
    }

    pVrfIntfInfo->portOid = portOid;

    return XP_NO_ERR;
}

XP_STATUS xpSaiVrfIntfVlanIdSet(xpsInterfaceId_t intfId,
                                sai_object_id_t vlanOid)
{
    XP_STATUS             status        = XP_NO_ERR;
    xpSaiVrfIntfDbEntry_t *pVrfIntfInfo = NULL;

    status = xpSaiVrfIntfInfoGet(intfId, &pVrfIntfInfo);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not find VRF interface info, intfId %d\n", intfId);
        return status;
    }

    pVrfIntfInfo->vlanOid = vlanOid;

    return XP_NO_ERR;
}

XP_STATUS xpSaiVrfIntfTypeSet(xpsInterfaceId_t intfId, int32_t rifType)
{
    XP_STATUS             status        = XP_NO_ERR;
    xpSaiVrfIntfDbEntry_t *pVrfIntfInfo = NULL;

    status = xpSaiVrfIntfInfoGet(intfId, &pVrfIntfInfo);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not find VRF interface info, intfId %d\n", intfId);
        return status;
    }

    pVrfIntfInfo->rifType = rifType;

    return status;
}

XP_STATUS xpSaiVrfIntfPortGet(xpsInterfaceId_t intfId, sai_object_id_t *portOid)
{
    XP_STATUS             status        = XP_NO_ERR;
    xpSaiVrfIntfDbEntry_t *pVrfIntfInfo = NULL;

    if (portOid == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer as an argument, intfId %d\n", intfId);
        return XP_ERR_INVALID_ARG;
    }

    status = xpSaiVrfIntfInfoGet(intfId, &pVrfIntfInfo);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not find VRF interface info, intfId %d\n", intfId);
        return status;
    }

    *portOid = pVrfIntfInfo->portOid;

    return XP_NO_ERR;
}

XP_STATUS xpSaiVrfIntfVlanIdGet(xpsInterfaceId_t intfId,
                                sai_object_id_t *vlanOid)
{
    XP_STATUS             status        = XP_NO_ERR;
    xpSaiVrfIntfDbEntry_t *pVrfIntfInfo = NULL;

    if (vlanOid == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer as an argument, intfId %d\n", intfId);
        return XP_ERR_INVALID_ARG;
    }

    status = xpSaiVrfIntfInfoGet(intfId, &pVrfIntfInfo);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not find VRF interface info, intfId %d\n", intfId);
        return status;
    }

    *vlanOid = pVrfIntfInfo->vlanOid;

    return XP_NO_ERR;
}

XP_STATUS xpSaiVrfIntfTypeGet(xpsInterfaceId_t intfId, int32_t *rifType)
{
    XP_STATUS             status        = XP_NO_ERR;
    xpSaiVrfIntfDbEntry_t *pVrfIntfInfo = NULL;

    if (rifType == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer as an argument, intfId %d\n", intfId);
        return XP_ERR_INVALID_ARG;
    }

    status = xpSaiVrfIntfInfoGet(intfId, &pVrfIntfInfo);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not find VRF interface info, intfId %d\n", intfId);
        return status;
    }

    *rifType = pVrfIntfInfo->rifType;

    return XP_NO_ERR;
}

XP_STATUS xpSaiVrfIntfVrfIdGet(xpsInterfaceId_t intfId, uint32_t* pVrfId)
{
    XP_STATUS           status       = XP_NO_ERR;
    xpSaiVrfIntfDbEntry_t* pVrfIntfInfo = NULL;

    if (pVrfId == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer as an argument, intfId %d\n", intfId);
        return XP_ERR_INVALID_ARG;
    }

    status = xpSaiVrfIntfInfoGet(intfId, &pVrfIntfInfo);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not find VRF interface info, intfId %d\n", intfId);
        return status;
    }

    *pVrfId = pVrfIntfInfo->vrfId;

    return XP_NO_ERR;
}

XP_STATUS xpSaiVrfMaxIdGet(xpDevice_t devId, uint32_t* pMaxVrfId)
{
    XP_STATUS status = XP_NO_ERR;
    status = xpsL3GetMaxVrf(devId, pMaxVrfId);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsL3GetMaxVrf Failed %d \n", status);
        return status;
    }
    return status;
}

/**
 * \brief Walk through the routing interfaces of the particular VRF
 *
 * \param [in] vrfId
 * \param [in] pIntfId
 * \param [out] pNextIntfId
 *
 * \return XP_STATUS
 */
static XP_STATUS xpSaiVrfIntfGetNext(uint32_t vrfId, xpsInterfaceId_t* pIntfId,
                                     xpsInterfaceId_t* pNextIntfId)
{
    XP_STATUS           status       = XP_NO_ERR;
    xpSaiVrfIntfDbEntry_t* pVrfIntfInfo = NULL;
    xpSaiVrfIntfDbEntry_t  vrfIntfKey;

    if (pIntfId)
    {
        vrfIntfKey.keyIntfId = *pIntfId;
        pVrfIntfInfo = &vrfIntfKey;
    }

    for (;;)
    {
        status = xpsStateGetNextData(XP_SCOPE_DEFAULT, vrfIntfDbHandle, pVrfIntfInfo,
                                     (void**)&pVrfIntfInfo);
        if (status != XP_NO_ERR)
        {
            return status;
        }

        if (pVrfIntfInfo == NULL)
        {
            return XP_ERR_NULL_POINTER;
        }

        if (vrfId == pVrfIntfInfo->vrfId)
        {
            *pNextIntfId = pVrfIntfInfo->keyIntfId;
            return XP_NO_ERR;
        }
    }

    return XP_ERR_KEY_NOT_FOUND;
}

/**
 * \brief Apply an updated MAC address
 *
 * \param [in] devId
 * \param [in] vrfId
 * \param [in] macAddress
 *
 * \return XP_STATUS
 */
static XP_STATUS xpSaiVrfMacAddressApply(xpDevice_t devId, uint32_t vrfId,
                                         macAddr_t macAddress)
{
    XP_STATUS      status  = XP_NO_ERR;
    sai_mac_t      mac;

    memcpy(mac, macAddress, 6);

    status = (XP_STATUS)xpsL3AddIngressRouterMac(devId, mac);

    return status;
}

/**
 * \brief Apply an updated VRF admin state to intfs
 *
 * \param [in] devId
 * \param [in] vrfId
 *
 * \return XP_STATUS
 */
static XP_STATUS xpSaiVrfAdminV6StateApply(xpDevice_t devId, uint32_t vrfId)
{
    XP_STATUS         status       = XP_NO_ERR;
    xpsInterfaceId_t* pIntfId      = NULL;
    xpsInterfaceId_t  intfId       = 0;

    while (xpSaiVrfIntfGetNext(vrfId, pIntfId, &intfId) == XP_NO_ERR)
    {
        status = xpSaiRouterInterfaceAdminV6StateApply(devId, intfId);
        if (status != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to apply IPv6 admin state to HW, devId %d, vrfId %d\n",
                           devId, vrfId);
            return status;
        }

        pIntfId = &intfId;
    }

    return XP_NO_ERR;
}

/**
 * \brief Apply an updated VRF admin state to intfs
 *
 * \param [in] devId
 * \param [in] vrfId
 *
 * \return XP_STATUS
 */
static XP_STATUS xpSaiVrfAdminV4StateApply(xpDevice_t devId, uint32_t vrfId)
{
    XP_STATUS         status       = XP_NO_ERR;
    xpsInterfaceId_t* pIntfId      = NULL;
    xpsInterfaceId_t  intfId       = 0;

    while (xpSaiVrfIntfGetNext(vrfId, pIntfId, &intfId) == XP_NO_ERR)
    {
        status = xpSaiRouterInterfaceAdminV4StateApply(devId, intfId);
        if (status != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to apply IPv4 admin state to HW, devId %d, vrfId %d\n",
                           devId, vrfId);
            return status;
        }

        pIntfId = &intfId;
    }

    return XP_NO_ERR;
}

/**
 * \brief Remove VRF info from XPS state
 *
 * \param [in] vrfId
 *
 * \return XP_STATUS
 */
static XP_STATUS xpSaiVrfDestroy(uint32_t vrfId)
{
    XP_STATUS      status  = XP_NO_ERR;
    xpSaiVrfDbEntry_t vrfKey;
    xpSaiVrfDbEntry_t *vrfInfo = NULL;

    vrfKey.keyVrfId = vrfId;

    // Remove the corresponding state
    if ((status = xpsStateDeleteData(XP_SCOPE_DEFAULT, vrfDbHandle, &vrfKey,
                                     (void**)&vrfInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to delete VRF from DB, vrfId %d\n", vrfId);
        return status;
    }

    // Free the memory allocated for the corresponding state
    if ((status = xpsStateHeapFree(vrfInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to free VRF structure, vrfId %d\n", vrfId);
        return status;
    }

    return status;
}

/**
 * \brief Get a pointer to an existing VRF info
 *
 * \param [in] vrfId
 * \param [out] ppVrfInfo
 *
 * \return XP_STATUS
 */
static XP_STATUS xpSaiVrfInfoGet(uint32_t vrfId, xpSaiVrfDbEntry_t **ppVrfInfo)
{
    XP_STATUS      status   = XP_NO_ERR;
    xpSaiVrfDbEntry_t vrfKey;

    vrfKey.keyVrfId = vrfId;

    status = xpsStateSearchData(XP_SCOPE_DEFAULT, vrfDbHandle, &vrfKey,
                                (void**)ppVrfInfo);

    if (*ppVrfInfo == NULL)
    {
        XP_SAI_LOG_DBG("Could not find VRF info, vrfId %d", vrfId);
        return XP_ERR_KEY_NOT_FOUND;
    }

    return status;
}

/**
 * \brief Vrf State Key Compare function
 *
 * This API is used by the state manager as a method to compare
 * keys
 *
 * \param [in] void* key1
 * \param [in] void* key2
 *
 * \return int32_t
 */
static int32_t xpSaiVrfKeyCompare(void *key1, void *key2)
{
    return ((int32_t)(((xpSaiVrfDbEntry_t*)key1)->keyVrfId) - (int32_t)(((
                                                                             xpSaiVrfDbEntry_t*)key2)->keyVrfId));
}

/**
 * \brief API to initialize the VRF info structure
 *
 * \param [in] info
 *
 * \return XP_STATUS
 */
static XP_STATUS xpSaiVrfInfoInit(xpSaiVrfDbEntry_t *info)
{
    memset(info, 0, sizeof(xpSaiVrfDbEntry_t));

    return XP_NO_ERR;
}

/**
 * \brief Vrf State Key Compare function
 *
 * This API is used by the state manager as a method to compare
 * keys
 *
 * \param [in] void* key1
 * \param [in] void* key2
 *
 * \return int32_t
 */
static int32_t xpSaiVrfIntfKeyCompare(void *key1, void *key2)
{
    xpSaiVrfIntfDbEntry_t* pEntry1 = (xpSaiVrfIntfDbEntry_t*)key1;
    xpSaiVrfIntfDbEntry_t* pEntry2 = (xpSaiVrfIntfDbEntry_t*)key2;

    return (((int32_t)pEntry1->keyIntfId) - ((int32_t)pEntry2->keyIntfId));
}

/**
 * \brief API to initialize the VRF interface info structure
 *
 * \param [in] info
 *
 * \return XP_STATUS
 */
static XP_STATUS xpSaiVrfIntfInfoInit(xpSaiVrfIntfDbEntry_t *info)
{
    // Invalidate the VRF id
    info->vrfId = 0;

    // Invalidate the interface id
    info->keyIntfId = 0;

    // Invalidate the V4 admin state
    info->adminV4State = 0;

    // Invalidate the V6 admin state
    info->adminV6State = 0;

    // Invalidate MAC address value
    memset(info->mac, 0, sizeof(macAddr_t));

    return XP_NO_ERR;
}

/**
 * \brief Get a pointer to an existing routing interface info
 *
 * \param [in] vrfId
 * \param [out] ppVrfInfo
 *
 * \return XP_STATUS
 */
XP_STATUS xpSaiVrfIntfInfoGet(xpsInterfaceId_t intfId,
                              xpSaiVrfIntfDbEntry_t **ppVrfIntfInfo)
{
    XP_STATUS           status  = XP_NO_ERR;
    xpSaiVrfIntfDbEntry_t  vrfIntfKey;

    vrfIntfKey.keyIntfId = intfId;

    status = xpsStateSearchData(XP_SCOPE_DEFAULT, vrfIntfDbHandle, &vrfIntfKey,
                                (void**)ppVrfIntfInfo);

    if (*ppVrfIntfInfo == NULL)
    {
        XP_SAI_LOG_DBG("Could not find VRF interface info, intfId %d\n", intfId);
        return XP_ERR_KEY_NOT_FOUND;
    }

    return status;
}

/**
 * \brief API to search a routing interface in the VRF DB
 *
 * \param [in] intfId
 *
 * \return int
 */
static int xpSaiVrfIntfExists(xpsInterfaceId_t intfId)
{
    xpSaiVrfIntfDbEntry_t *pVrfIntfInfo = NULL;
    xpSaiVrfIntfDbEntry_t  vrfIntfKey;

    memset(&vrfIntfKey, 0, sizeof(vrfIntfKey));

    vrfIntfKey.keyIntfId = intfId;

    xpsStateSearchData(XP_SCOPE_DEFAULT, vrfIntfDbHandle, &vrfIntfKey,
                       (void **)&pVrfIntfInfo);

    return (pVrfIntfInfo != NULL);
}

/**
 * \brief API to set a routing interface VRF id
 *
 * \param [in] intfId
 * \param [in] vrfId
 *
 * \return XP_STATUS
 */
static XP_STATUS xpSaiVrfIntfVrfIdSet(xpsInterfaceId_t intfId, uint32_t vrfId)
{
    XP_STATUS           status       = XP_NO_ERR;
    xpSaiVrfIntfDbEntry_t* pVrfIntfInfo = NULL;

    status = xpSaiVrfIntfInfoGet(intfId, &pVrfIntfInfo);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not find VRF interface info, intfId %d\n", intfId);
        return status;
    }

    pVrfIntfInfo->vrfId = vrfId;

    return XP_NO_ERR;
}

/**
 * \brief Re-apply an updated V4 admin state to the interface
 *
 * \param [in] devId
 * \param [in] intfId
 *
 * \return XP_STATUS
 */
static XP_STATUS xpSaiVrfIntfAdminV4ReApply(xpDevice_t devId,
                                            xpsInterfaceId_t intfId)
{
    XP_STATUS  status       = XP_NO_ERR;
    uint32_t   adminV4State = 0;

    status = xpSaiVrfIntfAdminV4StateGet(intfId, &adminV4State);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not get IPv4 routing admin state, devId %d, intfId %d\n",
                       devId, intfId);
        return status;
    }

    status = xpsL3SetIntfIpv4UcRoutingEn(devId, intfId, adminV4State);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not set IPv4 routing admin state, devId %d, intfId %d\n",
                       devId, intfId);
        return status;
    }

    status = xpSaiVrfIntfMcAdminV4StateGet(intfId, &adminV4State);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not get IPv4 multicast routing admin state, devId %d, intfId %d\n",
                       devId, intfId);
        return status;
    }

    status = xpsL3SetIntfIpv4McRoutingEn(devId, intfId, adminV4State);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not set IPv4 multicast routing admin state, devId %d, intfId %d\n",
                       devId, intfId);
        return status;
    }

    return XP_NO_ERR;
}

/**
 * \brief Re-apply an updated V6 admin state to the interface
 *
 * \param [in] devId
 * \param [in] intfId
 *
 * \return XP_STATUS
 */
static XP_STATUS xpSaiVrfIntfAdminV6ReApply(xpDevice_t devId,
                                            xpsInterfaceId_t intfId)
{
    XP_STATUS  status       = XP_NO_ERR;
    uint32_t   adminV6State = 0;

    status = xpSaiVrfIntfAdminV6StateGet(intfId, &adminV6State);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not get IPv6 routing admin state, devId %d, intfId %d\n",
                       devId, intfId);
        return status;
    }

    status = xpsL3SetIntfIpv6UcRoutingEn(devId, intfId, adminV6State);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not set IPv6 routing admin state, devId %d, intfId %d\n",
                       devId, intfId);
        return status;
    }

    status = xpSaiVrfIntfMcAdminV6StateGet(intfId, &adminV6State);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not get IPv6 multicast routing admin state, devId %d, intfId %d\n",
                       devId, intfId);
        return status;
    }

    status = xpsL3SetIntfIpv6McRoutingEn(devId, intfId, adminV6State);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not set IPv6 multicast routing admin state, devId %d, intfId %d\n",
                       devId, intfId);
        return status;
    }

    return XP_NO_ERR;
}

/**
 * \brief Get the number of ip routes associted with the VRF
 *
 * \param [in] devId Device identifier
 * \param [in] vrfId VRF identifier
 * \param [out] pRoutesNum Number of ip routes
 *
 * \return XP_STATUS
 */
static XP_STATUS xpSaiVrfRoutesNumGet(xpDevice_t devId, uint32_t vrfId,
                                      uint32_t* pRoutesNum)
{
    XP_STATUS       status   = XP_NO_ERR;
    xpSaiVrfDbEntry_t* pVrfInfo = NULL;

    status = xpSaiVrfInfoGet(vrfId, &pVrfInfo);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not find VRF, devId %d, vrfId %d\n", devId, vrfId);
        return XP_ERR_INVALID_ID;
    }

    *pRoutesNum = pVrfInfo->routesNum;

    return XP_NO_ERR;
}

/**
 * \brief Get the number of ip hosts associted with the VRF
 *
 * \param [in] devId Device identifier
 * \param [in] vrfId VRF identifier
 * \param [out] pHostsNum Number of ip hosts
 *
 * \return XP_STATUS
 */
static XP_STATUS xpSaiVrfHostsNumGet(xpDevice_t devId, uint32_t vrfId,
                                     uint32_t* pHostsNum)
{
    XP_STATUS       status   = XP_NO_ERR;
    xpSaiVrfDbEntry_t* pVrfInfo = NULL;

    status = xpSaiVrfInfoGet(vrfId, &pVrfInfo);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not find VRF, devId %d, vrfId %d\n", devId, vrfId);
        return XP_ERR_NOT_FOUND;
    }

    *pHostsNum = pVrfInfo->hostsNum;

    return XP_NO_ERR;
}

/**
 * \brief Set the number of ip hosts associted with the VRF
 *
 * \param [in] devId Device identifier
 * \param [in] vrfId VRF identifier
 * \param [in] hostsNum Number of ip hosts
 *
 * \return XP_STATUS
 */
static XP_STATUS xpSaiVrfHostsNumSet(xpDevice_t devId, uint32_t vrfId,
                                     uint32_t hostsNum)
{
    XP_STATUS       status   = XP_NO_ERR;
    xpSaiVrfDbEntry_t* pVrfInfo = NULL;

    status = xpSaiVrfInfoGet(vrfId, &pVrfInfo);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not find VRF, devId %d, vrfId %d\n", devId, vrfId);
        return XP_ERR_NOT_FOUND;
    }

    pVrfInfo->hostsNum = hostsNum;

    return XP_NO_ERR;
}

/**
 * \brief Increment number of routing interfaces associted with the VRF
 *
 * \param [in] devId Device identifier
 * \param [in] vrfId VRF identifier
 *
 * \return XP_STATUS
 */
static XP_STATUS xpSaiVrfRifsNumInc(xpDevice_t devId, uint32_t vrfId)
{
    XP_STATUS       status   = XP_NO_ERR;
    xpSaiVrfDbEntry_t* pVrfInfo = NULL;

    status = xpSaiVrfInfoGet(vrfId, &pVrfInfo);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not find VRF, devId %d, vrfId %d\n", devId, vrfId);
        return XP_ERR_NOT_FOUND;
    }

    pVrfInfo->rifsNum++;

    return XP_NO_ERR;
}

/**
 * \brief Decrement number of routing interfaces associted with the VRF
 *
 * \param [in] devId Device identifier
 * \param [in] vrfId VRF identifier
 *
 * \return XP_STATUS
 */
static XP_STATUS xpSaiVrfRifsNumDec(xpDevice_t devId, uint32_t vrfId)
{
    XP_STATUS       status   = XP_NO_ERR;
    xpSaiVrfDbEntry_t* pVrfInfo = NULL;

    status = xpSaiVrfInfoGet(vrfId, &pVrfInfo);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not find VRF, devId %d, vrfId %d\n", devId, vrfId);
        return XP_ERR_NOT_FOUND;
    }

    pVrfInfo->rifsNum--;

    return XP_NO_ERR;
}

/**
 * \brief Get the number of routing interfaces associted with the VRF
 *
 * \param [in] devId Device identifier
 * \param [in] vrfId VRF identifier
 * \param [out] pRifsNum Number of routing interfaces
 *
 * \return XP_STATUS
 */
static XP_STATUS xpSaiVrfRifsNumGet(xpDevice_t devId, uint32_t vrfId,
                                    uint32_t* pRifsNum)
{
    XP_STATUS       status   = XP_NO_ERR;
    xpSaiVrfDbEntry_t* pVrfInfo = NULL;

    status = xpSaiVrfInfoGet(vrfId, &pVrfInfo);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not find VRF, devId %d, vrfId %d\n", devId, vrfId);
        return XP_ERR_NOT_FOUND;
    }

    *pRifsNum = pVrfInfo->rifsNum;

    return XP_NO_ERR;
}

/**
 * \brief Walk through the VRFs
 *
 * \param [in/out] pVrfInfo
 *
 * \return XP_STATUS
 */
static XP_STATUS xpSaiVrfGetNext(xpSaiVrfDbEntry_t** pVrfInfo)
{
    XP_STATUS       status   = XP_NO_ERR;

    status = xpsStateGetNextData(XP_SCOPE_DEFAULT, vrfDbHandle, *pVrfInfo,
                                 (void**)pVrfInfo);
    if (status != XP_NO_ERR)
    {
        return status;
    }

    if (*pVrfInfo == NULL)
    {
        return XP_ERR_NULL_POINTER;
    }

    return XP_NO_ERR;
}

sai_status_t xpSaiCountVrfObjects(uint32_t *count)
{
    XP_STATUS  retVal  = XP_NO_ERR;

    if (!count)
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpsStateGetCount(XP_SCOPE_DEFAULT, vrfDbHandle, count);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get Count. return Value : %d\n", retVal);
    }
    return xpsStatus2SaiStatus(retVal);
}

sai_status_t xpSaiGetVrfObjectList(uint32_t *object_count,
                                   sai_object_key_t *object_list)
{
    XP_STATUS       retVal      = XP_NO_ERR;
    sai_status_t    saiRetVal   = SAI_STATUS_SUCCESS;
    uint32_t        objCount    = 0;
    xpsDevice_t     devId       = xpSaiGetDevId();

    xpSaiVrfDbEntry_t   *pVrfEntryNext    = NULL;

    saiRetVal = xpSaiCountVrfObjects(&objCount);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get object count!\n");
        return saiRetVal;
    }

    if (objCount > *object_count)
    {
        *object_count = objCount;
        XP_SAI_LOG_ERR("Buffer overflow occured\n");
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    if (object_list == NULL)
    {
        *object_count = objCount;
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    *object_count = objCount;

    for (uint32_t i = 0; i < *object_count; i++)
    {
        retVal = xpsStateGetNextData(XP_SCOPE_DEFAULT, vrfDbHandle, pVrfEntryNext,
                                     (void **)&pVrfEntryNext);
        if (retVal != XP_NO_ERR || pVrfEntryNext == NULL)
        {
            XP_SAI_LOG_ERR("Failed to retrieve Vrf object, error %d\n", retVal);
            return SAI_STATUS_FAILURE;
        }

        saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_VIRTUAL_ROUTER, devId,
                                     (sai_uint64_t)pVrfEntryNext->keyVrfId, &object_list[i].key.object_id);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("SAI objectId could not be created, error %d\n", saiRetVal);
            return saiRetVal;
        }
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiCountVrfIntfObjects(uint32_t *count)
{
    XP_STATUS  retVal  = XP_NO_ERR;

    if (!count)
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpsStateGetCount(XP_SCOPE_DEFAULT, vrfIntfDbHandle, count);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get Count. return Value : %d\n", retVal);
    }
    return xpsStatus2SaiStatus(retVal);
}

sai_status_t xpSaiGetVrfIntfObjectList(uint32_t *object_count,
                                       sai_object_key_t *object_list)
{
    XP_STATUS       retVal      = XP_NO_ERR;
    sai_status_t    saiRetVal   = SAI_STATUS_SUCCESS;
    uint32_t        objCount    = 0;
    xpsDevice_t     devId       = xpSaiGetDevId();

    xpSaiVrfIntfDbEntry_t *pVrfIntfNext = NULL;

    saiRetVal = xpSaiCountVrfIntfObjects(&objCount);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get object count!\n");
        return saiRetVal;
    }

    if (objCount > *object_count)
    {
        *object_count = objCount;
        XP_SAI_LOG_ERR("Buffer overflow occured\n");
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    if (object_list == NULL)
    {
        *object_count = objCount;
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    *object_count = objCount;

    for (uint32_t i = 0; i < *object_count; i++)
    {
        retVal = xpsStateGetNextData(XP_SCOPE_DEFAULT, vrfIntfDbHandle, pVrfIntfNext,
                                     (void **)&pVrfIntfNext);
        if (retVal != XP_NO_ERR || pVrfIntfNext == NULL)
        {
            XP_SAI_LOG_ERR("Failed to retrieve VrfIntf object, error %d\n", retVal);
            return SAI_STATUS_FAILURE;
        }

        saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_ROUTER_INTERFACE, devId,
                                     (sai_uint64_t)pVrfIntfNext->keyIntfId, &object_list[i].key.object_id);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("SAI objectId could not be created, error %d\n", saiRetVal);
            return saiRetVal;
        }
    }

    return SAI_STATUS_SUCCESS;
}
/* CPSS expects all Virtual Routers to be deleted before removing
   LPM DB in switch shutdown flow. */
sai_status_t xpSaiVrfFlushAll(xpDevice_t devId)
{
    XP_STATUS           status   = XP_NO_ERR;
    xpSaiVrfDbEntry_t*  pVrf     = NULL;

    while (xpSaiVrfGetNext(&pVrf) == XP_NO_ERR)
    {
        status = xpsL3VrfRemove(devId, pVrf->keyVrfId, pVrf->xpsVrfdefUcNhId,
                                pVrf->xpsVrfdefMcNhId);
        if (status != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpsL3VrfRemove Failed dev %d, vrfId %d status %d\n",
                           devId,  pVrf->keyVrfId, status);
            return xpsStatus2SaiStatus(status);
        }
    }
    return SAI_STATUS_SUCCESS;
}

#ifdef __cplusplus
}
#endif

