// xpsVxlan.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpsVxlan.h"
#include "xpsInternal.h"
#include "xpsScope.h"
#include "xpsLock.h"
#include "xpsTunnel.h"
#include "xpsAcl.h"
#include "xpsPort.h"
#include "xpsAllocator.h"
#include "openXpsLag.h"
#include "cpssHalUtil.h"
#include "cpssHalL3.h"
#include "cpssHalTunnel.h"
#include "cpssHalTcam.h"
#include "cpssHalVlan.h"
#include "cpssHalAcl.h"
#include "cpssHalMulticast.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

uint32_t vxlanIpv6AclTableId_g = 0;


static xpsDbHandle_t vxlanDbHndl = XPS_STATE_INVALID_DB_HANDLE;

extern uint32_t curMaxTnlTermEntries;
//uint32_t curMaxVxlanUniTermEntries = XPS_VXLAN_UNI_HW_MAX_IDS;

typedef struct xpsVxlanVniInfo_t
{
    uint32_t vniId;
    uint32_t refCnt;
} xpsVxlanVniInfo_t;

static int32_t vxlanKeyCompare(void* key1, void* key2)
{
    return (*(int32_t*)(key2) - *(int32_t*)(key1));
}

static xpsDbHandle_t vniGblDbHandle = XPS_STATE_INVALID_DB_HANDLE;

static int32_t xpsVniDbKeyComp(void* key1, void* key2)
{
    return (((xpsVniDbEntry_t *)key1)->vniId - ((xpsVniDbEntry_t *)key2)->vniId);
}

static xpsDbHandle_t vniMapGblDbHandle = XPS_STATE_INVALID_DB_HANDLE;
static int32_t xpsVniMapDbKeyComp(void* _key1, void* _key2)
{
    xpsVxlanVniMapDbEntry_t *key1 = (xpsVxlanVniMapDbEntry_t *)_key1;
    xpsVxlanVniMapDbEntry_t *key2 = (xpsVxlanVniMapDbEntry_t *)_key2;

    if (key1->vniId > key2->vniId)
    {
        return 1;
    }
    else if (key1->vniId < key2->vniId)
    {
        return -1;
    }

    if (key1->mapType > key2->mapType)
    {
        return 1;
    }
    else if (key1->mapType < key2->mapType)
    {
        return -1;
    }

    if (key1->value > key2->value)
    {
        return 1;
    }
    else if (key1->value < key2->value)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

static XP_STATUS xpsVniInsertDbEntry(xpsScope_t scopeId, uint32_t vniId,
                                     xpsVniDbEntry_t **vniDbEntry)
{
    XP_STATUS status = XP_NO_ERR;

    status = xpsStateHeapMalloc(sizeof(xpsVniDbEntry_t), (void **)vniDbEntry);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Allocation failed for VNI Db entry\n");
        return status;
    }

    memset(*vniDbEntry, 0, sizeof(xpsVniDbEntry_t));

    (*vniDbEntry)->vniId = vniId;

    status = xpsStateInsertData(scopeId, vniGblDbHandle, (void *)*vniDbEntry);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to insert VNI DB entry\n");
        return status;
    }

    return XP_NO_ERR;
}

XP_STATUS xpsVniGetDbEntry(xpsScope_t scopeId, uint32_t vniId,
                           xpsVniDbEntry_t **vniDbEntry)
{
    XP_STATUS status = XP_NO_ERR;
    xpsVniDbEntry_t keyDbEntry;

    memset(&keyDbEntry, 0, sizeof(keyDbEntry));

    *vniDbEntry = NULL;
    keyDbEntry.vniId = vniId;
    status = xpsStateSearchData(scopeId, vniGblDbHandle, (xpsDbKey_t)&keyDbEntry,
                                (void **)vniDbEntry);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to insert VNI Db Entry\n");
        return status;
    }

    if (!(*vniDbEntry))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
              "Failed to Get VNI Db Entry\n");
        return XP_ERR_NOT_FOUND;
    }

    return XP_NO_ERR;
}

static XP_STATUS xpsVniDeleteDbEntry(xpsScope_t scopeId, uint32_t vniId)
{
    XP_STATUS status = XP_NO_ERR;
    xpsVniDbEntry_t keyVniDbEntry;
    xpsVniDbEntry_t *vniDbEntry = NULL;

    memset(&keyVniDbEntry, 0, sizeof(keyVniDbEntry));

    keyVniDbEntry.vniId = vniId;
    status = xpsStateDeleteData(scopeId, vniGblDbHandle, (xpsDbKey_t)&keyVniDbEntry,
                                (void **)&vniDbEntry);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to delete VNI DB entry\n");
        return status;
    }

    status = xpsStateHeapFree((void *)vniDbEntry);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to free VNI DB entry\n");
        return status;
    }

    return XP_NO_ERR;
}

static XP_STATUS xpsVniMapInsertDbEntry(xpsScope_t scopeId, uint32_t vniId,
                                        xpsVxlanMapType_e mapType,
                                        uint16_t value,
                                        xpsVxlanVniMapDbEntry_t **vniMapDbEntry)
{
    XP_STATUS status = XP_NO_ERR;

    status = xpsStateHeapMalloc(sizeof(xpsVxlanVniMapDbEntry_t),
                                (void **)vniMapDbEntry);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Allocation failed for VNI Map Db entry\n");
        return status;
    }

    memset(*vniMapDbEntry, 0, sizeof(xpsVxlanVniMapDbEntry_t));

    (*vniMapDbEntry)->vniId = vniId;
    (*vniMapDbEntry)->mapType = mapType;
    (*vniMapDbEntry)->value = value;

    status = xpsStateInsertData(scopeId, vniMapGblDbHandle, (void *)*vniMapDbEntry);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to insert VNI Map DB entry\n");
        return status;
    }
    return XP_NO_ERR;
}

static XP_STATUS xpsVniMapDeleteDbEntry(xpsScope_t scopeId, uint32_t vniId,
                                        xpsVxlanMapType_e mapType,
                                        uint16_t value)
{
    XP_STATUS status = XP_NO_ERR;
    xpsVxlanVniMapDbEntry_t keyVniMapDbEntry;
    xpsVxlanVniMapDbEntry_t *vniMapDbEntry = NULL;

    memset(&keyVniMapDbEntry, 0, sizeof(keyVniMapDbEntry));

    keyVniMapDbEntry.vniId = vniId;
    keyVniMapDbEntry.mapType = mapType;
    keyVniMapDbEntry.value = value;
    status = xpsStateDeleteData(scopeId, vniMapGblDbHandle,
                                (xpsDbKey_t)&keyVniMapDbEntry,
                                (void **)&vniMapDbEntry);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to delete VNI Map DB entry\n");
        return status;
    }

    status = xpsStateHeapFree((void *)vniMapDbEntry);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to free VNI Map DB entry\n");
        return status;
    }

    return XP_NO_ERR;
}

XP_STATUS xpsVniMapGetDbEntry(xpsScope_t scopeId, uint32_t vniId,
                              xpsVxlanMapType_e mapType,
                              uint16_t value,
                              xpsVxlanVniMapDbEntry_t **vniMapDbEntry)
{
    XP_STATUS status = XP_NO_ERR;
    xpsVxlanVniMapDbEntry_t keyDbEntry;

    memset(&keyDbEntry, 0, sizeof(keyDbEntry));

    *vniMapDbEntry = NULL;
    keyDbEntry.vniId = vniId;
    keyDbEntry.mapType = mapType;
    keyDbEntry.value = value;
    status = xpsStateSearchData(scopeId, vniMapGblDbHandle, (xpsDbKey_t)&keyDbEntry,
                                (void **)vniMapDbEntry);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to insert VNI Db Entry\n");
        return status;
    }

    if (!(*vniMapDbEntry))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
              "Failed to Get VNI Db Entry\n");
        return XP_ERR_NOT_FOUND;
    }

    return XP_NO_ERR;
}

XP_STATUS xpsVxlanInit(void)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS retVal = XP_NO_ERR;

    retVal = xpsVxlanInitScope(XP_SCOPE_DEFAULT);

    XPS_FUNC_EXIT_LOG();

    return retVal;
}

XP_STATUS xpsVxlanInitScope(xpsScope_t scopeId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS retVal = XP_NO_ERR;
    GT_STATUS cpssStatus = GT_OK;
    xpsDevice_t devId = 0;
    GT_U32    ttiClientGrpId = 0xffffffff;

    retVal = xpsAllocatorInitIdAllocator(scopeId,
                                         XP_ALLOC_GLOBAL_EPORT,
                                         XPS_GLOBAL_EPORT_MAX_NUM,
                                         XPS_GLOBAL_EPORT_RANGE_START);
    if (retVal != XP_NO_ERR)
    {
        XPS_FUNC_EXIT_LOG();
        return retVal;
    }

    retVal = xpsAllocatorInitIdAllocator(scopeId,
                                         XP_ALLOC_L2_ECMP_EPORT,
                                         XPS_L2_ECMP_EPORT_MAX_NUM,
                                         XPS_L2_ECMP_EPORT_RANGE_START);
    if (retVal != XP_NO_ERR)
    {
        XPS_FUNC_EXIT_LOG();
        return retVal;
    }

    retVal = xpsAllocatorInitIdAllocator(scopeId,
                                         XP_ALLOC_LTT_ECMP,
                                         XPS_LTT_ECMP_MAX_NUM,
                                         XPS_LTT_ECMP_RANGE_START);
    if (retVal != XP_NO_ERR)
    {
        XPS_FUNC_EXIT_LOG();
        return retVal;
    }
    retVal = xpsAllocatorInitIdAllocator(scopeId,
                                         XP_ALLOC_VXLAN_IPV6_ACL,
                                         1024, 0);
    if (retVal != XP_NO_ERR)
    {
        XPS_FUNC_EXIT_LOG();
        return retVal;
    }

    vxlanDbHndl = XPS_VXLAN_DB_HNDL;
    if ((retVal = xpsStateRegisterDb(scopeId, "Vxlan Db", XPS_GLOBAL,
                                     &vxlanKeyCompare, vxlanDbHndl)) != XP_NO_ERR)
    {
        vxlanDbHndl = XPS_STATE_INVALID_DB_HANDLE;
        return retVal;
    }

    vniGblDbHandle = XPS_VXLAN_VNI_DB_HNDL;
    retVal = xpsStateRegisterDb(scopeId, "Vxlan VNI", XPS_GLOBAL,
                                xpsVniDbKeyComp, vniGblDbHandle);
    if (retVal != XP_NO_ERR)
    {
        vniGblDbHandle = XPS_STATE_INVALID_DB_HANDLE;
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Failed to register VNI DB\n");
        return retVal;
    }

    vniMapGblDbHandle = XPS_VXLAN_VNI_MAP_DB_HNDL;
    retVal = xpsStateRegisterDb(scopeId, "Vxlan VNI MAP", XPS_GLOBAL,
                                xpsVniMapDbKeyComp, vniMapGblDbHandle);
    if (retVal != XP_NO_ERR)
    {
        vniMapGblDbHandle = XPS_STATE_INVALID_DB_HANDLE;
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Failed to register VNI DB\n");
        return retVal;
    }
#if 0
    retVal = xpsAllocatorInitIdAllocator(scopeId, XP_ALLOC_VXLAN_UNI_TTI_HW_ENTRY,
                                         _4K,
                                         XPS_VXLAN_UNI_TERM_HW_RANGE_START);
    if (retVal != XP_NO_ERR)
    {
        vniGblDbHandle = XPS_STATE_INVALID_DB_HANDLE;
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Failed to register VNI DB\n");
        return retVal;
    }
#endif
    /* Configure Global Eport Range */

    cpssStatus = cpssHalGlobalEportRangeUpdate(devId, CPSS_HAL_EPORT_TYPE_GLOBAL,
                                               XPS_GLOBAL_EPORT_RANGE_START,
                                               (XPS_GLOBAL_EPORT_RANGE_START+XPS_GLOBAL_EPORT_MAX_NUM) - 1);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "failed to set Global Eport range : %d  ", cpssStatus);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }

    cpssStatus = cpssHalGlobalEportRangeUpdate(devId, CPSS_HAL_EPORT_TYPE_L2ECMP,
                                               XPS_L2_ECMP_EPORT_RANGE_START,
                                               (XPS_L2_ECMP_EPORT_RANGE_START+XPS_L2_ECMP_EPORT_MAX_NUM) - 1);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "failed to set L2ECMP Eport range : %d  ", cpssStatus);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }

    cpssStatus = cpssHalTcamAclClientGroupIdGet(devId, CPSS_DXCH_TCAM_TTI_E,
                                                &ttiClientGrpId);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "failed to TTI groupId : %d  ", cpssStatus);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }
    /*TODO: For Falcon and Aldrin2, separate VTcam for UNI TTI rules works as expected.
            whereas for M0 devices, it was failing in Egress ACL cases.
            For now, use same VTCam used by Tunnel Termination
    */

#if 0
    /*
     * Create VTCam ID (UNI_TERM_TABLE_ID) for assigning pkt eVlan forn UNI interface.
     * This TCAM has 30B rule size.
     */

    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC  vTcamInfo;
    memset(&vTcamInfo, 0, sizeof(vTcamInfo));
    vTcamInfo.clientGroup = ttiClientGrpId; //TODO update the mapping
    vTcamInfo.hitNumber   = 0;
    vTcamInfo.autoResize  = GT_FALSE;

    /*
     * Minimum number of rules for this TCAM. Logical index TCAM
     * cannot re-size itself. Setting vale of 1024.
     * Revisit this number, if necessary.
     */
    vTcamInfo.guaranteedNumOfRules = XPS_VXLAN_UNI_HW_MAX_IDS;
    vTcamInfo.ruleAdditionMethod   =
        CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_LOGICAL_INDEX_E;
    vTcamInfo.ruleSize             = CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E;
    /*Create vTcam*/
    if ((cpssStatus = cpssHalVtcamCreate(XPS_GLOBAL_TACM_MGR,
                                         UNI_TERM_TABLE_ID, &vTcamInfo)) != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " cpssHalVtcamCreate failed with err : %d\n ", cpssStatus);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }
#endif
    cpssStatus = cpssHalTunnelVxlanInit(devId);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed vxlan init %d\n", cpssStatus);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }

    cpssStatus = cpssHalTunnelVxlanIPv4TTIKeySet(devId);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " cpssHalTunnelVxlanIPv4TTIKeySet with err : %d\n for Device :%d ",
              cpssStatus, devId);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }

    cpssStatus = cpssHalTunnelVxlanIPv6TTIKeySet(devId);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " cpssHalTunnelVxlanIPv6TTIKeySet with err : %d\n for Device :%d ",
              cpssStatus, devId);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }

    xpAclTableInfo_t tableInfo;
    memset(&tableInfo, 0x00, sizeof(xpAclTableInfo_t));

    tableInfo.tableType = XPS_ACL_TABLE_TYPE_IPV6;
    retVal = xpsAclCreateTable(devId, tableInfo, &vxlanIpv6AclTableId_g);
    if (retVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: Failed to create table for the device %u \n", devId);
        return retVal;
    }

    cpssStatus = cpssHalTunnelEnableMeshIdFiltering(devId,
                                                    GT_TRUE);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " cpssHalTunnelEnableMeshIdFiltering with err : %d\n for Device :%d ",
              cpssStatus, devId);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }
    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVxlanDeInit(void)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS retVal = XP_NO_ERR;

    retVal = xpsVxlanDeInitScope(XP_SCOPE_DEFAULT);

    XPS_FUNC_EXIT_LOG();

    return retVal;
}

XP_STATUS xpsVxlanDeInitScope(xpsScope_t scopeId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS retVal = XP_NO_ERR;

    if (vniMapGblDbHandle != XPS_STATE_INVALID_DB_HANDLE)
    {
        if ((retVal = xpsStateDeRegisterDb(scopeId, &vniMapGblDbHandle)) != XP_NO_ERR)
        {
            XPS_FUNC_EXIT_LOG();
            return retVal;
        }
    }
    if (vniGblDbHandle != XPS_STATE_INVALID_DB_HANDLE)
    {
        if ((retVal = xpsStateDeRegisterDb(scopeId, &vniGblDbHandle)) != XP_NO_ERR)
        {
            XPS_FUNC_EXIT_LOG();
            return retVal;
        }
    }
    if (vxlanDbHndl != XPS_STATE_INVALID_DB_HANDLE)
    {
        if ((retVal = xpsStateDeRegisterDb(scopeId, &vxlanDbHndl)) != XP_NO_ERR)
        {
            XPS_FUNC_EXIT_LOG();
            return retVal;
        }
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVxlanCreateTunnelInterface(inetAddr_t *lclEpIpAddr,
                                        inetAddr_t *rmtEpIpAddr,
                                        xpsInterfaceId_t *tnlIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    return xpsVxlanCreateTunnelInterfaceScope(XP_SCOPE_DEFAULT, lclEpIpAddr,
                                              rmtEpIpAddr, tnlIntfId);

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}
XP_STATUS xpsVxlanCreateTunnelInterfaceScope(xpsScope_t scopeId,
                                             inetAddr_t *lclEpIpAddr,
                                             inetAddr_t *rmtEpIpAddr,
                                             xpsInterfaceId_t *tnlIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsVxlanCreateTunnelInterfaceScope);

    XP_STATUS rt = XP_NO_ERR;
    xpsIpTunnelData_t ipData;
    xpsVxlanDbEntry_t *dbEntry = NULL;
    xpsRBTree_t *rbtree = NULL;

    memset(&ipData, 0, sizeof(xpsIpTunnelData_t));
    ipData.type = XP_IP_VXLAN_TUNNEL;

    ipData.lclEpIpAddr = *lclEpIpAddr;
    ipData.rmtEpIpAddr = *rmtEpIpAddr;

    rt = xpsIpTunnelCreate(scopeId, XPS_TUNNEL_VXLAN, &ipData, tnlIntfId);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Ip tunnel Creation failed");
        return rt;
    }

    if ((rt = xpsStateHeapMalloc(sizeof(xpsVxlanDbEntry_t),
                                 (void**)&dbEntry)) != XP_NO_ERR)
    {
        XPS_FUNC_EXIT_LOG();
        return rt;
    }

    if (rt != XP_NO_ERR)
    {
        XPS_FUNC_EXIT_LOG();
        return rt;
    }
    memset(dbEntry, 0, sizeof(xpsVxlanDbEntry_t));

    dbEntry->tnlIntfId = *tnlIntfId;

    //Reuse the vxlanKeyCompare compare function as data-type is same
    if ((rt = xpsRBTInit(&rbtree, &vxlanKeyCompare, XPS_VERSION)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Global DB initialization failed");
        xpsStateHeapFree((void*)dbEntry);
        return rt;
    }

    if (!rbtree)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Global DB, out of memory");
        xpsStateHeapFree((void*)dbEntry);
        return XP_ERR_OUT_OF_MEM;
    }
    dbEntry->vniList = rbtree;

    if ((rt = xpsStateInsertData(scopeId, vxlanDbHndl,
                                 (void*)dbEntry)) != XP_NO_ERR)
    {
        xpsRBTDelete(dbEntry->vniList);
        xpsStateHeapFree((void*)dbEntry);
        XPS_FUNC_EXIT_LOG();
        return rt;
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVxlanGetTunnelDb(xpsScope_t scopeId,
                              xpsInterfaceId_t tnlIntfId,
                              xpsVxlanDbEntry_t **info)
{
    XP_STATUS result = XP_NO_ERR;
    xpsVxlanDbEntry_t keyVxlanCtx;

    memset(&keyVxlanCtx, 0x0, sizeof(xpsVxlanDbEntry_t));
    keyVxlanCtx.tnlIntfId = tnlIntfId;
    if ((result = xpsStateSearchData(scopeId, vxlanDbHndl,
                                     (xpsDbKey_t)&keyVxlanCtx, (void**)info)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Search data failed, tnlIntfId(%d)", tnlIntfId);
        return result;
    }

    if (!*info)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Vxlan does not exist");
        return XP_ERR_KEY_NOT_FOUND;
    }

    return result;
}

static XP_STATUS xpsTunnelGetPortIntfList(xpsDevice_t devId,
                                          xpsInterfaceId_t tnlIntfId,
                                          xpsLagPortIntfList_t *portIntfList)
{
    XP_STATUS rt = XP_NO_ERR;
    xpsIpTnlGblDbEntry_t                *ipTnlGblDbEntry = NULL;
    xpsInterfaceType_e portIntfType;
    xpsInterfaceId_t portInterfaceId;
    xpsScope_t scopeId = 0;

    if (!portIntfList)
    {
        return XP_ERR_NULL_POINTER;
    }

    memset(portIntfList, 0, sizeof(xpsLagPortIntfList_t));

    /* Get Scope Id from devId */
    if ((rt = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return rt;
    }

    rt = xpsIpTunnelGblGetDbEntry(scopeId, tnlIntfId, &ipTnlGblDbEntry);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "TunnelGet failed : %d ",
              tnlIntfId);
        return rt;
    }
    portIntfList->size = 0;
    for (uint32_t i = 0 ; i < ipTnlGblDbEntry->numOfEports; i++)
    {
        portInterfaceId = ipTnlGblDbEntry->ePorts[i].intfId;
        rt = xpsInterfaceGetType(portInterfaceId, &portIntfType);
        if (rt != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to fetch interface type from interface id %d\n", portInterfaceId);
            return rt;
        }

        if (portIntfType == XPS_PORT)
        {
            portIntfList->portIntf[portIntfList->size] = portInterfaceId;
            (portIntfList->size)++;
        }
        else if (portIntfType == XPS_LAG)
        {
            if ((rt = xpsLagGetPortIntfList(portInterfaceId, portIntfList)) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Get ports failed, lag interface(%d)", portInterfaceId);
                return rt;
            }
        }
    }

    return rt;
}

XP_STATUS xpsVxlanClearTunnelEportConfig(xpsDevice_t devId, uint32_t ePort)
{
    GT_STATUS cpssStatus = GT_OK;
    cpssStatus = cpssHalTunnelEPorTSrcMacClear(devId, ePort);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to unbind tunnel to ePort %d\n", cpssStatus);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }
    cpssStatus = cpssHalTunnelToEPortMapClear(devId, ePort);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to unbind tunnel to ePort %d\n", cpssStatus);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }

    cpssStatus = cpssHalTunnelEPortToPhyPortMapClear(devId, ePort);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to unbind ePort to physical port %d\n", cpssStatus);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }

    cpssStatus = cpssHalVlanPortTranslationEnableSet(devId, ePort,
                                                     CPSS_DIRECTION_EGRESS_E,
                                                     CPSS_DXCH_BRG_VLAN_TRANSLATION_DISABLE_E);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssHalVlanPortTranslationEnableSet failed port (%d)err: %d \n ",
              ePort, cpssStatus);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }
    return XP_NO_ERR;
}

