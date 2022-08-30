// xpsInterface.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/


#include "xpsLock.h"
#include "xpsAllocator.h"
#include "xpsPort.h"
#include "cpssHalUtil.h"
#include "cpssHalDevice.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \file xpsInterface.c
 * \brief Provides xps a utility to manage software interfaces
 *
 * This file manages software interfaces. A software interface
 * is a logical interface construct that only exists within the
 * xps and above domain.
 *
 * It is an abstraction over a hardware vif and its
 * encapsulation, as well as an abstraction which provides a
 * notion of l1/l2/l3 interface properties which may be
 * implemented differently in different devices within the xp
 * family.
 *
 * All interfaces have an associated software id which is
 * managed by this set of APIs.
 *
 */

/**
 * \def XPS_DEF_NUM_ALLOCATOR_GROUPS
 *
 * This value represents the number of allocators used to
 * allocate different vif interfaces and bd interfaces
 */

#define XPS_DEF_NUM_ALLOCATOR_GROUPS 8
/**
 * \struct xpsInterfaceAllocatorRangeInfo
 * \brief This structure contains the start range and size(maxIds) for a
 * type of allocator
 */

typedef struct xpsInterfaceAllocatorRangeInfo
{
    xpsAllocatorId      allocatorType;
    uint32_t             rangeStart;
    uint32_t             rangeSize;

} xpsInterfaceAllocatorRangeInfo_t;

//TODO Module managers must tune the values of rangeStart and rangeSize
static xpsInterfaceAllocatorRangeInfo_t
defaultAllocatorRange[XPS_DEF_NUM_ALLOCATOR_GROUPS] =
{
    // allocatorType ,                         rangeStart,                   rangeSize
    {XPS_ALLOCATOR_VIF_PORT,                  XPS_GLOBAL_START_PORT,          4*1024  },
    {XPS_ALLOCATOR_VIF_LAG,                   4*1024,                         4*1024  },
    {XPS_ALLOCATOR_VIF_TUNNEL_ENCAP,          9*1024,                         5*1024  },
    {XPS_ALLOCATOR_VIF_MULTI_PORT,            14*1024,                        5*1024  },
    {XPS_ALLOCATOR_VIF_SEGMENT_ROUTE,         19*1024,                        4*1024  },
    {XPS_ALLOCATOR_AC,                        23*1024,                        13*1024 },
    {XPS_ALLOCATOR_L2_DOMAIN_TYPE_ROUTE_ONLY, 36*1024,                        5*1024  },
    {XPS_ALLOCATOR_BRIDGE_PORT,               41*1024,                        4*1024  },
};

static int32_t intfChildKeyCompare(void* key1, void* key2)
{
    return ((int32_t)(((xpsInterfaceChildInfo_t*)key1)->keyIntfId) - (int32_t)(((
                                                                                    xpsInterfaceChildInfo_t*)key2)->keyIntfId));
}

/**
 * \var intfDbHndl
 * \brief Static global handle to the interface db
 *
 */

static xpsDbHandle_t intfDbHndl = XPS_STATE_INVALID_DB_HANDLE;


/**
 * \fn intfKeyCompare
 * \brief Statically defined key compare function for interface
 *        manager's database
 *
 * This API is defined as static, but will be registered via
 * function pointer to be used internally by State manager to do
 * comparisons for the interface manager database
 *
 * \param [in] void* key1
 * \param [in] void* key2
 *
 * \return int32_t
 */

static int32_t intfKeyCompare(void* key1, void* key2)
{
    return ((int32_t)(((xpsInterfaceInfo_t*)key1)->keyIntfId) - (int32_t)(((
                                                                               xpsInterfaceInfo_t*)key2)->keyIntfId));
}

XP_STATUS xpsInterfaceInitAllocator(xpsScope_t scopeId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;

    for (uint32_t i = 0; i <  XPS_DEF_NUM_ALLOCATOR_GROUPS; i++)
    {
        status = xpsAllocatorInitIdAllocator(scopeId,
                                             defaultAllocatorRange[i].allocatorType,
                                             defaultAllocatorRange[i].rangeSize, defaultAllocatorRange[i].rangeStart);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Interface init failed for allocator type (%d)",
                  defaultAllocatorRange[i].allocatorType);
            return status;
        }
    }

    XPS_FUNC_EXIT_LOG();

    return status;
}

