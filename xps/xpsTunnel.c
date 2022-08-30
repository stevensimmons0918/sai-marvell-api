// xpsTunnel.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpsTunnel.h"
#include "xpsInit.h"
#include "xpsInternal.h"
#include "xpsInterface.h"
#include "xpsScope.h"
#include "xpsLock.h"
#include "xpsAllocator.h"
#include "cpss/dxCh/dxChxGen/virtualTcam/cpssDxChVirtualTcam.h"
#include "cpss/dxCh/dxChxGen/tti/cpssDxChTtiTypes.h"
#include "cpssHalUtil.h"
#include "cpssHalTunnel.h"
#include "cpssHalTcam.h"
#include "cpssHalAcl.h"
#include "cpssHalL3.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#define XPS_TUNNEL_TERM_HW_RANGE_START 0
#define XPS_TUNNEL_TERM_HW_MAX_IDS     (1024)
#define XPS_TTI_TCAM_CLIENT_GROUP      CPSS_TCAM_CLIENT_GROUP2

uint32_t curMaxTnlTermEntries = XPS_TUNNEL_TERM_HW_MAX_IDS;
uint32_t tunnelStartIpv6LastIdx_g = 0;

typedef enum xpsTnlStaticDataType_e
{
    XPS_TNL_STATIC_INSERTION_IDS,
} xpsTnlStaticDataType_e;

/* Gloabl insertion ids to be used across tunnels and devices */
typedef struct xpsTnlInsertionIds_t
{
    xpsTnlStaticDataType_e staticDataType;
    uint32_t vxLanHdrInsrtId;
    uint32_t nvgreHdrInsrtId;
    uint32_t greHdrInsrtId;
    uint32_t vpnGreHdrInsrtId;
    uint32_t greErspan2HdrInsrtId;
    uint32_t geneveHdrInsrtId;
    uint32_t geneveFlowHdrInsrtId;
    uint32_t geneveFlowTemplateHdrInsrtId;
    uint32_t genevePacketTrakkerHdrInsrtId;
} xpsTnlInsertionIds_t;

/*
 * Tunnel nexthop state. This is used for sharing the L2 encap insertion
 * id across tunnels.
 */
typedef struct xpsTnlNextHopDbEntry_t
{
    uint32_t nhId;
    uint32_t insertionId;
    uint32_t refCount;
} xpsTnlNextHopDbEntry_t;

/* Global PBB tunnel state */
typedef struct xpsPbbTnlGblDbEntry_t
{
    xpsPbbTunnelData_t tnlData;
    xpVif_t vifId;
    uint32_t l2HdrInsertionId;
} xpsPbbTnlGblDbEntry_t;

/* tunnel start allocation entry */
typedef struct xpsTnlStartAllocGblDbEntry_t
{
    uint32_t             index;
    xpIpPrefixType_t     type;
} xpsTnlStartAllocGblDbEntry_t;


static xpsDbHandle_t tnlEntryDbHndl = XPS_STATE_INVALID_DB_HANDLE;

static xpsDbHandle_t ipTnlStartGblDbHndl = XPS_STATE_INVALID_DB_HANDLE;

static xpsDbHandle_t ipTnlGblDbHndl = XPS_STATE_INVALID_DB_HANDLE;

static int32_t ipTnlGblDbEntryCompare(void *key1, void *key2)
{
    return ((xpsIpTnlGblDbEntry_t *)key1)->vifId - ((xpsIpTnlGblDbEntry_t *)
                                                    key2)->vifId;
}

static int32_t tnlDbEntryCompare(void* key1, void* key2)
{
    return ((xpsTnlDbEntry_t *)key1)->vifId - ((xpsTnlDbEntry_t *)key2)->vifId;
}

static int32_t ipTnlStartGblDbEntryCompare(void *key1, void *key2)
{
    return ((xpsTnlStartAllocGblDbEntry_t *)key1)->index - ((
                                                                xpsTnlStartAllocGblDbEntry_t *)key2)->index;
}

static int32_t tnlTermDbEntryCompare(void* key1, void* key2)
{
    return ((xpsIpTnlTermEntry_t *)key1)->termEntryId - ((xpsIpTnlTermEntry_t *)
                                                         key2)->termEntryId;
}

/* State manager database handles */

XP_STATUS xpsIpTunnelGblDbGetData(xpsScope_t scopeId, xpsInterfaceId_t intfId,
                                  xpsIpTunnelData_t *data)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS rt = XP_NO_ERR;
    xpsIpTnlGblDbEntry_t keyIpTnlGblEntry;
    xpsIpTnlGblDbEntry_t *ipTnlGblEntry;

    keyIpTnlGblEntry.vifId = intfId;

    rt = xpsStateSearchData(scopeId, ipTnlGblDbHndl, (xpsDbKey_t)&keyIpTnlGblEntry,
                            (void **)&ipTnlGblEntry);
    if (rt != XP_NO_ERR)
    {
        return rt;
    }

    if (!ipTnlGblEntry)
    {
        rt = XP_ERR_NULL_POINTER;
        return rt;
    }

    *data = ipTnlGblEntry->tnlData;

    XPS_FUNC_EXIT_LOG();

    return rt;
}

XP_STATUS xpsIpTunnelGblGetDbEntry(xpsScope_t scopeId, xpsInterfaceId_t intfId,
                                   xpsIpTnlGblDbEntry_t **ipTnlGblEntry)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS rt = XP_NO_ERR;
    xpsIpTnlGblDbEntry_t keyIpTnlGblEntry;
    memset(&keyIpTnlGblEntry, 0, sizeof(keyIpTnlGblEntry));

    keyIpTnlGblEntry.vifId = intfId;

    rt = xpsStateSearchData(scopeId, ipTnlGblDbHndl, (xpsDbKey_t)&keyIpTnlGblEntry,
                            (void **)ipTnlGblEntry);
    if (rt != XP_NO_ERR)
    {
        return rt;
    }

    if (!*ipTnlGblEntry)
    {
        rt = XP_ERR_NULL_POINTER;
        return rt;
    }

    XPS_FUNC_EXIT_LOG();

    return rt;
}

XP_STATUS xpsIpTunnelDbAddEntry(xpsDevice_t devId, xpsInterfaceId_t intfId,
                                uint32_t tnlTableIndex, int32_t nhId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsIpTunnelDbAddEntry);
    XP_STATUS status = XP_NO_ERR;
    xpsTnlDbEntry_t *dbEntry = NULL;

    //Allocate space for the tunnel db entry
    status = xpsStateHeapMalloc(sizeof(xpsTnlDbEntry_t), (void **)&dbEntry);
    if (status != XP_NO_ERR)
    {
        return status;
    }

    memset(dbEntry, 0, sizeof(xpsTnlDbEntry_t));
    dbEntry->vifId = intfId;
    dbEntry->tnlTableIndex = tnlTableIndex;

    // Insert the db entry into the tunnel database
    status = xpsStateInsertDataForDevice(devId, tnlEntryDbHndl, (void *)dbEntry);
    if (status != XP_NO_ERR)
    {
        // Free allocated memory in case of an error
        xpsStateHeapFree((void*)dbEntry);
        return status;
    }

    XPS_FUNC_EXIT_LOG();

    return status;
}

XP_STATUS xpsIpTunnelDbRemoveEntry(xpsDevice_t devId, xpsInterfaceId_t intfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsIpTunnelDbRemoveEntry);
    XP_STATUS status = XP_NO_ERR;
    xpsTnlDbEntry_t dbEntry;
    xpsTnlDbEntry_t *lookupDbEntry;

    dbEntry.vifId = intfId;

    //Remove the db entry from the tunnel database
    status = xpsStateDeleteDataForDevice(devId, tnlEntryDbHndl,
                                         (xpsDbKey_t)&dbEntry, (void **)&lookupDbEntry);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Deletion of entry from tunnel database failed, "
              "devId(%d) interface(%d)", devId, intfId);
        return status;
    }

    //Free the allocated memory as well
    status = xpsStateHeapFree(lookupDbEntry);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Heap freeing failed");
        return status;
    }
    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsIpTunnelDbGetEntry(xpsDevice_t devId, xpsInterfaceId_t intfId,
                                xpsTnlDbEntry_t **lookupDbEntry)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;
    xpsTnlDbEntry_t dbEntry;

    dbEntry.vifId = intfId;

    status = xpsStateSearchDataForDevice(devId, tnlEntryDbHndl,
                                         (xpsDbKey_t)&dbEntry, (void **)lookupDbEntry);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Search data failed, interface(%d)", intfId);
        return status;
    }

    if (!(*lookupDbEntry))
    {
        status = XP_ERR_NOT_FOUND;
        return status;
    }

    XPS_FUNC_EXIT_LOG();

    return status;
}

XP_STATUS xpsIpTunnelDbGetTunnelTermIndex(xpsDevice_t devId,
                                          xpsInterfaceId_t intfId, uint32_t *index)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;
    xpsTnlDbEntry_t *tnlDbEntry = NULL;

    status = xpsIpTunnelDbGetEntry(devId, intfId, &tnlDbEntry);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Tunnel interface(%d) does not exist", intfId);
        return status;
    }

    *index = tnlDbEntry->tnlTableIndex;

    XPS_FUNC_EXIT_LOG();

    return status;
}

XP_STATUS xpsIpTnlGblDbAddOptionalInterface(xpsDevice_t devId,
                                            xpsInterfaceId_t baseIntfId, xpsInterfaceId_t optIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();
    return XP_NO_ERR;
}




XP_STATUS xpsTunnelInit(void)
{
    XPS_FUNC_ENTRY_LOG();

    return xpsTunnelInitScope(XP_SCOPE_DEFAULT);
}

XP_STATUS xpsTunnelTunnelIdDisplayTable(xpDevice_t devId,
                                        uint32_t *numOfValidEntries)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsTunnelLocalVtepDisplayTable(xpDevice_t devId,
                                         uint32_t *numOfValidEntries)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

/**
 * \private
 * \fn xpsTunnelInitTunnelStartAllocator
 * \brief initialize the allocator for Tunnel Start
 *
 * Note:
 *
 * \param [in] devId
 *
 * \return xp_status
 */
static XP_STATUS xpsTunnelInitTunnelStartAllocator(xpsDevice_t xpsDevId)
{
    XP_STATUS  status = XP_NO_ERR;
    xpsScope_t scope;
    GT_U32 numOfEntries = 0, startIndex = 0;
#if 0
    GT_STATUS cpssStatus;

    cpssStatus = cpssDxChCfgTableNumEntriesGet(xpsDevId, CPSS_DXCH_CFG_TABLE_ARP_E,
                                               &numOfEntries);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get TS table depth. Device : %d Status : %d\n", xpsDevId,
              cpssStatus);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }
#endif
    /* Get Scope Id from devId */
    if ((status = xpsScopeGetScopeId(xpsDevId, &scope))!= XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " Tunnel Start Allocator scope get failed");
        return status;
    }


    /*
       Ipv4 and Ipv6 TS entries share HW arp-table
       For Falcon, each memory line HW can hold 8 ARP or 2 IPv4 or 1 IPv6 entries
       So the table index can be like below. But note that allocation
       MUST not overlap with same memory line index.
       ARP - 0,1,2...96K
       TS V4 - 0,1,....24K
       TS V6 - 0,2,4,... 12K
    */
    numOfEntries = cpssHalGetSKUmaxTunStart(xpsDevId)/4;
    startIndex = XP_ROUTER_ARP_DA_AND_TUNNEL_START_MAX_IDS(xpsDevId)/4;

    status = xpsAllocatorInitIdAllocator(scope,
                                         XPS_ALLOCATOR_TUNNEL_START_v4,
                                         numOfEntries,
                                         startIndex);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " V4 Init Allocator failed : %d \n ", status);
        return status;
    }

    tunnelStartIpv6LastIdx_g = (XP_ROUTER_ARP_DA_AND_TUNNEL_START_MAX_IDS(
                                    xpsDevId) +
                                (numOfEntries*4))/8;

    status = xpsAllocatorInitIdAllocator(scope,
                                         XPS_ALLOCATOR_TUNNEL_START_v6,
                                         numOfEntries/2,
                                         0);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " V4 Init Allocator failed : %d \n ", status);
        return status;
    }
    return status;
}