XP_STATUS xpsVxlanSetTunnelEportConfig(xpsDevice_t devId,
                                       uint32_t tnlStartId,
                                       uint32_t ePort,
                                       xpsInterfaceInfo_t *l3IntfInfo,
                                       xpsInterfaceId_t mtuPortIntf,
                                       xpsInterfaceId_t intfId)
{
    XP_STATUS rt = XP_NO_ERR;
    GT_STATUS cpssStatus = GT_OK;
    xpsScope_t scopeId;
    CPSS_INTERFACE_TYPE_ENT portType = CPSS_INTERFACE_PORT_E;
    uint32_t macSaTableIndex = 0;
    GT_ETHERADDR cpssMacAddr;
    uint32_t portNum = 0;
    xpsInterfaceType_e portIntfType = XPS_INVALID_IF_TYPE;

    memset(&cpssMacAddr, 0, sizeof(GT_ETHERADDR));
    rt = xpsScopeGetScopeId(devId, &scopeId);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: Failed to get scope Id for device %d\n", devId);
        return rt;
    }

    rt = xpsInterfaceGetType(intfId, &portIntfType);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to fetch interface type from interface id %d\n", intfId);
        return rt;
    }

    if (XPS_LAG == portIntfType)
    {
        portType = CPSS_INTERFACE_TRUNK_E;
        portNum = xpsUtilXpstoCpssInterfaceConvert(intfId, XPS_LAG);
    }
    else if (XPS_PORT == portIntfType)
    {
        portType = CPSS_INTERFACE_PORT_E;
        portNum = intfId;
    }
    else
    {
        return XP_ERR_INVALID_PARAMS;
    }
    cpssStatus = cpssHalTunnelToEPortMapSet(devId, ePort,
                                            tnlStartId);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to bind tunnel to ePort %d\n", cpssStatus);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }

    cpssStatus = cpssHalTunnelEPortToPhyPortMapSet(devId, ePort,
                                                   portType, portNum);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to bind ePort to physical port %d\n", cpssStatus);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }

    /*There is no tunnel MTU SAI attribute. For now set it to Port MTU.
      Assumption here is that all members of LAG/VLAN have same MTU.
      Take one of the member MTU for eport */
    cpssStatus = cpssHalTunnelEPorTMtuProfileSet(devId, mtuPortIntf, ePort);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed cpssHalTunnelEPorTMtuProfileSet %d\n", cpssStatus);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }

    // Set the SA MAC address to the Global MAC SA table
    memcpy(cpssMacAddr.arEther, l3IntfInfo->egressSAMac,
           sizeof(cpssMacAddr.arEther));
    cpssStatus = cpssHalL3GetMacSaTableIndex(&cpssMacAddr, &macSaTableIndex);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get MAC SA table index to RIF id %d\n", l3IntfInfo->keyIntfId);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }

    cpssStatus = cpssHalTunnelEPorTSrcMacSet(devId, macSaTableIndex, ePort);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed cpssHalTunnelEPorTSrcMacSet %d\n", cpssStatus);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }

    cpssStatus = cpssHalVlanPortTranslationEnableSet(devId, ePort,
                                                     CPSS_DIRECTION_EGRESS_E,
                                                     CPSS_DXCH_BRG_VLAN_TRANSLATION_VID0_E);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssHalVlanPortTranslationEnableSet failed port (%d)err: %d \n ",
              ePort, cpssStatus);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }
    return rt;
}

XP_STATUS xpsVxlanAddTunnelEntry(xpsDevice_t devId, xpsInterfaceId_t tnlIntfId,
                                 xpsInterfaceId_t underlayL3IntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsVxlanAddTunnelEntry);

    XP_STATUS rt = XP_NO_ERR;
    GT_STATUS cpssStatus = GT_OK;
    xpsVxlanDbEntry_t *lookupEntry = NULL;
    xpsIpTnlGblDbEntry_t *ipTnlGblDbEntry = NULL;
    xpsScope_t scopeId;
    xpsInterfaceType_e intfType;
    xpsLagPortIntfList_t lagPortList;
    memset(&lagPortList, 0, sizeof(lagPortList));
    uint32_t prevEportIdx;
    uint32_t ePort = 0;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        return XP_ERR_INVALID_DEV_ID;
    }

    rt = xpsScopeGetScopeId(devId, &scopeId);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: Failed to get scope Id for device %d\n", devId);
        return rt;
    }

    rt = xpsIpTunnelGblGetDbEntry(scopeId, tnlIntfId, &ipTnlGblDbEntry);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "TunnelGet failed : %d ",
              tnlIntfId);
        return rt;
    }

    prevEportIdx = ipTnlGblDbEntry->numOfEports;

    rt = xpsVxlanGetTunnelDb(scopeId, tnlIntfId, &lookupEntry);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Vxlan Tunnel Not found :%d \n", tnlIntfId);
        return rt;
    }
    xpsInterfaceInfo_t *l3IntfInfo = NULL;
    rt = xpsInterfaceGetInfoScope(scopeId, underlayL3IntfId, &l3IntfInfo);
    if (rt != XP_NO_ERR || l3IntfInfo == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to fetch interface type from l3 interface id %d\n", underlayL3IntfId);
        return rt;
    }

    if (l3IntfInfo)
    {
        uint32_t tunnelStartId = 0;
        intfType = l3IntfInfo->type;

        if ((rt = xpsAllocateTunnelStartIndex(scopeId,
                                              ipTnlGblDbEntry->tnlData.lclEpIpAddr.type,
                                              &tunnelStartId)) != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " Tunnel Start Pointer allocation is failed");
            return rt;
        }
        /* For Tunnel over SVI, ePort must be allocated per vlan member.
           In case of port or LAG RIF, ePort to Phy mapping is direct,
           for SVI, it is not. */
        if (intfType == XPS_VLAN_ROUTER)
        {
            uint16_t vlanId = XPS_INTF_MAP_INTF_TO_BD(underlayL3IntfId);
            uint16_t numOfIntfs = 0;
            xpsInterfaceId_t *intfList = NULL;

            rt = xpsVlanGetInterfaceList(devId, vlanId, &intfList, &numOfIntfs);
            if (rt != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "failed to get interface list of vlan :%d error:%d \n", vlanId, rt);
                return rt;
            }

            for (uint32_t i = 0; i < numOfIntfs; i++)
            {
                ePort = 0;
                rt = xpsAllocatorAllocateId(scopeId, XP_ALLOC_GLOBAL_EPORT, &ePort);
                if (rt != XP_NO_ERR)
                {
                    XPS_FUNC_EXIT_LOG();
                    return rt;
                }
                ipTnlGblDbEntry->ePorts[ipTnlGblDbEntry->numOfEports].ePort = ePort;
                ipTnlGblDbEntry->ePorts[ipTnlGblDbEntry->numOfEports].l3IntfId =
                    underlayL3IntfId;
                ipTnlGblDbEntry->ePorts[ipTnlGblDbEntry->numOfEports].tnlStartId =
                    tunnelStartId;
                ipTnlGblDbEntry->ePorts[ipTnlGblDbEntry->numOfEports].intfId = intfList[i];
                ipTnlGblDbEntry->numOfEports++;
            }
        }
        else
        {
            rt = xpsAllocatorAllocateId(scopeId, XP_ALLOC_GLOBAL_EPORT, &ePort);
            if (rt != XP_NO_ERR)
            {
                xpsStateHeapFree((void*)ipTnlGblDbEntry);
                XPS_FUNC_EXIT_LOG();
                return rt;
            }
            rt = xpsL3RetrieveInterfaceMapping(scopeId, underlayL3IntfId,
                                               &(ipTnlGblDbEntry->ePorts[ipTnlGblDbEntry->numOfEports].intfId));
            if (rt != XP_NO_ERR)
            {
                xpsStateHeapFree((void*)ipTnlGblDbEntry);
                XPS_FUNC_EXIT_LOG();
                return rt;
            }
            ipTnlGblDbEntry->ePorts[ipTnlGblDbEntry->numOfEports].ePort = ePort;
            ipTnlGblDbEntry->ePorts[ipTnlGblDbEntry->numOfEports].l3IntfId =
                underlayL3IntfId;
            ipTnlGblDbEntry->ePorts[ipTnlGblDbEntry->numOfEports].tnlStartId =
                tunnelStartId;
            ipTnlGblDbEntry->numOfEports++;
        }
    }

    /* Select the first member for ePort's MTU configuration.*/
    xpsInterfaceType_e portfType;
    uint32_t mtuPortIntf = ipTnlGblDbEntry->ePorts[prevEportIdx].intfId;

    //Get interfacetype
    rt = xpsInterfaceGetTypeScope(devId, mtuPortIntf, &portfType);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Interface Get type for interface(%d) failed", mtuPortIntf);
        return rt;
    }

    if (portfType == XPS_LAG)
    {
        if ((rt = xpsLagGetPortIntfList(mtuPortIntf, &lagPortList)) != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Get ports failed, lag interface(%d)", mtuPortIntf);
            return rt;
        }
        if (lagPortList.size)
        {
            mtuPortIntf = lagPortList.portIntf[0];
        }
    }

    for (uint32_t i = prevEportIdx ; i < ipTnlGblDbEntry->numOfEports; i++)
    {
        rt = xpsVxlanSetTunnelEportConfig(devId,
                                          ipTnlGblDbEntry->ePorts[i].tnlStartId,
                                          ipTnlGblDbEntry->ePorts[i].ePort,
                                          l3IntfInfo,
                                          mtuPortIntf,
                                          ipTnlGblDbEntry->ePorts[i].intfId);
        if (rt != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " Eport Set Failed  %d\n", ipTnlGblDbEntry->ePorts[i].intfId);
            return rt;
        }
    }

    /* Set tunnel primaryEport MTU profile to the first ePort in the list. */
    cpssStatus = cpssHalTunnelEPorTMtuProfileSet(devId,
                                                 mtuPortIntf,
                                                 ipTnlGblDbEntry->primaryEport);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed cpssHalTunnelEPorTMtuProfileSet %d\n", cpssStatus);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }


    rt = xpsIntfChildAdd(l3IntfInfo, tnlIntfId, XPS_TUNNEL_VXLAN,
                         tnlIntfId/*lookupEntry->ePort*/);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to add tnl intf %d to L3 Intf %d\n", tnlIntfId,
              underlayL3IntfId);
        return rt;
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVxlanRemoveTunnelEntry(xpsDevice_t devId,
                                    xpsInterfaceId_t tnlIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS rt = XP_NO_ERR;
    xpsIpTnlGblDbEntry_t *lookupEntry = NULL;
    xpsScope_t scopeId;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        return XP_ERR_INVALID_DEV_ID;
    }

    rt = xpsScopeGetScopeId(devId, &scopeId);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: Failed to get scope Id for device %d\n", devId);
        return rt;
    }

    rt = xpsIpTunnelGblGetDbEntry(scopeId, tnlIntfId, &lookupEntry);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Vxlan Tunnel Not found :%d \n", tnlIntfId);
        return rt;
    }

    for (uint32_t i=0; i < lookupEntry->numOfEports; i++)
    {
        rt = xpsVxlanClearTunnelEportConfig(devId, lookupEntry->ePorts[i].ePort);
        if (rt != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "ClearTunnelEport Failed :%d \n", lookupEntry->ePorts[i].ePort);
            return rt;
        }

        rt = xpsAllocatorReleaseId(scopeId, XP_ALLOC_GLOBAL_EPORT,
                                   lookupEntry->ePorts[i].ePort);
        if (rt != XP_NO_ERR)
        {
            XPS_FUNC_EXIT_LOG();
            return rt;
        }

        xpsInterfaceInfo_t *l3IntfInfo = NULL;
        rt = xpsInterfaceGetInfoScope(scopeId, lookupEntry->ePorts[i].l3IntfId,
                                      &l3IntfInfo);
        if (rt != XP_NO_ERR || l3IntfInfo == NULL)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to fetch interface type from l3 interface id %d\n",
                  lookupEntry->ePorts[i].l3IntfId);
            return rt;
        }

        /* For SVI, remove for first member and skip rest. */
        rt = xpsIntfChildRemove(l3IntfInfo, tnlIntfId);
        if (rt != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEFAULT,
                  "remove tnl intf %d from L3 Intf %d\n", tnlIntfId,
                  lookupEntry->ePorts[i].l3IntfId);
            continue;
        }

        rt = xpsReleaseTunnelStartIndex(devId, lookupEntry->ePorts[i].tnlStartId);
        if (rt != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Tunnel (%d) Start pointer (%d) release failed", tnlIntfId,
                  lookupEntry->ePorts[i].tnlStartId);
            return rt;
        }
    }

    rt = xpsClearTunnelEcmpConfig(devId, lookupEntry->ecmpStartIdx,
                                  lookupEntry->numOfEports);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Tunnel (%d) Clear (%d) failed", tnlIntfId,
              lookupEntry->ecmpStartIdx);
        return rt;
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVxlanDestroyTunnelInterface(xpsInterfaceId_t tnlIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    return xpsVxlanDestroyTunnelInterfaceScope(XP_SCOPE_DEFAULT,  tnlIntfId);

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

static XP_STATUS xpsVxlanDeleteDbEntry(xpsScope_t scopeId,
                                       xpsInterfaceId_t tunnelIntfId)
{
    XP_STATUS status = XP_NO_ERR;
    xpsVxlanDbEntry_t keyVxlanDbEntry;
    xpsVxlanDbEntry_t *vxlanDbEntry = NULL;

    memset(&keyVxlanDbEntry, 0, sizeof(keyVxlanDbEntry));

    keyVxlanDbEntry.tnlIntfId = tunnelIntfId;
    status = xpsStateDeleteData(scopeId, vxlanDbHndl, (xpsDbKey_t)&keyVxlanDbEntry,
                                (void **)&vxlanDbEntry);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to delete VNI DB entry\n");
        return status;
    }

    status = xpsStateHeapFree((void *)vxlanDbEntry);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to free VNI DB entry\n");
        return status;
    }

    return XP_NO_ERR;
}

XP_STATUS xpsVxlanDestroyTunnelInterfaceScope(xpsScope_t scopeId,
                                              xpsInterfaceId_t tnlIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsVxlanDestroyTunnelInterfaceScope);

    XP_STATUS rt = XP_NO_ERR;
    xpsVxlanDbEntry_t *lookupEntry = NULL;

    rt = xpsVxlanGetTunnelDb(scopeId, tnlIntfId, &lookupEntry);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Vxlan Tunnel Not found :%d \n", tnlIntfId);
        return rt;
    }

    rt = xpsIpTunnelDelete(scopeId, tnlIntfId);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Ip tunnel delete failed, tunnel interface(%d)", tnlIntfId);
        return rt;
    }

    if (lookupEntry->vniList)
    {
        rt = xpsRBTDelete(lookupEntry->vniList);
        if (rt != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "RBTDel failed");
            return rt;
        }
    }

    rt = xpsVxlanDeleteDbEntry(scopeId, tnlIntfId);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Delete the vxlan context failed,"
              "tnlIntfId(%d)", tnlIntfId);
        return rt;
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVxlanSetTunnelNextHopData(xpsDevice_t devId,
                                       xpsInterfaceId_t tnlIntfId, uint32_t nhId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVxlanUpdateTunnelNextHopData(xpsDevice_t devId,
                                          xpsInterfaceId_t tnlIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVxlanSetUdpPort(xpsDevice_t devId, uint32_t udpPort)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVxlanGetUdpPort(xpsDevice_t devId, uint32_t *udpPort)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVxlanAddLocalVtep(xpsDevice_t devId, ipv4Addr_t localIp)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVxlanRemoveLocalVtep(xpsDevice_t devId, ipv4Addr_t localIp)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVxlanSetTunnelConfig(xpsDevice_t devId, xpsInterfaceId_t tnlIntfId,
                                  xpsVxlanTunnelConfig_t *tunnelConfig)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsVxlanSetTunnelConfig);

    XP_STATUS rt = XP_NO_ERR;
    xpsIpTunnelConfig_t cfg;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        return XP_ERR_INVALID_DEV_ID;
    }

    cfg.vxlanCfg = *tunnelConfig;

    rt = xpsIpTunnelSetConfig(devId, tnlIntfId, XP_IP_VXLAN_TUNNEL, &cfg);
    if (rt)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Set VxLan tunnel config failed, tunnel interface(%d)", tnlIntfId);
        return rt;
    }

    return rt;

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVxlanGetTunnelConfig(xpsDevice_t devId, xpsInterfaceId_t tnlIntfId,
                                  xpsVxlanTunnelConfig_t *tunnelConfig)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVxlanGetTunnelRemoteIp(xpsDevice_t devId,
                                    xpsInterfaceId_t tnlIntfId, ipv4Addr_t *rmtEpIpAddr)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsVxlanGetTunnelRemoteIp);

    XP_STATUS rt = XP_NO_ERR;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        return XP_ERR_INVALID_DEV_ID;
    }

    rt = xpsIpTunnelGetRemoteIp(devId, XP_IP_VXLAN_TUNNEL, tnlIntfId, rmtEpIpAddr);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get VxLan tunnel remote IP failed, tunnel interface(%d)", tnlIntfId);
        return rt;
    }

    return rt;

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVxlanRemoveEportFromTunnelTermination
(
    xpsDevice_t devId,
    xpsInterfaceId_t tnlIntfId,
    xpsInterfaceId_t intfId,
    uint32_t ePort
)
{
    GT_STATUS  rc = GT_OK;
    XP_STATUS  rt = XP_NO_ERR;
    xpsScope_t scopeId;
    uint32_t intfPresent = 0;
    uint32_t ttiRuleId = 0xFFFFFFFF;
    xpsIpTnlTermEntry_t *termInfo = NULL;

    rt = xpsScopeGetScopeId(devId, &scopeId);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: Failed to get scope Id for device %d\n", devId);
        return rt;
    }

    while (xpsIpTunnelGetNextTermEntry(scopeId, tnlIntfId,
                                       &termInfo) == XP_NO_ERR)
    {
        for (uint32_t i = 0; i < termInfo->dcpEntryCnt; i++)
        {
            intfPresent = 0;
            for (uint32_t j = 0; j < termInfo->entry[i].ruleCnt; j++)
            {
                if (!intfPresent)
                {
                    if (termInfo->entry[i].rule[j].intfId == intfId)
                    {
                        intfPresent = 1;
                        ttiRuleId = termInfo->entry[i].rule[j].ttiRuleId;
                    }
                    continue;
                }
                else if (j != 0)
                {
                    termInfo->entry[i].rule[j-1] = termInfo->entry[i].rule[j];
                }
            }

            if (intfPresent)
            {
                rc = cpssHalTunnelTerminationEntryDelete(XPS_GLOBAL_TACM_MGR,
                                                         TUNNEL_TERM_TABLE_ID, ttiRuleId);
                if (rc != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "cpssHalTunnelTerminationEntryDelete failed err: %d ruleId :%d \n ", rc,
                          ttiRuleId);
                    return xpsConvertCpssStatusToXPStatus(rc);
                }

                rc = cpssHalPclPortIngressPolicyEnable(devId, ePort,
                                                       GT_FALSE);
                if (rc != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          " cpssHalPclPortIngressPolicyEnable err: %d ePort :%d \n ", rc,
                          ePort);
                    return xpsConvertCpssStatusToXPStatus(rc);
                }

                rt = xpsAllocatorReleaseId(scopeId, XP_ALLOC_TUNNEL_TERM_HW_ENTRY, ttiRuleId);
                if (rt != XP_NO_ERR)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Unable to release RuleId :%d | error: %d.\n",
                          ttiRuleId, rt);
                    return rt;
                }
                termInfo->entry[i].ruleCnt--;
            }
        }
    }

    return rt;
}

