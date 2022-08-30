// xpsNhGrp.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include <algorithm>
#include "xpsNhGrp.h"
#include "xpsState.h"
#include "xpsScope.h"
#include "xpsLock.h"
#include "xpsCommon.h"
#include "xpsInternal.h"
#include "xpsAllocator.h"
#include "cpssHalUtil.h"
#include "cpssHalNhgrp.h"
#include "cpssHalDevice.h"
#include <cpss/dxCh/dxChxGen/ip/cpssDxChIp.h>
#include <cpss/dxCh/dxChxGen/trunk/cpssDxChTrunk.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/tti/cpssDxChTti.h>
#include <cpss/dxCh/dxChxGen/ip/cpssDxChIpTypes.h>
#include "cpssHalVlan.h"
#include "cpssHalL3.h"
#include "cpssHalLag.h"
#include "xpsL3.h"

// Number of salt bytes in hash config.
// There doesnt seem to be a macro from CPSS for it.
// There is CRC_NUM_BYTES_USED_FOR_GENERATION_CNS but inside
// .c file
#define XPS_NHGRP_NUM_SALT_BYTES    70

uint32_t nhGrpMaxSize = 0;   //MAX next hop per group
/**
 * \brief Next Hop Group Static State Database Handle
 *
 */
static xpsDbHandle_t xpsL3NhGrpStaticDataDbHandle = XPS_STATE_INVALID_DB_HANDLE;


/**
 * \brief Next Hop Group Manager State Database Handle
 *
 */
static xpsDbHandle_t nextHopGroupDbHandle = XPS_STATE_INVALID_DB_HANDLE;


#ifdef __cplusplus
extern "C" {
#endif
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/*
 * Static Function Prototypes
 */

static int32_t nextHopGroupKeyCompare(void *key1, void *key2);

static int32_t nextHopGroupStaticVariablesKeyCompare(void *key1, void *key2);

static XP_STATUS nextHopGroupEntryInit(xpsL3NextHopGroupEntry_t *pNhGrpEntry,
                                       uint32_t nhGrpId, uint32_t nhBaseId, uint32_t groupSize);

static XP_STATUS nextHopGroupEntryDeInit(xpsL3NextHopGroupEntry_t *pNhGrpEntry);

static XP_STATUS xpsL3DestroyApplyRouteNextHopGroup(xpsDevice_t devId,
                                                    xpsL3NextHopGroupEntry_t* pNhGrpInfo);

static XP_STATUS xpsL3InsertStaticVariablesRouteNextHopGroup(
    xpsScope_t scopeId);

static XP_STATUS xpsL3RemoveStaticVariablesRouteNextHopGroup(
    xpsScope_t scopeId);

static XP_STATUS xpsL3GetStaticVariablesRouteNextHopGroup(xpsScope_t scopeId,
                                                          xpsL3NextHopGroupStaticEntry_t** ppNhGrpStaticVarEntry);

static XP_STATUS xpsL3SetRouteNextHopGroup(xpsDevice_t devId, uint32_t nhId,
                                           xpsL3NextHopEntry_t *pL3NextHopEntry);

static XP_STATUS xpsL3RemoveRouteNextHopAllGroup(xpsDevice_t devId,
                                                 uint32_t nhId);

static XP_STATUS xpsL3AllocateEcmpTableIndex(xpsScope_t scopeId,
                                             uint32_t *index);

static XP_STATUS xpsL3FreeEcmpTableIndex(xpsScope_t scopeId, uint32_t index);

extern XP_STATUS xpsL3ConvertXpsNhEntryToCpssNhEntry(xpsDevice_t devId,
                                                     xpsL3NextHopEntry_t *xpsNhEntry, CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC *cpssNhEntry);
extern XP_STATUS xpsL3SetRouteNextHopHw(xpsDevice_t devId, uint32_t hwNhId,
                                        xpsL3NextHopEntry_t *pL3NextHopEntry);
extern XP_STATUS xpsL3RemoveRouteNhHw(xpsDevice_t devId, uint32_t hwNhId);

static bool randomEnable = false;
bool xpsL3EcmpRandomEnableGet(void)
{
    return randomEnable;
}

void xpsL3EcmpRandomEnableSet(bool enable)
{
    randomEnable = enable;
}
/*
 * API Implementation
 */

XP_STATUS xpsL3InitRouteNextHopGroup(void)
{
    return xpsL3InitRouteNextHopGroupScope(XP_SCOPE_DEFAULT);
}

XP_STATUS xpsL3InitRouteNextHopGroupScope(xpsScope_t scopeId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;
    uint32_t ecmpTableDepth = 0;
    GT_STATUS cpssStatus = GT_OK;
    uint32_t nhGrpCount = 0;

    xpsLockCreate(XP_LOCKINDEX_XPS_NHGRP, 1);

    // Create global Next Hop Group DB
    nextHopGroupDbHandle = XPS_NEXTHOP_GROUP_DB_HNDL;
    if ((status = xpsStateRegisterDb(scopeId, "Next Hop Group DB", XPS_GLOBAL,
                                     &nextHopGroupKeyCompare, nextHopGroupDbHandle)) != XP_NO_ERR)
    {
        nextHopGroupDbHandle = XPS_STATE_INVALID_DB_HANDLE;
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Could not create Next Hop Group DB");
        return status;
    }

    // Create global Next Hop Group DB
    xpsL3NhGrpStaticDataDbHandle = XPS_NEXTHOP_GROUP_STATIC_DB_HNDL;
    if ((status = xpsStateRegisterDb(scopeId, "Next Hop Group global variables DB",
                                     XPS_GLOBAL, &nextHopGroupStaticVariablesKeyCompare,
                                     xpsL3NhGrpStaticDataDbHandle)) != XP_NO_ERR)
    {
        xpsL3NhGrpStaticDataDbHandle = XPS_STATE_INVALID_DB_HANDLE;
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Could not create Next Hop Group global variables DB");
        return status;
    }

    status = xpsL3InsertStaticVariablesRouteNextHopGroup(scopeId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Could not insert static variables into the Next Hop Group DB");
        return status;
    }
    if ((int)cpssHalSetSKUmaxNhEcmpMbrPerGrp(nhGrpMaxSize) < 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssHalSetSKUmaxNhEcmpMbrPerGrp", nhGrpMaxSize);
        return XP_ERR_INVALID_INPUT;
    }
    status = xpsL3SetRouteNextHopGlobalMaxSize(XPS_L3_NH_GRP_MAX_NEXTHOPS_PER_GRP(
                                                   0));
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed static variables set into NextHop Group DB");
        return status;
    }
    // Get ECMP table depth
    // XXX: Using device 0 as no device id available in global init
    cpssStatus = cpssDxChCfgTableNumEntriesGet(0,
                                               CPSS_DXCH_CFG_TABLE_ROUTER_ECMP_QOS_E, &ecmpTableDepth);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get QOS ECMP table depth\n");
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }

    // Init ECMP table index allocator
    status = xpsAllocatorInitIdAllocator(scopeId, XPS_ALLOCATOR_ECMP_TBL,
                                         ecmpTableDepth, XPS_L3_ECMP_TBL_RANGE_START);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to init ECMP table index ID allocator. Status : %d\n", status);
        return status;
    }

    // Get NH group count
    // XXX: Using devid as 0 as no devid available at global init
    status = xpsL3GetNexthopGroupCount(0, &nhGrpCount);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get NH group count. Status : %d\n", status);
        return status;
    }

    // Init Nexthop group allocator
    status = xpsAllocatorInitIdAllocator(scopeId, XPS_ALLOCATOR_L3_NEXT_HOP_GRP,
                                         nhGrpCount,
                                         XPS_L3_NH_GRP_RANGE_START);


    XPS_FUNC_EXIT_LOG();
    return status;
}

XP_STATUS xpsL3DeInitRouteNextHopGroup(void)
{
    return xpsL3DeInitRouteNextHopGroupScope(XP_SCOPE_DEFAULT);
}

XP_STATUS xpsL3DeInitRouteNextHopGroupScope(xpsScope_t scopeId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;

    status = xpsL3RemoveStaticVariablesRouteNextHopGroup(scopeId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Could not remove static variables from the Next Hop Group DB, ");
        return status;
    }

    // Purge global Next Hop Group global variables DB
    if ((status = xpsStateDeRegisterDb(scopeId,
                                       &xpsL3NhGrpStaticDataDbHandle)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Could not deregister Next Hop Group global variables DB");
        return status;
    }

    // Purge global Next Hop Group DB
    if ((status = xpsStateDeRegisterDb(scopeId,
                                       &nextHopGroupDbHandle)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Could not deregister Next Hop Group DB");
        return status;
    }

    return status;

    XPS_FUNC_EXIT_LOG();
}

XP_STATUS xpsL3SetRouteNextHopGlobalMaxSize(uint32_t size)
{
    return xpsL3SetRouteNextHopGlobalMaxSizeScope(XP_SCOPE_DEFAULT, size);
}

XP_STATUS xpsL3SetRouteNextHopGlobalMaxSizeScope(xpsScope_t scopeId,
                                                 uint32_t size)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS                       status          = XP_NO_ERR;
    xpsL3NextHopGroupStaticEntry_t* pNhGrpStaticVar = NULL;

    if (size > nhGrpMaxSize)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Size %d is not supported. The maximum is %d", size, nhGrpMaxSize);
        return XP_ERR_INVALID_PARAMS;
    }

    status = xpsL3GetStaticVariablesRouteNextHopGroup(scopeId, &pNhGrpStaticVar);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Could not get next hop group static variables, status %d", status);
        return status;
    }

    pNhGrpStaticVar->groupSize = size;

    XPS_FUNC_EXIT_LOG();
    return status;
}