XP_STATUS xpsTunnelInitScope(xpsScope_t scopeId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS rt = XP_NO_ERR;
    GT_STATUS   rc = GT_OK;
    xpsDevice_t devId;
    CPSS_PP_FAMILY_TYPE_ENT devType;
    uint32_t maxTunnelTermIds;
    GT_U32    ttiClientGrpId = 0xffffffff;

    /* Commenting Old XPS code. Some of the below functionality is required
       to handle Tunnel Origination. Hence commented for time being.*/
#if 0
    if (scopeId == XP_SCOPE_DEFAULT)
    {
        rt = xpTunnelMgr::instance()->init();
        if (rt != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Tunnel manager initialization failed");
            return rt;
        }
    }

    tnlEntryDbHndl = XPS_TNL_ENTRY_DB_HNDL;
    rt = xpsStateRegisterDb(scopeId, "Tunnel state", XPS_PER_DEVICE,
                            &tnlDbEntryCompare, tnlEntryDbHndl);
    if (rt != XP_NO_ERR)
    {
        tnlNexthopEntryDbHndl = XPS_STATE_INVALID_DB_HANDLE;
        return rt;
    }
#endif
    rt = xpsScopeGetFirstDevice(scopeId, &devId);
    if (rt != XP_NO_ERR)
    {
        return rt;
    }

    devType = cpssHalDevPPFamilyGet(devId);
    if (devType == CPSS_MAX_FAMILY)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Failed to get dev Type");
        return xpsConvertCpssStatusToXPStatus(GT_FAIL);
    }


    tnlEntryDbHndl = XPS_TNL_ENTRY_DB_HNDL;
    rt = xpsStateRegisterDb(scopeId, "Tunnel state", XPS_PER_DEVICE,
                            &tnlDbEntryCompare, tnlEntryDbHndl);
    if (rt != XP_NO_ERR)
    {
        tnlEntryDbHndl = XPS_STATE_INVALID_DB_HANDLE;
        return rt;
    }

    ipTnlGblDbHndl = XPS_IP_TNL_GBL_DB_HNDL;
    rt = xpsStateRegisterDb(scopeId, "IP Tnl global DB", XPS_GLOBAL,
                            &ipTnlGblDbEntryCompare, ipTnlGblDbHndl);
    if (rt != XP_NO_ERR)
    {
        ipTnlGblDbHndl = XPS_STATE_INVALID_DB_HANDLE;
        return rt;
    }

    ipTnlStartGblDbHndl = XPS_IP_TNL_START_GBL_DB_HNDL;
    rt = xpsStateRegisterDb(scopeId, "IP Tnl global DB", XPS_GLOBAL,
                            &ipTnlStartGblDbEntryCompare, ipTnlStartGblDbHndl);
    if (rt != XP_NO_ERR)
    {
        ipTnlStartGblDbHndl = XPS_STATE_INVALID_DB_HANDLE;
        return rt;
    }

    /*
     * Initialize the CPSS TTI Logical index Id Allocator.
     * Reserving max 6K TTI entries for IP Tunnels.
     * We can revisit this number based on future requirements.
    */
    //TODO
    if (devType == CPSS_PP_FAMILY_DXCH_AC3X_E ||
        devType == CPSS_PP_FAMILY_DXCH_ALDRIN_E ||
        devType == CPSS_PP_FAMILY_DXCH_ALDRIN2_E ||
        devType == CPSS_PP_FAMILY_DXCH_BOBCAT2_E)
    {
        maxTunnelTermIds = (2*XPS_TUNNEL_TERM_HW_MAX_IDS);
    }
    else if (devType == CPSS_PP_FAMILY_DXCH_FALCON_E ||
             devType == CPSS_PP_FAMILY_DXCH_AC5X_E)
    {
        maxTunnelTermIds = (6*XPS_TUNNEL_TERM_HW_MAX_IDS);
    }
    else
    {
        return XP_ERR_INVALID_ID;
    }

    rt = xpsAllocatorInitIdAllocator(scopeId, XP_ALLOC_TUNNEL_TERM_HW_ENTRY,
                                     maxTunnelTermIds,
                                     XPS_TUNNEL_TERM_HW_RANGE_START);

    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to initialize HW Tunnel Term Id allocator :%d\n", rt);
        return rt;
    }


    rt = xpsTunnelInitTunnelStartAllocator(devId);
    if (rt != XP_NO_ERR)
    {
        return rt;
    }

    if ((rc = cpssHalTcamTtiClientGroupSet(devId)) != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " cpssHalTcamTtiClientGroupSet failed with err : %d\n ", rc);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    rc = cpssHalTcamAclClientGroupIdGet(devId, CPSS_DXCH_TCAM_TTI_E,
                                        &ttiClientGrpId);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "failed to TTI groupId : %d  ", rc);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    /*
     * Create VTCam ID 1 (TUNNEL_TERM_TABLE_ID) for all Tunnel Termination.
     * This TCAM has 30B rule size, currently this is used by IP-in-Ip tunnel.
     * The same can be used by GRE/MPLS. Create a new VTCAM incase rule property
     * is different then below.
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
    vTcamInfo.guaranteedNumOfRules = XPS_TUNNEL_TERM_HW_MAX_IDS;
    vTcamInfo.ruleAdditionMethod   =
        CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_LOGICAL_INDEX_E;
    vTcamInfo.ruleSize             = CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E;
    /*Create vTcam*/
    if ((rc = cpssHalVtcamCreate(XPS_GLOBAL_TACM_MGR,
                                 TUNNEL_TERM_TABLE_ID, &vTcamInfo)) != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " cpssHalVtcamCreate failed with err : %d\n ", rc);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    XPS_FUNC_EXIT_LOG();
    return rt;
}

XP_STATUS xpsTunnelDeInit(void)
{
    XPS_FUNC_ENTRY_LOG();

    return xpsTunnelDeInitScope(XP_SCOPE_DEFAULT);
}

XP_STATUS xpsTunnelDeInitScope(xpsScope_t scopeId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS rt = XP_NO_ERR;

    /* Commenting Old XPS code. Some of the below functionality is required
       to handle Tunnel Origination. Hence commented for time being.*/
#if 0
    rt = xpsTnlDestroyStaticInsertionIds(scopeId);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Tunnel Destroy static insertion ids failed");
        return rt;
    }

    if (scopeId == XP_SCOPE_DEFAULT)
    {
        rt = (XP_STATUS)xpTunnelMgr::instance()->deInit();
        if (rt != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Tunnel manager de-initialization failed");
            return rt;
        }
    }

    rt = xpsStateDeRegisterDb(scopeId, &tnlNexthopEntryDbHndl);
    if (rt != XP_NO_ERR)
    {
        return rt;
    }
#endif

    rt = xpsStateDeRegisterDb(scopeId, &tnlEntryDbHndl);
    if (rt != XP_NO_ERR)
    {
        return rt;
    }

    rt = xpsStateDeRegisterDb(scopeId, &ipTnlGblDbHndl);
    if (rt != XP_NO_ERR)
    {
        return rt;
    }

    rt = xpsStateDeRegisterDb(scopeId, &ipTnlStartGblDbHndl);
    if (rt != XP_NO_ERR)
    {
        return rt;
    }

#if 0
    rt = xpsStateDeRegisterDb(scopeId, &tnlStaticDataDbHndl);
    if (rt != XP_NO_ERR)
    {
        return rt;
    }
#endif

    xpsLockDestroy(XP_LOCKINDEX_XPS_TUNNEL);

    XPS_FUNC_EXIT_LOG();
    return rt;
}

XP_STATUS xpsTunnelAddDevice(xpsDevice_t devId, xpsInitType_t initType)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS rt = XP_NO_ERR;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        return XP_ERR_INVALID_DEV_ID;
    }

    rt = xpsStateInsertDbForDevice(devId, tnlEntryDbHndl, &tnlDbEntryCompare);
    if (rt != XP_NO_ERR)
    {
        return rt;
    }

    XPS_FUNC_EXIT_LOG();

    return rt;
}

XP_STATUS xpsTunnelRemoveDevice(xpsDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS rt = XP_NO_ERR;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        return XP_ERR_INVALID_DEV_ID;
    }


    XPS_FUNC_EXIT_LOG();

    return rt;
}

XP_STATUS xpsTunnelEnableTermination(xpsDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsTunnelDisableTermination(xpsDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsTunnelHandleTunnelIvifTableRehash(xpsDevice_t devId,
                                               xpsHashIndexList_t *indexList)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsIpTunnelCreate(xpsScope_t scopeId, xpsInterfaceType_e type,
                            xpsIpTunnelData_t *data,
                            xpsInterfaceId_t *tnlIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsIpTunnelCreate);
    XP_STATUS rt = XP_NO_ERR;
    GT_STATUS rc = GT_OK;
    xpsIpTnlGblDbEntry_t *ipTnlGblDbEntry = NULL;
    xpsRBTree_t *rbtree = NULL;
    uint32_t devId = 0;

    if (data == NULL || tnlIntfId == NULL)
    {
        return XP_ERR_INVALID_ARG;
    }

    rt = xpsInterfaceCreateScope(scopeId, type, tnlIntfId);
    if (rt != XP_NO_ERR)
    {
        return rt;
    }

    rt = xpsStateHeapMalloc(sizeof(xpsIpTnlGblDbEntry_t),
                            (void **)&ipTnlGblDbEntry);
    if (rt != XP_NO_ERR)
    {
        return rt;
    }

    memset(ipTnlGblDbEntry, 0, sizeof(xpsIpTnlGblDbEntry_t));
    ipTnlGblDbEntry->vifId = (xpVif_t)*tnlIntfId;
    ipTnlGblDbEntry->l2HdrInsertionId = XPS_INVALID_INSPTR;
    ipTnlGblDbEntry->l3HdrInsertionId = XPS_INVALID_INSPTR;
    memcpy(&ipTnlGblDbEntry->tnlData, data, sizeof(xpsIpTunnelData_t));

    //Reuse the vxlanKeyCompare compare function as data-type is same
    if ((rt = xpsRBTInit(&rbtree, &tnlTermDbEntryCompare,
                         XPS_VERSION)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Global DB initialization failed");
        xpsStateHeapFree((void *)ipTnlGblDbEntry);
        return rt;
    }

    if (!rbtree)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Global DB, out of memory");
        xpsStateHeapFree((void *)ipTnlGblDbEntry);
        return XP_ERR_OUT_OF_MEM;
    }
    ipTnlGblDbEntry->termEntryList = rbtree;

    /* Allocate L2ECMP EPORT (primary ePort) for the tunnel */

    rt = xpsAllocatorAllocateId(scopeId, XP_ALLOC_L2_ECMP_EPORT,
                                &(ipTnlGblDbEntry->primaryEport));
    if (rt != XP_NO_ERR)
    {
        XPS_FUNC_EXIT_LOG();
        xpsRBTDelete(ipTnlGblDbEntry->termEntryList);
        xpsStateHeapFree((void *)ipTnlGblDbEntry);
        return rt;
    }

    rt = xpsStateInsertData(scopeId, ipTnlGblDbHndl, (void *)ipTnlGblDbEntry);
    if (rt != XP_NO_ERR)
    {
        xpsRBTDelete(ipTnlGblDbEntry->termEntryList);
        xpsStateHeapFree((void *)ipTnlGblDbEntry);
        return rt;
    }

    rc = cpssHalPclPortIngressPolicyEnable(devId, ipTnlGblDbEntry->primaryEport,
                                           GT_TRUE);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " cpssHalPclPortIngressPolicyEnable err: %d ePort :%d \n ", rc,
              ipTnlGblDbEntry->primaryEport);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    rc = cpssHalL3EnableUcRoutingOnPort(devId, ipTnlGblDbEntry->primaryEport,
                                        CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " cpssHalL3EnableUcRoutingOnPort err: %d ePort :%d \n ", rc,
              ipTnlGblDbEntry->primaryEport);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    rc = cpssHalL3EnableUcRoutingOnPort(devId, ipTnlGblDbEntry->primaryEport,
                                        CPSS_IP_PROTOCOL_IPV6_E, GT_TRUE);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " cpssHalL3EnableUcRoutingOnPort err: %d ePort :%d \n ", rc,
              ipTnlGblDbEntry->primaryEport);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    // Enable the FDB based UC routing
    rc =  cpssHalL3EnableBridgeFdbRoutingOnPort(devId,
                                                ipTnlGblDbEntry->primaryEport,
                                                CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set the FDB based UC routing enable for port %d\n",
              ipTnlGblDbEntry->primaryEport);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    rc =  cpssHalL3EnableBridgeFdbRoutingOnPort(devId,
                                                ipTnlGblDbEntry->primaryEport,
                                                CPSS_IP_PROTOCOL_IPV6_E, GT_TRUE);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set the FDB based UC routing enable for port %d\n",
              ipTnlGblDbEntry->primaryEport);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    rc = cpssHalTunnelSetEPortMeshId(devId, ipTnlGblDbEntry->primaryEport, 1);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set meshId for port %d\n",
              ipTnlGblDbEntry->primaryEport);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    XPS_FUNC_EXIT_LOG();
    return rt;
}

