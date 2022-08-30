// xpsL2Multicast.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpsEnums.h"
#include "xpsUtil.h"
#include "xpsState.h"
#include "xpsInternal.h"
#include "xpsMulticast.h"
#include "xpsInterface.h"
#include "xpsMirror.h"
#include "xpsInit.h"
#include "xpsVlan.h"
#include "xpsL3.h"
#include "xpsPort.h"
#include "xpsScope.h"
#include "xpsLock.h"
#include "xpsAllocator.h"
#include "xpsLag.h"
#include "cpssHalMulticast.h"
#include "cpssHalUtil.h"
#include "cpssHalDevice.h"


#ifdef __cplusplus
extern "C" {
#endif


#define XPS_MAX_L2MC_MEMBERS_PRT_GROUP 16
static xpsDbHandle_t xpsMcL2IntfListDbHndl = XPS_STATE_INVALID_DB_HANDLE;
static xpsDbHandle_t lagL2McGrpListDbHndl = XPS_STATE_INVALID_DB_HANDLE;












#define XPS_MC_L3_DOMAIN_ID_INCR_REF_CNT(ipv6McL3DomainId, devId) ipv6McL3DomaindId.refCnt[devId]++;
#define XPS_MC_L3_DOMAIN_ID_DECR_REF_CNT(ipv6McL3DomainId, devId) ipv6McL3DomaindId.refCnt[devId]--;
#define XPS_MC_L3_DOMAIN_ID_GET_REF_CNT(ipv6McL3DomainId, devId) ipv6McL3DomainId.refCnt[devId];

typedef struct xpsMcL2DomainIdDbEntry
{
    // Key
    uint32_t vlanId;

    // Data
    xpMcastDomainId_t  mcastDomainId;
    xpsDeviceMap deviceMap;

} xpsMcL2DomainIdDbEntry;

typedef struct xpsMcL2DomainIdtoVlanIdDbEntry
{
    // Key
    xpMcastDomainId_t mcastDomainId;

    // Data
    uint32_t  vlanId;

} xpsMcL2DomainIdtoVlanIdDbEntry;

#if 1
typedef struct xpsMcL3DomainIdDbEntry
{
    // Key
    uint32_t vrfId;

    // Data
    xpMcastDomainId_t  mcastDomainId;
    uint32_t refCnt[XP_MAX_DEVICES];
    xpsDeviceMap deviceMap;

} xpsMcL3DomainIdDbEntry;
#endif

#define XPS_L2MC_DEFAULT_L2MC_IN_LAG 16

typedef struct xpsLagL2McListDbEntry
{
    //Key
    xpsInterfaceId_t lagIntfId; //lagId or lag's bridgePortId

    //Data
    xpsMcL2InterfaceListId_t l2IntfId;
    uint16_t numOfL2McGrps;
    uint32_t l2McGrpId[XPS_L2MC_DEFAULT_L2MC_IN_LAG];
} xpsLagL2McListDbEntry;

typedef struct xpsMcL2EncapDbEntry
{
    // Key
    uint32_t encapMdtIndex;

    // Data
    uint32_t multicastVifIdx;
    xpsDeviceMap deviceMap;
    uint32_t          encapData;
    xpsL2EncapType_e  encapType;
    // Next and Prev encap nodes of this encap node
    struct xpsMcL2EncapDbEntry *prevEncapDbEntry;
    struct xpsMcL2EncapDbEntry *nextEncapDbEntry;
} xpsMcL2EncapDbEntry;

typedef struct xpsMcL2TagTypeDbEntry
{
    // Key
    xpsMcL2InterfaceListId_t   ifListId;
    xpsL2EncapType_e           tagType;

    // Data
    xpsMcL2EncapDbEntry *encapDbEntry;
} xpsMcL2TagTypeDbEntry;

typedef struct xpsMcL2TunnelVifDbEntry
{
    // Key
    xpsMcL2InterfaceListId_t   ifListId;
    xpsInterfaceId_t           intfId;

    // Data
    xpsMcL2EncapDbEntry *encapDbEntry;
} xpsMcL2TunnelVifDbEntry;

typedef struct xpsMcL2IntfListDbEntry
{
    // xps L2MC Group-id Key
    xpsMcL2InterfaceListId_t ifListId;

    // Data
    uint32_t             vlanId;
    uint32_t             xpsVidx;

    /* RouteEntryBaseIndex (NH table)*/
    uint32_t xpsRouteEntryIndex;

    /* NOTE: For IGMP First node of the pair is used.
       For other usecases, there can be more than one pair used
       for this MC-grp. In future, for such scenarios, change this list.
    */
    uint32_t             xpsMLLPairIdx;
    uint16_t numPortIntfs;
    uint16_t numLagIntfs;
    uint16_t numOfIntfs;
    xpsInterfaceId_t intfList[XPS_MAX_L2MC_MEMBERS_PRT_GROUP];
#if 0
    xpsDeviceMap         deviceMap;
    // First and Last encap nodes for this L2 interface list
    xpsMcL2EncapDbEntry *firstEncapDbEntry;
    xpsMcL2EncapDbEntry *lastEncapDbEntry;
#endif
} xpsMcL2IntfListDbEntry;

typedef struct xpsMcL3EncapDbEntry
{
    // Key
    uint32_t encapMdtIndex;

    // Data
    uint32_t bdId;
    uint32_t multicastVifIdx;
    uint32_t          encapData;
    xpsL3EncapType_e  encapType;
    // Next and Prev Encap entries
    struct xpsMcL3EncapDbEntry *nextEncapDbEntry;
    struct xpsMcL3EncapDbEntry *prevEncapDbEntry;
} xpsMcL3EncapDbEntry;

typedef struct xpsMcL3L3IntfDbEntry
{
    // Key
    xpsMcL3InterfaceListId_t   ifListId;
    uint32_t                   bdId;

    // Data
    xpsDeviceMap               deviceMap;
    // First and Last encap MDT Nodes for this L3 interface
    xpsMcL3EncapDbEntry        *firstEncapDbEntry;
    xpsMcL3EncapDbEntry        *lastEncapDbEntry;

    // Next and Prev L3 interfaces in the same L3 interfaces list
    struct xpsMcL3L3IntfDbEntry *prevl3Interface;
    struct xpsMcL3L3IntfDbEntry *nextl3Interface;

    // Next and Prev L3 interfaces derived from the same L2 interface
    struct xpsMcL3L3IntfDbEntry *nextByL2Interface;
    struct xpsMcL3L3IntfDbEntry *prevByL2Interface;
} xpsMcL3L3IntfDbEntry;

typedef xpsMcL3L3IntfDbEntry xpsMcL3Interface;

typedef struct xpsMcL3IntfListDbEntry
{
    // Key
    xpsMcL3InterfaceListId_t ifListId;

    // Data
    xpsDeviceMap deviceMap;
    // First and Last L3 interfaces on this L3 interface list
    xpsMcL3Interface *firstl3Interface;
    xpsMcL3Interface *lastl3Interface;
} xpsMcL3IntfListDbEntry;









static int32_t xpsMcL2IntfListDbEntryComp(void* arg1, void* arg2)
{
    xpsMcL2IntfListDbEntry *elem1 = (xpsMcL2IntfListDbEntry*)arg1;
    xpsMcL2IntfListDbEntry *elem2 = (xpsMcL2IntfListDbEntry*)arg2;
    // Compare Interface Lists.
    uint32_t result = elem1->ifListId - elem2->ifListId;
    return result;
}

static int32_t lagL2McListKeyComp(void* key1, void* key2)
{
    return ((((xpsLagL2McListDbEntry *) key1)->lagIntfId) -
            (((xpsLagL2McListDbEntry *) key2)->lagIntfId));
}




















#if 1



#endif






static XP_STATUS xpsMcL2IntfListDbEntryInsert(xpsScope_t scopeId,
                                              xpsMcL2InterfaceListId_t ifListId, xpsMcL2IntfListDbEntry **intfListDbEntry)
{
    XP_STATUS result;

    // Allocate memory for the L2 interface list database state.
    if ((result = xpsStateHeapMalloc(sizeof(xpsMcL2IntfListDbEntry),
                                     (void**)intfListDbEntry)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Allocate memory for L2 interface list failed");
        return result;
    }

    // Initialize.
    memset((*intfListDbEntry), 0x00, sizeof(xpsMcL2IntfListDbEntry));
    (*intfListDbEntry)->ifListId = ifListId;

    // Insert the L2 interface List state to the database.
    if ((result = xpsStateInsertData(scopeId, xpsMcL2IntfListDbHndl,
                                     (void*)(*intfListDbEntry))) != XP_NO_ERR)
    {
        // Free the memory if insertion failed.
        xpsStateHeapFree((void*)(*intfListDbEntry));
        (*intfListDbEntry) = NULL;
        return result;
    }

    return result;
}



static XP_STATUS xpsMcL2IntfListDbEntryDelete(xpsScope_t scopeId,
                                              xpsMcL2IntfListDbEntry **intfListDbEntry)
{
    XP_STATUS retVal;

    // Delete the interface list entry in the database.
    retVal = xpsStateDeleteData(scopeId, xpsMcL2IntfListDbHndl, (*intfListDbEntry),
                                (void**)intfListDbEntry);
    if (retVal != XP_NO_ERR)
    {
        return retVal;
    }

    // Free the memory for the database state.
    retVal = xpsStateHeapFree((void*)(*intfListDbEntry));
    if (retVal != XP_NO_ERR)
    {
        return retVal;
    }

    // Reset the pointer.
    (*intfListDbEntry) = NULL;

    return retVal;
}


































#if 1

#endif
XP_STATUS xpsMcGetCreateMcL2DomainId(xpDevice_t devId, uint32_t vlanId,
                                     xpMcastDomainId_t *mcL2DomainId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMcReleaseDestroyL2DomainId(xpDevice_t devId, uint32_t vlanId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastAllocateIpv6L3DomainId(xpsMcastDomainId_t *mcL3DomainId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastAllocateIpv6L3DomainIdScope(xpsScope_t scopeId,
                                                  xpsMcastDomainId_t *mcL3DomainId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastReleaseIpv6L3DomainId(xpsMcastDomainId_t mcL3DomainId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastReleaseIpv6L3DomainIdScope(xpsScope_t scopeId,
                                                 xpsMcastDomainId_t mcL3DomainId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

#if 1
XP_STATUS xpsMcGetCreateMcL3DomainId(xpDevice_t devId, uint32_t vrfId,
                                     xpMcastDomainId_t *mcL3DomainId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMcReleaseDestroyL3DomainId(xpDevice_t devId, uint32_t vrfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMcDestroyL3DomainId(uint32_t vrfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMcDestroyL3DomainIdScope(xpsScope_t scopeId, uint32_t vrfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

#endif






















static XP_STATUS xpsMcL2GetIntfListDbEntry(xpsScope_t scopeId,
                                           xpsMcL2InterfaceListId_t ifListId, xpsMcL2IntfListDbEntry **intfListDbEntry)
{
    XP_STATUS retVal;
    xpsInterfaceType_e ifType;
    xpsMcL2IntfListDbEntry intfListDbKey;

    // Find the interface type for the L2 interface to be added.
    retVal = xpsInterfaceGetTypeScope(scopeId, ifListId, &ifType);
    if (retVal != XP_NO_ERR)
    {
        return retVal;
    }

    // Make sure its a valid L2 Interface.
    if (ifType != XPS_L2_MCAST_GROUP)
    {
        return XP_ERR_INVALID_ARG;
    }

    // Find the Interface List state in the Database
    intfListDbKey.ifListId = ifListId;

    retVal = xpsStateSearchData(scopeId, xpsMcL2IntfListDbHndl, &intfListDbKey,
                                (void**)intfListDbEntry);
    if (retVal != XP_NO_ERR)
    {
        return retVal;
    }

    if (*intfListDbEntry == NULL)
    {
        return XP_ERR_KEY_NOT_FOUND;
    }

    return retVal;
}

/* Following APIs for LAG*/
static XP_STATUS xpsL2McGetLagL2McListDb(xpsScope_t scopeId,
                                         xpsInterfaceId_t lagIntfId,
                                         xpsLagL2McListDbEntry ** lagL2McListEntryPtr)
{
    XP_STATUS result = XP_NO_ERR;
    xpsLagL2McListDbEntry keyLagL2McListEntry;

    memset(&keyLagL2McListEntry, 0x0, sizeof(xpsLagL2McListDbEntry));
    keyLagL2McListEntry.lagIntfId = lagIntfId;
    if ((result = xpsStateSearchData(scopeId, lagL2McGrpListDbHndl,
                                     (xpsDbKey_t)&keyLagL2McListEntry, (void**)lagL2McListEntryPtr)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Search data failed");
        return result;
    }

    return result;
}

static XP_STATUS xpsL2McRemoveL2McInLagDb(xpsScope_t scopeId,
                                          xpsInterfaceId_t lagIntfId)
{

    XP_STATUS result = XP_NO_ERR;
    xpsLagL2McListDbEntry *lagL2McListEntry = NULL;
    xpsLagL2McListDbEntry keyLagL2McListEntry;

    memset(&keyLagL2McListEntry, 0x0, sizeof(xpsLagL2McListDbEntry));
    keyLagL2McListEntry.lagIntfId = lagIntfId;

    if ((result = xpsStateDeleteData(scopeId, lagL2McGrpListDbHndl,
                                     (xpsDbKey_t)&keyLagL2McListEntry, (void**)&lagL2McListEntry)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Delete data for lag interface(%d) failed", lagIntfId);
        return result;
    }

    if (lagL2McListEntry == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Delete data for lag interface(%d) failed", lagIntfId);
        return XP_ERR_NOT_FOUND;
    }

    if ((result = xpsStateHeapFree((void*)lagL2McListEntry)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "XPS state Heap Free failed");
        return result;
    }

    return result;
}

static XP_STATUS xpsL2McRemoveL2McFromLagDb(xpsScope_t scopeId,
                                            xpsInterfaceId_t lagIntfId,
                                            xpsMcL2InterfaceListId_t l2IntfListId)
{
    XP_STATUS result = XP_NO_ERR;
    xpsLagL2McListDbEntry *lagL2McListEntryNew = NULL, *lagL2McListEntry = NULL;
    uint16_t j = 0, numOfL2McGrps, l2mcPresent = 0;

    result = xpsL2McGetLagL2McListDb(scopeId, lagIntfId, &lagL2McListEntry);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get Lag L2MC List for lag interface(%d) failed", lagIntfId);
        return result;
    }

    if (lagL2McListEntry == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid arguments");
        return XP_ERR_INVALID_ARG;
    }

    numOfL2McGrps = lagL2McListEntry->numOfL2McGrps;

    //If no l2McGrpId or only one vlan and invalid input - return error
    if ((!numOfL2McGrps) || ((numOfL2McGrps == 1) &&
                             (lagL2McListEntry->l2McGrpId[0] != l2IntfListId)))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid vlan id");
        return XP_ERR_INVALID_ARG;
    }

    if ((numOfL2McGrps == 1) && (lagL2McListEntry->l2McGrpId[0] == l2IntfListId))
    {
        //Only l2mc in the lag matches the l2IntfListId to be removed then delete the context
        result = xpsL2McRemoveL2McInLagDb(scopeId, lagIntfId);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Remove the Lag from vlan list,"
                  " lag interface(%d)", lagIntfId);
            return result;
        }

        //Lag-L2Mc Db deleted. Return
        return XP_NO_ERR;
    }

    //Delete the l2mc grp from the context
    lagL2McListEntry->numOfL2McGrps = numOfL2McGrps - 1;
    for (j = 0; j < numOfL2McGrps; j++)
    {
        if (!l2mcPresent)
        {
            if (lagL2McListEntry->l2McGrpId[j] == l2IntfListId)
            {
                l2mcPresent = 1;
            }
            continue;
        }
        else if (j !=0)
        {
            lagL2McListEntry->l2McGrpId[j-1] = lagL2McListEntry->l2McGrpId[j];
        }
    }

    if (!l2mcPresent)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid arguments");
        return XP_ERR_INVALID_ARG;
    }

    if (xpsDAIsCtxShrinkNeeded(numOfL2McGrps, XPS_MAX_L2MC_MEMBERS_PRT_GROUP))
    {
        result = xpsDynamicArrayShrink((void**)&lagL2McListEntryNew,
                                       lagL2McListEntry, sizeof(xpsLagL2McListDbEntry),
                                       sizeof(uint32_t), numOfL2McGrps, XPS_MAX_L2MC_MEMBERS_PRT_GROUP);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Dynamic array shrink failed");
            return result;
        }

        result = xpsL2McRemoveL2McInLagDb(scopeId, lagIntfId);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Remove the Lag from l2mc list,"
                  " lag interface(%d)", lagIntfId);
            xpsStateHeapFree((void*)lagL2McListEntryNew);
            lagL2McListEntryNew = NULL;
            return result;
        }

        if ((result = xpsStateInsertData(scopeId, lagL2McGrpListDbHndl,
                                         (void*)lagL2McListEntryNew)) != XP_NO_ERR)
        {
            xpsStateHeapFree((void*)lagL2McListEntryNew);
            lagL2McListEntryNew = NULL;
            return result;
        }
    }

    return result;
}

static XP_STATUS xpsL2McAddL2McInLagDb(xpsScope_t scopeId,
                                       xpsInterfaceId_t lagIntfId,
                                       xpsMcL2InterfaceListId_t l2IntfListId)
{

    XP_STATUS result = XP_NO_ERR;
    xpsLagL2McListDbEntry *lagL2McListEntryNew = NULL, * lagL2McListEntry = NULL;
    uint16_t numOfL2McGrps = 0;

    result = xpsL2McGetLagL2McListDb(scopeId, lagIntfId, &lagL2McListEntry);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get Lag L2Mc List for lag interface(%d) failed", lagIntfId);
        return result;
    }

    //If first l2mc grp for lag, then need to create the db entry.
    if (NULL == lagL2McListEntry)
    {
        if ((result = xpsStateHeapMalloc(sizeof(xpsLagL2McListDbEntry),
                                         (void**)&lagL2McListEntryNew)) != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "XPS state Heap Allocation failed");
            return result;
        }
        if (lagL2McListEntryNew == NULL)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Recieved null pointer");
            return XP_ERR_NULL_POINTER;
        }

        memset(lagL2McListEntryNew, 0, sizeof(xpsLagL2McListDbEntry));

        //First L2MC addition in lag.
        lagL2McListEntryNew->lagIntfId = lagIntfId;
        lagL2McListEntryNew->numOfL2McGrps = 1;
        lagL2McListEntryNew->l2McGrpId[0] = l2IntfListId;

        if ((result = xpsStateInsertData(scopeId, lagL2McGrpListDbHndl,
                                         (void*)lagL2McListEntryNew)) != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Inserting data in state failed");
            xpsStateHeapFree((void*)lagL2McListEntryNew);
            return result;
        }

        return XP_NO_ERR;
    }

    numOfL2McGrps = lagL2McListEntry->numOfL2McGrps;

    //Create a new context with double dynamic array size. Also delete old context
    if (xpsDAIsCtxGrowthNeeded(numOfL2McGrps, XPS_L2MC_DEFAULT_L2MC_IN_LAG))
    {
        result = xpsDynamicArrayGrow((void**)&lagL2McListEntryNew, lagL2McListEntry,
                                     sizeof(xpsLagL2McListDbEntry),
                                     sizeof(uint32_t), numOfL2McGrps, XPS_L2MC_DEFAULT_L2MC_IN_LAG);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Dynamic array grow failed");
            return result;
        }

        //Delete the old context
        result = xpsL2McRemoveL2McInLagDb(scopeId, lagIntfId);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Delete lag from vlan list failed, lag interface(%d)", lagIntfId);
            xpsStateHeapFree((void*)lagL2McListEntryNew);
            return result;
        }
        if ((result = xpsStateInsertData(scopeId, lagL2McGrpListDbHndl,
                                         (void*)lagL2McListEntryNew)) != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Inserting data in state failed");
            xpsStateHeapFree((void*)lagL2McListEntryNew);
            return result;
        }
        lagL2McListEntry = lagL2McListEntryNew;
    }

    lagL2McListEntry->numOfL2McGrps += 1;
    lagL2McListEntry->l2McGrpId[numOfL2McGrps] = l2IntfListId;

    return result;
}

static XP_STATUS xpsMcL2AddDelPortToVidx(xpsDevice_t devId,
                                         xpsMcL2IntfListDbEntry *l2IntfListDbEntry,
                                         xpsInterfaceId_t intfId, bool isDel)
{
    XP_STATUS retVal = XP_NO_ERR;
    GT_STATUS rc = GT_OK;
    xpsInterfaceType_e intfType;
    xpsInterfaceInfo_t *intfInfo = NULL;
    xpsLagPortIntfList_t lagPortList;
    GT_U8 cpssDevId;
    GT_U32 cpssPortNum, i;
    xpsDevice_t    portDevId = 0;
    xpPort_t      portId = 0;
    xpsInterfaceId_t bindPortIntf;
    xpsScope_t scopeId;
    memset(&lagPortList, 0, sizeof(lagPortList));
    if ((retVal = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return retVal;
    }

    if (!l2IntfListDbEntry)
    {
        return XP_ERR_NULL_POINTER;
    }

    retVal = xpsInterfaceGetInfoScope(scopeId, intfId, &intfInfo);

    if ((retVal != XP_NO_ERR) || (intfInfo == NULL))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get interface information for Interface id : %d\n", intfId);
        return retVal;
    }

    intfType = intfInfo->type;
    if (intfType != XPS_PORT && intfType != XPS_LAG)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Interface %d is not a port interface \n", intfId);
        return XP_ERR_INVALID_PARAMS;
    }


    if (intfType == XPS_PORT)
    {
        lagPortList.portIntf[0] = intfId;
        lagPortList.size = 1;
    }
    else if (intfType == XPS_LAG)
    {
        if ((retVal = xpsLagGetPortIntfListScope(scopeId, intfId,
                                                 &lagPortList)) != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Get ports failed, lag interface(%d)", intfId);
            return retVal;
        }
    }

    for (i = 0; i < lagPortList.size; i++)
    {
        bindPortIntf = lagPortList.portIntf[i];

        // Get device and local port id for the device the port belongs to
        retVal = xpsPortGetDevAndPortNumFromIntfScope(scopeId, bindPortIntf, &portDevId,
                                                      &portId);
        if (retVal != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to get port and dev Id from port intf : %d \n", bindPortIntf);
            return retVal;
        }

        cpssDevId = xpsGlobalIdToDevId(portDevId, portId);
        cpssPortNum = xpsGlobalPortToPortnum(portDevId, portId);

        if (isDel == false)
        {
            rc = cpssHalBrgMcMemberAdd(cpssDevId, l2IntfListDbEntry->xpsVidx, cpssPortNum);
            if (rc != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to add port : %d \n", intfId);
                return xpsConvertCpssStatusToXPStatus(rc);
            }
        }
        else
        {
            rc = cpssHalBrgMcMemberDelete(cpssDevId, l2IntfListDbEntry->xpsVidx,
                                          cpssPortNum);
            if (rc != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to remove port : %d \n", intfId);
                return xpsConvertCpssStatusToXPStatus(rc);
            }
        }
    }

    return retVal;
}