XP_STATUS xpsL3GetRouteNextHopGlobalMaxSize(uint32_t* pGroupSize)
{
    return xpsL3GetRouteNextHopGlobalMaxSizeScope(XP_SCOPE_DEFAULT, pGroupSize);
}

XP_STATUS xpsL3GetRouteNextHopGlobalMaxSizeScope(xpsScope_t scopeId,
                                                 uint32_t* pGroupSize)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS                       status          = XP_NO_ERR;
    xpsL3NextHopGroupStaticEntry_t* pNhGrpStaticVar = NULL;

    status = xpsL3GetStaticVariablesRouteNextHopGroup(scopeId, &pNhGrpStaticVar);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Could not get next hop group static variables, status %d", status);
        return status;
    }

    *pGroupSize = pNhGrpStaticVar->groupSize;

    XPS_FUNC_EXIT_LOG();
    return status;
}

XP_STATUS xpsL3CreateRouteNextHopGroup(uint32_t *pNhGrpId)
{
    return  xpsL3CreateRouteNextHopGroupScope(XP_SCOPE_DEFAULT, pNhGrpId);
}

XP_STATUS xpsL3CreateRouteNextHopGroupScope(xpsScope_t scopeId,
                                            uint32_t *pNhGrpId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS                  status      = XP_NO_ERR;
    GT_STATUS                  cpssStatus = GT_OK;
    xpsL3NextHopGroupEntry_t*  pNhGrpInfo  = NULL;
    uint32_t                   nhBaseId    = 0;
    uint32_t                   groupSize   = 0;
    uint32_t                   ecmpTableIdx = 0;
    uint32_t                   nhGrpBlkId = 0;
    uint32_t maxNHEntries = 0;
    uint32_t i = 0;

    status = xpsL3GetRouteNextHopGlobalMaxSizeScope(scopeId, &groupSize);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Could not get next hops group size, status %d", status);
        return status;
    }

    if (pNhGrpId == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: NULL parameter pNhGrpId passed\n");
        return XP_ERR_INVALID_INPUT;
    }

    // Allocate next hop group in HW
    status = xpsL3CreateNextHopGroupScope(scopeId, groupSize, &nhGrpBlkId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Could not create next hop group, status %d", status);
        return status;
    }

    // As of now, allocator doesnt support creating a block of Ids.
    // So, allocating a block number and offsetting into
    // actual NH id which is first NH of this group

    cpssStatus = cpssDxChCfgTableNumEntriesGet(0,
                                               CPSS_DXCH_CFG_TABLE_ROUTER_NEXT_HOP_E, &maxNHEntries);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get NH table depth for Status %d\n", cpssStatus);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }

    nhBaseId = maxNHEntries - (nhGrpBlkId * nhGrpMaxSize) - nhGrpMaxSize;
    /* Check if the id is already used by NH Table.
     * If so, TABLE FULL. Return error.
     */
    for (i = nhBaseId; i < nhBaseId+nhGrpMaxSize; i++)
    {
        status = xpsAllocatorAllocateWithId(scopeId, XPS_ALLOCATOR_L3_NEXT_HOP_TBL, i);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "NHId %d already reserved by NH TBL : %d\n", i, status);
            return status;
        }
        else
        {
            status = xpsAllocatorReleaseId(scopeId, XPS_ALLOCATOR_L3_NEXT_HOP_TBL, i);
            if (status != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to release ECMP table index. Status : %d\n", status);
                return status;
            }
        }
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
          "Alloc NHGrp Id : %d (Alloced %d)\n", nhBaseId, nhGrpBlkId);

    // Allocate ECMP table index
    status = xpsL3AllocateEcmpTableIndex(scopeId, &ecmpTableIdx);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to allocate ECMP table Idx\n");
        return status;
    }
    *pNhGrpId = ecmpTableIdx;

    // Create a new Nexthop Group structure
    if ((status = xpsStateHeapMalloc(sizeof(xpsL3NextHopGroupEntry_t),
                                     (void**)&pNhGrpInfo)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Could not allocate Next Hop Group variables nhIdMap, status %d", status);
        return status;
    }

    // Initialize the allocated entry
    if ((status = nextHopGroupEntryInit(pNhGrpInfo, ecmpTableIdx, nhBaseId,
                                        groupSize)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Could not initialize Next Hop Group structure, nhGrpId %d", nhBaseId);
        xpsStateHeapFree((void *)pNhGrpInfo);
        return status;
    }

    // Insert the state into the database
    if ((status = xpsStateInsertData(scopeId, nextHopGroupDbHandle,
                                     (void*)pNhGrpInfo)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Could not insert Next Hop Group structure to the DB, nhGrpId %d", nhBaseId);
        return status;
    }

    XPS_FUNC_EXIT_LOG();
    return status;
}

XP_STATUS xpsL3DestroyRouteNextHopGroup(xpsDevice_t devId, uint32_t nhGrpId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS                 status       = XP_NO_ERR;
    xpsL3NextHopGroupEntry_t* pNhGrpEntry  = NULL;

    xpsL3NextHopGroupEntry_t  nhGrpKey;
    xpsScope_t                scopeId;

    /* Get Scope Id from devId */
    if ((status = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return status;
    }

    memset(&nhGrpKey, 0, sizeof(nhGrpKey));

    nhGrpKey.keyNhGroupId = nhGrpId;

    // Remove the corresponding state
    status = xpsStateDeleteData(scopeId, nextHopGroupDbHandle, &nhGrpKey,
                                (void**)&pNhGrpEntry);
    if ((pNhGrpEntry == NULL) || (status != XP_NO_ERR))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to delete Next Hop Group from DB, nhGrpId %d", nhGrpId);
        return XP_ERR_FREEING_UNALLOCATED_ID;
    }

    // Apply next hop group to HW
    if ((status = xpsL3DestroyApplyRouteNextHopGroup(devId,
                                                     pNhGrpEntry)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Could not apply next hop group destroy to HW, nhGrpId %d", nhGrpId);
        return status;
    }

    // Deinitialize the allocated entry
    if ((status = nextHopGroupEntryDeInit(pNhGrpEntry)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Could not deinitialize Next Hop Group structure, nhGrpId %d", nhGrpId);
        return status;
    }

    // Free the memory allocated for the corresponding state
    if ((status = xpsStateHeapFree(pNhGrpEntry)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to free Next Hop Group structure, nhGrpId %d", nhGrpId);
        return status;
    }

    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEFAULT,
          "xpsL3DestroyRouteNextHopGroup removed nhGrpId %d", nhGrpId);
    XPS_FUNC_EXIT_LOG();
    return status;
}

/**
 * Get the number of instances of the nexthop present in nexthop group
 */
static void xpsL3GetNextHopCountRouteNextHopGroup(xpsL3NextHopGroupEntry_t
                                                  *pNhGrpEntry, uint32_t nhId, uint32_t *nhCount)
{
    uint32_t i;
    uint32_t count = 0;

    for (i = 0; i < pNhGrpEntry->groupSize; i++)
        if (pNhGrpEntry->nhIdMap[i] == nhId)
        {
            count++;
        }

    *nhCount = count;
}

void xpsL3NextHopGroupSetDbEntryNhIdMaps(xpsL3NextHopGroupEntry_t* pNhGrpEntry,
                                         uint32_t *nhId)
{
    XPS_FUNC_ENTRY_LOG();

    uint32_t  i;
    memset(pNhGrpEntry->nhIdMap, 0, nhGrpMaxSize * sizeof(uint32_t));

    for (i = 0; i < nhGrpMaxSize; i++)
    {
        pNhGrpEntry->nhIdMap[i] = nhId[i];
    }

    XPS_FUNC_EXIT_LOG();

}

static  XP_STATUS xpsL3NextHopGroupProgramNhId(xpsDevice_t devId,
                                               xpsL3NextHopGroupEntry_t* pNhGrpEntry,
                                               uint32_t configSize, uint32_t *nhId)
{
    XP_STATUS status;
    uint32_t nhGrpMemberId=0;

    /* Copy the Nh data to the NH group members */
    for (nhGrpMemberId = 0; nhGrpMemberId < configSize; nhGrpMemberId++)
    {
#if 0
        if (nhId[nhGrpMemberId] == pNhGrpEntry->nhIdMap[nhGrpMemberId])
        {
            continue;
        }
#endif

        status = xpsL3CopyNextHopAndProgramNextHopInGroup(devId, nhId[nhGrpMemberId],
                                                          (pNhGrpEntry->baseNhId + nhGrpMemberId));
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to copy from nhId %d to %d, status %d", nhId[nhGrpMemberId],
                  nhGrpMemberId, status);
            return status;
        }
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
              "copied  from nhId %d to %d", nhId[nhGrpMemberId],
              (pNhGrpEntry->baseNhId + nhGrpMemberId));
    }

    return XP_NO_ERR;
}

static uint32_t xpsL3GetNextHopGroupConfigSize(uint32_t maxSize,
                                               uint32_t nhCount)
{
    return (maxSize - (maxSize % (nhCount)));
}

XP_STATUS xpsL3AddNextHopRouteNextHopGroup(xpsDevice_t devId, uint32_t nhGrpId,
                                           uint32_t nhId, uint32_t weight)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS                 status      = XP_NO_ERR;
    xpsL3NextHopGroupEntry_t* pNhGrpEntry = NULL;
    xpsScope_t                scopeId;
    uint32_t                  *tempNhIdx  = NULL;
    void                      *headNhIdx  = NULL;
    uint32_t                  i           = 0;
    GT_STATUS                 cpssStatus = GT_OK;
    CPSS_DXCH_IP_ECMP_ENTRY_STC ecmpEntry;
    uint32_t configSize = 0;

    /* Weight 0 makes no sense */
    if (weight == 0)
    {
        return XP_ERR_INVALID_PARAMS;
    }

    /* Get Scope Id from devId */
    if ((status = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return status;
    }
    status = xpsL3GetRouteNextHopGroup(scopeId, nhGrpId, &pNhGrpEntry);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Could not get next hop group from DB, nhGrpId %d, status %d", nhGrpId, status);
        return status;
    }

    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "xpsL3AddNextHopRouteNextHopGroup grp %d nh %d \n", nhGrpId, nhId);
    for (i=0; i <nhGrpMaxSize; i++)
    {
        if (pNhGrpEntry->nextHopIdx[i] == nhId)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "next hop id %d is already added to nexthop group %d", nhId, nhGrpId);
            status = XP_ERR_KEY_EXISTS;
            return status;
        }
    }

    /* Check if the NH group can accomodate the NH */
    if (pNhGrpEntry->nhCount >= pNhGrpEntry->groupSize)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "The next hop group is full, pNhGrpEntry->groupSize %d, pNhGrpEntry->nhCount %d",
              pNhGrpEntry->groupSize, pNhGrpEntry->nhCount);
        status = XP_ERR_RESOURCE_NOT_AVAILABLE;
        return status;
    }

    if (pNhGrpEntry->nhCount != 0)
    {
        /*Re-calculate for previous Count*/
        configSize = xpsL3GetNextHopGroupConfigSize(nhGrpMaxSize, pNhGrpEntry->nhCount);

        for (i=0; (i < configSize) ; i++)
        {
            status = xpsL3ClearEcmpRouteNextHop(devId, (pNhGrpEntry->baseNhId + i));
            if (status != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to delete nhId (%d) at index (%d) from nhGroup (%d)\n", nhId, i,
                      nhGrpId);
                return status;
            }
        }
    }

    pNhGrpEntry->nextHopIdx[pNhGrpEntry->nhCount] = nhId;
    pNhGrpEntry->nhCount++;

    // Create a tempNhIdx array
    if ((headNhIdx = xpMalloc(nhGrpMaxSize * sizeof(uint32_t))) == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Could not allocate temp Next Hop Group variables tempNhIdx, status %d",
              XP_ERR_OUT_OF_MEM);
        return XP_ERR_OUT_OF_MEM;
    }
    memset(headNhIdx, 0, nhGrpMaxSize * sizeof(uint32_t));
    tempNhIdx = (uint32_t *)headNhIdx;

    /*Re-calculate for even load-balancing*/
    configSize = xpsL3GetNextHopGroupConfigSize(nhGrpMaxSize, pNhGrpEntry->nhCount);

    // Program the ECMP entry into the HW
    memset(&ecmpEntry, 0, sizeof(ecmpEntry));
    ecmpEntry.numOfPaths = configSize;// pNhGrpEntry->groupSize;
    ecmpEntry.routeEntryBaseIndex = pNhGrpEntry->baseNhId;
    ecmpEntry.randomEnable = (xpsL3EcmpRandomEnableGet() ? GT_TRUE : GT_FALSE);

    if (PRV_CPSS_SIP_6_CHECK_MAC(devId))
    {
        ecmpEntry.multiPathMode = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E;
    }
    // XXX: Currently, XPS doesnt have an API to add program a NH group specific to a device.
    // It has only per-device API to add member to an ECMP group. But, we need to write one ECMP table
    // entry per NH group to a device. Without modifications to XPS API signatures, this API is the
    // place we can write ECMP table entry. But, the same entry shall programmed every time a new member
    // is added. This has no functional impact. We can prevent this by using a device map with flag in global
    // NH group DB entry. But, decision needs to be made whether to go with that approach or make the XPS APIs clean.

    cpssStatus = cpssHalWriteIpEcmpEntry(devId, pNhGrpEntry->keyNhGroupId,
                                         &ecmpEntry);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to add ECMP table entry: Index: %d, status: %d\n",
              pNhGrpEntry->keyNhGroupId, cpssStatus);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }

    for (i=0; i < configSize; i++)
    {
        tempNhIdx[i] = pNhGrpEntry->nextHopIdx[(i%(pNhGrpEntry->nhCount))];
    }

    status = xpsL3NextHopGroupProgramNhId(devId, pNhGrpEntry, configSize,
                                          tempNhIdx);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to program nhId (%d) into nhGroup (%d)\n", pNhGrpEntry, tempNhIdx);
        // Free the memory allocated for tempNhIdx
        xpFree(headNhIdx);
        return status;
    }

    /* Set the Nh id in the DB entry */
    xpsL3NextHopGroupSetDbEntryNhIdMaps(pNhGrpEntry, tempNhIdx);

    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "Added nhId %d to nhGrpId %d, nhCount %d", nhId, nhGrpId, pNhGrpEntry->nhCount);
    // Free the memory allocated for tempNhIdx
    xpFree(headNhIdx);

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