XP_STATUS xpsIpTunnelDelete(xpsScope_t scopeId, xpsInterfaceId_t tnlIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsIpTunnelDelete);
    XP_STATUS rt = XP_NO_ERR;
    GT_STATUS rc = GT_OK;
    xpsIpTnlGblDbEntry_t *ipTnlGblDbEntry;
    xpsIpTnlGblDbEntry_t keyIpTnlGblDbEntry;
    keyIpTnlGblDbEntry.vifId = (xpVif_t)tnlIntfId;
    uint32_t devId = 0;

    rt = xpsStateDeleteData(scopeId, ipTnlGblDbHndl,
                            (xpsDbKey_t)&keyIpTnlGblDbEntry, (void **)&ipTnlGblDbEntry);
    if (rt != XP_NO_ERR)
    {
        return rt;
    }

    if (ipTnlGblDbEntry == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Tunnel does not exist");
        rt = XP_ERR_NULL_POINTER;
        return rt;
    }

    rc = cpssHalPclPortIngressPolicyEnable(devId, ipTnlGblDbEntry->primaryEport,
                                           GT_FALSE);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " cpssHalPclPortIngressPolicyEnable err: %d ePort :%d \n ", rc,
              ipTnlGblDbEntry->primaryEport);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    rc = cpssHalL3EnableUcRoutingOnPort(devId, ipTnlGblDbEntry->primaryEport,
                                        CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " cpssHalPclPortIngressPolicyEnable err: %d ePort :%d \n ", rc,
              ipTnlGblDbEntry->primaryEport);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    rc = cpssHalL3EnableUcRoutingOnPort(devId, ipTnlGblDbEntry->primaryEport,
                                        CPSS_IP_PROTOCOL_IPV6_E, GT_FALSE);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " cpssHalPclPortIngressPolicyEnable err: %d ePort :%d \n ", rc,
              ipTnlGblDbEntry->primaryEport);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    // Enable the FDB based UC routing
    rc =  cpssHalL3EnableBridgeFdbRoutingOnPort(devId,
                                                ipTnlGblDbEntry->primaryEport,
                                                CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set the FDB based UC routing enable for port %d\n",
              ipTnlGblDbEntry->primaryEport);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    rc =  cpssHalL3EnableBridgeFdbRoutingOnPort(devId,
                                                ipTnlGblDbEntry->primaryEport,
                                                CPSS_IP_PROTOCOL_IPV6_E, GT_FALSE);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set the FDB based UC routing enable for port %d\n",
              ipTnlGblDbEntry->primaryEport);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    rc = cpssHalTunnelSetEPortMeshId(devId, ipTnlGblDbEntry->primaryEport, 0);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set meshId for port %d\n",
              ipTnlGblDbEntry->primaryEport);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    rt = xpsAllocatorReleaseId(scopeId, XP_ALLOC_L2_ECMP_EPORT,
                               ipTnlGblDbEntry->primaryEport);
    if (rt != XP_NO_ERR)
    {
        XPS_FUNC_EXIT_LOG();
        return rt;
    }


    if (ipTnlGblDbEntry->termEntryList)
    {
        rt = xpsRBTDelete(ipTnlGblDbEntry->termEntryList);
        if (rt != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "RBTDel failed");
            return rt;
        }
    }

    rt = xpsInterfaceDestroyScope(scopeId, tnlIntfId);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Interface destroy failed, tunnel interface(%d)", tnlIntfId);
        return rt;
    }

    rt = xpsStateHeapFree((void *)ipTnlGblDbEntry);
    if (rt != XP_NO_ERR)
    {
        return rt;
    }

    XPS_FUNC_EXIT_LOG();

    return rt;
}

/**
 * \private
 * \fn xpsAllocateRouterArpDaAndTunnelStartIndex
 * \brief allocate Arp Da Index
 *
 * Note: Tunnel Start is not supported.
 *
 * \param [in] devId
 * * \param [out] index
 * \return xp_status
 */
XP_STATUS xpsAllocateTunnelStartIndex(xpsDevice_t xpsDevId,
                                      xpIpPrefixType_t type, uint32_t* index)
{
    xpsScope_t                    scope;
    xpsTnlStartAllocGblDbEntry_t *tsGblDbEntry;
    XP_STATUS                     rt;

    /* Get Scope Id from devId */
    if ((rt = xpsScopeGetScopeId(xpsDevId, &scope))!= XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " Tunnel Start scope get failed");
        return rt;
    }

    rt = xpsStateHeapMalloc(sizeof(xpsTnlStartAllocGblDbEntry_t),
                            (void **)&tsGblDbEntry);
    if (rt != XP_NO_ERR)
    {
        return rt;
    }

    rt = xpsAllocatorAllocateId(scope,
                                (type == XP_PREFIX_TYPE_IPV4) ? XPS_ALLOCATOR_TUNNEL_START_v4 :
                                XPS_ALLOCATOR_TUNNEL_START_v6, index);
    if (rt != XP_NO_ERR)
    {
        xpsStateHeapFree(tsGblDbEntry);
        return rt;
    }

    if (type == XP_PREFIX_TYPE_IPV6)
    {
        *index = tunnelStartIpv6LastIdx_g - (2 * (*index));
    }

    tsGblDbEntry->index = *index;
    tsGblDbEntry->type = type;

    rt = xpsStateInsertData(scope, ipTnlStartGblDbHndl, (void *)tsGblDbEntry);
    if (rt != XP_NO_ERR)
    {
        if (type == XP_PREFIX_TYPE_IPV6)
        {
            *index = (tunnelStartIpv6LastIdx_g - *index)/2;
        }
        xpsAllocatorReleaseId(scope,
                              (type == XP_PREFIX_TYPE_IPV4) ? XPS_ALLOCATOR_TUNNEL_START_v4 :
                              XPS_ALLOCATOR_TUNNEL_START_v6, *index);
        xpsStateHeapFree(tsGblDbEntry);
        return rt;
    }
    return XP_NO_ERR;
}

/**
 * \private
 * \brief Release tunnel start index
 *
 * \param [in] devId
 * * \param [out] index
 * \return xp_status
 */
XP_STATUS xpsReleaseTunnelStartIndex(xpsDevice_t xpsDevId,
                                     uint32_t index)
{
    xpsScope_t                    scope;
    xpsTnlStartAllocGblDbEntry_t *tsGblDbEntry = NULL, key;
    XP_STATUS                     rt;
    //uint32_t                      offset;

    /* Get Scope Id from devId */
    if ((rt = xpsScopeGetScopeId(xpsDevId, &scope))!= XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " Tunnel Start %d scope get failed", index);
        return rt;
    }

    key.index = index;

    rt = xpsStateSearchData(scope, ipTnlStartGblDbHndl, (xpsDbKey_t)&key,
                            (void **)&tsGblDbEntry);
    if ((rt != XP_NO_ERR) || (tsGblDbEntry == NULL))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " Tunnel Start %d entry not found is failed", index);
        return rt;
    }

    if (tsGblDbEntry->type == XP_PREFIX_TYPE_IPV6)
    {
        index = (tunnelStartIpv6LastIdx_g - index)/2;
    }

    rt = xpsAllocatorReleaseId(scope,
                               (tsGblDbEntry->type == XP_PREFIX_TYPE_IPV4) ? XPS_ALLOCATOR_TUNNEL_START_v4 :
                               XPS_ALLOCATOR_TUNNEL_START_v6, index);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " Tunnel Start %d pointer release is failed", index);
    }

    rt = xpsStateDeleteData(scope, ipTnlStartGblDbHndl, (xpsDbKey_t)&key,
                            (void **)&tsGblDbEntry);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " Tunnel Start %d entry free is failed", index);
        return rt;
    }

    return xpsStateHeapFree(tsGblDbEntry);
}

XP_STATUS xpsIpTunnelUpdateTunnelStart(xpsDevice_t devId,
                                       xpsInterfaceId_t intfId,
                                       XpIpTunnelMode_t *encapTtlMode,
                                       uint32_t encapTtl,
                                       XpIpTunnelMode_t *encapDscpMode,
                                       uint32_t encapDscp,
                                       xpsVxlanUdpSrcPortMode_t *udpSrcPortMode,
                                       uint16_t udpSrcPort)
{
    XP_STATUS rt = XP_NO_ERR;
    GT_STATUS status = GT_OK;
    xpsIpTnlGblDbEntry_t                *ipTnlGblEntry = NULL;
    xpsScope_t                          scopeId;
    CPSS_DXCH_TUNNEL_START_CONFIG_UNT   tunnel_start_entry;
    CPSS_TUNNEL_TYPE_ENT                tunnel_start_type;
    memset(&tunnel_start_entry, 0, sizeof(CPSS_DXCH_TUNNEL_START_CONFIG_UNT));

    /* Get Scope Id from devId */
    if ((rt = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return rt;
    }

    rt = xpsIpTunnelGblGetDbEntry(scopeId, intfId, &ipTnlGblEntry);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "TunnelGet failed : %d ",
              intfId);
        return rt;
    }

    if (ipTnlGblEntry->tnlData.lclEpIpAddr.type == XP_PREFIX_TYPE_IPV4)
    {
        tunnel_start_type = CPSS_TUNNEL_GENERIC_IPV4_E;
    }
    else
    {
        tunnel_start_type = CPSS_TUNNEL_GENERIC_IPV6_E;
    }

    for (uint32_t i = 0; i < ipTnlGblEntry->numOfEports; i++)
    {

        status = cpssHalTunnelTunnelStartEntryRead(devId,
                                                   ipTnlGblEntry->ePorts[i].tnlStartId,
                                                   &tunnel_start_type,
                                                   &tunnel_start_entry);
        if (status != GT_OK)
        {
            rt = XP_ERR_INVALID_PARAMS;
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Tunnel start read failed");
            return rt;
        }

        if (encapTtlMode)
        {
            if (*encapTtlMode == XP_IP_TUNNEL_MODE_UNIFORM)
            {
                if (tunnel_start_type == CPSS_TUNNEL_GENERIC_IPV4_E)
                {
                    tunnel_start_entry.ipv4Cfg.ttl = 0;
                }
                else
                {
                    tunnel_start_entry.ipv6Cfg.ttl = 0;
                }
            }
            else
            {
                if (tunnel_start_type == CPSS_TUNNEL_GENERIC_IPV4_E)
                {
                    tunnel_start_entry.ipv4Cfg.ttl = encapTtl;
                }
                else
                {
                    tunnel_start_entry.ipv6Cfg.ttl = encapTtl;
                }
            }
        }

        if (encapDscpMode)
        {
            if (*encapDscpMode == XP_IP_TUNNEL_MODE_UNIFORM)
            {
                if (tunnel_start_type == CPSS_TUNNEL_GENERIC_IPV4_E)
                {
                    tunnel_start_entry.ipv4Cfg.dscpMarkMode =
                        CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_PACKET_QOS_PROFILE_E;
                    tunnel_start_entry.ipv4Cfg.dscp = encapDscp;
                }
                else
                {
                    tunnel_start_entry.ipv6Cfg.dscpMarkMode =
                        CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_PACKET_QOS_PROFILE_E;
                    tunnel_start_entry.ipv6Cfg.dscp = encapDscp;
                }
            }
            else
            {
                if (tunnel_start_type == CPSS_TUNNEL_GENERIC_IPV4_E)
                {
                    tunnel_start_entry.ipv4Cfg.dscpMarkMode =
                        CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
                    tunnel_start_entry.ipv4Cfg.dscp = encapDscp;
                }
                else
                {
                    tunnel_start_entry.ipv6Cfg.dscpMarkMode =
                        CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
                    tunnel_start_entry.ipv6Cfg.dscp = encapDscp;
                }
            }
        }

        if (udpSrcPortMode)
        {
            if (*udpSrcPortMode == XPS_VXLAN_UDP_PORT_HASH)
            {
                if (tunnel_start_type == CPSS_TUNNEL_GENERIC_IPV4_E)
                {
                    tunnel_start_entry.ipv4Cfg.udpSrcPort = 0;
                    tunnel_start_entry.ipv4Cfg.profileIndex =
                        CPSSHAL_GEN_TUNNEL_PROFILE_VXLAN_UDP_PORT_FROM_HASH;
                }
                else
                {
                    tunnel_start_entry.ipv6Cfg.udpSrcPort = 0;
                    tunnel_start_entry.ipv6Cfg.profileIndex =
                        CPSSHAL_GEN_TUNNEL_PROFILE_VXLAN_UDP_PORT_FROM_HASH;
                }
            }
            else
            {
                if (tunnel_start_type == CPSS_TUNNEL_GENERIC_IPV4_E)
                {
                    tunnel_start_entry.ipv4Cfg.udpSrcPort = udpSrcPort;
                    tunnel_start_entry.ipv4Cfg.profileIndex =
                        CPSSHAL_GEN_TUNNEL_PROFILE_VXLAN_UDP_PORT_FROM_TS;
                }
                else
                {
                    tunnel_start_entry.ipv6Cfg.udpSrcPort = udpSrcPort;
                    tunnel_start_entry.ipv6Cfg.profileIndex =
                        CPSSHAL_GEN_TUNNEL_PROFILE_VXLAN_UDP_PORT_FROM_TS;
                }
            }
        }

        status = cpssHalTunnelTunnelStartEntryWrite(devId,
                                                    ipTnlGblEntry->ePorts[i].tnlStartId,
                                                    tunnel_start_type,
                                                    &tunnel_start_entry);
        if (status != GT_OK)
        {
            rt = XP_ERR_INVALID_PARAMS;
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Tunnel start Update failed");
            return rt;
        }
    }

    if (encapTtlMode)
    {
        ipTnlGblEntry->tnlData.ecpTtlMode = *encapTtlMode;
    }
    if (encapDscpMode)
    {
        ipTnlGblEntry->tnlData.ecpDscpMode = *encapDscpMode;
    }

    if (encapTtl)
    {
        ipTnlGblEntry->tnlData.ecpTtl = encapTtl;
    }
    if (encapDscp)
    {
        ipTnlGblEntry->tnlData.ecpDscp = encapDscp;
    }

    if (udpSrcPortMode)
    {
        ipTnlGblEntry->tnlData.cfg.vxlanCfg.udpSrcPortMode = *udpSrcPortMode;
    }

    if (udpSrcPort)
    {
        ipTnlGblEntry->tnlData.cfg.vxlanCfg.udpSrcPort = udpSrcPort;
    }

    return rt;
}