XP_STATUS xpsVxlanAddEportToTunnelTermination
(
    xpsDevice_t devId,
    xpsInterfaceId_t tnlIntfId,
    xpsInterfaceId_t memberIntfId,
    uint32_t ePort,
    uint32_t priEPort
)
{
    GT_STATUS  rc = GT_OK;
    XP_STATUS  rt = XP_NO_ERR;
    xpsScope_t scopeId;
    xpsIpTnlTermEntry_t *termInfo = NULL;
    xpsInterfaceType_e memberIntfType;
    uint32_t lastTtiRuleId = 0xFFFFFFFF;
    uint32_t ruleIdx = 0xFFFFFFFF;
    uint32_t ttiRuleId = 0xFFFFFFFF;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC ruleType;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC ruleData;
    CPSS_DXCH_TTI_ACTION_STC                   ttiAction;
    CPSS_DXCH_TTI_RULE_UNT                     ttiMask;
    CPSS_DXCH_TTI_RULE_UNT                     ttiPattern;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC RuleAttributes;

    cpssOsMemSet(&ruleType, 0, sizeof(ruleType));
    cpssOsMemSet(&ruleData, 0, sizeof(ruleData));

    rt = xpsScopeGetScopeId(devId, &scopeId);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: Failed to get scope Id for device %d\n", devId);
        return rt;
    }
    RuleAttributes.priority          = 0; /* Not be used */
    ruleType.ruleType = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_TTI_E;
    ruleType.rule.tti.ruleFormat = CPSS_DXCH_TTI_RULE_UDB_30_E;

    while (xpsIpTunnelGetNextTermEntry(scopeId, tnlIntfId,
                                       &termInfo) == XP_NO_ERR)
    {
        for (uint32_t i = 0; i < termInfo->dcpEntryCnt; i++)
        {
            ruleIdx = termInfo->entry[i].ruleCnt;
            if (ruleIdx == 0)
            {
                continue;
            }

            lastTtiRuleId = termInfo->entry[i].rule[ruleIdx-1].ttiRuleId;

            cpssOsMemSet(&ttiMask, 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
            cpssOsMemSet(&ttiPattern, 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
            cpssOsMemSet(&ttiAction, 0, sizeof(CPSS_DXCH_TTI_ACTION_STC));

            ruleData.rule.tti.actionPtr     = &ttiAction;
            ruleData.rule.tti.maskPtr       = &ttiMask;
            ruleData.rule.tti.patternPtr    = &ttiPattern;

            rc = cpssHalTunnelTerminationRuleGet(XPS_GLOBAL_TACM_MGR, TUNNEL_TERM_TABLE_ID,
                                                 lastTtiRuleId,
                                                 &ruleType, &ruleData);
            if (rc != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssHalTunnelTerminationRuleGet failed err: %d ruleId :%d \n ", rc,
                      lastTtiRuleId);
                return xpsConvertCpssStatusToXPStatus(rc);
            }

            rt = xpsInterfaceGetType(memberIntfId, &memberIntfType);
            if (rt != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to fetch interface type from interface id %d\n", memberIntfId);
                return rt;
            }
            if (memberIntfType == XPS_LAG)
            {
                ttiPattern.udbArray.udb[1] = (xpsUtilXpstoCpssInterfaceConvert(
                                                  memberIntfId, XPS_LAG) & 0xFF);
                ttiMask.udbArray.udb[1]    = 0xFF;

                ttiPattern.udbArray.udb[0] |= (1 << 5); //Is Trunk bit.
                ttiMask.udbArray.udb[0] |= (1 << 5); //Check Trunk bit.
            }
            else
            {
                ttiPattern.udbArray.udb[1] = memberIntfId & 0xFF;
                ttiMask.udbArray.udb[1]    = 0xFF;

                ttiMask.udbArray.udb[0] |= (1 << 5); //check Trunk bit.
            }

            ttiAction.sourceEPort = priEPort;

            rt = xpsAllocatorAllocateId(scopeId, XP_ALLOC_TUNNEL_TERM_HW_ENTRY, &ttiRuleId);
            if (rt != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "xpsAllocatorAllocateId: Unable to allocate HW Tunnel id, error: %d.\n", rt);
                return rt;
            }

            if (ttiRuleId >= curMaxTnlTermEntries)
            {
                rc = cpssHalVirtualTcamResize(XPS_GLOBAL_TACM_MGR, TUNNEL_TERM_TABLE_ID,
                                              CPSS_DXCH_VIRTUAL_TCAM_APPEND_CNS, GT_TRUE, 128);
                if (rc != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "cpssHalVirtualTcamResize failed err: %d\n ", rc);
                    return xpsConvertCpssStatusToXPStatus(rc);
                }
                curMaxTnlTermEntries += 128;
            }

            rc = cpssHalTunnelTerminationEntryAdd(XPS_GLOBAL_TACM_MGR, TUNNEL_TERM_TABLE_ID,
                                                  ttiRuleId, &RuleAttributes,
                                                  &ruleType, &ruleData);
            if (rc != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssHalTunnelTerminationEntryAdd failed err: %d ruleId :%d \n ", rc,
                      ttiRuleId);
                return xpsConvertCpssStatusToXPStatus(rc);
            }

            termInfo->entry[i].rule[ruleIdx].ttiRuleId = ttiRuleId;
            termInfo->entry[i].rule[ruleIdx].intfId = memberIntfId;
            termInfo->entry[i].ruleCnt++;
        }

        rc = cpssHalPclPortIngressPolicyEnable(devId, ePort, GT_TRUE);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssHalTunnelTerminationEntryAdd failed err: %d ruleId :%d \n ", rc,
                  ttiRuleId);
            return xpsConvertCpssStatusToXPStatus(rc);
        }
    }
    return rt;
}

XP_STATUS xpsVxlanAddTunnelTermination(
    xpsDevice_t devId,
    xpsInterfaceId_t tnlIntfId,
    uint32_t tnlTermId,
    inetAddr_t *srcIp,
    inetAddr_t *dstIp,
    uint32_t vniId,
    xpsVlan_t vlanId,
    XpIpTunnelMode_t dcpTtlMode,
    XpIpTunnelMode_t dcpDscpMode,
    uint16_t udpDstPort
)
{
    XPS_FUNC_ENTRY_LOG();
    XPS_LOCK(xpsVxlanAddTunnelTermination);

    GT_STATUS  rc = GT_OK;
    XP_STATUS  rt = XP_NO_ERR;
    xpsScope_t scopeId;
    xpsIpTnlGblDbEntry_t *lookupEntry = NULL;
    xpsVniDbEntry_t *vniCtx = NULL;
    xpsInterfaceId_t memberIntfId;
    xpsInterfaceType_e memberIntfType;
    ipv4Addr_t zerov4Ip;
    ipv6Addr_t zerov6Ip;
    memset(&zerov4Ip, 0, sizeof(ipv4Addr_t));
    memset(&zerov6Ip, 0, sizeof(ipv6Addr_t));

    if (IS_DEVICE_VALID(devId) == 0)
    {
        return XP_ERR_INVALID_DEV_ID;
    }

    rt = xpsScopeGetScopeId(devId, &scopeId);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: Failed to get scope Id for device %d\n", devId);
        return rt;
    }

    rt = xpsIpTunnelGblGetDbEntry(scopeId, tnlIntfId, &lookupEntry);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Vxlan Tunnel Not found :%d \n", tnlIntfId);
        return rt;
    }

    rt = xpsVniGetDbEntry(scopeId, vniId, &vniCtx);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get VNI Context Db failed, vni(%d)", vniId);
        return rt;
    }
    xpsIpTnlTermEntry_t* termInfo = NULL;
    rt = xpsIpTunnelFindTermEntry(scopeId, tnlIntfId, tnlTermId,
                                  &termInfo);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEFAULT,
              "tnl Term not found (%d) (%d)", tnlIntfId, tnlTermId);
    }

    if (termInfo == NULL)
    {
        rt = xpsIpTunnelAddToTermList(scopeId, tnlIntfId, tnlTermId, &termInfo);
        if (rt != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Tnl Term Add failed, vni(%d)", tnlTermId);
            return rt;
        }
    }

    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC       TcamRuleType;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ID             ttiRuleId = 0XFFFFFFFF;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC RuleAttributes;
    CPSS_DXCH_TTI_ACTION_STC                   ttiAction;
    CPSS_DXCH_TTI_RULE_UNT                     ttiMask;
    CPSS_DXCH_TTI_RULE_UNT                     ttiPattern;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC       RuleData;
    int i = 0;

    RuleAttributes.priority          = 0; /* Not be used */
    RuleData.valid                   = GT_TRUE;
    TcamRuleType.ruleType = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_TTI_E;

    cpssOsMemSet(&ttiMask, 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
    cpssOsMemSet(&ttiPattern, 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
    cpssOsMemSet(&ttiAction, 0, sizeof(CPSS_DXCH_TTI_ACTION_STC));

    TcamRuleType.rule.tti.ruleFormat = CPSS_DXCH_TTI_RULE_UDB_30_E;

    if (dstIp->type == XP_PREFIX_TYPE_IPV4)
    {
        /* PCL-ID */
        ttiPattern.udbArray.udb[0] = (CPSS_HAL_TTI_UDB_IPV4_VXLAN_PCL_ID & 0x1F);
        ttiMask.udbArray.udb[0]    = 0x1F;

        /*Source port/trunk set below*/

        /* IP header */
        ttiPattern.udbArray.udb[2] = 17; /* udp */
        ttiMask.udbArray.udb[2]    = 0xFF;

        /* Udp port */
        ttiPattern.udbArray.udb[3] = (udpDstPort>>8) & 0xFF;
        ttiMask.udbArray.udb[3]    = 0xFF;

        ttiPattern.udbArray.udb[4] = udpDstPort & 0xFF;
        ttiMask.udbArray.udb[4]    = 0xFF;

        /* VNI ID */
        ttiPattern.udbArray.udb[5] = (vniId>>16) & 0xFF;
        ttiMask.udbArray.udb[5]    = 0xFF;

        ttiPattern.udbArray.udb[6] = (vniId>>8) & 0xFF;
        ttiMask.udbArray.udb[6]    = 0xFF;

        ttiPattern.udbArray.udb[7] = vniId & 0xFF;
        ttiMask.udbArray.udb[7]    = 0xFF;

        if (memcmp(dstIp->addr.ipv4Addr, &zerov4Ip, sizeof(ipv4Addr_t)))
        {
            /* DIP */
            for (i = 0; i < CPSS_HAL_IPV4_ADDR_LEN; i++)
            {
                ttiPattern.udbArray.udb[i+8] =
                    dstIp->addr.ipv4Addr[i];
                ttiMask.udbArray.udb[i+8] = 0xFF;
            }
        }

        if (memcmp(srcIp->addr.ipv4Addr, &zerov4Ip, sizeof(ipv4Addr_t)))
        {
            /* SIP */
            for (i = 0; i < CPSS_HAL_IPV4_ADDR_LEN; i++)
            {
                ttiPattern.udbArray.udb[i+12] =
                    srcIp->addr.ipv4Addr[i];
                ttiMask.udbArray.udb[i+12] = 0xFF;
            }
        }
    }
    else
    {
        /* PCL-ID */
        ttiPattern.udbArray.udb[0] = CPSS_HAL_TTI_UDB_IPV6_VXLAN_PCL_ID & 0x1F;
        ttiMask.udbArray.udb[0]    = 0x1F;

        /*Source port/trunk set below*/

        /* IP header */
        ttiPattern.udbArray.udb[2] = 17; /* udp */
        ttiMask.udbArray.udb[2]    = 0xFF;

        /* Udp port */
        ttiPattern.udbArray.udb[3] = (udpDstPort>>8) & 0xFF;
        ttiMask.udbArray.udb[3]    = 0xFF;

        ttiPattern.udbArray.udb[4] = udpDstPort & 0xFF;
        ttiMask.udbArray.udb[4]    = 0xFF;

        if (memcmp(dstIp->addr.ipv6Addr, &zerov6Ip, sizeof(ipv6Addr_t)))
        {
            /* DIP */
            for (i = 0; i < CPSS_HAL_IPV6_ADDR_LEN; i++)
            {
                ttiPattern.udbArray.udb[5+i] =
                    dstIp->addr.ipv6Addr[i];
                ttiMask.udbArray.udb[5+i] = 0xFF;
            }
        }
    }

    /*Set MAC2ME*/
    ttiPattern.udbArray.udb[0] |= (1 << 7);
    ttiMask.udbArray.udb[0] |= (1 << 7);

    ttiAction.command                       = CPSS_PACKET_CMD_FORWARD_E;
    ttiAction.redirectCommand               = CPSS_DXCH_TTI_NO_REDIRECT_E;
    ttiAction.userDefinedCpuCode            = CPSS_NET_FIRST_USER_DEFINED_E;
    ttiAction.tunnelTerminate               = GT_TRUE;
    ttiAction.ttPassengerPacketType         =
        CPSS_DXCH_TTI_PASSENGER_ETHERNET_NO_CRC_E;
    ttiAction.qosPrecedence                 =
        CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
    ttiAction.keepPreviousQoS               = GT_TRUE;;
    ttiAction.modifyTag0Up                  = CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_UP_E;
    ttiAction.modifyDscp                    =
        CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_DSCP_E;
    ttiAction.setMacToMe                    = GT_TRUE;
    /* Enable Split Horizon Filter */
    ttiAction.sourceIdSetEnable = GT_TRUE;
    /* The packet was sent from tunnel. We need to set mesh id to 1 (0 is from local network) */
    ttiAction.sourceId = 1;

    if (dstIp->type == XP_PREFIX_TYPE_IPV4)
    {
        ttiAction.tag0VlanCmd                 = CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E;
        ttiAction.tag0VlanId                  = vniCtx->hwEVlanId;
        ttiAction.tag1VlanCmd                 = CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E;
        ttiAction.tag1VlanId                  = 0;
        ttiAction.sourceEPortAssignmentEnable = GT_TRUE;
    }

    if (dstIp->type == XP_PREFIX_TYPE_IPV6)
    {
        ttiAction.tag0VlanCmd                 = CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E;
        ttiAction.tag0VlanId                  = 0xFFF; /* Should be overwritten */
        ttiAction.tag1VlanCmd                 = CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E;
        ttiAction.tag1VlanId                  = 0xFFF; /* Should be overwritten */

        /* IPCL will assign source ePort and eVlan */
        ttiAction.pcl0OverrideConfigIndex =
            CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E;
        ttiAction.iPclConfigIndex         = XPS_VXLAN_IPV6_PCL_CFG_TABLE_INDEX_CNS;
        ttiAction.iPclUdbConfigTableEnable = GT_TRUE;
        ttiAction.iPclUdbConfigTableIndex  =
            XPS_VXLAN_IPV6_PCL_UDB_PACKET_TYPE_CNS;
        ttiAction.sourceEPortAssignmentEnable = GT_TRUE;
    }

    if (dcpTtlMode == XP_IP_TUNNEL_MODE_UNIFORM)
    {
        ttiAction.copyTtlExpFromTunnelHeader = GT_TRUE;
    }

    if (dcpDscpMode == XP_IP_TUNNEL_MODE_UNIFORM)
    {
        ttiAction.modifyDscp = CPSS_DXCH_TTI_MODIFY_DSCP_ENABLE_E;
    }

    uint32_t dcpEntIdx = termInfo->dcpEntryCnt;

    xpsIpTnlTermEntry_t* termCtxNewPtr = NULL;
    if (xpsDAIsCtxGrowthNeeded(dcpEntIdx, MAX_DECAP_ENTRY))
    {
        rt = xpsDynamicArrayGrow((void **)&termCtxNewPtr, (void *)termInfo,
                                 sizeof(xpsIpTnlTermEntry_t),
                                 sizeof(xpsIpTnlDecapRuleInfo_t), dcpEntIdx, MAX_DECAP_ENTRY);
        if (rt != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Dynamic array grow failed");
            return rt;
        }

        //Delete node from tree
        rt = xpsIpTunnelDelFromTermList(scopeId, tnlIntfId, tnlTermId);
        if (rt != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "tnl Term Del Failed (%d) (%d) ", tnlIntfId, tnlTermId);
            if (termCtxNewPtr)
            {
                free(termCtxNewPtr);
                termCtxNewPtr = NULL;
            }
        }

        termInfo = NULL;
        //Add node to tree
        rt = xpsIpTunnelAddToTermList(scopeId, tnlIntfId, tnlTermId, &termInfo);
        if (rt != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Tnl Term Add failed, vni(%d)", tnlTermId);
            if (termCtxNewPtr)
            {
                free(termCtxNewPtr);
                termCtxNewPtr = NULL;
            }
            return rt;
        }

        if (termCtxNewPtr)
        {
            memcpy(termInfo, termCtxNewPtr, sizeof(xpsIpTnlTermEntry_t));
            free(termCtxNewPtr);
            termCtxNewPtr = NULL;
        }
    }

    uint32_t ruleIdx = 0;
    for (uint32_t i = 0; i< lookupEntry->numOfEports; i++)
    {
        ttiPattern.udbArray.udb[0] &= ~(1 << 5); //Is Trunk bit.
        ttiMask.udbArray.udb[0] &= ~(1 << 5); //Check Trunk bit.

        ruleIdx = termInfo->entry[dcpEntIdx].ruleCnt;
        ttiRuleId = 0;
        memberIntfId = lookupEntry->ePorts[i].intfId;
        rt = xpsInterfaceGetType(memberIntfId, &memberIntfType);
        if (rt != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to fetch interface type from interface id %d\n", memberIntfId);
            return rt;
        }

        if (memberIntfType == XPS_LAG)
        {
            ttiPattern.udbArray.udb[1] = (xpsUtilXpstoCpssInterfaceConvert(
                                              memberIntfId, XPS_LAG) & 0xFF);
            ttiMask.udbArray.udb[1]    = 0xFF;

            ttiPattern.udbArray.udb[0] |= (1 << 5); //Is Trunk bit.
            ttiMask.udbArray.udb[0] |= (1 << 5); //Check Trunk bit.
        }
        else
        {
            ttiPattern.udbArray.udb[1] = memberIntfId & 0xFF;
            ttiMask.udbArray.udb[1]    = 0xFF;

            ttiMask.udbArray.udb[0] |= (1 << 5); //check Trunk bit.
        }

        ttiAction.sourceEPort =
            lookupEntry->primaryEport;//lookupEntry->ePorts[i].ePort;

        rt = xpsAllocatorAllocateId(scopeId, XP_ALLOC_TUNNEL_TERM_HW_ENTRY, &ttiRuleId);
        if (rt != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "xpsAllocatorAllocateId: Unable to allocate HW Tunnel id, error: %d.\n", rt);
            return rt;
        }

        if (ttiRuleId >= curMaxTnlTermEntries)
        {
            rc = cpssHalVirtualTcamResize(XPS_GLOBAL_TACM_MGR, TUNNEL_TERM_TABLE_ID,
                                          CPSS_DXCH_VIRTUAL_TCAM_APPEND_CNS, GT_TRUE, 128);
            if (rc != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssHalVirtualTcamResize failed err: %d\n ", rc);
                return xpsConvertCpssStatusToXPStatus(rc);
            }
            curMaxTnlTermEntries += 128;
        }

        RuleData.rule.tti.actionPtr     = &ttiAction;
        RuleData.rule.tti.maskPtr       = &ttiMask;
        RuleData.rule.tti.patternPtr    = &ttiPattern;
        rc = cpssHalTunnelTerminationEntryAdd(XPS_GLOBAL_TACM_MGR, TUNNEL_TERM_TABLE_ID,
                                              ttiRuleId, &RuleAttributes,
                                              &TcamRuleType, &RuleData);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssHalTunnelTerminationEntryAdd failed err: %d ruleId :%d \n ", rc,
                  ttiRuleId);
            return xpsConvertCpssStatusToXPStatus(rc);
        }

        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEFAULT,
              "cpssHalTunnelTerminationEntryAdd Success ttiRuleId :%d \n", ttiRuleId);

        rc = cpssHalPclPortIngressPolicyEnable(devId, lookupEntry->ePorts[i].ePort,
                                               GT_TRUE);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssHalTunnelTerminationEntryAdd failed err: %d ruleId :%d \n ", rc,
                  ttiRuleId);
            return xpsConvertCpssStatusToXPStatus(rc);
        }

        termInfo->entry[dcpEntIdx].rule[ruleIdx].ttiRuleId = ttiRuleId;
        termInfo->entry[dcpEntIdx].rule[ruleIdx].intfId = memberIntfId;
        termInfo->entry[dcpEntIdx].ruleCnt++;
    }

    if (dstIp->type == XP_PREFIX_TYPE_IPV6)
    {
        GT_U32 maxKeyFlds = 21;
        xpsAclkeyFieldList_t       aclFieldData;
        xpsAclkeyField_t           aclFieldList[maxKeyFlds];
        uint8_t                   iacl_value[maxKeyFlds];
        uint8_t                   iacl_mask[maxKeyFlds];
        xpsPclAction_t             aclEntryData;
        uint32_t i;
        uint32_t aclRuleId = 0;

        rt = xpsAllocatorAllocateId(scopeId, XP_ALLOC_VXLAN_IPV6_ACL, &aclRuleId);
        if (rt != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "xpsAllocatorAllocateId: error: %d.\n", rt);
            return rt;
        }

        memset(&aclFieldData, 0x00, sizeof(aclFieldData));
        memset(&aclEntryData, 0x00, sizeof(aclEntryData));
        memset(&aclFieldList, 0x00, sizeof(aclFieldList));
        memset(iacl_value, 0x00, sizeof(iacl_value));
        memset(iacl_mask, 0x00, sizeof(iacl_mask));

        aclEntryData.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
        //        aclEntryData.sourcePort.assignSourcePortEnable = GT_TRUE;
        //        aclEntryData.sourcePort.sourcePortValue = lookupEntry->ePorts[0].ePort;
        aclEntryData.vlan.ingress.modifyVlan = CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;
        aclEntryData.vlan.ingress.vlanId = vniCtx->hwEVlanId;
        aclEntryData.vlan.ingress.precedence =
            CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;

        if (vniCtx->isL3Vni)
        {
            /* for ipv6 tti unit can't assign vrf so pcl redirect is used */
            aclEntryData.redirect.redirectCmd =
                CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_VIRT_ROUTER_E;
            aclEntryData.redirect.data.vrfId = vniCtx->vrfId;
        }

        for (i =0; i < maxKeyFlds; i++)
        {
            aclFieldList[i].value = (uint8_t*)&iacl_value[i];
            aclFieldList[i].mask  = (uint8_t*)&iacl_mask[i];
        }

        aclFieldData.fldList = aclFieldList;
        aclFieldData.numFlds = maxKeyFlds;
        aclFieldData.isValid = 1;

        uint8_t value = XPS_VXLAN_IPV6_PCL_ID_CNS & 0xFF;
        uint8_t keyMask = 0xFF;
        aclFieldData.fldList[0].keyFlds = XPS_PCL_PCLID;
        memcpy(aclFieldData.fldList[0].value, &value, sizeof(uint8_t));
        memcpy(aclFieldData.fldList[0].mask, &keyMask, sizeof(uint8_t));
        value = (XPS_VXLAN_IPV6_PCL_ID_CNS>>8) & 0x3;
        keyMask = 0x3;
        memcpy(aclFieldData.fldList[1].value, &value, sizeof(uint8_t));
        memcpy(aclFieldData.fldList[1].mask, &keyMask, sizeof(uint8_t));

        aclFieldData.fldList[2].keyFlds = XPS_PCL_VNI;
        value = ((vniId>>16) & 0xFF);
        keyMask = 0xFF;
        memcpy(aclFieldData.fldList[2].value, &value, sizeof(uint8_t));
        memcpy(aclFieldData.fldList[2].mask, &keyMask, sizeof(uint8_t));
        value = ((vniId>>8) & 0xFF);
        keyMask = 0xFF;
        memcpy(aclFieldData.fldList[3].value, &value, sizeof(uint8_t));
        memcpy(aclFieldData.fldList[3].mask, &keyMask, sizeof(uint8_t));
        value = (vniId & 0xFF);
        keyMask = 0xFF;
        memcpy(aclFieldData.fldList[4].value, &value, sizeof(uint8_t));
        memcpy(aclFieldData.fldList[4].mask, &keyMask, sizeof(uint8_t));

        if (memcmp(srcIp->addr.ipv6Addr, &zerov6Ip, sizeof(ipv6Addr_t)))
        {
            aclFieldData.fldList[5].keyFlds = XPS_PCL_IPV6_SIP;
            for (i =0; i < CPSS_HAL_IPV6_ADDR_LEN; i++)
            {
                memcpy(aclFieldData.fldList[i+5].value, &srcIp->addr.ipv6Addr[i],
                       sizeof(uint8_t));
                memcpy(aclFieldData.fldList[i+5].mask, &keyMask, sizeof(uint8_t));
            }
        }

        rt =  xpsAclWriteEntry(devId, vxlanIpv6AclTableId_g, aclRuleId, aclRuleId,
                               &aclFieldData,
                               aclEntryData, XPS_PCL_VXLAN_IPV6_KEY, true);
        if (rt != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Error: xpsAclWriteEntry failed for ruleId: %d \n", aclRuleId);
            return rt;
        }

        termInfo->entry[termInfo->dcpEntryCnt].aclRuleId = aclRuleId;
    }
    termInfo->dcpEntryCnt++;

    XPS_FUNC_EXIT_LOG();

    return rt;
}