XP_STATUS xpsL3RemoveNextHopRouteNextHopGroup(xpsDevice_t devId,
                                              uint32_t nhGrpId, uint32_t nhId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS                 status      = XP_NO_ERR;
    xpsL3NextHopGroupEntry_t* pNhGrpEntry = NULL;
    xpsScope_t                scopeId;
    bool                      nhFound     = false;
    uint32_t                  *tempNhIdx;
    void                      *headNhIdx;
    uint32_t                  i           = 0;
    uint32_t configSize = 0;
    GT_STATUS                 cpssStatus = GT_OK;
    CPSS_DXCH_IP_ECMP_ENTRY_STC ecmpEntry;

    /* Get Scope Id from devId */
    if ((status = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return status;
    }

    status = xpsL3GetRouteNextHopGroup(scopeId, nhGrpId, &pNhGrpEntry);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Could not get next hop group from DB, nhGrpId %d, status %d", nhGrpId, status);
        return status;
    }

    if (pNhGrpEntry->nhCount == 0)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "The next hop group is empty, nhGrpId %d, userNhId %d", nhGrpId, nhId);
        status = XP_ERR_RESOURCE_NOT_AVAILABLE;
        return status;
    }
    // Create a tempNhIdx array
    if ((headNhIdx = xpMalloc(nhGrpMaxSize * sizeof(uint32_t))) == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Could not allocate temp Next Hop Group variables tempNhIdx, status %d",
              XP_ERR_OUT_OF_MEM);
        return XP_ERR_OUT_OF_MEM;
    }
    memset(headNhIdx, 0, nhGrpMaxSize * sizeof(uint32_t));
    tempNhIdx = (uint32_t *)headNhIdx;

    /*remove all members and update ARP refCnt*/
    if (pNhGrpEntry->nhCount != 0)
    {
        /*Re-calculate for even load-balancing*/
        configSize = xpsL3GetNextHopGroupConfigSize(nhGrpMaxSize, pNhGrpEntry->nhCount);

        for (i=0; (i < configSize) ; i++)
        {
            status = xpsL3ClearEcmpRouteNextHop(devId, (pNhGrpEntry->baseNhId + i));
            if (status != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to delete nhId (%d) at index (%d) from nhGroup (%d)\n", nhId, i,
                      nhGrpId);
                xpFree(headNhIdx);
                return status;
            }
        }
    }
    for (i = 0; i < pNhGrpEntry->nhCount && !nhFound; i++)
    {
        if (pNhGrpEntry->nextHopIdx[i] == nhId)
        {
            nhFound=true;
            for (uint32_t j = i; j < pNhGrpEntry->nhCount; j++)
            {
                pNhGrpEntry->nextHopIdx[j] = pNhGrpEntry->nextHopIdx[j+1];
            }
            pNhGrpEntry->nhCount--;
            pNhGrpEntry->nextHopIdx[pNhGrpEntry->nhCount] = 0;
        }
    }

    /* If no instances of this NH present, return error */
    if (!nhFound)
    {
        xpFree(headNhIdx);
        return XP_ERR_NOT_FOUND;
    }