XP_STATUS xpsIpTunnelFindTermEntry(xpsScope_t scopeId,
                                   xpsInterfaceId_t tnlIntfId,
                                   uint32_t termId,
                                   xpsIpTnlTermEntry_t **ppInfo)
{
    XP_STATUS      status   = XP_NO_ERR;
    xpsIpTnlTermEntry_t childKey;
    xpsIpTnlGblDbEntry_t *lookupEntry = NULL;

    status = xpsIpTunnelGblGetDbEntry(scopeId, tnlIntfId, &lookupEntry);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Tunnel Not found :%d \n", tnlIntfId);
        return status;
    }

    childKey.termEntryId = termId;

    *ppInfo = (xpsIpTnlTermEntry_t*)xpsRBTSearchNode(lookupEntry->termEntryList,
                                                     (xpsDbKey_t)&childKey);

    if (*ppInfo == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEFAULT,
              "Ket not found(%d) failed", termId);
        return XP_ERR_KEY_NOT_FOUND;
    }

    return status;
}

XP_STATUS xpsIpTunnelAddToTermList(xpsScope_t scopeId,
                                   xpsInterfaceId_t tnlIntfId,
                                   uint32_t termEntryId,
                                   xpsIpTnlTermEntry_t **termInfo)
{
    XP_STATUS           status      = XP_NO_ERR;
    xpsIpTnlGblDbEntry_t *lookupEntry = NULL;

    status = xpsIpTunnelGblGetDbEntry(scopeId, tnlIntfId, &lookupEntry);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Tunnel Not found :%d \n", tnlIntfId);
        return status;
    }

    // Create a new child Interface Info structure
    if ((status = xpsStateHeapMalloc(sizeof(xpsIpTnlTermEntry_t),
                                     (void**)termInfo)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Could not allocate interface(%d) failed", termEntryId);
        return status;
    }

    memset(*termInfo, 0, sizeof(xpsIpTnlTermEntry_t));
    (*termInfo)->termEntryId = termEntryId;

    // Add a new node
    status = xpsRBTAddNode(lookupEntry->termEntryList, (void*)(*termInfo));
    if ((status != XP_NO_ERR))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Add node failedi :%d ",
              status);
        return status;
    }

    return status;
}

XP_STATUS xpsIpTunnelDelFromTermList(xpsScope_t scopeId,
                                     xpsInterfaceId_t tnlIntfId, uint32_t termEntryId)
{
    XP_STATUS           status  = XP_NO_ERR;
    xpsIpTnlTermEntry_t* termInfo = NULL;
    xpsIpTnlTermEntry_t childKey;
    xpsIpTnlGblDbEntry_t *lookupEntry = NULL;

    status = xpsIpTunnelGblGetDbEntry(scopeId, tnlIntfId, &lookupEntry);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Tunnel Not found :%d \n", tnlIntfId);
        return status;
    }

    status = xpsIpTunnelFindTermEntry(scopeId, lookupEntry->vifId, termEntryId,
                                      &termInfo);
    if (status != XP_NO_ERR || termInfo == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "tnl Term not found (%d) in tnl", termEntryId, lookupEntry->vifId);
    }
    childKey.termEntryId = termEntryId;
    termInfo = (xpsIpTnlTermEntry_t*)xpsRBTDeleteNode(lookupEntry->termEntryList,
                                                      (xpsDbKey_t)&childKey);

    // Remove the corresponding state
    if (termInfo == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Ket not found(%d) failed", termEntryId);
        return XP_ERR_NOT_FOUND;
    }

    // Free the memory allocated for the corresponding state
    if ((status = xpsStateHeapFree((void *)termInfo)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "failed for free (%d) failed", termEntryId);
        return status;
    }
    return status;
}

XP_STATUS xpsIpTunnelGetNextTermEntry(xpsScope_t scopeId,
                                      xpsInterfaceId_t tnlIntfId,
                                      xpsIpTnlTermEntry_t **nextInfo)
{
    XP_STATUS status = XP_NO_ERR;
    xpsIpTnlGblDbEntry_t *lookupEntry = NULL;

    status = xpsIpTunnelGblGetDbEntry(scopeId, tnlIntfId, &lookupEntry);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Vxlan Tunnel Not found :%d \n", tnlIntfId);
        return status;
    }

    *nextInfo = (xpsIpTnlTermEntry_t*)xpsRBTGetInorderSuccessor(
                    lookupEntry->termEntryList,
                    (xpsDbKey_t)*nextInfo);

    if (*nextInfo == NULL)
    {
        return XP_ERR_NULL_POINTER;
    }
    return XP_NO_ERR;
}


XP_STATUS xpsIpTunnelUpdateTerminationAction(xpsDevice_t devId,
                                             xpsInterfaceId_t tnlIntfId,
                                             XpIpTunnelMode_t *decapTtlMode,
                                             XpIpTunnelMode_t *decapDscpMode)
{
    XPS_FUNC_ENTRY_LOG();
    XPS_LOCK(xpsIpTunnelUpdateTermination);

    GT_STATUS   rc = GT_OK;
    XP_STATUS rt = XP_NO_ERR;
    xpsIpTnlGblDbEntry_t *ipTnlGblEntry = NULL;
    xpsScope_t scopeId;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ID RuleId = 0xFFFFFFFF;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_TYPE_STC actionType;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_DATA_STC actionData;
    CPSS_DXCH_TTI_ACTION_STC                    ttiAction;

    memset(&actionType, 0, sizeof(actionType));
    memset(&actionData, 0, sizeof(actionData));
    memset(&ttiAction, 0, sizeof(ttiAction));

    if (decapTtlMode == NULL &&
        decapDscpMode == NULL)
    {
        return XP_ERR_NULL_POINTER;
    }

    if ((rt = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return rt;
    }

    rt = xpsIpTunnelGblGetDbEntry(scopeId, tnlIntfId, &ipTnlGblEntry);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "TunnelGet failed : %d ",
              tnlIntfId);
        return rt;
    }

    /* DSCP modify is not supported for M0 devices.
     * Update xps DB and return success.
     */
    if ((decapDscpMode) &&
        (cpssHalDevPPFamilyGet(devId) != CPSS_PP_FAMILY_DXCH_FALCON_E))
    {
        ipTnlGblEntry->tnlData.dcpDscpMode = *decapDscpMode;
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEFAULT,
              "DSCP Modify not supported in HW. Return Success !!!");
        return XP_NO_ERR;
    }
    xpsIpTnlTermEntry_t *termInfo = NULL;
    while (xpsIpTunnelGetNextTermEntry(scopeId, tnlIntfId,
                                       &termInfo) == XP_NO_ERR)
    {
        for (uint32_t i = 0; i < termInfo->dcpEntryCnt; i++)
        {
            for (uint32_t j = 0; j < termInfo->entry[i].ruleCnt; j++)
            {
                RuleId = termInfo->entry[i].rule[j].ttiRuleId;
                actionType.ruleType = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_TTI_E;
                actionData.action.tti.actionPtr = &ttiAction;

                rc = cpssHalTunnelTerminationActionGet(XPS_GLOBAL_TACM_MGR,
                                                       TUNNEL_TERM_TABLE_ID, RuleId,
                                                       &actionType, &actionData);
                if (rc != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "cpssHalTunnelTerminationActionGet failed err: %d ruleId :%d \n ", rc,
                          tnlIntfId);
                    return xpsConvertCpssStatusToXPStatus(rc);
                }

                if (decapTtlMode)
                {
                    if (*decapTtlMode == XP_IP_TUNNEL_MODE_UNIFORM)
                    {
                        actionData.action.tti.actionPtr->copyTtlExpFromTunnelHeader = GT_TRUE;
                    }
                    else
                    {
                        actionData.action.tti.actionPtr->copyTtlExpFromTunnelHeader = GT_FALSE;
                    }
                }

                if (decapDscpMode)
                {
                    if (*decapDscpMode == XP_IP_TUNNEL_MODE_UNIFORM)
                    {
                        actionData.action.tti.actionPtr->modifyDscp =
                            CPSS_DXCH_TTI_MODIFY_DSCP_ENABLE_E;
                    }
                    else
                    {
                        actionData.action.tti.actionPtr->modifyDscp =
                            CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_DSCP_E;
                    }
                }

                rc = cpssHalTunnelTerminationActionUpdate(XPS_GLOBAL_TACM_MGR,
                                                          TUNNEL_TERM_TABLE_ID, RuleId,
                                                          &actionType, &actionData);
                if (rc != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "cpssHalTunnelTerminationActionUpdate failed err: %d ruleId :%d \n ", rc,
                          tnlIntfId);
                    return xpsConvertCpssStatusToXPStatus(rc);
                }
            }
        }
    }

    if (decapTtlMode)
    {
        ipTnlGblEntry->tnlData.dcpTtlMode = *decapTtlMode;
    }
    if (decapDscpMode)
    {
        ipTnlGblEntry->tnlData.dcpDscpMode = *decapDscpMode;
    }

    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "cpssHalTunnelTerminationEntryUpdate Success Intf %d Rule %d\n", tnlIntfId,
          RuleId);

    XPS_FUNC_EXIT_LOG();

    return rt;
}

XP_STATUS xpsIpTunnelDeleteTermination(xpsDevice_t devId,
                                       xpsInterfaceId_t intfId,
                                       uint32_t tnlTermId)
{
    XPS_FUNC_ENTRY_LOG();
    XPS_LOCK(xpsIpTunnelDeleteTermination);

    GT_STATUS   rc = GT_OK;
    XP_STATUS rt = XP_NO_ERR;
    xpsIpTnlGblDbEntry_t *ipTnlGblEntry = NULL;
    xpsScope_t scopeId;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ID RuleId = 0xFFFFFFFF;

    if ((rt = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return rt;
    }

    rt = xpsIpTunnelGblGetDbEntry(scopeId, intfId, &ipTnlGblEntry);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "TunnelGet failed : %d ",
              intfId);
        return rt;
    }

    xpsIpTnlTermEntry_t* termInfo = NULL;
    rt = xpsIpTunnelFindTermEntry(scopeId, intfId, tnlTermId,
                                  &termInfo);
    if (rt != XP_NO_ERR || termInfo == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "tnl Term not found (%d) (%d)", intfId, tnlTermId);
    }

    for (uint32_t i =0 ; i < termInfo->dcpEntryCnt; i++)
    {
        for (uint32_t j=0; j <  termInfo->entry[i].ruleCnt; j++)
        {
            RuleId = termInfo->entry[i].rule[j].ttiRuleId;

            rc = cpssHalTunnelTerminationEntryDelete(XPS_GLOBAL_TACM_MGR,
                                                     TUNNEL_TERM_TABLE_ID, RuleId);
            if (rc != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssHalTunnelTerminationEntryDelete failed err: %d ruleId :%d \n ", rc,
                      intfId);
                return xpsConvertCpssStatusToXPStatus(rc);
            }

            rt = xpsAllocatorReleaseId(scopeId, XP_ALLOC_TUNNEL_TERM_HW_ENTRY, RuleId);
            if (rt != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Unable to release tunnelId: %u | RuleId :%d | error: %d.\n",
                      intfId, RuleId, rt);
                return rt;
            }

            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
                  "cpssHalTunnelTerminationEntryDelete Success Intf %d Rule %d\n", intfId,
                  RuleId);
        }
    }

    XPS_FUNC_EXIT_LOG();

    return rt;
}

/*
   Below API adds Tunnel Termination Entries.
   Since there is no update from NOS on passenger protocol type,
   install two rules, one for each passenger pkt
   IPv4 and second for IPv6, extend if required for
   additional tnl passenger.
 */