XP_STATUS xpsVxlanRemoveTunnelTermination(
    xpsDevice_t devId,
    xpsInterfaceId_t tnlIntfId,
    uint32_t tnlTermId,
    bool isIpv6
)
{
    XPS_FUNC_ENTRY_LOG();
    XPS_LOCK(xpsVxlanRemoveTunnelTermination);

    GT_STATUS rc = GT_OK;
    XP_STATUS rt = XP_NO_ERR;
    xpsScope_t scopeId;
    xpsIpTnlGblDbEntry_t *lookupEntry = NULL;
    uint32_t aclRuleId = 0;
    uint32_t ttiRuleId = 0;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        return XP_ERR_INVALID_DEV_ID;
    }

    rt = xpsScopeGetScopeId(devId, &scopeId);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: Failed to get scope Id for device %d\n", devId);
        return rt;
    }

    rt = xpsIpTunnelGblGetDbEntry(scopeId, tnlIntfId, &lookupEntry);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Vxlan Tunnel Not found :%d \n", tnlIntfId);
        return rt;
    }

    xpsIpTnlTermEntry_t* termInfo = NULL;
    rt = xpsIpTunnelFindTermEntry(scopeId, tnlIntfId, tnlTermId,
                                  &termInfo);
    if (rt != XP_NO_ERR || termInfo == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "tnl Term not found (%d) (%d)", tnlIntfId, tnlTermId);
        return rt;
    }

    for (uint32_t i =0 ; i < termInfo->dcpEntryCnt; i++)
    {
        for (uint32_t j=0; j <  termInfo->entry[i].ruleCnt; j++)
        {
            ttiRuleId = termInfo->entry[i].rule[j].ttiRuleId;
            rc = cpssHalTunnelTerminationEntryDelete(XPS_GLOBAL_TACM_MGR,
                                                     TUNNEL_TERM_TABLE_ID, ttiRuleId);
            if (rc != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssHalTunnelTerminationEntryDelete failed err: %d ruleId :%d \n ", rc,
                      ttiRuleId);
                return xpsConvertCpssStatusToXPStatus(rc);
            }

            rt = xpsAllocatorReleaseId(scopeId, XP_ALLOC_TUNNEL_TERM_HW_ENTRY, ttiRuleId);
            if (rt != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Unable to release RuleId :%d | error: %d.\n",
                      ttiRuleId, rt);
                return rt;
            }
        }

        if (isIpv6)
        {
            aclRuleId = termInfo->entry[i].aclRuleId;
            rt =  xpsAclDeleteEntry(devId, vxlanIpv6AclTableId_g, aclRuleId, true);
            if (rt != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Error: xpsAclDeleteEntry failed for ruleId: %d \n", aclRuleId);
                return rt;
            }

            rt = xpsAllocatorReleaseId(scopeId, XP_ALLOC_VXLAN_IPV6_ACL, aclRuleId);
            if (rt != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Unable to release RuleId :%d | error: %d.\n",
                      aclRuleId, rt);
                return rt;
            }
        }
    }

    rt = xpsIpTunnelDelFromTermList(scopeId, tnlIntfId, tnlTermId);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "tnl Term Del Failed (%d) (%d) ", tnlIntfId, tnlTermId);
    }

    for (uint32_t i = 0; i< lookupEntry->numOfEports; i++)
    {
        rc = cpssHalPclPortIngressPolicyEnable(devId, lookupEntry->ePorts[i].ePort,
                                               GT_FALSE);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssHalTunnelTerminationEntryAdd failed err: %d ruleId :%d \n ", rc,
                  ttiRuleId);
            return xpsConvertCpssStatusToXPStatus(rc);
        }
    }

    XPS_FUNC_EXIT_LOG();

    return rt;
}


XP_STATUS xpsVxlanRemoveVni(xpsDevice_t devId, uint32_t vni)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsVxlanAddMcTunnelEntry(xpsDevice_t devId,
                                   xpsInterfaceId_t tnlIntfId, ipv4Addr_t lclEpIpAddr, ipv4Addr_t rmtEpIpAddr,
                                   xpsInterfaceId_t l3IntfId, xpsInterfaceId_t portIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

static XP_STATUS xpsVxlanAddIntfDb(xpsScope_t scopeId, uint32_t vniId,
                                   xpsInterfaceId_t intfId,
                                   xpsInterfaceType_e intfType,
                                   uint32_t ttiRuleId,
                                   xpsVniDbEntry_t **vniCtxNewPtr)
{

    XP_STATUS result = XP_NO_ERR;
    xpsVniDbEntry_t *vniCtx = NULL;
    uint16_t numOfIntfs;

    result = xpsVniGetDbEntry(scopeId, vniId, &vniCtx);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get Vni Context Db failed, vniId(%d)", vniId);
        return result;
    }

    numOfIntfs = vniCtx->numOfIntfs;

    //Number of interfaces increase exponentially starting from XPS_MAX_VNI_MEMBERS_GROUP by default
    //Everytime the number reaches the current size, the size is doubled
    if (xpsDAIsCtxGrowthNeeded(numOfIntfs, XPS_MAX_VNI_MEMBERS_GROUP))
    {
        result = xpsDynamicArrayGrow((void **)vniCtxNewPtr, (void *)vniCtx,
                                     sizeof(xpsVniDbEntry_t),
                                     sizeof(xpsInterfaceId_t), numOfIntfs, XPS_MAX_VNI_MEMBERS_GROUP);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Dynamic array grow failed");
            return result;
        }

        //Delete the old vniCtx
        result = xpsVniDeleteDbEntry(scopeId, vniId);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Delete the vlan context failed,"
                  "vniId(%d)", vniId);
            xpsStateHeapFree((void *)(*vniCtxNewPtr));
            return result;
        }

        //Insert the new context
        if ((result = xpsStateInsertData(scopeId, vniGblDbHandle,
                                         (void*)(*vniCtxNewPtr))) != XP_NO_ERR)
        {
            xpsStateHeapFree((void*)(*vniCtxNewPtr));
            (*vniCtxNewPtr) = NULL;
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Inserting data in state failed");
            return result;
        }
        vniCtx = *vniCtxNewPtr;
    }

    vniCtx->intfList[numOfIntfs].intfId = intfId;
    vniCtx->intfList[numOfIntfs].intfType = intfType;
    vniCtx->intfList[numOfIntfs].ttiRuleId = ttiRuleId;
    vniCtx->numOfIntfs += 1;

    return result;
}

static XP_STATUS xpsVxlanRemoveIntfDb(xpsScope_t scopeId, xpsVlan_t vniId,
                                      xpsInterfaceId_t intfId, xpsVniDbEntry_t **vniCtxNew)
{

    XP_STATUS result = XP_NO_ERR;
    xpsVniDbEntry_t *vniCtx = NULL;
    uint32_t j = 0, numOfIntfs, intfPresent = 0;

    result = xpsVniGetDbEntry(scopeId, vniId, &vniCtx);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get VNI Context Db failed, vniId(%d)", vniId);
        return result;
    }

    numOfIntfs = vniCtx->numOfIntfs;

    if (!numOfIntfs)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Vlan does not have any interface");
        return XP_ERR_INVALID_ARG;
    }

    //Delete the element from the context
    vniCtx->numOfIntfs = numOfIntfs - 1;
    for (j = 0; j < numOfIntfs; j++)
    {
        if (!intfPresent)
        {
            if (vniCtx->intfList[j].intfId == intfId)
            {
                intfPresent = 1;
            }
            continue;
        }
        else if (j != 0)
        {
            vniCtx->intfList[j-1] = vniCtx->intfList[j];
        }
    }

    if (!intfPresent)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid arguments");
        return XP_ERR_INVALID_ARG;
    }

    if (xpsDAIsCtxShrinkNeeded(numOfIntfs, XPS_MAX_VNI_MEMBERS_GROUP))
    {
        result = xpsDynamicArrayShrink((void **)vniCtxNew, (void *)vniCtx,
                                       sizeof(xpsVniDbEntry_t),
                                       sizeof(xpsInterfaceId_t), numOfIntfs, XPS_MAX_VNI_MEMBERS_GROUP);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Dynamic array shrink failed");
            return result;
        }

        result = xpsVniDeleteDbEntry(scopeId, vniId);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Delete the vni context failed,"
                  "vniId(%d)", vniId);
            xpsStateHeapFree((void*)(*vniCtxNew));
            *vniCtxNew = NULL;
            return result;
        }

        if ((result = xpsStateInsertData(scopeId, vniGblDbHandle,
                                         (void*)*vniCtxNew)) != XP_NO_ERR)
        {
            xpsStateHeapFree((void*)(*vniCtxNew));
            *vniCtxNew = NULL;
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Inserting data in state failed");
            return result;
        }
    }

    return result;
}


static XP_STATUS xpsVxlanGetMemberInfo(xpsScope_t scopeId,
                                       uint32_t vniId,
                                       xpsInterfaceId_t intfId,
                                       xpsVniMember_t *memberInfo)
{
    XP_STATUS xpsRetVal = XP_NO_ERR;
    xpsVniDbEntry_t *vniCtx = NULL;

    if (!memberInfo)
    {
        return XP_ERR_NULL_POINTER;
    }

    xpsRetVal = xpsVniGetDbEntry(scopeId, vniId, &vniCtx);
    if (xpsRetVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get VNI Context Db failed, vni(%d)", vniId);
        return xpsRetVal;
    }

    for (uint16_t i = 0; i < vniCtx->numOfIntfs; i++)
    {
        if (vniCtx->intfList[i].intfId == intfId)
        {
            memcpy(memberInfo, &(vniCtx->intfList[i]), sizeof(xpsVniMember_t));
            return XP_NO_ERR;
        }
    }

    return XP_ERR_NOT_FOUND;
}

static XP_STATUS xpsVxlanDeleteUNIRule(xpsDevice_t devId,
                                       uint32_t ttiRuleId)
{
    XPS_FUNC_ENTRY_LOG();
    GT_STATUS   rc = GT_OK;
    XP_STATUS   xpRetVal = XP_NO_ERR;
    XPS_LOCK(xpsVxlanDeleteUNIRule);
    xpsScope_t scopeId;

    /* Get Scope Id from devId */
    if ((xpRetVal = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return xpRetVal;
    }

    rc = cpssHalTunnelTerminationEntryDelete(XPS_GLOBAL_TACM_MGR,
                                             UNI_TERM_TABLE_ID, ttiRuleId);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "TTI UNI Rule failed err: %d ruleId :%d \n ", rc,
              ttiRuleId);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    xpRetVal = xpsAllocatorReleaseId(scopeId,
                                     XP_ALLOC_TUNNEL_TERM_HW_ENTRY/*XP_ALLOC_VXLAN_UNI_TTI_HW_ENTRY*/,
                                     ttiRuleId);
    if (xpRetVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Unable to release ttiRuleId :%d | error: %d.\n",
              ttiRuleId, xpRetVal);
        return xpRetVal;
    }

    return xpRetVal;
}