static XP_STATUS xpsMcL2AddPortOrLAGToList(xpsDevice_t devId,
                                           xpsMcL2InterfaceListId_t l2IntfListId,
                                           xpsMcL2IntfListDbEntry **intfListDbNewEntry,
                                           xpsInterfaceId_t intfId,
                                           xpsL2EncapType_e tagType, uint32_t tagData)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpsScope_t scopeId;
    uint16_t numOfIntfs;
    xpsInterfaceInfo_t *intfInfo = NULL;
    xpsMcL2IntfListDbEntry *l2IntfListDbEntry = NULL;
    xpsInterfaceType_e intfType;

    /* Get Scope Id from devId */
    if ((retVal = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return retVal;
    }

    retVal = xpsInterfaceGetInfoScope(scopeId, intfId, &intfInfo);

    if ((retVal != XP_NO_ERR) || (intfInfo == NULL))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get interface information for Interface id : %d\n", intfId);
        return retVal;
    }

    intfType = intfInfo->type;
    if (intfType != XPS_PORT && intfType != XPS_LAG)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Interface %d is not a port interface \n", intfId);
        return XP_ERR_INVALID_PARAMS;
    }


    // Get the L2 IntfListDbEntry from the L2 interface list id.
    retVal = xpsMcL2GetIntfListDbEntry(scopeId, l2IntfListId, &l2IntfListDbEntry);
    if (retVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsMcL2GetIntfListDbEntry Failed l2IntfListId %d ", l2IntfListId);
        return retVal;
    }

    retVal = xpsMcL2AddDelPortToVidx(devId, l2IntfListDbEntry, intfId, false);
    if (retVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsMcL2AddDelPortToVidx Failed l2IntfListId %d intfid %d ", l2IntfListId,
              intfId);
        return retVal;
    }

    if (intfType == XPS_LAG)
    {
        retVal = xpsL2McAddL2McInLagDb(scopeId, intfId, l2IntfListId);
        if (retVal != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Add L2Mc in lag db failed"
                  "L2Mc(%d) interface(%d)", l2IntfListId, intfId);
            return retVal;
        }

    }

    numOfIntfs = l2IntfListDbEntry->numOfIntfs;

    //Number of interfaces increase exponentially starting from XPS_MAX_L2MC_MEMBERS_PRT_GROUP by default
    //Everytime the number reaches the current size, the size is doubled
    if (xpsDAIsCtxGrowthNeeded(numOfIntfs, XPS_MAX_L2MC_MEMBERS_PRT_GROUP))
    {
        retVal = xpsDynamicArrayGrow((void **)intfListDbNewEntry,
                                     (void *)l2IntfListDbEntry,
                                     sizeof(xpsMcL2IntfListDbEntry),
                                     sizeof(xpsInterfaceId_t), numOfIntfs, XPS_MAX_L2MC_MEMBERS_PRT_GROUP);
        if (retVal != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Dynamic array grow failed");
            return retVal;
        }

        //Delete the old l2IntfListDbEntry
        retVal = xpsMcL2IntfListDbEntryDelete(scopeId, &l2IntfListDbEntry);
        if (retVal != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Delete the L2MC context failed,"
                  "L2MC Id(%d)", l2IntfListId);
            xpsStateHeapFree((void *)(*intfListDbNewEntry));
            return retVal;
        }

        //Insert the new context
        if ((retVal = xpsStateInsertData(scopeId, xpsMcL2IntfListDbHndl,
                                         (void*)(*intfListDbNewEntry))) != XP_NO_ERR)
        {
            xpsStateHeapFree((void*)(*intfListDbNewEntry));
            (*intfListDbNewEntry) = NULL;
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Inserting data in state failed");
            return retVal;
        }
        l2IntfListDbEntry = *intfListDbNewEntry;
    }

    l2IntfListDbEntry->intfList[numOfIntfs] = intfId;
    l2IntfListDbEntry->numOfIntfs += 1;
    if (intfType == XPS_PORT)
    {
        l2IntfListDbEntry->numPortIntfs++;
    }
    else if (intfType == XPS_LAG)
    {
        l2IntfListDbEntry->numLagIntfs++;
    }

    return retVal;
}