#if 0
    for (i=0; (i < nhGrpMaxSize) && (pNhGrpEntry->nhCount); i++)
    {
        tempNhIdx[i] = pNhGrpEntry->nextHopIdx[(i%pNhGrpEntry->nhCount)];
    }
#endif

    if (pNhGrpEntry->nhCount > 0)
    {
        /*Re-calculate new count for even load-balancing*/
        configSize = xpsL3GetNextHopGroupConfigSize(nhGrpMaxSize, pNhGrpEntry->nhCount);
        // Program the ECMP entry into the HW
        memset(&ecmpEntry, 0, sizeof(ecmpEntry));
        ecmpEntry.numOfPaths = configSize;// pNhGrpEntry->groupSize;
        ecmpEntry.routeEntryBaseIndex = pNhGrpEntry->baseNhId;
        ecmpEntry.randomEnable = (xpsL3EcmpRandomEnableGet() ? GT_TRUE : GT_FALSE);

        if (PRV_CPSS_SIP_6_CHECK_MAC(devId))
        {
            ecmpEntry.multiPathMode = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E;
        }

        cpssStatus = cpssHalWriteIpEcmpEntry(devId, pNhGrpEntry->keyNhGroupId,
                                             &ecmpEntry);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to add ECMP table entry: Index: %d, status: %d\n",
                  pNhGrpEntry->keyNhGroupId, cpssStatus);
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }
        for (uint32_t i = 0; i < configSize; i++)
        {
            tempNhIdx[i] = pNhGrpEntry->nextHopIdx[(i % pNhGrpEntry->nhCount)];
        }

        status = xpsL3NextHopGroupProgramNhId(devId, pNhGrpEntry, configSize,
                                              tempNhIdx);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to program nhId (%d) into nhGroup (%d)\n", nhId, nhGrpId);
            xpFree(headNhIdx);
            return status;
        }
    }
    else
    {
        /*nothing to do, all entries removed*/
    }

    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "Removed nhId %d from nhGrpId %d, groupCount %d  nhCount %d",
          nhId, nhGrpId, pNhGrpEntry->groupCount, pNhGrpEntry->nhCount);
    /* Set the Nh id in the DB entry */
    xpsL3NextHopGroupSetDbEntryNhIdMaps(pNhGrpEntry, tempNhIdx);

    // Free the memory allocated for tempNhIdx
    xpFree(headNhIdx);

    XPS_FUNC_EXIT_LOG();
    return status;
}

/**
 * Gets the list of unique NH ids in a NH group
 */
static void xpL3GetUniqueNhIdListRouteNextHopGroup(xpsL3NextHopGroupEntry_t*
                                                   pNhGrpEntry, uint32_t *uniqueList, uint32_t *validSize)
{
    uint32_t i;
    std::vector<uint32_t>     nhIdList;
    std::vector<uint32_t>::iterator last;

    /* Collect all the NH into a vector */
    for (i = 0; i < pNhGrpEntry->groupSize; i++)
    {
        if (pNhGrpEntry->nhIdMap[i])
        {
            nhIdList.push_back(pNhGrpEntry->nhIdMap[i]);
        }
    }

    /*
     * 1. Sort the vector.
     * 2. Remove the duplicate NH ids from the vector.
     * 3. Erase excess elements
     */
    std::sort(nhIdList.begin(), nhIdList.end());
    last = std::unique(nhIdList.begin(), nhIdList.end());
    nhIdList.erase(last, nhIdList.end());

    std::copy(nhIdList.begin(), nhIdList.end(), uniqueList);
    *validSize = nhIdList.size();
}


XP_STATUS xpsL3GetCountRouteNextHopGroup(uint32_t nhGrpId, uint32_t* pCount)
{
    return xpsL3GetCountRouteNextHopGroupScope(XP_SCOPE_DEFAULT, nhGrpId, pCount);
}

XP_STATUS xpsL3GetCountRouteNextHopGroupScope(xpsScope_t scopeId,
                                              uint32_t nhGrpId, uint32_t* pCount)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS                 status       = XP_NO_ERR;
    xpsL3NextHopGroupEntry_t* pNhGrpEntry  = NULL;

    status = xpsL3GetRouteNextHopGroup(scopeId, nhGrpId, &pNhGrpEntry);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Could not get next hop group, status %d, nhGrpId %d", status, nhGrpId);
        return status;
    }

    *pCount = pNhGrpEntry->nhCount;

    XPS_FUNC_EXIT_LOG();
    return status;
}

XP_STATUS xpsL3GetNextNextHopRouteNextHopGroup(uint32_t nhGrpId,
                                               uint32_t* pNhId, uint32_t* pNextNhId)
{
    return xpsL3GetNextNextHopRouteNextHopGroupScope(XP_SCOPE_DEFAULT, nhGrpId,
                                                     pNhId, pNextNhId);

}

XP_STATUS xpsL3GetNextNextHopRouteNextHopGroupScope(xpsScope_t scopeId,
                                                    uint32_t nhGrpId, uint32_t* pNhId, uint32_t* pNextNhId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS                 status       = XP_NO_ERR;
    xpsL3NextHopGroupEntry_t* pNhGrpEntry  = NULL;
    uint32_t                  i            = 0;
    uint32_t                  *uniqueNhIdList;
    void                      *headNhIdList;
    uint32_t                  uniqueNhIdSize = 0;

    status = xpsL3GetRouteNextHopGroup(scopeId, nhGrpId, &pNhGrpEntry);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Could not get next hop group, status %d, nhGrpId %d", status, nhGrpId);
        return status;
    }

    if (pNhGrpEntry->groupSize == 0)
    {
        status = XP_ERR_NOT_FOUND;
        return status;
    }

    // Create a uniqueNhIdList array
    if ((headNhIdList = xpMalloc(nhGrpMaxSize * sizeof(uint32_t))) == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Could not allocate temp Next Hop Group variables uniqueNhIdList, status %d",
              status);
        return status;
    }
    uniqueNhIdList = (uint32_t *) headNhIdList;
    /*
     * XXX: Due to weight configuration/re-configuration, the given NH id may present in multiple
     * non-consecutive locations in the NH group state. Need return unique next NH id in this case.
     * The other way to achieve this is to insert the NH id in sorted order while adding itself. This
     * approach will have some cons and need to change the state and related code. Deferring the activity
     * as TODO later on.
     */
    xpL3GetUniqueNhIdListRouteNextHopGroup(pNhGrpEntry, uniqueNhIdList,
                                           &uniqueNhIdSize);

    if (uniqueNhIdSize == 0)
    {
        xpFree(headNhIdList);
        return XP_ERR_NOT_FOUND;
    }

    /* First NH id case */
    if (pNhId == NULL)
    {
        *pNextNhId = uniqueNhIdList[0];
        xpFree(headNhIdList);
        return XP_NO_ERR;
    }

    /* Check for the given current NH id, and return the next one */
    for (i = 0; i < (uniqueNhIdSize - 1); i++)
    {
        if (uniqueNhIdList[i] == *pNhId)
        {
            *pNextNhId = uniqueNhIdList[i + 1];
            xpFree(headNhIdList);
            return XP_NO_ERR;
        }
    }

    // Free the memory allocated for uniqueNhIdList
    xpFree(headNhIdList);

    /* No next element */
    status = XP_ERR_NOT_FOUND;

    XPS_FUNC_EXIT_LOG();

    return status;
}

int xpsL3ExistsRouteNextHopGroup(uint32_t nhGrpId)
{
    return xpsL3ExistsRouteNextHopGroupScope(XP_SCOPE_DEFAULT, nhGrpId);
}

int xpsL3ExistsRouteNextHopGroupScope(xpsScope_t scopeId, uint32_t nhGrpId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS                 status       = XP_NO_ERR;
    xpsL3NextHopGroupEntry_t* pNhGrpEntry  = NULL;

    status = xpsL3GetRouteNextHopGroup(scopeId, nhGrpId, &pNhGrpEntry);

    XPS_FUNC_EXIT_LOG();
    return (status == XP_NO_ERR);
}

XP_STATUS xpsL3SetRouteNextHopNextHopGroup(xpsDevice_t devId, uint32_t nhId,
                                           xpsL3NextHopEntry_t *pL3NextHopEntry)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;

    /* Update next hop fields in HW for all groups where the next hop participates */
    status = xpsL3SetRouteNextHopGroup(devId, nhId, pL3NextHopEntry);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Could not apply next hop to HW, nhId %d", nhId);
        return status;
    }

    XPS_FUNC_EXIT_LOG();

    return status;
}