static XP_STATUS xpsVxlanGetNextMemberIntf(xpsScope_t scopeId,
                                           xpsVniDbEntry_t *vniCtx,
                                           uint32_t *index,
                                           xpsVniMember_t *info)
{
    XP_STATUS retVal = XP_NO_ERR;
    uint32_t i = 0;
    if (!vniCtx || !index)
    {
        return XP_ERR_NULL_POINTER;
    }

    for (i = *index; i < vniCtx->numOfIntfs; i++)
    {
        if (info)
        {
            info->intfId = vniCtx->intfList[i].intfId;
            info->intfType = vniCtx->intfList[i].intfType;
        }
        break;
    }
    *index = i+1;
    return retVal;
}
static XP_STATUS xpsVxlanAddMLLPairInHw(xpsDevice_t devId,
                                        xpsVniDbEntry_t *vniCtx)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpsScope_t scopeId;
    xpsVniMember_t intfFirstInfo;
    xpsVniMember_t intfSecondInfo;
    uint32_t i = 0;
    uint32_t j = 0;
    uint32_t intfCnt = 0;
    uint32_t intfNumPair = 0;
    uint32_t mllPairIdx = 0;
    uint32_t mllPairNextIdx = 0;
    uint32_t firstBaseIdx = 0;
    xpsInterfaceId_t firstNodeEgrId = 0;
    xpsInterfaceId_t secondNodeEgrId = 0;
    uint32_t firstNodeMeshId = 0;
    uint32_t secondNodeMeshId = 0;
    CPSS_INTERFACE_TYPE_ENT firstNodeIntfType = CPSS_INTERFACE_PORT_E;
    CPSS_INTERFACE_TYPE_ENT secondNodeIntfType = CPSS_INTERFACE_PORT_E;

    if (!vniCtx)
    {
        return XP_ERR_NULL_POINTER;
    }

    /* Get Scope Id from devId */
    if ((retVal = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return retVal;
    }

    intfCnt = vniCtx->numOfIntfs;
    intfNumPair = (intfCnt/2)+(intfCnt%2);

    if (!intfCnt)
    {
        vniCtx->hwL2MllPairIdx = firstBaseIdx;
        return retVal;
    }

    for (i = 0; i < intfNumPair-1; i++)
    {
        /* Case: Create MLL Pairs, except first and last pair index*/
        memset(&intfFirstInfo, 0, sizeof(intfFirstInfo));
        memset(&intfSecondInfo, 0, sizeof(intfSecondInfo));
        retVal = xpsVxlanGetNextMemberIntf(scopeId, vniCtx, &j,
                                           &intfFirstInfo);
        if (retVal != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "First xpsVxlanGetNextMemberIntf Failed : %d\n", retVal);
            return retVal;
        }

        if (intfFirstInfo.intfType == XPS_LAG)
        {
            firstNodeEgrId = xpsUtilXpstoCpssInterfaceConvert(intfFirstInfo.intfId,
                                                              XPS_LAG);
            firstNodeIntfType = CPSS_INTERFACE_TRUNK_E;
        }
        else if (intfFirstInfo.intfType == XPS_PORT ||
                 intfFirstInfo.intfType == XPS_TUNNEL_VXLAN)
        {
            firstNodeEgrId = intfFirstInfo.intfId;
            firstNodeIntfType = CPSS_INTERFACE_PORT_E;
        }

        if (intfFirstInfo.intfType == XPS_TUNNEL_VXLAN)
        {
            firstNodeMeshId = 1;
        }

        retVal = xpsVxlanGetNextMemberIntf(scopeId, vniCtx, &j,
                                           &intfSecondInfo);
        if (retVal != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Second xpsVxlanGetNextMemberIntf Failed : %d\n", retVal);
            return retVal;
        }
        if (intfSecondInfo.intfType == XPS_LAG)
        {
            secondNodeEgrId = xpsUtilXpstoCpssInterfaceConvert(intfSecondInfo.intfId,
                                                               XPS_LAG);
            secondNodeIntfType = CPSS_INTERFACE_TRUNK_E;
        }
        else if (intfSecondInfo.intfType == XPS_PORT ||
                 intfSecondInfo.intfType == XPS_TUNNEL_VXLAN)
        {
            secondNodeEgrId = intfSecondInfo.intfId;
            secondNodeIntfType = CPSS_INTERFACE_PORT_E;
        }

        if (intfSecondInfo.intfType == XPS_TUNNEL_VXLAN)
        {
            secondNodeMeshId = 1;
        }

        /* Allocate MLL PAIR index */
        if (mllPairIdx == 0)
        {
            retVal = xpsAllocatorAllocateId(scopeId, XP_ALLOC_MLL_PAIR_HW_ENTRY,
                                            &mllPairIdx);
            if (retVal != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "xpsAllocatorAllocateId Failed : %d\n", retVal);
                return retVal;
            }
            firstBaseIdx = mllPairIdx;
        }

        /* Allocate MLL PAIR next index */
        retVal = xpsAllocatorAllocateId(scopeId, XP_ALLOC_MLL_PAIR_HW_ENTRY,
                                        &mllPairNextIdx);
        if (retVal != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "xpsAllocatorAllocateId Failed : %d\n", retVal);
            return retVal;
        }

        retVal = xpsL2McMLLPairWrite(devId, vniCtx,
                                     CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E,
                                     firstNodeIntfType,
                                     secondNodeIntfType,
                                     firstNodeEgrId,
                                     secondNodeEgrId,
                                     firstNodeMeshId,
                                     secondNodeMeshId,
                                     mllPairIdx, mllPairNextIdx, GT_FALSE);
        if (retVal != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "FirstOnly xpsL2McMLLPairWrite Failed : %d\n", retVal);
            return retVal;
        }
        /* Swap the index for next loop or last pair config*/
        mllPairIdx = mllPairNextIdx;
        mllPairNextIdx = 0;
    }

    if (i == 0 || i > 0) /* first pair or last pair */
    {
        if (intfCnt%2) /* Only one node in pair*/
        {
            memset(&intfFirstInfo, 0, sizeof(intfFirstInfo));
            retVal = xpsVxlanGetNextMemberIntf(scopeId, vniCtx, &j,
                                               &intfFirstInfo);
            if (retVal != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "First xpsVxlanGetNextMemberIntf Failed : %d\n", retVal);
                return retVal;
            }
            if (intfFirstInfo.intfType == XPS_LAG)
            {
                firstNodeEgrId = xpsUtilXpstoCpssInterfaceConvert(intfFirstInfo.intfId,
                                                                  XPS_LAG);
                firstNodeIntfType = CPSS_INTERFACE_TRUNK_E;
            }
            else if (intfFirstInfo.intfType == XPS_PORT ||
                     intfFirstInfo.intfType == XPS_TUNNEL_VXLAN)
            {
                firstNodeEgrId = intfFirstInfo.intfId;
                firstNodeIntfType = CPSS_INTERFACE_PORT_E;
            }

            if (intfFirstInfo.intfType == XPS_TUNNEL_VXLAN)
            {
                firstNodeMeshId = 1;
            }

            /* Allocate MLL PAIR index */
            if (mllPairIdx == 0)
            {
                retVal = xpsAllocatorAllocateId(scopeId, XP_ALLOC_MLL_PAIR_HW_ENTRY,
                                                &mllPairIdx);
                if (retVal != XP_NO_ERR)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "xpsAllocatorAllocateId Failed : %d\n", retVal);
                    return retVal;
                }
                /* First pair and First Node Only */
                if (i == 0)
                {
                    firstBaseIdx = mllPairIdx;
                }
            }
            retVal = xpsL2McMLLPairWrite(devId, vniCtx,
                                         CPSS_DXCH_PAIR_READ_WRITE_FIRST_ONLY_E,
                                         firstNodeIntfType, CPSS_INTERFACE_INDEX_E,
                                         firstNodeEgrId, 0,
                                         firstNodeMeshId,
                                         secondNodeMeshId,
                                         mllPairIdx, 0, GT_TRUE);
            if (retVal != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "FirstOnly xpsL2McMLLPairWrite Failed : %d\n", retVal);
                return retVal;
            }
        }
        else /* Both nodes are in pair*/
        {
            memset(&intfFirstInfo, 0, sizeof(intfFirstInfo));
            memset(&intfSecondInfo, 0, sizeof(intfSecondInfo));
            retVal = xpsVxlanGetNextMemberIntf(scopeId, vniCtx, &j,
                                               &intfFirstInfo);
            if (retVal != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "First xpsVxlanGetNextMemberIntf Failed : %d\n", retVal);
                return retVal;
            }
            if (intfFirstInfo.intfType == XPS_LAG)
            {
                firstNodeEgrId = xpsUtilXpstoCpssInterfaceConvert(intfFirstInfo.intfId,
                                                                  XPS_LAG);
                firstNodeIntfType = CPSS_INTERFACE_TRUNK_E;
            }
            else if (intfFirstInfo.intfType == XPS_PORT ||
                     intfFirstInfo.intfType == XPS_TUNNEL_VXLAN)
            {
                firstNodeEgrId = intfFirstInfo.intfId;
                firstNodeIntfType = CPSS_INTERFACE_PORT_E;
            }

            if (intfFirstInfo.intfType == XPS_TUNNEL_VXLAN)
            {
                firstNodeMeshId = 1;
            }

            retVal = xpsVxlanGetNextMemberIntf(scopeId, vniCtx, &j,
                                               &intfSecondInfo);
            if (retVal != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Second xpsVxlanGetNextMemberIntf Failed : %d\n", retVal);
                return retVal;
            }
            if (intfSecondInfo.intfType == XPS_LAG)
            {
                secondNodeEgrId = xpsUtilXpstoCpssInterfaceConvert(intfSecondInfo.intfId,
                                                                   XPS_LAG);
                secondNodeIntfType = CPSS_INTERFACE_TRUNK_E;
            }
            else if (intfSecondInfo.intfType == XPS_PORT ||
                     intfSecondInfo.intfType == XPS_TUNNEL_VXLAN)
            {
                secondNodeEgrId = intfSecondInfo.intfId;
                secondNodeIntfType = CPSS_INTERFACE_PORT_E;
            }

            if (intfSecondInfo.intfType == XPS_TUNNEL_VXLAN)
            {
                secondNodeMeshId = 1;
            }

            /* Allocate MLL PAIR index */
            if (mllPairIdx == 0)
            {
                retVal = xpsAllocatorAllocateId(scopeId, XP_ALLOC_MLL_PAIR_HW_ENTRY,
                                                &mllPairIdx);
                if (retVal != XP_NO_ERR)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "xpsAllocatorAllocateId Failed : %d\n", retVal);
                    return retVal;
                }
                /*First pair Only*/
                if (i == 0)
                {
                    firstBaseIdx = mllPairIdx;
                }
            }
            mllPairNextIdx = 0;

            retVal = xpsL2McMLLPairWrite(devId, vniCtx,
                                         CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E,
                                         firstNodeIntfType,
                                         secondNodeIntfType,
                                         firstNodeEgrId,
                                         secondNodeEgrId,
                                         firstNodeMeshId,
                                         secondNodeMeshId,
                                         mllPairIdx, mllPairNextIdx, GT_TRUE);
            if (retVal != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "FirstOnly xpsL2McMLLPairWrite Failed : %d\n", retVal);
                return retVal;
            }
        }
    }
    vniCtx->hwL2MllPairIdx = firstBaseIdx;
    return retVal;
}

static XP_STATUS xpsVxlanUpdateL2MLL(xpsDevice_t devId, xpsVniDbEntry_t *vniCtx)
{
    XP_STATUS xpsRetVal = XP_NO_ERR;
    GT_STATUS rc = GT_OK;
    uint32_t prevBaseIdx = 0;
    uint32_t maxVidxIndex = 0;

    prevBaseIdx = vniCtx->hwL2MllPairIdx;
    xpsRetVal = xpsVxlanAddMLLPairInHw(devId, vniCtx);
    if (xpsRetVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Add MLL Pair"
              "vniId(%d) ", vniCtx->vniId);
        return xpsRetVal;
    }

    rc = cpssHalL2MllLookupMaxVidxIndexGet(devId, &maxVidxIndex);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssHalL2MllLookupMaxVidxIndexGet failed");
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    CPSS_DXCH_L2_MLL_LTT_ENTRY_STC l2MllEntry;
    memset(&l2MllEntry, 0, sizeof(CPSS_DXCH_L2_MLL_LTT_ENTRY_STC));
    l2MllEntry.mllPointer = vniCtx->hwL2MllPairIdx;
    rc = cpssHalL2MllLttEntrySet(devId, (vniCtx->hwEVidxId)-(maxVidxIndex+1),
                                 &l2MllEntry);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "lL2Mll failed (%d) evidx :%d ",
              vniCtx->hwL2MllPairIdx, vniCtx->hwEVidxId);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    /* Release Prev MLL list */
    if (prevBaseIdx != 0)
    {
        xpsRetVal = xpsMcReleaseOldL2MllPairIdx(devId, prevBaseIdx);
        if (xpsRetVal != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "xpsMcReleaseOldL2MllPairIdx failed,"
                  "vniId(%d)", vniCtx->vniId);
            return xpsRetVal;
        }
    }

    return xpsRetVal;
}

XP_STATUS xpsVxlanRemoveVlanToVniDbEntry(xpsDevice_t devId, uint32_t vniId,
                                         xpsVxlanMapType_e mapType,
                                         xpsVlan_t vlanId)
{
    XP_STATUS xpsRetVal = XP_NO_ERR;
    GT_STATUS rc = GT_OK;
    xpsVniDbEntry_t *vniCtx = NULL;
    xpsInterfaceId_t *intfList  = NULL;
    xpsInterfaceId_t intfId;
    uint16_t numOfIntfs = 0;
    xpsInterfaceType_e intfType;
    GT_U32     cpssDevNum;
    GT_U32     cpssPortNum;
    xpsScope_t scopeId;
    xpsLagPortIntfList_t lagPortList;
    xpsVxlanVniMapDbEntry_t *vniEncapMapCtx = NULL;
    xpsVxlanVniMapDbEntry_t *vniDecapMapCtx = NULL;
    xpsIpTnlGblDbEntry_t *lookupEntry = NULL;

    /* Get Scope Id from devId */
    if ((xpsRetVal = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return xpsRetVal;
    }

    /* VLAN can be mapped in Encap (VLN-to-VNI) or
       in Decap (VNI-to-VLAN).
       Check if Vlan is already added to VNI */
    xpsRetVal = xpsVniGetDbEntry(scopeId, vniId, &vniCtx);
    if (xpsRetVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get VNI Context Db failed, vni(%d)", vniId);
        return xpsRetVal;
    }

    xpsRetVal = xpsVniMapGetDbEntry(scopeId, vniId, VXLAN_MAP_VLAN_TO_VNI, vlanId,
                                    &vniEncapMapCtx);
    if (xpsRetVal != XP_NO_ERR && xpsRetVal != XP_ERR_NOT_FOUND)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get VNI Encap MAP Context Db failed, vni(%d)", vniId);
        return xpsRetVal;
    }

    xpsRetVal = xpsVniMapGetDbEntry(scopeId, vniId, VXLAN_MAP_VNI_TO_VLAN, vlanId,
                                    &vniDecapMapCtx);
    if (xpsRetVal != XP_NO_ERR && xpsRetVal != XP_ERR_NOT_FOUND)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get VNI Encap MAP Context Db failed, vni(%d)", vniId);
        return xpsRetVal;
    }

    if (mapType == VXLAN_MAP_VLAN_TO_VNI)
    {
        if (!vniEncapMapCtx)
        {
            return XP_ERR_INVALID_INPUT;
        }
        if (vniDecapMapCtx)
        {
            /* Delete Encap ctx */
            xpsRetVal = xpsVniMapDeleteDbEntry(scopeId, vniId, mapType, vlanId);
            if (xpsRetVal != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to create VNI Map DB entry\n");
                return xpsRetVal;
            }
            vniCtx->refCnt--;
            return XP_ERR_RESOURCE_BUSY;
        }
    }
    else if (mapType == VXLAN_MAP_VNI_TO_VLAN)
    {
        if (!vniDecapMapCtx)
        {
            return XP_ERR_INVALID_INPUT;
        }
        if (vniEncapMapCtx)
        {
            /* Delete Decap ctx */
            xpsRetVal = xpsVniMapDeleteDbEntry(scopeId, vniId, mapType, vlanId);
            if (xpsRetVal != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to create VNI Map DB entry\n");
                return xpsRetVal;
            }
            vniCtx->refCnt--;
            return XP_ERR_RESOURCE_BUSY;
        }
    }

    xpsRetVal = xpsVlanGetInterfaceList(devId, vlanId, &intfList, &numOfIntfs);
    if (xpsRetVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "failed to get interface list of vlan :%d error:%d \n", vlanId, xpsRetVal);
        return xpsRetVal;
    }

    /* Delete Encap/Decap ctx */
    xpsRetVal = xpsVniMapDeleteDbEntry(scopeId, vniId, mapType, vlanId);
    if (xpsRetVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to create VNI Map DB entry\n");
        return xpsRetVal;
    }

    /* Remove Members from EVlan */
    for (int i =0; i<numOfIntfs; i++)
    {
        memset(&lagPortList, 0, sizeof(lagPortList));
        //Get interfacetype
        intfId = intfList[i];
        xpsRetVal = xpsInterfaceGetTypeScope(devId, intfId, &intfType);
        if (xpsRetVal != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Interface Get type for interface(%d) failed", intfId);
            return xpsRetVal;
        }

        if (intfType == XPS_PORT)
        {
            lagPortList.portIntf[0] = intfId;
            lagPortList.size = 1;
        }
        else if (intfType == XPS_LAG)
        {
            if ((xpsRetVal = xpsLagGetPortIntfList(intfId, &lagPortList)) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Get ports failed, lag interface(%d)", intfId);
                return xpsRetVal;
            }
        }
#if 0
        else if (intfType == XPS_TUNNEL_VXLAN)
        {
            /*Port list size can be 0,when all SVI members are removed*/
            xpsRetVal = xpsVxlanTunnelGetPortIntfList(devId, intfId, &lagPortList);
            if ((xpsRetVal != XP_NO_ERR))
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Get ports failed,tnlId (%d)", intfId);
                return xpsRetVal;
            }
        }
#endif
        for (uint32_t j = 0; j < lagPortList.size; j++)
        {
            cpssDevNum = xpsGlobalIdToDevId(devId, lagPortList.portIntf[j]);
            cpssPortNum = xpsGlobalPortToPortnum(devId, lagPortList.portIntf[j]);
            if (intfType != XPS_TUNNEL_VXLAN)
            {
                rc = cpssHalBrgVlanPortDelete(cpssDevNum, vniCtx->hwEVlanId,
                                              cpssPortNum);
                if (rc != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "eVlan member add failed (%d)", vniCtx->hwEVlanId);
                    return xpsConvertCpssStatusToXPStatus(rc);
                }
                rc = cpssHalTtiPortLookupEnableSet(cpssDevNum, cpssPortNum,
                                                   CPSS_DXCH_TTI_KEY_ETH_E, GT_FALSE);
                if (rc != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "cpssHalTtiPortLookupEnableSet failed err: %d \n ", rc);
                    return xpsConvertCpssStatusToXPStatus(rc);
                }
                rc = cpssHalVlanPortTranslationEnableSet(cpssDevNum, cpssPortNum,
                                                         CPSS_DIRECTION_EGRESS_E,
                                                         CPSS_DXCH_BRG_VLAN_TRANSLATION_DISABLE_E);
                if (rc != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "cpssHalVlanPortTranslationEnableSet failed port (%d)err: %d \n ",
                          cpssPortNum, rc);
                    return xpsConvertCpssStatusToXPStatus(rc);
                }
            }
        }

        if (intfType != XPS_TUNNEL_VXLAN)
        {
            xpsVniMember_t memberInfo;
            memset(&memberInfo, 0, sizeof(xpsVniMember_t));
            xpsRetVal = xpsVxlanGetMemberInfo(scopeId, vniId, intfId, &memberInfo);
            if (xpsRetVal != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Get VxlanMember failed"
                      "vniId(%d) interface(%d)", vniId, intfId);
                return xpsRetVal;
            }

            xpsRetVal = xpsVxlanDeleteUNIRule(devId, memberInfo.ttiRuleId);
            if (xpsRetVal != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "TTI UNI RuleDel failed"
                      "vniId(%d) interface(%d)", vniId, intfId);
                return xpsRetVal;
            }
            xpsRetVal = xpsVxlanRemoveIntfDb(scopeId, vniId, intfId, &vniCtx);
            if (xpsRetVal != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Remove interface db failed"
                      "vniId(%d) interface(%d)", vniId, intfId);
                return xpsRetVal;
            }
        }
        else
        {
            xpsRetVal = xpsIpTunnelGblGetDbEntry(scopeId, intfId, &lookupEntry);
            if (xpsRetVal != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Vxlan Tunnel Not found :%d \n", intfId);
                return xpsRetVal;
            }

            /* Iterate Tunnel EPort list and update the VNI member Info*/
            for (uint32_t i = 0; i < lookupEntry->numOfEports; i++)
            {
                rc = cpssHalTunnelSetEPortMeshId(devId, lookupEntry->ePorts[i].ePort, 0);
                if (rc != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Failed to set meshId for port %d\n",
                          lookupEntry->ePorts[i].ePort);
                    return xpsConvertCpssStatusToXPStatus(rc);
                }
            }
            xpsRetVal = xpsVxlanRemoveIntfDb(scopeId, vniId, lookupEntry->primaryEport,
                                             &vniCtx);
            if (xpsRetVal != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Remove interface db failed"
                      "vniId(%d) interface(%d)", vniId, lookupEntry->primaryEport);
                return xpsRetVal;
            }
        }
    }

    xpsRetVal = xpsVxlanUpdateL2MLL(devId, vniCtx);
    if (xpsRetVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsVxlanUpdateL2MLL Failed (%d)", xpsRetVal);
        return xpsRetVal;
    }

    /* Reset egress vlan translation for eVlan
       Mutiple vlans cannot be mapped same VNI,
       on egress, vlan tagging is not configurable for multiple vlans*/
    rc = cpssHalVlanTranslationEntryWrite(devId, vniCtx->hwEVlanId,
                                          CPSS_DIRECTION_EGRESS_E, 0);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Egr Vlan trans failed (%d) vlanId :%d ",
              vniCtx->hwEVlanId, vlanId);
        return xpsConvertCpssStatusToXPStatus(rc);
    }
    xpsRetVal = xpsVlanSetVniInVlanDb(scopeId, vlanId, 0);
    if (xpsRetVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsVlanSetVniInVlanDb failed,"
              "vniId(%d) Vid :%d", vniId, vlanId);
        return xpsRetVal;
    }
    vniCtx->refCnt--;

    return xpsRetVal;
}

static XP_STATUS xpsVxlanAddUNIRule(xpsDevice_t devId,
                                    xpsVlan_t vlanId,
                                    xpsInterfaceId_t memberIntfId,
                                    xpsInterfaceType_e memberIntfType,
                                    xpsL2EncapType_e tagType,
                                    uint32_t hwEVlanId,
                                    uint32_t *ttiRuleId)
{
    XPS_FUNC_ENTRY_LOG();

    GT_STATUS   rc = GT_OK;
    XP_STATUS rt = XP_NO_ERR;
    xpsScope_t scopeId;

    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC       TcamRuleType;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ID             RuleId = 0XFFFFFFFF;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC RuleAttributes;
    CPSS_DXCH_TTI_ACTION_STC                   ttiAction;
    CPSS_DXCH_TTI_RULE_UNT                     ttiMask;
    CPSS_DXCH_TTI_RULE_UNT                     ttiPattern;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC       RuleData;
    XPS_LOCK(xpsVxlanAddUNIRule);

    if ((rt = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return rt;
    }

    /* Logical Index based TCam */
    RuleAttributes.priority          = 0; /* Not be used */
    RuleData.valid                   = GT_TRUE;
    TcamRuleType.ruleType = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_TTI_E;

    cpssOsMemSet(&ttiMask, 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
    cpssOsMemSet(&ttiPattern, 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
    cpssOsMemSet(&ttiAction, 0, sizeof(CPSS_DXCH_TTI_ACTION_STC));


    TcamRuleType.rule.tti.ruleFormat = CPSS_DXCH_TTI_RULE_ETH_E;

    ttiPattern.eth.common.pclId  = CPSS_HAL_TTI_UDB_UNI_VXLAN_PCL_ID;

    if (memberIntfType == XPS_LAG)
    {
        ttiPattern.eth.common.srcIsTrunk = GT_TRUE;
        ttiPattern.eth.common.srcPortTrunk = xpsUtilXpstoCpssInterfaceConvert(
                                                 memberIntfId, XPS_LAG);
        ttiMask.eth.common.srcIsTrunk = GT_TRUE;
        ttiMask.eth.common.srcPortTrunk = 0xFF;
    }
    else
    {
        ttiPattern.eth.common.srcPortTrunk = memberIntfId;
        ttiMask.eth.common.srcPortTrunk = 0xFF;
    }

    if (vlanId)
    {
        ttiPattern.eth.common.vid = vlanId;
        ttiMask.eth.common.vid = 0x1FFF;
        if (tagType == XP_L2_ENCAP_DOT1Q_TAGGED)
        {
            ttiPattern.eth.common.isTagged = GT_TRUE;
            ttiMask.eth.common.isTagged = GT_TRUE;
        }
    }

    /* Accept only NOT mac2me pkts. */
    ttiPattern.eth.macToMe = GT_FALSE;
    ttiMask.eth.macToMe = GT_TRUE;

    ttiAction.command                       = CPSS_PACKET_CMD_FORWARD_E;
    ttiAction.redirectCommand               = CPSS_DXCH_TTI_NO_REDIRECT_E;
    ttiAction.userDefinedCpuCode            = CPSS_NET_FIRST_USER_DEFINED_E;
    ttiAction.tag0VlanCmd                   = CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E;
    ttiAction.tag0VlanId                       = hwEVlanId;
    ttiAction.tag1VlanCmd                   = CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E;

    ttiAction.keepPreviousQoS               = GT_TRUE;;
    ttiAction.modifyDscp                    =
        CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_DSCP_E;

    RuleData.rule.tti.actionPtr     = &ttiAction;
    RuleData.rule.tti.maskPtr       = &ttiMask;
    RuleData.rule.tti.patternPtr    = &ttiPattern;

    rt = xpsAllocatorAllocateId(scopeId,
                                XP_ALLOC_TUNNEL_TERM_HW_ENTRY/*XP_ALLOC_VXLAN_UNI_TTI_HW_ENTRY*/,
                                &RuleId);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsAllocatorAllocateId: Unable to allocate HW Tunnel id, error: %d.\n", rt);
        return rt;
    }

    if (RuleId >= curMaxTnlTermEntries)
    {
        rc = cpssHalVirtualTcamResize(XPS_GLOBAL_TACM_MGR, UNI_TERM_TABLE_ID,
                                      CPSS_DXCH_VIRTUAL_TCAM_APPEND_CNS, GT_TRUE, 128);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssHalVirtualTcamResize failed err: %d ruleId :%d \n ", rc, RuleId);
            return xpsConvertCpssStatusToXPStatus(rc);
        }
        curMaxTnlTermEntries += 128;
    }

    rc = cpssHalTunnelTerminationEntryAdd(XPS_GLOBAL_TACM_MGR, UNI_TERM_TABLE_ID,
                                          RuleId, &RuleAttributes,
                                          &TcamRuleType, &RuleData);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssHalTunnelTerminationEntryAdd failed err: %d ruleId :%d \n ", rc,
              RuleId);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    *ttiRuleId = RuleId;

    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEFAULT,
          "Vxlan UNI TTI Success %d RuleId :%d \n", memberIntfId, RuleId);

    XPS_FUNC_EXIT_LOG();
    return rt;
}