XP_STATUS xpsIpTunnelAddTermination(xpsDevice_t devId,
                                    xpsInterfaceId_t intfId,
                                    uint32_t tnlTermId)
{
    XPS_FUNC_ENTRY_LOG();
    XPS_LOCK(xpsIpTunnelAddTermination);

    GT_STATUS   rc = GT_OK;
    XP_STATUS rt = XP_NO_ERR;
    xpsIpTnlGblDbEntry_t *ipTnlGblEntry = NULL;
    xpsScope_t scopeId;
    uint32_t ruleIdx = 0;

    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC       TcamRuleType;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ID             RuleId = 0XFFFFFFFF;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC RuleAttributes;
    CPSS_DXCH_TTI_ACTION_STC                   ttiAction;
    CPSS_DXCH_TTI_RULE_UNT                     ttiMask;
    CPSS_DXCH_TTI_RULE_UNT                     ttiPattern;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC       RuleData;
    int i = 0;

    if ((rt = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return rt;
    }

    rt = xpsIpTunnelGblGetDbEntry(scopeId, intfId, &ipTnlGblEntry);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "TunnelGet failed : %d ",
              intfId);
        return rt;
    }

    xpsIpTnlTermEntry_t* termInfo = NULL;
    rt = xpsIpTunnelFindTermEntry(scopeId, intfId, tnlTermId,
                                  &termInfo);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEFAULT,
              "tnl Term not found (%d) (%d)", intfId, tnlTermId);
    }

    if (termInfo == NULL)
    {
        rt = xpsIpTunnelAddToTermList(scopeId, intfId, tnlTermId, &termInfo);
        if (rt != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Tnl Term Add failed, vni(%d)", tnlTermId);
            return rt;
        }
    }

    ruleIdx = termInfo->entry[termInfo->dcpEntryCnt].ruleCnt;

    /* Logical Index based TCam */
    RuleAttributes.priority          = 0; /* Not be used */
    RuleData.valid                   = GT_TRUE;
    TcamRuleType.ruleType = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_TTI_E;

    cpssOsMemSet(&ttiMask, 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
    cpssOsMemSet(&ttiPattern, 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
    cpssOsMemSet(&ttiAction, 0, sizeof(CPSS_DXCH_TTI_ACTION_STC));

    if (ttiPattern.ipv4.common.vid != 0)
    {
        ttiMask.ipv4.common.vid = 0xfff;
    }

    if (ipTnlGblEntry->tnlData.lclEpIpAddr.type == XP_PREFIX_TYPE_IPV6)
    {
        TcamRuleType.rule.tti.ruleFormat = CPSS_DXCH_TTI_RULE_UDB_30_E;

        /* PCL-ID pattern*/
        ttiPattern.udbArray.udb[0] = 0x1;
        ttiMask.udbArray.udb[0]    = 0x1F;

        /*
           Passenger Tunnel Protocol pattern.
           Passenger is V4
         */
        ttiPattern.udbArray.udb[1] = 0x0;
        ttiMask.udbArray.udb[1]    = 0x6;
        /*IPv6 DIP pattern*/
        for (i = 0; i < 16; i++)
        {
            ttiPattern.udbArray.udb[i+2] =
                ipTnlGblEntry->tnlData.lclEpIpAddr.addr.ipv6Addr[i];
            ttiMask.udbArray.udb[i+2] = 0xFF;
        }
    }
    else
    {
        TcamRuleType.rule.tti.ruleFormat = CPSS_DXCH_TTI_RULE_IPV4_E;

        ttiPattern.ipv4.common.pclId  = CPSS_HAL_TTI_IPV4_PCL_ID;

        COPY_IPV4_ADDR_T(&ttiPattern.ipv4.destIp,
                         ipTnlGblEntry->tnlData.lclEpIpAddr.addr.ipv4Addr);
        COPY_IPV4_ADDR_T(&ttiPattern.ipv4.srcIp,
                         ipTnlGblEntry->tnlData.rmtEpIpAddr.addr.ipv4Addr);

        if (ttiPattern.ipv4.destIp.u32Ip != 0)
        {
            ttiMask.ipv4.destIp.u32Ip = 0xffffffff;
        }
        if (ttiPattern.ipv4.srcIp.u32Ip != 0)
        {
            ttiMask.ipv4.srcIp.u32Ip = 0xffffffff;
        }

        /* Passenger is V4*/
        ttiPattern.ipv4.tunneltype = 0;
        ttiMask.ipv4.tunneltype = 0x6;
    }

    ttiAction.command                       = CPSS_PACKET_CMD_FORWARD_E;
    ttiAction.redirectCommand               = CPSS_DXCH_TTI_NO_REDIRECT_E;
    ttiAction.userDefinedCpuCode            = CPSS_NET_FIRST_USER_DEFINED_E;
    ttiAction.tunnelTerminate               = GT_TRUE;
    ttiAction.ttPassengerPacketType         = CPSS_DXCH_TTI_PASSENGER_IPV4V6_E;
    ttiAction.tag0VlanPrecedence            =
        CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;
    ttiAction.tag0VlanCmd                   = CPSS_DXCH_TTI_VLAN_DO_NOT_MODIFY_E;
    ttiAction.tag1VlanCmd                   = CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E;
    ttiAction.qosPrecedence                 =
        CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
    ttiAction.keepPreviousQoS               = GT_TRUE;;
    ttiAction.modifyTag0Up                  = CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_UP_E;
    ttiAction.modifyDscp                    =
        CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_DSCP_E;

    ttiAction.setMacToMe                    = GT_TRUE;

    if (ipTnlGblEntry->tnlData.dcpTtlMode == XP_IP_TUNNEL_MODE_UNIFORM)
    {
        ttiAction.copyTtlExpFromTunnelHeader = GT_TRUE;
    }

    if (ipTnlGblEntry->tnlData.dcpDscpMode == XP_IP_TUNNEL_MODE_UNIFORM)
    {
        ttiAction.modifyDscp = CPSS_DXCH_TTI_MODIFY_DSCP_ENABLE_E;
    }

    RuleData.rule.tti.actionPtr     = &ttiAction;
    RuleData.rule.tti.maskPtr       = &ttiMask;
    RuleData.rule.tti.patternPtr    = &ttiPattern;

    rt = xpsAllocatorAllocateId(scopeId, XP_ALLOC_TUNNEL_TERM_HW_ENTRY, &RuleId);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsAllocatorAllocateId: Unable to allocate HW Tunnel id, error: %d.\n", rt);
        return rt;
    }

    if (RuleId >= curMaxTnlTermEntries)
    {
        rc = cpssHalVirtualTcamResize(XPS_GLOBAL_TACM_MGR, TUNNEL_TERM_TABLE_ID,
                                      CPSS_DXCH_VIRTUAL_TCAM_APPEND_CNS, GT_TRUE, 128);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssHalVirtualTcamResize failed err: %d ruleId :%d \n ", rc, intfId);
            return xpsConvertCpssStatusToXPStatus(rc);
        }
        curMaxTnlTermEntries += 128;
    }


    rc = cpssHalTunnelTerminationEntryAdd(XPS_GLOBAL_TACM_MGR, TUNNEL_TERM_TABLE_ID,
                                          RuleId, &RuleAttributes,
                                          &TcamRuleType, &RuleData);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssHalTunnelTerminationEntryAdd failed err: %d ruleId :%d \n ", rc,
              intfId);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    termInfo->entry[termInfo->dcpEntryCnt].rule[ruleIdx].ttiRuleId = RuleId;
    termInfo->entry[termInfo->dcpEntryCnt].rule[ruleIdx].intfId = 0;
    termInfo->entry[termInfo->dcpEntryCnt].ruleCnt++;
    termInfo->dcpEntryCnt++;

    /*
     * Checking the DSCP Decap mode supported by CPSS. Clean-up the code
     * based on the outcome.
    */
#ifdef ECN_CAP // Not used.
    CPSS_DXCH_TUNNEL_START_ECN_MODE_ENT curEcnMode, newEcnMode;
    newEcnMode = CPSS_DXCH_TUNNEL_START_ECN_COMPATIBILITY_MODE_E;

    if (ipTnlGblEntry->tnlData.dcpEcnMode == XP_IP_TUNNEL_ECN_MODE_COPY_FROM_OUTER)
    {
        newEcnMode = CPSS_DXCH_TUNNEL_START_ECN_NORMAL_MODE_E;
    }

    rc = cpssHalTunnelStartEcnModeGet(devId, &curEcnMode);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssHalTunnelStartEcnModeGet failed err: %d \n ", rc);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    /*
     * Note ECN Mode is global configuration.
     * Overwrite previous value will impact previuos ECN behavior of the
     * tunnels irrespective of configuration.
     * User should assign and makre sure same mode configured to all tunnels.
    */

    if (curEcnMode != newEcnMode)
    {
        rc = cpssHalTunnelStartEcnModeSet(devId, newEcnMode);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssHalTunnelStartEcnModeSet failed err: %d Mode %d \n ", rc, newEcnMode);
            return xpsConvertCpssStatusToXPStatus(rc);
        }
    }
#endif

    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "cpssHalTunnelTerminationEntryAdd Success %d RuleId :%d \n", intfId, RuleId);

    XPS_FUNC_EXIT_LOG();
    return rt;
}

// TODO: Handle Tunnel start.

XP_STATUS xpsIpTunnelAddOrigination(xpsDevice_t devId, xpsInterfaceId_t intfId,
                                    xpsInterfaceId_t baseIntfId, xpsIpTunnelData_t *data)
{
    XPS_FUNC_ENTRY_LOG();

    /* TODO Handle CPSS Tunnel Start here */


    XPS_LOCK(xpsIpTunnelAddOrigination);
    XP_STATUS rt = XP_NO_ERR;
    uint32_t tunnelStartId;
    xpsIpTnlGblDbEntry_t *ipTnlGblEntry = NULL;
    xpsScope_t scopeId;

    /* Get Scope Id from devId */
    if ((rt = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return rt;
    }


    rt = xpsIpTunnelGblGetDbEntry(scopeId, intfId, &ipTnlGblEntry);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "TunnelGet failed : %d ",
              intfId);
        return rt;
    }

    if ((rt = xpsAllocateTunnelStartIndex(scopeId, data->lclEpIpAddr.type,
                                          &tunnelStartId)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " Tunnel Start Pointer allocation is failed");
        return rt;
    }

    /* save allocated pointer */
    ipTnlGblEntry->l3HdrInsertionId = tunnelStartId;


    XPS_FUNC_EXIT_LOG();

    return rt;
}

XP_STATUS xpsIpTunnelAdd(xpsDevice_t devId, xpsInterfaceId_t intfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsIpTunnelAdd);
    XP_STATUS rt = XP_NO_ERR;
    xpsIpTnlGblDbEntry_t *ipTnlGblEntry;
    xpsScope_t scopeId;
#if 0
    xpIndexList_t index;
    xpIpTnlTerminationEntry_t tnlTermEntry;
    xpsHashIndexList_t hashIndexList;

    memset(&hashIndexList, 0, sizeof(hashIndexList));
    memset(&tnlTermEntry, 0, sizeof(xpIpTnlTerminationEntry_t));
#endif
    /* Get Scope Id from devId */
    if ((rt = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return rt;
    }

    rt = xpsIpTunnelGblGetDbEntry(scopeId, intfId, &ipTnlGblEntry);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "TunnelGet failed : %d ",
              intfId);
        return rt;
    }

    if (ipTnlGblEntry->tnlData.type == XP_IP_GENEVE_TUNNEL &&
        ipTnlGblEntry->tnlData.optionFormat != XPS_IP_GENEVE_FIXED_HDR_FORMAT)
    {
        rt = XP_ERR_INVALID_INPUT;
        return rt;
    }

    /* Set the tunnel orignation */
    rt = xpsIpTunnelAddOrigination(devId, intfId, 0, &ipTnlGblEntry->tnlData);
    if (rt != XP_NO_ERR)
    {
        return rt;
    }