XP_STATUS xpsInterfaceInit(void)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;

    status =  xpsInterfaceInitScope(XP_SCOPE_DEFAULT);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Interface Init failed ");
        return status;
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsInterfaceInitScope(xpsScope_t scopeId)
{
    XPS_FUNC_ENTRY_LOG();

    xpsLockCreate(XP_LOCKINDEX_XPS_INTERFACE, 1);

    XP_STATUS status = XP_NO_ERR;

    status = xpsInterfaceInitAllocator(scopeId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "XPS Interface allocation Init failed");
        return status;
    }

    // Create Global Interface Db
    intfDbHndl = XPS_INTF_DB_HNDL;
    if ((status = xpsStateRegisterDb(scopeId, "Interface Db", XPS_GLOBAL,
                                     &intfKeyCompare, intfDbHndl)) != XP_NO_ERR)
    {
        intfDbHndl = XPS_STATE_INVALID_DB_HANDLE;
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Global interface creation failed");
        return status;
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsInterfaceAddDevice(xpsDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsInterfaceRemoveDevice(xpsDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsInterfaceDeInit()
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;

    status = xpsInterfaceDeInitScope(XP_SCOPE_DEFAULT);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Interface DeInit failed ");
        return status;
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsInterfaceDeInitScope(xpsScope_t scopeId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;

    // Purge Global Interface Db
    if ((status = xpsStateDeRegisterDb(scopeId, &intfDbHndl)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Global interface purge failed");
        return status;
    }

    xpsLockDestroy(XP_LOCKINDEX_XPS_INTERFACE);

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsInterfaceIsBridgePort(xpsScope_t scopeId, xpsInterfaceId_t intfId,
                                   bool *isBridgePort)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;
    xpsInterfaceInfo_t *info = NULL;

    /* get intferface info from the list */
    if ((status = xpsInterfaceGetInfoScope(scopeId, intfId, &info)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "get Interface info from list failed, interface(%d)", intfId);
        return status;
    }

    if (info)
    {
        *isBridgePort = (info->type == XPS_BRIDGE_PORT);
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsInterfaceIsExist(xpsInterfaceId_t intfId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;

    status =  xpsInterfaceIsExistScope(XP_SCOPE_DEFAULT, intfId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Interface search failed ");
        return status;
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsInterfaceIsExistScope(xpsScope_t scopeId, xpsInterfaceId_t intfId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status         = XP_NO_ERR;
    xpsInterfaceInfo_t intfInfoKey;
    xpsInterfaceInfo_t *info = NULL;
    intfInfoKey.keyIntfId    = intfId;

    /* search for an interface in the list */
    if ((status = xpsStateSearchData(scopeId, intfDbHndl, (xpsDbKey_t)&intfInfoKey,
                                     (void**)&info)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Search Interface in list, interface(%d)", intfId);
        return status;
    }

    /* interface context found ? */
    if (info == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Interface does not exist, interface(%d)", intfId);
        status = XP_ERR_NOT_FOUND;
        return status;
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsInterfaceCreateBridgePort(uint32_t vifId, uint16_t vlanId,
                                       uint32_t *brPortId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;
    XPS_LOCK(xpsInterfaceCreateBridgePort);

    status =  xpsInterfaceCreateScope(XP_SCOPE_DEFAULT, XPS_BRIDGE_PORT, brPortId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Bridge Port Create Failed");
        return status;
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsInterfaceCreate(xpsInterfaceType_e type, xpsInterfaceId_t *intfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsInterfaceCreate);

    XP_STATUS status = XP_NO_ERR;

    status = xpsInterfaceCreateScope(XP_SCOPE_DEFAULT, type, intfId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Interface create failed ");
        return status;
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsInterfaceCreateScope(xpsScope_t scopeId, xpsInterfaceType_e type,
                                  xpsInterfaceId_t *intfId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status         = XP_NO_ERR;
    uint32_t  ecmpSize       = XPS_INTF_DEFAULT_ECMP_SIZE;
    xpsInterfaceInfo_t *info = NULL;
    void* metadata           = NULL;
    xpsDevice_t devId = 0;
    xpsRBTree_t *rbtree = NULL;

    // Allocate Vif for the new interface
    switch (type)
    {
        case XPS_PORT:
            status = xpsAllocatorAllocateId(scopeId, XPS_ALLOCATOR_VIF_PORT,
                                            (uint32_t*)intfId);
            if (status != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "VIF allocation for port failed");
                return status;
            }

            if ((status = xpsStateHeapMalloc(sizeof(xpsPhysicalPortMetadata_t),
                                             &metadata)) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Metadata allocation for port failed");
                (void)xpsAllocatorReleaseId(scopeId, XPS_ALLOCATOR_VIF_PORT, (uint32_t)*intfId);
                return status;
            }
            if (!metadata)
            {
                status = XP_ERR_NULL_POINTER;
                (void)xpsAllocatorReleaseId(scopeId, XPS_ALLOCATOR_VIF_PORT, (uint32_t)*intfId);
                return status;
            }

            memset(metadata, 0, sizeof(xpsPhysicalPortMetadata_t));

            if ((status = xpsRBTInit(&rbtree, &intfChildKeyCompare,
                                     XPS_VERSION)) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Global DB initialization failed");
                xpsStateHeapFree(metadata);
                return status;
            }
            if (!rbtree)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Global DB, out of memory");
                return XP_ERR_OUT_OF_MEM;
            }

            break;

        case XPS_LAG:
            status = xpsAllocatorAllocateId(scopeId, XPS_ALLOCATOR_VIF_LAG,
                                            (uint32_t*)intfId);
            if (status != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "VIF allocation for LAG failed");
                return status;
            }
            /* Validate the HW limit.*/
            if (xpsUtilXpstoCpssInterfaceConvert(*intfId,
                                                 XPS_LAG) > cpssHalGetSKUMaxLAGrps(devId))
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "LAG Table Full. Max (%d) Allocated (%d) ",
                      cpssHalGetSKUMaxLAGrps(devId),
                      xpsUtilXpstoCpssInterfaceConvert(*intfId, XPS_LAG));
                (void)xpsAllocatorReleaseId(scopeId, XPS_ALLOCATOR_VIF_LAG, (uint32_t)*intfId);
                return XP_ERR_OUT_OF_RANGE;
            }

            break;

        case XPS_TUNNEL_MPLS:
        case XPS_TUNNEL_VXLAN:
        case XPS_TUNNEL_NVGRE:
        case XPS_TUNNEL_GENEVE:
        case XPS_TUNNEL_PBB:
        case XPS_TUNNEL_GRE:
        case XPS_TUNNEL_VPN_GRE:
        case XPS_TUNNEL_VPN_GRE_LOOSE:
        case XPS_TUNNEL_GRE_ERSPAN2:
        case XPS_TUNNEL_IP_IN_IP:
            status = xpsAllocatorAllocateId(scopeId, XPS_ALLOCATOR_VIF_TUNNEL_ENCAP,
                                            (uint32_t*)intfId);
            if (status != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "VIF allocation for tunnel failed");
                return status;
            }
            break;

        case XPS_L2_MCAST_GROUP:
        case XPS_L3_MCAST_GROUP:
            status = xpsAllocatorAllocateId(scopeId, XPS_ALLOCATOR_VIF_MULTI_PORT,
                                            (uint32_t*)intfId);
            if (status != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "VIF allocation for multicast group failed");
                return status;
            }
            break;

        case XPS_PORT_ROUTER:
        case XPS_TUNNEL_ROUTER:
        case XPS_VPN_ROUTER:
        case XPS_SUBINTERFACE_ROUTER:
            status = xpsAllocatorAllocateId(scopeId,
                                            XPS_ALLOCATOR_L2_DOMAIN_TYPE_ROUTE_ONLY, (uint32_t*)intfId);
            if (status != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Bd allocation for route failed");
                return status;
            }

            // Map the allocated BD into an interface ID
            *intfId = XPS_INTF_MAP_BD_TO_INTF(*intfId);
            if ((status = xpsRBTInit(&rbtree, &intfChildKeyCompare,
                                     XPS_VERSION)) != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Global DB initialization failed");
                return status;
            }
            if (!rbtree)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Global DB, out of memory");
                return XP_ERR_OUT_OF_MEM;
            }
            break;

        case XPS_AC:
            status = xpsAllocatorAllocateId(scopeId, XPS_ALLOCATOR_AC, (uint32_t*)intfId);
            if (status != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "AC allocation failed\n");
                return status;
            }
            *((uint32_t*)(intfId)) = *((uint32_t*)(intfId)) + XPS_AC_INTERFACE_START_OFFSET;
            break;

        case XPS_SEGMENT_ROUTING:
            status = xpsAllocatorAllocateId(scopeId, XPS_ALLOCATOR_VIF_SEGMENT_ROUTE,
                                            (uint32_t*)intfId);
            if (status != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "VIF allocation for multicast group failed");
                return status;
            }
            break;

        case XPS_BRIDGE_PORT:
            status = xpsAllocatorAllocateId(scopeId, XPS_ALLOCATOR_BRIDGE_PORT,
                                            (uint32_t*)intfId);
            if (status != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "VIF allocation for bridge port failed");
                return status;
            }
            break;

        default:
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid parameter");
            status = XP_ERR_INVALID_PARAMS;
            return status;
    }


    // Allocate space for the interface info struct
    if ((status = xpsStateHeapMalloc(sizeof(xpsInterfaceInfo_t),
                                     (void**)&info)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Space allocation for Interface info failed");
        if (metadata)
        {
            xpsStateHeapFree(metadata);
        }
        if (rbtree)
        {
            xpsRBTDelete(rbtree);
        }
        return status;
    }
    if (!info)
    {
        status = XP_ERR_NULL_POINTER;
        if (metadata)
        {
            xpsStateHeapFree(metadata);
        }
        if (rbtree)
        {
            xpsRBTDelete(rbtree);
        }
        return status;
    }
    memset(info, 0, sizeof(xpsInterfaceInfo_t));

    // Set the interface properties
    info->keyIntfId  = *intfId;
    info->type       = type;
    info->l2EcmpSize = ecmpSize;
    info->pMetadata  = metadata;
    info->stpId  = XPS_INVALID_STP;
    info->intfMappingInfo.type = XPS_INVALID_IF_TYPE;
    info->intfMappingInfo.keyIntfId = XPS_INTF_INVALID_ID;


    if (rbtree)
    {
        info->rbtree = rbtree;
    }
    // Insert the interface info structure into the database, using the vif as a key
    //

    if ((status = xpsStateInsertData(scopeId, intfDbHndl,
                                     (void*)info)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Insertion of interface info failed");
        // Free Allocated memory
        xpsStateHeapFree((void*)info);
        if (metadata)
        {
            xpsStateHeapFree(metadata);
        }

        if (rbtree)
        {
            xpsRBTDelete(rbtree);
        }
        return status;
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsInterfaceCreateRouterOverVlan(uint32_t vlanId,
                                           xpsInterfaceId_t *intfId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;

    status =  xpsInterfaceCreateRouterOverVlanScope(XP_SCOPE_DEFAULT, vlanId,
                                                    intfId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Vlan Router Interface create failed ");
        return status;
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsInterfaceCreateRouterOverVlanScope(xpsScope_t scopeId,
                                                uint32_t vlanId, xpsInterfaceId_t *intfId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;
    xpsInterfaceInfo_t *info = NULL;
    xpsRBTree_t *rbtree = NULL;

    if (intfId == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG, "Null pointer passed!");
        return XP_ERR_INVALID_PARAMS;
    }

    // Map the vlanId into a interface id (vlan id == bd id)
    *intfId = XPS_INTF_MAP_BD_TO_INTF(vlanId);

    // Allocate space for the interface info data
    if ((status = xpsStateHeapMalloc(sizeof(xpsInterfaceInfo_t),
                                     (void**)&info)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Space allocation for Interface info failed");
        xpsStateHeapFree((void*)info);
        return status;
    }

    if (!info)
    {
        status = XP_ERR_NULL_POINTER;
        return status;
    }

    if ((status = xpsRBTInit(&rbtree, &intfChildKeyCompare,
                             XPS_VERSION)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Global DB initialization failed");
        xpsStateHeapFree((void*)info);
        return status;
    }
    if (!rbtree)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Global DB, out of memory");
        return XP_ERR_OUT_OF_MEM;
    }

    // Set the interface properties
    info->keyIntfId  = *intfId;
    info->type       = XPS_VLAN_ROUTER;
    info->l2EcmpSize = XPS_INTF_DEFAULT_ECMP_SIZE;

    info->rbtree = rbtree;

    // Insert the interface info structure into the databse, using the mapped bd as a key
    if ((status = xpsStateInsertData(scopeId, intfDbHndl,
                                     (void*)info)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
              "Insertion of interface info failed");
        // Free Allocated Memory
        xpsStateHeapFree((void*)info);
        return status;
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsInterfaceDestroy(xpsInterfaceId_t intfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsInterfaceDestroy);

    XP_STATUS status = XP_NO_ERR;

    status = xpsInterfaceDestroyScope(XP_SCOPE_DEFAULT, intfId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Interface destroy failed ");
        return status;
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsInterfaceDestroyBridgePort(xpsInterfaceId_t intfId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_LOCK(xpsInterfaceDestroyBridgePort);

    XP_STATUS status = XP_NO_ERR;

    status = xpsInterfaceDestroyScope(XP_SCOPE_DEFAULT, intfId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Interface destroy failed ");
        return status;
    }


    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsInterfaceDestroyScope(xpsScope_t scopeId, xpsInterfaceId_t intfId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;
    xpsInterfaceInfo_t intfInfoKey;
    xpsInterfaceInfo_t *info = NULL;
    intfInfoKey.keyIntfId = intfId;

    // Remove the interface from the db
    if ((status = xpsStateDeleteData(scopeId, intfDbHndl, (xpsDbKey_t)&intfInfoKey,
                                     (void**)&info)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Removal of Interface info failed");
        return status;
    }
    if (!info)
    {
        status = XP_ERR_NOT_FOUND;
        return status;
    }

    switch (info->type)
    {
        // Release the interface Id back into the free pool
        case XPS_PORT_ROUTER:
        case XPS_VLAN_ROUTER:
        case XPS_TUNNEL_ROUTER:
        case XPS_VPN_ROUTER:
        case XPS_SUBINTERFACE_ROUTER:
            intfId = XPS_INTF_MAP_INTF_TO_BD(intfId);

            status = xpsAllocatorReleaseId(scopeId, XPS_ALLOCATOR_L2_DOMAIN_TYPE_ROUTE_ONLY,
                                           intfId);
            if (status != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Release of Interface Id failed");
                return status;
            }
            break;

        case  XPS_AC:
            status = xpsAllocatorReleaseId(scopeId, XPS_ALLOCATOR_AC, intfId);
            if (status != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Release of AC (%d) failed\n",
                      intfId);
                return status;
            }
            break;

        case XPS_PORT:
            status = xpsAllocatorReleaseId(scopeId, XPS_ALLOCATOR_VIF_PORT, intfId);
            if (status != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Release of single port interface Id failed");
                return status;

            }

            status = xpsStateHeapFree(info->pMetadata);
            if (status != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Release of Port Metadata failed");
                return status;
            }
            info->pMetadata = NULL;
            break;

        case XPS_TUNNEL_MPLS:
        case XPS_TUNNEL_VXLAN:
        case XPS_TUNNEL_NVGRE:
        case XPS_TUNNEL_GENEVE:
        case XPS_TUNNEL_PBB:
        case XPS_TUNNEL_GRE:
        case XPS_TUNNEL_VPN_GRE:
        case XPS_TUNNEL_VPN_GRE_LOOSE:
        case XPS_TUNNEL_GRE_ERSPAN2:
        case XPS_TUNNEL_IP_IN_IP:
            status = xpsAllocatorReleaseId(scopeId, XPS_ALLOCATOR_VIF_TUNNEL_ENCAP, intfId);
            if (status != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Release of Tunnel Interface Id failed");
                return status;
            }
            break;

        case XPS_L2_MCAST_GROUP:
        case XPS_L3_MCAST_GROUP:
            status = xpsAllocatorReleaseId(scopeId, XPS_ALLOCATOR_VIF_MULTI_PORT, intfId);
            if (status != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Release of Multi-port group interface Id failed");
                return status;
            }
            break;

        case XPS_SEGMENT_ROUTING:
            status = xpsAllocatorReleaseId(scopeId, XPS_ALLOCATOR_VIF_SEGMENT_ROUTE,
                                           intfId);
            if (status != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Release vif segment route failed, interface(%d)", intfId);
                return status;
            }
            break;

        case XPS_LAG:
            status = xpsAllocatorReleaseId(scopeId, XPS_ALLOCATOR_VIF_LAG, intfId);
            if (status != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Release of LAG interface(%d)",
                      intfId);
                return status;
            }
            break;

        case XPS_BRIDGE_PORT:
            status = xpsAllocatorReleaseId(scopeId, XPS_ALLOCATOR_BRIDGE_PORT, intfId);
            if (status != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Release of bridge port interface(%d)", intfId);
                return status;
            }
            break;

        default:
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid parameter");
            status = XP_ERR_INVALID_PARAMS;
            return status;
    }
    if (info->rbtree)
    {
        status = xpsRBTDelete(info->rbtree);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "RBTDel failed");
            return status;
        }
    }

    // Free the allocated memory
    if ((status = xpsStateHeapFree((void*)info)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Freeing allocated memory failed");
        return status;
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsInterfaceDestroyRouterOverVlan(uint32_t vlanId,
                                            xpsInterfaceId_t intfId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;

    status =  xpsInterfaceDestroyRouterOverVlanScope(XP_SCOPE_DEFAULT, vlanId,
                                                     intfId);

    if (status != XP_NO_ERR)
    {

        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " Getting Interface Info failed");
        return status;
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsInterfaceDestroyRouterOverVlanScope(xpsScope_t scopeId,
                                                 uint32_t vlanId, xpsInterfaceId_t intfId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;
    uint32_t  check  = 0;
    xpsInterfaceInfo_t *info = NULL;
    xpsInterfaceInfo_t intfInfoKey;

    // Map bdId into interface id (vlanId == bd)
    check = XPS_INTF_MAP_BD_TO_INTF(vlanId);

    // Ensure that the interface id corresponds to the router interface for this vlan
    if ((xpsInterfaceId_t)check != intfId)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Router interface %d is not associated with vlan %d\n", intfId, vlanId);
        return XP_ERR_INVALID_PARAMS;
    }

    intfInfoKey.keyIntfId = intfId;

    // Remove the record from the database
    if ((status = xpsStateDeleteData(scopeId, intfDbHndl, (xpsDbKey_t)&intfInfoKey,
                                     (void**)&info)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Removal of data failed, interface(%d)", intfId);
        return status;
    }

    // Free the memory
    if ((status = xpsStateHeapFree((void*)info)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Freeing allocated memory failed");
        return status;
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


XP_STATUS xpsInterfaceGetInfo(xpsInterfaceId_t intfId,
                              xpsInterfaceInfo_t **info)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;

    status =  xpsInterfaceGetInfoScope(XP_SCOPE_DEFAULT, intfId, info);
    if (status != XP_NO_ERR)
    {

        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " Getting Interface Info failed");
        return status;
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsInterfaceGetInfoScope(xpsScope_t scopeId, xpsInterfaceId_t intfId,
                                   xpsInterfaceInfo_t **info)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;
    xpsInterfaceInfo_t intfInfoKey;
    intfInfoKey.keyIntfId = intfId;

    if ((status = xpsStateSearchData(scopeId, intfDbHndl, (xpsDbKey_t)&intfInfoKey,
                                     (void**)info)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Search data failed, interface(%d)", intfId);
        return status;
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsInterfaceGetType(xpsInterfaceId_t intfId, xpsInterfaceType_e *type)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;

    status = xpsInterfaceGetTypeScope(XP_SCOPE_DEFAULT, intfId, type);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Getting Interface(%d) type failed", intfId);
        return status;
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsInterfaceGetTypeScope(xpsScope_t scopeId, xpsInterfaceId_t intfId,
                                   xpsInterfaceType_e *type)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status         = XP_NO_ERR;
    xpsInterfaceInfo_t *info = NULL;
    xpsInterfaceInfo_t intfInfoKey;
    intfInfoKey.keyIntfId    = intfId;

    if ((status = xpsStateSearchData(scopeId, intfDbHndl, (xpsDbKey_t)&intfInfoKey,
                                     (void**)&info)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Search data failed, interface(%d)", intfId);
        return status;
    }
    if (!info)
    {
        status = XP_ERR_NOT_FOUND;
        return status;
    }

    *type = info->type;

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsInterfaceSetL3MtuProfile(xpsInterfaceId_t intfId,
                                      uint32_t l3MtuProfile)
{
    XPS_FUNC_ENTRY_LOG();

    xpsInterfaceInfo_t  intfInfoKey;
    xpsInterfaceInfo_t *pIntfInfo = NULL;
    XP_STATUS           status = XP_NO_ERR;

    memset(&intfInfoKey, 0, sizeof(xpsInterfaceInfo_t));
    intfInfoKey.keyIntfId = intfId;

    status = xpsStateSearchData(XP_SCOPE_DEFAULT, intfDbHndl,
                                (xpsDbKey_t)&intfInfoKey, (void**)&pIntfInfo);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Search data failed, interface(%d)", intfId);
        return status;
    }
    if (!pIntfInfo)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Info is invalid for intfId:%d", intfId);
        return XP_ERR_NOT_FOUND;
    }

    pIntfInfo->mtuPorfileIndex = l3MtuProfile;

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsInterfaceSetL2EcmpSize(xpsInterfaceId_t intfId,
                                    uint32_t l2EcmpSize)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;

    status = xpsInterfaceSetL2EcmpSizeScope(XP_SCOPE_DEFAULT, intfId, l2EcmpSize);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Setting Interface L2 Ecmp Size failed");
        return status;
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsInterfaceSetL2EcmpSizeScope(xpsScope_t scopeId,
                                         xpsInterfaceId_t intfId, uint32_t l2EcmpSize)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;
    xpsInterfaceInfo_t *info = NULL;
    xpsInterfaceInfo_t intfInfoKey;
    intfInfoKey.keyIntfId = intfId;

    if ((status = xpsStateSearchData(scopeId, intfDbHndl, (xpsDbKey_t)&intfInfoKey,
                                     (void**)&info)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Search data failed, interface(%d)", intfId);
        return status;
    }
    if (!info)
    {
        status = XP_ERR_NOT_FOUND;
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Info is invalid for intfId:%d", intfId);
        return status;
    }

    info->l2EcmpSize = l2EcmpSize;

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsInterfaceGetL2EcmpSize(xpsInterfaceId_t intfId,
                                    uint32_t *l2EcmpSize)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;

    status = xpsInterfaceGetL2EcmpSizeScope(XP_SCOPE_DEFAULT, intfId, l2EcmpSize);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Getting Interface L2 Ecmp Size failed");
        return status;
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsInterfaceGetL2EcmpSizeScope(xpsScope_t scopeId,
                                         xpsInterfaceId_t intfId, uint32_t *l2EcmpSize)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;
    xpsInterfaceInfo_t *info = NULL;
    xpsInterfaceInfo_t intfInfoKey;
    intfInfoKey.keyIntfId = intfId;

    if ((status = xpsStateSearchData(scopeId, intfDbHndl, (xpsDbKey_t)&intfInfoKey,
                                     (void**)&info)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Search data failed, interface(%d)", intfId);
        return status;
    }
    if (!info)
    {
        status = XP_ERR_NOT_FOUND;
        return status;
    }

    *l2EcmpSize = info->l2EcmpSize;

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsInterfaceSetIngressPorts(xpsDevice_t devId,
                                      xpsInterfaceId_t intfId, xpsPortList_t *portList)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsInterfaceSetEgressPorts(xpsDevice_t devId, xpsInterfaceId_t intfId,
                                     xpsPortList_t *portList)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsInterfaceSetPorts(xpsDevice_t devId, xpsInterfaceId_t intfId,
                               xpsPortList_t *portList)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsInterfaceGetIngressPorts(xpsDevice_t devId,
                                      xpsInterfaceId_t intfId, xpsPortList_t *portList)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsInterfaceGetPorts(xpsDevice_t devId, xpsInterfaceId_t intfId,
                               xpsPortList_t *portList)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}


XP_STATUS xpsInterfaceGetFirst(xpsInterfaceId_t *intfId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;

    status =  xpsInterfaceGetFirstScope(XP_SCOPE_DEFAULT, intfId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Getting first interface ID  failed");
        return status;
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsInterfaceGetFirstScope(xpsScope_t scopeId,
                                    xpsInterfaceId_t *intfId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;
    xpsInterfaceInfo_t *info = NULL;

    if (intfId == NULL)
    {
        return XP_ERR_NULL_POINTER;
    }

    /* Get first interface entry from the list */
    if ((status = xpsStateGetNextData(scopeId, intfDbHndl, (xpsDbKey_t)NULL,
                                      (void**)&info)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get interface entry from list failed");
        return status;
    }

    /* interface context found ? */
    if (info == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Interface does not exist");
        status = XP_ERR_NOT_FOUND;
        return status;
    }

    *intfId = info->keyIntfId;

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsInterfaceGetNext(xpsInterfaceId_t intfId,
                              xpsInterfaceId_t *intfIdNext)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;

    status = xpsInterfaceGetNextScope(XP_SCOPE_DEFAULT, intfId, intfIdNext);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Getting next interface  failed");
        return status;
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsInterfaceGetNextScope(xpsScope_t scopeId, xpsInterfaceId_t intfId,
                                   xpsInterfaceId_t *intfIdNext)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;
    xpsInterfaceInfo_t *info = NULL;
    xpsInterfaceInfo_t intfInfoKey;

    if (intfIdNext == NULL)
    {
        return XP_ERR_NULL_POINTER;
    }

    intfInfoKey.keyIntfId = intfId;

    /* get next interface entry from the list */
    if ((status = xpsStateGetNextData(scopeId, intfDbHndl, (xpsDbKey_t)&intfInfoKey,
                                      (void**)&info)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get interface entry from list failed");
        return status;
    }

    /* interface context found ? */
    if (info == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Interface does not exist, interface(%d)", intfId);
        status = XP_ERR_NOT_FOUND;
        return status;
    }

    *intfIdNext = info->keyIntfId;

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsInterfaceGetFirstInfo(xpsScope_t scopeId,
                                   xpsInterfaceInfo_t **info)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;

    /* Get first interface entry from the list */
    if ((status = xpsStateGetNextData(scopeId, intfDbHndl, (xpsDbKey_t)NULL,
                                      (void**)info)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get interface entry from list failed");
        return status;
    }

    /* interface context found ? */
    if (*info == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Interface does not exist");
        status = XP_ERR_NOT_FOUND;
        return status;
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsInterfaceGetNextInfo(xpsScope_t scopeId, xpsInterfaceId_t intfId,
                                  xpsInterfaceInfo_t **info)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;
    xpsInterfaceInfo_t intfInfoKey;

    intfInfoKey.keyIntfId = intfId;

    /* get next interface entry from the list */
    if ((status = xpsStateGetNextData(scopeId, intfDbHndl, (xpsDbKey_t)&intfInfoKey,
                                      (void**)info)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get interface entry from list failed");
        return status;
    }

    /* interface context found ? */
    if (*info == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Interface does not exist, interface(%d)", intfId);
        status = XP_ERR_NOT_FOUND;
        return status;
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsInterfaceAddToDB(xpsInterfaceInfo_t *info)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;

    status = xpsInterfaceAddToDBScope(XP_SCOPE_DEFAULT, info);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Adding to DB  failed");
        return status;
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsInterfaceAddToDBScope(xpsScope_t scopeId, xpsInterfaceInfo_t *info)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;

    status = xpsStateInsertData(scopeId, intfDbHndl, (void*)info);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Insert data  failed");
        return status;
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsInterfaceRemoveFromDB(xpsInterfaceId_t intfId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;

    status = xpsInterfaceRemoveFromDBScope(XP_SCOPE_DEFAULT, intfId);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Remove from DB failed");
        return status;
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsInterfaceRemoveFromDBScope(xpsScope_t scopeId,
                                        xpsInterfaceId_t intfId)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;
    xpsInterfaceInfo_t *info;
    xpsInterfaceInfo_t intfInfoKey;
    intfInfoKey.keyIntfId = intfId;

    status =  xpsStateDeleteData(scopeId, intfDbHndl, (xpsDbKey_t)&intfInfoKey,
                                 (void**)&info);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Remove from DB Failed");
        return status;
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsInterfaceStripIncomingEtag(xpsDevice_t devId,  uint32_t vifId,
                                        uint8_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsInterfaceGetIpMtuProfileIndex(xpsInterfaceId_t intfId,
                                           uint32_t *ipMtuProfileIndex)
{
    XPS_FUNC_ENTRY_LOG();

    XP_STATUS status = XP_NO_ERR;
    xpsInterfaceInfo_t *intfInfo;

    if ((status = xpsInterfaceGetInfo(intfId, &intfInfo)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Get interfaceType for interface(%d) failed:%d", intfId, status);
        return status;
    }

    if (intfInfo == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get interface information for L3 Interface id : %d\n", intfId);
        return status;
    }

    *ipMtuProfileIndex = intfInfo->mtuPorfileIndex;

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsIntfChildAdd(xpsInterfaceInfo_t *info,
                          xpsInterfaceId_t intfId,
                          xpsInterfaceType_e type,
                          uint32_t hwId)
{
    XP_STATUS           status      = XP_NO_ERR;
    xpsInterfaceChildInfo_t* childInfo = NULL;

    if (!info)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Null object(%d) failed", intfId);
        return XP_ERR_NULL_POINTER;
    }

    // Create a new child Interface Info structure
    if ((status = xpsStateHeapMalloc(sizeof(xpsInterfaceChildInfo_t),
                                     (void**)&childInfo)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Could not allocate interface(%d) failed", intfId);
        return status;
    }

    memset(childInfo, 0, sizeof(xpsInterfaceChildInfo_t));
    childInfo->keyIntfId = intfId;
    childInfo->type = type;
    childInfo->hwId = hwId;

    // Add a new node
    status = xpsRBTAddNode(info->rbtree, (void*)childInfo);
    if ((status != XP_NO_ERR) && (status != XP_ERR_KEY_EXISTS))
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Add node failed");
        return status;
    }

    return status;
}

XP_STATUS xpsIntfChildRemove(xpsInterfaceInfo_t *info, xpsInterfaceId_t intfId)
{
    XP_STATUS           status  = XP_NO_ERR;
    xpsInterfaceChildInfo_t* childInfo = NULL;
    xpsInterfaceChildInfo_t childKey;

    if (!info)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Null object(%d) failed", intfId);
        return XP_ERR_NULL_POINTER;
    }

    childKey.keyIntfId = intfId;

    childInfo = (xpsInterfaceChildInfo_t*)xpsRBTDeleteNode(info->rbtree,
                                                           (xpsDbKey_t)&childKey);

    // Remove the corresponding state
    if (childInfo == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_WARNING,
              "Ket not found(%d) failed", intfId);
        return XP_ERR_NOT_FOUND;
    }

    // Free the memory allocated for the corresponding state
    if ((status = xpsStateHeapFree((void *)childInfo)) != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "failed for free (%d) failed", intfId);
        return status;
    }

    return status;

}

XP_STATUS xpsIntfChildFind(xpsInterfaceInfo_t *info, xpsInterfaceId_t intfId,
                           xpsInterfaceChildInfo_t **ppInfo)
{
    XP_STATUS      status   = XP_NO_ERR;
    xpsInterfaceChildInfo_t childKey;

    if (!info)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Null object(%d) failed", intfId);
        return XP_ERR_NULL_POINTER;
    }
    childKey.keyIntfId = intfId;

    *ppInfo = (xpsInterfaceChildInfo_t*)xpsRBTSearchNode(info->rbtree,
                                                         (xpsDbKey_t)&childKey);

    if (*ppInfo == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Ket not found(%d) failed", intfId);
        return XP_ERR_KEY_NOT_FOUND;
    }

    return status;
}

XP_STATUS xpsIntfChildGetNext(xpsInterfaceInfo_t *info,
                              xpsInterfaceChildInfo_t **nextInfo)
{
    if (!info)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Null object failed");
        return XP_ERR_NULL_POINTER;
    }

    *nextInfo = (xpsInterfaceChildInfo_t*)xpsRBTGetInorderSuccessor(info->rbtree,
                                                                    (xpsDbKey_t)*nextInfo);

    if (*nextInfo == NULL)
    {
        return XP_ERR_NULL_POINTER;
    }
    return XP_NO_ERR;
}

#ifdef __cplusplus
}
#endif