XP_STATUS xpsVxlanRemoveMemberToVniDbEntry(xpsDevice_t devId, uint32_t vniId,
                                           xpsVlan_t vlanId, xpsInterfaceId_t intfId)
{
    XP_STATUS xpsRetVal = XP_NO_ERR;
    GT_STATUS rc = GT_OK;
    xpsVniDbEntry_t *vniCtx = NULL;
    xpsInterfaceType_e intfType;
    GT_U32     cpssDevNum;
    GT_U32     cpssPortNum;
    xpsScope_t scopeId;
    xpsLagPortIntfList_t lagPortList;
    xpsIpTnlGblDbEntry_t *lookupEntry = NULL;
    XPS_LOCK(xpsVxlanRemoveMemberToVniDbEntry);

    /* Get Scope Id from devId */
    if ((xpsRetVal = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return xpsRetVal;
    }

    /* VLAN can be mapped in Encap (VLN-to-VNI) or
       in Decap (VNI-to-VLAN).
       Check if Vlan is already added to VNI */
    xpsRetVal = xpsVniGetDbEntry(scopeId, vniId, &vniCtx);
    if (xpsRetVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get VNI Context Db failed, vni(%d)", vniId);
        return xpsRetVal;
    }

    memset(&lagPortList, 0, sizeof(lagPortList));
    //Get interfacetype
    xpsRetVal = xpsInterfaceGetTypeScope(devId, intfId, &intfType);
    if (xpsRetVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Interface Get type for interface(%d) failed", intfId);
        return xpsRetVal;
    }

    if (intfType == XPS_PORT)
    {
        lagPortList.portIntf[0] = intfId;
        lagPortList.size = 1;
    }
    else if (intfType == XPS_LAG)
    {
        if ((xpsRetVal = xpsLagGetPortIntfList(intfId, &lagPortList)) != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Get ports failed, lag interface(%d)", intfId);
            return xpsRetVal;
        }
    }
#if 0
    else if (intfType == XPS_TUNNEL_VXLAN)
    {
        /*Port list size can be 0,when all SVI members are removed*/
        xpsRetVal = xpsVxlanTunnelGetPortIntfList(devId, intfId, &lagPortList);
        if ((xpsRetVal != XP_NO_ERR))
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Get ports failed,tnlId (%d)", intfId);
            return xpsRetVal;
        }
    }
#endif
    for (uint32_t j = 0; j < lagPortList.size; j++)
    {
        cpssDevNum = xpsGlobalIdToDevId(devId, lagPortList.portIntf[j]);
        cpssPortNum = xpsGlobalPortToPortnum(devId, lagPortList.portIntf[j]);

        if (intfType != XPS_TUNNEL_VXLAN)
        {
            //Tunnel ports are removed during tunnel delete.
            rc = cpssHalBrgVlanPortDelete(cpssDevNum, vniCtx->hwEVlanId,
                                          cpssPortNum);
            if (rc != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "eVlan member add failed (%d)", vniCtx->hwEVlanId);
                return xpsConvertCpssStatusToXPStatus(rc);
            }
            rc = cpssHalTtiPortLookupEnableSet(cpssDevNum, cpssPortNum,
                                               CPSS_DXCH_TTI_KEY_ETH_E, GT_FALSE);
            if (rc != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssHalTtiPortLookupEnableSet failed err: %d \n ", rc);
                return xpsConvertCpssStatusToXPStatus(rc);
            }
            rc = cpssHalVlanPortTranslationEnableSet(cpssDevNum, cpssPortNum,
                                                     CPSS_DIRECTION_EGRESS_E,
                                                     CPSS_DXCH_BRG_VLAN_TRANSLATION_DISABLE_E);
            if (rc != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssHalVlanPortTranslationEnableSet failed port (%d)err: %d \n ", cpssPortNum,
                      rc);
                return xpsConvertCpssStatusToXPStatus(rc);
            }
        }
    }

    if (intfType != XPS_TUNNEL_VXLAN)
    {
        xpsVniMember_t memberInfo;
        memset(&memberInfo, 0, sizeof(xpsVniMember_t));
        xpsRetVal = xpsVxlanGetMemberInfo(scopeId, vniId, intfId, &memberInfo);
        if (xpsRetVal != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Get VxlanMember failed"
                  "vniId(%d) interface(%d)", vniId, intfId);
            return xpsRetVal;
        }

        xpsRetVal = xpsVxlanDeleteUNIRule(devId, memberInfo.ttiRuleId);
        if (xpsRetVal != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "TTI UNI RuleDel failed"
                  "vniId(%d) interface(%d)", vniId, intfId);
            return xpsRetVal;
        }
        xpsRetVal = xpsVxlanRemoveIntfDb(scopeId, vniId, intfId, &vniCtx);
        if (xpsRetVal != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Remove interface db failed"
                  "vniId(%d) interface(%d)", vniId, intfId);
            return xpsRetVal;
        }
    }
    else
    {
        xpsRetVal = xpsIpTunnelGblGetDbEntry(scopeId, intfId, &lookupEntry);
        if (xpsRetVal != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Vxlan Tunnel Not found :%d \n", intfId);
            return xpsRetVal;
        }

        /* Iterate Tunnel EPort list and update the VNI member Info*/
        for (uint32_t i = 0; i < lookupEntry->numOfEports; i++)
        {
            rc = cpssHalTunnelSetEPortMeshId(devId, lookupEntry->ePorts[i].ePort, 0);
            if (rc != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to set meshId for port %d\n",
                      lookupEntry->ePorts[i].ePort);
                return xpsConvertCpssStatusToXPStatus(rc);
            }
        }
        xpsRetVal = xpsVxlanRemoveIntfDb(scopeId, vniId, lookupEntry->primaryEport,
                                         &vniCtx);
        if (xpsRetVal != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Remove interface db failed"
                  "vniId(%d) interface(%d)", vniId, lookupEntry->primaryEport);
            return xpsRetVal;
        }
    }

    xpsRetVal = xpsVxlanUpdateL2MLL(devId, vniCtx);
    if (xpsRetVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsVxlanUpdateL2MLL Failed (%d)", xpsRetVal);
        return xpsRetVal;
    }

    return xpsRetVal;
}

XP_STATUS xpsVxlanAddMemberToVniDbEntry(xpsDevice_t devId, uint32_t vniId,
                                        xpsVlan_t vlanId, xpsInterfaceId_t intfId)
{
    XP_STATUS xpsRetVal = XP_NO_ERR;
    GT_STATUS rc = GT_OK;
    xpsVniDbEntry_t *vniCtx = NULL;
    xpsInterfaceType_e intfType;
    xpsLagPortIntfList_t lagPortList;
    xpsScope_t scopeId;
    uint32_t ttiRuleId = 0xFFFFFFFF;
    xpsL2EncapType_e tagType = XP_L2_ENCAP_MAX;
    xpsIpTnlGblDbEntry_t *lookupEntry = NULL;

    GT_U32     cpssDevNum;
    GT_U32     cpssPortNum;
    CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT cpssTagType =
        CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E;
    GT_BOOL tag = GT_FALSE;

    XPS_LOCK(xpsVxlanAddMemberToVniDbEntry);

    memset(&lagPortList, 0, sizeof(lagPortList));

    //get scope from Device ID
    xpsRetVal = xpsScopeGetScopeId(devId, &scopeId);
    if (xpsRetVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Getting scope id failed");
        return xpsRetVal;
    }

    /* VLAN can be mapped in Encap (VLN-to-VNI) or
       in Decap (VNI-to-VLAN).
       Check if Vlan is already added to VNI */
    xpsRetVal = xpsVniGetDbEntry(scopeId, vniId, &vniCtx);
    if (xpsRetVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get VNI Context Db failed, vni(%d)", vniId);
        return xpsRetVal;
    }

    //Get interfacetype
    xpsRetVal = xpsInterfaceGetTypeScope(devId, intfId, &intfType);
    if (xpsRetVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Interface Get type for interface(%d) failed", intfId);
        return xpsRetVal;
    }

    if (intfType != XPS_TUNNEL_VXLAN)
    {
        if ((xpsRetVal = xpsVlanGetIntfTagTypeScope(scopeId, vlanId,
                                                    intfId, &tagType)) != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Get tag type scope failed with xps error code %d", xpsRetVal);
            return xpsRetVal;
        }

        /*Add TTI rule to qualify source port and assign eVlan.*/
        xpsRetVal = xpsVxlanAddUNIRule(devId, vlanId, intfId,
                                       intfType, tagType,
                                       vniCtx->hwEVlanId, &(ttiRuleId));
        if (xpsRetVal != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "UNI TTI Rule failed"
                  "vniId(%d) interface(%d)", vniId, intfId);
            return xpsRetVal;
        }
        xpsRetVal = xpsVxlanAddIntfDb(scopeId, vniId, intfId, intfType, ttiRuleId,
                                      &vniCtx);
        if (xpsRetVal != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Add interface db failed"
                  "vniId(%d) interface(%d)", vniId, intfId);
            return xpsRetVal;
        }
    }
    else
    {
        xpsRetVal = xpsIpTunnelGblGetDbEntry(scopeId, intfId, &lookupEntry);
        if (xpsRetVal != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Vxlan Tunnel Not found :%d \n", intfId);
            return xpsRetVal;
        }
        /* Iterate Tunnel EPort list and update the VNI member Info*/
        for (uint32_t i = 0; i < lookupEntry->numOfEports; i++)
        {
            rc = cpssHalTunnelSetEPortMeshId(devId, lookupEntry->ePorts[i].ePort, 1);
            if (rc != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to set meshId for port %d\n",
                      lookupEntry->ePorts[i].ePort);
                return xpsConvertCpssStatusToXPStatus(rc);
            }
        }
        xpsRetVal = xpsVxlanAddIntfDb(scopeId, vniId, lookupEntry->primaryEport,
                                      intfType, ttiRuleId,
                                      &vniCtx);
        if (xpsRetVal != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Add interface db failed"
                  "vniId(%d) interface(%d)", vniId, lookupEntry->primaryEport);
            return xpsRetVal;
        }
    }

    if (intfType == XPS_PORT)
    {
        lagPortList.portIntf[0] = intfId;
        lagPortList.size = 1;
    }
    else if (intfType == XPS_LAG)
    {
        if ((xpsRetVal = xpsLagGetPortIntfList(intfId, &lagPortList)) != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Get ports failed, lag interface(%d)", intfId);
            return xpsRetVal;
        }
    }
#if 0
    else if (intfType == XPS_TUNNEL_VXLAN)
    {
        xpsRetVal = xpsVxlanTunnelGetPortIntfList(devId, intfId, &lagPortList);

        if ((xpsRetVal != XP_NO_ERR) || (lagPortList.size == 0))
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Get ports failed,tnlId (%d)", intfId);
            return xpsRetVal;
        }
    }
#endif
    for (uint32_t j = 0; j < lagPortList.size; j++)
    {
        cpssDevNum = xpsGlobalIdToDevId(devId, lagPortList.portIntf[j]);
        cpssPortNum = xpsGlobalPortToPortnum(devId, lagPortList.portIntf[j]);

        if (intfType != XPS_TUNNEL_VXLAN)
        {
            if (tagType == XP_L2_ENCAP_DOT1Q_TAGGED)
            {
                cpssTagType = CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E;
                tag = GT_TRUE;
            }

            rc = cpssHalVlanMemberAdd(cpssDevNum, vniCtx->hwEVlanId,
                                      cpssPortNum, tag,
                                      cpssTagType);
            if (rc != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "eVlan member add failed (%d)", vniCtx->hwEVlanId);
                return xpsConvertCpssStatusToXPStatus(rc);
            }
            rc = cpssHalTtiPortLookupEnableSet(cpssDevNum, cpssPortNum,
                                               CPSS_DXCH_TTI_KEY_ETH_E, GT_TRUE);
            if (rc != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssHalTtiPortLookupEnableSet failed err: %d \n ", rc);
                return xpsConvertCpssStatusToXPStatus(rc);
            }

            rc = cpssHalVlanPortTranslationEnableSet(cpssDevNum, cpssPortNum,
                                                     CPSS_DIRECTION_EGRESS_E,
                                                     CPSS_DXCH_BRG_VLAN_TRANSLATION_VID0_E);
            if (rc != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssHalVlanPortTranslationEnableSet failed port (%d)err: %d \n ", cpssPortNum,
                      rc);
                return xpsConvertCpssStatusToXPStatus(rc);
            }
        }
    }

    xpsRetVal = xpsVxlanUpdateL2MLL(devId, vniCtx);
    if (xpsRetVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsVxlanUpdateL2MLL Failed (%d)", xpsRetVal);
        return xpsRetVal;
    }

    return xpsRetVal;
}

XP_STATUS xpsVxlanAddVlanToVniDbEntry(xpsDevice_t devId, uint32_t vniId,
                                      xpsVxlanMapType_e mapType,
                                      xpsVlan_t vlanId)
{
    XP_STATUS xpsRetVal = XP_NO_ERR;
    GT_STATUS rc = GT_OK;
    xpsVniDbEntry_t *vniCtx = NULL;
    xpsInterfaceId_t *intfList  = NULL;
    xpsVxlanVniMapDbEntry_t *vniEncapMapCtx = NULL;
    xpsVxlanVniMapDbEntry_t *vniDecapMapCtx = NULL;
    xpsVxlanVniMapDbEntry_t *dbEntry = NULL;
    xpsInterfaceId_t intfId;
    uint16_t numOfIntfs = 0;
    xpsInterfaceType_e intfType;
    xpsLagPortIntfList_t lagPortList;
    xpsScope_t scopeId;
    uint32_t ttiRuleId = 0XFFFFFFFF;
    xpsIpTnlGblDbEntry_t *lookupEntry = NULL;

    GT_U32     cpssDevNum;
    GT_U32     cpssPortNum;
    CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT cpssTagType =
        CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E;
    GT_BOOL tag = GT_FALSE;

    XPS_LOCK(xpsVxlanAddVlanToVniDbEntry);

    //get scope from Device ID
    xpsRetVal = xpsScopeGetScopeId(devId, &scopeId);
    if (xpsRetVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Getting scope id failed");
        return xpsRetVal;
    }

    /* VLAN can be mapped in Encap (VLN-to-VNI) or
       in Decap (VNI-to-VLAN).
       Check if Vlan is already added to VNI */
    xpsRetVal = xpsVniGetDbEntry(scopeId, vniId, &vniCtx);
    if (xpsRetVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get VNI Context Db failed, vni(%d)", vniId);
        return xpsRetVal;
    }

    xpsRetVal = xpsVniMapGetDbEntry(scopeId, vniId, VXLAN_MAP_VLAN_TO_VNI, vlanId,
                                    &vniEncapMapCtx);
    if (xpsRetVal != XP_NO_ERR && xpsRetVal != XP_ERR_NOT_FOUND)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get VNI Encap MAP Context Db failed, vni(%d)", vniId);
        return xpsRetVal;
    }

    xpsRetVal = xpsVniMapGetDbEntry(scopeId, vniId, VXLAN_MAP_VNI_TO_VLAN, vlanId,
                                    &vniDecapMapCtx);
    if (xpsRetVal != XP_NO_ERR && xpsRetVal != XP_ERR_NOT_FOUND)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get VNI Encap MAP Context Db failed, vni(%d)", vniId);
        return xpsRetVal;
    }
    if (mapType == VXLAN_MAP_VLAN_TO_VNI)
    {
        if (!vniEncapMapCtx && vniDecapMapCtx)
        {
            /* Create and store the state */
            xpsRetVal = xpsVniMapInsertDbEntry(scopeId, vniId, mapType, vlanId, &dbEntry);
            if (xpsRetVal != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to create VNI Map DB entry\n");
                return xpsRetVal;
            }
            vniCtx->refCnt++;
            return XP_ERR_RESOURCE_BUSY;
        }
    }
    else if (mapType == VXLAN_MAP_VNI_TO_VLAN)
    {
        if (!vniDecapMapCtx && vniEncapMapCtx)
        {
            /* Create and store the state */
            xpsRetVal = xpsVniMapInsertDbEntry(scopeId, vniId, mapType, vlanId, &dbEntry);
            if (xpsRetVal != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to create VNI Map DB entry\n");
                return xpsRetVal;
            }
            vniCtx->refCnt++;
            return XP_ERR_RESOURCE_BUSY;
        }
    }

    xpsRetVal = xpsVlanGetInterfaceList(devId, vlanId, &intfList, &numOfIntfs);
    if (xpsRetVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "failed to get interface list of vlan :%d error:%d \n", vlanId, xpsRetVal);
        return xpsRetVal;
    }

    xpsL2EncapType_e tagType = XP_L2_ENCAP_MAX;
    for (int i =0; i<numOfIntfs; i++)
    {
        memset(&lagPortList, 0, sizeof(lagPortList));
        //Get interfacetype
        intfId = intfList[i];
        xpsRetVal = xpsInterfaceGetTypeScope(devId, intfId, &intfType);
        if (xpsRetVal != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Interface Get type for interface(%d) failed", intfId);
            return xpsRetVal;
        }
        if (intfType != XPS_TUNNEL_VXLAN)
        {
            if ((xpsRetVal = xpsVlanGetIntfTagTypeScope(scopeId, vlanId,
                                                        intfId, &tagType)) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Get tag type scope failed with xps error code %d", xpsRetVal);
                return xpsRetVal;
            }

            /*Add TTI rule to qualify source port and assign eVlan.*/
            xpsRetVal = xpsVxlanAddUNIRule(devId, vlanId, intfId,
                                           intfType, tagType,
                                           vniCtx->hwEVlanId, &(ttiRuleId));
            if (xpsRetVal != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "UNI TTI Rule failed"
                      "vniId(%d) interface(%d)", vniId, intfId);
                return xpsRetVal;
            }
            xpsRetVal = xpsVxlanAddIntfDb(scopeId, vniId, intfId, intfType, ttiRuleId,
                                          &vniCtx);
            if (xpsRetVal != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Add interface db failed"
                      "vniId(%d) interface(%d)", vniId, intfId);
                return xpsRetVal;
            }
        }
        else
        {
            //Below case will not hit in current SAI flow, as tunnel is added to
            // Vni when tunnel_bridge_port added to vlan.
            xpsRetVal = xpsIpTunnelGblGetDbEntry(scopeId, intfId, &lookupEntry);
            if (xpsRetVal != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Vxlan Tunnel Not found :%d \n", intfId);
                return xpsRetVal;
            }
            /* Iterate Tunnel EPort list and update the VNI member Info*/
            for (uint32_t i = 0; i < lookupEntry->numOfEports; i++)
            {
                rc = cpssHalTunnelSetEPortMeshId(devId, lookupEntry->ePorts[i].ePort, 1);
                if (rc != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Failed to set meshId for port %d\n",
                          lookupEntry->ePorts[i].ePort);
                    return xpsConvertCpssStatusToXPStatus(rc);
                }
            }
            xpsRetVal = xpsVxlanAddIntfDb(scopeId, vniId, lookupEntry->primaryEport,
                                          intfType, ttiRuleId,
                                          &vniCtx);
            if (xpsRetVal != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Add interface db failed"
                      "vniId(%d) interface(%d)", vniId, lookupEntry->primaryEport);
                return xpsRetVal;
            }
        }

        if (intfType == XPS_PORT)
        {
            lagPortList.portIntf[0] = intfId;
            lagPortList.size = 1;
        }
        else if (intfType == XPS_LAG)
        {
            if ((xpsRetVal = xpsLagGetPortIntfList(intfId, &lagPortList)) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Get ports failed, lag interface(%d)", intfId);
                return xpsRetVal;
            }
        }