#if 0
    // Multicast only creates origination

    if (xpsIsIPv4MulticastAddress(ipTnlGblEntry->tnlData.rmtEpIpAddr))
    {
        /*
         * Add the DB entry for the multicast tunnel entry. Used 0xFFFF as table id as multicast
         * entry doesnt have a termination table entry
         */
        rt = xpsIpTunnelDbAddEntry(devId, intfId, 0XFFFF, -1);
        if (rt != XP_NO_ERR)
        {
            return rt;
        }

        return rt;
    }

    /* Populate FL tunnel termination parameters */
    switch (ipTnlGblEntry->tnlData.type)
    {
        case XP_IP_VXLAN_TUNNEL:
            COPY_IPV4_ADDR_T(tnlTermEntry.key.vxlanTnl.rmtEpIpAddr,
                             ipTnlGblEntry->tnlData.rmtEpIpAddr);
            tnlTermEntry.data.vxlanTnl.tunnelVif = intfId;
            tnlTermEntry.data.vxlanTnl.spanState = SPAN_STATE_FORWARD;
            break;

        case XP_IP_NVGRE_TUNNEL:
            COPY_IPV4_ADDR_T(tnlTermEntry.key.nvgreTnl.rmtEpIpAddr,
                             ipTnlGblEntry->tnlData.rmtEpIpAddr);
            tnlTermEntry.data.nvgreTnl.tunnelVif = intfId;
            tnlTermEntry.data.nvgreTnl.spanState = SPAN_STATE_FORWARD;
            break;

        case XP_IP_OVER_IP_TUNNEL:
            COPY_IPV4_ADDR_T(tnlTermEntry.key.ipInIpTnl.lclEpIpAddr,
                             ipTnlGblEntry->tnlData.lclEpIpAddr);
            COPY_IPV4_ADDR_T(tnlTermEntry.key.ipInIpTnl.rmtEpIpAddr,
                             ipTnlGblEntry->tnlData.rmtEpIpAddr);
            tnlTermEntry.data.ipInIpTnl.tunnelVif = intfId;
            tnlTermEntry.data.ipInIpTnl.spanState = SPAN_STATE_FORWARD;
            break;

        case XP_IP_GRE_TUNNEL:
            COPY_IPV4_ADDR_T(tnlTermEntry.key.greTnl.lclEpIpAddr,
                             ipTnlGblEntry->tnlData.lclEpIpAddr);
            COPY_IPV4_ADDR_T(tnlTermEntry.key.greTnl.rmtEpIpAddr,
                             ipTnlGblEntry->tnlData.rmtEpIpAddr);
            tnlTermEntry.data.greTnl.tunnelVif = intfId;
            tnlTermEntry.data.greTnl.spanState = SPAN_STATE_FORWARD;
            break;

        case XP_IP_VPN_GRE_TUNNEL:
            COPY_IPV4_ADDR_T(tnlTermEntry.key.vpnGreTnl.lclEpIpAddr,
                             ipTnlGblEntry->tnlData.lclEpIpAddr);
            COPY_IPV4_ADDR_T(tnlTermEntry.key.vpnGreTnl.rmtEpIpAddr,
                             ipTnlGblEntry->tnlData.rmtEpIpAddr);
            tnlTermEntry.key.vpnGreTnl.vpnLabel = ipTnlGblEntry->tnlData.vpnLabel;
            tnlTermEntry.data.vpnGreTnl.tunnelVif = intfId;
            tnlTermEntry.data.vpnGreTnl.spanState = SPAN_STATE_FORWARD;
            break;

        case XP_IP_VPN_GRE_TUNNEL_LOOSE:
            COPY_IPV4_ADDR_T(tnlTermEntry.key.vpnGreTnl.lclEpIpAddr,
                             ipTnlGblEntry->tnlData.lclEpIpAddr);
            COPY_IPV4_ADDR_T(tnlTermEntry.key.vpnGreTnl.rmtEpIpAddr,
                             ipTnlGblEntry->tnlData.rmtEpIpAddr);
            tnlTermEntry.data.vpnGreTnl.tunnelVif = intfId;
            tnlTermEntry.data.vpnGreTnl.spanState = SPAN_STATE_FORWARD;
            break;

        case XP_IP_GENEVE_TUNNEL:
            tnlTermEntry.data.geneveTnl.spanState = SPAN_STATE_FORWARD;
            COPY_IPV4_ADDR_T(tnlTermEntry.key.geneveTnl.rmtEpIpAddr,
                             ipTnlGblEntry->tnlData.rmtEpIpAddr);
            tnlTermEntry.data.geneveTnl.tunnelVif = intfId;
            break;

        default:
            rt = XP_ERR_INVALID_PARAMS;
            return rt;
    }

    tnlTermEntry.tunnelType = ipTnlGblEntry->tnlData.type;

    /* Set termination entry for unicast tunnels */
    rt = xpTunnelMgr::instance()->setIpTnlTerminationEntry(devId, &tnlTermEntry,
                                                           index);
    if (rt != XP_NO_ERR)
    {
        return rt;
    }

    if (index.size() > XP_INDEX_LIST_SIZE_FOR_NO_REHASH)
    {
        xpUtil::convertXpIndexListToXpHashIndexList(index, hashIndexList);

        rt = xpsHandleTunnelIvifTableRehash(devId, &hashIndexList);
        if (rt != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Ip tunnel handle rehash failed");
            return rt;
        }
    }

    // Below manages HW ID. Not required.
    /* Create tunnel state DB entry */
    rt = xpsIpTunnelDbAddEntry(devId, intfId, index[0], -1);
    if (rt != XP_NO_ERR)
    {
        return rt;
    }

#endif
    XPS_FUNC_EXIT_LOG();
    return rt;
}

XP_STATUS xpsIpTunnelSetConfigByMirrorSession(xpsDevice_t devId,
                                              xpsInterfaceId_t tnlIntfId, xpsMirrorData_t * mirrorSessionData)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsIpTunnelSetConfig);
    XP_STATUS rt = XP_NO_ERR;
    CPSS_DXCH_TUNNEL_START_CONFIG_UNT   tunnel_start_entry;
    CPSS_TUNNEL_TYPE_ENT                tunnel_start_type;
    GT_STATUS                           status;
    xpsIpTnlGblDbEntry_t               *ipTnlGblEntry = NULL;
    xpsScope_t                          scopeId;

    /* Get Scope Id from devId */
    if ((rt = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return rt;
    }

    rt = xpsIpTunnelGblGetDbEntry(scopeId, tnlIntfId, &ipTnlGblEntry);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "TunnelGet failed : %d ",
              tnlIntfId);
        return rt;
    }

    switch (ipTnlGblEntry->tnlData.type)
    {
        case XP_IP_GRE_ERSPAN2_TUNNEL:
            status = cpssHalTunnelTunnelStartEntryRead(devId,
                                                       ipTnlGblEntry->l3HdrInsertionId,
                                                       &tunnel_start_type,
                                                       &tunnel_start_entry);
            if (status != GT_OK)
            {
                rt = XP_ERR_INVALID_PARAMS;
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Tunnel start read failed");
                return rt;
            }

            if (ipTnlGblEntry->tnlData.lclEpIpAddr.type == XP_PREFIX_TYPE_IPV4)
            {
                tunnel_start_entry.ipv4Cfg.dscp = (mirrorSessionData->erspan2Data.tos >> 2);
                tunnel_start_entry.ipv4Cfg.up = mirrorSessionData->erspan2Data.tc;
            }
            else
            {
                tunnel_start_entry.ipv6Cfg.dscp = (mirrorSessionData->erspan2Data.tos >> 2);
                tunnel_start_entry.ipv6Cfg.up = mirrorSessionData->erspan2Data.tc;
            }
            status = cpssHalTunnelTunnelStartEntryWrite(devId,
                                                        ipTnlGblEntry->l3HdrInsertionId,
                                                        tunnel_start_type,
                                                        &tunnel_start_entry);
            if (status != GT_OK)
            {
                rt = XP_ERR_INVALID_PARAMS;
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Tunnel start write failed");
                return rt;
            }
            break;

        default:
            rt = XP_ERR_INVALID_PARAMS;
            return rt;
    }

    XPS_FUNC_EXIT_LOG();

    return rt;
}

XP_STATUS xpsIpTunnelClearConfig(xpsDevice_t devId, xpsInterfaceId_t tnlIntfId,
                                 xpIpTunnelType_t type)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsIpTunnelClearConfig);
    XP_STATUS rt = XP_NO_ERR;
    CPSS_DXCH_TUNNEL_START_CONFIG_UNT   tunnel_start_entry;
    CPSS_TUNNEL_TYPE_ENT                tunnel_start_type=
        CPSS_TUNNEL_GENERIC_IPV4_E;
    GT_STATUS                           status;
    xpsIpTnlGblDbEntry_t                *ipTnlGblEntry = NULL;
    xpsScope_t                          scopeId;

    /* Get Scope Id from devId */
    if ((rt = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return rt;
    }

    rt = xpsIpTunnelGblGetDbEntry(scopeId, tnlIntfId, &ipTnlGblEntry);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "TunnelGet failed : %d ",
              tnlIntfId);
        return rt;
    }
    switch (type)
    {
        case XP_IP_GRE_ERSPAN2_TUNNEL:
            {
                status = cpssHalTunnelTunnelStartEntryWrite(devId,
                                                            ipTnlGblEntry->l3HdrInsertionId,
                                                            tunnel_start_type,
                                                            &tunnel_start_entry);
                if (status != GT_OK)
                {
                    rt = XP_ERR_INVALID_PARAMS;
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Tunnel start write failed with error code %d", status);
                    return rt;
                }
            }
            break;

        default:
            rt = XP_ERR_INVALID_PARAMS;
            return rt;
    }

    XPS_FUNC_EXIT_LOG();

    return rt;
}