XP_STATUS xpsL3DestroyRouteNextHopNextHopGroup(xpsDevice_t devId,
                                               uint32_t nhEcmpSize, uint32_t nhId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status  = XP_NO_ERR;
    xpsScope_t scopeId;

    /* Get Scope Id from devId */
    if ((status = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return status;
    }

    if (xpsL3ExistsRouteNextHopGroupScope(scopeId, nhId))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Can not destroy next hop that is used by a group, nhId %d", nhId);
        return XP_ERR_INVALID_PARAMS;
    }

    status = xpsL3ClearRouteNextHop(devId, nhId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Could not clear next hop, nhId %d", nhId);
        return status;
    }

    status = xpsL3DestroyRouteNextHopScope(scopeId, nhEcmpSize, nhId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Could not destroy next hop, nhEcmpSize %d, nhId %d", nhEcmpSize, nhId);
        return status;
    }

    status = xpsL3RemoveRouteNextHopAllGroup(devId, nhId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Could not remove next hop from groups, nhId %d", nhId);
        return status;
    }

    XPS_FUNC_EXIT_LOG();

    return status;
}

/*
 * Static Function Implementation
 */

/**
 * \brief Next Hop Group State Key Compare function
 *
 * This API is used by the state manager as a method to compare
 * keys
 *
 * \param [in] void* key1
 * \param [in] void* key2
 *
 * \return int32_t
 */
static int32_t nextHopGroupKeyCompare(void *key1, void *key2)
{
    return ((int32_t)(((xpsL3NextHopGroupEntry_t*)key1)->keyNhGroupId) -
            (int32_t)(((xpsL3NextHopGroupEntry_t*)key2)->keyNhGroupId));
}

/**
 * \brief Next Hop Group global variables State Key Compare function
 *
 * This API is used by the state manager as a method to compare
 * keys
 *
 * \param [in] void* key1
 * \param [in] void* key2
 *
 * \return int32_t
 */
static int32_t nextHopGroupStaticVariablesKeyCompare(void *key1, void *key2)
{
    return ((int32_t)(((xpsL3NextHopGroupStaticEntry_t*)key1)->keyStaticDataType)
            - (int32_t)(((xpsL3NextHopGroupStaticEntry_t*)key2)->keyStaticDataType));
}

/**
 * \brief API to initialize the Next Hop Group entry structure
 *
 * \param [in] pNhGrpEntry
 * \param [in] nhGrpId
 * \param [in] groupSize
 *
 * \return XP_STATUS
 */
static XP_STATUS nextHopGroupEntryInit(xpsL3NextHopGroupEntry_t *pNhGrpEntry,
                                       uint32_t nhGrpId, uint32_t nhBaseId, uint32_t groupSize)
{
    XP_STATUS status = XP_NO_ERR;
    memset(pNhGrpEntry, 0, sizeof(xpsL3NextHopGroupEntry_t));
    pNhGrpEntry->hwPbrLeafId = XPS_INTF_INVALID_ID;

    // Create a nhIdMap array
    if ((status = xpsStateHeapMalloc(nhGrpMaxSize * sizeof(uint32_t),
                                     (void **)&(pNhGrpEntry->nhIdMap))) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Could not allocate Next Hop Group variables nhIdMap, status %d", status);
        return status;
    }
    // Create a nextHopIdx array
    if ((status = xpsStateHeapMalloc(nhGrpMaxSize * sizeof(uint32_t),
                                     (void **)&(pNhGrpEntry->nextHopIdx))) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Could not allocate Next Hop Group global variables nextHopIdx, status %d",
              status);
        // Free the memory allocated for nextHopIdx
        xpsStateHeapFree(pNhGrpEntry->nhIdMap);

        return status;
    }

    memset(pNhGrpEntry->nhIdMap, 0, nhGrpMaxSize * sizeof(uint32_t));
    memset(pNhGrpEntry->nextHopIdx, 0, nhGrpMaxSize * sizeof(uint32_t));

    pNhGrpEntry->keyNhGroupId = nhGrpId;
    pNhGrpEntry->baseNhId = nhBaseId;
    pNhGrpEntry->groupSize = groupSize;

    return XP_NO_ERR;
}

/**
 * \brief API to deinitialize the Next Hop Group entry structure
 *
 * \param [in] pNhGrpEntry
 *
 * \return XP_STATUS
 */
static XP_STATUS nextHopGroupEntryDeInit(xpsL3NextHopGroupEntry_t *pNhGrpEntry)
{
    XP_STATUS status = XP_NO_ERR;

    // Free the memory allocated for nextHopIdx
    status = xpsStateHeapFree(pNhGrpEntry->nextHopIdx);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to free Next Hop Group global variables nextHopIdx, status %d", status);
    }
    // Free the memory allocated for nhIdMap
    status = xpsStateHeapFree(pNhGrpEntry->nhIdMap);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to free Next Hop Group global variables nhIdMap, status %d", status);
    }

    memset(pNhGrpEntry, 0, sizeof(xpsL3NextHopGroupEntry_t));

    return XP_NO_ERR;
}

/**
 * \brief Walk through the next hop groups
 *
 * \param [in] scopeId Scope identifier
 * \param [in] pNhGrpId
 * \param [out] ppNextNhGrpId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3GetNextRouteNextHopGroup(xpsScope_t scopeId,
                                        xpsL3NextHopGroupEntry_t* pNhGrpEntry, xpsL3NextHopGroupEntry_t** ppNhGrpEntry)
{
    XP_STATUS status = XP_NO_ERR;

    status = xpsStateGetNextData(scopeId, nextHopGroupDbHandle, pNhGrpEntry,
                                 (void**)ppNhGrpEntry);

    if (*ppNhGrpEntry == NULL)
    {
        return XP_ERR_KEY_NOT_FOUND;
    }

    return status;
}

/**
 * \brief Get a pointer to an Next Hop Group entry
 *
 * \param [in] scopeId Scope identifier
 * \param [in] nhGrpId
 * \param [out] pNhGrpEntry
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3GetRouteNextHopGroup(xpsScope_t scopeId, uint32_t nhGrpId,
                                    xpsL3NextHopGroupEntry_t** ppNhGrpEntry)
{
    XPS_FUNC_ENTRY_LOG();
    XP_STATUS                status  = XP_NO_ERR;
    xpsL3NextHopGroupEntry_t nhGrpKey;

    memset(&nhGrpKey, 0, sizeof(nhGrpKey));

    nhGrpKey.keyNhGroupId = nhGrpId;

    status = xpsStateSearchData(scopeId, nextHopGroupDbHandle, &nhGrpKey,
                                (void**)ppNhGrpEntry);

    if (*ppNhGrpEntry == NULL)
    {
        return XP_ERR_KEY_NOT_FOUND;
    }

    XPS_FUNC_EXIT_LOG();
    return status;
}

/**
 * \brief Apply next hop group destroy to HW
 *
 * \param [in] devId
 * \param [in] pNhGrpInfo
 *
 * \return int
 */
static XP_STATUS xpsL3DestroyApplyRouteNextHopGroup(xpsDevice_t devId,
                                                    xpsL3NextHopGroupEntry_t* pNhGrpInfo)
{
    XP_STATUS                 status       = XP_NO_ERR;
    uint32_t                  i            = 0;
    xpsScope_t                scopeId;
    GT_STATUS                 cpssStatus = GT_OK;
    CPSS_DXCH_IP_ECMP_ENTRY_STC ecmpEntry;
    uint32_t nhGrpBlkId       = 0;
    uint32_t maxNHEntries = 0;

    /* Get Scope Id from devId */
    if ((status = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return status;
    }

    for (i = 0; i < pNhGrpInfo->groupSize; i++)
    {
        status = xpsL3ClearEcmpRouteNextHop(devId, pNhGrpInfo->baseNhId + i);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Could not clear next hop in HW, status %d, nhId %d", status,
                  pNhGrpInfo->baseNhId + i);
            return status;
        }
    }

    // Create an invalid entry and write to HW.
    memset(&ecmpEntry, 0, sizeof(ecmpEntry));
    ecmpEntry.routeEntryBaseIndex = 0;
    ecmpEntry.multiPathMode = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E;
    ecmpEntry.numOfPaths = 1;
    ecmpEntry.randomEnable = (xpsL3EcmpRandomEnableGet() ? GT_TRUE : GT_FALSE);;

    if (PRV_CPSS_SIP_6_CHECK_MAC(devId))
    {
        ecmpEntry.multiPathMode = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E;
    }

    // Invalidate the ECMP table entry

    cpssStatus = cpssHalWriteIpEcmpEntry(devId, pNhGrpInfo->keyNhGroupId,
                                         &ecmpEntry);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to write invalidated ECMP entry into HW: index: %d: status: %d\n",
              pNhGrpInfo->keyNhGroupId, cpssStatus);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }

    // Free the ECMP table index
    status = xpsL3FreeEcmpTableIndex(scopeId, pNhGrpInfo->keyNhGroupId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to free ecmp table index: %d Status: %d\n", pNhGrpInfo->keyNhGroupId,
              status);
        return status;
    }

    cpssStatus = cpssDxChCfgTableNumEntriesGet(0,
                                               CPSS_DXCH_CFG_TABLE_ROUTER_NEXT_HOP_E, &maxNHEntries);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get NH table depth for Status %d\n", cpssStatus);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }

    nhGrpBlkId = (maxNHEntries - (pNhGrpInfo->baseNhId  +
                                  nhGrpMaxSize))/nhGrpMaxSize;

    status = xpsL3DestroyNextHopGroupScope(scopeId, pNhGrpInfo->groupSize,
                                           nhGrpBlkId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Could not destroy next hops in HW, status %d, nhId %d, nhEcmpSize %d", status,
              pNhGrpInfo->keyNhGroupId, pNhGrpInfo->groupSize);
        return status;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG, "Destroyed : %d Alloc :%d \n",
          pNhGrpInfo->baseNhId, nhGrpBlkId);
    return status;
}