#if 0
        else if (intfType == XPS_TUNNEL_VXLAN)
        {
            xpsRetVal = xpsVxlanTunnelGetPortIntfList(devId, intfId, &lagPortList);
            if ((xpsRetVal != XP_NO_ERR) || (lagPortList.size == 0))
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Get ports failed,tnlId (%d)", intfId);
                return xpsRetVal;
            }
        }
#endif
        for (uint32_t j = 0; j < lagPortList.size; j++)
        {
            cpssDevNum = xpsGlobalIdToDevId(devId, lagPortList.portIntf[j]);
            cpssPortNum = xpsGlobalPortToPortnum(devId, lagPortList.portIntf[j]);

            if (intfType != XPS_TUNNEL_VXLAN)
            {
                if (tagType == XP_L2_ENCAP_DOT1Q_TAGGED)
                {
                    cpssTagType = CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E;
                    tag = GT_TRUE;
                }
                rc = cpssHalVlanMemberAdd(cpssDevNum, vniCtx->hwEVlanId,
                                          cpssPortNum, tag,
                                          cpssTagType);
                if (rc != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "eVlan member add failed (%d)", vniCtx->hwEVlanId);
                    return xpsConvertCpssStatusToXPStatus(rc);
                }
                rc = cpssHalTtiPortLookupEnableSet(cpssDevNum, cpssPortNum,
                                                   CPSS_DXCH_TTI_KEY_ETH_E, GT_TRUE);
                if (rc != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "cpssHalTtiPortLookupEnableSet failed err: %d \n ", rc);
                    return xpsConvertCpssStatusToXPStatus(rc);
                }

                rc = cpssHalVlanPortTranslationEnableSet(cpssDevNum, cpssPortNum,
                                                         CPSS_DIRECTION_EGRESS_E,
                                                         CPSS_DXCH_BRG_VLAN_TRANSLATION_VID0_E);
                if (rc != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "cpssHalVlanPortTranslationEnableSet failed port (%d)err: %d \n ",
                          cpssPortNum, rc);
                    return xpsConvertCpssStatusToXPStatus(rc);
                }
            }
        }
    }

    xpsRetVal = xpsVxlanUpdateL2MLL(devId, vniCtx);
    if (xpsRetVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsVxlanUpdateL2MLL Failed (%d)", xpsRetVal);
        return xpsRetVal;
    }

    rc = cpssHalVlanTranslationEntryWrite(devId, vniCtx->hwEVlanId,
                                          CPSS_DIRECTION_EGRESS_E, vlanId);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Egr Vlan trans failed (%d) vlanId :%d ",
              vniCtx->hwEVlanId, vlanId);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    /* Create and store the state */
    xpsRetVal = xpsVniMapInsertDbEntry(scopeId, vniId, mapType, vlanId, &dbEntry);
    if (xpsRetVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to create VNI Map DB entry\n");
        return xpsRetVal;
    }

    xpsRetVal = xpsVlanSetVniInVlanDb(scopeId, vlanId, vniId);
    if (xpsRetVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsVlanSetVniInVlanDb failed,"
              "vniId(%d) Vid :%d", vniId, vlanId);
        return xpsRetVal;
    }

    vniCtx->refCnt++;
    return xpsRetVal;
}

XP_STATUS xpsVxlanVniCreateDbEntry(xpsDevice_t devId, uint32_t vniId)
{
    XP_STATUS xpsRetVal = XP_NO_ERR;
    xpsVniDbEntry_t *dbEntry = NULL;
    xpsScope_t scopeId;

    XPS_LOCK(xpsVxlanVniCreateDbEntry);

    //get scope from Device ID
    xpsRetVal = xpsScopeGetScopeId(devId, &scopeId);
    if (xpsRetVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Getting scope id failed");
        return xpsRetVal;
    }

    /* Create and store the state */
    xpsRetVal = xpsVniInsertDbEntry(scopeId, vniId, &dbEntry);
    if (xpsRetVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to create VNI DB entry\n");
        return xpsRetVal;
    }

    return xpsRetVal;
}

XP_STATUS xpsVxlanVniDeleteDbEntry(xpsDevice_t devId, uint32_t vniId)
{
    XP_STATUS xpsRetVal = XP_NO_ERR;
    xpsVniDbEntry_t *vniCtx = NULL;
    xpsScope_t scopeId;

    XPS_LOCK(xpsVxlanVniDeleteDbEntry);

    //get scope from Device ID
    xpsRetVal = xpsScopeGetScopeId(devId, &scopeId);
    if (xpsRetVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Getting scope id failed");
        return xpsRetVal;
    }

    xpsRetVal = xpsVniGetDbEntry(scopeId, vniId, &vniCtx);
    if (xpsRetVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get VNI Context Db failed, vni(%d)", vniId);
        return xpsRetVal;
    }

    /*Make sure all encap and decap mappers are deleted */
    if (vniCtx->refCnt)
    {
        return XP_ERR_NOT_SUPPORTED;
    }

    xpsRetVal = xpsVniDeleteDbEntry(scopeId, vniId);
    if (xpsRetVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Delete the vni context failed,"
              "vniId(%d)", vniId);
        return xpsRetVal;
    }

    return xpsRetVal;
}

XP_STATUS xpsVxlanVniDeleteInHw(xpsDevice_t devId, uint32_t vniId,
                                uint32_t hwEVlanId,
                                uint32_t hwEVidxId, bool isL3Vni)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;
    xpsScope_t scopeId=0;

    CPSS_PORTS_BMP_STC  portsMembers;
    CPSS_PORTS_BMP_STC  portsTagging;
    GT_STATUS           rc = GT_OK;
    CPSS_DXCH_BRG_VLAN_INFO_STC  cpss_vlan_info;
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC ports_tagging_cmd;
    XPS_LOCK(xpsVxlanVniDeleteInHw);

    memset(&cpss_vlan_info, 0, sizeof(cpss_vlan_info));
    memset(&portsMembers, 0, sizeof(CPSS_PORTS_BMP_STC));
    memset(&portsTagging, 0, sizeof(CPSS_PORTS_BMP_STC));
    memset(&ports_tagging_cmd, 0, sizeof(CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC));

    if ((rc = cpssHalWriteBrgVlanEntry(devId, hwEVlanId, &portsMembers,
                                       &portsTagging, &cpss_vlan_info, &ports_tagging_cmd)) != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "eVlan %d create failed",
              hwEVlanId);


        return xpsConvertCpssStatusToXPStatus(rc);
    }

    rc = cpssHalInvalidateBrgVlanEntry(devId, hwEVlanId);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Destroy vlan failed :%d ",
              hwEVlanId);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    status = xpsAllocatorReleaseId(scopeId, XP_ALLOC_RIF_HW_ENTRY, hwEVlanId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsAllocatorReleaseId Failed : %d\n", status);
        return status;
    }
    status = xpsAllocatorReleaseId(scopeId, XP_ALLOC_MC_EVIDX_HW_TBL,
                                   hwEVidxId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsAllocatorReleaseId Failed : %d\n", status);
        return status;
    }

    if (isL3Vni)
    {
        rc = cpssDxChBrgVlanIpUcRouteEnable(devId, hwEVlanId,
                                            CPSS_IP_PROTOCOL_IPV4V6_E,
                                            GT_FALSE);
        if (xpsConvertCpssStatusToXPStatus(rc) != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "enabling routing on vlan failed ");
            return xpsConvertCpssStatusToXPStatus(rc);
        }

        rc = cpssHalBrgVlanVrfIdSet(devId, hwEVlanId, 0);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssHalBrgVlanVrfIdSet Failed VlanId %d Vrf %d\n", hwEVlanId, 0);
            return xpsConvertCpssStatusToXPStatus(rc);
        }
    }

    XPS_FUNC_EXIT_LOG();
    return status;
}

XP_STATUS xpsVxlanVniCreateInHw(xpsDevice_t devId, uint32_t vniId,
                                uint32_t *hwEVlanId,
                                uint32_t *hwEVidxId, bool isL3Vni, uint32_t vrfId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;
    xpsScope_t scopeId=0;

    CPSS_PORTS_BMP_STC  portsMembers;
    CPSS_PORTS_BMP_STC  portsTagging;
    GT_STATUS           rc = GT_OK;
    CPSS_DXCH_BRG_VLAN_INFO_STC  cpss_vlan_info;
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC ports_tagging_cmd;
    GT_STATUS cpss_status = GT_OK;

    XPS_LOCK(xpsVxlanVniCreateInHw);

    memset(&cpss_vlan_info, 0, sizeof(cpss_vlan_info));
    memset(&portsMembers, 0, sizeof(CPSS_PORTS_BMP_STC));
    memset(&portsTagging, 0, sizeof(CPSS_PORTS_BMP_STC));
    memset(&ports_tagging_cmd, 0, sizeof(CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC));

    /*Allocate EVidx*/
    status = xpsAllocatorAllocateId(scopeId, XP_ALLOC_MC_EVIDX_HW_TBL,
                                    hwEVidxId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsAllocatorAllocateId Failed : %d\n", status);
        return status;
    }

    /*Allocate eVlan (VSI) */
    status = xpsAllocatorAllocateId(scopeId, XP_ALLOC_RIF_HW_ENTRY, hwEVlanId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsAllocatorAllocateId Failed : %d\n", status);
        return status;
    }

    cpss_vlan_info.naMsgToCpuEn = GT_TRUE;
    cpss_vlan_info.floodVidx = *hwEVidxId;
    cpss_vlan_info.floodVidxMode =
        CPSS_DXCH_BRG_VLAN_FLOOD_VIDX_MODE_ALL_FLOODED_TRAFFIC_E;
    cpss_vlan_info.fidValue = *hwEVlanId;

    if ((rc = cpssHalWriteBrgVlanEntry(devId, *hwEVlanId, &portsMembers,
                                       &portsTagging, &cpss_vlan_info, &ports_tagging_cmd)) != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "eVlan %d create failed",
              *hwEVlanId);

        status = xpsAllocatorReleaseId(scopeId, XP_ALLOC_RIF_HW_ENTRY, *hwEVlanId);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "xpsAllocatorReleaseId Failed : %d\n", status);
            return status;
        }
        status = xpsAllocatorReleaseId(scopeId, XP_ALLOC_MC_EVIDX_HW_TBL,
                                       *hwEVidxId);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "xpsAllocatorReleaseId Failed : %d\n", status);
            return status;
        }

        return xpsConvertCpssStatusToXPStatus(rc);
    }

    rc = cpssHalTunnelVlanToVniMapSet(devId, *hwEVlanId, vniId);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "VxLan Vlan VNI map failed");
        status = xpsAllocatorReleaseId(scopeId, XP_ALLOC_RIF_HW_ENTRY, *hwEVlanId);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "xpsAllocatorReleaseId Failed : %d\n", status);
            return status;
        }
        status = xpsAllocatorReleaseId(scopeId, XP_ALLOC_MC_EVIDX_HW_TBL,
                                       *hwEVidxId);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "xpsAllocatorReleaseId Failed : %d\n", status);
            return status;
        }
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    if (isL3Vni)
    {
        cpss_status = cpssHalBrgVlanIpUcRouteEnable(devId, *hwEVlanId);
        if (xpsConvertCpssStatusToXPStatus(cpss_status) != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "enabling routing on vlan failed ");
            return xpsConvertCpssStatusToXPStatus(cpss_status);
        }

        cpss_status = cpssHalBrgVlanVrfIdSet(devId, *hwEVlanId, vrfId);
        if (cpss_status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssHalBrgVlanVrfIdSet Failed VlanId %d Vrf %d\n", *hwEVlanId, vrfId);
            return xpsConvertCpssStatusToXPStatus(cpss_status);
        }
    }
    XPS_FUNC_EXIT_LOG();
    return status;
}

XP_STATUS xpsVxlanTunnelFindVni(xpsScope_t scopeId, xpsInterfaceId_t tnlIntfId,
                                uint32_t vniId,
                                xpsVxlanVniInfo_t **ppInfo)
{
    XP_STATUS      status   = XP_NO_ERR;
    xpsVxlanVniInfo_t childKey;
    xpsVxlanDbEntry_t *lookupEntry = NULL;

    status = xpsVxlanGetTunnelDb(scopeId, tnlIntfId, &lookupEntry);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Vxlan Tunnel Not found :%d \n", tnlIntfId);
        return status;
    }

    childKey.vniId = vniId;

    *ppInfo = (xpsVxlanVniInfo_t*)xpsRBTSearchNode(lookupEntry->vniList,
                                                   (xpsDbKey_t)&childKey);

    if (*ppInfo == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEFAULT,
              "Ket not found(%d) failed", vniId);
        return XP_ERR_KEY_NOT_FOUND;
    }

    return status;
}

XP_STATUS xpsVxlanTunnelAddToVniList(xpsDevice_t devId,
                                     xpsInterfaceId_t tnlIntfId,
                                     uint32_t vniId)
{
    XP_STATUS           status      = XP_NO_ERR;
    GT_STATUS rc = GT_OK;
    GT_U32     cpssDevNum;
    GT_U32     cpssPortNum;
    xpsVxlanVniInfo_t* vniIdNode = NULL;
    xpsVxlanDbEntry_t *lookupEntry = NULL;
    xpsLagPortIntfList_t portList;
    xpsVniDbEntry_t *vniCtx = NULL;
    xpsScope_t          scopeId;

    status = xpsScopeGetScopeId(devId, &scopeId);
    if (status != XP_NO_ERR)
    {
        return status;
    }

    status = xpsVxlanGetTunnelDb(scopeId, tnlIntfId, &lookupEntry);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Vxlan Tunnel Not found :%d \n", tnlIntfId);
        return status;
    }

    status = xpsVxlanTunnelFindVni(scopeId, lookupEntry->tnlIntfId, vniId,
                                   &vniIdNode);
    if (status == XP_NO_ERR && vniIdNode)
    {
        vniIdNode->refCnt++;
        return XP_NO_ERR;
    }

    vniIdNode = NULL;
    // Create a new child Interface Info structure
    if ((status = xpsStateHeapMalloc(sizeof(xpsVxlanVniInfo_t),
                                     (void**)&vniIdNode)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Could not allocate interface(%d) failed", vniId);
        return status;
    }

    memset(vniIdNode, 0, sizeof(xpsVxlanVniInfo_t));
    vniIdNode->vniId = vniId;

    // Add a new node
    status = xpsRBTAddNode(lookupEntry->vniList, (void*)vniIdNode);
    if ((status != XP_NO_ERR))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Add node failedi :%d ",
              status);
        return status;
    }

    vniIdNode->refCnt++;

    status = xpsVniGetDbEntry(scopeId, vniId, &vniCtx);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get VNI Context Db failed, vni(%d)", vniId);
        return status;
    }
    status = xpsTunnelGetPortIntfList(devId, tnlIntfId, &portList);
    if ((status != XP_NO_ERR) || (portList.size == 0))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get ports failed,tnlId (%d)", tnlIntfId);
        return status;
    }

    for (uint32_t j = 0; j < portList.size; j++)
    {
        cpssDevNum = xpsGlobalIdToDevId(devId, portList.portIntf[j]);
        cpssPortNum = xpsGlobalPortToPortnum(devId, portList.portIntf[j]);

        rc = cpssHalVlanMemberAdd(cpssDevNum, vniCtx->hwEVlanId,
                                  cpssPortNum, GT_FALSE,
                                  CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "eVlan member add failed (%d)", vniCtx->hwEVlanId);
            return xpsConvertCpssStatusToXPStatus(rc);
        }
    }

    return status;
}

XP_STATUS xpsVxlanTunnelDelFromVniList(xpsDevice_t devId,
                                       xpsInterfaceId_t tnlIntfId, uint32_t vniId)
{
    XP_STATUS           status  = XP_NO_ERR;
    xpsVxlanVniInfo_t* vniInfo = NULL;
    xpsVxlanVniInfo_t childKey;
    xpsVxlanDbEntry_t *lookupEntry = NULL;
    xpsScope_t          scopeId;

    status = xpsScopeGetScopeId(devId, &scopeId);
    if (status != XP_NO_ERR)
    {
        return status;
    }
    status = xpsVxlanGetTunnelDb(scopeId, tnlIntfId, &lookupEntry);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Vxlan Tunnel Not found :%d \n", tnlIntfId);
        return status;
    }
    status = xpsVxlanTunnelFindVni(scopeId, lookupEntry->tnlIntfId, vniId,
                                   &vniInfo);
    if (status != XP_NO_ERR || vniInfo == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "VNI not found (%d) in tnl", vniId, lookupEntry->tnlIntfId);
    }
    vniInfo->refCnt--;
    if (vniInfo->refCnt == 0)
    {
        childKey.vniId = vniId;
        vniInfo = (xpsVxlanVniInfo_t*)xpsRBTDeleteNode(lookupEntry->vniList,
                                                       (xpsDbKey_t)&childKey);

        // Remove the corresponding state
        if (vniInfo == NULL)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Ket not found(%d) failed", vniId);
            return XP_ERR_NOT_FOUND;
        }

        // Free the memory allocated for the corresponding state
        if ((status = xpsStateHeapFree((void *)vniInfo)) != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "failed for free (%d) failed", vniId);
            return status;
        }
    }
    return status;
}

XP_STATUS xpsVxlanTunnelGetNextVni(xpsScope_t scopeId,
                                   xpsInterfaceId_t tnlIntfId,
                                   xpsVxlanVniInfo_t **nextInfo)
{
    XP_STATUS status = XP_NO_ERR;
    xpsVxlanDbEntry_t *lookupEntry = NULL;

    status = xpsVxlanGetTunnelDb(scopeId, tnlIntfId, &lookupEntry);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Vxlan Tunnel Not found :%d \n", tnlIntfId);
        return status;
    }

    *nextInfo = (xpsVxlanVniInfo_t*)xpsRBTGetInorderSuccessor(lookupEntry->vniList,
                                                              (xpsDbKey_t)*nextInfo);

    if (*nextInfo == NULL)
    {
        return XP_ERR_NULL_POINTER;
    }
    return XP_NO_ERR;
}

static XP_STATUS xpsVxlanRemoveEportFromDb(xpsScope_t scopeId,
                                           xpsInterfaceId_t tnlIntfId,
                                           xpsInterfaceId_t intfId,
                                           uint32_t *ePort,
                                           xpsIpTnlGblDbEntry_t **vxlanCtxNew)
{

    XP_STATUS result = XP_NO_ERR;
    xpsIpTnlGblDbEntry_t *vxlanCtx = NULL;
    uint32_t j = 0, numOfEports, intfPresent = 0;

    result = xpsIpTunnelGblGetDbEntry(scopeId, tnlIntfId, &vxlanCtx);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Vxlan Tunnel Not found :%d \n", tnlIntfId);
        return result;
    }
    numOfEports = vxlanCtx->numOfEports;

    if (!numOfEports)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Vxlan does not have any interface");
        return XP_ERR_INVALID_ARG;
    }

    //Delete the element from the context
    vxlanCtx->numOfEports = numOfEports - 1;
    for (j = 0; j < numOfEports; j++)
    {
        if (!intfPresent)
        {
            if (vxlanCtx->ePorts[j].intfId == intfId)
            {
                intfPresent = 1;
                if (ePort)
                {
                    *ePort = vxlanCtx->ePorts[j].ePort;
                }
            }
            continue;
        }
        else if (j != 0)
        {
            vxlanCtx->ePorts[j-1] = vxlanCtx->ePorts[j];
        }
    }

    if (!intfPresent)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid arguments");
        return XP_ERR_INVALID_ARG;
    }

    if (xpsDAIsCtxShrinkNeeded(numOfEports, MAX_EPORT_SIZE))
    {
        result = xpsIpTunnelReSize(scopeId, tnlIntfId, vxlanCtxNew, vxlanCtx, false);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Dynamic array grow failed");
            return result;
        }
    }

    return result;
}

static XP_STATUS xpsVxlanAddEportToDb(xpsScope_t scopeId,
                                      xpsInterfaceId_t l3IntfId,
                                      xpsInterfaceId_t tnlIntfId,
                                      uint32_t tnlStartIdx,
                                      uint32_t ePort,
                                      xpsInterfaceId_t intfId,
                                      xpsIpTnlGblDbEntry_t **vxlanCtxNewPtr)
{
    XP_STATUS result = XP_NO_ERR;
    xpsIpTnlGblDbEntry_t *vxlanCtx = NULL;
    uint16_t numOfEports;
    uint16_t mxEports = MAX_EPORT_SIZE;

    result = xpsIpTunnelGblGetDbEntry(scopeId, tnlIntfId, &vxlanCtx);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Vxlan Tunnel Not found :%d \n", tnlIntfId);
        return result;
    }

    numOfEports = vxlanCtx->numOfEports;

    //Number of interfaces increase exponentially starting from MAX_EPORT_SIZE by default
    //Everytime the number reaches the current size, the size is doubled
    if (xpsDAIsCtxGrowthNeeded(numOfEports, MAX_EPORT_SIZE))
    {
        result = xpsIpTunnelReSize(scopeId, tnlIntfId, vxlanCtxNewPtr, vxlanCtx, true);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Dynamic array grow failed");
            return result;
        }

        vxlanCtx = *vxlanCtxNewPtr;
        mxEports = numOfEports + MAX_EPORT_SIZE;
    }

    if (numOfEports >= mxEports)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Max interface count reached : %d \n", numOfEports);
        return XP_ERR_OUT_OF_MEM;
    }

    vxlanCtx->ePorts[numOfEports].ePort = ePort;
    vxlanCtx->ePorts[numOfEports].intfId = intfId;
    vxlanCtx->ePorts[numOfEports].l3IntfId = l3IntfId;
    vxlanCtx->ePorts[numOfEports].tnlStartId = tnlStartIdx;
    vxlanCtx->numOfEports += 1;

    return result;
}