XP_STATUS xpsIpTunnelSetConfig(xpsDevice_t devId, xpsInterfaceId_t tnlIntfId,
                               xpIpTunnelType_t type, xpsIpTunnelConfig_t * cfg)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsIpTunnelSetConfig);
    XP_STATUS rt = XP_NO_ERR;
    CPSS_DXCH_TUNNEL_START_CONFIG_UNT   tunnel_start_entry;
    CPSS_TUNNEL_TYPE_ENT                tunnel_start_type;
    GT_STATUS                           status;
    xpsIpTnlGblDbEntry_t                *ipTnlGblEntry = NULL;
    xpsScope_t                          scopeId;

    /* Get Scope Id from devId */
    if ((rt = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return rt;
    }

    rt = xpsIpTunnelGblGetDbEntry(scopeId, tnlIntfId, &ipTnlGblEntry);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "TunnelGet failed : %d ",
              tnlIntfId);
        return rt;
    }

    switch (type)
    {
        case XP_IP_GRE_ERSPAN2_TUNNEL:
            if (memcmp(&ipTnlGblEntry->tnlData.cfg, cfg, sizeof(xpsIpTunnelConfig_t)))
            {
                memcpy(&ipTnlGblEntry->tnlData.cfg, cfg, sizeof(xpsIpTunnelConfig_t));

                memset(&tunnel_start_entry, 0, sizeof(tunnel_start_entry));

                if (ipTnlGblEntry->tnlData.lclEpIpAddr.type == XP_PREFIX_TYPE_IPV4)
                {
                    tunnel_start_type = CPSS_TUNNEL_GENERIC_IPV4_E;
                    tunnel_start_entry.ipv4Cfg.ipHeaderProtocol =
                        CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E;
                    tunnel_start_entry.ipv4Cfg.greProtocolForEthernet =
                        ipTnlGblEntry->tnlData.cfg.ipgreCfg.protocol;
                    COPY_IPV4_ADDR_T(&tunnel_start_entry.ipv4Cfg.destIp,
                                     ipTnlGblEntry->tnlData.rmtEpIpAddr.addr.ipv4Addr);
                    COPY_IPV4_ADDR_T(&tunnel_start_entry.ipv4Cfg.srcIp,
                                     ipTnlGblEntry->tnlData.lclEpIpAddr.addr.ipv4Addr);
                    tunnel_start_entry.ipv4Cfg.ttl =
                        ipTnlGblEntry->tnlData.cfg.ipgreCfg.ttlHopLimit;
                    tunnel_start_entry.ipv4Cfg.dscpMarkMode =
                        CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
                    tunnel_start_entry.ipv4Cfg.dscp =
                        ipTnlGblEntry->tnlData.cfg.ipgreCfg.dscp;
                    tunnel_start_entry.ipv4Cfg.tagEnable =
                        ipTnlGblEntry->tnlData.cfg.ipgreCfg.vlanId ? GT_TRUE : GT_FALSE;
                    tunnel_start_entry.ipv4Cfg.vlanId = ipTnlGblEntry->tnlData.cfg.ipgreCfg.vlanId;
                    tunnel_start_entry.ipv4Cfg.upMarkMode =
                        CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
                    tunnel_start_entry.ipv4Cfg.up = ipTnlGblEntry->tnlData.cfg.ipgreCfg.vlanPri;
                    COPY_MAC_ADDR_T(&tunnel_start_entry.ipv4Cfg.macDa,
                                    &ipTnlGblEntry->tnlData.cfg.ipgreCfg.dstMacAddr);
                }
                else
                {
                    tunnel_start_type = CPSS_TUNNEL_GENERIC_IPV6_E;
                    tunnel_start_entry.ipv6Cfg.ipHeaderProtocol =
                        CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E;
                    tunnel_start_entry.ipv6Cfg.greProtocolForEthernet =
                        ipTnlGblEntry->tnlData.cfg.ipgreCfg.protocol;
                    COPY_IPV6_ADDR_T(&tunnel_start_entry.ipv6Cfg.destIp,
                                     ipTnlGblEntry->tnlData.rmtEpIpAddr.addr.ipv6Addr);
                    COPY_IPV6_ADDR_T(&tunnel_start_entry.ipv6Cfg.srcIp,
                                     ipTnlGblEntry->tnlData.lclEpIpAddr.addr.ipv6Addr);
                    tunnel_start_entry.ipv6Cfg.ttl =
                        ipTnlGblEntry->tnlData.cfg.ipgreCfg.ttlHopLimit;
                    tunnel_start_entry.ipv6Cfg.dscpMarkMode =
                        CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
                    tunnel_start_entry.ipv6Cfg.tagEnable =
                        ipTnlGblEntry->tnlData.cfg.ipgreCfg.vlanId ? GT_TRUE : GT_FALSE;
                    tunnel_start_entry.ipv6Cfg.vlanId = ipTnlGblEntry->tnlData.cfg.ipgreCfg.vlanId;
                    tunnel_start_entry.ipv6Cfg.upMarkMode =
                        CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
                    tunnel_start_entry.ipv6Cfg.up = ipTnlGblEntry->tnlData.cfg.ipgreCfg.vlanPri;
                    COPY_MAC_ADDR_T(&tunnel_start_entry.ipv6Cfg.macDa,
                                    &ipTnlGblEntry->tnlData.cfg.ipgreCfg.dstMacAddr);
                }
                status = cpssHalTunnelTunnelStartEntryWrite(devId,
                                                            ipTnlGblEntry->l3HdrInsertionId,
                                                            tunnel_start_type,
                                                            &tunnel_start_entry);
                if (status != GT_OK)
                {
                    rt = XP_ERR_INVALID_PARAMS;
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Tunnel start write failed");
                    return rt;
                }
            }
            break;

        case XP_IP_VXLAN_TUNNEL:
            if (memcmp(&ipTnlGblEntry->tnlData.cfg, cfg, sizeof(xpsIpTunnelConfig_t)))
            {
                memcpy(&ipTnlGblEntry->tnlData.cfg, cfg, sizeof(xpsIpTunnelConfig_t));

                memset(&tunnel_start_entry, 0, sizeof(tunnel_start_entry));

                if (ipTnlGblEntry->tnlData.lclEpIpAddr.type == XP_PREFIX_TYPE_IPV4)
                {
                    tunnel_start_type = CPSS_TUNNEL_GENERIC_IPV4_E;
                    tunnel_start_entry.ipv4Cfg.ipHeaderProtocol =
                        CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E;
                    COPY_IPV4_ADDR_T(&tunnel_start_entry.ipv4Cfg.destIp,
                                     ipTnlGblEntry->tnlData.rmtEpIpAddr.addr.ipv4Addr);
                    COPY_IPV4_ADDR_T(&tunnel_start_entry.ipv4Cfg.srcIp,
                                     ipTnlGblEntry->tnlData.lclEpIpAddr.addr.ipv4Addr);
                    tunnel_start_entry.ipv4Cfg.ttl =
                        (ipTnlGblEntry->tnlData.cfg.vxlanCfg.ttlMode == XP_IP_TUNNEL_MODE_UNIFORM) ? 0 :
                        ipTnlGblEntry->tnlData.cfg.vxlanCfg.ttl;
                    tunnel_start_entry.ipv4Cfg.dscpMarkMode =
                        (ipTnlGblEntry->tnlData.cfg.vxlanCfg.dscpMode == XP_IP_TUNNEL_MODE_UNIFORM)
                        ?CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_PACKET_QOS_PROFILE_E:
                        CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
                    tunnel_start_entry.ipv4Cfg.dscp =
                        ipTnlGblEntry->tnlData.cfg.vxlanCfg.dscp;
                    tunnel_start_entry.ipv4Cfg.profileIndex =
                        (ipTnlGblEntry->tnlData.cfg.vxlanCfg.udpSrcPortMode ==
                         XPS_VXLAN_UDP_PORT_STATIC)
                        ? CPSSHAL_GEN_TUNNEL_PROFILE_VXLAN_UDP_PORT_FROM_TS :
                        CPSSHAL_GEN_TUNNEL_PROFILE_VXLAN_UDP_PORT_FROM_HASH;
                    tunnel_start_entry.ipv4Cfg.udpSrcPort =
                        ipTnlGblEntry->tnlData.cfg.vxlanCfg.udpSrcPort;
                    tunnel_start_entry.ipv4Cfg.udpDstPort =
                        ipTnlGblEntry->tnlData.cfg.vxlanCfg.udpDstPort;
                    COPY_MAC_ADDR_T(&tunnel_start_entry.ipv4Cfg.macDa,
                                    &ipTnlGblEntry->tnlData.cfg.vxlanCfg.dstMacAddr);

                    tunnel_start_entry.ipv4Cfg.tagEnable =
                        ipTnlGblEntry->tnlData.cfg.vxlanCfg.vlanId ? GT_TRUE : GT_FALSE;
                    tunnel_start_entry.ipv4Cfg.vlanId = ipTnlGblEntry->tnlData.cfg.vxlanCfg.vlanId;
                }
                else
                {
                    tunnel_start_type = CPSS_TUNNEL_GENERIC_IPV6_E;
                    tunnel_start_entry.ipv6Cfg.ipHeaderProtocol =
                        CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E;
                    COPY_IPV6_ADDR_T(&tunnel_start_entry.ipv6Cfg.destIp,
                                     ipTnlGblEntry->tnlData.rmtEpIpAddr.addr.ipv6Addr);
                    COPY_IPV6_ADDR_T(&tunnel_start_entry.ipv6Cfg.srcIp,
                                     ipTnlGblEntry->tnlData.lclEpIpAddr.addr.ipv6Addr);
                    tunnel_start_entry.ipv6Cfg.ttl =
                        (ipTnlGblEntry->tnlData.cfg.vxlanCfg.ttlMode == XP_IP_TUNNEL_MODE_UNIFORM) ? 0 :
                        ipTnlGblEntry->tnlData.cfg.vxlanCfg.ttl;
                    tunnel_start_entry.ipv6Cfg.dscpMarkMode =
                        (ipTnlGblEntry->tnlData.cfg.vxlanCfg.dscpMode == XP_IP_TUNNEL_MODE_UNIFORM)
                        ?CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_PACKET_QOS_PROFILE_E:
                        CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E;
                    tunnel_start_entry.ipv6Cfg.dscp =
                        ipTnlGblEntry->tnlData.cfg.vxlanCfg.dscp;
                    tunnel_start_entry.ipv6Cfg.profileIndex =
                        (ipTnlGblEntry->tnlData.cfg.vxlanCfg.udpSrcPortMode ==
                         XPS_VXLAN_UDP_PORT_STATIC)
                        ? CPSSHAL_GEN_TUNNEL_PROFILE_VXLAN_UDP_PORT_FROM_TS :
                        CPSSHAL_GEN_TUNNEL_PROFILE_VXLAN_UDP_PORT_FROM_HASH;
                    tunnel_start_entry.ipv6Cfg.udpSrcPort =
                        ipTnlGblEntry->tnlData.cfg.vxlanCfg.udpSrcPort;
                    tunnel_start_entry.ipv6Cfg.udpDstPort =
                        ipTnlGblEntry->tnlData.cfg.vxlanCfg.udpDstPort;
                    COPY_MAC_ADDR_T(&tunnel_start_entry.ipv6Cfg.macDa,
                                    &ipTnlGblEntry->tnlData.cfg.vxlanCfg.dstMacAddr);

                    tunnel_start_entry.ipv6Cfg.tagEnable =
                        ipTnlGblEntry->tnlData.cfg.vxlanCfg.vlanId ? GT_TRUE : GT_FALSE;
                    tunnel_start_entry.ipv6Cfg.vlanId = ipTnlGblEntry->tnlData.cfg.vxlanCfg.vlanId;
                }
                status = cpssHalTunnelTunnelStartEntryWrite(devId,
                                                            ipTnlGblEntry->ePorts[ipTnlGblEntry->numOfEports-1].tnlStartId,
                                                            tunnel_start_type,
                                                            &tunnel_start_entry);
                if (status != GT_OK)
                {
                    rt = XP_ERR_INVALID_PARAMS;
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Tunnel start write failed");
                    return rt;
                }
            }
            break;

        default:
            rt = XP_ERR_INVALID_PARAMS;
            return rt;
    }

    XPS_FUNC_EXIT_LOG();

    return rt;
}

XP_STATUS xpsIpTunnelGetConfig(xpsDevice_t devId, xpsInterfaceId_t tnlIntfId,
                               xpIpTunnelType_t type, xpsIpTunnelConfig_t *cfg)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsIpTunnelGetConfig);
    XP_STATUS                           rt = XP_NO_ERR;
    CPSS_DXCH_TUNNEL_START_CONFIG_UNT   tunnel_start_entry;
    CPSS_TUNNEL_TYPE_ENT                tunnel_start_type;
    GT_STATUS                           status;
    xpsIpTnlGblDbEntry_t               *ipTnlGblEntry = NULL;
    xpsScope_t                          scopeId;

    /* Get Scope Id from devId */
    if ((rt = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return rt;
    }

    rt = xpsIpTunnelGblGetDbEntry(scopeId, tnlIntfId, &ipTnlGblEntry);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "TunnelGet failed : %d ",
              tnlIntfId);
        return rt;
    }

    switch (type)
    {
        case XP_IP_GRE_ERSPAN2_TUNNEL:
            memset(&tunnel_start_entry, 0, sizeof(tunnel_start_entry));
            status = cpssHalTunnelTunnelStartEntryRead(devId,
                                                       ipTnlGblEntry->l3HdrInsertionId,
                                                       &tunnel_start_type,
                                                       &tunnel_start_entry);
            if (status != GT_OK)
            {
                rt = XP_ERR_INVALID_PARAMS;
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Tunnel start read failed");
                return rt;
            }

            if (tunnel_start_type == CPSS_TUNNEL_GENERIC_IPV4_E)
            {
                cfg->ipgreCfg.protocol = tunnel_start_entry.ipv4Cfg.greProtocolForEthernet;
                cfg->ipgreCfg.ttlHopLimit = tunnel_start_entry.ipv4Cfg.ttl;
                if (tunnel_start_entry.ipv4Cfg.tagEnable == GT_TRUE)
                {
                    cfg->ipgreCfg.vlanId = tunnel_start_entry.ipv4Cfg.vlanId;
                }
                else
                {
                    cfg->ipgreCfg.vlanId = 0;
                }
                COPY_MAC_ADDR_T(&cfg->ipgreCfg.dstMacAddr, &tunnel_start_entry.ipv4Cfg.macDa);

                /* for now it all zeros
                #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
                    tunnel_start_entry.ipv4Cfg.greFlagsAndVersion = bswap_16(0x9000); checksum and seq bit are set
                #else
                    tunnel_start_entry.ipv4Cfg.greFlagsAndVersion = 0x9000; checksum and seq bit are set
                #endif
                */
            }
            else
            {
                cfg->ipgreCfg.protocol = tunnel_start_entry.ipv6Cfg.greProtocolForEthernet;
                cfg->ipgreCfg.ttlHopLimit = tunnel_start_entry.ipv6Cfg.ttl;
                if (tunnel_start_entry.ipv6Cfg.tagEnable == GT_TRUE)
                {
                    cfg->ipgreCfg.vlanId = tunnel_start_entry.ipv6Cfg.vlanId;
                }
                else
                {
                    cfg->ipgreCfg.vlanId = 0;
                }
                COPY_MAC_ADDR_T(&cfg->ipgreCfg.dstMacAddr, &tunnel_start_entry.ipv6Cfg.macDa);
                /* tunnel_start_entry.ipv6Cfg.greFlagsAndVersion = 0x9000; checksum and seq bit are set */
            }
            break;

        default:
            rt = XP_ERR_INVALID_PARAMS;
            return rt;
    }

    XPS_FUNC_EXIT_LOG();

    return rt;
}

XP_STATUS xpsIpTunnelGetRemoteIp(xpsDevice_t devId, xpIpTunnelType_t type,
                                 xpsInterfaceId_t tnlIntfId, ipv4Addr_t *rmtEpIpAddr)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsIpTunnelGetConfig);
    XP_STATUS                           rt = XP_NO_ERR;
    xpsIpTnlGblDbEntry_t               *ipTnlGblEntry = NULL;
    xpsScope_t                          scopeId;

    if ((rt = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return rt;
    }

    rt = xpsIpTunnelGblGetDbEntry(scopeId, tnlIntfId, &ipTnlGblEntry);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "TunnelGet failed : %d ",
              tnlIntfId);
        return rt;
    }

    if (ipTnlGblEntry->tnlData.rmtEpIpAddr.type == XP_PREFIX_TYPE_IPV4)
    {
        COPY_IPV4_ADDR_T(rmtEpIpAddr, ipTnlGblEntry->tnlData.rmtEpIpAddr.addr.ipv4Addr);
    }


    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