/**
 * \brief Copy from one next hop to another
 *
 * \param [in] devId
 * \param [in] srcNhId
 * \param [in] dstNhId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3CopyNextHopAndProgramNextHopInGroup(xpsDevice_t devId,
                                                   uint32_t srcNhId, uint32_t dstNhId)
{
    XPS_FUNC_ENTRY_LOG();
    CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC cpssNhEntry;
    GT_STATUS cpssStatus = GT_OK;
    xpsL3NextHopEntry_t xpsL3NextHopEntry;
    XP_STATUS status;

    memset(&cpssNhEntry, 0, sizeof(cpssNhEntry));
    memset(&xpsL3NextHopEntry, 0, sizeof(xpsL3NextHopEntry_t));

    /* Read the CPSS Route Nexthop entry from SW DB */
    status = xpsL3GetRouteNextHopDb(devId, srcNhId, &xpsL3NextHopEntry);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " get nh db failed for nh id %d ret %d", srcNhId, status);
        return status;
    }
    /*this also increments ARP ptr ref cnt*/
    status = xpsL3ConvertXpsNhEntryToCpssNhEntry(devId, &xpsL3NextHopEntry,
                                                 &cpssNhEntry);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "prepare nh hw entry failed nh id %d ret %d", srcNhId, status);
        return status;
    }
#if 0
    cpssStatus = cpssDxChIpUcRouteEntriesRead(devId, srcNhId, &cpssNhEntry, 1);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to read the NH entry from Hw : NH id : %d\n", srcNhId);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }
#endif

    /* Copy it to destination */
    cpssStatus = cpssHalWriteIpUcRouteEntries(devId, dstNhId, &cpssNhEntry, 1);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to copy to destination route nexthop entry for NH : %d", dstNhId);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }

    XPS_FUNC_EXIT_LOG();
    return XP_NO_ERR;
}

/**
 * \brief Update ECMP size for matched next hop ids.
 *
 * \param [in] devId
 * \param [in] nhIdx
 * \param [in] srcNhId
 * \param [in] dstNhId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3CopyNextHopRouteNextHopGroup(xpsDevice_t devId, uint32_t srcNhId,
                                            uint32_t dstNhId)
{
    XPS_FUNC_ENTRY_LOG();
    XP_STATUS status = XP_NO_ERR;
    xpsL3NextHopEntry_t       l3NextHopEntry;

    memset(&l3NextHopEntry, 0, sizeof(l3NextHopEntry));

    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
          "Copy contents of nhId %d to %d", srcNhId, dstNhId);

    status = xpsL3GetRouteNextHop(devId, srcNhId, &l3NextHopEntry);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get a next hop from HW, nhId %d, status %d", srcNhId, status);
        return status;
    }

    status = xpsL3SetRouteNextHopAndNextHopGroup(devId, dstNhId, &l3NextHopEntry);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set the next hop to HW, nhId %d, status %d", dstNhId, status);
        return status;
    }

    XPS_FUNC_EXIT_LOG();
    return XP_NO_ERR;
}

/**
 * \brief Insert static variables into the DB
 *
 * \param [in] scopeId Scope identifier
 *
 * \return XP_STATUS
 */
static XP_STATUS xpsL3InsertStaticVariablesRouteNextHopGroup(xpsScope_t scopeId)
{
    XP_STATUS                        status           = XP_NO_ERR;
    xpsL3NextHopGroupStaticEntry_t*  pNhGrpStaticVar  = NULL;

    // Create a new structure
    if ((status = xpsStateHeapMalloc(sizeof(xpsL3NextHopGroupStaticEntry_t),
                                     (void**)&pNhGrpStaticVar)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Could not allocate Next Hop Group global variables structure, status %d",
              status);
        return status;
    }

    // Initialize the allocated entry
    pNhGrpStaticVar->keyStaticDataType = XPS_L3_NHGRP_STATIC_VARIABLES;
    pNhGrpStaticVar->groupSize = 0;

    // Insert the state into the database
    if ((status = xpsStateInsertData(scopeId, xpsL3NhGrpStaticDataDbHandle,
                                     (void*)pNhGrpStaticVar)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Could not insert Next Hop Group global variables structure to the DB, status %d",
              status);
        return status;
    }

    return status;
}

/**
 * \brief Remove static variables from the DB
 *
 * \param [in] scopeId Scope identifier
 *
 * \return XP_STATUS
 */
static XP_STATUS xpsL3RemoveStaticVariablesRouteNextHopGroup(xpsScope_t scopeId)
{
    XP_STATUS                        status           = XP_NO_ERR;
    xpsL3NextHopGroupStaticEntry_t*  pNhGrpStaticVar  = NULL;
    xpsL3NextHopGroupStaticEntry_t   key;

    memset(&key, 0, sizeof(key));

    key.keyStaticDataType = XPS_L3_NHGRP_STATIC_VARIABLES;

    // Remove the corresponding state
    status = xpsStateDeleteData(scopeId, xpsL3NhGrpStaticDataDbHandle, &key,
                                (void**)&pNhGrpStaticVar);
    if ((pNhGrpStaticVar == NULL) || (status != XP_NO_ERR))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to delete Next Hop Group global variables from DB, status %d", status);
        return status;
    }

    // Free the memory allocated for the corresponding state
    status = xpsStateHeapFree(pNhGrpStaticVar);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to free Next Hop Group global variables structure, status %d", status);
        return status;
    }

    return XP_NO_ERR;
}

/**
 * \brief Get a pointer to the Next Hop Group static variable entry
 *
 * \param [in] scopeId Scope identifier
 * \param [out] ppNhGrpStaticVarEntry
 *
 * \return XP_STATUS
 */
static XP_STATUS xpsL3GetStaticVariablesRouteNextHopGroup(xpsScope_t scopeId,
                                                          xpsL3NextHopGroupStaticEntry_t** ppNhGrpStaticVarEntry)
{
    XP_STATUS                      status  = XP_NO_ERR;
    xpsL3NextHopGroupStaticEntry_t key;

    memset(&key, 0, sizeof(key));

    key.keyStaticDataType = XPS_L3_NHGRP_STATIC_VARIABLES;

    status = xpsStateSearchData(scopeId, xpsL3NhGrpStaticDataDbHandle, &key,
                                (void**)ppNhGrpStaticVarEntry);

    if (*ppNhGrpStaticVarEntry == NULL)
    {
        return XP_ERR_KEY_NOT_FOUND;
    }

    return status;
}

/**
 * \brief Update destination next hops for all groups
 *
 * \param [in] devId Device identifier
 * \param [in] nhId Next hop identifier
 * \param [in] pL3NextHopEntry Next hop contents
 *
 * \return int
 */
static XP_STATUS xpsL3SetRouteNextHopGroup(xpsDevice_t devId, uint32_t nhId,
                                           xpsL3NextHopEntry_t *pL3NextHopEntry)
{
    XP_STATUS                 status        = XP_NO_ERR;
    xpsL3NextHopGroupEntry_t* pNhGrpEntry   = NULL;
    uint32_t                  i             = 0;
    xpsScope_t                scopeId;

    /* Get Scope Id from devId */
    if ((status = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return status;
    }

    while (xpsL3GetNextRouteNextHopGroup(scopeId, pNhGrpEntry,
                                         &pNhGrpEntry) == XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG, "keyGroupId %d, groupSize %d",
              pNhGrpEntry->keyNhGroupId, pNhGrpEntry->groupSize);

        for (i = 0; i < pNhGrpEntry->groupSize; i++)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
                  "keyGroupId %d, groupSize %d, userNhId %d, nhId %d",
                  pNhGrpEntry->keyNhGroupId, pNhGrpEntry->groupSize, pNhGrpEntry->nhIdMap[i],
                  nhId);

            if (pNhGrpEntry->nhIdMap[i] == nhId)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
                      "update to route nhId %d instead of %d", pNhGrpEntry->baseNhId + i, nhId);
                status = xpsL3RemoveRouteNhHw(devId, pNhGrpEntry->baseNhId + i);
                /*directly update HW entry */
                status = xpsL3SetRouteNextHopHw(devId, pNhGrpEntry->baseNhId + i,
                                                pL3NextHopEntry);
                if (status != XP_NO_ERR)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Could not set next hop to HW, xpNhId %d", pNhGrpEntry->baseNhId + i);
                    return status;
                }
            }
        }
    }

    return status;
}