XP_STATUS xpsVxlanUpdateNNIPortOnLagRIf(xpsDevice_t devId,
                                        xpsInterfaceInfo_t *l3IntfInfo,
                                        xpsInterfaceId_t portIntfId,
                                        bool add)
{
    XP_STATUS rt = XP_NO_ERR;
    GT_STATUS rc = GT_OK;
    xpsInterfaceChildInfo_t *nextInfo = NULL;
    xpsVniDbEntry_t *vniCtx = NULL;
    xpsVxlanVniInfo_t *vniInfo = NULL;
    GT_U8               cpssDevId;
    GT_U32              cpssPortNum;
    xpsScope_t          scopeId;
    xpsDevice_t         portDevId = 0;
    xpPort_t            portId = 0;

    rt = xpsScopeGetScopeId(devId, &scopeId);
    if (rt != XP_NO_ERR)
    {
        return rt;
    }

    rt = xpsPortGetDevAndPortNumFromIntfScope(scopeId, portIntfId, &portDevId,
                                              &portId);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get port and dev Id from port intf : %d \n", portIntfId);
        return rt;
    }

    cpssDevId = xpsGlobalIdToDevId(portDevId, portId);
    cpssPortNum = xpsGlobalPortToPortnum(portDevId, portId);

    while (xpsIntfChildGetNext(l3IntfInfo, &nextInfo) == XP_NO_ERR)
    {
        while (xpsVxlanTunnelGetNextVni(scopeId, nextInfo->keyIntfId,
                                        &vniInfo) == XP_NO_ERR)
        {
            rt = xpsVniGetDbEntry(scopeId, vniInfo->vniId, &vniCtx);
            if (rt != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Get VNI Context Db failed, vni(%d)", vniInfo->vniId);
                return rt;
            }

            if (add)
            {
                rc = cpssHalVlanMemberAdd(cpssDevId, vniCtx->hwEVlanId,
                                          cpssPortNum, GT_FALSE,
                                          CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E);
                if (rc != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "eVlan member add failed (%d)", vniCtx->hwEVlanId);
                    return xpsConvertCpssStatusToXPStatus(rc);
                }
            }
            else
            {
                rc = cpssHalBrgVlanPortDelete(cpssDevId, vniCtx->hwEVlanId,
                                              cpssPortNum);
                if (rc != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "eVlan member add failed (%d)", vniCtx->hwEVlanId);
                    return xpsConvertCpssStatusToXPStatus(rc);
                }
            }
        }
    }
    return rt;
}

static XP_STATUS xpsVxlanRemoveNNIPortFromTunnel(xpsDevice_t devId,
                                                 xpsInterfaceId_t tnlIntfId,
                                                 xpsInterfaceId_t portIntfId,
                                                 uint32_t *portEPort)
{
    XP_STATUS rt = XP_NO_ERR;
    //    GT_STATUS rc = GT_OK;
    xpsScope_t          scopeId;
    xpsIpTnlGblDbEntry_t *vxlanCtx = NULL;
    uint32_t ePort = 0;
    uint32_t prevNumEPorts = 0;
    uint32_t prevStartIdx = 0;
    GT_STATUS rc;

    rt = xpsScopeGetScopeId(devId, &scopeId);
    if (rt != XP_NO_ERR)
    {
        return rt;
    }

    rt = xpsIpTunnelGblGetDbEntry(scopeId, tnlIntfId, &vxlanCtx);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Vxlan Tunnel Not found :%d \n", tnlIntfId);
        return rt;
    }

    prevNumEPorts = vxlanCtx->numOfEports;
    prevStartIdx = vxlanCtx->ecmpStartIdx;


    rt = xpsVxlanRemoveEportFromDb(scopeId, tnlIntfId, portIntfId, &ePort,
                                   &vxlanCtx);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsVxlanUpdateL2MLL Failed (%d)", rt);
        return rt;
    }
    /* Update New L2ECMP mapping */
    rt = xpsSetTunnelEcmpConfig(devId, tnlIntfId);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " xpsSetTunnelEcmpConfig Failed  %d\n", ePort);
        return rt;
    }

    /* Free the prev L2Ecmp Table Index */
    rt = xpsClearTunnelEcmpConfig(devId, prevStartIdx, prevNumEPorts);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " xpsClearTunnelEcmpConfig Failed  %d\n", ePort);
        return rt;
    }

    rt = xpsVxlanRemoveEportFromTunnelTermination(devId, tnlIntfId, portIntfId,
                                                  ePort);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsVxlanRemoveEportFromTunnelTermination Failed (%d)", rt);
        return rt;
    }
#if 0
    rc = cpssHalBrgFdbNaToCpuPerPortSet(devId, ePort, GT_FALSE);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Tnl Learn config Failed (%d)", rc);
        return xpsConvertCpssStatusToXPStatus(rc);
    }
#endif
    rt = xpsVxlanClearTunnelEportConfig(devId, ePort);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "ClearTunnelEport Failed :%d \n", ePort);
        return rt;
    }

    rc = cpssHalTunnelSetEPortMeshId(devId, ePort, 0);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set meshId for port %d\n",
              ePort);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    if (portEPort)
    {
        *portEPort = ePort;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEFAULT, "Remove Eport %d\n",
          (portEPort == NULL)? 0: *portEPort);
    return rt;
}

static XP_STATUS xpsVxlanAddNNIPortToTunnel(xpsDevice_t devId,
                                            xpsInterfaceInfo_t *l3IntfInfo,
                                            xpsInterfaceId_t tnlIntfId,
                                            xpsInterfaceId_t portIntfId,
                                            xpsInterfaceId_t portMtuIntfId,
                                            uint32_t tnlStartIdx,
                                            uint32_t ePort)
{
    XP_STATUS rt = XP_NO_ERR;
    //    GT_STATUS rc = GT_OK;
    xpsScope_t          scopeId;
    xpsIpTnlGblDbEntry_t *vxlanCtx = NULL;
    uint32_t prevNumEPorts = 0;
    uint32_t prevStartIdx = 0;
    GT_STATUS rc;

    rt = xpsScopeGetScopeId(devId, &scopeId);
    if (rt != XP_NO_ERR)
    {
        return rt;
    }

    rt = xpsIpTunnelGblGetDbEntry(scopeId, tnlIntfId, &vxlanCtx);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Vxlan Tunnel Not found :%d \n", tnlIntfId);
        return rt;
    }

    prevNumEPorts = vxlanCtx->numOfEports;
    prevStartIdx = vxlanCtx->ecmpStartIdx;

    if (l3IntfInfo->type == XPS_VLAN_ROUTER && tnlStartIdx == 0xFFFFFFFF)
    {
        /* SVI member Eports map to same Tunnel Start entry. */
        tnlStartIdx = vxlanCtx->ePorts[0].tnlStartId;
    }

    rt = xpsVxlanAddEportToDb(scopeId, l3IntfInfo->keyIntfId, tnlIntfId,
                              tnlStartIdx, ePort, portIntfId, &vxlanCtx);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsVxlanUpdateL2MLL Failed (%d)", rt);
        return rt;
    }

    rt = xpsVxlanSetTunnelEportConfig(devId, tnlStartIdx, ePort,
                                      l3IntfInfo,
                                      portMtuIntfId,
                                      portIntfId);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " Eport Set Failed  %d\n", ePort);
        return rt;
    }

    /* Update New L2ECMP mapping */
    rt = xpsSetTunnelEcmpConfig(devId, tnlIntfId);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " xpsSetTunnelEcmpConfig Failed  %d\n", ePort);
        return rt;
    }

    /* Free the prev L2Ecmp Table Index */
    rt = xpsClearTunnelEcmpConfig(devId, prevStartIdx, prevNumEPorts);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " xpsClearTunnelEcmpConfig Failed  %d\n", ePort);
        return rt;
    }

    rt = xpsVxlanAddEportToTunnelTermination(devId, tnlIntfId, portIntfId, ePort,
                                             vxlanCtx->primaryEport);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsVxlanAddEportToTunnelTermination Failed (%d)", rt);
        return rt;
    }
#if 0
    rc = cpssHalBrgFdbNaToCpuPerPortSet(devId, ePort,
                                        (GT_BOOL)(vxlanCtx->isLearnEnable));
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Tnl Learn config Failed (%d)", rc);
        return xpsConvertCpssStatusToXPStatus(rc);
    }
#endif
    rc = cpssHalTunnelSetEPortMeshId(devId, ePort, 1);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set meshId for port %d\n",
              ePort);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEFAULT, "Add Eport %d\n", ePort);
    return rt;
}

XP_STATUS xpsVxlanUpdateNNIPortOnSVI(xpsDevice_t devId,
                                     xpsInterfaceInfo_t *l3IntfInfo,
                                     xpsInterfaceId_t portIntfId, /*Can be lag or port*/
                                     bool add)
{
    XP_STATUS rt = XP_NO_ERR;
    GT_STATUS rc = GT_OK;
    xpsInterfaceChildInfo_t *nextInfo = NULL;
    xpsVniDbEntry_t *vniCtx = NULL;
    xpsVxlanVniInfo_t *vniInfo = NULL;
    GT_U8               cpssDevId;
    GT_U32              cpssPortNum;
    xpsScope_t          scopeId;
    xpsDevice_t         portDevId = 0;
    xpPort_t            portId = 0;
    uint32_t            ePort = 0;
    xpsInterfaceType_e portIntfType;

    xpsLagPortIntfList_t portIntfList;
    memset(&portIntfList, 0, sizeof(portIntfList));

    rt = xpsScopeGetScopeId(devId, &scopeId);
    if (rt != XP_NO_ERR)
    {
        return rt;
    }

    rt = xpsInterfaceGetType(portIntfId, &portIntfType);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to fetch interface type from interface id %d\n", portIntfId);
        return rt;
    }

    if (portIntfType == XPS_PORT)
    {
        portIntfList.portIntf[0] = portIntfId;
        portIntfList.size = 1;
    }
    else if (portIntfType == XPS_LAG)
    {
        if ((rt = xpsLagGetPortIntfList(portIntfId, &portIntfList)) != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Get ports failed, lag interface(%d)", portIntfId);
            return rt;
        }
    }
    /* Iterate Tunnels on L3 RIF*/
    while (xpsIntfChildGetNext(l3IntfInfo, &nextInfo) == XP_NO_ERR)
    {
        ePort = 0;
        if (add)
        {
            rt = xpsAllocatorAllocateId(scopeId, XP_ALLOC_GLOBAL_EPORT, &ePort);
            if (rt != XP_NO_ERR)
            {
                XPS_FUNC_EXIT_LOG();
                return rt;
            }

            rt = xpsVxlanAddNNIPortToTunnel(devId, l3IntfInfo, nextInfo->keyIntfId,
                                            portIntfId, portIntfList.portIntf[0], 0xFFFFFFFF, ePort);
            if (rt != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "xpsVxlanAddNNIPortToTunnel Failed : %d \n", portIntfId);
                return rt;
            }
        }
        else
        {
            rt = xpsVxlanRemoveNNIPortFromTunnel(devId, nextInfo->keyIntfId,
                                                 portIntfId, &ePort);
            if (rt != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "xpsVxlanRemoveNNIPortFromTunnel Failed : %d \n", portIntfId);
                return rt;
            }

            rt = xpsAllocatorReleaseId(scopeId, XP_ALLOC_GLOBAL_EPORT, ePort);
            if (rt != XP_NO_ERR)
            {
                XPS_FUNC_EXIT_LOG();
                return rt;
            }
        }
        /*Iterate VNI list per tunnel and update L2MLL*/
        while (xpsVxlanTunnelGetNextVni(scopeId, nextInfo->keyIntfId,
                                        &vniInfo) == XP_NO_ERR)
        {
            rt = xpsVniGetDbEntry(scopeId, vniInfo->vniId, &vniCtx);
            if (rt != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Get VNI Context Db failed, vni(%d)", vniInfo->vniId);
                return rt;
            }

            if (add)
            {
                for (uint32_t i = 0; i < portIntfList.size; i++)
                {
                    rt = xpsPortGetDevAndPortNumFromIntfScope(scopeId, portIntfList.portIntf[i],
                                                              &portDevId,
                                                              &portId);
                    if (rt != XP_NO_ERR)
                    {
                        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                              "Failed to get port and dev Id from port intf : %d \n", portIntfId);
                        return rt;
                    }

                    cpssDevId = xpsGlobalIdToDevId(portDevId, portId);
                    cpssPortNum = xpsGlobalPortToPortnum(portDevId, portId);

                    rc = cpssHalVlanMemberAdd(cpssDevId, vniCtx->hwEVlanId,
                                              cpssPortNum, GT_FALSE,
                                              CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E);
                    if (rc != GT_OK)
                    {
                        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                              "eVlan member add failed (%d)", vniCtx->hwEVlanId);
                        return xpsConvertCpssStatusToXPStatus(rc);
                    }
                }
#if 0
                rt = xpsVxlanAddIntfDb(scopeId, vniCtx->vniId, ePort, XPS_TUNNEL_VXLAN,
                                       0xFFFFFFFF,
                                       &vniCtx);
                if (rt != XP_NO_ERR)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Add interface db failed"
                          "vniId(%d) interface(%d)", vniCtx->vniId, ePort);
                    return rt;
                }
#endif

                rt = xpsVxlanUpdateL2MLL(devId, vniCtx);
                if (rt != XP_NO_ERR)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "xpsVxlanUpdateL2MLL Failed (%d)", rt);
                    return rt;
                }
            }
            else
            {
                for (uint32_t i = 0; i < portIntfList.size; i++)
                {
                    rt = xpsPortGetDevAndPortNumFromIntfScope(scopeId, portIntfList.portIntf[i],
                                                              &portDevId,
                                                              &portId);
                    if (rt != XP_NO_ERR)
                    {
                        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                              "Failed to get port and dev Id from port intf : %d \n",
                              portIntfList.portIntf[i]);
                        return rt;
                    }

                    cpssDevId = xpsGlobalIdToDevId(portDevId, portId);
                    cpssPortNum = xpsGlobalPortToPortnum(portDevId, portId);

                    rc = cpssHalBrgVlanPortDelete(cpssDevId, vniCtx->hwEVlanId,
                                                  cpssPortNum);
                    if (rc != GT_OK)
                    {
                        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                              "eVlan member add failed (%d)", vniCtx->hwEVlanId);
                        return xpsConvertCpssStatusToXPStatus(rc);
                    }
                }
#if 0
                rt = xpsVxlanRemoveIntfDb(scopeId, vniCtx->vniId, ePort, &vniCtx);
                if (rt != XP_NO_ERR)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Add interface db failed"
                          "vniId(%d) interface(%d)", vniCtx->vniId, ePort);
                    return rt;
                }
#endif

                rt = xpsVxlanUpdateL2MLL(devId, vniCtx);
                if (rt != XP_NO_ERR)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "xpsVxlanUpdateL2MLL Failed (%d)", rt);
                    return rt;
                }
            }
        }
    }
    return rt;
}

XP_STATUS xpsVxlanTunnelLearnModeSet(xpsDevice_t devId,
                                     xpsInterfaceId_t tnlIntfId, xpPktCmd_e pktCmd)
{
    XP_STATUS rt = XP_NO_ERR;
    GT_STATUS rc = GT_OK;
    xpsScope_t          scopeId;
    GT_BOOL learnEnable = GT_FALSE;
    xpsIpTnlGblDbEntry_t *vxlanCtx = NULL;

    rt = xpsScopeGetScopeId(devId, &scopeId);
    if (rt != XP_NO_ERR)
    {
        return rt;
    }

    rt = xpsIpTunnelGblGetDbEntry(scopeId, tnlIntfId, &vxlanCtx);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Vxlan Tunnel Not found :%d \n", tnlIntfId);
        return rt;
    }

    learnEnable = (pktCmd == XP_PKTCMD_FWD) ? GT_FALSE:GT_TRUE;
#if 0
    for (uint32_t i=0; i < vxlanCtx->numOfEports; i++)
    {
        rc = cpssHalBrgFdbNaToCpuPerPortSet(devId, vxlanCtx->ePorts[i].ePort,
                                            learnEnable);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "ePort learn mode failed (%d) (%d)", vxlanCtx->ePorts[i].ePort, learnEnable);
            return xpsConvertCpssStatusToXPStatus(rc);
        }
    }
#endif
    rc = cpssHalBrgFdbNaToCpuPerPortSet(devId, vxlanCtx->primaryEport,
                                        learnEnable);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "ePort learn mode failed (%d) (%d)", vxlanCtx->primaryEport, learnEnable);
        return xpsConvertCpssStatusToXPStatus(rc);
    }
    vxlanCtx->isLearnEnable = (bool)learnEnable;

    return rt;
}

XP_STATUS xpsVxlanGetTunnelEPort(xpsDevice_t devId,
                                 xpsInterfaceId_t tnlIntfId,
                                 uint32_t *ePort)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpsIpTnlGblDbEntry_t *vxlanCtxPtr = NULL;
    xpsScope_t scopeId;

    /* Get Scope Id from devId */
    if ((retVal = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return retVal;
    }

    retVal = xpsIpTunnelGblGetDbEntry(scopeId, tnlIntfId, &vxlanCtxPtr);
    if (retVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get Db for tunnel %d, rc = %d\n", tnlIntfId, retVal);
        return retVal;
    }

    *ePort = vxlanCtxPtr->primaryEport;

    return retVal;
}

XP_STATUS xpsVxlanGetTunnelHwEVlanId(xpsDevice_t devId,
                                     xpsVlan_t vlanId,
                                     xpsVlan_t *hwEVlanId)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpsVniDbEntry_t *vniCtx = NULL;
    uint32_t vniId;
    xpsScope_t scopeId;

    /* Get Scope Id from devId */
    if ((retVal = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return retVal;
    }

    retVal = xpsVlanGetVniInVlanDb(scopeId, vlanId, &vniId);
    if (retVal != XP_NO_ERR)
    {
        return retVal;
    }

    retVal = xpsVniGetDbEntry(scopeId, vniId, &vniCtx);
    if (retVal != XP_NO_ERR)
    {
        return retVal;
    }

    *hwEVlanId = vniCtx->hwEVlanId;

    return retVal;
}

XP_STATUS xpsVxlanGetTunnelVlanByHwEVlanId(xpsDevice_t devId,
                                           xpsVlan_t hwEVlanId,
                                           xpsVlan_t *vlanId)
{
    GT_STATUS rc = GT_OK;
    xpsVlan_t vlan = 0;

    rc = cpssHalVlanTranslationEntryRead(devId, hwEVlanId,
                                         CPSS_DIRECTION_EGRESS_E, &vlan);
    if (rc != GT_OK)
    {
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    *vlanId = vlan;

    return XP_NO_ERR;
}

XP_STATUS xpsVxlanGetTunnelIdbyEPort(xpsDevice_t devId,
                                     xpsVlan_t   vlanId,
                                     uint32_t    ePort,
                                     xpsInterfaceId_t *tnlIntfId)
{
    XP_STATUS xpsRetVal = XP_NO_ERR;
    xpsInterfaceId_t *intfList  = NULL;
    uint16_t numOfIntfs = 0;
    xpsInterfaceId_t intfId;
    xpsScope_t scopeId;
    xpsInterfaceType_e intfType;
    xpsIpTnlGblDbEntry_t *lookupEntry = NULL;

    /* Get Scope Id from devId */
    if ((xpsRetVal = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return xpsRetVal;
    }

    xpsRetVal = xpsVlanGetInterfaceList(devId, vlanId, &intfList, &numOfIntfs);
    if (xpsRetVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "failed to get interface list of vlan :%d error:%d \n", vlanId, xpsRetVal);
        return xpsRetVal;
    }

    for (int i =0; i<numOfIntfs; i++)
    {
        intfId = intfList[i];
        xpsRetVal = xpsInterfaceGetTypeScope(devId, intfId, &intfType);
        if (xpsRetVal != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Interface Get type for interface(%d) failed", intfId);
            return xpsRetVal;
        }

        if (intfType == XPS_TUNNEL_VXLAN)
        {
            xpsRetVal = xpsIpTunnelGblGetDbEntry(scopeId, intfId, &lookupEntry);
            if (xpsRetVal != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Vxlan Tunnel Not found :%d \n", intfId);
                return xpsRetVal;
            }
            if (lookupEntry->primaryEport == ePort)
            {
                *tnlIntfId = intfId;
            }
            else
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
                      "Vxlan Tunnel PriEport mismatch:%d PriEPort %d ePort %d\n",
                      intfId, lookupEntry->primaryEport, ePort);
                return xpsRetVal;
            }
        }
    }

    return xpsRetVal;
}

#ifdef __cplusplus
}
#endif