static XP_STATUS xpsMcL2RemovePortOrLAGFromList(xpsDevice_t devId,
                                                xpsMcL2InterfaceListId_t l2IntfListId,
                                                xpsMcL2IntfListDbEntry **intfListDbNewEntry,
                                                xpsInterfaceId_t intfId)
{
    XP_STATUS retVal;
    xpsScope_t scopeId;
    xpsInterfaceType_e intfType;

    uint16_t numOfIntfs;
    uint16_t j;
    uint32_t intfPresent = 0;
    xpsMcL2IntfListDbEntry *l2IntfListDbEntry = NULL;

    /* Get Scope Id from devId */
    if ((retVal = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return retVal;
    }

    // Find the interface type for the interface to be removed.
    if ((retVal = xpsInterfaceGetTypeScope(scopeId, intfId,
                                           &intfType)) != XP_NO_ERR)
    {
        return retVal;
    }
    // Get the L2 IntfListDbEntry from the L2 interface list id.
    retVal = xpsMcL2GetIntfListDbEntry(scopeId, l2IntfListId, &l2IntfListDbEntry);
    if (retVal != XP_NO_ERR)
    {
        return retVal;
    }

    retVal = xpsMcL2AddDelPortToVidx(devId, l2IntfListDbEntry, intfId, true);
    if (retVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsMcL2AddDelPortToVidx Failed l2IntfListId %d intfid %d ", l2IntfListId,
              intfId);
        return retVal;
    }

    numOfIntfs = l2IntfListDbEntry->numOfIntfs;
    if (!numOfIntfs)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "L2Mc does not have any interface");
        return XP_ERR_INVALID_ARG;
    }


    if (intfType == XPS_PORT)
    {
        l2IntfListDbEntry->numPortIntfs--;
    }
    else if (intfType == XPS_LAG)
    {
        retVal = xpsL2McRemoveL2McFromLagDb(scopeId, intfId, l2IntfListId);
        if (retVal != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Remove L2Mc from lag failed, l2IntfListId(%d) interface(%d)", l2IntfListId,
                  intfId);
            return retVal;
        }

        l2IntfListDbEntry->numLagIntfs--;
    }

    //Delete the element from the context
    l2IntfListDbEntry->numOfIntfs = numOfIntfs - 1;
    for (j = 0; j < numOfIntfs; j++)
    {
        if (!intfPresent)
        {
            if (l2IntfListDbEntry->intfList[j] == intfId)
            {
                intfPresent = 1;
            }
            continue;
        }
        else if (j != 0)
        {
            l2IntfListDbEntry->intfList[j-1] = l2IntfListDbEntry->intfList[j];
        }
    }

    if (!intfPresent)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid arguments");
        return XP_ERR_INVALID_ARG;
    }

    if (xpsDAIsCtxShrinkNeeded(numOfIntfs, XPS_MAX_L2MC_MEMBERS_PRT_GROUP))
    {
        retVal = xpsDynamicArrayShrink((void **)intfListDbNewEntry,
                                       (void *)l2IntfListDbEntry,
                                       sizeof(xpsMcL2IntfListDbEntry),
                                       sizeof(xpsInterfaceId_t), numOfIntfs, XPS_MAX_L2MC_MEMBERS_PRT_GROUP);
        if (retVal != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Dynamic array shrink failed");
            return retVal;
        }

        //Delete the old l2IntfListDbEntry
        retVal = xpsMcL2IntfListDbEntryDelete(scopeId, &l2IntfListDbEntry);
        if (retVal != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Delete the l2mc id context failed,"
                  "L2MC Id(%d)", l2IntfListId);
            xpsStateHeapFree((void*)(*intfListDbNewEntry));
            *intfListDbNewEntry = NULL;
            return retVal;
        }

        if ((retVal = xpsStateInsertData(scopeId, xpsMcL2IntfListDbHndl,
                                         (void*)*intfListDbNewEntry)) != XP_NO_ERR)
        {
            xpsStateHeapFree((void*)(*intfListDbNewEntry));
            *intfListDbNewEntry = NULL;
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Inserting data in state failed");
            return retVal;
        }
        l2IntfListDbEntry = *intfListDbNewEntry;
    }

    return retVal;
}




















