XP_STATUS xpsL3GetRouteNextHopNextHopGroup(xpsDevice_t devId, uint32_t nhId,
                                           uint32_t *nhGrpId)
{
    XPS_FUNC_ENTRY_LOG();
    XP_STATUS                 status        = XP_NO_ERR;
    xpsL3NextHopGroupEntry_t* pNhGrpEntry   = NULL;
    uint32_t*                 pNhId         = NULL;
    uint32_t                  curNhId       = 0;
    xpsScope_t                scopeId;

    if (nhGrpId == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Null pointer as an argument\n");
        return XP_ERR_NULL_POINTER;
    }

    /* Get Scope Id from devId */
    if ((status = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return status;
    }

    while (xpsL3GetNextRouteNextHopGroup(scopeId, pNhGrpEntry,
                                         &pNhGrpEntry) == XP_NO_ERR)
    {
        while (xpsL3GetNextNextHopRouteNextHopGroupScope(scopeId,
                                                         pNhGrpEntry->keyNhGroupId, pNhId, &curNhId) == XP_NO_ERR)
        {
            if (curNhId == nhId)
            {
                /* If given nhId exist in any NH group then return */
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "nhId : %d is already bound with nhGroup : %d\n", curNhId,
                      pNhGrpEntry->keyNhGroupId);
                *nhGrpId = pNhGrpEntry->keyNhGroupId;
                return XP_NO_ERR;
            }

            pNhId = &curNhId;
        }
    }

    XPS_FUNC_EXIT_LOG();
    return XP_ERR_NOT_FOUND;
}

/**
 * \brief Remove next hop from all groups
 *
 * \param [in] devId Device identifier
 * \param [in] nhId Next hop identifier
 *
 * \return XP_STATUS
 */
static XP_STATUS xpsL3RemoveRouteNextHopAllGroup(xpsDevice_t devId,
                                                 uint32_t nhId)
{
    XP_STATUS                 status        = XP_NO_ERR;
    xpsL3NextHopGroupEntry_t* pNhGrpEntry   = NULL;
    uint32_t*                 pNhId         = NULL;
    uint32_t                  curNhId       = 0;
    xpsScope_t                scopeId;

    /* Get Scope Id from devId */
    if ((status = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return status;
    }

    while (xpsL3GetNextRouteNextHopGroup(scopeId, pNhGrpEntry,
                                         &pNhGrpEntry) == XP_NO_ERR)
    {
        while ((status = xpsL3GetNextNextHopRouteNextHopGroupScope(scopeId,
                                                                   pNhGrpEntry->keyNhGroupId, pNhId, &curNhId)) == XP_NO_ERR)
        {
            if (curNhId == nhId)
            {
                status = xpsL3RemoveNextHopRouteNextHopGroup(devId, pNhGrpEntry->keyNhGroupId,
                                                             nhId);
                break;
            }

            pNhId = &curNhId;
        }
    }

    return status;
}

int xpsL3ExistsNextHopRouteNextHopGroupScope(xpsScope_t scopeId,
                                             uint32_t nhGrpId, uint32_t nhId)
{
    XPS_FUNC_ENTRY_LOG();
    xpsL3NextHopGroupEntry_t *pNhGrpEntry;
    uint32_t i;
    XP_STATUS status;

    status = xpsL3GetRouteNextHopGroup(scopeId, nhGrpId, &pNhGrpEntry);
    if (status != XP_NO_ERR)
    {
        return status;
    }

    /* If the NH id is present in the group */
    if (pNhGrpEntry != NULL && pNhGrpEntry->nhIdMap != NULL)
    {
        for (i = 0; i < pNhGrpEntry->groupSize; i++)
            if (pNhGrpEntry->nhIdMap[i] == nhId)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEFAULT,
                      "Found nhGrpId %d NhId %d idx :%d \n", nhGrpId, nhId, i);
                return true;
            }
    }

    XPS_FUNC_EXIT_LOG();
    return false;
}

int xpsL3ExistsNextHopRouteNextHopGroup(xpsDevice_t devId, uint32_t nhGrpId,
                                        uint32_t nhId)
{
    XPS_FUNC_ENTRY_LOG();
    xpsScope_t scopeId;
    XP_STATUS status;

    status = xpsScopeGetScopeId(devId, &scopeId);
    if (status != XP_NO_ERR)
    {
        return status;
    }

    XPS_FUNC_EXIT_LOG();
    return xpsL3ExistsNextHopRouteNextHopGroupScope(scopeId, nhGrpId, nhId);
}

XP_STATUS xpsL3GetNextHopWeightRouteNextHopGroup(xpsDevice_t devId,
                                                 uint32_t nhGrpId, uint32_t nhId, uint32_t *weight)
{
    XPS_FUNC_ENTRY_LOG();
    xpsScope_t scopeId;
    XP_STATUS status;

    status = xpsScopeGetScopeId(devId, &scopeId);
    if (status != XP_NO_ERR)
    {
        return status;
    }

    XPS_FUNC_EXIT_LOG();
    return xpsL3GetNextHopWeightRouteNextHopGroupScope(scopeId, nhGrpId, nhId,
                                                       weight);
}

XP_STATUS xpsL3GetNextHopWeightRouteNextHopGroupScope(xpsScope_t scopeId,
                                                      uint32_t nhGrpId, uint32_t nhId, uint32_t *weight)
{
    XPS_FUNC_ENTRY_LOG();
    xpsL3NextHopGroupEntry_t *pNhGrpEntry;
    XP_STATUS status;
    uint32_t count = 0;

    status = xpsL3GetRouteNextHopGroup(scopeId, nhGrpId, &pNhGrpEntry);
    if (status != XP_NO_ERR)
    {
        return status;
    }

    /* Get the number of instances of the nexthop */
    xpsL3GetNextHopCountRouteNextHopGroup(pNhGrpEntry, nhId, &count);

    /* If count is 0, the NH doesnt exist in the NH group. Return error */
    if (count == 0)
    {
        return XP_ERR_NOT_FOUND;
    }

    *weight = count;

    XPS_FUNC_EXIT_LOG();
    return XP_NO_ERR;
}

XP_STATUS xpsL3SetNextHopWeightRouteNextHopGroupScope(xpsDevice_t devId,
                                                      uint32_t nhGrpId, uint32_t nhId, uint32_t weight)

{
    return XP_ERR_OP_NOT_SUPPORTED;
}

XP_STATUS xpsL3SetNextHopWeightRouteNextHopGroup(xpsDevice_t devId,
                                                 uint32_t nhGrpId, uint32_t nhId, uint32_t weight)
{
    xpsScope_t scopeId;
    XP_STATUS status;

    status = xpsScopeGetScopeId(devId, &scopeId);
    if (status != XP_NO_ERR)
    {
        return status;
    }

    return xpsL3SetNextHopWeightRouteNextHopGroupScope(scopeId, nhGrpId, nhId,
                                                       weight);
}

XP_STATUS xpsCountNextHopGroupObjects(uint32_t *count)
{
    XPS_FUNC_ENTRY_LOG();
    XP_STATUS  retVal  = XP_NO_ERR;
    xpsScope_t scopeId = XP_SCOPE_DEFAULT;

    retVal = xpsStateGetCount(scopeId, nextHopGroupDbHandle, count);
    XPS_FUNC_EXIT_LOG();
    return retVal;
}

XP_STATUS xpsGetNhGrpIdList(xpsScope_t scopeId, uint32_t *nexthopGrp_id)
{
    XPS_FUNC_ENTRY_LOG();
    XP_STATUS                 result                  = XP_NO_ERR;
    xpsL3NextHopGroupEntry_t *curNextHopGroupDbEntry  = NULL;
    xpsL3NextHopGroupEntry_t *prevNextHopGroupDbEntry = NULL;
    uint32_t                  count                   = 0;

    result = xpsStateGetNextData(scopeId, nextHopGroupDbHandle, NULL,
                                 (void **)&curNextHopGroupDbEntry);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
              "Failed to get Data | retVal : %d\n", result);
        return result;
    }

    while (curNextHopGroupDbEntry)
    {
        //Store the node information in previous and get-next
        prevNextHopGroupDbEntry = curNextHopGroupDbEntry;
        nexthopGrp_id[count]    = prevNextHopGroupDbEntry->keyNhGroupId;

        result = xpsStateGetNextData(scopeId, nextHopGroupDbHandle,
                                     (xpsDbKey_t)prevNextHopGroupDbEntry, (void **)&curNextHopGroupDbEntry);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
                  "Failed to get Data | retVal : %d\n", result);
            return result;
        }
        count++;
    }
    XPS_FUNC_EXIT_LOG();
    return result;
}