//MULTIDEV HAS TO BE NOTED LaTER






XP_STATUS xpsIpTnlGetL2InsId(xpsDevice_t devId, xpVif_t vifId, uint32_t nhId,
                             uint32_t *l2HdrInsrtId)
{
    XPS_FUNC_ENTRY_LOG();


    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsIpTnlSetNexthopData(xpsDevice_t devId, xpVif_t vifId,
                                 uint32_t nhId)
{
    XPS_FUNC_ENTRY_LOG();


    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsIpTnlSetMcOIF(xpsDevice_t devId, xpsInterfaceId_t tunIntfId,
                           xpsIpMcOIFData_t *oifData)
{
    XPS_FUNC_ENTRY_LOG();


    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpIpTunnelRemove(xpsDevice_t devId, xpIpTunnelType_t type,
                           xpsInterfaceId_t tnlIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS rt = XP_NO_ERR;;
    xpsTnlDbEntry_t *tnlDbEntry = NULL;
    xpsIpTnlGblDbEntry_t *ipTnlGblEntry = NULL;
    xpsScope_t scopeId;

    /* Get Scope Id from devId */
    if ((rt = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return rt;
    }

    rt = xpsIpTunnelGblGetDbEntry(scopeId, tnlIntfId, &ipTnlGblEntry);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "TunnelGet failed : %d ",
              tnlIntfId);
        return rt;
    }


    rt = xpsIpTunnelDbGetEntry(devId, tnlIntfId, &tnlDbEntry);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Tunnel interface(%d) does not exist", tnlIntfId);
        return rt;
    }
    //rt = xpsIpTunnelClearConfig(devId, tnlIntfId, type);
    //if (rt != XP_NO_ERR)
    //{
    //    LOGFN(xpLogModXps,XP_SUBMOD_MAIN,XP_LOG_ERROR,"Tunnel interface(%d) data failed to clear", tnlIntfId);
    //    return rt;
    //}

    if (ipTnlGblEntry->l3HdrInsertionId != XPS_INVALID_INSPTR)
    {
        rt = xpsReleaseTunnelStartIndex(devId, ipTnlGblEntry->l3HdrInsertionId);
        if (rt != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Tunnel (%d) Start pointer (%d) release failed", tnlIntfId,
                  ipTnlGblEntry->l3HdrInsertionId);
            return rt;
        }
        ipTnlGblEntry->l3HdrInsertionId = XPS_INVALID_INSPTR;
    }


    rt = xpsIpTunnelDbRemoveEntry(devId, tnlIntfId);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Remove entry from Tunnel db failed,"
              " tunnel interface(%d)", tnlIntfId);
        return rt;
    }

    XPS_FUNC_EXIT_LOG();

    return rt;
}

XP_STATUS xpsIpTunnelAddLocalVtep(xpsDevice_t devId, ipv4Addr_t localIp,
                                  xpIpTunnelType_t tnlType)
{
    XPS_FUNC_ENTRY_LOG();


    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsIpTunnelRemoveLocalVtep(xpsDevice_t devId, ipv4Addr_t localIp,
                                     xpIpTunnelType_t tnlType)
{
    XPS_FUNC_ENTRY_LOG();


    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsIpTunnelUpdateNexthop(xpsDevice_t devId,
                                   xpsInterfaceId_t tnlIntfId)
{
    XPS_FUNC_ENTRY_LOG();


    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsRegisterRemoteVtepLearnHandler(remoteVtepLearnPktHandler func,
                                            void *user_data)
{
    XPS_FUNC_ENTRY_LOG();


    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsUnregisterRemoteVtepLearnHandler()
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}



XP_STATUS xpsMplsRemoveTunnelOrigination(xpsDevice_t devId,
                                         xpsInterfaceId_t mplsTnlId)
{
    XPS_FUNC_ENTRY_LOG();


    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsErspanTunnelAdd(xpsDevice_t devId, xpsInterfaceId_t tnlIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsErspanTunnelAdd);
    XP_STATUS rt = XP_NO_ERR;
    xpsIpTnlGblDbEntry_t *ipTnlGblEntry;
    xpsScope_t scopeId;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        return XP_ERR_INVALID_DEV_ID;
    }

    /* Get Scope Id from devId */
    if ((rt = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return rt;
    }

    rt = xpsIpTunnelGblGetDbEntry(scopeId, tnlIntfId, &ipTnlGblEntry);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "TunnelGet failed : %d ",
              tnlIntfId);
        return rt;
    }
    /* Set the tunnel start entry */
    rt = xpsIpTunnelAddOrigination(devId, tnlIntfId, 0, &ipTnlGblEntry->tnlData);
    if (rt != XP_NO_ERR)
    {
        return rt;
    }

    /* Create tunnel state DB entry */
    rt = xpsIpTunnelDbAddEntry(devId, tnlIntfId, ipTnlGblEntry->l3HdrInsertionId,
                               -1);
    if (rt != XP_NO_ERR)
    {
        return rt;
    }

    XPS_FUNC_EXIT_LOG();

    return rt;
}



XP_STATUS xpsPbbTunnelCreate(xpsScope_t scopeId, xpsInterfaceType_e type,
                             xpsPbbTunnelData_t *data, xpsInterfaceId_t *tnlIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPbbTunnelAddOrigination(xpsDevice_t devId,
                                     xpsInterfaceId_t pbbTnlId, uint32_t insertionId, xpsPbbTunnelData_t *tnlData)
{
    XPS_FUNC_ENTRY_LOG();


    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPbbAddTunnelTerminationEntry(xpsDevice_t devId,
                                          xpsInterfaceId_t intfId, xpsPbbTunnelData_t *tnlData)
{
    XPS_FUNC_ENTRY_LOG();


    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPbbTunnelAddLocalEntry(xpsDevice_t devId, xpsInterfaceId_t intfId,
                                    xpsPbbTunnelData_t *tnlData)
{
    XPS_FUNC_ENTRY_LOG();


    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPbbTunnelDelete(xpsScope_t scopeId, xpsInterfaceId_t tnlIntfId)
{
    XPS_FUNC_ENTRY_LOG();


    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPbbTunnelAdd(xpsDevice_t devId, xpsInterfaceId_t intfId)
{
    XPS_FUNC_ENTRY_LOG();


    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpPbbTunnelRemove(xpsDevice_t devId, xpsInterfaceId_t tnlIntfId)
{
    XPS_FUNC_ENTRY_LOG();


    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSetIpTnlTerminationFieldData(xpsDevice_t devId, uint32_t index,
                                          uint32_t fieldName, uint32_t *fieldData)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSetTunnelEcmpConfig(xpsDevice_t devId, xpsInterfaceId_t tnlIntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsSetTunnelEcmpConfig);
    XP_STATUS rt = XP_NO_ERR;
    GT_STATUS rc = GT_OK;
    xpsIpTnlGblDbEntry_t *ipTnlGblDbEntry = NULL;
    xpsScope_t scopeId;
    CPSS_DXCH_BRG_L2_ECMP_LTT_ENTRY_STC ecmpLttInfo;
    CPSS_DXCH_BRG_L2_ECMP_ENTRY_STC    ecmpEntry;

    memset(&ecmpLttInfo, 0, sizeof(ecmpLttInfo));
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
    /* Allocate index range based on numOfEports.
       If all the members are removed, numEports becomes 0.
       Hence init with max value */
    uint32_t startIdx = (XPS_LTT_ECMP_MAX_NUM-1);

    if (ipTnlGblDbEntry->numOfEports)
    {
        rt = xpsAllocatorAllocateIdGroup(scopeId, XP_ALLOC_LTT_ECMP,
                                         ipTnlGblDbEntry->numOfEports, &startIdx);
        if (rt != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "L2EcmpTblIdx failed : %d ",
                  tnlIntfId);
            return rt;
        }
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEFAULT,
              "Set pEport %d startIdx %d numEport %d\n",
              ipTnlGblDbEntry->primaryEport, startIdx, ipTnlGblDbEntry->numOfEports);

        ecmpLttInfo.ecmpEnable = GT_TRUE;
        ecmpLttInfo.ecmpNumOfPaths = ipTnlGblDbEntry->numOfEports;
        ecmpLttInfo.ecmpStartIndex = startIdx;

        rc = cpssHalTunnelL2EcmpLttTableSet(devId, (ipTnlGblDbEntry->primaryEport)-512,
                                            &ecmpLttInfo);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " L2EcmpLttTableSet failed  %d Status : %d\n", devId, rc);
            return xpsConvertCpssStatusToXPStatus(rc);
        }

        for (uint32_t i = startIdx, j = 0; j < ipTnlGblDbEntry->numOfEports; i++, j++)
        {
            memset(&ecmpEntry, 0, sizeof(ecmpEntry));
            ecmpEntry.targetEport   = ipTnlGblDbEntry->ePorts[j].ePort;
            rc = cpssHalTunnelL2EcmpTableSet(devId, i, &ecmpEntry);
            if (rc != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      " L2EcmpTableSet failed  %d Status : %d\n", devId, rc);
                return xpsConvertCpssStatusToXPStatus(rc);
            }
        }
    }
    ipTnlGblDbEntry->ecmpStartIdx = startIdx;

    XPS_FUNC_EXIT_LOG();
    return XP_NO_ERR;
}

XP_STATUS xpsClearTunnelEcmpConfig(xpsDevice_t devId, uint32_t startIdx,
                                   uint32_t size)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsClearTunnelEcmpConfig);
    XP_STATUS rt = XP_NO_ERR;
    GT_STATUS rc = GT_OK;
    xpsScope_t scopeId;
    CPSS_DXCH_BRG_L2_ECMP_ENTRY_STC    ecmpEntry;

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

    for (uint32_t i = startIdx; i < size; i++)
    {
        memset(&ecmpEntry, 0, sizeof(ecmpEntry));
        rc = cpssHalTunnelL2EcmpTableSet(devId, i, &ecmpEntry);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " L2EcmpTableSet failed  %d Status : %d\n", devId, rc);
            return xpsConvertCpssStatusToXPStatus(rc);
        }
    }
    rt = xpsAllocatorReleaseIdGroup(scopeId, XP_ALLOC_LTT_ECMP, size, startIdx);
    if (rt != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "L2EcmpTblIdx failed : %d ",
              rt);
        return rt;
    }

    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEFAULT,
          "Release startIdx %d  size %d ",
          startIdx, size);

    return XP_NO_ERR;
}

XP_STATUS xpsIpTunnelReSize(xpsScope_t scopeId, xpsInterfaceId_t tnlIntfId,
                            xpsIpTnlGblDbEntry_t **vxlanCtxNewPtr,
                            xpsIpTnlGblDbEntry_t *vxlanCtx, bool isGrow)
{
    XP_STATUS result = XP_NO_ERR;
    uint16_t numOfEports;

    if (!vxlanCtx)
    {
        return XP_ERR_NULL_POINTER;
    }

    numOfEports = vxlanCtx->numOfEports;

    if (isGrow)
    {
        result = xpsDynamicArrayGrow((void **)vxlanCtxNewPtr, (void *)vxlanCtx,
                                     sizeof(xpsIpTnlGblDbEntry_t),
                                     sizeof(xpsVxlanEPortInfo_t), numOfEports, MAX_EPORT_SIZE);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Dynamic array grow failed");
            return result;
        }
    }
    else
    {
        result = xpsDynamicArrayShrink((void **)vxlanCtxNewPtr, (void *)vxlanCtx,
                                       sizeof(xpsIpTnlGblDbEntry_t),
                                       sizeof(xpsVxlanEPortInfo_t), numOfEports, MAX_EPORT_SIZE);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Dynamic array shrink failed");
            return result;
        }

    }
    //Delete the old vxlanCtx
    xpsIpTnlGblDbEntry_t keyVxlanDbEntry;
    memset(&keyVxlanDbEntry, 0, sizeof(keyVxlanDbEntry));
    keyVxlanDbEntry.vifId = tnlIntfId;

    result = xpsStateDeleteData(scopeId, ipTnlGblDbHndl,
                                (xpsDbKey_t)&keyVxlanDbEntry, (void **)&vxlanCtx);
    if (result != XP_NO_ERR)
    {
        return result;
    }

    //Insert the new context
    if ((result = xpsStateInsertData(scopeId, ipTnlGblDbHndl,
                                     (void*)(*vxlanCtxNewPtr))) != XP_NO_ERR)
    {
        xpsStateHeapFree((void*)(*vxlanCtxNewPtr));
        (*vxlanCtxNewPtr) = NULL;
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Inserting data in state failed");
        return result;
    }

    return result;
}

#ifdef __cplusplus
}
#endif