/* Public APIs Begin Here*/

XP_STATUS xpsMulticastGetVifFromL2InterfaceList(xpsMcL2InterfaceListId_t
                                                ifListId, xpL2EncapType_e encapType, xpVif_t *mcVif)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastGetVifFromL2InterfaceListScope(xpsScope_t scopeId,
                                                     xpsMcL2InterfaceListId_t ifListId, xpL2EncapType_e encapType, xpVif_t *mcVif)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastGetVifFromL3InterfaceList(xpsMcL3InterfaceListId_t
                                                ifListId, xpsInterfaceId_t l3IntfId, xpL3EncapType_e encapType, xpVif_t *mcVif)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastGetVifFromL3InterfaceListScope(xpsScope_t scopeId,
                                                     xpsMcL3InterfaceListId_t ifListId, xpsInterfaceId_t l3IntfId,
                                                     xpL3EncapType_e encapType, xpVif_t *mcVif)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastRemoveInterfaceFromL2InterfaceList(xpsDevice_t devId,
                                                         xpsMcL2InterfaceListId_t l2IntfListId,
                                                         xpsInterfaceId_t intfId)
{
    XPS_FUNC_ENTRY_LOG();


    XP_STATUS retVal;
    xpsInterfaceType_e ifType;
    xpsMcL2IntfListDbEntry *l2IntfListDbEntry;
    xpsScope_t scopeId;

    // Make sure the device Id is valid
    if (IS_DEVICE_VALID(devId) == 0)
    {
        return XP_ERR_INVALID_ID;
    }

    /* Get Scope Id from devId */
    if ((retVal = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return retVal;
    }

    // Get the L2 IntfListDbEntry from the L2 interface list id.
    retVal = xpsMcL2GetIntfListDbEntry(scopeId, l2IntfListId, &l2IntfListDbEntry);
    if (retVal != XP_NO_ERR)
    {
        return retVal;
    }

    // If not found, can't proceed
    if (l2IntfListDbEntry == NULL)
    {
        retVal = XP_ERR_INVALID_DATA;
        return retVal;
    }

    /* Got a good interface List Id */

    // Find the interface type for the interface to be removed.
    retVal = xpsInterfaceGetTypeScope(scopeId, intfId, &ifType);
    if (retVal != XP_NO_ERR)
    {
        return retVal;
    }

    switch (ifType)
    {
        case XPS_PORT:
        case XPS_BRIDGE_PORT:
        case XPS_EXTENDED_PORT:
        case XPS_LAG:
            // Ports, bridgePorts and LAGs are programmed into the same encap node by tag type.
            // In case of bridgePort, associated physicalPort will be retrieved inside the below api
            retVal = xpsMcL2RemovePortOrLAGFromList(devId, l2IntfListId, &l2IntfListDbEntry,
                                                    intfId);
            break;
        case XPS_TUNNEL_VXLAN:
        case XPS_TUNNEL_NVGRE:
        case XPS_TUNNEL_PBB:
        case XPS_TUNNEL_GENEVE:
#if 0
            // Tunnels are programmed into their own encap nodes.
            retVal = xpsMcL2RemoveTunnelFromList(devId, l2IntfListDbEntry, intfId);
            break;
#endif
        default:
            retVal = XP_ERR_INVALID_ARG;
            return retVal;
    }

    XPS_FUNC_EXIT_LOG();

    return retVal;
}

XP_STATUS xpsMulticastRemoveInterfaceFromL3InterfaceList(xpsDevice_t devId,
                                                         xpsMcL3InterfaceListId_t l3IntfListId, xpsInterfaceId_t l3IntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastRemoveBridgingInterfaceFromL3InterfaceList(
    xpsDevice_t devId, xpsMcL3InterfaceListId_t l3IntfListId,
    xpsMcL2InterfaceListId_t l2IntfListId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastAddInterfaceToL2InterfaceList(xpsDevice_t devId,
                                                    xpsMcL2InterfaceListId_t l2IntfListId,
                                                    xpsInterfaceId_t intfId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS retVal;
    uint32_t encapData;
    xpsL2EncapType_e encapType;
    xpsInterfaceType_e ifType;
    xpsMcL2IntfListDbEntry *l2IntfListDbEntry;
    xpsScope_t scopeId;
    uint16_t vlanId;
    uint32_t bridgeIntfId;

    // Make sure the device Id is valid
    if (IS_DEVICE_VALID(devId) == 0)
    {
        return XP_ERR_INVALID_ID;
    }

    /* Get Scope Id from devId */
    if ((retVal = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        return retVal;
    }

    // Get the L2 IntfListDbEntry from the L2 interface list id.
    retVal = xpsMcL2GetIntfListDbEntry(scopeId, l2IntfListId, &l2IntfListDbEntry);
    if (retVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsMcL2GetIntfListDbEntry Failed, l2IntfListId(%d)", l2IntfListId);
        return retVal;
    }

    // If not found, can't proceed
    if (l2IntfListDbEntry == NULL)
    {
        retVal = XP_ERR_INVALID_DATA;
        return retVal;
    }

    /* Got a good interface List Id */

    // Find the interface type for the interface to be added.
    retVal = xpsInterfaceGetTypeScope(scopeId, intfId, &ifType);
    if (retVal != XP_NO_ERR)
    {
        return retVal;
    }

    // Accept only L2 interface types
    switch (ifType)
    {
        case XPS_PORT:
        case XPS_EXTENDED_PORT:
        case XPS_LAG:
        case XPS_TUNNEL_MPLS:
        case XPS_TUNNEL_VXLAN:
        case XPS_TUNNEL_NVGRE:
        case XPS_TUNNEL_GENEVE:
        case XPS_TUNNEL_PBB:
            // Find the tag type for this interface in this vlan from the VLAN mgr.
            retVal = xpsVlanGetEndpointInfo(scopeId, l2IntfListDbEntry->vlanId, intfId,
                                            &encapType, &encapData);
            if (retVal != XP_NO_ERR)
            {
                return retVal;
            }
            break;
        case XPS_BRIDGE_PORT:
            // Find the tag type for this interface in this vlan from the VLAN mgr.
            bridgeIntfId = intfId;
            vlanId = XPS_INTF_MAP_BRIDGE_PORT_TO_VLAN(bridgeIntfId);
            intfId = XPS_INTF_MAP_BRIDGE_PORT_TO_INTF(bridgeIntfId);

            retVal = xpsVlanGetEndpointInfo(scopeId, vlanId, intfId, &encapType,
                                            &encapData);
            if (retVal != XP_NO_ERR)
            {
                return retVal;
            }

            if (l2IntfListDbEntry->vlanId != encapData)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "BridgeId doesnt match, received->bdId:%d, intfVlan->bdId:%d",
                      l2IntfListDbEntry->vlanId, encapData);
                retVal = XP_ERR_INVALID_ARG;
                return retVal;
            }
            encapData = vlanId;
            break;
        default:
            retVal = XP_ERR_INVALID_ARG;
            return retVal;
    }


    // Switch encapType
    switch (encapType)
    {
        case XP_L2_ENCAP_DOT1Q_UNTAGGED:
        case XP_L2_ENCAP_DOT1Q_TAGGED:
        case XP_L2_ENCAP_QINQ_UNTAGGED:
        case XP_L2_ENCAP_QINQ_CTAGGED:
        case XP_L2_ENCAP_QINQ_STAGGED:
            // For these encaps, multiple endpoints are programmed into the same encap node by tag type.
            retVal = xpsMcL2AddPortOrLAGToList(devId, l2IntfListId, &l2IntfListDbEntry,
                                               intfId, encapType,
                                               encapData);
            break;
        case XP_L2_ENCAP_VXLAN:
        case XP_L2_ENCAP_NVGRE:
        case XP_L2_ENCAP_PBB:
        case XP_L2_ENCAP_GENEVE:
        case XP_L2_ENCAP_MPLS:
#if 0
            // For these encaps, each endpoint gets its own encap type.
            retVal = xpsMcL2AddTunnelToList(devId, l2IntfListDbEntry, intfId, encapType,
                                            encapData);
            break;
#endif
        default:
            retVal = XP_ERR_INVALID_ARG;
            return retVal;
    }

    XPS_FUNC_EXIT_LOG();

    return retVal;
}

XP_STATUS xpsMulticastAddInterfaceToL3InterfaceList(xpsDevice_t devId,
                                                    xpsMcL3InterfaceListId_t l3IntfListId, xpsInterfaceId_t l3IntfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastAddBridgingInterfaceToL3InterfaceList(xpsDevice_t devId,
                                                            xpsMcL3InterfaceListId_t l3IntfListId, xpsMcL2InterfaceListId_t l2IntfListId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastRemoveL2InterfaceListFromDevice(xpsDevice_t devId,
                                                      xpsMcL2InterfaceListId_t l2IntfListId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastRemoveL3InterfaceListFromDevice(xpsDevice_t devId,
                                                      xpsMcL3InterfaceListId_t l3IntfListId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastAddL2InterfaceListToDevice(xpsDevice_t devId,
                                                 xpsMcL2InterfaceListId_t l2IntfListId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastAddL3InterfaceListToDevice(xpsDevice_t devId,
                                                 xpsMcL3InterfaceListId_t ifListId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastDestroyL2InterfaceList(xpsMcL2InterfaceListId_t
                                             l2IntfListId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastDestroyL2InterfaceListScope(xpsScope_t scopeId,
                                                  xpsMcL2InterfaceListId_t l2IntfListId)
{
    XPS_FUNC_ENTRY_LOG();


    XP_STATUS retVal;
    xpsMcL2IntfListDbEntry *l2IntfListDbEntry;

    // Get the L2 IntfListDbEntry from the L2 interface list id.
    retVal = xpsMcL2GetIntfListDbEntry(scopeId, l2IntfListId, &l2IntfListDbEntry);
    if (retVal != XP_NO_ERR)
    {
        return retVal;
    }

    // If not found, can't proceed
    if (l2IntfListDbEntry == NULL)
    {
        retVal = XP_ERR_INVALID_DATA;
        return retVal;
    }

#if 0
    // Remove all encap nodes from the L2 interface list
    retVal = xpsMcL2RemoveEncapFromListForAllDevices(l2IntfListDbEntry);
    if (retVal != XP_NO_ERR)
    {
        return retVal;
    }
#endif

    retVal = xpsAllocatorReleaseId(XP_SCOPE_DEFAULT, XPS_ALLOCATOR_L3_NEXT_HOP_TBL,
                                   l2IntfListDbEntry->xpsRouteEntryIndex);
    if (retVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to release MC NH table index %d. ret %d",
              l2IntfListDbEntry->xpsRouteEntryIndex, retVal);
        return retVal;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
          "REL NH table index %d", l2IntfListDbEntry->xpsRouteEntryIndex);


    retVal = xpsAllocatorReleaseId(XP_SCOPE_DEFAULT, XP_ALLOC_MLL_PAIR_HW_ENTRY,
                                   l2IntfListDbEntry->xpsMLLPairIdx);
    if (retVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to release MLL PAIR table index %d. ret %d",
              l2IntfListDbEntry->xpsMLLPairIdx, retVal);
        return retVal;
    }

    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
          "REL MLL Pair index %d", l2IntfListDbEntry->xpsMLLPairIdx);
    // Delete L2 Interface List state from database
    retVal = xpsMcL2IntfListDbEntryDelete(scopeId, &l2IntfListDbEntry);
    if (retVal != XP_NO_ERR)
    {
        return retVal;
    }

    // Destroy the L2 Interface List
    retVal = xpsInterfaceDestroyScope(scopeId, l2IntfListId);
    if (retVal != XP_NO_ERR)
    {
        return retVal;
    }

    XPS_FUNC_EXIT_LOG();

    return retVal;
}

XP_STATUS xpsMulticastDestroyL3InterfaceList(xpsMcL3InterfaceListId_t ifListId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastDestroyL3InterfaceListScope(xpsScope_t scopeId,
                                                  xpsMcL3InterfaceListId_t ifListId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastCreateL2InterfaceList(xpVlan_t vlanId,
                                            xpsMcL2InterfaceListId_t *ifListId)
{
    XPS_FUNC_ENTRY_LOG();

    return xpsMulticastCreateL2InterfaceListScope(XP_SCOPE_DEFAULT, vlanId,
                                                  ifListId);
}

XP_STATUS xpsMulticastCreateL2InterfaceListScope(xpsScope_t scopeId,
                                                 xpVlan_t vlanId, xpsMcL2InterfaceListId_t *ifListId)
{
    XPS_FUNC_ENTRY_LOG();


    XP_STATUS retVal;
    xpsMcL2IntfListDbEntry *l2IntfListDbEntry = NULL;
    uint32_t mllPairIdx = 0;
    uint32_t routeEntIdx = 0;

    // Check for valid pointer.
    if (!ifListId)
    {
        return XP_ERR_NULL_POINTER;
    }

    // Create an interface of L2 Interface List type.
    retVal = xpsInterfaceCreateScope(scopeId, XPS_L2_MCAST_GROUP, ifListId);
    if (retVal != XP_NO_ERR)
    {
        return retVal;
    }

    // Add the interface list to the database
    retVal = xpsMcL2IntfListDbEntryInsert(scopeId, (*ifListId), &l2IntfListDbEntry);
    if (retVal != XP_NO_ERR)
    {
        return retVal;
    }

    /* Allocate MLL PAIR index */
    retVal = xpsAllocatorAllocateId(scopeId, XP_ALLOC_MLL_PAIR_HW_ENTRY,
                                    &mllPairIdx);
    if (retVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsAllocatorAllocateId Failed : %d\n", retVal);
        return retVal;
    }

    /* allocate routerEntryIndex.*/
    retVal = xpsAllocatorAllocateId(scopeId, XPS_ALLOCATOR_L3_NEXT_HOP_TBL,
                                    &routeEntIdx);
    if (retVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsAllocatorAllocateId Failed : %d\n", retVal);
        return retVal;
    }


    // Set the vlan id for the state
    l2IntfListDbEntry->vlanId = (uint32_t)vlanId;
    l2IntfListDbEntry->xpsMLLPairIdx = mllPairIdx;
    l2IntfListDbEntry->xpsRouteEntryIndex = routeEntIdx;

    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
          "Alloc MLL Pair index %d", l2IntfListDbEntry->xpsMLLPairIdx);
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
          "ALLOC NH table index %d", l2IntfListDbEntry->xpsRouteEntryIndex);

    XPS_FUNC_EXIT_LOG();

    return retVal;
}

XP_STATUS xpsMulticastCreateL3InterfaceList(xpsMcL3InterfaceListId_t *ifListId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastCreateL3InterfaceListScope(xpsScope_t scopeId,
                                                 xpsMcL3InterfaceListId_t *ifListId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastAddIPv4BridgeEntry(xpsDevice_t devId,
                                         xpsMulticastIPv4BridgeEntry_t *entry,
                                         uint32_t routeEntryIdx)
{
    XPS_FUNC_ENTRY_LOG();


    XPS_LOCK(xpsMulticastAddIPv4BridgeEntry);
    XP_STATUS result = XP_NO_ERR;
    GT_STATUS rc = GT_OK;
    GT_IPADDR ipGrpAddr;
    GT_IPADDR ipSrcAddr;
    GT_U32 ipGrpAddrLen = 0;
    GT_U32 ipSrcAddrLen = 0;
    GT_U32 lpmDbId = XPS_CPSS_DEFAULT_LPM_DB_ID;
    CPSS_DXCH_IP_LTT_ENTRY_STC mcRouteLttEntry;
    memset(&mcRouteLttEntry, 0, sizeof(CPSS_DXCH_IP_LTT_ENTRY_STC));

    if (!entry)
    {
        return XP_ERR_NULL_POINTER;
    }

    COPY_IPV4_ADDR_T(&ipGrpAddr, entry->groupAddress);
    COPY_IPV4_ADDR_T(&ipSrcAddr, entry->sourceAddress);

    if (ipSrcAddr.u32Ip != 0)
    {
        ipSrcAddrLen = 32;
    }

    if (ipGrpAddr.u32Ip != 0)
    {
        ipGrpAddrLen = 32;
    }

    /* (S,G) implictly creates (*,G) entry. Hence search will pass for case
       (S,G) and then (*,G) */
#if 0
    rc = cpssHalIpLpmIpv4McEntrySearch(lpmDbId, 0, &ipGrpAddr, ipGrpAddrLen,
                                       &ipSrcAddr, ipSrcAddrLen);
    if (rc == GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Already exist Mc Route : %d \n", rc);
        return xpsConvertCpssStatusToXPStatus(rc);
    }
#endif
    mcRouteLttEntry.routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
    mcRouteLttEntry.routeEntryBaseIndex = routeEntryIdx;
    mcRouteLttEntry.priority = CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_LPM_E;

    rc = cpssHalIpLpmIpv4McEntryAdd(lpmDbId, 0, &ipGrpAddr, ipGrpAddrLen,
                                    &ipSrcAddr, ipSrcAddrLen,
                                    &mcRouteLttEntry,
                                    GT_FALSE, GT_TRUE);

    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to Add Mc Route : %d \n", rc);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    XPS_FUNC_EXIT_LOG();

    return result;
}

XP_STATUS xpsMulticastAddIPv4RouteEntry(xpsDevice_t devId,
                                        xpsMulticastIPv4RouteEntry_t * entry, xpsHashIndexList_t *indexList)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastAddIPv6RouteEntry(xpsDevice_t devId,
                                        xpsMulticastIPv6RouteEntry_t * entry, xpsHashIndexList_t *indexList)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastAddIPv4PimBiDirRPFEntry(xpsDevice_t devId,
                                              xpsMulticastIPv4PimBidirRpfEntry_t * entry, xpsHashIndexList_t *indexList)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastGetIPv4BridgeEntry(xpsDevice_t devId, uint32_t index,
                                         xpsMulticastIPv4BridgeEntry_t *entry)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastGetIPv4RouteEntry(xpsDevice_t devId, uint32_t index,
                                        xpsMulticastIPv4RouteEntry_t * entry)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastGetIPv4PimBiDirRPFEntry(xpsDevice_t devId, uint32_t index,
                                              xpsMulticastIPv4PimBidirRpfEntry_t * entry)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastRemoveIPv4BridgeEntry(xpsDevice_t devId,
                                            xpsMulticastIPv4BridgeEntry_t *entry)
{
    XPS_FUNC_ENTRY_LOG();
    XPS_LOCK(xpsMulticastRemoveIPv4BridgeEntry);

    GT_STATUS rc = GT_OK;
    GT_IPADDR ipGrpAddr;
    GT_IPADDR ipSrcAddr;
    GT_U32 ipGrpAddrLen = 0;
    GT_U32 ipSrcAddrLen = 0;
    GT_U32 lpmDbId = XPS_CPSS_DEFAULT_LPM_DB_ID;
    CPSS_DXCH_IP_LTT_ENTRY_STC mcRouteLttEntry;
    memset(&mcRouteLttEntry, 0, sizeof(CPSS_DXCH_IP_LTT_ENTRY_STC));

    if (!entry)
    {
        return XP_ERR_NULL_POINTER;
    }

    COPY_IPV4_ADDR_T(&ipGrpAddr, entry->groupAddress);
    COPY_IPV4_ADDR_T(&ipSrcAddr, entry->sourceAddress);

    if (ipSrcAddr.u32Ip != 0)
    {
        ipSrcAddrLen = 32;
    }

    if (ipGrpAddr.u32Ip != 0)
    {
        ipGrpAddrLen = 32;
    }

    rc = cpssHalIpLpmIpv4McEntrySearch(lpmDbId, 0, &ipGrpAddr, ipGrpAddrLen,
                                       &ipSrcAddr, ipSrcAddrLen);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to Search Mc Route : %d \n", rc);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    rc = cpssHalIpLpmIpv4McEntryDel(lpmDbId, 0, &ipGrpAddr, ipGrpAddrLen,
                                    &ipSrcAddr, ipSrcAddrLen);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to Del Mc Route : %d \n", rc);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastRemoveIPv4RouteEntry(xpsDevice_t devId, uint32_t index)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastRemoveIPv4PimBiDirRPFEntry(xpsDevice_t devId,
                                                 uint32_t index)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastAddIPv6BridgeEntry(xpsDevice_t devId,
                                         xpsMulticastIPv6BridgeEntry_t *entry, xpsHashIndexList_t *indexList)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastGetIPv6BridgeEntry(xpsDevice_t devId, uint32_t index,
                                         xpsMulticastIPv6BridgeEntry_t *entry)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastGetIPv6RouteEntry(xpsDevice_t devId, uint32_t index,
                                        xpsMulticastIPv6RouteEntry_t * entry)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastRemoveIPv6BridgeEntry(xpsDevice_t devId, uint32_t index)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastRemoveIPv6RouteEntry(xpsDevice_t devId, uint32_t index)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastTriggerIpv4BridgeTableAging(xpDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastConfigureIpv4BridgeTableAging(xpDevice_t devId,
                                                    uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastConfigureIpv4BridgeEntryAging(xpDevice_t devId,
                                                    uint8_t enable, uint32_t index)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastGetIpv4BridgeTableAgingStatus(xpDevice_t devId,
                                                    bool *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastGetIpv4BridgeEntryAgingStatus(xpDevice_t devId,
                                                    bool *enable, uint32_t index)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastSetIpv4BridgeTableAgingCycle(xpDevice_t devId,
                                                   uint32_t unit)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastGetIpv4BridgeTableAgingCycle(xpDevice_t devId,
                                                   uint32_t *sec)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastRegisterIpv4BridgeTableDefaultAgingHandler(
    xpDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastRegisterIpv4BridgeTableAgingHandler(xpDevice_t devId,
                                                          multicastAgingHandler func, void *userData)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastUnregisterIpv4BridgeTableAgingHandler(xpDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastTriggerIpv4RouteTableAging(xpDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastConfigureIpv4RouteTableAging(xpDevice_t devId,
                                                   uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastConfigureIpv4RouteEntryAging(xpDevice_t devId,
                                                   uint8_t enable, uint32_t index)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastGetIpv4RouteTableAgingStatus(xpDevice_t devId,
                                                   bool *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastGetIpv4RouteEntryAgingStatus(xpDevice_t devId,
                                                   bool *enable, uint32_t index)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastSetIpv4RouteTableAgingCycle(xpDevice_t devId,
                                                  uint32_t sec)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastGetIpv4RouteTableAgingCycle(xpDevice_t devId,
                                                  uint32_t *sec)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastRegisterIpv4RouteTableDefaultAgingHandler(
    xpDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastRegisterIpv4RouteTableAgingHandler(xpDevice_t devId,
                                                         multicastAgingHandler func, void *userData)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastUnregisterIpv4RouteTableAgingHandler(xpDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


XP_STATUS xpsMulticastTriggerIpv6BridgeTableAging(xpDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastConfigureIpv6BridgeTableAging(xpDevice_t devId,
                                                    bool enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastConfigureIpv6BridgeEntryAging(xpDevice_t devId,
                                                    bool enable, uint32_t index)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastGetIpv6BridgeTableAgingStatus(xpDevice_t devId,
                                                    bool *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastGetIpv6BridgeEntryAgingStatus(xpDevice_t devId,
                                                    bool *enable, uint32_t index)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastSetIpv6BridgeTableAgingCycle(xpDevice_t devId,
                                                   uint32_t unit)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastGetIpv6BridgeTableAgingCycle(xpDevice_t devId,
                                                   uint32_t *sec)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastRegisterIpv6BridgeTableDefaultAgingHandler(
    xpDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastRegisterIpv6BridgeTableAgingHandler(xpDevice_t devId,
                                                          multicastAgingHandler func, void *userData)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastUnregisterIpv6BridgeTableAgingHandler(xpDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastTriggerIpv6RouteTableAging(xpDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastConfigureIpv6RouteTableAging(xpDevice_t devId,
                                                   bool enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastConfigureIpv6RouteEntryAging(xpDevice_t devId,
                                                   bool enable, uint32_t index)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastGetIpv6RouteTableAgingStatus(xpDevice_t devId,
                                                   bool *enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastGetIpv6RouteEntryAgingStatus(xpDevice_t devId,
                                                   bool *enable, uint32_t index)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastSetIpv6RouteTableAgingCycle(xpDevice_t devId,
                                                  uint32_t sec)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastSetIpv4MulticastBridgeEntryPktCmd(uint8_t devId,
                                                        uint32_t index, xpMulticastPktCmdType_e pktCmd)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastSetIpv4MulticastBridgeEntryVifIdx(uint8_t devId,
                                                        uint32_t index, uint32_t vifIdx)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastSetIpv6MulticastBridgeEntryPktCmd(uint8_t devId,
                                                        uint32_t index, xpMulticastPktCmdType_e pktCmd)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastSetIpv6MulticastBridgeEntryVifIdx(uint8_t devId,
                                                        uint32_t index, uint32_t vifIdx)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastSetIpv4MulticastRouteEntryPktCmd(uint8_t devId,
                                                       uint32_t index, xpMulticastPktCmdType_e pktCmd)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastSetIpv4MulticastRouteEntryVifIdx(uint8_t devId,
                                                       uint32_t index, uint32_t vifIdx)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastSetIpv6MulticastRouteEntryPktCmd(uint8_t devId,
                                                       uint32_t index, xpMulticastPktCmdType_e pktCmd)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastSetIpv6MulticastRouteEntryVifIdx(uint8_t devId,
                                                       uint32_t index, uint32_t vifIdx)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastSetIpv4MulticastRouteEntryRpfParams(uint8_t devId,
                                                          uint32_t index, xpMulticastRpfCheckType_e rpfType, uint32_t rpfValue,
                                                          xpMulticastRpfFailCmd_e rpfFailCmd)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastSetIpv6MulticastRouteEntryRpfParams(uint8_t devId,
                                                          uint32_t index, xpMulticastRpfCheckType_e rpfType, uint32_t rpfValue,
                                                          xpMulticastRpfFailCmd_e rpfFailCmd)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastGetIpv6RouteTableAgingCycle(xpDevice_t devId,
                                                  uint32_t *sec)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastRegisterIpv6RouteTableDefaultAgingHandler(
    xpDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastRegisterIpv6RouteTableAgingHandler(xpDevice_t devId,
                                                         multicastAgingHandler func, void *userData)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastUnregisterIpv6RouteTableAgingHandler(xpDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


XP_STATUS xpsMulticastAddDevice(xpsDevice_t devId, xpsInitType_t initType)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastRemoveDevice(xpsDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastInit(void)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastInitScope(xpsScope_t scopeId)
{
    XPS_FUNC_ENTRY_LOG();


    XP_STATUS retVal;
    GT_STATUS rc = GT_OK;
    uint32_t devId = 0;
    CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC routeEntryPtr;
    memset(&routeEntryPtr, 0, sizeof(CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC));

    rc = cpssHalIpMllBridgeEnable(devId, GT_TRUE);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " cpssHalIpMllBridgeEnable failed rc : %d ", rc);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    /* Need below entry to flood MC on ingress vlan (bridge-engine) decision,
       when, MC route look-up fails. MC route-look is triggered on vlan when MC
       KEY type, XG_AND_SG/SG/XG is set from SAI.*/
    routeEntryPtr.cmd = CPSS_PACKET_CMD_BRIDGE_E;

    rc = cpssHalIpMcRouteEntriesWrite(devId, XPS_l3_DEFAULT_MC_NEXTHOP,
                                      &routeEntryPtr);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " DefMC Entry cpssHalIpMcRouteEntriesWrite failed rc : %d ", rc);
        return xpsConvertCpssStatusToXPStatus(rc);
    }


    //    xpsLockCreate(XP_LOCKINDEX_XPS_MULTICAST, 1);

    /* HW MLL PAIR allocator shared between L2 and IP MLL PAIR table */
    retVal = xpsAllocatorInitIdAllocator(scopeId, XP_ALLOC_MLL_PAIR_HW_ENTRY,
                                         cpssHalGetSKUmaxMCMLLPair(devId),
                                         XPS_MLL_PAIR_RANGE_START);

    /* HW VIDX.*/
    retVal = xpsAllocatorInitIdAllocator(scopeId, XP_ALLOC_MC_VIDX_HW_TBL,
                                         cpssHalGetSKUmaxMCgrps(devId),
                                         XPS_MLL_VIDX_RANGE_START);

    /* HW eVIDX.*/
    retVal = xpsAllocatorInitIdAllocator(scopeId, XP_ALLOC_MC_EVIDX_HW_TBL,
                                         cpssHalGetSKUmaxMCeVidxgrps(devId),
                                         XPS_MLL_EVIDX_RANGE_START);

    xpsMcL2IntfListDbHndl = XPS_MCL2_INTFLIST_DB_HNDL;
    // Register xpsMcL2IntfList Database
    retVal = xpsStateRegisterDb(scopeId, "xpsMcL2IntfListDbEntry state", XPS_GLOBAL,
                                &xpsMcL2IntfListDbEntryComp, xpsMcL2IntfListDbHndl);
    if (retVal != XP_NO_ERR)
    {
        xpsMcL2IntfListDbHndl = XPS_STATE_INVALID_DB_HANDLE;
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " Register xpsMcL2IntfList Database failed");
        return retVal;
    }

    lagL2McGrpListDbHndl = XPS_LAG_L2MC_GRP_LIST_DB_HNDL;
    retVal = xpsStateRegisterDb(scopeId, "Lag - L2MCGrp ", XPS_GLOBAL,
                                &lagL2McListKeyComp, lagL2McGrpListDbHndl);
    if (retVal != XP_NO_ERR)
    {
        lagL2McGrpListDbHndl = XPS_STATE_INVALID_DB_HANDLE;
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Lag - L2Mc group state DB register failed");
        return retVal;
    }

    XPS_FUNC_EXIT_LOG();

    return retVal;
}

XP_STATUS xpsMulticastDeInit(void)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastDeInitScope(xpsScope_t scopeId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS retVal = XP_NO_ERR;

    retVal = xpsStateDeRegisterDb(scopeId, &lagL2McGrpListDbHndl);
    if (retVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Lag vlan list state De-Register DB handler failed");
        return retVal;
    }

    // Deregister xpsMcL2IntfList Database
    retVal = xpsStateDeRegisterDb(scopeId, &xpsMcL2IntfListDbHndl);
    if (retVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Deregister xpsMcL2IntfList Database failed");
        return retVal;
    }
    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastAddIpv4RouteControlEntry(xpsDevice_t devId,
                                               uint32_t vrfId, ipv4Addr_t groupAddress, uint32_t reasonCode,
                                               xpsMulticastPktCmdType_e pktCmd, xpsHashIndexList_t *indexList)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastGetIpv4RouteControlEntryReasonCode(xpDevice_t devId,
                                                         uint32_t index,
                                                         uint32_t *reasonCode)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastAddIpv6RouteControlEntry(xpsDevice_t devId,
                                               xpMcastDomainId_t ipv6McL3DomainId, ipv6Addr_t groupAddress,
                                               uint32_t reasonCode, xpsMulticastPktCmdType_e pktCmd,
                                               xpsHashIndexList_t *indexList)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastGetIpv6RouteControlEntryReasonCode(xpDevice_t devId,
                                                         uint32_t index,
                                                         uint32_t *reasonCode)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastSetIpv4BridgeRehashLevel(xpDevice_t devId,
                                               uint8_t numOfRehashLevels)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastGetIpv4BridgeRehashLevel(xpDevice_t devId,
                                               uint8_t* numOfRehashLevels)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastSetIpv6BridgeRehashLevel(xpDevice_t devId,
                                               uint8_t numOfRehashLevels)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastGetIpv6BridgeRehashLevel(xpDevice_t devId,
                                               uint8_t* numOfRehashLevels)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastSetIpv4RouteRehashLevel(xpDevice_t devId,
                                              uint8_t numOfRehashLevels)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastGetIpv4RouteRehashLevel(xpDevice_t devId,
                                              uint8_t* numOfRehashLevels)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastSetIpv6RouteRehashLevel(xpDevice_t devId,
                                              uint8_t numOfRehashLevels)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastGetIpv6RouteRehashLevel(xpDevice_t devId,
                                              uint8_t* numOfRehashLevels)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastSetIpv4PIMBiDirRPFRehashLevel(xpDevice_t devId,
                                                    uint8_t numOfRehashLevels)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastGetIpv4PIMBiDirRPFRehashLevel(xpDevice_t devId,
                                                    uint8_t* numOfRehashLevels)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastSetIpv6PIMBiDirRPFRehashLevel(xpDevice_t devId,
                                                    uint8_t numOfRehashLevels)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastGetIpv6PIMBiDirRPFRehashLevel(xpDevice_t devId,
                                                    uint8_t* numOfRehashLevels)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastGetIPv4BridgeEntryIndex(xpsDevice_t devId,
                                              xpsMulticastIPv4BridgeEntry_t *entry, uint32_t *index)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastGetIPv6BridgeEntryIndex(xpsDevice_t devId,
                                              xpsMulticastIPv6BridgeEntry_t *entry, uint32_t *index)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastGetIPv4RouteEntryIndex(xpsDevice_t devId,
                                             xpsMulticastIPv4RouteEntry_t *entry, uint32_t *index)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastGetIPv6RouteEntryIndex(xpsDevice_t devId,
                                             xpsMulticastIPv6RouteEntry_t *entry, uint32_t *index)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsMulticastAllocateVidx(xpsScope_t scopeId, uint32_t *vidx)
{
    XP_STATUS retVal = XP_NO_ERR;
    int devNum;
    int portNum;
    int cpssCscdPort;
    int cpssCscdMaxPorts;
    XP_DEV_TYPE_T devType;
    cpssHalGetDeviceType((xpsDevice_t)scopeId, &devType);

    /* Allocate MLL PAIR index */
    retVal = xpsAllocatorAllocateId(scopeId, XP_ALLOC_MC_VIDX_HW_TBL,
                                    vidx);
    if (retVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsAllocatorAllocateId Failed : %d\n", retVal);
        return retVal;
    }

    if (devType == ALDB2B)
    {
        XPS_B2B_CSCD_PORTS_ITER(devNum, portNum, cpssCscdPort, cpssCscdMaxPorts)
        {
            if ((cpssHalBrgMcMemberAdd(devNum, *vidx, cpssCscdPort)) != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Cascade port add failed for devNum %d vidx %d port %d", devNum, *vidx,
                      cpssCscdPort);
                return XP_ERR_INVALID_DATA;
            }
        }
    }

    return retVal;
}

XP_STATUS xpsMulticastReleaseVidx(xpsScope_t scopeId, uint32_t vidx)
{
    XP_STATUS retVal = XP_NO_ERR;
    GT_STATUS rc = GT_OK;
    int devNum;
    int portNum;
    int cpssCscdPort;
    int cpssCscdMaxPorts;
    XP_DEV_TYPE_T devType;
    cpssHalGetDeviceType((xpsDevice_t)scopeId, &devType);

    if (devType == ALDB2B)
    {
        XPS_B2B_CSCD_PORTS_ITER(devNum, portNum, cpssCscdPort, cpssCscdMaxPorts)
        {
            rc = cpssHalBrgMcMemberDelete(devNum, vidx, cpssCscdPort);
            if (rc != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Cascade port Del failed for devNum %d vidx %d port %d", devNum, vidx,
                      cpssCscdPort);
                return xpsConvertCpssStatusToXPStatus(rc);
            }
        }
    }

    rc = cpssHalBrgMcGroupDelete(0, vidx);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " cpssHalBrgMcGroupDelete failed rc : %d ", rc);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    /* Allocate MLL PAIR index */
    retVal = xpsAllocatorReleaseId(scopeId, XP_ALLOC_MC_VIDX_HW_TBL,
                                   vidx);
    if (retVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsAllocatorReleaseId Failed : %d\n", retVal);
        return retVal;
    }
    return retVal;
}

XP_STATUS xpsL2McReleaseOldIpMllPairIdx(xpsDevice_t devId,
                                        xpsMcL2InterfaceListId_t l2IntfListId)
{
    XP_STATUS retVal = XP_NO_ERR;
    GT_STATUS rc = GT_OK;
    xpsScope_t scopeId;

    xpsMcL2IntfListDbEntry *l2IntfListDbEntry;
    uint32_t mllPairIdx = 0;
    uint32_t nextMllPairIdx = 0;
    CPSS_DXCH_IP_MLL_PAIR_STC mllPairEntry;

    // Get Scope Id from devId
    if ((retVal = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Device does not exist, devId(%d)", devId);
        return retVal;
    }
    // Get the L2 IntfListDbEntry from the L2 interface list id.
    retVal = xpsMcL2GetIntfListDbEntry(scopeId, l2IntfListId, &l2IntfListDbEntry);
    if (retVal != XP_NO_ERR)
    {
        return retVal;
    }

    // If not found, can't proceed
    if (l2IntfListDbEntry == NULL)
    {
        retVal = XP_ERR_INVALID_DATA;
        return retVal;
    }

    memset(&mllPairEntry, 0, sizeof(CPSS_DXCH_IP_MLL_PAIR_STC));
    mllPairIdx = l2IntfListDbEntry->xpsMLLPairIdx;
    while (mllPairIdx != 0)
    {
        rc = cpssHalIpMLLPairRead(devId, mllPairIdx, CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E,
                                  &mllPairEntry);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " cpssHalIpMLLPairWrite failed rc : %d ", rc);
            return xpsConvertCpssStatusToXPStatus(rc);
        }
        nextMllPairIdx = mllPairEntry.nextPointer;

        memset(&mllPairEntry, 0, sizeof(CPSS_DXCH_IP_MLL_PAIR_STC));
        /*CPSS expects non-zero value */
        mllPairEntry.firstMllNode.mllRPFFailCommand = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
        mllPairEntry.secondMllNode.mllRPFFailCommand = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
        rc = cpssHalIpMLLPairWrite(devId, mllPairIdx, CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E,
                                   &mllPairEntry);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " cpssHalIpMLLPairWrite failed rc : %d ", rc);
            return xpsConvertCpssStatusToXPStatus(rc);
        }

        mllPairIdx = nextMllPairIdx;
    }

    return retVal;
}

XP_STATUS xpsMulticastIpMLLPairCreateFirstOnly(xpsDevice_t devId,
                                               xpsMcL2InterfaceListId_t l2IntfListId,
                                               uint32_t xpsVidx)
{
    xpsScope_t scopeId;
    XP_STATUS retVal = XP_NO_ERR;
    GT_STATUS rc = GT_OK;

    xpsMcL2IntfListDbEntry *l2IntfListDbEntry = NULL;
    CPSS_DXCH_IP_MLL_PAIR_STC mllPairEntry;

    memset(&mllPairEntry, 0, sizeof(CPSS_DXCH_IP_MLL_PAIR_STC));

    // Get Scope Id from devId
    if ((retVal = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Device does not exist, devId(%d)", devId);
        return retVal;
    }
    // Get the L2 IntfListDbEntry from the L2 interface list id.
    retVal = xpsMcL2GetIntfListDbEntry(scopeId, l2IntfListId, &l2IntfListDbEntry);
    if (retVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsMcL2GetIntfListDbEntry Failed, l2IntfListId(%d)", l2IntfListId);
        return retVal;
    }

    /* IGMP L2 MC needs only one node. Extend this function in future for second node. */

    mllPairEntry.firstMllNode.mllRPFFailCommand = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    mllPairEntry.firstMllNode.nextHopInterface.type = CPSS_INTERFACE_VIDX_E;
    mllPairEntry.firstMllNode.nextHopInterface.vidx = xpsVidx;
    mllPairEntry.firstMllNode.nextHopVlanId = l2IntfListDbEntry->vlanId;
    mllPairEntry.firstMllNode.last = GT_TRUE;

    rc = cpssHalIpMLLPairWrite(devId, l2IntfListDbEntry->xpsMLLPairIdx,
                               CPSS_DXCH_PAIR_READ_WRITE_FIRST_ONLY_E,
                               &mllPairEntry);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " cpssHalIpMLLPairWrite failed rc : %d ", rc);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    l2IntfListDbEntry->xpsVidx = xpsVidx;

    return retVal;
}


XP_STATUS xpsMulticastIpRouteEntryCreate(xpsDevice_t devId,
                                         xpsMcL2InterfaceListId_t l2IntfListId)
{
    xpsScope_t scopeId;
    GT_STATUS rc = GT_OK;
    XP_STATUS retVal = XP_NO_ERR;

    xpsMcL2IntfListDbEntry *l2IntfListDbEntry = NULL;
    CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC routeEntryPtr;

    memset(&routeEntryPtr, 0, sizeof(CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC));

    // Get Scope Id from devId
    if ((retVal = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Device does not exist, devId(%d)", devId);
        return retVal;
    }
    // Get the L2 IntfListDbEntry from the L2 interface list id.
    retVal = xpsMcL2GetIntfListDbEntry(scopeId, l2IntfListId, &l2IntfListDbEntry);
    if (retVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "xpsMcL2GetIntfListDbEntry Failed, l2IntfListId(%d)", l2IntfListId);
        return retVal;
    }
    routeEntryPtr.cmd = CPSS_PACKET_CMD_ROUTE_E;
    routeEntryPtr.internalMLLPointer = l2IntfListDbEntry->xpsMLLPairIdx;

    rc = cpssHalIpMcRouteEntriesWrite(devId, l2IntfListDbEntry->xpsRouteEntryIndex,
                                      &routeEntryPtr);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " cpssHalIpMcRouteEntriesWrite failed rc : %d ", rc);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    return retVal;
}

XP_STATUS xpsMcL2GetRouteEntryIdx(xpsDevice_t devId,
                                  xpsMcL2InterfaceListId_t l2IntfListId,
                                  uint32_t *idx)
{

    XP_STATUS xpsStatus = XP_NO_ERR;
    xpsScope_t scopeId;
    xpsMcL2IntfListDbEntry *l2IntfListDbEntry=NULL;

    // Get Scope Id from devId
    if ((xpsStatus = xpsScopeGetScopeId(devId, &scopeId))!= XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Device does not exist, devId(%d)", devId);
        return (xpsStatus);
    }

    xpsStatus = xpsMcL2GetIntfListDbEntry(scopeId, l2IntfListId,
                                          &l2IntfListDbEntry);
    if (xpsStatus != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Multicast L2 Interface List id does not exist, l2IntfListId(%d)",
              l2IntfListId);
        return (xpsStatus);
    }
    *idx = l2IntfListDbEntry->xpsRouteEntryIndex;

    return xpsStatus;
}

XP_STATUS xpsMulticastAddRemoveLagPortToL2Mc(xpsDevice_t devId,
                                             xpsInterfaceId_t lagIntfId,
                                             xpsInterfaceId_t portIntfId, bool isDel)
{
    XP_STATUS result = XP_NO_ERR;
    GT_STATUS rc = GT_OK;
    xpsScope_t scopeId;
    uint32_t i = 0;
    xpsLagL2McListDbEntry * lagL2McListEntry = NULL;
    xpsMcL2IntfListDbEntry *l2IntfListDbEntry = NULL;

    GT_U8 cpssDevId;
    GT_U32 cpssPortNum;
    //get scope from Device ID
    result = xpsScopeGetScopeId(devId, &scopeId);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Getting scope id failed");
        return result;
    }

    result = xpsL2McGetLagL2McListDb(scopeId, lagIntfId, &lagL2McListEntry);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get Lag Vlan List for lag interface(%d) failed", lagIntfId);
        return result;
    }

    if (!lagL2McListEntry)
    {
        result = XP_NO_ERR;
        return result;
    }


    for (i = 0; i < lagL2McListEntry->numOfL2McGrps; i++)
    {
        // Get the L2 IntfListDbEntry from the L2 interface list id.
        result = xpsMcL2GetIntfListDbEntry(scopeId, lagL2McListEntry->l2McGrpId[i],
                                           &l2IntfListDbEntry);
        if (result != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "xpsMcL2GetIntfListDbEntry Failed l2mcId %d ", lagL2McListEntry->l2McGrpId[i]);
            return result;
        }

        cpssDevId = xpsGlobalIdToDevId(devId, portIntfId);
        cpssPortNum = xpsGlobalPortToPortnum(devId, portIntfId);

        if (isDel == false)
        {
            rc = cpssHalBrgMcMemberAdd(cpssDevId, l2IntfListDbEntry->xpsVidx, cpssPortNum);
            if (rc != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to add port : %d \n", portIntfId);
                return xpsConvertCpssStatusToXPStatus(rc);
            }
        }
        else
        {
            rc = cpssHalBrgMcMemberDelete(cpssDevId, l2IntfListDbEntry->xpsVidx,
                                          cpssPortNum);
            if (rc != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to remove port : %d \n", portIntfId);
                return xpsConvertCpssStatusToXPStatus(rc);
            }
        }
    }

    return result;
}

XP_STATUS xpsL2McMLLPairWrite(xpsDevice_t devId,
                              void *ctx,
                              CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT format,
                              CPSS_INTERFACE_TYPE_ENT firstNodeEgrType,
                              CPSS_INTERFACE_TYPE_ENT secondNodeEgrType,
                              uint32_t firstNodeEgrId,
                              uint32_t secondNodeEgrId,
                              uint32_t firstNodeMeshId,
                              uint32_t secondNodeMeshId,
                              uint16_t mllPairIdx,
                              uint16_t mllPairNextIdx,
                              GT_BOOL isLast)
{
    XP_STATUS retVal = XP_NO_ERR;
    GT_STATUS rc = GT_OK;
    CPSS_DXCH_L2_MLL_PAIR_STC mllPairEntry;
    memset(&mllPairEntry, 0, sizeof(CPSS_DXCH_L2_MLL_PAIR_STC));

    if (!ctx)
    {
        return XP_ERR_NULL_POINTER;
    }

    switch (format)
    {
        case CPSS_DXCH_PAIR_READ_WRITE_FIRST_ONLY_E:
            {
                mllPairEntry.firstMllNode.egressInterface.type = firstNodeEgrType;
                if (firstNodeEgrType == CPSS_INTERFACE_VIDX_E)
                {
                    mllPairEntry.firstMllNode.egressInterface.vidx = firstNodeEgrId;
                }
                else if (firstNodeEgrType == CPSS_INTERFACE_TRUNK_E)
                {
                    mllPairEntry.firstMllNode.egressInterface.trunkId = firstNodeEgrId;
                }
                else if (firstNodeEgrType == CPSS_INTERFACE_PORT_E)
                {
                    if (IS_EPORT(firstNodeEgrId))
                    {
                        mllPairEntry.firstMllNode.egressInterface.devPort.hwDevNum = devId;
                        mllPairEntry.firstMllNode.egressInterface.devPort.portNum = firstNodeEgrId;
                    }
                    else
                    {
                        mllPairEntry.firstMllNode.egressInterface.devPort.hwDevNum =
                            xpsGlobalIdToDevId(devId, firstNodeEgrId);
                        mllPairEntry.firstMllNode.egressInterface.devPort.portNum =
                            xpsGlobalPortToPortnum(devId, firstNodeEgrId);
                    }
                }
                mllPairEntry.firstMllNode.meshId = firstNodeMeshId;
                mllPairEntry.firstMllNode.last = isLast;
            }
            break;
        case CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E:
            {
                mllPairEntry.firstMllNode.egressInterface.type = firstNodeEgrType;
                if (firstNodeEgrType == CPSS_INTERFACE_VIDX_E)
                {
                    mllPairEntry.firstMllNode.egressInterface.vidx = firstNodeEgrId;
                }
                else if (firstNodeEgrType == CPSS_INTERFACE_TRUNK_E)
                {
                    mllPairEntry.firstMllNode.egressInterface.trunkId = firstNodeEgrId;
                }
                else if (firstNodeEgrType == CPSS_INTERFACE_PORT_E)
                {
                    if (IS_EPORT(firstNodeEgrId))
                    {
                        mllPairEntry.firstMllNode.egressInterface.devPort.hwDevNum = devId;
                        mllPairEntry.firstMllNode.egressInterface.devPort.portNum = firstNodeEgrId;
                    }
                    else
                    {
                        mllPairEntry.firstMllNode.egressInterface.devPort.hwDevNum =
                            xpsGlobalIdToDevId(devId, firstNodeEgrId);
                        mllPairEntry.firstMllNode.egressInterface.devPort.portNum =
                            xpsGlobalPortToPortnum(devId, firstNodeEgrId);
                    }
                }
                mllPairEntry.firstMllNode.meshId = firstNodeMeshId;
                mllPairEntry.firstMllNode.last = GT_FALSE;

                mllPairEntry.secondMllNode.egressInterface.type = secondNodeEgrType;
                if (secondNodeEgrType == CPSS_INTERFACE_VIDX_E)
                {
                    mllPairEntry.secondMllNode.egressInterface.vidx = secondNodeEgrId;
                }
                else if (secondNodeEgrType == CPSS_INTERFACE_TRUNK_E)
                {
                    mllPairEntry.secondMllNode.egressInterface.trunkId = secondNodeEgrId;
                }
                else if (secondNodeEgrType == CPSS_INTERFACE_PORT_E)
                {
                    if (IS_EPORT(secondNodeEgrId))
                    {
                        mllPairEntry.secondMllNode.egressInterface.devPort.hwDevNum = devId;
                        mllPairEntry.secondMllNode.egressInterface.devPort.portNum = secondNodeEgrId;
                    }
                    else
                    {
                        mllPairEntry.secondMllNode.egressInterface.devPort.hwDevNum =
                            xpsGlobalIdToDevId(devId, secondNodeEgrId);
                        mllPairEntry.secondMllNode.egressInterface.devPort.portNum =
                            xpsGlobalPortToPortnum(devId, secondNodeEgrId);
                    }
                }
                mllPairEntry.secondMllNode.meshId = secondNodeMeshId;
                mllPairEntry.secondMllNode.last = isLast;
            }
            break;
        case CPSS_DXCH_PAIR_READ_WRITE_SECOND_NEXT_POINTER_ONLY_E:
        default:
            {
            }
            break;
    }
    mllPairEntry.nextPointer = mllPairNextIdx;
    rc = cpssHalL2MLLPairWrite(devId, mllPairIdx, format, &mllPairEntry);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " cpssHalL2MLLPairWrite failed rc : %d ", rc);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    return retVal;
}

XP_STATUS xpsMcReleaseOldL2MllPairIdx(xpsDevice_t devId,
                                      uint32_t prevBaseIdx)
{
    XP_STATUS retVal = XP_NO_ERR;
    GT_STATUS rc = GT_OK;
    CPSS_DXCH_L2_MLL_PAIR_STC mllPairEntry;
    uint32_t mllPairIdx = prevBaseIdx;
    uint32_t nextMllPairIdx = 0;
    memset(&mllPairEntry, 0, sizeof(CPSS_DXCH_L2_MLL_PAIR_STC));

    while (mllPairIdx != 0)
    {
        rc = cpssHalL2MLLPairRead(devId, mllPairIdx, CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E,
                                  &mllPairEntry);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " cpssHalL2MLLPairWrite failed rc : %d ", rc);
            return xpsConvertCpssStatusToXPStatus(rc);
        }
        nextMllPairIdx = mllPairEntry.nextPointer;

        memset(&mllPairEntry, 0, sizeof(CPSS_DXCH_L2_MLL_PAIR_STC));
        /*CPSS expects non-zero value */
        rc = cpssHalL2MLLPairWrite(devId, mllPairIdx, CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E,
                                   &mllPairEntry);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " cpssHalL2MLLPairWrite failed rc : %d ", rc);
            return xpsConvertCpssStatusToXPStatus(rc);
        }
        retVal = xpsAllocatorReleaseId(XP_SCOPE_DEFAULT, XP_ALLOC_MLL_PAIR_HW_ENTRY,
                                       mllPairIdx);
        if (retVal != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to release MLL PAIR table index %d. ret %d",
                  mllPairIdx, retVal);
            return retVal;
        }
        mllPairIdx = nextMllPairIdx;
    }

    return retVal;
}

#ifdef __cplusplus
}
#endif