XP_STATUS xpsL3GetMaxEcmpSize(xpDevice_t devId, uint32_t *ecmpSize)
{
    XPS_FUNC_ENTRY_LOG();
    XP_STATUS status = XP_NO_ERR;

    if (IS_DEVICE_VALID(devId) == 0)
    {
        return XP_ERR_INVALID_DEV_ID;
    }

    *ecmpSize = nhGrpMaxSize;//XPS_L3_NH_GRP_MAX_NEXTHOPS_PER_GRP;
    XPS_FUNC_EXIT_LOG();
    return status;
}

XP_STATUS xpsL3GetNextHopGroupMaxEcmpSize(uint32_t *ecmpSize)
{
    XPS_FUNC_ENTRY_LOG();
    XP_STATUS status = XP_NO_ERR;

    if (ecmpSize == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error reading max ecmp size\n");
        return XP_ERR_INVALID_INPUT;
    }
    *ecmpSize = nhGrpMaxSize;

    XPS_FUNC_EXIT_LOG();
    return status;
}

XP_STATUS xpsL3SetNextHopGroupMaxEcmpSize(uint32_t ecmpSize)
{
    XPS_FUNC_ENTRY_LOG();
    XP_STATUS status = XP_NO_ERR;

    nhGrpMaxSize = ecmpSize;


    XPS_FUNC_EXIT_LOG();
    return status;
}

XP_STATUS xpsL3GetNextHopGroupsMaxNum(uint32_t *maxNum)
{
    XPS_FUNC_ENTRY_LOG();
    XP_STATUS status = XP_NO_ERR;

    if (maxNum == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "NULL output variable passed\n");
        return XP_ERR_INVALID_INPUT;
    }

    // Get NH group count
    // XXX: Using devid as 0 as this API signature doesnt have DevId. Need to change signature?
    status = xpsL3GetNexthopGroupCount(0, maxNum);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get NH group count. Status : %d\n", status);
        return status;
    }

    XPS_FUNC_EXIT_LOG();
    return status;
}

#if 0
//TODO: Need to revise this API and signature
XP_STATUS xpsL3SetNextHopGroupsMaxMem(xpRangeProfile_t *rangeProfile)
{
    XPS_FUNC_ENTRY_LOG();
    XP_STATUS status = XP_NO_ERR;

    if (rangeProfile == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: not getting valid profile in SetNextHopGroupsMaxMem\n");
        return XP_ERR_INVALID_INPUT;
    }
    nhGrpMaxMem = rangeProfile->nhRangeInfo[1].rangeSize;

    XPS_FUNC_EXIT_LOG();
    return status;
}
#endif

static XP_STATUS xpsL3AllocateEcmpTableIndex(xpsScope_t scopeId,
                                             uint32_t *index)
{
    XP_STATUS status = XP_NO_ERR;

    status = xpsAllocatorAllocateId(scopeId, XPS_ALLOCATOR_ECMP_TBL, index);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to allocate ECMP table index. Status : %d\n", status);
        return status;
    }

    return XP_NO_ERR;
}

static XP_STATUS xpsL3FreeEcmpTableIndex(xpsScope_t scopeId, uint32_t index)
{
    XP_STATUS status = XP_NO_ERR;

    status = xpsAllocatorReleaseId(scopeId, XPS_ALLOCATOR_ECMP_TBL, index);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to release ECMP table index. Status : %d\n", status);
        return status;
    }

    return XP_NO_ERR;
}

static XP_STATUS xpsL3RouteNexthopInitializeEcmp(GT_U8 cpssDevNum)
{
    GT_STATUS cpssStatus = GT_OK;
    GT_U8 saltValue = 0;
    GT_U32 crcSeed = 0;
    GT_U32 startBit = 16, numBits = 16; // using all bits of hash

    // NOTE: The config here follows the example from prvTgfBasicIpv4UcEcmpRoutingHashCheckLttRouteConfigurationSet unit test code

    // Set global hash mode
    cpssStatus = cpssDxChTrunkHashGlobalModeSet(cpssDevNum,
                                                CPSS_DXCH_TRUNK_LBH_PACKETS_INFO_CRC_E);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set global hash mode: Dev num: %d Status: %d\n", cpssDevNum,
              cpssStatus);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }

    // Set CRC parameters
    cpssStatus = cpssDxChTrunkHashCrcParametersSet(cpssDevNum,
                                                   CPSS_DXCH_TRUNK_LBH_CRC_32_MODE_E, crcSeed, 0);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set CRC parameters: Dev num: %d Status: %d\n", cpssDevNum,
              cpssStatus);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }

    for (int i = 0; i < XPS_NHGRP_NUM_SALT_BYTES; i++)
    {
        cpssStatus = cpssDxChTrunkHashCrcSaltByteSet(cpssDevNum, 0, i, saltValue);
        if (cpssStatus != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set salt byte %d value %d at device %d: status %d\n", i, saltValue,
                  cpssDevNum, cpssStatus);
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }

    }
    cpssStatus = cpssDxChIpEcmpHashNumBitsSet(cpssDevNum, startBit, numBits);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set the hash bit config: Device: %d, status: %d\n", cpssDevNum,
              cpssStatus);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }

    return XP_NO_ERR;
}

XP_STATUS xpsL3RouteNextHopGroupAddDevice(xpsDevice_t devId,
                                          xpsInitType_t initType)
{
    GT_U8 cpssDevNum;
    XP_STATUS status = XP_NO_ERR;

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        status = xpsL3RouteNexthopInitializeEcmp(cpssDevNum);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set ECMP parameters on device: %d Status %d\n", cpssDevNum, status);
            return status;
        }
    }

    return XP_NO_ERR;
}

XP_STATUS xpsL3RouteNextHopGroupRemoveDevice(xpsDevice_t devId)
{
    return XP_NO_ERR;
}

XP_STATUS xpsL3GetNexthopGroupCount(xpsDevice_t devId, uint32_t *count)
{
    GT_U32 nhTableDepth = 0;
    GT_STATUS cpssStatus = GT_OK;

    // Get NH table depth
    cpssStatus = cpssDxChCfgTableNumEntriesGet(devId,
                                               CPSS_DXCH_CFG_TABLE_ROUTER_NEXT_HOP_E, &nhTableDepth);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get NH table depth for devId %d. Status %d\n", devId, cpssStatus);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }

    // Calculate NH group count
    *count = (nhTableDepth/nhGrpMaxSize);

    return XP_NO_ERR;
}

XP_STATUS xpsL3RandomEnableRewrite(xpsDevice_t devId, bool enable)
{
    XP_STATUS status = XP_NO_ERR;
    GT_STATUS cpssStatus = GT_OK;
    uint32_t *nhgrpId = NULL;
    uint32_t count;

    status = xpsCountNextHopGroupObjects(&count);
    if (status != XP_NO_ERR)
    {
        return status;
    }

    nhgrpId = (uint32_t *)xpMalloc(sizeof(uint32_t) * (count));
    if (nhgrpId == NULL)
    {
        return XP_ERR_MEM_ALLOC_ERROR;
    }

    status = xpsGetNhGrpIdList(XP_SCOPE_DEFAULT, nhgrpId);
    if (status != XP_NO_ERR)
    {
        xpFree(nhgrpId);
        return status;
    }

    for (uint32_t i = 0; i < count; i++)
    {
        cpssStatus = cpssHalIpEcmpEntryRandomEnableSet(devId, nhgrpId[i],
                                                       (enable ? GT_TRUE : GT_FALSE));
        if (cpssStatus != GT_OK)
        {
            xpFree(nhgrpId);
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set IpEcmpEntry for devId %d nhgrpId:%d . Status %d\n", devId,
                  nhgrpId[i], cpssStatus);
            return xpsConvertCpssStatusToXPStatus(cpssStatus);
        }
    }
    xpFree(nhgrpId);
    return XP_NO_ERR;
}

XP_STATUS xpsL3SetRandomHashSeed(xpsDevice_t devId, uint32_t hashSeed)
{
    GT_STATUS cpssStatus = GT_OK;

    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
          "xpsL3SetRandomHashSeed setting hashseed to (%d) for DevId (%d)", hashSeed,
          devId);

    cpssStatus =  cpssHalSetRandomHashSeedParameters(devId,
                                                     CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L3_ECMP_E, hashSeed);
    if (cpssStatus != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set the pseudo random seed for ECMP DevId (%d)", devId);
        return xpsConvertCpssStatusToXPStatus(cpssStatus);
    }

    return XP_NO_ERR;
}

#ifdef __cplusplus
}
#endif

