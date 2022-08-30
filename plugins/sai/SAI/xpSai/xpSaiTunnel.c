// xpSaiTunnel.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpSaiTunnel.h"
#include "xpsIpinIp.h"
#include "xpsVxlan.h"
#include "xpSaiValidationArrays.h"
#include "xpsAllocator.h"
#include "xpSaiStub.h"

XP_SAI_LOG_REGISTER_API(SAI_API_TUNNEL);
#define SAI_TUNNEL_MAP_OBJID_TYPE_BITS       (32)
#define SAI_TUNNEL_MAP_OBJID_TYPE_MASK       (0x000000FF00000000ULL)

#define XP_SAI_TUNNEL_MAX_IDS                (1024)          ///< Max number of tunnel IDs
#define XP_SAI_TUNNEL_RANGE_START            (0)
#define XP_SAI_TUNNEL_TERM_ENTRY_MAX_IDS     (1024)
#define XP_SAI_TUNNEL_TERM_ENTRY_RANGE_START (0)
#define XP_SAI_TUNNEL_MAP_MAX_IDS            (6 * 4 * 1024) ///< Max number of tunnel map IDs (Num of map type * 4K VNI)
#define XP_SAI_TUNNEL_MAP_RANGE_START        (0)
#define XP_SAI_TUNNEL_MAP_ENTRY_MAX_IDS      (XP_SAI_TUNNEL_MAP_MAX_IDS)
#define XP_SAI_TUNNEL_MAP_ENTRY_RANGE_START  (0)
#define XP_SAI_TUNNEL_MAP_ENTRY_ATTR_VNI_ID_MIN  0
#define XP_SAI_TUNNEL_MAP_ENTRY_ATTR_VNI_ID_MAX  0xFFFFFF

static sai_tunnel_api_t* _xpSaiTunnelApi;

static xpsDbHandle_t xpSaiTunnelStateDbHndl          =
    XPS_STATE_INVALID_DB_HANDLE;
static xpsDbHandle_t xpSaiTunnelTermEntryStateDbHndl =
    XPS_STATE_INVALID_DB_HANDLE;
static xpsDbHandle_t xpSaiTunnelMapStateDbHndl       =
    XPS_STATE_INVALID_DB_HANDLE;
static xpsDbHandle_t xpSaiTunnelMapEntryStateDbHndl  =
    XPS_STATE_INVALID_DB_HANDLE;
static xpsDbHandle_t xpSaiTunnelBridgeStateDbHndl    =
    XPS_STATE_INVALID_DB_HANDLE;
extern xpsDbHandle_t xpSaiSwitchStaticDataDbHandle;
/**
 * @brief SAI Tunnel Map Entry attributes
 */
typedef struct _xpSaiTunnelMapEntryAttributesT
{
    sai_attribute_value_t mapType;
    sai_attribute_value_t mapId;
    sai_attribute_value_t oecnKey;
    sai_attribute_value_t oecnVal;
    sai_attribute_value_t uecnKey;
    sai_attribute_value_t uecnVal;
    sai_attribute_value_t vlanIdKey;
    sai_attribute_value_t vlanIdVal;
    sai_attribute_value_t vniIdKey;
    sai_attribute_value_t vniIdVal;
    sai_attribute_value_t bridgeIdKey;
    sai_attribute_value_t bridgeIdVal;
    sai_attribute_value_t vrfIdKey;
    sai_attribute_value_t vrfIdVal;
} xpSaiTunnelMapEntryAttributesT;

/**
 * @brief SAI Tunnel Map Entry Data Base Entry struct
 */
typedef struct _xpSaiTunnelMapEntryDbEntry
{
    /* Key */
    uint32_t        tnlMapEntryId; ///< Tunnel map entry identifier

    /* Context */
    int32_t         mapType;       ///< Tunnel map type
    sai_object_id_t mapId;         ///< Tunnel map identifier
    uint8_t         oecnKey;       ///< Inner ECN key
    uint8_t         oecnVal;       ///< Inner ECN value
    uint8_t         uecnKey;       ///< Outer ECN key
    uint8_t         uecnVal;       ///< Outer ECN value
    uint16_t        vlanIdKey;     ///< Vlan ID key
    uint16_t        vlanId;        ///< Vlan ID value
    uint32_t        vniIdKey;      ///< VNI ID key
    uint32_t        vniId;         ///< VNI ID value
    sai_object_id_t bridgeIdKey;   ///< Bridge ID key
    sai_object_id_t bridgeId;      ///< Bridge ID value
    sai_object_id_t vrfIdKey;      ///< Virtual Router ID key
    sai_object_id_t vrfId;         ///< Virtual Router ID value

    bool            isConfgInHW;  // Set/clear HW prog status.
    uint32_t        refCnt;       // Tunnels using this entry.
} xpSaiTunnelMapEntryInfoT;

/**
 * @brief SAI Tunnel Map attributes
 */
typedef struct _xpSaiTunnelMapAttributesT
{
    sai_attribute_value_t mapType;
    sai_attribute_value_t mapEntryList;
} xpSaiTunnelMapAttributesT;

/**
 * @brief SAI Tunnel Map Data Base Entry struct
 */
typedef struct _xpSaiTunnelMapDbEntry
{
    /* Key */
    uint32_t        tnlMapId;                            ///< Tunnel map identifier

    /* Context */
    int32_t         mapType;                             ///< Tunnel map type
    uint32_t
    mapEntryCnt;                         ///< Tunnel map entries count
    sai_object_id_t
    mapEntryId[XP_SAI_TUNNEL_THRESHOLD]; ///< Tunnel map entries associated with map
} xpSaiTunnelMapInfoT;

/**
 * @brief SAI Tunnel attributes
 */
typedef struct _xpSaiTunnelAttributesT
{
    sai_attribute_value_t tunnelType;
    sai_attribute_value_t underlayIntfId;
    sai_attribute_value_t overlayIntfId;
    sai_attribute_value_t peerMode;
    sai_attribute_value_t encapSrcIp;
    sai_attribute_value_t encapDstIp;
    sai_attribute_value_t encapTtlMode;
    sai_attribute_value_t encapTtlVal;
    sai_attribute_value_t encapDscpMode;
    sai_attribute_value_t encapDscpVal;
    sai_attribute_value_t encapGreKeyValid;
    sai_attribute_value_t encapGreKey;
    sai_attribute_value_t encapEcnMode;
    sai_attribute_value_t encapMappers;
    sai_attribute_value_t decapEcnMode;
    sai_attribute_value_t decapMappers;
    sai_attribute_value_t decapTtlMode;
    sai_attribute_value_t decapDscpMode;
    sai_attribute_value_t udpSrcPortMode;
    sai_attribute_value_t udpSrcPort;
    sai_attribute_value_t termTblEntryList;
} xpSaiTunnelAttributesT;

/**
 * @brief SAI Tunnel Bridge Port struct
 */
typedef struct _xpSaiTnlBrPort
{
    sai_object_id_t  brPortId;              ///< Bridge Port associated with tunnel
    sai_object_id_t  brId;                  ///< Bridge ID associated with tunnel
    bool
    isTnlOnBrPortAdded;    ///< Set to true when tunnel added to bridge port
    bool
    isTnlBrPortL2McGroupMbr; ///< Set to true when tunnel bridge port becomes Mcast group member
    sai_ip_address_t tnlEndpointIp;         ///< Tunnel Endpoint IP
    sai_object_id_t
    l2McGroupId;             ///< Mcast group ID associated with tunnel Bridge port
} xpSaiTnlBrPort;

/**
 * @brief SAI Tunnel Data Base Entry struct
 */

typedef struct _xpSaiTunnelBridgeDbEntry
{
    /* Key */
    uint32_t
    tunnelId;                                    ///< Tunnel identifier
    /* Context */
    uint32_t
    brPortCnt;                                   ///< Count of Bridge Ports associated with tunnel
    xpSaiTnlBrPort
    tnlBrPortList[XP_SAI_TUNNEL_THRESHOLD];      ///< Bridge Ports associated with tunnel
} xpSaiTunnelBridgeInfoT;


/**
 * @brief SAI Tunnel Termination Table attributes
 */
typedef struct _xpSaiTunnelTermTableEntryAttributesT
{
    sai_attribute_value_t vrfId;
    sai_attribute_value_t entryType;
    sai_attribute_value_t dstIpAddr;
    sai_attribute_value_t srcIpAddr;
    sai_attribute_value_t tunnelType;
    sai_attribute_value_t tunnelId;
} xpSaiTunnelTermTableEntryAttributesT;

/**
 * @brief SAI Tunnel Termination Table Entry Data Base Entry struct
 */

typedef struct _xpSaiTunnelEntryDbEntry
{
    /* Key */
    uint32_t         tnlEntryId; ///< Tunnel termination table entry identifier

    /* Context */
    sai_object_id_t  vrfId;      ///< Virtual router Id
    int32_t          entryType;  ///< Tunnel Entry type
    sai_ip_address_t dstIpAddr;  ///< Tunnel termination IP address
    uint32_t         nextHopId;  ///< Tunnel Next Hop ID
    xpsL3NextHopEntry_t nextHopEntry;   ///< Tunnel Next Hop
    sai_ip_address_t srcIpAddr;  ///< Tunnel source IP address
    int32_t          tnlType;    ///< Tunnel type
    sai_object_id_t  tnlId;      ///< Tunnel id to be use for decap
    xpsInterfaceId_t tnlIntfId;  ///< XPS Tunnel interface id
    uint32_t         count;
} xpSaiTunnelEntryInfoT;

static sai_status_t xpSaiVxlanTunnelCreate(xpsDevice_t xpsDevId,
                                           sai_object_id_t tunnelId);

static sai_status_t xpSaiVxlanTunnelRemove(xpsDevice_t xpsDevId,
                                           sai_object_id_t tunnelId);
//Func: xpSaiTunnelCtxKeyComp

static int32_t xpSaiTunnelCtxKeyComp(void *key1, void *key2)
{
    return ((((xpSaiTunnelInfoT*)key1)->tunnelId) - (((xpSaiTunnelInfoT*)
                                                      key2)->tunnelId));
}

//Func: xpSaiTunnelBridgeCtxKeyComp
static int32_t xpSaiTunnelBridgeCtxKeyComp(void *key1, void *key2)
{
    return ((((xpSaiTunnelBridgeInfoT*)key1)->tunnelId) - (((
                                                                xpSaiTunnelBridgeInfoT*)key2)->tunnelId));
}

//Func: xpSaiTunnelEntryCtxKeyComp

static int32_t xpSaiTunnelEntryCtxKeyComp(void *key1, void *key2)
{
    return ((((xpSaiTunnelEntryInfoT*)key1)->tnlEntryId) - (((
                                                                 xpSaiTunnelEntryInfoT*)key2)->tnlEntryId));
}

//Func: xpSaiTunnelMapCtxKeyComp

static int32_t xpSaiTunnelMapCtxKeyComp(void *key1, void *key2)
{
    return ((((xpSaiTunnelMapInfoT*)key1)->tnlMapId) - (((xpSaiTunnelMapInfoT*)
                                                         key2)->tnlMapId));
}

//Func: xpSaiTunnelMapEntryCtxKeyComp

static int32_t xpSaiTunnelMapEntryCtxKeyComp(void *key1, void *key2)
{
    return ((((xpSaiTunnelMapEntryInfoT*)key1)->tnlMapEntryId) - (((
                                                                       xpSaiTunnelMapEntryInfoT*)key2)->tnlMapEntryId));
}

//Func: xpSaiTunnelGetCtxDb

XP_STATUS xpSaiTunnelGetCtxDb(xpsDevice_t xpsDevId,
                              sai_object_id_t tunnelId, xpSaiTunnelInfoT **pTunnelEntry)
{
    XP_STATUS        retVal   = XP_NO_ERR;
    xpsScope_t       scope    = xpSaiScopeFromDevGet(xpsDevId);
    uint32_t         xpsTnlId = xpSaiObjIdValueGet(tunnelId);
    xpSaiTunnelInfoT key;

    if (NULL == pTunnelEntry)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return XP_ERR_NULL_POINTER;
    }

    memset(&key, 0, sizeof(xpSaiTunnelInfoT));
    key.tunnelId = xpsTnlId;

    retVal = xpsStateSearchData(scope, xpSaiTunnelStateDbHndl, (xpsDbKey_t)&key,
                                (void**)pTunnelEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateSearchData: Search data failed, tunnelId: %d | error: %d.\n",
                       xpsTnlId, retVal);
        return retVal;
    }

    if (!(*pTunnelEntry))
    {
        XP_SAI_LOG_ERR("Tunnel does not exist.\n");
        return XP_ERR_KEY_NOT_FOUND;
    }

    return retVal;
}

//fn xpSaiTunnelBridgeInfoGetCtxDb
static XP_STATUS xpSaiTunnelBridgeInfoGetCtxDb(xpsDevice_t xpsDevId,
                                               sai_object_id_t tunnelId, xpSaiTunnelBridgeInfoT **pTunnelBridgeEntry)
{
    XP_STATUS        retVal   = XP_NO_ERR;
    xpsScope_t       scope    = xpSaiScopeFromDevGet(xpsDevId);
    uint32_t         xpsTnlId = xpSaiObjIdValueGet(tunnelId);
    xpSaiTunnelBridgeInfoT key;

    if (NULL == pTunnelBridgeEntry)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return XP_ERR_NULL_POINTER;
    }

    memset(&key, 0, sizeof(xpSaiTunnelBridgeInfoT));
    key.tunnelId = xpsTnlId;

    retVal = xpsStateSearchData(scope, xpSaiTunnelBridgeStateDbHndl,
                                (xpsDbKey_t)&key, (void**)pTunnelBridgeEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateSearchData: Search data failed, tunnelId: %d | error: %d.\n",
                       xpsTnlId, retVal);
        return retVal;
    }

    if (!(*pTunnelBridgeEntry))
    {
        XP_SAI_LOG_ERR("Tunnel does not exist.\n");
        return XP_ERR_KEY_NOT_FOUND;
    }

    return retVal;
}
//Func: xpSaiTunnelEntryGetCtxDb

static XP_STATUS xpSaiTunnelEntryGetCtxDb(xpsDevice_t xpsDevId,
                                          sai_object_id_t tnlEntryId, xpSaiTunnelEntryInfoT **pTnlTermEntry)
{
    XP_STATUS             retVal        = XP_NO_ERR;
    xpsScope_t            scope         = xpSaiScopeFromDevGet(xpsDevId);
    uint32_t              xpsTnlEntryId = xpSaiObjIdValueGet(tnlEntryId);
    xpSaiTunnelEntryInfoT key;

    if (NULL == pTnlTermEntry)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return XP_ERR_NULL_POINTER;
    }

    memset(&key, 0, sizeof(xpSaiTunnelEntryInfoT));
    key.tnlEntryId = xpsTnlEntryId;

    retVal = xpsStateSearchData(scope, xpSaiTunnelTermEntryStateDbHndl,
                                (xpsDbKey_t)&key, (void**)pTnlTermEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateSearchData: Search data failed, tnlEntryId: %d | error: %d.\n",
                       xpsTnlEntryId, retVal);
        return retVal;
    }

    if (!(*pTnlTermEntry))
    {
        XP_SAI_LOG_ERR("Tunnel termination table entry does not exist.\n");
        return XP_ERR_KEY_NOT_FOUND;
    }

    return retVal;
}

//Func: xpSaiTunnelMapGetCtxDb

static XP_STATUS xpSaiTunnelMapGetCtxDb(xpsDevice_t xpsDevId,
                                        sai_object_id_t tnlMapId, xpSaiTunnelMapInfoT **pTnlMapEntry)
{
    XP_STATUS           retVal      = XP_NO_ERR;
    xpsScope_t          scope       = xpSaiScopeFromDevGet(xpsDevId);
    uint32_t            xpsTnlMapId = xpSaiObjIdValueGet(tnlMapId);
    xpSaiTunnelMapInfoT key;

    if (NULL == pTnlMapEntry)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return XP_ERR_NULL_POINTER;
    }

    memset(&key, 0, sizeof(xpSaiTunnelMapInfoT));
    key.tnlMapId = xpsTnlMapId;

    retVal = xpsStateSearchData(scope, xpSaiTunnelMapStateDbHndl, (xpsDbKey_t)&key,
                                (void**)pTnlMapEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateSearchData: Search data failed, tnlMapId: %d | error: %d.\n",
                       xpsTnlMapId, retVal);
        return retVal;
    }

    if (!(*pTnlMapEntry))
    {
        XP_SAI_LOG_ERR("Tunnel map does not exist.\n");
        return XP_ERR_KEY_NOT_FOUND;
    }

    return retVal;
}

//Func: xpSaiTunnelMapEntryGetCtxDb

static XP_STATUS xpSaiTunnelMapEntryGetCtxDb(xpsDevice_t xpsDevId,
                                             sai_object_id_t tnlMapEntryId, xpSaiTunnelMapEntryInfoT **pTnlMapEntry)
{
    XP_STATUS                retVal           = XP_NO_ERR;
    xpsScope_t               scope            = xpSaiScopeFromDevGet(xpsDevId);
    uint32_t                 xpsTnlMapEntryId = xpSaiObjIdValueGet(tnlMapEntryId);
    xpSaiTunnelMapEntryInfoT key;

    if (NULL == pTnlMapEntry)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return XP_ERR_NULL_POINTER;
    }

    memset(&key, 0, sizeof(xpSaiTunnelMapEntryInfoT));
    key.tnlMapEntryId = xpsTnlMapEntryId;

    retVal = xpsStateSearchData(scope, xpSaiTunnelMapEntryStateDbHndl,
                                (xpsDbKey_t)&key, (void**)pTnlMapEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateSearchData: Search data failed, tnlMapEntryId: %d | error: %d.\n",
                       xpsTnlMapEntryId, retVal);
        return retVal;
    }

    if (!(*pTnlMapEntry))
    {
        XP_SAI_LOG_ERR("Tunnel Map Entry does not exist.\n");
        return XP_ERR_KEY_NOT_FOUND;
    }

    return retVal;
}

//Func: xpSaiTunnelInsertCtxDb

static XP_STATUS xpSaiTunnelInsertCtxDb(xpsDevice_t xpsDevId,
                                        sai_object_id_t tunnelId, xpSaiTunnelInfoT **pTunnelEntry)
{
    xpsScope_t scope    = xpSaiScopeFromDevGet(xpsDevId);
    uint32_t   xpsTnlId = xpSaiObjIdValueGet(tunnelId);
    XP_STATUS  retVal   = XP_NO_ERR;

    if (NULL == pTunnelEntry)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return XP_ERR_NULL_POINTER;
    }

    retVal = xpsStateHeapMalloc(sizeof(xpSaiTunnelInfoT), (void**)pTunnelEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateHeapMalloc: XPS state Heap Allocation failed, error: %d.\n",
                       retVal);
        return retVal;
    }

    memset(*pTunnelEntry, 0, sizeof(xpSaiTunnelInfoT));

    (*pTunnelEntry)->tunnelId = xpsTnlId;

    /* Insert the Tunnel context into the database, tunnelId is the key */
    retVal = xpsStateInsertData(scope, xpSaiTunnelStateDbHndl,
                                (void*)*pTunnelEntry);
    if (retVal != XP_NO_ERR)
    {
        xpsStateHeapFree((void*)*pTunnelEntry);
        *pTunnelEntry = NULL;
        XP_SAI_LOG_ERR("xpsStateInsertData: Data insertion failed, error: %d.\n",
                       retVal);
        return retVal;
    }

    return retVal;
}

//Func: xpSaiTunnelBridgeInfoInsertCtxDb

static XP_STATUS xpSaiTunnelBridgeInfoInsertCtxDb(xpsDevice_t xpsDevId,
                                                  sai_object_id_t tunnelId, xpSaiTunnelBridgeInfoT **pTunnelBridgeEntry)
{
    xpsScope_t scope    = xpSaiScopeFromDevGet(xpsDevId);
    uint32_t   xpsTnlId = xpSaiObjIdValueGet(tunnelId);
    XP_STATUS  retVal   = XP_NO_ERR;

    if (NULL == pTunnelBridgeEntry)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return XP_ERR_NULL_POINTER;
    }

    retVal = xpsStateHeapMalloc(sizeof(xpSaiTunnelBridgeInfoT),
                                (void**)pTunnelBridgeEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateHeapMalloc: XPS state Heap Allocation failed, error: %d.\n",
                       retVal);
        return retVal;
    }

    memset(*pTunnelBridgeEntry, 0, sizeof(xpSaiTunnelBridgeInfoT));

    (*pTunnelBridgeEntry)->tunnelId = xpsTnlId;

    /* Insert the Tunnel context into the database, tunnelId is the key */
    retVal = xpsStateInsertData(scope, xpSaiTunnelBridgeStateDbHndl,
                                (void*)*pTunnelBridgeEntry);
    if (retVal != XP_NO_ERR)
    {
        xpsStateHeapFree((void*)*pTunnelBridgeEntry);
        *pTunnelBridgeEntry = NULL;
        if (retVal != XP_ERR_KEY_EXISTS)
        {
            XP_SAI_LOG_ERR("xpsStateInsertData: Data insertion failed, error: %d.\n",
                           retVal);
        }
        return retVal;
    }

    return retVal;
}

static XP_STATUS xpSaiTunnelEntryInsertCtxDb(xpsDevice_t xpsDevId,
                                             sai_object_id_t tnlEntryId, xpSaiTunnelEntryInfoT **pTnlTermEntry)
{
    xpsScope_t scope         = xpSaiScopeFromDevGet(xpsDevId);
    uint32_t   xpsTnlEntryId = xpSaiObjIdValueGet(tnlEntryId);
    XP_STATUS  retVal        = XP_NO_ERR;

    if (NULL == pTnlTermEntry)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return XP_ERR_NULL_POINTER;
    }

    retVal = xpsStateHeapMalloc(sizeof(xpSaiTunnelEntryInfoT),
                                (void**)pTnlTermEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateHeapMalloc: XPS state Heap Allocation failed, error: %d.\n",
                       retVal);
        return retVal;
    }

    memset(*pTnlTermEntry, 0, sizeof(xpSaiTunnelEntryInfoT));

    (*pTnlTermEntry)->tnlEntryId = xpsTnlEntryId;

    /* Insert the Tunnel termination table entry context into the database, tnlEntryId is the key */
    retVal = xpsStateInsertData(scope, xpSaiTunnelTermEntryStateDbHndl,
                                (void*)*pTnlTermEntry);
    if (retVal != XP_NO_ERR)
    {
        xpsStateHeapFree((void*)*pTnlTermEntry);
        *pTnlTermEntry = NULL;
        XP_SAI_LOG_ERR("xpsStateInsertData: Data insertion failed, error: %d.\n",
                       retVal);
        return retVal;
    }

    return retVal;
}

//Func: xpSaiTunnelMapInsertCtxDb

static XP_STATUS xpSaiTunnelMapInsertCtxDb(xpsDevice_t xpsDevId,
                                           sai_object_id_t tnlMapId, xpSaiTunnelMapInfoT **pTnlMapEntry)
{
    xpsScope_t scope       = xpSaiScopeFromDevGet(xpsDevId);
    uint32_t   xpsTnlMapId = xpSaiObjIdValueGet(tnlMapId);
    XP_STATUS  retVal      = XP_NO_ERR;

    if (NULL == pTnlMapEntry)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return XP_ERR_NULL_POINTER;
    }

    retVal = xpsStateHeapMalloc(sizeof(xpSaiTunnelMapInfoT), (void**)pTnlMapEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateHeapMalloc: XPS state Heap Allocation failed, error: %d.\n",
                       retVal);
        return retVal;
    }

    memset(*pTnlMapEntry, 0, sizeof(xpSaiTunnelMapInfoT));

    (*pTnlMapEntry)->tnlMapId = xpsTnlMapId;

    /* Insert the Tunnel Map context into the database, tnlMapId is the key */
    retVal = xpsStateInsertData(scope, xpSaiTunnelMapStateDbHndl,
                                (void*)*pTnlMapEntry);
    if (retVal != XP_NO_ERR)
    {
        xpsStateHeapFree((void*)*pTnlMapEntry);
        *pTnlMapEntry = NULL;
        XP_SAI_LOG_ERR("xpsStateInsertData: Data insertion failed, error: %d.\n",
                       retVal);
        return retVal;
    }

    return retVal;
}

//Func: xpSaiTunnelMapEntryInsertCtxDb

static XP_STATUS xpSaiTunnelMapEntryInsertCtxDb(xpsDevice_t xpsDevId,
                                                sai_object_id_t tnlMapEntryId, xpSaiTunnelMapEntryInfoT **pTnlMapEntry)
{
    xpsScope_t scope            = xpSaiScopeFromDevGet(xpsDevId);
    uint32_t   xpsTnlMapEntryId = xpSaiObjIdValueGet(tnlMapEntryId);
    XP_STATUS  retVal           = XP_NO_ERR;

    if (NULL == pTnlMapEntry)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return XP_ERR_NULL_POINTER;
    }

    retVal = xpsStateHeapMalloc(sizeof(xpSaiTunnelMapEntryInfoT),
                                (void**)pTnlMapEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateHeapMalloc: XPS state Heap Allocation failed, error: %d.\n",
                       retVal);
        return retVal;
    }

    memset(*pTnlMapEntry, 0, sizeof(xpSaiTunnelMapEntryInfoT));

    (*pTnlMapEntry)->tnlMapEntryId = xpsTnlMapEntryId;

    /* Insert the Tunnel Map Entry context into the database, tnlMapEntryId is the key */
    retVal = xpsStateInsertData(scope, xpSaiTunnelMapEntryStateDbHndl,
                                (void*)*pTnlMapEntry);
    if (retVal != XP_NO_ERR)
    {
        xpsStateHeapFree((void*)*pTnlMapEntry);
        *pTnlMapEntry = NULL;
        XP_SAI_LOG_ERR("xpsStateInsertData: Data insertion failed, error: %d.\n",
                       retVal);
        return retVal;
    }

    return retVal;
}

//Func: xpSaiTunnelDeleteCtxDb

static XP_STATUS xpSaiTunnelDeleteCtxDb(xpsDevice_t xpsDevId,
                                        sai_object_id_t tunnelId)
{
    XP_STATUS         retVal       = XP_NO_ERR;
    xpsScope_t        scope        = xpSaiScopeFromDevGet(xpsDevId);
    uint32_t          xpsTnlId     = xpSaiObjIdValueGet(tunnelId);
    xpSaiTunnelInfoT *pTunnelEntry = NULL;
    xpSaiTunnelInfoT  key;

    memset(&key, 0, sizeof(xpSaiTunnelInfoT));
    key.tunnelId = xpsTnlId;

    retVal = xpsStateDeleteData(scope, xpSaiTunnelStateDbHndl, (xpsDbKey_t)&key,
                                (void**)&pTunnelEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateDeleteData: Delete Tunnel data failed, tunnelId: %d | error: %d.\n",
                       xpsTnlId, retVal);
        return retVal;
    }

    retVal = xpsStateHeapFree((void*)pTunnelEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateHeapFree: XPS state Heap Free failed, error: %d.\n",
                       retVal);
        return retVal;
    }

    return retVal;
}

static XP_STATUS xpSaiTunnelBridgeInfoDeleteCtxDb(xpsDevice_t xpsDevId,
                                                  sai_object_id_t tunnelId)
{
    XP_STATUS         retVal       = XP_NO_ERR;
    xpsScope_t        scope        = xpSaiScopeFromDevGet(xpsDevId);
    uint32_t          xpsTnlId     = xpSaiObjIdValueGet(tunnelId);
    xpSaiTunnelBridgeInfoT *pTunnelBridgeEntry = NULL;
    xpSaiTunnelBridgeInfoT  key;

    memset(&key, 0, sizeof(xpSaiTunnelBridgeInfoT));
    key.tunnelId = xpsTnlId;

    retVal = xpsStateDeleteData(scope, xpSaiTunnelBridgeStateDbHndl,
                                (xpsDbKey_t)&key, (void**)&pTunnelBridgeEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateDeleteData: Delete Tunnel data failed, tunnelId: %d | error: %d.\n",
                       xpsTnlId, retVal);
        return retVal;
    }

    retVal = xpsStateHeapFree((void*)pTunnelBridgeEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateHeapFree: XPS state Heap Free failed, error: %d.\n",
                       retVal);
        return retVal;
    }

    return retVal;
}
//Func: xpSaiTunnelEntryDeleteCtxDb

static XP_STATUS xpSaiTunnelEntryDeleteCtxDb(xpsDevice_t xpsDevId,
                                             sai_object_id_t tnlEntryId)
{
    XP_STATUS              retVal        = XP_NO_ERR;
    xpsScope_t             scope         = xpSaiScopeFromDevGet(xpsDevId);
    uint32_t               xpsTnlEntryId = xpSaiObjIdValueGet(tnlEntryId);
    xpSaiTunnelEntryInfoT *pTnlTermEntry = NULL;
    xpSaiTunnelEntryInfoT  key;

    memset(&key, 0, sizeof(xpSaiTunnelEntryInfoT));
    key.tnlEntryId = xpsTnlEntryId;

    retVal = xpsStateDeleteData(scope, xpSaiTunnelTermEntryStateDbHndl,
                                (xpsDbKey_t)&key, (void**)&pTnlTermEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateDeleteData: Delete Tunnel termination table entry data failed, tnlEntryId: %d | error: %d.\n",
                       xpsTnlEntryId, retVal);
        return retVal;
    }

    retVal = xpsStateHeapFree((void*)pTnlTermEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateHeapFree: XPS state Heap Free failed, error: %d.\n",
                       retVal);
        return retVal;
    }

    return retVal;
}

//Func: xpSaiTunnelMapDeleteCtxDb

static XP_STATUS xpSaiTunnelMapDeleteCtxDb(xpsDevice_t xpsDevId,
                                           sai_object_id_t tnlMapId)
{
    XP_STATUS            retVal       = XP_NO_ERR;
    xpsScope_t           scope        = xpSaiScopeFromDevGet(xpsDevId);
    uint32_t             xpsTnlMapId  = xpSaiObjIdValueGet(tnlMapId);
    xpSaiTunnelMapInfoT *pTnlMapEntry = NULL;
    xpSaiTunnelMapInfoT  key;

    memset(&key, 0, sizeof(xpSaiTunnelMapInfoT));
    key.tnlMapId = xpsTnlMapId;

    retVal = xpsStateDeleteData(scope, xpSaiTunnelMapStateDbHndl, (xpsDbKey_t)&key,
                                (void**)&pTnlMapEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateDeleteData: Delete Tunnel data failed, tnlMapId: %d | error: %d.\n",
                       xpsTnlMapId, retVal);
        return retVal;
    }

    retVal = xpsStateHeapFree((void*)pTnlMapEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateHeapFree: XPS state Heap Free failed, error: %d.\n",
                       retVal);
        return retVal;
    }

    return retVal;
}

//Func: xpSaiTunnelMapEntryDeleteCtxDb

static XP_STATUS xpSaiTunnelMapEntryDeleteCtxDb(xpsDevice_t xpsDevId,
                                                sai_object_id_t tnlMapEntryId)
{
    XP_STATUS                 retVal           = XP_NO_ERR;
    xpsScope_t                scope            = xpSaiScopeFromDevGet(xpsDevId);
    uint32_t                  xpsTnlMapEntryId = xpSaiObjIdValueGet(tnlMapEntryId);
    xpSaiTunnelMapEntryInfoT *pTnlMapEntry     = NULL;
    xpSaiTunnelMapEntryInfoT  key;

    memset(&key, 0, sizeof(xpSaiTunnelMapEntryInfoT));
    key.tnlMapEntryId = xpsTnlMapEntryId;

    retVal = xpsStateDeleteData(scope, xpSaiTunnelMapEntryStateDbHndl,
                                (xpsDbKey_t)&key, (void**)&pTnlMapEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateDeleteData: Delete Tunnel data failed, tnlMapEntryId: %d | error: %d.\n",
                       xpsTnlMapEntryId, retVal);
        return retVal;
    }

    retVal = xpsStateHeapFree((void*)pTnlMapEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateHeapFree: XPS state Heap Free failed, error: %d.\n",
                       retVal);
        return retVal;
    }

    return retVal;
}

//Func: xpSaiTunnelIdAllocate

static XP_STATUS xpSaiTunnelIdAllocate(xpsDevice_t xpsDevId,
                                       xpSaiTunnelAttributesT *attributes, sai_object_id_t *tunnelId)
{
    XP_STATUS         retVal       = XP_NO_ERR;
    sai_status_t      saiRetVal    = SAI_STATUS_SUCCESS;
    uint32_t          tnlId        = 0;
    uint32_t          count        = 0;
    xpSaiTunnelInfoT *pTunnelEntry = NULL;
    xpsScope_t        scope        = xpSaiScopeFromDevGet(xpsDevId);

    XP_SAI_LOG_DBG("enter xpSaiTunnelIdAllocate\n");

    if ((NULL == attributes) || (NULL == tunnelId))
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return XP_ERR_NULL_POINTER;
    }

    retVal = xpsAllocatorAllocateId(scope, XP_SAI_ALLOC_TUNNEL, &tnlId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsAllocatorAllocateId: Unable to allocate Tunnel id, error: %d.\n",
                       retVal);
        return retVal;
    }

    saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_TUNNEL, xpsDevId,
                                 (sai_uint64_t)tnlId, tunnelId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiObjIdCreate: Unable to create SAI object, error: %d.\n",
                       saiRetVal);
        return XP_ERR_INVALID_ID;
    }

    retVal = xpSaiTunnelInsertCtxDb(xpsDevId, tnlId, &pTunnelEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiTunnelInsertCtxDb: Unable to allocate XPS state Heap, error: %d.\n",
                       retVal);
        return retVal;
    }

    pTunnelEntry->tunnelType = attributes->tunnelType.s32;
    pTunnelEntry->uLayIntfId = attributes->underlayIntfId.oid;
    pTunnelEntry->oLayIntfId = attributes->overlayIntfId.oid;
    pTunnelEntry->tnlSrcIp = attributes->encapSrcIp.ipaddr;
    pTunnelEntry->tnlDstIp = attributes->encapDstIp.ipaddr;
    pTunnelEntry->ecpTtl = attributes->encapTtlVal.s32;
    pTunnelEntry->ecpTtlMode = attributes->encapTtlMode.s32;
    pTunnelEntry->ecpDscp = attributes->encapDscpVal.s32;
    pTunnelEntry->ecpDscpMode = attributes->encapDscpMode.s32;
    pTunnelEntry->ecpEcnMode = attributes->encapEcnMode.s32;
    pTunnelEntry->dcpEcnMode = attributes->decapEcnMode.s32;
    pTunnelEntry->dcpTtlMode = attributes->decapTtlMode.s32;
    pTunnelEntry->dcpDscpMode = attributes->decapDscpMode.s32;
    pTunnelEntry->udpSrcPortMode = attributes->udpSrcPortMode.s32;
    pTunnelEntry->udpSrcPort = attributes->udpSrcPort.u16;
    pTunnelEntry->peerMode = attributes->peerMode.s32;


    pTunnelEntry->tnlGreKeyValid = attributes->encapGreKeyValid.booldata;

    if (pTunnelEntry->tnlGreKeyValid)
    {
        pTunnelEntry->tnlGreKey  = attributes->encapGreKey.u32;
    }

    pTunnelEntry->ecpMapListCnt = attributes->encapMappers.objlist.count;

    for (count = 0; count < pTunnelEntry->ecpMapListCnt; ++count)
    {
        pTunnelEntry->ecpMapList[count] = attributes->encapMappers.objlist.list[count];
    }

    pTunnelEntry->dcpMapListCnt = attributes->decapMappers.objlist.count;

    for (count = 0; count < pTunnelEntry->dcpMapListCnt; ++count)
    {
        pTunnelEntry->dcpMapList[count] = attributes->decapMappers.objlist.list[count];
    }
    /* TO_DO: Check the need for this. For now commenting*/
    //pTunnelEntry->inrSaMissCmd = XP_PKTCMD_FWD_MIRROR;
    return retVal;
}

//Func: xpSaiTunnelIdRelease

static XP_STATUS xpSaiTunnelIdRelease(xpsDevice_t xpsDevId,
                                      sai_object_id_t tunnelId)
{
    XP_STATUS         retVal       = XP_NO_ERR;
    xpSaiTunnelInfoT *pTunnelEntry = NULL;
    xpsScope_t        scope        = xpSaiScopeFromDevGet(xpsDevId);

    XP_SAI_LOG_DBG("enter xpSaiTunnelIdRelease\n");

    /* Search corresponding object */
    retVal = xpSaiTunnelGetCtxDb(xpsDevId, tunnelId, &pTunnelEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiTunnelGetCtxDb: Failed to fetch from XPS DB for tunnelId: %"
                       PRIu64 " | error: %d.\n", tunnelId, retVal);
        return retVal;
    }

    retVal = xpsAllocatorReleaseId(scope, XP_SAI_ALLOC_TUNNEL,
                                   pTunnelEntry->tunnelId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsAllocatorReleaseId: Unable to release tunnelId: %u | error: %d.\n",
                       pTunnelEntry->tunnelId, retVal);
        return retVal;
    }

    retVal = xpSaiTunnelDeleteCtxDb(xpsDevId, tunnelId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiTunnelDeleteCtxDb: Unable to deallocate XPS state entry, error: %d.\n",
                       retVal);
        return retVal;
    }

    return retVal;
}

//Func: xpSaiTunnelTermEntryIdAllocate

static XP_STATUS xpSaiTunnelTermEntryIdAllocate(xpsDevice_t xpsDevId,
                                                xpSaiTunnelTermTableEntryAttributesT *attributes,
                                                sai_object_id_t *tnlTermEntryId)
{
    XP_STATUS              retVal        = XP_NO_ERR;
    sai_status_t           saiRetVal     = SAI_STATUS_SUCCESS;
    uint32_t               entryId       = 0;
    xpSaiTunnelEntryInfoT *pTnlTermEntry = NULL;
    xpsScope_t             scope         = xpSaiScopeFromDevGet(xpsDevId);

    XP_SAI_LOG_DBG("enter xpSaiTunnelIdRelease\n");

    if ((NULL == attributes) || (NULL == tnlTermEntryId))
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return XP_ERR_NULL_POINTER;
    }

    retVal = xpsAllocatorAllocateId(scope, XP_SAI_ALLOC_TUNNEL_TERM_ENTRY,
                                    &entryId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsAllocatorAllocateId: Unable to allocate Tunnel termination entry id, error: %d.\n",
                       retVal);
        return retVal;
    }

    saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_TUNNEL_TERM_TABLE_ENTRY, xpsDevId,
                                 (sai_uint64_t)entryId, tnlTermEntryId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiObjIdCreate: Unable to create SAI object, error: %d.\n",
                       saiRetVal);
        return XP_ERR_INVALID_ID;
    }

    retVal = xpSaiTunnelEntryInsertCtxDb(xpsDevId, entryId, &pTnlTermEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiTunnelEntryInsertCtxDb: Unable to allocate XPS state Heap, error: %d.\n",
                       retVal);
        return retVal;
    }

    pTnlTermEntry->vrfId = attributes->vrfId.oid;
    pTnlTermEntry->entryType = attributes->entryType.s32;
    pTnlTermEntry->dstIpAddr = attributes->dstIpAddr.ipaddr;
    if (attributes->entryType.s32 == SAI_TUNNEL_TERM_TABLE_ENTRY_TYPE_P2P)
    {
        pTnlTermEntry->srcIpAddr = attributes->srcIpAddr.ipaddr;
    }

    pTnlTermEntry->tnlType = attributes->tunnelType.s32;
    pTnlTermEntry->tnlId = attributes->tunnelId.oid;

    return retVal;
}

//Func: xpSaiTunnelTermEntryIdRelease

static XP_STATUS xpSaiTunnelTermEntryIdRelease(xpsDevice_t xpsDevId,
                                               sai_object_id_t tnlTermEntryId)
{
    XP_STATUS              retVal        = XP_NO_ERR;
    xpSaiTunnelEntryInfoT *pTnlTermEntry = NULL;
    xpsScope_t             scope         = xpSaiScopeFromDevGet(xpsDevId);

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    /* Search corresponding object */
    retVal = xpSaiTunnelEntryGetCtxDb(xpsDevId, tnlTermEntryId, &pTnlTermEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiTunnelEntryGetCtxDb: Failed to fetch from XPS DB for tnlTermEntryId: %"
                       PRIu64 " | error: %d.\n", tnlTermEntryId, retVal);
        return retVal;
    }

    retVal = xpsAllocatorReleaseId(scope, XP_SAI_ALLOC_TUNNEL_TERM_ENTRY,
                                   pTnlTermEntry->tnlEntryId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsAllocatorReleaseId: Unable to release tnlTermEntryId: %u | error: %d.\n",
                       pTnlTermEntry->tnlEntryId, retVal);
        return retVal;
    }

    retVal = xpSaiTunnelEntryDeleteCtxDb(xpsDevId, tnlTermEntryId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiTunnelEntryDeleteCtxDb: Unable to deallocate XPS state entry, error: %d.\n",
                       retVal);
        return retVal;
    }

    return retVal;
}

//Func: xpSaiTunnelMapObjIdCreate

static sai_status_t xpSaiTunnelMapObjIdCreate(xpsDevice_t xpsDevId,
                                              uint32_t localId, sai_tunnel_map_type_t tnlMapType, sai_object_id_t *oid)
{
    sai_status_t    retVal = SAI_STATUS_SUCCESS;
    sai_object_id_t localOid = SAI_NULL_OBJECT_ID;
    sai_object_id_t mask = (sai_object_id_t)((uint8_t)tnlMapType & 0xff);

    retVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_TUNNEL_MAP, xpsDevId,
                              (sai_uint64_t)localId, &localOid);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiTunnelMapObjIdCreate: Unable to create SAI object, error: %d\n",
                       retVal);
        return retVal;
    }

    *oid = localOid | (mask << (SAI_TUNNEL_MAP_OBJID_TYPE_BITS));

    return retVal;
}

//Func: xpSaiTunnelMapObjIdMapTypeGet

static sai_uint32_t xpSaiTunnelMapObjIdMapTypeGet(sai_object_id_t sai_object_id)
{
    return (sai_uint32_t)((sai_object_id & SAI_TUNNEL_MAP_OBJID_TYPE_MASK) >>
                          SAI_TUNNEL_MAP_OBJID_TYPE_BITS);
}

//Func: xpSaiTunnelMapIdAllocate

XP_STATUS xpSaiTunnelMapIdAllocate(xpsDevice_t xpsDevId,
                                   xpSaiTunnelMapAttributesT *attributes, sai_object_id_t *tnlMapObjId)
{
    XP_STATUS            retVal    = XP_NO_ERR;
    sai_status_t         saiRetVal = SAI_STATUS_SUCCESS;
    uint32_t             tnlMapId  = 0;
    xpSaiTunnelMapInfoT *entry     = NULL;
    xpsScope_t           scope     = xpSaiScopeFromDevGet(xpsDevId);

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    if ((NULL == attributes) || (NULL == tnlMapObjId))
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return XP_ERR_NULL_POINTER;
    }

    retVal = xpsAllocatorAllocateId(scope, XP_SAI_ALLOC_TUNNEL_MAP, &tnlMapId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsAllocatorAllocateId: Unable to allocate Tunnel map id, error: %d.\n",
                       retVal);
        return retVal;
    }

    saiRetVal = xpSaiTunnelMapObjIdCreate(xpsDevId, (sai_uint64_t)tnlMapId,
                                          (sai_tunnel_map_type_t)attributes->mapType.s32, tnlMapObjId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiObjIdCreate: Unable to create SAI object, error: %d.\n",
                       saiRetVal);
        return XP_ERR_INVALID_ID;
    }

    retVal = xpSaiTunnelMapInsertCtxDb(xpsDevId, tnlMapId, &entry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiTunnelMapInsertCtxDb: Unable to allocate XPS state Heap, error: %d.\n",
                       retVal);
        return retVal;
    }

    entry->mapType = attributes->mapType.s32;

    return retVal;
}

//Func: xpSaiTunnelMapReleaseId

XP_STATUS xpSaiTunnelMapReleaseId(xpsDevice_t xpsDevId,
                                  sai_object_id_t tnlMapId)
{
    XP_STATUS            retVal = XP_NO_ERR;
    xpSaiTunnelMapInfoT *entry  = NULL;
    xpsScope_t           scope  = xpSaiScopeFromDevGet(xpsDevId);

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    if (SAI_NULL_OBJECT_ID == tnlMapId)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return XP_ERR_INVALID_INPUT;
    }

    /* Search corresponding object */
    retVal = xpSaiTunnelMapGetCtxDb(xpsDevId, tnlMapId, &entry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiTunnelMapGetCtxDb: Failed to fetch from XPS DB for tnlMapId: %"
                       PRIu64 " | error: %d.\n", tnlMapId, retVal);
        return retVal;
    }

    retVal = xpsAllocatorReleaseId(scope, XP_SAI_ALLOC_TUNNEL_MAP, entry->tnlMapId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsAllocatorReleaseId: Unable to release tunnelMapId: %u | error: %d.\n",
                       entry->tnlMapId, retVal);
        return retVal;
    }

    retVal = xpSaiTunnelMapDeleteCtxDb(xpsDevId, tnlMapId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiTunnelMapDeleteCtxDb: Unable to deallocate XPS state entry, error: %d.\n",
                       retVal);
        return retVal;
    }

    return retVal;
}

//Func: xpSaiTunnelMapEntryIdAllocate

static XP_STATUS xpSaiTunnelMapEntryIdAllocate(xpsDevice_t xpsDevId,
                                               xpSaiTunnelMapEntryAttributesT *attributes, sai_object_id_t *tunnelMapEntryId)
{
    XP_STATUS                 retVal        = XP_NO_ERR;
    sai_status_t              saiRetVal     = SAI_STATUS_SUCCESS;
    uint32_t                  tnlMapEntryId = 0;
    xpSaiTunnelMapEntryInfoT *pTnlMapEntry  = NULL;
    xpsScope_t                scope         = xpSaiScopeFromDevGet(xpsDevId);

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    if ((NULL == attributes) || (NULL == tunnelMapEntryId))
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return XP_ERR_NULL_POINTER;
    }

    retVal = xpsAllocatorAllocateId(scope, XP_SAI_ALLOC_TUNNEL_MAP_ENTRY,
                                    &tnlMapEntryId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsAllocatorAllocateId: Unable to allocate Tunnel map entry id, error: %d.\n",
                       retVal);
        return retVal;
    }

    saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_TUNNEL_MAP_ENTRY, xpsDevId,
                                 (sai_uint64_t)tnlMapEntryId, tunnelMapEntryId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiObjIdCreate: Unable to create SAI object, error: %d.\n",
                       saiRetVal);
        return XP_ERR_INVALID_ID;
    }

    retVal = xpSaiTunnelMapEntryInsertCtxDb(xpsDevId, tnlMapEntryId, &pTnlMapEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiTunnelMapEntryInsertCtxDb: Unable to allocate XPS state Heap, error: %d.\n",
                       retVal);
        return retVal;
    }

    pTnlMapEntry->mapType = attributes->mapType.s32;
    pTnlMapEntry->mapId = attributes->mapId.oid;

    if (attributes->mapType.s32 == SAI_TUNNEL_MAP_TYPE_VNI_TO_VLAN_ID)
    {
        pTnlMapEntry->vniIdKey = attributes->vniIdKey.u32;
        pTnlMapEntry->vlanId = attributes->vlanIdVal.u16;
    }
    else if (attributes->mapType.s32 == SAI_TUNNEL_MAP_TYPE_VLAN_ID_TO_VNI)
    {
        pTnlMapEntry->vlanIdKey = attributes->vlanIdKey.u16;
        pTnlMapEntry->vniId = attributes->vniIdVal.u32;
    }
    else if (attributes->mapType.s32 == SAI_TUNNEL_MAP_TYPE_VNI_TO_BRIDGE_IF)
    {
        pTnlMapEntry->vniIdKey = attributes->vniIdKey.u32;
        pTnlMapEntry->bridgeId = attributes->bridgeIdVal.oid;
    }
    else if (attributes->mapType.s32 == SAI_TUNNEL_MAP_TYPE_BRIDGE_IF_TO_VNI)
    {
        pTnlMapEntry->bridgeIdKey = attributes->bridgeIdKey.oid;
        pTnlMapEntry->vniId = attributes->vniIdVal.u32;
    }
    else if (attributes->mapType.s32 ==
             SAI_TUNNEL_MAP_TYPE_VIRTUAL_ROUTER_ID_TO_VNI)
    {
        pTnlMapEntry->vrfIdKey = attributes->vrfIdKey.u32;
        pTnlMapEntry->vniId = attributes->vniIdVal.u32;
    }
    else if (attributes->mapType.s32 ==
             SAI_TUNNEL_MAP_TYPE_VNI_TO_VIRTUAL_ROUTER_ID)
    {
        pTnlMapEntry->vniIdKey = attributes->vniIdKey.u32;
        pTnlMapEntry->vrfId = attributes->vrfIdVal.u32;
    }
    else
    {
        XP_SAI_LOG_ERR("xpSaiTunnelMapEntryIdAllocate: Tunnel map type %d is unknown.\n",
                       attributes->mapType.s32);
    }

    return retVal;
}

//Func: xpSaiTunnelMapEntryIdRelease

static XP_STATUS xpSaiTunnelMapEntryIdRelease(xpsDevice_t xpsDevId,
                                              sai_object_id_t tunnelMapEntryId)
{
    XP_STATUS                 retVal       = XP_NO_ERR;
    xpSaiTunnelMapEntryInfoT *pTnlMapEntry = NULL;
    xpsScope_t                scope        = xpSaiScopeFromDevGet(xpsDevId);

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    /* Search corresponding object */
    retVal = xpSaiTunnelMapEntryGetCtxDb(xpsDevId, tunnelMapEntryId, &pTnlMapEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiTunnelMapEntryGetCtxDb: Failed to fetch from XPS DB for tunnelMapEntryId: %"
                       PRIu64 " | error: %d.\n", tunnelMapEntryId, retVal);
        return retVal;
    }

    retVal = xpsAllocatorReleaseId(scope, XP_SAI_ALLOC_TUNNEL_MAP_ENTRY,
                                   pTnlMapEntry->tnlMapEntryId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsAllocatorReleaseId: Unable to release tunnelMapEntryId: %u | error: %d.\n",
                       pTnlMapEntry->tnlMapEntryId, retVal);
        return retVal;
    }

    retVal = xpSaiTunnelMapEntryDeleteCtxDb(xpsDevId, tunnelMapEntryId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiTunnelMapEntryDeleteCtxDb: Unable to deallocate XPS state entry, error: %d.\n",
                       retVal);
        return retVal;
    }

    return retVal;
}

//Func: xpSaiSetDefaultTunnelAttributeVals

static void xpSaiSetDefaultTunnelAttributeVals(xpSaiTunnelAttributesT*
                                               attributes)
{
    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    if (NULL == attributes)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return;
    }

    memset(attributes, 0, sizeof(xpSaiTunnelAttributesT));

    attributes->encapSrcIp.ipaddr.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    attributes->encapSrcIp.ipaddr.addr.ip4    = 0;
    attributes->encapDstIp.ipaddr.addr.ip4    = 0;
    attributes->encapTtlMode.s32              = SAI_TUNNEL_TTL_MODE_UNIFORM_MODEL;
    attributes->encapDscpMode.s32             = SAI_TUNNEL_DSCP_MODE_UNIFORM_MODEL;
    attributes->encapGreKeyValid.booldata     = false;
    attributes->encapGreKey.u32               = 0;
    attributes->encapEcnMode.s32              = SAI_TUNNEL_ENCAP_ECN_MODE_STANDARD;
    attributes->decapEcnMode.s32              = SAI_TUNNEL_DECAP_ECN_MODE_STANDARD;
    attributes->decapTtlMode.s32              = SAI_TUNNEL_TTL_MODE_UNIFORM_MODEL;
    attributes->decapDscpMode.s32             = SAI_TUNNEL_DSCP_MODE_UNIFORM_MODEL;
#if SAI_API_VERSION >= SAI_VERSION(1,8,1)
    attributes->udpSrcPortMode.s32            =
        SAI_TUNNEL_VXLAN_UDP_SPORT_MODE_EPHEMERAL;
#endif
    attributes->udpSrcPort.u16                = 0;
}

//Func: xpSaiUpdateTunnelAttributeVals

static sai_status_t xpSaiUpdateTunnelAttributeVals(const uint32_t attr_count,
                                                   const sai_attribute_t* attr_list, xpSaiTunnelAttributesT* attributes)
{
    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    for (uint32_t count = 0; count < attr_count; ++count)
    {
        switch (attr_list[count].id)
        {
            case SAI_TUNNEL_ATTR_TYPE:
                {
                    attributes->tunnelType = attr_list[count].value;
                    break;
                }
            case SAI_TUNNEL_ATTR_UNDERLAY_INTERFACE:
                {
                    attributes->underlayIntfId = attr_list[count].value;
                    break;
                }
            case SAI_TUNNEL_ATTR_OVERLAY_INTERFACE:
                {
                    attributes->overlayIntfId = attr_list[count].value;
                    break;
                }
            case SAI_TUNNEL_ATTR_ENCAP_SRC_IP:
                {
                    attributes->encapSrcIp = attr_list[count].value;
                    break;
                }
            case SAI_TUNNEL_ATTR_ENCAP_TTL_MODE:
                {
                    attributes->encapTtlMode = attr_list[count].value;
                    break;
                }
            case SAI_TUNNEL_ATTR_ENCAP_TTL_VAL:
                {
                    attributes->encapTtlVal = attr_list[count].value;
                    break;
                }
            case SAI_TUNNEL_ATTR_ENCAP_DSCP_MODE:
                {
                    attributes->encapDscpMode = attr_list[count].value;
                    break;
                }
            case SAI_TUNNEL_ATTR_ENCAP_DSCP_VAL:
                {
                    attributes->encapDscpVal = attr_list[count].value;
                    break;
                }
            case SAI_TUNNEL_ATTR_ENCAP_GRE_KEY_VALID:
                {
                    attributes->encapGreKeyValid = attr_list[count].value;
                    break;
                }
            case SAI_TUNNEL_ATTR_ENCAP_GRE_KEY:
                {
                    attributes->encapGreKey = attr_list[count].value;
                    break;
                }
            case SAI_TUNNEL_ATTR_ENCAP_ECN_MODE:
                {
                    attributes->encapEcnMode = attr_list[count].value;
                    break;
                }
            case SAI_TUNNEL_ATTR_ENCAP_MAPPERS:
                {
                    attributes->encapMappers = attr_list[count].value;
                    break;
                }
            case SAI_TUNNEL_ATTR_DECAP_ECN_MODE:
                {
                    attributes->decapEcnMode = attr_list[count].value;
                    break;
                }
            case SAI_TUNNEL_ATTR_DECAP_MAPPERS:
                {
                    attributes->decapMappers = attr_list[count].value;
                    break;
                }
            case SAI_TUNNEL_ATTR_DECAP_TTL_MODE:
                {
                    attributes->decapTtlMode = attr_list[count].value;
                    break;
                }
            case SAI_TUNNEL_ATTR_DECAP_DSCP_MODE:
                {
                    attributes->decapDscpMode = attr_list[count].value;
                    break;
                }
            case SAI_TUNNEL_ATTR_ENCAP_DST_IP:
                {
                    attributes->encapDstIp = attr_list[count].value;
                    break;
                }
            case SAI_TUNNEL_ATTR_PEER_MODE:
                {
                    attributes->peerMode = attr_list[count].value;
                    break;
                }
#if SAI_API_VERSION >= SAI_VERSION(1,8,1)
            case SAI_TUNNEL_ATTR_VXLAN_UDP_SPORT_MODE:
                {
                    attributes->udpSrcPortMode = attr_list[count].value;
                    break;
                }
            case SAI_TUNNEL_ATTR_VXLAN_UDP_SPORT:
                {
                    attributes->udpSrcPort = attr_list[count].value;
                    break;
                }
#endif
            default:
                {
                    XP_SAI_LOG_ERR("Failed to set %d.\n", attr_list[count].id);
                }
        }
    }
    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiCreateTunnel

sai_status_t xpSaiCreateTunnel(sai_object_id_t *tunnel_id,
                               sai_object_id_t switch_id,
                               uint32_t attr_count,
                               const sai_attribute_t *attr_list)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    XP_STATUS    retVal    = XP_NO_ERR;
    xpsDevice_t  xpsDevId  = 0;

    xpSaiTunnelAttributesT attributes;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    memset(&attributes, 0, sizeof(attributes));

    /* Check incoming attributes */
    saiRetVal = xpSaiAttrCheck(attr_count, attr_list,
                               TUNNEL_VALIDATION_ARRAY_SIZE, tunnel_attribs,
                               SAI_COMMON_API_CREATE);

    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiAttrCheck: Attribute check failed with error: %d.\n",
                       saiRetVal);
        return saiRetVal;
    }

    xpSaiSetDefaultTunnelAttributeVals(&attributes);
    saiRetVal = xpSaiUpdateTunnelAttributeVals(attr_count, attr_list, &attributes);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiUpdateTunnelAttributeVals: Failed to update attributes with error: %d.\n",
                       saiRetVal);
        return saiRetVal;
    }

    if ((NULL == tunnel_id) || (NULL == attr_list))
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument.\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    /* Get xpsDevId from switch_id */
    xpsDevId = xpSaiObjIdSwitchGet(switch_id);

    if ((attributes.tunnelType.s32 != SAI_TUNNEL_TYPE_VXLAN) &&
        (attributes.tunnelType.s32 != SAI_TUNNEL_TYPE_IPINIP))
    {
        XP_SAI_LOG_INFO("[skip] SAI Tunnel type %d is not implemented.\n",
                        attributes.tunnelType.s32);
        // SONIC Workaround
        return SAI_STATUS_SUCCESS;
    }

    retVal = xpSaiTunnelIdAllocate(xpsDevId, &attributes, tunnel_id);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiTunnelIdAllocate: SAI object Tunnel could not be created, error: %d.\n",
                       retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    if (attributes.tunnelType.s32 == SAI_TUNNEL_TYPE_VXLAN)
    {
        /* Create a VXLAN tunnel */
        saiRetVal = xpSaiVxlanTunnelCreate(xpsDevId, *tunnel_id);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiVxlanTunnelCreate: Failed to create VXLAN tunnel, error: %d.\n",
                           saiRetVal);
            return saiRetVal;
        }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiRemoveTunnel

sai_status_t xpSaiRemoveTunnel(sai_object_id_t tunnel_id)
{
    XP_STATUS         retVal   = XP_NO_ERR;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    xpSaiTunnelInfoT *tunnel   = NULL;
    xpsDevice_t       xpsDevId = 0;
    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    /* Check incoming parameters */
    if (!XDK_SAI_OBJID_TYPE_CHECK(tunnel_id, SAI_OBJECT_TYPE_TUNNEL))
    {
        XP_SAI_LOG_DBG("Wrong object type received (%lu).\n",
                       xpSaiObjIdTypeGet(tunnel_id));
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    /* Get xpsDevId from tunnel_id */
    xpsDevId = xpSaiObjIdSwitchGet(tunnel_id);

    /* Fetch from state DB */
    retVal = xpSaiTunnelGetCtxDb(xpsDevId, tunnel_id, &tunnel);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiTunnelGetCtxDb: Failed to fetch from XPS DB for tunnel_id: %"
                       PRIu64 " | error: %d.\n", tunnel_id, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    if (NULL == tunnel)
    {
        XP_SAI_LOG_ERR("Item was not found.\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    if (tunnel->entryCnt > 0)
    {
        XP_SAI_LOG_ERR("Need to remove all (%u) tunnel termination table entries associated with tunnel_id: %"
                       PRIu64 " first!\n", tunnel->entryCnt, tunnel_id);
        return SAI_STATUS_OBJECT_IN_USE;
    }

    if (tunnel->tunnelType == SAI_TUNNEL_TYPE_VXLAN)
    {
        /* Remove a VXLAN tunnel */
        saiRetVal = xpSaiVxlanTunnelRemove(xpsDevId, tunnel_id);
        if (saiRetVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpSaiVxlanTunnelRemove: Failed: %d\n",
                           saiRetVal);
            return saiRetVal;
        }
    }

    retVal = xpSaiTunnelIdRelease(xpsDevId, tunnel_id);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiTunnelIdRelease: SAI object Tunnel could not be removed, tunnel_id: %"
                       PRIu64 " | error: %d.\n", tunnel_id, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t xpSaiSetTunnelDecapDscpMode(xpsDevice_t xpsDevId,
                                                xpSaiTunnelInfoT *tunnel,
                                                int32_t decapDscpMode)
{
    XP_STATUS retVal = XP_NO_ERR;
    XpIpTunnelMode_t dcpDscpMode;
    int32_t prevDscpMode;

    if (tunnel == NULL)
    {
        return xpsStatus2SaiStatus(XP_ERR_NULL_POINTER);
    }

    prevDscpMode = tunnel->dcpDscpMode;

    if (prevDscpMode == decapDscpMode)
    {
        return SAI_STATUS_SUCCESS;
    }


    if (tunnel->entryCnt == 0)
    {
        tunnel->dcpDscpMode = decapDscpMode;
        return SAI_STATUS_SUCCESS;
    }
    switch (decapDscpMode)
    {
        case SAI_TUNNEL_DSCP_MODE_UNIFORM_MODEL:
            dcpDscpMode = XP_IP_TUNNEL_MODE_UNIFORM;
            break;

        case SAI_TUNNEL_DSCP_MODE_PIPE_MODEL:
            dcpDscpMode = XP_IP_TUNNEL_MODE_PIPE;
            break;
        default:
            {
                XP_SAI_LOG_ERR("Invalid input parameter : %d \n", decapDscpMode);
                return SAI_STATUS_FAILURE;
            }
    }

    if (tunnel->tunnelType != SAI_TUNNEL_TYPE_IPINIP &&
        tunnel->tunnelType != SAI_TUNNEL_TYPE_VXLAN)
    {
        XP_SAI_LOG_ERR("xpSaiTunnelEntryGetCtxDb: InvalidType %d | Type : %d error: %d.\n",
                       tunnel->tunnelId, tunnel->tunnelType, retVal);
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    retVal = xpsIpTunnelUpdateTerminationAction(xpsDevId,
                                                tunnel->tnlIntfId,
                                                NULL, &dcpDscpMode);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Update Action: Failed IpInIp Tunnel entry, tnlIntfId: %d | error: %d.\n",
                       tunnel->tnlIntfId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    tunnel->dcpDscpMode = decapDscpMode;
    return SAI_STATUS_SUCCESS;
}

static sai_status_t xpSaiSetTunnelEncapTtlMode(xpsDevice_t xpsDevId,
                                               xpSaiTunnelInfoT *tunnel,
                                               int32_t encapTtlMode)
{
    XP_STATUS retVal = XP_NO_ERR;
    XpIpTunnelMode_t ecpTtlMode;
    int32_t prevTtlMode;

    if (tunnel == NULL)
    {
        return xpsStatus2SaiStatus(XP_ERR_NULL_POINTER);
    }

    prevTtlMode = tunnel->ecpTtlMode;

    if (prevTtlMode == encapTtlMode)
    {
        return SAI_STATUS_SUCCESS;
    }

    switch (encapTtlMode)
    {
        case SAI_TUNNEL_TTL_MODE_UNIFORM_MODEL:
            ecpTtlMode = XP_IP_TUNNEL_MODE_UNIFORM;
            break;
        case SAI_TUNNEL_TTL_MODE_PIPE_MODEL:
            ecpTtlMode = XP_IP_TUNNEL_MODE_PIPE;
            /*TTL must be non-zero to update*/
            if (tunnel->ecpTtl == 0)
            {
                tunnel->ecpTtlMode = encapTtlMode;
                return SAI_STATUS_SUCCESS;
            }
            break;
        default:
            {
                XP_SAI_LOG_ERR("Invalid input parameter : %d \n", encapTtlMode);
                return SAI_STATUS_FAILURE;
            }
    }

    retVal = xpsIpTunnelUpdateTunnelStart(xpsDevId,
                                          tunnel->tnlIntfId,
                                          &ecpTtlMode, tunnel->ecpTtl, NULL, 0, NULL, 0);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("TTL update, error %d\n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    tunnel->ecpTtlMode = encapTtlMode;
    return SAI_STATUS_SUCCESS;
}

static sai_status_t xpSaiSetTunnelEncapTtl(xpsDevice_t xpsDevId,
                                           xpSaiTunnelInfoT *tunnel,
                                           uint8_t encapTtl)
{
    XP_STATUS retVal = XP_NO_ERR;
    XpIpTunnelMode_t ecpTtlMode;
    uint8_t prevTtl;

    if (tunnel == NULL)
    {
        return xpsStatus2SaiStatus(XP_ERR_NULL_POINTER);
    }

    prevTtl = tunnel->ecpTtl;

    if (prevTtl == encapTtl)
    {
        return SAI_STATUS_SUCCESS;
    }

    switch (tunnel->ecpTtlMode)
    {
        case SAI_TUNNEL_TTL_MODE_UNIFORM_MODEL:
            {
                ecpTtlMode = XP_IP_TUNNEL_MODE_UNIFORM;
                tunnel->ecpTtl= encapTtl;
                return SAI_STATUS_SUCCESS;
            }
            break;
        case SAI_TUNNEL_TTL_MODE_PIPE_MODEL:
            ecpTtlMode = XP_IP_TUNNEL_MODE_PIPE;
            /*TTL must be non-zero to update*/
            if (encapTtl == 0)
            {
                return SAI_STATUS_INVALID_ATTR_VALUE_0;
            }
            break;
        default:
            {
                XP_SAI_LOG_ERR("Invalid input parameter : %d \n", tunnel->ecpTtlMode);
                return SAI_STATUS_FAILURE;
            }
    }

    retVal = xpsIpTunnelUpdateTunnelStart(xpsDevId,
                                          tunnel->tnlIntfId,
                                          &ecpTtlMode, encapTtl, NULL, 0, NULL, 0);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("TTL update, error %d\n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    tunnel->ecpTtl = encapTtl;
    return SAI_STATUS_SUCCESS;
}

#if SAI_API_VERSION >= SAI_VERSION(1,8,1)
static sai_status_t xpSaiSetTunnelUdpSrcPortMode(xpsDevice_t xpsDevId,
                                                 xpSaiTunnelInfoT *tunnel,
                                                 uint32_t portMode)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpsVxlanUdpSrcPortMode_t mode;
    uint32_t prevMode;

    if (tunnel == NULL)
    {
        return xpsStatus2SaiStatus(XP_ERR_NULL_POINTER);
    }

    prevMode = tunnel->udpSrcPortMode;

    if (prevMode == portMode)
    {
        return SAI_STATUS_SUCCESS;
    }

    switch (portMode)
    {
        case SAI_TUNNEL_VXLAN_UDP_SPORT_MODE_EPHEMERAL:
            mode = XPS_VXLAN_UDP_PORT_HASH;
            break;
        case SAI_TUNNEL_VXLAN_UDP_SPORT_MODE_USER_DEFINED:
            mode = XPS_VXLAN_UDP_PORT_STATIC;
            break;
        default:
            {
                XP_SAI_LOG_ERR("Invalid input parameter : %d \n", portMode);
                return SAI_STATUS_FAILURE;
            }
    }

    retVal = xpsIpTunnelUpdateTunnelStart(xpsDevId,
                                          tunnel->tnlIntfId,
                                          NULL, 0, NULL, 0, &mode, tunnel->udpSrcPort);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("DSCP mode update, error %d\n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    tunnel->udpSrcPortMode = portMode;
    return SAI_STATUS_SUCCESS;
}

static sai_status_t xpSaiSetTunnelUdpSrcPort(xpsDevice_t xpsDevId,
                                             xpSaiTunnelInfoT *tunnel,
                                             uint16_t udpSrcPort)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpsVxlanUdpSrcPortMode_t mode;
    uint16_t prevVal;

    if (tunnel == NULL)
    {
        return xpsStatus2SaiStatus(XP_ERR_NULL_POINTER);
    }

    prevVal = tunnel->udpSrcPort;

    if (prevVal == udpSrcPort)
    {
        return SAI_STATUS_SUCCESS;
    }

    switch (tunnel->udpSrcPortMode)
    {
        case SAI_TUNNEL_VXLAN_UDP_SPORT_MODE_USER_DEFINED:
            {
                mode = XPS_VXLAN_UDP_PORT_STATIC;
            }
            break;
        case SAI_TUNNEL_VXLAN_UDP_SPORT_MODE_EPHEMERAL:
            {
                mode = XPS_VXLAN_UDP_PORT_HASH;
                tunnel->udpSrcPort = udpSrcPort;
                return SAI_STATUS_SUCCESS;
            }
        default:
            {
                XP_SAI_LOG_ERR("Invalid input parameter : %d \n", tunnel->udpSrcPortMode);
                return SAI_STATUS_FAILURE;
            }
    }

    retVal = xpsIpTunnelUpdateTunnelStart(xpsDevId,
                                          tunnel->tnlIntfId,
                                          NULL, 0, NULL, 0, &mode, udpSrcPort);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("UDP Src Port update, error %d\n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    tunnel->udpSrcPort = udpSrcPort;
    return SAI_STATUS_SUCCESS;
}
#endif

static sai_status_t xpSaiSetTunnelEncapDscpMode(xpsDevice_t xpsDevId,
                                                xpSaiTunnelInfoT *tunnel,
                                                int32_t encapDscpMode)
{
    XP_STATUS retVal = XP_NO_ERR;
    XpIpTunnelMode_t ecpDscpMode;
    int32_t prevDscpMode;

    if (tunnel == NULL)
    {
        return xpsStatus2SaiStatus(XP_ERR_NULL_POINTER);
    }

    prevDscpMode = tunnel->ecpDscpMode;

    if (prevDscpMode == encapDscpMode)
    {
        return SAI_STATUS_SUCCESS;
    }

    switch (encapDscpMode)
    {
        case SAI_TUNNEL_DSCP_MODE_UNIFORM_MODEL:
            ecpDscpMode = XP_IP_TUNNEL_MODE_UNIFORM;
            break;
        case SAI_TUNNEL_DSCP_MODE_PIPE_MODEL:
            ecpDscpMode = XP_IP_TUNNEL_MODE_PIPE;
            break;
        default:
            {
                XP_SAI_LOG_ERR("Invalid input parameter : %d \n", encapDscpMode);
                return SAI_STATUS_FAILURE;
            }
    }

    retVal = xpsIpTunnelUpdateTunnelStart(xpsDevId,
                                          tunnel->tnlIntfId,
                                          NULL, 0, &ecpDscpMode, tunnel->ecpDscp, NULL, 0);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("DSCP mode update, error %d\n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    tunnel->ecpDscpMode = encapDscpMode;
    return SAI_STATUS_SUCCESS;
}

static sai_status_t xpSaiSetTunnelEncapDscp(xpsDevice_t xpsDevId,
                                            xpSaiTunnelInfoT *tunnel,
                                            uint8_t encapDscp)
{
    XP_STATUS retVal = XP_NO_ERR;
    XpIpTunnelMode_t ecpDscpMode;
    uint8_t prevDscp;

    if (tunnel == NULL)
    {
        return xpsStatus2SaiStatus(XP_ERR_NULL_POINTER);
    }

    prevDscp = tunnel->ecpDscp;

    if (prevDscp == encapDscp)
    {
        return SAI_STATUS_SUCCESS;
    }

    switch (tunnel->ecpDscpMode)
    {
        case SAI_TUNNEL_DSCP_MODE_UNIFORM_MODEL:
            {
                ecpDscpMode = XP_IP_TUNNEL_MODE_UNIFORM;
                tunnel->ecpDscp = encapDscp;
                return SAI_STATUS_SUCCESS;
            }
            break;
        case SAI_TUNNEL_DSCP_MODE_PIPE_MODEL:
            ecpDscpMode = XP_IP_TUNNEL_MODE_PIPE;
            break;
        default:
            {
                XP_SAI_LOG_ERR("Invalid input parameter : %d \n", tunnel->ecpDscpMode);
                return SAI_STATUS_FAILURE;
            }
    }

    retVal = xpsIpTunnelUpdateTunnelStart(xpsDevId,
                                          tunnel->tnlIntfId,
                                          NULL, 0, &ecpDscpMode, encapDscp, NULL, 0);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("DSCP update, error %d\n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    tunnel->ecpDscp = encapDscp;
    return SAI_STATUS_SUCCESS;
}

static sai_status_t xpSaiSetTunnelDecapTtlMode(xpsDevice_t xpsDevId,
                                               xpSaiTunnelInfoT *tunnel,
                                               int32_t decapTtlMode)
{
    XP_STATUS retVal = XP_NO_ERR;
    XpIpTunnelMode_t dcpTtlMode;
    int32_t prevTtlMode;

    if (tunnel == NULL)
    {
        return xpsStatus2SaiStatus(XP_ERR_NULL_POINTER);
    }

    prevTtlMode = tunnel->dcpTtlMode;

    if (prevTtlMode == decapTtlMode)
    {
        return SAI_STATUS_SUCCESS;
    }

    if (tunnel->entryCnt == 0)
    {
        tunnel->dcpTtlMode = decapTtlMode;
        return SAI_STATUS_SUCCESS;
    }
    switch (decapTtlMode)
    {
        case SAI_TUNNEL_TTL_MODE_UNIFORM_MODEL:
            dcpTtlMode = XP_IP_TUNNEL_MODE_UNIFORM;
            break;

        case SAI_TUNNEL_TTL_MODE_PIPE_MODEL:
            dcpTtlMode = XP_IP_TUNNEL_MODE_PIPE;
            break;
        default:
            {
                XP_SAI_LOG_ERR("Invalid input parameter : %d \n", decapTtlMode);
                return SAI_STATUS_FAILURE;
            }
    }

    if (tunnel->tunnelType != SAI_TUNNEL_TYPE_IPINIP &&
        tunnel->tunnelType != SAI_TUNNEL_TYPE_VXLAN)
    {
        XP_SAI_LOG_ERR("xpSaiTunnelEntryGetCtxDb: InvalidType %d | Type : %d error: %d.\n",
                       tunnel->tunnelId, tunnel->tunnelType, retVal);
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    retVal = xpsIpTunnelUpdateTerminationAction(xpsDevId,
                                                tunnel->tnlIntfId,
                                                &dcpTtlMode, NULL);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Update Action: Failed IpInIp Tunnel entry, tnlIntfId: %d | error: %d.\n",
                       tunnel->tnlIntfId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    tunnel->dcpTtlMode = decapTtlMode;
    return SAI_STATUS_SUCCESS;
}
//Func: xpSaiSetTunnelAttribute

sai_status_t xpSaiSetTunnelAttribute(sai_object_id_t tunnel_id,
                                     const sai_attribute_t *attr)
{
    sai_status_t      saiRetVal = SAI_STATUS_SUCCESS;
    XP_STATUS         retVal    = XP_NO_ERR;
    xpSaiTunnelInfoT *tunnel    = NULL;
    xpsDevice_t       xpsDevId  = 0;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    if (NULL == attr)
    {
        XP_SAI_LOG_ERR("Invalid parameter received!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    /* Check incoming attributes */
    saiRetVal = xpSaiAttrCheck(1, attr,
                               TUNNEL_VALIDATION_ARRAY_SIZE, tunnel_attribs,
                               SAI_COMMON_API_SET);

    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiAttrCheck: Attribute check failed with error: %d.\n",
                       saiRetVal);
        return saiRetVal;
    }

    /* Get xpsDevId from tunnel_id */
    xpsDevId = xpSaiObjIdSwitchGet(tunnel_id);

    /* Fetch from state DB */
    retVal = xpSaiTunnelGetCtxDb(xpsDevId, tunnel_id, &tunnel);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiTunnelGetCtxDb: Failed to fetch from XPS DB for tunnel_id: %"
                       PRIu64 " | error: %d.\n", tunnel_id, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    switch (attr->id)
    {
        case SAI_TUNNEL_ATTR_ENCAP_GRE_KEY:
            {
                if (tunnel->tnlGreKeyValid)
                {
                    tunnel->tnlGreKey = attr->value.u32;
                }
                break;
            }
        case SAI_TUNNEL_ATTR_DECAP_TTL_MODE:
            {
                saiRetVal = xpSaiSetTunnelDecapTtlMode(xpsDevId, tunnel, attr->value.s32);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    return saiRetVal;
                }
            }
            break;
        case SAI_TUNNEL_ATTR_ENCAP_TTL_MODE:
            {
                saiRetVal = xpSaiSetTunnelEncapTtlMode(xpsDevId, tunnel, attr->value.s32);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    return saiRetVal;
                }
            }
            break;
        case SAI_TUNNEL_ATTR_ENCAP_TTL_VAL:
            {
                saiRetVal = xpSaiSetTunnelEncapTtl(xpsDevId, tunnel, attr->value.s32);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    return saiRetVal;
                }
            }
            break;
        case SAI_TUNNEL_ATTR_DECAP_DSCP_MODE:
            {
                saiRetVal = xpSaiSetTunnelDecapDscpMode(xpsDevId, tunnel, attr->value.s32);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    return saiRetVal;
                }
            }
            break;
        case SAI_TUNNEL_ATTR_ENCAP_DSCP_MODE:
            {
                saiRetVal = xpSaiSetTunnelEncapDscpMode(xpsDevId, tunnel, attr->value.s32);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    return saiRetVal;
                }
            }
            break;
        case SAI_TUNNEL_ATTR_ENCAP_DSCP_VAL:
            {
                saiRetVal = xpSaiSetTunnelEncapDscp(xpsDevId, tunnel, attr->value.s32);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    return saiRetVal;
                }
            }
            break;
#if SAI_API_VERSION >= SAI_VERSION(1,8,1)
        case SAI_TUNNEL_ATTR_VXLAN_UDP_SPORT_MODE:
            {
                saiRetVal = xpSaiSetTunnelUdpSrcPortMode(xpsDevId, tunnel, attr->value.s32);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    return saiRetVal;
                }
            }
            break;
        case SAI_TUNNEL_ATTR_VXLAN_UDP_SPORT:
            {
                saiRetVal = xpSaiSetTunnelUdpSrcPort(xpsDevId, tunnel, attr->value.s32);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    return saiRetVal;
                }
            }
            break;
#endif

        default:
            {
                XP_SAI_LOG_ERR("Invalid parameter received %d.\n", attr->id);
                return SAI_STATUS_INVALID_PARAMETER;
            }
    }
    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetTunnelAttrEncapSrcIp

static sai_status_t xpSaiGetTunnelAttrEncapSrcIp(sai_object_id_t tunnelId,
                                                 sai_attribute_value_t* value)
{
    XP_STATUS         retVal   = XP_NO_ERR;
    xpsDevice_t       xpsDevId = 0;
    xpSaiTunnelInfoT *tunnel   = NULL;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    if (NULL == value)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return SAI_STATUS_FAILURE;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(tunnelId, SAI_OBJECT_TYPE_TUNNEL))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).\n",
                       xpSaiObjIdTypeGet(tunnelId));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    /* Get xpsDevId from tunnelId */
    xpsDevId = xpSaiObjIdSwitchGet(tunnelId);

    /* Search corresponding object */
    retVal = xpSaiTunnelGetCtxDb(xpsDevId, tunnelId, &tunnel);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiTunnelGetCtxDb: Failed to fetch from XPS DB for tunnelId: %"
                       PRIu64 " | error: %d.\n", tunnelId, retVal)
        return retVal;
    }

    value->ipaddr.addr.ip4 = tunnel->tnlSrcIp.addr.ip4;

    return xpsStatus2SaiStatus(retVal);
}

//Func: xpSaiTunnelGetAttrEncapMapList

static sai_status_t xpSaiTunnelGetAttrEncapMapList(sai_object_id_t tunnelId,
                                                   sai_attribute_value_t* value)
{
    XP_STATUS         retVal   = XP_NO_ERR;
    xpsDevice_t       xpsDevId = 0;
    xpSaiTunnelInfoT *tunnel   = NULL;
    uint32_t          count    = 0;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    if (NULL == value)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return SAI_STATUS_FAILURE;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(tunnelId, SAI_OBJECT_TYPE_TUNNEL))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).\n",
                       xpSaiObjIdTypeGet(tunnelId));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    /* Get xpsDevId from tunnel_id */
    xpsDevId = xpSaiObjIdSwitchGet(tunnelId);

    /* Search corresponding object */
    retVal = xpSaiTunnelGetCtxDb(xpsDevId, tunnelId, &tunnel);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiTunnelGetCtxDb: Failed to fetch from XPS DB for tunnelId: %"
                       PRIu64 " | error: %d.\n", tunnelId, retVal);
        return retVal;
    }

    XP_SAI_LOG_DBG("Tunnel %" PRIu64 " is associated with %d mappers.\n", tunnelId,
                   tunnel->ecpMapListCnt);

    if (value->objlist.count < tunnel->ecpMapListCnt)
    {
        value->objlist.count = tunnel->ecpMapListCnt;
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    if ((tunnel->ecpMapListCnt != 0) && (NULL == value->objlist.list))
    {
        XP_SAI_LOG_ERR("Invalid parameters received.\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    for (count = 0; count < tunnel->ecpMapListCnt; ++count)
    {
        value->objlist.list[count] = tunnel->ecpMapList[count];
    }

    value->objlist.count = tunnel->ecpMapListCnt;

    return xpsStatus2SaiStatus(retVal);
}

//Func: xpSaiTunnelGetAttrDecapMapList

static sai_status_t xpSaiTunnelGetAttrDecapMapList(sai_object_id_t tunnelId,
                                                   sai_attribute_value_t* value)
{
    XP_STATUS         retVal   = XP_NO_ERR;
    xpsDevice_t       xpsDevId = 0;
    xpSaiTunnelInfoT *tunnel   = NULL;
    uint32_t          count    = 0;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    if (NULL == value)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return SAI_STATUS_FAILURE;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(tunnelId, SAI_OBJECT_TYPE_TUNNEL))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).\n",
                       xpSaiObjIdTypeGet(tunnelId));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    /* Get xpsDevId from tunnel_id */
    xpsDevId = xpSaiObjIdSwitchGet(tunnelId);

    /* Search corresponding object */
    retVal = xpSaiTunnelGetCtxDb(xpsDevId, tunnelId, &tunnel);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiTunnelGetCtxDb: Failed to fetch from XPS DB for tunnelId: %"
                       PRIu64 " | error: %d.\n", tunnelId, retVal);
        return retVal;
    }

    XP_SAI_LOG_DBG("Tunnel %" PRIu64 " is associated with %d mappers.\n", tunnelId,
                   tunnel->dcpMapListCnt);

    if (value->objlist.count < tunnel->dcpMapListCnt)
    {
        value->objlist.count = tunnel->dcpMapListCnt;
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    if ((tunnel->dcpMapListCnt != 0) && (NULL == value->objlist.list))
    {
        XP_SAI_LOG_ERR("Invalid parameters received.\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    for (count = 0; count < tunnel->dcpMapListCnt; ++count)
    {
        value->objlist.list[count] = tunnel->dcpMapList[count];
    }

    value->objlist.count = tunnel->dcpMapListCnt;

    return xpsStatus2SaiStatus(retVal);
}

//Func: xpSaiTunnelGetAttrTermTableEntryList

static sai_status_t xpSaiTunnelGetAttrTermTableEntryList(
    sai_object_id_t tunnel_id, sai_attribute_value_t* value)
{
    XP_STATUS         retVal   = XP_NO_ERR;
    xpsDevice_t       xpsDevId = 0;
    xpSaiTunnelInfoT *tunnel   = NULL;
    uint32_t          count    = 0;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    if (NULL == value)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return SAI_STATUS_FAILURE;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(tunnel_id, SAI_OBJECT_TYPE_TUNNEL))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).\n",
                       xpSaiObjIdTypeGet(tunnel_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    /* Get xpsDevId from tunnel_id */
    xpsDevId = xpSaiObjIdSwitchGet(tunnel_id);

    /* Search corresponding object */
    retVal = xpSaiTunnelGetCtxDb(xpsDevId, tunnel_id, &tunnel);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiTunnelGetCtxDb: Failed to fetch from XPS DB for tunnel_id: %"
                       PRIu64 " | error: %d.\n", tunnel_id, retVal);
        return retVal;
    }

    XP_SAI_LOG_DBG("Tunnel %" PRIu64
                   " associated with %d termination table entries.\n", tunnel_id,
                   tunnel->entryCnt);

    if (value->objlist.count < tunnel->entryCnt)
    {
        value->objlist.count = tunnel->entryCnt;
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    if ((tunnel->entryCnt != 0) && (NULL == value->objlist.list))
    {
        XP_SAI_LOG_ERR("Invalid parameters received.\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    for (count = 0; count < tunnel->entryCnt; ++count)
    {
        value->objlist.list[count] = tunnel->entryId[count];
    }

    value->objlist.count = tunnel->entryCnt;

    return xpsStatus2SaiStatus(retVal);
}

//Func: xpSaiGetTunnelAttribute

static sai_status_t xpSaiGetTunnelAttribute(sai_object_id_t tunnel_id,
                                            xpSaiTunnelInfoT *tunnel, sai_attribute_t *attr)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    if ((NULL == tunnel) || (NULL == attr))
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return SAI_STATUS_FAILURE;
    }

    switch (attr->id)
    {
        case SAI_TUNNEL_ATTR_TYPE:
            {
                attr->value.s32 = tunnel->tunnelType;
                break;
            }
        case SAI_TUNNEL_ATTR_UNDERLAY_INTERFACE:
            {
                attr->value.oid = tunnel->uLayIntfId;
                break;
            }
        case SAI_TUNNEL_ATTR_OVERLAY_INTERFACE:
            {
                attr->value.oid = tunnel->oLayIntfId;
                break;
            }
        case SAI_TUNNEL_ATTR_ENCAP_SRC_IP:
            {
                saiRetVal = xpSaiGetTunnelAttrEncapSrcIp(tunnel_id, &attr->value);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("xpSaiGetTunnelAttrEncapSrcIp: Failed to get (SAI_TUNNEL_ATTR_ENCAP_SRC_IP), tunnel_id %"
                                   PRIu64 " | error: %d.\n",
                                   tunnel_id, saiRetVal);
                    return saiRetVal;
                }
                break;
            }
        case SAI_TUNNEL_ATTR_ENCAP_GRE_KEY_VALID:
            {
                attr->value.booldata = tunnel->tnlGreKeyValid;
                break;
            }
        case SAI_TUNNEL_ATTR_ENCAP_GRE_KEY:
            {
                attr->value.u32 = tunnel->tnlGreKey;
                break;
            }
        case SAI_TUNNEL_ATTR_ENCAP_MAPPERS:
            {
                saiRetVal = xpSaiTunnelGetAttrEncapMapList(tunnel_id, &attr->value);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("xpSaiTunnelGetAttrEncapMapList: Failed to get (SAI_TUNNEL_ATTR_ENCAP_MAPPERS), tunnel_id %"
                                   PRIu64 " | error: %d.\n",
                                   tunnel_id, saiRetVal);
                    return saiRetVal;
                }
                break;
            }
        case SAI_TUNNEL_ATTR_DECAP_MAPPERS:
            {
                saiRetVal = xpSaiTunnelGetAttrDecapMapList(tunnel_id, &attr->value);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("xpSaiTunnelGetAttrDecapMapList: Failed to get (SAI_TUNNEL_ATTR_DECAP_MAPPERS), tunnel_id %"
                                   PRIu64 " | error: %d.\n",
                                   tunnel_id, saiRetVal);
                    return saiRetVal;
                }
                break;
            }
        case SAI_TUNNEL_ATTR_TERM_TABLE_ENTRY_LIST:
            {
                saiRetVal = xpSaiTunnelGetAttrTermTableEntryList(tunnel_id, &attr->value);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    if (saiRetVal != SAI_STATUS_BUFFER_OVERFLOW)
                    {
                        XP_SAI_LOG_ERR("xpSaiTunnelGetAttrTermTableEntryList: Failed to get (SAI_TUNNEL_ATTR_TERM_TABLE_ENTRY_LIST), tunnel_id %"
                                       PRIu64
                                       " | error: %d.\n", tunnel_id, saiRetVal);
                    }
                    return saiRetVal;
                }
                break;
            }
        case SAI_TUNNEL_ATTR_DECAP_DSCP_MODE:
            {
                attr->value.s32 = tunnel->dcpDscpMode;
                break;
            }
        case SAI_TUNNEL_ATTR_DECAP_TTL_MODE:
            {
                attr->value.s32 = tunnel->dcpTtlMode;
                break;
            }
        case SAI_TUNNEL_ATTR_DECAP_ECN_MODE:
            {
                attr->value.s32 = tunnel->dcpEcnMode;
                break;
            }
        case SAI_TUNNEL_ATTR_ENCAP_TTL_MODE:
            {
                attr->value.s32 = tunnel->ecpTtlMode;
                break;
            }
        case SAI_TUNNEL_ATTR_ENCAP_TTL_VAL:
            {
                attr->value.s32 = tunnel->ecpTtl;
                break;
            }
        case SAI_TUNNEL_ATTR_ENCAP_DSCP_MODE:
            {
                attr->value.s32 = tunnel->ecpDscpMode;
                break;
            }
        case SAI_TUNNEL_ATTR_ENCAP_DSCP_VAL:
            {
                attr->value.s32 = tunnel->ecpDscp;
                break;
            }
#if SAI_API_VERSION >= SAI_VERSION(1,8,1)
        case SAI_TUNNEL_ATTR_VXLAN_UDP_SPORT_MODE:
            {
                attr->value.s32 = tunnel->udpSrcPortMode;
                break;
            }
        case SAI_TUNNEL_ATTR_VXLAN_UDP_SPORT:
            {
                attr->value.u16 = tunnel->udpSrcPort;
                break;
            }
#endif
        case SAI_TUNNEL_ATTR_ENCAP_ECN_MODE:
            {
                XP_SAI_LOG_ERR("Attribute %d is not supported.\n", attr->id);
                return SAI_STATUS_NOT_SUPPORTED;
            }
        default:
            {
                XP_SAI_LOG_ERR("Invalid parameter received %d.\n", attr->id);
                return SAI_STATUS_INVALID_PARAMETER;
            }
    }
    return saiRetVal;
}

//Func: xpSaiGetTunnelAttributes

sai_status_t xpSaiGetTunnelAttributes(sai_object_id_t tunnel_id,
                                      uint32_t attr_count, sai_attribute_t *attr_list)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    xpsDevice_t  xpsDevId  = 0;
    uint32_t     count     = 0;

    if (NULL == attr_list)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return SAI_STATUS_FAILURE;
    }

    /* Check incoming attributes */
    saiRetVal = xpSaiAttrCheck(attr_count, attr_list,
                               TUNNEL_VALIDATION_ARRAY_SIZE, tunnel_attribs,
                               SAI_COMMON_API_GET);

    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiAttrCheck: Attribute check failed with error: %d.\n",
                       saiRetVal);
        return saiRetVal;
    }

    /* Get xpsDevId from tunnel_id */
    xpsDevId = xpSaiObjIdSwitchGet(tunnel_id);

    for (count = 0; count < attr_count; ++count)
    {
        XP_STATUS         retVal = XP_NO_ERR;
        xpSaiTunnelInfoT *tunnel = NULL;

        /* Fetch from state DB */
        retVal = xpSaiTunnelGetCtxDb(xpsDevId, tunnel_id, &tunnel);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpSaiTunnelGetCtxDb: Failed to fetch from XPS DB for tunnel_id: %"
                           PRIu64 " | error: %d.\n", tunnel_id, retVal);
            return xpsStatus2SaiStatus(retVal);
        }

        saiRetVal = xpSaiGetTunnelAttribute(tunnel_id, tunnel, &attr_list[count]);
        if (SAI_STATUS_SUCCESS != saiRetVal)
        {
            XP_SAI_LOG_ERR("xpSaiGetTunnelAttribute failed, error: %d.\n", saiRetVal);
            return saiRetVal;
        }
    }

    return saiRetVal;
}

//Func: xpSaiTunnelGetStats

sai_status_t xpSaiTunnelGetStats(sai_object_id_t tunnel_id,
                                 uint32_t number_of_counters,
                                 const sai_stat_id_t *counter_ids,
                                 uint64_t *counters)
{
    sai_status_t saiRetVal = SAI_STATUS_NOT_IMPLEMENTED;
    return saiRetVal;
}

//Func: xpSaiTunnelClearStats

sai_status_t xpSaiTunnelClearStats(sai_object_id_t tunnel_id,
                                   uint32_t number_of_counters,
                                   const sai_stat_id_t *counter_ids)
{
    sai_status_t saiRetVal = SAI_STATUS_NOT_IMPLEMENTED;
    return saiRetVal;
}

//Func: xpSaiSetDefaultTunnelTermTableEntryAttributeVals

static void xpSaiSetDefaultTunnelTermTableEntryAttributeVals(
    xpSaiTunnelTermTableEntryAttributesT* attributes)
{
    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);
}

//Func: xpSaiUpdateTunnelTermTableEntryAttributeVals

static sai_status_t xpSaiUpdateTunnelTermTableEntryAttributeVals(
    const uint32_t attr_count, const sai_attribute_t* attr_list,
    xpSaiTunnelTermTableEntryAttributesT* attributes)
{
    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    for (uint32_t count = 0; count < attr_count; ++count)
    {
        switch (attr_list[count].id)
        {
            case SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_VR_ID:
                {
                    attributes->vrfId = attr_list[count].value;
                    break;
                }
            case SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_TYPE:
                {
                    attributes->entryType = attr_list[count].value;
                    break;
                }
            case SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_DST_IP:
                {
                    attributes->dstIpAddr = attr_list[count].value;
                    break;
                }
            case SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_SRC_IP:
                {
                    attributes->srcIpAddr = attr_list[count].value;
                    break;
                }
            case SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_TUNNEL_TYPE:
                {
                    attributes->tunnelType = attr_list[count].value;
                    break;
                }
            case SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_ACTION_TUNNEL_ID:
                {
                    attributes->tunnelId = attr_list[count].value;
                    break;
                }
            default:
                {
                    XP_SAI_LOG_NOTICE("Not supported to set %d.\n", attr_list[count].id);
                }
        }
    }
    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiTunnelAddTermTableEntry

static sai_status_t xpSaiTunnelAddTermTableEntry(xpsDevice_t xpsDevId,
                                                 sai_object_id_t tunnelId, sai_object_id_t entryId)
{
    XP_STATUS         retVal = XP_NO_ERR;
    xpsScope_t  scope        = 0;
    xpSaiTunnelInfoT *entry  = NULL;
    xpSaiTunnelInfoT *newEntry = NULL;
    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(tunnelId, SAI_OBJECT_TYPE_TUNNEL))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).\n",
                       xpSaiObjIdTypeGet(tunnelId));
        return SAI_STATUS_INVALID_PARAMETER;
    }
    /* Get scope from xpsDevId */
    scope = xpSaiScopeFromDevGet(xpsDevId);
    /* Search for the node in global SAI TUNNEL DB */
    retVal = xpSaiTunnelGetCtxDb(xpsDevId, tunnelId, &entry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiTunnelGetCtxDb: Failed to fetch from XPS DB for tunnelId: %"
                       PRIu64 " | error: %d.\n", tunnelId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }
    if (xpsDAIsCtxGrowthNeeded(entry->entryCnt, XP_SAI_TUNNEL_THRESHOLD))
    {
        /* Double the entry list array size */
        retVal = xpSaiDynamicArrayGrow((void**)&newEntry, (void*)entry,
                                       sizeof(xpSaiTunnelInfoT),
                                       sizeof(sai_object_id_t), entry->entryCnt,
                                       XP_SAI_TUNNEL_THRESHOLD);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpSaiDynamicArrayGrow: Dynamic array grow failed, error: %d.\n",
                           retVal);
            return xpsStatus2SaiStatus(retVal);
        }

        /* Remove the corresponding state from SAI TUNNEL DB and add new state */
        retVal = xpSaiTunnelDeleteCtxDb(xpsDevId, tunnelId);
        if (retVal != XP_NO_ERR)
        {
            xpsStateHeapFree((void*)newEntry);
            XP_SAI_LOG_ERR("xpSaiTunnelDeleteCtxDb: Deletion of data failed for tunnelId: %"
                           PRIu64 " | error: %d.\n", tunnelId, retVal);
            return xpsStatus2SaiStatus(retVal);
        }

        /* Insert the new state */
        retVal = xpsStateInsertData(scope, xpSaiTunnelStateDbHndl, (void*)newEntry);
        if (retVal != XP_NO_ERR)
        {
            xpsStateHeapFree((void*)newEntry);
            XP_SAI_LOG_ERR("xpsStateInsertData: Insertion of data failed, error: %d.\n",
                           retVal);
            return retVal;
        }

        entry = newEntry;
    }

    entry->entryId[entry->entryCnt++] = entryId;

    return xpsStatus2SaiStatus(retVal);
}

//Func: xpSaiTunnelRemoveTermTableEntry

static sai_status_t xpSaiTunnelRemoveTermTableEntry(xpsDevice_t xpsDevId,
                                                    sai_object_id_t tunnelId, sai_object_id_t entryId)
{
    XP_STATUS         retVal           = XP_NO_ERR;
    xpSaiTunnelInfoT *entry            = NULL;
    xpSaiTunnelInfoT *newEntry         = NULL;
    sai_uint8_t       isEnrtyIdPresent = 0;
    sai_uint32_t      count            = 0;
    xpsScope_t        scope            = 0;
    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(tunnelId, SAI_OBJECT_TYPE_TUNNEL))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).\n",
                       xpSaiObjIdTypeGet(tunnelId));
        return SAI_STATUS_INVALID_PARAMETER;
    }
    /* Get scope from xpsDevId */
    scope = xpSaiScopeFromDevGet(xpsDevId);

    /* Search for the node in global SAI TUNNEL DB */
    retVal = xpSaiTunnelGetCtxDb(xpsDevId, tunnelId, &entry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiTunnelGetCtxDb: Failed to fetch from XPS DB for tunnelId: %"
                       PRIu64 " | error: %d.\n", tunnelId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    /* Remove the Term Entry Id from associated Tunnel */
    for (count = 0; count < entry->entryCnt; ++count)
    {
        if (!isEnrtyIdPresent)
        {
            if (entry->entryId[count] == entryId)
            {
                isEnrtyIdPresent = 1;
            }
            continue;
        }
        else
        {
            entry->entryId[count-1] = entry->entryId[count];
        }
    }

    if (!isEnrtyIdPresent)
    {
        return xpsStatus2SaiStatus(XP_ERR_INVALID_ARG);
    }

    /* Decrement the entryCnt count */
    entry->entryCnt--;

    if (xpsDAIsCtxShrinkNeeded(entry->entryCnt, XP_SAI_TUNNEL_THRESHOLD))
    {
        /* Shrink the entryId array size */
        retVal = xpSaiDynamicArrayShrink((void**)&newEntry, (void*)entry,
                                         sizeof(xpSaiTunnelInfoT),
                                         sizeof(sai_object_id_t), entry->entryCnt,
                                         XP_SAI_TUNNEL_THRESHOLD);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpSaiDynamicArrayShrink: Dynamic array shrink failed, error: %d.\n",
                           retVal);
            return xpsStatus2SaiStatus(retVal);
        }

        /* Remove the corresponding state from SAI TUNNEL DB and add new state */
        retVal = xpSaiTunnelDeleteCtxDb(xpsDevId, tunnelId);
        if (retVal != XP_NO_ERR)
        {
            xpsStateHeapFree((void*)newEntry);
            XP_SAI_LOG_ERR("xpSaiTunnelDeleteCtxDb: Deletion of data failed for tunnelId: %"
                           PRIu64 " | error: %d.\n", tunnelId, retVal);
            return xpsStatus2SaiStatus(retVal);
        }

        /* Insert the new state */
        retVal = xpsStateInsertData(scope, xpSaiTunnelStateDbHndl, (void*)newEntry);
        if (retVal != XP_NO_ERR)
        {
            xpsStateHeapFree((void*)newEntry);
            XP_SAI_LOG_ERR("xpsStateInsertData: Insertion of data failed, error: %d.\n",
                           retVal);
            return retVal;
        }
    }

    return xpsStatus2SaiStatus(retVal);
}

//Func: xpSaiTunnelIntfAddToBridge

#if 0 //SAI-2731.
static sai_status_t xpSaiTunnelIntfAddToBridge(xpsDevice_t xpsDevId,
                                               xpSaiTunnelInfoT *entry, sai_object_id_t tnlMapOid)
{
    sai_status_t              saiRetVal    = SAI_STATUS_SUCCESS;
    XP_STATUS                 retVal       = XP_NO_ERR;
    xpSaiTunnelMapInfoT      *pTnlMap      = NULL;
    xpSaiTunnelMapEntryInfoT *pTnlMapEntry = NULL;
    uint32_t                  entryCnt     = 0;
    uint32_t                  count        = 0;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    if (NULL == entry)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return SAI_STATUS_FAILURE;
    }

    /* Search for the node in global SAI TUNNEL MAP DB */
    retVal = xpSaiTunnelMapGetCtxDb(xpsDevId, tnlMapOid, &pTnlMap);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiTunnelMapGetCtxDb: Failed to fetch from XPS DB for tnlMapOid: %"
                       PRIu64 " | error: %d.\n", tnlMapOid, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    for (entryCnt = 0; entryCnt < pTnlMap->mapEntryCnt; ++entryCnt)
    {
        /* Search for the node in global SAI TUNNEL MAP ENTRY DB */
        retVal = xpSaiTunnelMapEntryGetCtxDb(xpsDevId, pTnlMap->mapEntryId[entryCnt],
                                             &pTnlMapEntry);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpSaiTunnelMapEntryGetCtxDb: Failed to fetch from XPS DB for tnlMapEntryId: %"
                           PRIu64 " | error: %d.\n",
                           pTnlMap->mapEntryId[entryCnt], retVal);
            return xpsStatus2SaiStatus(retVal);
        }

        for (count = 0; count < entry->brPortCnt; ++count)
        {
            if (!entry->tnlBrPortList[count].isTnlOnBrPortAdded &&
                pTnlMapEntry->bridgeId == entry->tnlBrPortList[count].brId)
            {
                /* Add tunnel interface as bridgePort and program it to the .1D bridge */
                saiRetVal = xpSaiBridgeAddVxTunnel(entry->tnlBrPortList[count].brPortId,
                                                   entry->tnlIntfId, pTnlMapEntry->vniIdKey);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("xpSaiBridgeAddVxTunnel: Failed to add tunnel interface as bridgePort: %"
                                   PRIu64 " and program it to the .1D bridge.\n",
                                   entry->tnlBrPortList[count].brPortId);
                    return saiRetVal;
                }

                entry->tnlBrPortList[count].isTnlOnBrPortAdded = true;
            }
        }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiTunnelIntfRemoveFromBridge

static sai_status_t xpSaiTunnelIntfRemoveFromBridge(xpsDevice_t xpsDevId,
                                                    xpSaiTunnelInfoT *entry)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    uint32_t     count     = 0;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    if (NULL == entry)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return SAI_STATUS_FAILURE;
    }

    /* Remove tunnel interface as bridgePort and program it to the .1D bridge */
    for (count = 0; count < entry->brPortCnt; ++count)
    {
        saiRetVal = xpSaiBridgeRemoveVxTunnel(entry->tnlBrPortList[count].brPortId,
                                              entry->tnlIntfId);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to remove tunnel interface as bridgePort: %" PRIu64
                           " and program it to the .1D bridge.\n",
                           entry->tnlBrPortList[count].brPortId);
            return saiRetVal;
        }
    }

    entry->tnlIntfId = XPS_INTF_INVALID_ID;

    return SAI_STATUS_SUCCESS;
}
//Func: xpSaiTunnelIntfAddToVlan

static sai_status_t xpSaiTunnelIntfAddToVlan(xpsDevice_t xpsDevId,
                                             xpsInterfaceId_t tnlIntfId, sai_object_id_t tnlMapOid)
{
    XP_STATUS                 retVal       = XP_NO_ERR;
    xpSaiTunnelMapInfoT      *pTnlMap      = NULL;
    xpSaiTunnelMapEntryInfoT *pTnlMapEntry = NULL;
    uint32_t                  count        = 0;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    /* Search for the node in global SAI TUNNEL MAP DB */
    retVal = xpSaiTunnelMapGetCtxDb(xpsDevId, tnlMapOid, &pTnlMap);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiTunnelMapGetCtxDb: Failed to fetch from XPS DB for tnlMapOid: %"
                       PRIu64 " | error: %d.\n", tnlMapOid, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    for (count = 0; count < pTnlMap->mapEntryCnt; ++count)
    {
        /* Search for the node in global SAI TUNNEL MAP ENTRY DB */
        retVal = xpSaiTunnelMapEntryGetCtxDb(xpsDevId, pTnlMap->mapEntryId[count],
                                             &pTnlMapEntry);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpSaiTunnelMapEntryGetCtxDb: Failed to fetch from XPS DB for tnlMapEntryId: %"
                           PRIu64 " | error: %d.\n",
                           pTnlMap->mapEntryId[count], retVal);
            return xpsStatus2SaiStatus(retVal);
        }

        /* Add tunnel interface to the tunnel VLAN */
        retVal = xpsVlanAddEndpoint(xpsDevId, pTnlMapEntry->vlanId, tnlIntfId,
                                    XP_L2_ENCAP_VXLAN, pTnlMapEntry->vniIdKey);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpsVlanAddEndpoint: Failed to add endpoint Tunnel: %d to the VLAN: %d, error: %d.\n",
                           tnlIntfId, pTnlMapEntry->vlanId, retVal);
            return xpsStatus2SaiStatus(retVal);
        }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiTunnelIntfRemoveFromVlan

static sai_status_t xpSaiTunnelIntfRemoveFromVlan(xpsDevice_t xpsDevId,
                                                  xpsInterfaceId_t tnlIntfId, sai_object_id_t tnlMapOid)
{
    XP_STATUS                 retVal       = XP_NO_ERR;
    xpSaiTunnelMapInfoT      *pTnlMap      = NULL;
    xpSaiTunnelMapEntryInfoT *pTnlMapEntry = NULL;
    uint32_t                  count        = 0;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    /* Search for the node in global SAI TUNNEL MAP DB */
    retVal = xpSaiTunnelMapGetCtxDb(xpsDevId, tnlMapOid, &pTnlMap);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiTunnelMapGetCtxDb: Failed to fetch from XPS DB for tnlMapOid: %"
                       PRIu64 " | error: %d.\n", tnlMapOid, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    for (count = 0; count < pTnlMap->mapEntryCnt; ++count)
    {
        /* Search for the node in global SAI TUNNEL MAP ENTRY DB */
        retVal = xpSaiTunnelMapEntryGetCtxDb(xpsDevId, pTnlMap->mapEntryId[count],
                                             &pTnlMapEntry);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpSaiTunnelMapEntryGetCtxDb: Failed to fetch from XPS DB for tnlMapEntryId: %"
                           PRIu64 " | error: %d.\n",
                           pTnlMap->mapEntryId[count], retVal);
            return xpsStatus2SaiStatus(retVal);
        }

        /* Remove tunnel interface from the tunnel VLAN */
        retVal = xpsVlanRemoveEndpoint(xpsDevId, pTnlMapEntry->vlanId, tnlIntfId);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpsVlanRemoveEndpoint: Failed to remove endpoint Tunnel: %d from the VLAN: %d, error: %d.\n",
                           tnlIntfId, pTnlMapEntry->vlanId, retVal);
            return xpsStatus2SaiStatus(retVal);
        }
    }

    return SAI_STATUS_SUCCESS;
}
#endif //SAI-2731


sai_status_t xpSaiConvertIpTunnelData(xpSaiTunnelInfoT *pTunnel,
                                      xpsIpTunnelData_t *ipData)
{
    if (pTunnel == NULL || ipData == NULL)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return SAI_STATUS_FAILURE;
    }
    ipData->ecpTtl = pTunnel->ecpTtl;
    ipData->ecpDscp = pTunnel->ecpDscp;

    switch (pTunnel->ecpTtlMode)
    {
        case SAI_TUNNEL_TTL_MODE_UNIFORM_MODEL:
            ipData->ecpTtlMode = XP_IP_TUNNEL_MODE_UNIFORM;
            break;

        case SAI_TUNNEL_TTL_MODE_PIPE_MODEL:
            ipData->ecpTtlMode = XP_IP_TUNNEL_MODE_PIPE;
            break;
        default:
            {
                XP_SAI_LOG_ERR("Invalid input parameter.\n");
                return SAI_STATUS_FAILURE;
            }
    }
    switch (pTunnel->dcpTtlMode)
    {
        case SAI_TUNNEL_TTL_MODE_UNIFORM_MODEL:
            ipData->dcpTtlMode = XP_IP_TUNNEL_MODE_UNIFORM;
            break;

        case SAI_TUNNEL_TTL_MODE_PIPE_MODEL:
            ipData->dcpTtlMode = XP_IP_TUNNEL_MODE_PIPE;
            break;
        default:
            {
                XP_SAI_LOG_ERR("Invalid input parameter.\n");
                return SAI_STATUS_FAILURE;
            }
    }
    switch (pTunnel->ecpDscpMode)
    {
        case SAI_TUNNEL_DSCP_MODE_UNIFORM_MODEL:
            ipData->ecpDscpMode = XP_IP_TUNNEL_MODE_UNIFORM;
            break;

        case SAI_TUNNEL_DSCP_MODE_PIPE_MODEL:
            ipData->ecpDscpMode = XP_IP_TUNNEL_MODE_PIPE;
            break;
        default:
            {
                XP_SAI_LOG_ERR("Invalid input parameter.\n");
                return SAI_STATUS_FAILURE;
            }
    }
    switch (pTunnel->dcpDscpMode)
    {
        case SAI_TUNNEL_DSCP_MODE_UNIFORM_MODEL:
            ipData->dcpDscpMode = XP_IP_TUNNEL_MODE_UNIFORM;
            break;

        case SAI_TUNNEL_DSCP_MODE_PIPE_MODEL:
            ipData->dcpDscpMode = XP_IP_TUNNEL_MODE_PIPE;
            break;
        default:
            {
                XP_SAI_LOG_ERR("Invalid input parameter.\n");
                return SAI_STATUS_FAILURE;
            }
    }
    switch (pTunnel->ecpEcnMode)
    {
        case SAI_TUNNEL_ENCAP_ECN_MODE_STANDARD:
            ipData->ecpEcnMode = XP_IP_TUNNEL_ECN_MODE_STANDARD;
            break;

        case SAI_TUNNEL_ENCAP_ECN_MODE_USER_DEFINED:
            ipData->ecpEcnMode = XP_IP_TUNNEL_ECN_MODE_USER_DEFINED;
            break;
        default:
            {
                XP_SAI_LOG_ERR("Invalid input parameter.\n");
                return SAI_STATUS_FAILURE;
            }
    }
    switch (pTunnel->dcpEcnMode)
    {
        case SAI_TUNNEL_DECAP_ECN_MODE_STANDARD:
            ipData->dcpEcnMode = XP_IP_TUNNEL_ECN_MODE_STANDARD;
            break;

        case SAI_TUNNEL_DECAP_ECN_MODE_COPY_FROM_OUTER:
            ipData->dcpEcnMode = XP_IP_TUNNEL_ECN_MODE_COPY_FROM_OUTER;
            break;
        case SAI_TUNNEL_DECAP_ECN_MODE_USER_DEFINED:
            ipData->dcpEcnMode = XP_IP_TUNNEL_ECN_MODE_USER_DEFINED;
            break;
        default:
            {
                XP_SAI_LOG_ERR("Invalid input parameter.\n");
                return SAI_STATUS_FAILURE;
            }
    }
    return SAI_STATUS_SUCCESS;
}

// Create IpInIp Tunnel
static sai_status_t xpSaiIpInIpTunnelCreate(xpsDevice_t xpsDevId,
                                            xpSaiTunnelEntryInfoT *entry)
{
    sai_status_t        saiRetVal    = SAI_STATUS_SUCCESS;
    XP_STATUS           retVal       = XP_NO_ERR;
    xpSaiTunnelInfoT   *pTunnel      = NULL;
    uint32_t            prefixIdx    = 0xFFFFFFFF;
    xpsL3RouteEntry_t   l3RouteEntry;
    xpsIpTunnelData_t ipData;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    if (NULL == entry)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return SAI_STATUS_FAILURE;
    }

    memset(&ipData, 0, sizeof(xpsIpTunnelData_t));
    ipData.type = XP_IP_OVER_IP_TUNNEL;
    if (entry->dstIpAddr.addr_family == SAI_IP_ADDR_FAMILY_IPV6)
    {
        ipData.lclEpIpAddr.type = XP_PREFIX_TYPE_IPV6;
    }
    else if (entry->dstIpAddr.addr_family == SAI_IP_ADDR_FAMILY_IPV4)
    {
        ipData.lclEpIpAddr.type = XP_PREFIX_TYPE_IPV4;
    }
    else
    {
        XP_SAI_LOG_ERR("Invalid Dst Addr Family %d \n", entry->srcIpAddr.addr_family);
    }

    if (entry->srcIpAddr.addr_family == SAI_IP_ADDR_FAMILY_IPV6)
    {
        ipData.rmtEpIpAddr.type = XP_PREFIX_TYPE_IPV6;
    }
    else if (entry->srcIpAddr.addr_family == SAI_IP_ADDR_FAMILY_IPV4)
    {
        ipData.rmtEpIpAddr.type = XP_PREFIX_TYPE_IPV4;
    }
    else
    {
        XP_SAI_LOG_ERR("Invalid Src Addr Family %d \n", entry->srcIpAddr.addr_family);
    }

    /* Set local IP address for xps tunnel creation */

    xpSaiIpCopy((uint8_t*)&ipData.lclEpIpAddr.addr,
                (uint8_t*)&entry->dstIpAddr.addr,
                (sai_ip_addr_family_t)entry->dstIpAddr.addr_family);

    if (entry->entryType == SAI_TUNNEL_TERM_TABLE_ENTRY_TYPE_P2P)
    {
        /* Set remote IP address for xps tunnel creation */
        xpSaiIpCopy((uint8_t*)&ipData.rmtEpIpAddr.addr,
                    (uint8_t*)&entry->srcIpAddr.addr,
                    (sai_ip_addr_family_t)entry->srcIpAddr.addr_family);
    }

    /* Search for the node in global SAI TUNNEL DB */
    retVal = xpSaiTunnelGetCtxDb(xpsDevId, entry->tnlId, &pTunnel);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiTunnelGetCtxDb: Failed to fetch from XPS DB for tunnelId: %"
                       PRIu64 " | error: %d.\n", entry->tnlId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    saiRetVal = xpSaiConvertIpTunnelData(pTunnel, &ipData);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiConvertIpTunnelData: Failed to Update attr tunnelId: %"
                       PRIu64 " | error: %d.\n", entry->tnlId, saiRetVal);
        return saiRetVal;
    }

    /* 1.Create a IPinIP tunnel interface */
    retVal = xpsIpinIpCreateTunnelInterface(&ipData, &entry->tnlIntfId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsIpinIpCreateTunnelInterface: Failed to create IpinIp Tunnel Interface, error: %d.\n",
                       retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    /* TODO : Functionality Incomplete.
       Handle Tunnel Start and Termination for P2P*/
    if (entry->entryType == SAI_TUNNEL_TERM_TABLE_ENTRY_TYPE_P2P)
    {
        /* Perform src ip lookup to obtain the nhopId */
        memset(&l3RouteEntry, 0, sizeof(xpsL3RouteEntry_t));
        l3RouteEntry.vrfId = (uint32_t)xpSaiObjIdValueGet(entry->vrfId);
        if (ipData.rmtEpIpAddr.type == XP_PREFIX_TYPE_IPV4)
        {
            memcpy(l3RouteEntry.ipv4Addr, ipData.rmtEpIpAddr.addr.ipv4Addr,
                   sizeof(ipv4Addr_t));
        }
        else
        {
            memcpy(l3RouteEntry.ipv6Addr, ipData.rmtEpIpAddr.addr.ipv6Addr,
                   sizeof(ipv6Addr_t));
        }
        retVal = xpsL3FindIpRouteLpmEntry(xpsDevId, &l3RouteEntry, &prefixIdx);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpsL3FindIpRouteLpmEntry: Failed to find IP route entry, error: %d.\n",
                           retVal);
            xpsIpinIpDestroyTunnelInterface(entry->tnlIntfId);
            return xpsStatus2SaiStatus(retVal);
        }

        entry->nextHopId = l3RouteEntry.nhId;

        memset(&entry->nextHopEntry, 0, sizeof(xpsL3NextHopEntry_t));

        /*TODO fetch NH details from SAI NH DB instaead of xps*/
        saiRetVal = xpSaiNextHopGetXpsNextHop(entry->nextHopId, &entry->nextHopEntry);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiNextHopGetXpsNextHop: Failed to get Next Hop entry,nhopId: %d | error: %d.\n",
                           entry->nextHopId, saiRetVal);
            xpsIpinIpDestroyTunnelInterface(entry->tnlIntfId);
            return saiRetVal;
        }

        /*
         * Add a IpInIp tunnel to a device.
         * Add Tunnel Origination and Termination entries.
         */
        retVal = xpsIpinIpAddTunnelEntry(xpsDevId, entry->tnlIntfId);
        if ((retVal != XP_NO_ERR) && (retVal != XP_ERR_KEY_EXISTS))
        {
            XP_SAI_LOG_ERR("xpsIpinIpAddTunnelEntry: Failed to add IpInIp Tunnel entry, tnlIntfId: %d | error: %d.\n",
                           entry->tnlIntfId, retVal);
            xpsIpinIpDestroyTunnelInterface(entry->tnlIntfId);
            return xpsStatus2SaiStatus(retVal);
        }
    }
    /* P2MP Tunnels only terminate the tunnels. */
    else if (entry->entryType == SAI_TUNNEL_TERM_TABLE_ENTRY_TYPE_P2MP)
    {
        retVal = xpsIpTunnelAddTermination(xpsDevId, entry->tnlIntfId,
                                           entry->tnlEntryId);
        if ((retVal != XP_NO_ERR) && (retVal != XP_ERR_KEY_EXISTS))
        {
            XP_SAI_LOG_ERR("xpsIpTunnelAddTermination: Failed to add IpInIp Tunnel entry, tnlIntfId: %d | error: %d.\n",
                           entry->tnlIntfId, retVal);
            xpsIpinIpDestroyTunnelInterface(entry->tnlIntfId);
            return xpsStatus2SaiStatus(retVal);
        }

        XP_SAI_LOG_DBG("xpsIpTunnelAddTermination: Added IpInIp Tunnel entry, tnlIntfId: %d | ret %d.\n",
                       entry->tnlIntfId, retVal);
        //TODO Change Decap model
        pTunnel->tnlIntfId = entry->tnlIntfId;
    }
    else
    {
        XP_SAI_LOG_ERR("Wrong SAI Tunnel termination table entry type passed!\n");
        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    return SAI_STATUS_SUCCESS;
}

xpsVxlanMapType_e xpSaiConvertVxlanMapType(sai_tunnel_map_type_t type)
{
    switch (type)
    {
        case SAI_TUNNEL_MAP_TYPE_VNI_TO_VLAN_ID:
            {
                return VXLAN_MAP_VNI_TO_VLAN;
            }
        case SAI_TUNNEL_MAP_TYPE_VLAN_ID_TO_VNI:
            {
                return VXLAN_MAP_VLAN_TO_VNI;
            }
        case SAI_TUNNEL_MAP_TYPE_VNI_TO_VIRTUAL_ROUTER_ID:
            {
                return VXLAN_MAP_VNI_TO_VRF;
            }
        case SAI_TUNNEL_MAP_TYPE_VIRTUAL_ROUTER_ID_TO_VNI:
            {
                return VXLAN_MAP_VRF_TO_VNI;
            }
        default:
            {
                XP_SAI_LOG_ERR("Wrong SAI Map type :%d \n", (uint32_t)type);
                return VXLAN_MAP_NONE;
            }
    }
    return VXLAN_MAP_NONE;
}

//Func: xpSaiVxlanTunnelCreate

static sai_status_t xpSaiVxlanTunnelCreate(xpsDevice_t xpsDevId,
                                           sai_object_id_t tunnelId)
{
    sai_status_t        saiRetVal    = SAI_STATUS_SUCCESS;
    XP_STATUS           retVal       = XP_NO_ERR;
    xpSaiTunnelInfoT *pTunnel = NULL;
    inetAddr_t localIp, remoteIp;
    xpsInterfaceId_t  l3IntfId  = 0;
    xpsVxlanTunnelConfig_t tunnelConfig;
    xpSaiTunnelMapInfoT      *pTnlMap      = NULL;
    xpSaiTunnelMapEntryInfoT *pTnlMapEntry = NULL;
    xpsVlan_t       brVlanId  = 0;
    uint32_t        vniId     = 0;
    uint32_t        vrfId     = 0;
    uint32_t        count     = 0;
    int32_t         mapType   = 0;
    xpsVniDbEntry_t *vniCtx = NULL;
    uint32_t xpsHwEVlan = 0;
    uint32_t xpsHwEVidx = 0;
    xpsVxlanMapType_e xpsMapType = VXLAN_MAP_NONE;
    xpsScope_t       scopeId    = xpSaiScopeFromDevGet(xpsDevId);
    xpSaiSwitchEntry_t* pSwitchEntry = NULL;
    xpSaiSwitchEntry_t  key;

    memset(&localIp, 0, sizeof(inetAddr_t));
    memset(&remoteIp, 0, sizeof(inetAddr_t));

    retVal = xpSaiTunnelGetCtxDb(xpsDevId, tunnelId, &pTunnel);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiTunnelGetCtxDb: Failed to fetch from XPS DB for tunnelId: %"
                       PRIu64 " | error: %d.\n", tunnelId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    if (pTunnel->tnlSrcIp.addr_family == SAI_IP_ADDR_FAMILY_IPV4)
    {
        localIp.type = remoteIp.type = XP_PREFIX_TYPE_IPV4;
    }
    else
    {
        localIp.type = remoteIp.type = XP_PREFIX_TYPE_IPV6;
    }
    xpSaiIpCopy((uint8_t*)&localIp.addr,
                (uint8_t*)&pTunnel->tnlSrcIp.addr,
                pTunnel->tnlSrcIp.addr_family);
    xpSaiIpCopy((uint8_t*)&remoteIp.addr,
                (uint8_t*)&pTunnel->tnlDstIp.addr,
                pTunnel->tnlDstIp.addr_family);

    bool isLoopbackRif = false;
    saiRetVal = xpSaiGetRouterInterfaceL3IntfId(pTunnel->uLayIntfId, &l3IntfId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        if ((XDK_SAI_OBJID_TYPE_CHECK(pTunnel->uLayIntfId,
                                      SAI_OBJECT_TYPE_ROUTER_INTERFACE)) &&
            xpSaiRouterInterfaceLoopbackIdCheck(pTunnel->uLayIntfId))
        {
            isLoopbackRif = true;
        }
        else
        {
            XP_SAI_LOG_ERR("xpSaiGetRouterInterfaceL3IntfId() failed with saiRetVal : %d\n",
                           saiRetVal);
            return saiRetVal;
        }
    }

    retVal = xpsVxlanCreateTunnelInterface(&localIp, &remoteIp,
                                           &pTunnel->tnlIntfId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsVxlanCreateTunnelInterface: Failed to create VXLAN Tunnel Interface, error: %d.\n",
                       retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    /* P2MP can do termination but not tunnel start.*/
    if (pTunnel->peerMode == SAI_TUNNEL_PEER_MODE_P2MP)
    {
        return SAI_STATUS_SUCCESS;
    }

    xpsL3NextHopEntry_t pL3NextHopEntry;
    xpsL3RouteEntry_t pL3RouteEntry;
    uint32_t prfxBucketIdx = 0;
    uint32_t nhId[64] = {0};
    uint32_t nhIdCnt = 0;

    memset(&pL3RouteEntry, 0, sizeof(xpsL3RouteEntry_t));

    if (isLoopbackRif)
    {
        /*Default Underlay RIF from SONiC is on default RIF.*/
        pL3RouteEntry.vrfId = 0;
    }
    else
    {
        retVal = xpsL3GetIntfVrf(xpsDevId, l3IntfId, &pL3RouteEntry.vrfId);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpsL3GetIntfVrf failed: %d.\n",
                           retVal);
            return xpsStatus2SaiStatus(retVal);
        }
    }

    if (pTunnel->tnlDstIp.addr_family == SAI_IP_ADDR_FAMILY_IPV4)
    {
        pL3RouteEntry.type = XP_PREFIX_TYPE_IPV4;
        xpSaiIpCopy((uint8_t*)&pL3RouteEntry.ipv4Addr,
                    (uint8_t*)&pTunnel->tnlDstIp.addr,
                    pTunnel->tnlDstIp.addr_family);
        pL3RouteEntry.ipMaskLen = 32;
    }
    else
    {
        pL3RouteEntry.type = XP_PREFIX_TYPE_IPV6;
        xpSaiIpCopy((uint8_t*)&pL3RouteEntry.ipv6Addr,
                    (uint8_t*)&pTunnel->tnlDstIp.addr,
                    pTunnel->tnlDstIp.addr_family);
        pL3RouteEntry.ipMaskLen = 128;
    }

    for (; pL3RouteEntry.ipMaskLen>0; pL3RouteEntry.ipMaskLen--)
    {
        retVal = xpsL3FindIpRouteEntry(xpsDevId, &pL3RouteEntry, &prfxBucketIdx);
        if (retVal == XP_NO_ERR)
        {
            break;
        }
    }
    if (pL3RouteEntry.ipMaskLen == 0)
    {
        XP_SAI_LOG_ERR("xpsL3FindIpRouteEntry failed: %d\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    if (pL3RouteEntry.nhEcmpSize == 1)
    {
        nhId[0] = pL3RouteEntry.nhId;
        nhIdCnt = 1;
    }
    else
    {
        /*Tunnel Route via ECMP !!! */
        xpsL3NextHopGroupEntry_t* pNhGrpEntry = NULL;
        retVal = xpsL3GetRouteNextHopGroup(scopeId, pL3RouteEntry.nhId, &pNhGrpEntry);
        if (retVal != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Could not get next hop group from DB, nhGrpId %d, retVal %d",
                  pL3RouteEntry.nhId, retVal);
            return retVal;
        }
        memcpy(nhId, pNhGrpEntry->nextHopIdx, pNhGrpEntry->nhCount*sizeof(uint32_t));
        nhIdCnt = pNhGrpEntry->nhCount;
    }

    /* Iterate for the all NH's*/
    for (uint32_t nhIdx = 0; nhIdx < nhIdCnt; nhIdx++)
    {
        memset(&pL3NextHopEntry, 0, sizeof(xpsL3NextHopEntry_t));
        retVal = xpsL3GetRouteNextHopDb(xpsDevId, nhId[nhIdx], &pL3NextHopEntry);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpsL3FindIpRouteEntry failed: %d. idx : %d \n",
                           retVal, nhId[nhIdx]);
            return xpsStatus2SaiStatus(retVal);
        }

        l3IntfId = pL3NextHopEntry.nextHop.l3InterfaceId;

        retVal = xpsVxlanAddTunnelEntry(xpsDevId, pTunnel->tnlIntfId, l3IntfId);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpsVxlanAddTunnelEntry: Failed , tnlIntfId: %d | error: %d.\n",
                           pTunnel->tnlIntfId, retVal);
            xpsVxlanDestroyTunnelInterface(pTunnel->tnlIntfId);
            return xpsStatus2SaiStatus(retVal);
        }

        memset(&tunnelConfig, 0, sizeof(xpsVxlanTunnelConfig_t));
        tunnelConfig.ttlMode = (pTunnel->ecpTtlMode ==
                                SAI_TUNNEL_TTL_MODE_UNIFORM_MODEL) ? XP_IP_TUNNEL_MODE_UNIFORM :
                               XP_IP_TUNNEL_MODE_PIPE;
        tunnelConfig.ttl = pTunnel->ecpTtl;
        tunnelConfig.dscpMode = (pTunnel->ecpDscpMode ==
                                 SAI_TUNNEL_DSCP_MODE_UNIFORM_MODEL) ? XP_IP_TUNNEL_MODE_UNIFORM :
                                XP_IP_TUNNEL_MODE_PIPE;
        tunnelConfig.dscp = pTunnel->ecpDscp;
#if SAI_API_VERSION >= SAI_VERSION(1,8,1)
        tunnelConfig.udpSrcPortMode = (pTunnel->udpSrcPortMode ==
                                       SAI_TUNNEL_VXLAN_UDP_SPORT_MODE_EPHEMERAL) ? XPS_VXLAN_UDP_PORT_HASH :
                                      XPS_VXLAN_UDP_PORT_STATIC;
#endif
        tunnelConfig.udpSrcPort = pTunnel->udpSrcPort;

        memset(&key, 0, sizeof(key));
        key.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

        retVal = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle,
                                    &key, (void**)&pSwitchEntry);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpsStateSearchData failed with status: %d!\n", retVal);
            return xpsStatus2SaiStatus(retVal);
        }

        if (pSwitchEntry == NULL)
        {
            XP_SAI_LOG_ERR("Could not find switch static variables entry!\n");
            return SAI_STATUS_ITEM_NOT_FOUND;
        }
        tunnelConfig.udpDstPort = pSwitchEntry->vxlan_udp_dst_port;

        COPY_MAC_ADDR_T(&tunnelConfig.dstMacAddr, pL3NextHopEntry.nextHop.macDa);

        xpsInterfaceType_e intfType = XPS_INVALID_IF_TYPE;
        retVal = xpsInterfaceGetType(l3IntfId, &intfType);
        if (retVal)
        {
            XP_SAI_LOG_ERR("Error in xpsInterfaceGetType: error code: %d\n", retVal);
            return xpsStatus2SaiStatus(retVal);
        }
        if (intfType == XPS_VLAN_ROUTER)
        {
            tunnelConfig.vlanId = XPS_INTF_MAP_INTF_TO_BD(
                                      pL3NextHopEntry.nextHop.l3InterfaceId);
        }

        retVal =  xpsVxlanSetTunnelConfig(xpsDevId, pTunnel->tnlIntfId, &tunnelConfig);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpsVxlanSetTunnelConfig failed: %d.\n",
                           retVal);
            return xpsStatus2SaiStatus(retVal);
        }
    }

    /* Map Global ePorts to L2ECMP table and in-turn to primary L2ECMP ePort. */
    retVal =  xpsSetTunnelEcmpConfig(xpsDevId, pTunnel->tnlIntfId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsSetTunnelEcmpConfig failed: %d.\n",
                       retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    for (count = 0; count < pTunnel->ecpMapListCnt; ++count)
    {
        /* Search for the node in global SAI TUNNEL MAP DB */
        retVal = xpSaiTunnelMapGetCtxDb(xpsDevId, pTunnel->ecpMapList[count], &pTnlMap);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpSaiTunnelMapGetCtxDb: Failed %"
                           PRIu64 " | error: %d.\n", pTunnel->ecpMapList[count], retVal);
            return xpsStatus2SaiStatus(retVal);
        }
        mapType = pTnlMap->mapType;

        if (mapType == SAI_TUNNEL_MAP_TYPE_BRIDGE_IF_TO_VNI)
        {
            uint32_t entryCnt = 0;

            for (entryCnt = 0; entryCnt < pTnlMap->mapEntryCnt; ++entryCnt)
            {
                /* Search for the node in global SAI TUNNEL MAP ENTRY DB */
                retVal = xpSaiTunnelMapEntryGetCtxDb(xpsDevId, pTnlMap->mapEntryId[entryCnt],
                                                     &pTnlMapEntry);
                if (retVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("xpSaiTunnelMapEntryGetCtxDb: Failed: %"
                                   PRIu64 " | error: %d.\n", pTnlMap->mapEntryId[entryCnt], retVal);
                    return xpsStatus2SaiStatus(retVal);
                }

                brVlanId = (xpsVlan_t)xpSaiObjIdValueGet(pTnlMapEntry->bridgeIdKey);
                vniId= pTnlMapEntry->vniId;
                if (brVlanId && vniId) {}
            }
        }
        else if (mapType == SAI_TUNNEL_MAP_TYPE_VLAN_ID_TO_VNI)
        {
            uint32_t entryCnt = 0;

            for (entryCnt = 0; entryCnt < pTnlMap->mapEntryCnt; ++entryCnt)
            {
                /* Search for the node in global SAI TUNNEL MAP ENTRY DB */
                retVal = xpSaiTunnelMapEntryGetCtxDb(xpsDevId, pTnlMap->mapEntryId[entryCnt],
                                                     &pTnlMapEntry);
                if (retVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("xpSaiTunnelMapEntryGetCtxDb: Failed to fetch from XPS DB for tnlMapEntryId: %"
                                   PRIu64 " | error: %d.\n", pTnlMap->mapEntryId[entryCnt], retVal);
                    return xpsStatus2SaiStatus(retVal);
                }
                if (pTnlMapEntry->mapType != SAI_TUNNEL_MAP_TYPE_VLAN_ID_TO_VNI)
                {
                    continue;
                }

                brVlanId = pTnlMapEntry->vlanIdKey;
                vniId = pTnlMapEntry->vniId;
                xpsMapType = xpSaiConvertVxlanMapType((sai_tunnel_map_type_t)
                                                      pTnlMapEntry->mapType);
                if (xpsMapType == VXLAN_MAP_NONE)
                {
                    return SAI_STATUS_NOT_SUPPORTED;
                }
                // Not configured in HW
                if (!pTnlMapEntry->isConfgInHW)
                {
                    /* Get/Alloc VNI DB entry */
                    retVal = xpsVniGetDbEntry(scopeId, vniId, &vniCtx);
                    if (retVal == XP_ERR_NOT_FOUND)
                    {
                        retVal = xpsVxlanVniCreateDbEntry(xpsDevId, vniId);
                        if (retVal != XP_NO_ERR)
                        {
                            XP_SAI_LOG_ERR("VniCreate Failed: %"
                                           PRIu64 " | error: %d.\n", pTnlMap->mapEntryId[entryCnt], retVal);
                            return xpsStatus2SaiStatus(retVal);
                        }
                        retVal = xpsVxlanVniCreateInHw(xpsDevId, vniId, &xpsHwEVlan, &xpsHwEVidx, false,
                                                       0);
                        if (retVal != XP_NO_ERR)
                        {
                            XP_SAI_LOG_ERR("VniHWCreate Failed: %"
                                           PRIu64 " | error: %d.\n", pTnlMap->mapEntryId[entryCnt], retVal);
                            return xpsStatus2SaiStatus(retVal);
                        }
                        retVal = xpsVniGetDbEntry(scopeId, vniId, &vniCtx);
                        if (retVal != XP_NO_ERR)
                        {
                            XP_SAI_LOG_ERR("VniGet Failed: %"
                                           PRIu64 " | error: %d.\n", pTnlMap->mapEntryId[entryCnt], retVal);
                            return xpsStatus2SaiStatus(retVal);
                        }

                        vniCtx->hwEVlanId = xpsHwEVlan;
                        vniCtx->hwEVidxId = xpsHwEVidx;
                    }
                    else if (retVal != XP_NO_ERR && retVal != XP_ERR_NOT_FOUND)
                    {
                        XP_SAI_LOG_ERR("VniGet Failed: %"
                                       PRIu64 " | error: %d.\n", pTnlMap->mapEntryId[entryCnt], retVal);
                        return xpsStatus2SaiStatus(retVal);
                    }

                    /*Add Vlan to VNI DB.*/
                    retVal = xpsVxlanAddVlanToVniDbEntry(xpsDevId, vniId,
                                                         xpsMapType, brVlanId);
                    if (retVal != XP_NO_ERR && retVal != XP_ERR_RESOURCE_BUSY)
                    {
                        XP_SAI_LOG_ERR("AddVlanToVniDb Failed: %"
                                       PRIu64 " | error: %d.\n", pTnlMap->mapEntryId[entryCnt], retVal);
                        return xpsStatus2SaiStatus(retVal);
                    }
                    pTnlMapEntry->isConfgInHW = true;
                }
                pTnlMapEntry->refCnt++;

                /*Store VNI info in tunnel. This is needed to add ports to eVlan on
                  member ports add/delete (LAG) over NNI. Add Tunnel members to eVlan */
                retVal = xpsVxlanTunnelAddToVniList(xpsDevId, pTunnel->tnlIntfId, vniId);
                if (retVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("AddVniToTunnelDb Failed: %"
                                   PRIu64 " | error: %d.\n", pTnlMap->mapEntryId[entryCnt], retVal);
                    return xpsStatus2SaiStatus(retVal);
                }
            }
        }
        else if (mapType == SAI_TUNNEL_MAP_TYPE_VIRTUAL_ROUTER_ID_TO_VNI)
        {
            uint32_t entryCnt = 0;

            for (entryCnt = 0; entryCnt < pTnlMap->mapEntryCnt; ++entryCnt)
            {
                /* Search for the node in global SAI TUNNEL MAP ENTRY DB */
                retVal = xpSaiTunnelMapEntryGetCtxDb(xpsDevId, pTnlMap->mapEntryId[entryCnt],
                                                     &pTnlMapEntry);
                if (retVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("xpSaiTunnelMapEntryGetCtxDb: Failed to fetch from XPS DB for tnlMapEntryId: %"
                                   PRIu64 " | error: %d.\n", pTnlMap->mapEntryId[entryCnt], retVal);
                    return xpsStatus2SaiStatus(retVal);
                }
                if (pTnlMapEntry->mapType != SAI_TUNNEL_MAP_TYPE_VIRTUAL_ROUTER_ID_TO_VNI)
                {
                    continue;
                }
                vrfId = pTnlMapEntry->vrfIdKey;
                vniId = pTnlMapEntry->vniId;
                xpsMapType = xpSaiConvertVxlanMapType((sai_tunnel_map_type_t)
                                                      pTnlMapEntry->mapType);
                if (xpsMapType == VXLAN_MAP_NONE)
                {
                    return SAI_STATUS_NOT_SUPPORTED;
                }
                // Not configured in HW
                if (!pTnlMapEntry->isConfgInHW)
                {
                    /* Get/Alloc VNI DB entry */
                    retVal = xpsVniGetDbEntry(scopeId, vniId, &vniCtx);
                    if (retVal == XP_ERR_NOT_FOUND)
                    {
                        retVal = xpsVxlanVniCreateDbEntry(xpsDevId, vniId);
                        if (retVal != XP_NO_ERR)
                        {
                            XP_SAI_LOG_ERR("VniCreate Failed: %"
                                           PRIu64 " | error: %d.\n", pTnlMap->mapEntryId[entryCnt], retVal);
                            return xpsStatus2SaiStatus(retVal);
                        }
                        retVal = xpsVxlanVniCreateInHw(xpsDevId, vniId, &xpsHwEVlan, &xpsHwEVidx, true,
                                                       vrfId);
                        if (retVal != XP_NO_ERR)
                        {
                            XP_SAI_LOG_ERR("VniHWCreate Failed: %"
                                           PRIu64 " | error: %d.\n", pTnlMap->mapEntryId[entryCnt], retVal);
                            return xpsStatus2SaiStatus(retVal);
                        }
                        retVal = xpsVniGetDbEntry(scopeId, vniId, &vniCtx);
                        if (retVal != XP_NO_ERR)
                        {
                            XP_SAI_LOG_ERR("VniGet Failed: %"
                                           PRIu64 " | error: %d.\n", pTnlMap->mapEntryId[entryCnt], retVal);
                            return xpsStatus2SaiStatus(retVal);
                        }

                        vniCtx->hwEVlanId = xpsHwEVlan;
                        vniCtx->hwEVidxId = xpsHwEVidx;


                        vniCtx->isL3Vni = true;
                        vniCtx->vrfId = vrfId;
                    }
                    else if (retVal != XP_NO_ERR && retVal != XP_ERR_NOT_FOUND)
                    {
                        XP_SAI_LOG_ERR("VniGet Failed: %"
                                       PRIu64 " | error: %d.\n", pTnlMap->mapEntryId[entryCnt], retVal);
                        return xpsStatus2SaiStatus(retVal);
                    }

                    /* Get/Alloc VNI DB entry */
                    retVal = xpsVniGetDbEntry(scopeId, vniId, &vniCtx);
                    if (retVal != XP_NO_ERR)
                    {
                        XP_SAI_LOG_ERR("VniGet Failed: %"
                                       PRIu64 " | error: %d.\n", pTnlMap->mapEntryId[entryCnt], retVal);
                        return xpsStatus2SaiStatus(retVal);
                    }
                    vniCtx->refCnt++;
                    pTnlMapEntry->isConfgInHW = true;
                }
                pTnlMapEntry->refCnt++;

                /*Store VNI info in tunnel. This is needed to add ports to eVlan on
                  member ports add/delete (LAG) over NNI. Add Tunnel members to eVlan */
                retVal = xpsVxlanTunnelAddToVniList(xpsDevId, pTunnel->tnlIntfId, vniId);
                if (retVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("AddVniToTunnelDb Failed: %"
                                   PRIu64 " | error: %d.\n", pTnlMap->mapEntryId[entryCnt], retVal);
                    return xpsStatus2SaiStatus(retVal);
                }
            }
        }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiVxlanTunnelRemove

/*
NOTE: Tunnel wll be added to VNI when tunnel bridgePortOid
added to Vlan.
Make sure to remove tunnel member from Vlan before tunnel delete.
 */
static sai_status_t xpSaiVxlanTunnelRemove(xpsDevice_t xpsDevId,
                                           sai_object_id_t tunnelId)
{
    XP_STATUS           retVal       = XP_NO_ERR;
    xpSaiTunnelInfoT *pTunnel = NULL;
    xpSaiTunnelMapInfoT      *pTnlMap      = NULL;
    xpSaiTunnelMapEntryInfoT *pTnlMapEntry = NULL;
    int32_t                   mapType      = 0;
    xpsVlan_t       brVlanId  = 0;
    uint32_t        vniId     = 0;
    //uint32_t        vrfId     = 0;
    xpsVniDbEntry_t *vniCtx = NULL;
    xpsVxlanMapType_e xpsMapType = VXLAN_MAP_NONE;
    xpsScope_t       scopeId    = xpSaiScopeFromDevGet(xpsDevId);

    retVal = xpSaiTunnelGetCtxDb(xpsDevId, tunnelId, &pTunnel);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiTunnelGetCtxDb: Failed to fetch from XPS DB for tunnelId: %"
                       PRIu64 " | error: %d.\n", tunnelId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    /* Iterate encapList and remove VNI to VLAN mapping from VNI DB*/
    for (uint32_t count = 0; count < pTunnel->ecpMapListCnt; ++count)
    {
        /* Search for the node in global SAI TUNNEL MAP DB */
        retVal = xpSaiTunnelMapGetCtxDb(xpsDevId, pTunnel->ecpMapList[count], &pTnlMap);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpSaiTunnelMapGetCtxDb: Failed to fetch from XPS DB for tnlMapId: %"
                           PRIu64 " | error: %d.\n", pTunnel->dcpMapList[count], retVal);
            return xpsStatus2SaiStatus(retVal);
        }
        mapType = pTnlMap->mapType;

        if (mapType == SAI_TUNNEL_MAP_TYPE_BRIDGE_IF_TO_VNI)
        {

        }
        else if (mapType == SAI_TUNNEL_MAP_TYPE_VLAN_ID_TO_VNI)
        {
            uint32_t entryCnt = 0;

            for (entryCnt = 0; entryCnt < pTnlMap->mapEntryCnt; ++entryCnt)
            {
                /* Search for the node in global SAI TUNNEL MAP ENTRY DB */
                retVal = xpSaiTunnelMapEntryGetCtxDb(xpsDevId, pTnlMap->mapEntryId[entryCnt],
                                                     &pTnlMapEntry);
                if (retVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("xpSaiTunnelMapEntryGetCtxDb: Failed %"
                                   PRIu64 " | error: %d.\n", pTnlMap->mapEntryId[entryCnt], retVal);
                    return xpsStatus2SaiStatus(retVal);
                }
                if (pTnlMapEntry->mapType != SAI_TUNNEL_MAP_TYPE_VLAN_ID_TO_VNI)
                {
                    continue;
                }

                vniId = pTnlMapEntry->vniId;
                brVlanId = pTnlMapEntry->vlanIdKey;
                xpsMapType = xpSaiConvertVxlanMapType((sai_tunnel_map_type_t)
                                                      pTnlMapEntry->mapType);
                if (xpsMapType == VXLAN_MAP_NONE)
                {
                    return SAI_STATUS_NOT_SUPPORTED;
                }
                // Not configured in HW
                if (!pTnlMapEntry->isConfgInHW)
                {
                    XP_SAI_LOG_ERR("pTnlMapEntry Not HW configured in HW %"
                                   PRIu64 " | error: %d.\n", pTnlMap->mapEntryId[entryCnt], retVal);
                    return SAI_STATUS_FAILURE;
                }

                /* Remove VNI info in tunnel. This is needed to add ports to eVlan on
                   member ports add/delete (LAG) over NNI */
                retVal = xpsVxlanTunnelDelFromVniList(xpsDevId, pTunnel->tnlIntfId, vniId);
                if (retVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("AddVniToTunnelDb Failed: %"
                                   PRIu64 " | error: %d.\n", pTnlMap->mapEntryId[entryCnt], retVal);
                    return xpsStatus2SaiStatus(retVal);
                }
                /*
                  Note that Tunnel eVlan membership is cleaned-up during VNI delete.
                  But Tunnel ePorts from L2MLL will be removed during tnl-bridge port member
                  remove. Sine multiple tunnels can be on same RIF (underlying port),
                  it is modeled to remove membership during eVlan delete, else we need to
                  keep ref-cnt per VNI to Tnl list on ports, which complicates the design.
                  Since ePorts are removed L2MLL pair, flooding will not be an issue.
                  */
                //Clean-up the VNI for the last tunnel.
                if (pTnlMapEntry->refCnt == 1)
                {
                    /* Get VNI DB entry */
                    retVal = xpsVniGetDbEntry(scopeId, vniId, &vniCtx);
                    if (retVal != XP_NO_ERR)
                    {
                        XP_SAI_LOG_ERR("VniGet Failed: %"
                                       PRIu64 " | error: %d.\n", pTnlMap->mapEntryId[entryCnt], retVal);
                        return xpsStatus2SaiStatus(retVal);
                    }

                    /*Remove Vlan to VNI DB.*/
                    retVal = xpsVxlanRemoveVlanToVniDbEntry(xpsDevId, vniId,
                                                            xpsMapType, brVlanId);
                    if (retVal != XP_NO_ERR && retVal != XP_ERR_RESOURCE_BUSY)
                    {
                        XP_SAI_LOG_ERR("RemoveVlanToVniDb Failed: %"
                                       PRIu64 " | error: %d.\n", pTnlMap->mapEntryId[entryCnt], retVal);
                        return xpsStatus2SaiStatus(retVal);
                    }
                    if (vniCtx->refCnt == 0)
                    {
                        retVal = xpsVxlanVniDeleteInHw(xpsDevId, vniId, vniCtx->hwEVlanId,
                                                       vniCtx->hwEVidxId, false);
                        if (retVal != XP_NO_ERR)
                        {
                            XP_SAI_LOG_ERR("RemoveVxlanVniHW Failed: %"
                                           PRIu64 " | error: %d.\n", pTnlMap->mapEntryId[entryCnt], retVal);
                            return xpsStatus2SaiStatus(retVal);
                        }
                        retVal = xpsVxlanVniDeleteDbEntry(xpsDevId, vniId);
                        if (retVal != XP_NO_ERR)
                        {
                            XP_SAI_LOG_ERR("RemoveVxlanVniDb Failed: %"
                                           PRIu64 " | error: %d.\n", pTnlMap->mapEntryId[entryCnt], retVal);
                            return xpsStatus2SaiStatus(retVal);
                        }
                    }
                }

                pTnlMapEntry->refCnt--;
                if (pTnlMapEntry->refCnt == 0)
                {
                    pTnlMapEntry->isConfgInHW = false;
                }
            }
        }
        else if (mapType == SAI_TUNNEL_MAP_TYPE_VIRTUAL_ROUTER_ID_TO_VNI)
        {
            uint32_t entryCnt = 0;

            for (entryCnt = 0; entryCnt < pTnlMap->mapEntryCnt; ++entryCnt)
            {
                /* Search for the node in global SAI TUNNEL MAP ENTRY DB */
                retVal = xpSaiTunnelMapEntryGetCtxDb(xpsDevId, pTnlMap->mapEntryId[entryCnt],
                                                     &pTnlMapEntry);
                if (retVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("xpSaiTunnelMapEntryGetCtxDb: Failed %"
                                   PRIu64 " | error: %d.\n", pTnlMap->mapEntryId[entryCnt], retVal);
                    return xpsStatus2SaiStatus(retVal);
                }
                if (pTnlMapEntry->mapType != SAI_TUNNEL_MAP_TYPE_VIRTUAL_ROUTER_ID_TO_VNI)
                {
                    continue;
                }

                vniId = pTnlMapEntry->vniId;
                // vrfId = pTnlMapEntry->vrfIdKey;
                xpsMapType = xpSaiConvertVxlanMapType((sai_tunnel_map_type_t)
                                                      pTnlMapEntry->mapType);
                if (xpsMapType == VXLAN_MAP_NONE)
                {
                    return SAI_STATUS_NOT_SUPPORTED;
                }
                // Not configured in HW
                if (!pTnlMapEntry->isConfgInHW)
                {
                    XP_SAI_LOG_ERR("pTnlMapEntry Not HW configured in HW %"
                                   PRIu64 " | error: %d.\n", pTnlMap->mapEntryId[entryCnt], retVal);
                    return SAI_STATUS_FAILURE;
                }

                /* Remove VNI info in tunnel. This is needed to add ports to eVlan on
                   member ports add/delete (LAG) over NNI */
                retVal = xpsVxlanTunnelDelFromVniList(xpsDevId, pTunnel->tnlIntfId, vniId);
                if (retVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("AddVniToTunnelDb Failed: %"
                                   PRIu64 " | error: %d.\n", pTnlMap->mapEntryId[entryCnt], retVal);
                    return xpsStatus2SaiStatus(retVal);
                }
                /*
                  Note that Tunnel eVlan membership is cleaned-up during VNI delete.
                  But Tunnel ePorts from L2MLL will be removed during tnl-bridge port member
                  remove. Sine multiple tunnels can be on same RIF (underlying port),
                  it is modeled to remove membership during eVlan delete, else we need to
                  keep ref-cnt per VNI to Tnl list on ports, which complicates the design.
                  Since ePorts are removed L2MLL pair, flooding will not be an issue.
                  */
                //Clean-up the VNI for the last tunnel.
                if (pTnlMapEntry->refCnt == 1)
                {
                    /* Get VNI DB entry */
                    retVal = xpsVniGetDbEntry(scopeId, vniId, &vniCtx);
                    if (retVal != XP_NO_ERR)
                    {
                        XP_SAI_LOG_ERR("VniGet Failed: %"
                                       PRIu64 " | error: %d.\n", pTnlMap->mapEntryId[entryCnt], retVal);
                        return xpsStatus2SaiStatus(retVal);
                    }

                    vniCtx->refCnt--;
                    if (vniCtx->refCnt == 0)
                    {


                        retVal = xpsVxlanVniDeleteInHw(xpsDevId, vniId, vniCtx->hwEVlanId,
                                                       vniCtx->hwEVidxId, true);
                        if (retVal != XP_NO_ERR)
                        {
                            XP_SAI_LOG_ERR("RemoveVxlanVniHW Failed: %"
                                           PRIu64 " | error: %d.\n", pTnlMap->mapEntryId[entryCnt], retVal);
                            return xpsStatus2SaiStatus(retVal);
                        }
                        retVal = xpsVxlanVniDeleteDbEntry(xpsDevId, vniId);
                        if (retVal != XP_NO_ERR)
                        {
                            XP_SAI_LOG_ERR("RemoveVxlanVniDb Failed: %"
                                           PRIu64 " | error: %d.\n", pTnlMap->mapEntryId[entryCnt], retVal);
                            return xpsStatus2SaiStatus(retVal);
                        }
                    }
                }

                pTnlMapEntry->refCnt--;
                if (pTnlMapEntry->refCnt == 0)
                {
                    pTnlMapEntry->isConfgInHW = false;
                }
            }
        }
    }

    retVal = xpsVxlanRemoveTunnelEntry(xpsDevId, pTunnel->tnlIntfId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsVxlanRemoveTunnelEntry: Failed to remove VXLAN Tunnel entry, tnlIntfId: %d | error: %d.\n",
                       pTunnel->tnlIntfId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    retVal = xpsVxlanDestroyTunnelInterface(pTunnel->tnlIntfId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsVxlanDestroyTunnelInterface: Failed to remove VXLAN Tunnel Interface, error: %d.\n",
                       retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t xpSaiVxlanTunnelTerminationCreate(xpsDevice_t xpsDevId,
                                                      xpSaiTunnelEntryInfoT *entry)
{
    XP_STATUS           retVal       = XP_NO_ERR;
    xpSaiTunnelInfoT   *pTunnel      = NULL;
    uint32_t            count        = 0;
    int32_t             mapType      = 0;
    inetAddr_t srcIp, dstIp;
    xpSaiTunnelMapInfoT      *pTnlMap      = NULL;
    xpSaiTunnelMapEntryInfoT *pTnlMapEntry = NULL;
    xpsVlan_t       brVlanId  = 0;
    uint32_t        vniId     = 0;
    uint32_t        vrfId     = 0;
    xpsVniDbEntry_t *vniCtx = NULL;
    uint32_t xpsHwEVlan = 0;
    uint32_t xpsHwEVidx = 0;
    xpsVxlanMapType_e xpsMapType = VXLAN_MAP_NONE;
    XpIpTunnelMode_t ttlMode;
    XpIpTunnelMode_t dscpMode;
    xpsScope_t       scopeId    = xpSaiScopeFromDevGet(xpsDevId);
    xpSaiSwitchEntry_t* pSwitchEntry = NULL;
    xpSaiSwitchEntry_t  key;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    if (NULL == entry)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return SAI_STATUS_FAILURE;
    }

    memset(&key, 0, sizeof(key));
    key.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

    retVal = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle,
                                &key, (void**)&pSwitchEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateSearchData failed with status: %d!\n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    if (pSwitchEntry == NULL)
    {
        XP_SAI_LOG_ERR("Could not find switch static variables entry!\n");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    memset(&srcIp, 0, sizeof(inetAddr_t));
    memset(&dstIp, 0, sizeof(inetAddr_t));

    retVal = xpSaiTunnelGetCtxDb(xpsDevId, entry->tnlId, &pTunnel);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiTunnelGetCtxDb: Failed to fetch from XPS DB for tunnelId: %"
                       PRIu64 " | error: %d.\n", entry->tnlId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    if (entry->dstIpAddr.addr_family == SAI_IP_ADDR_FAMILY_IPV4)
    {
        srcIp.type = dstIp.type = XP_PREFIX_TYPE_IPV4;
    }
    else
    {
        srcIp.type = dstIp.type = XP_PREFIX_TYPE_IPV6;
    }

    xpSaiIpCopy((uint8_t*)&dstIp.addr,
                (uint8_t*)&entry->dstIpAddr.addr,
                (sai_ip_addr_family_t)entry->dstIpAddr.addr_family);

    if (entry->entryType == SAI_TUNNEL_TERM_TABLE_ENTRY_TYPE_P2P)
    {
        xpSaiIpCopy((uint8_t*)&srcIp.addr,
                    (uint8_t*)&entry->srcIpAddr.addr,
                    (sai_ip_addr_family_t)entry->srcIpAddr.addr_family);
    }

    ttlMode = (pTunnel->dcpTtlMode == SAI_TUNNEL_TTL_MODE_UNIFORM_MODEL) ?
              XP_IP_TUNNEL_MODE_UNIFORM : XP_IP_TUNNEL_MODE_PIPE;
    dscpMode = (pTunnel->dcpDscpMode == SAI_TUNNEL_TTL_MODE_UNIFORM_MODEL) ?
               XP_IP_TUNNEL_MODE_UNIFORM : XP_IP_TUNNEL_MODE_PIPE;

    for (count = 0; count < pTunnel->dcpMapListCnt; ++count)
    {
        /* Search for the node in global SAI TUNNEL MAP DB */
        retVal = xpSaiTunnelMapGetCtxDb(xpsDevId, pTunnel->dcpMapList[count], &pTnlMap);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpSaiTunnelMapGetCtxDb: Failed to fetch from XPS DB for tnlMapId: %"
                           PRIu64 " | error: %d.\n", pTunnel->dcpMapList[count], retVal);
            return xpsStatus2SaiStatus(retVal);
        }
        mapType = pTnlMap->mapType;

        if (mapType == SAI_TUNNEL_MAP_TYPE_VNI_TO_BRIDGE_IF)
        {
            uint32_t entryCnt = 0;
            for (entryCnt = 0; entryCnt < pTnlMap->mapEntryCnt; ++entryCnt)
            {
                /* Search for the node in global SAI TUNNEL MAP ENTRY DB */
                retVal = xpSaiTunnelMapEntryGetCtxDb(xpsDevId, pTnlMap->mapEntryId[entryCnt],
                                                     &pTnlMapEntry);
                if (retVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("xpSaiTunnelMapEntryGetCtxDb: Failed to fetch from XPS DB for tnlMapEntryId: %"
                                   PRIu64 " | error: %d.\n", pTnlMap->mapEntryId[entryCnt], retVal);
                    return xpsStatus2SaiStatus(retVal);
                }
                vniId = pTnlMapEntry->vniIdKey;
                brVlanId = (xpsVlan_t)xpSaiObjIdValueGet(pTnlMapEntry->bridgeId);

            }
        }
        else if (mapType == SAI_TUNNEL_MAP_TYPE_VNI_TO_VLAN_ID)
        {
            uint32_t entryCnt = 0;

            for (entryCnt = 0; entryCnt < pTnlMap->mapEntryCnt; ++entryCnt)
            {
                /* Search for the node in global SAI TUNNEL MAP ENTRY DB */
                retVal = xpSaiTunnelMapEntryGetCtxDb(xpsDevId, pTnlMap->mapEntryId[entryCnt],
                                                     &pTnlMapEntry);
                if (retVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("xpSaiTunnelMapEntryGetCtxDb: Failed to fetch from XPS DB for tnlMapEntryId: %"
                                   PRIu64 " | error: %d.\n", pTnlMap->mapEntryId[entryCnt], retVal);
                    return xpsStatus2SaiStatus(retVal);
                }
                if (pTnlMapEntry->mapType != SAI_TUNNEL_MAP_TYPE_VNI_TO_VLAN_ID)
                {
                    continue;
                }

                vniId = pTnlMapEntry->vniIdKey;
                brVlanId = pTnlMapEntry->vlanId;
                xpsMapType = xpSaiConvertVxlanMapType((sai_tunnel_map_type_t)
                                                      pTnlMapEntry->mapType);
                if (xpsMapType == VXLAN_MAP_NONE)
                {
                    return SAI_STATUS_NOT_SUPPORTED;
                }
                // Not configured in HW
                if (!pTnlMapEntry->isConfgInHW)
                {
                    /* Get/Alloc VNI DB entry */
                    retVal = xpsVniGetDbEntry(scopeId, vniId, &vniCtx);
                    if (retVal == XP_ERR_NOT_FOUND)
                    {
                        retVal = xpsVxlanVniCreateDbEntry(xpsDevId, vniId);
                        if (retVal != XP_NO_ERR)
                        {
                            XP_SAI_LOG_ERR("VniCreate Failed: %"
                                           PRIu64 " | error: %d.\n", pTnlMap->mapEntryId[entryCnt], retVal);
                            return xpsStatus2SaiStatus(retVal);
                        }
                        retVal = xpsVxlanVniCreateInHw(xpsDevId, vniId, &xpsHwEVlan, &xpsHwEVidx, false,
                                                       0);
                        if (retVal != XP_NO_ERR)
                        {
                            XP_SAI_LOG_ERR("VniHWCreate Failed: %"
                                           PRIu64 " | error: %d.\n", pTnlMap->mapEntryId[entryCnt], retVal);
                            return xpsStatus2SaiStatus(retVal);
                        }
                        retVal = xpsVniGetDbEntry(scopeId, vniId, &vniCtx);
                        if (retVal != XP_NO_ERR)
                        {
                            XP_SAI_LOG_ERR("VniGet Failed: %"
                                           PRIu64 " | error: %d.\n", pTnlMap->mapEntryId[entryCnt], retVal);
                            return xpsStatus2SaiStatus(retVal);
                        }
                        vniCtx->hwEVlanId = xpsHwEVlan;
                        vniCtx->hwEVidxId = xpsHwEVidx;
                    }
                    else if (retVal != XP_NO_ERR && retVal != XP_ERR_NOT_FOUND)
                    {
                        XP_SAI_LOG_ERR("VniGet Failed: %"
                                       PRIu64 " | error: %d.\n", pTnlMap->mapEntryId[entryCnt], retVal);
                        return xpsStatus2SaiStatus(retVal);
                    }

                    /*Add Vlan to VNI DB.*/
                    retVal = xpsVxlanAddVlanToVniDbEntry(xpsDevId, vniId,
                                                         xpsMapType, brVlanId);
                    if (retVal != XP_NO_ERR && retVal != XP_ERR_RESOURCE_BUSY)
                    {
                        XP_SAI_LOG_ERR("AddVlanToVniDb Failed: %"
                                       PRIu64 " | error: %d.\n", pTnlMap->mapEntryId[entryCnt], retVal);
                        return xpsStatus2SaiStatus(retVal);
                    }
                    pTnlMapEntry->isConfgInHW = true;
                }
                pTnlMapEntry->refCnt++;

                retVal = xpsVxlanAddTunnelTermination(xpsDevId, pTunnel->tnlIntfId,
                                                      entry->tnlEntryId, &srcIp,
                                                      &dstIp, vniId, brVlanId, ttlMode,
                                                      dscpMode, pSwitchEntry->vxlan_udp_dst_port);
                if (retVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("xpsVxlanAddTunnelTermination, error %d\n", retVal);
                }
                entry->count++;
            }
        }
        else if (mapType == SAI_TUNNEL_MAP_TYPE_VNI_TO_VIRTUAL_ROUTER_ID)
        {
            uint32_t entryCnt = 0;

            for (entryCnt = 0; entryCnt < pTnlMap->mapEntryCnt; ++entryCnt)
            {
                /* Search for the node in global SAI TUNNEL MAP ENTRY DB */
                retVal = xpSaiTunnelMapEntryGetCtxDb(xpsDevId, pTnlMap->mapEntryId[entryCnt],
                                                     &pTnlMapEntry);
                if (retVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("xpSaiTunnelMapEntryGetCtxDb: Failed to fetch from XPS DB for tnlMapEntryId: %"
                                   PRIu64 " | error: %d.\n", pTnlMap->mapEntryId[entryCnt], retVal);
                    return xpsStatus2SaiStatus(retVal);
                }
                if (pTnlMapEntry->mapType != SAI_TUNNEL_MAP_TYPE_VNI_TO_VIRTUAL_ROUTER_ID)
                {
                    continue;
                }

                vniId = pTnlMapEntry->vniIdKey;
                vrfId = pTnlMapEntry->vrfId;
                xpsMapType = xpSaiConvertVxlanMapType((sai_tunnel_map_type_t)
                                                      pTnlMapEntry->mapType);
                if (xpsMapType == VXLAN_MAP_NONE)
                {
                    return SAI_STATUS_NOT_SUPPORTED;
                }
                // Not configured in HW
                if (!pTnlMapEntry->isConfgInHW)
                {
                    /* Get/Alloc VNI DB entry */
                    retVal = xpsVniGetDbEntry(scopeId, vniId, &vniCtx);
                    if (retVal == XP_ERR_NOT_FOUND)
                    {
                        retVal = xpsVxlanVniCreateDbEntry(xpsDevId, vniId);
                        if (retVal != XP_NO_ERR)
                        {
                            XP_SAI_LOG_ERR("VniCreate Failed: %"
                                           PRIu64 " | error: %d.\n", pTnlMap->mapEntryId[entryCnt], retVal);
                            return xpsStatus2SaiStatus(retVal);
                        }
                        retVal = xpsVxlanVniCreateInHw(xpsDevId, vniId, &xpsHwEVlan, &xpsHwEVidx, true,
                                                       vrfId);
                        if (retVal != XP_NO_ERR)
                        {
                            XP_SAI_LOG_ERR("VniHWCreate Failed: %"
                                           PRIu64 " | error: %d.\n", pTnlMap->mapEntryId[entryCnt], retVal);
                            return xpsStatus2SaiStatus(retVal);
                        }
                        retVal = xpsVniGetDbEntry(scopeId, vniId, &vniCtx);
                        if (retVal != XP_NO_ERR)
                        {
                            XP_SAI_LOG_ERR("VniGet Failed: %"
                                           PRIu64 " | error: %d.\n", pTnlMap->mapEntryId[entryCnt], retVal);
                            return xpsStatus2SaiStatus(retVal);
                        }
                        vniCtx->hwEVlanId = xpsHwEVlan;
                        vniCtx->hwEVidxId = xpsHwEVidx;
                        vniCtx->isL3Vni = true;
                        vniCtx->vrfId = vrfId;
                    }
                    else if (retVal != XP_NO_ERR && retVal != XP_ERR_NOT_FOUND)
                    {
                        XP_SAI_LOG_ERR("VniGet Failed: %"
                                       PRIu64 " | error: %d.\n", pTnlMap->mapEntryId[entryCnt], retVal);
                        return xpsStatus2SaiStatus(retVal);
                    }

                    /* Get/Alloc VNI DB entry */
                    retVal = xpsVniGetDbEntry(scopeId, vniId, &vniCtx);
                    if (retVal != XP_NO_ERR)
                    {
                        XP_SAI_LOG_ERR("VniGet Failed: %"
                                       PRIu64 " | error: %d.\n", pTnlMap->mapEntryId[entryCnt], retVal);
                        return xpsStatus2SaiStatus(retVal);
                    }
                    vniCtx->refCnt++;
                    pTnlMapEntry->isConfgInHW = true;
                }
                pTnlMapEntry->refCnt++;

                retVal = xpsVxlanAddTunnelTermination(xpsDevId, pTunnel->tnlIntfId,
                                                      entry->tnlEntryId, &srcIp,
                                                      &dstIp, vniId, brVlanId, ttlMode,
                                                      dscpMode, pSwitchEntry->vxlan_udp_dst_port);
                if (retVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("xpsVxlanAddTunnelTermination, error %d\n", retVal);
                }
                entry->count++;
            }
        }
    }

    return SAI_STATUS_SUCCESS;
}

//Remove IpInIp Tunnel
static sai_status_t xpSaiIpinIpTunnelRemove(xpsDevice_t xpsDevId,
                                            xpSaiTunnelEntryInfoT *entry)
{
    XP_STATUS         retVal    = XP_NO_ERR;
    xpSaiTunnelInfoT *pTunnel   = NULL;
    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    if (NULL == entry)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return SAI_STATUS_FAILURE;
    }

    if (entry->tnlIntfId == XPS_INTF_INVALID_ID)
    {
        return SAI_STATUS_SUCCESS;
    }

    if (entry->entryType == SAI_TUNNEL_TERM_TABLE_ENTRY_TYPE_P2P)
    {
        /* Search for the node in global SAI TUNNEL DB */
        retVal = xpSaiTunnelGetCtxDb(xpsDevId, entry->tnlId, &pTunnel);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpSaiTunnelGetCtxDb: Failed to fetch from XPS DB for tunnelId: %"
                           PRIu64 " | error: %d.\n", entry->tnlId, retVal);
            return xpsStatus2SaiStatus(retVal);
        }

        /* Remove a IpInIp tunnel entry from a device */
        retVal = xpsIpinIpRemoveTunnelEntry(xpsDevId, entry->tnlIntfId);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpsIpinIpRemoveTunnelEntry: Failed to remove IPinIP Tunnel entry, tnlIntfId: %d | error: %d.\n",
                           entry->tnlIntfId, retVal);
            return xpsStatus2SaiStatus(retVal);
        }
    }
    else if (entry->entryType == SAI_TUNNEL_TERM_TABLE_ENTRY_TYPE_P2MP)
    {
        retVal = xpsIpTunnelDeleteTermination(xpsDevId, entry->tnlIntfId,
                                              entry->tnlEntryId);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpsIpTunnelDeleteTermination: Failed to remove P2MP IPinIP Tunnel entry, tnlIntfId: %d | error: %d.\n",
                           entry->tnlIntfId, retVal);
            return xpsStatus2SaiStatus(retVal);
        }
    }
    else
    {
        XP_SAI_LOG_ERR("Wrong SAI Tunnel termination table entry type passed!\n");
        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }
    /* Destroy a IPinIp tunnel interface */
    retVal = xpsIpinIpDestroyTunnelInterface(entry->tnlIntfId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsIpinIpDestroyTunnelInterface: Failed to destroy IpinIP Tunnel Interface, tnlIntfId: %d | error: %d.\n",
                       entry->tnlIntfId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    entry->tnlIntfId = XPS_INTF_INVALID_ID;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiVxlanTunnelTerminationRemove

static sai_status_t xpSaiVxlanTunnelTerminationRemove(xpsDevice_t xpsDevId,
                                                      xpSaiTunnelEntryInfoT *entry)
{
    XP_STATUS   retVal    = XP_NO_ERR;
    uint32_t    count     = 0;
    bool        isIpv6    = false;
    xpSaiTunnelInfoT *pTunnel      = NULL;
    xpSaiTunnelMapInfoT      *pTnlMap      = NULL;
    xpSaiTunnelMapEntryInfoT *pTnlMapEntry = NULL;
    int32_t                   mapType      = 0;
    xpsVlan_t       brVlanId  = 0;
    uint32_t        vniId     = 0;
    //uint32_t        vrfId     = 0;
    xpsVniDbEntry_t *vniCtx = NULL;
    xpsVxlanMapType_e xpsMapType = VXLAN_MAP_NONE;
    xpsScope_t       scopeId    = xpSaiScopeFromDevGet(xpsDevId);

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    if (NULL == entry)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return SAI_STATUS_FAILURE;
    }

    retVal = xpSaiTunnelGetCtxDb(xpsDevId, entry->tnlId, &pTunnel);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiTunnelGetCtxDb: Failed to fetch from XPS DB for tunnelId: %"
                       PRIu64 " | error: %d.\n", entry->tnlId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    if (entry->dstIpAddr.addr_family == SAI_IP_ADDR_FAMILY_IPV6)
    {
        isIpv6 = true;
    }

    /* Iterate DecapList and remove VNI to VLAN mapping from VNI DB*/
    for (count = 0; count < pTunnel->dcpMapListCnt; ++count)
    {
        /* Search for the node in global SAI TUNNEL MAP DB */
        retVal = xpSaiTunnelMapGetCtxDb(xpsDevId, pTunnel->dcpMapList[count], &pTnlMap);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpSaiTunnelMapGetCtxDb: Failed to fetch from XPS DB for tnlMapId: %"
                           PRIu64 " | error: %d.\n", pTunnel->dcpMapList[count], retVal);
            return xpsStatus2SaiStatus(retVal);
        }
        mapType = pTnlMap->mapType;

        if (mapType == SAI_TUNNEL_MAP_TYPE_VNI_TO_BRIDGE_IF)
        {

        }
        else if (mapType == SAI_TUNNEL_MAP_TYPE_VNI_TO_VLAN_ID)
        {
            uint32_t entryCnt = 0;

            for (entryCnt = 0; entryCnt < pTnlMap->mapEntryCnt; ++entryCnt)
            {
                /* Search for the node in global SAI TUNNEL MAP ENTRY DB */
                retVal = xpSaiTunnelMapEntryGetCtxDb(xpsDevId, pTnlMap->mapEntryId[entryCnt],
                                                     &pTnlMapEntry);
                if (retVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("xpSaiTunnelMapEntryGetCtxDb: Failed %"
                                   PRIu64 " | error: %d.\n", pTnlMap->mapEntryId[entryCnt], retVal);
                    return xpsStatus2SaiStatus(retVal);
                }
                if (pTnlMapEntry->mapType != SAI_TUNNEL_MAP_TYPE_VNI_TO_VLAN_ID)
                {
                    continue;
                }

                vniId = pTnlMapEntry->vniIdKey;
                brVlanId = pTnlMapEntry->vlanId;
                xpsMapType = xpSaiConvertVxlanMapType((sai_tunnel_map_type_t)
                                                      pTnlMapEntry->mapType);
                if (xpsMapType == VXLAN_MAP_NONE)
                {
                    return SAI_STATUS_NOT_SUPPORTED;
                }
                // Not configured in HW
                if (!pTnlMapEntry->isConfgInHW)
                {
                    XP_SAI_LOG_ERR("pTnlMapEntry Not HW configured in HW %"
                                   PRIu64 " | error: %d.\n", pTnlMap->mapEntryId[entryCnt], retVal);
                    return SAI_STATUS_FAILURE;
                }

                if (pTnlMapEntry->refCnt == 1)
                {
                    /* Get/Alloc VNI DB entry */
                    retVal = xpsVniGetDbEntry(scopeId, vniId, &vniCtx);
                    if (retVal != XP_NO_ERR)
                    {
                        XP_SAI_LOG_ERR("VniGet Failed: %"
                                       PRIu64 " | error: %d.\n", pTnlMap->mapEntryId[entryCnt], retVal);
                        return xpsStatus2SaiStatus(retVal);
                    }

                    /*Add Vlan to VNI DB.*/
                    retVal = xpsVxlanRemoveVlanToVniDbEntry(xpsDevId, vniId,
                                                            xpsMapType, brVlanId);
                    if (retVal != XP_NO_ERR && retVal != XP_ERR_RESOURCE_BUSY)
                    {
                        XP_SAI_LOG_ERR("RemoveVlanToVniDb Failed: %"
                                       PRIu64 " | error: %d.\n", pTnlMap->mapEntryId[entryCnt], retVal);
                        return xpsStatus2SaiStatus(retVal);
                    }
                    if (vniCtx->refCnt == 0)
                    {
                        retVal = xpsVxlanVniDeleteInHw(xpsDevId, vniId, vniCtx->hwEVlanId,
                                                       vniCtx->hwEVidxId, false);
                        if (retVal != XP_NO_ERR)
                        {
                            XP_SAI_LOG_ERR("RemoveVxlanVniHW Failed: %"
                                           PRIu64 " | error: %d.\n", pTnlMap->mapEntryId[entryCnt], retVal);
                            return xpsStatus2SaiStatus(retVal);
                        }
                        retVal = xpsVxlanVniDeleteDbEntry(xpsDevId, vniId);
                        if (retVal != XP_NO_ERR)
                        {
                            XP_SAI_LOG_ERR("RemoveVxlanVniDb Failed: %"
                                           PRIu64 " | error: %d.\n", pTnlMap->mapEntryId[entryCnt], retVal);
                            return xpsStatus2SaiStatus(retVal);
                        }
                    }
                }
                pTnlMapEntry->refCnt--;
                if (pTnlMapEntry->refCnt == 0)
                {
                    pTnlMapEntry->isConfgInHW = false;
                }

            }
        }
        else if (mapType == SAI_TUNNEL_MAP_TYPE_VNI_TO_VIRTUAL_ROUTER_ID)
        {
            uint32_t entryCnt = 0;

            for (entryCnt = 0; entryCnt < pTnlMap->mapEntryCnt; ++entryCnt)
            {
                /* Search for the node in global SAI TUNNEL MAP ENTRY DB */
                retVal = xpSaiTunnelMapEntryGetCtxDb(xpsDevId, pTnlMap->mapEntryId[entryCnt],
                                                     &pTnlMapEntry);
                if (retVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("xpSaiTunnelMapEntryGetCtxDb: Failed %"
                                   PRIu64 " | error: %d.\n", pTnlMap->mapEntryId[entryCnt], retVal);
                    return xpsStatus2SaiStatus(retVal);
                }
                if (pTnlMapEntry->mapType != SAI_TUNNEL_MAP_TYPE_VNI_TO_VIRTUAL_ROUTER_ID)
                {
                    continue;
                }

                vniId = pTnlMapEntry->vniIdKey;
                //vrfId = pTnlMapEntry->vrfId;
                xpsMapType = xpSaiConvertVxlanMapType((sai_tunnel_map_type_t)
                                                      pTnlMapEntry->mapType);
                if (xpsMapType == VXLAN_MAP_NONE)
                {
                    return SAI_STATUS_NOT_SUPPORTED;
                }
                // Not configured in HW
                if (!pTnlMapEntry->isConfgInHW)
                {
                    XP_SAI_LOG_ERR("pTnlMapEntry Not HW configured in HW %"
                                   PRIu64 " | error: %d.\n", pTnlMap->mapEntryId[entryCnt], retVal);
                    return SAI_STATUS_FAILURE;
                }

                if (pTnlMapEntry->refCnt == 1)
                {
                    /* Get/Alloc VNI DB entry */
                    retVal = xpsVniGetDbEntry(scopeId, vniId, &vniCtx);
                    if (retVal != XP_NO_ERR)
                    {
                        XP_SAI_LOG_ERR("VniGet Failed: %"
                                       PRIu64 " | error: %d.\n", pTnlMap->mapEntryId[entryCnt], retVal);
                        return xpsStatus2SaiStatus(retVal);
                    }

                    // /*Add Vlan to VNI DB.*/
                    // retVal = xpsVxlanRemoveVlanToVniDbEntry(xpsDevId, vniId,
                    //                                         xpsMapType, brVlanId);
                    // if (retVal != XP_NO_ERR && retVal != XP_ERR_RESOURCE_BUSY)
                    // {
                    //     XP_SAI_LOG_ERR("RemoveVlanToVniDb Failed: %"
                    //                    PRIu64 " | error: %d.\n", pTnlMap->mapEntryId[entryCnt], retVal);
                    //     return xpsStatus2SaiStatus(retVal);
                    // }
                    vniCtx->refCnt--;
                    if (vniCtx->refCnt == 0)
                    {
                        retVal = xpsVxlanVniDeleteInHw(xpsDevId, vniId, vniCtx->hwEVlanId,
                                                       vniCtx->hwEVidxId, true);
                        if (retVal != XP_NO_ERR)
                        {
                            XP_SAI_LOG_ERR("RemoveVxlanVniHW Failed: %"
                                           PRIu64 " | error: %d.\n", pTnlMap->mapEntryId[entryCnt], retVal);
                            return xpsStatus2SaiStatus(retVal);
                        }
                        retVal = xpsVxlanVniDeleteDbEntry(xpsDevId, vniId);
                        if (retVal != XP_NO_ERR)
                        {
                            XP_SAI_LOG_ERR("RemoveVxlanVniDb Failed: %"
                                           PRIu64 " | error: %d.\n", pTnlMap->mapEntryId[entryCnt], retVal);
                            return xpsStatus2SaiStatus(retVal);
                        }
                    }
                }
                pTnlMapEntry->refCnt--;
                if (pTnlMapEntry->refCnt == 0)
                {
                    pTnlMapEntry->isConfgInHW = false;
                }

            }
        }
    }
    /*Iterate TTI/ACL Rules per Termination entry.*/
    retVal = xpsVxlanRemoveTunnelTermination(xpsDevId, pTunnel->tnlIntfId,
                                             entry->tnlEntryId, isIpv6);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsVxlanRemoveTunnelTermination, error %d\n", retVal);
    }

    entry->count = 0;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiCreateTunnelTermTableEntry

sai_status_t xpSaiCreateTunnelTermTableEntry(sai_object_id_t
                                             *tunnel_term_table_entry_id,
                                             sai_object_id_t switch_id,
                                             uint32_t attr_count,
                                             const sai_attribute_t *attr_list)
{
    sai_status_t           saiRetVal       = SAI_STATUS_SUCCESS;
    XP_STATUS              retVal          = XP_NO_ERR;
    sai_object_id_t        oid             = SAI_NULL_OBJECT_ID;
    xpsDevice_t            xpsDevId        = 0;
    xpSaiTunnelEntryInfoT *tnlTermTblEntry = NULL;
    xpSaiTunnelTermTableEntryAttributesT attributes;
    sai_object_id_t        saiTnlId        = SAI_NULL_OBJECT_ID;
#if 0
    xpPktCmd_e             learnMode       = XP_PKTCMD_FWD_MIRROR;
    bool is_term_entry_nw = TRUE;
#endif
    xpSaiTunnelInfoT       *saiTnlInfo     = NULL;
    memset(&attributes, 0, sizeof(attributes));

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    /* Check incoming attributes */
    saiRetVal = xpSaiAttrCheck(attr_count, attr_list,
                               TUNNEL_TERM_TABLE_ENTRY_VALIDATION_ARRAY_SIZE, tunnel_term_table_entry_attribs,
                               SAI_COMMON_API_CREATE);

    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiAttrCheck: Attribute check failed with error: %d.\n",
                       saiRetVal);
        return saiRetVal;
    }

    xpSaiSetDefaultTunnelTermTableEntryAttributeVals(&attributes);
    saiRetVal = xpSaiUpdateTunnelTermTableEntryAttributeVals(attr_count, attr_list,
                                                             &attributes);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiUpdateTunnelTermTableEntryAttributeVals: Failed to update attributes with error: %d.\n",
                       saiRetVal);
        return saiRetVal;
    }

    if ((NULL == tunnel_term_table_entry_id) || (NULL == attr_list))
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    /* Get xpsDevId from switch_id */
    xpsDevId = xpSaiObjIdSwitchGet(switch_id);

    retVal = xpSaiTunnelTermEntryIdAllocate(xpsDevId, &attributes, &oid);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiTunnelTermEntryIdAllocate: SAI object Tunnel term entry could not be created, error: %d.\n",
                       retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    /* Fetch from state DB */
    retVal = xpSaiTunnelEntryGetCtxDb(xpsDevId, oid, &tnlTermTblEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiTunnelEntryGetCtxDb: Failed to fetch from XPS DB for tnlTermTblEntry: %"
                       PRIu64 " | error: %d.\n", oid, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    saiTnlId = tnlTermTblEntry->tnlId;
    retVal = xpSaiTunnelGetCtxDb(xpsDevId, saiTnlId, &saiTnlInfo);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiTunnelGetCtxDb: Failed to fetch from XPS DB for tunnel_id: %"
                       PRIu64 " | error: %d.\n", saiTnlId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    if ((attributes.tunnelType.s32 == SAI_TUNNEL_TYPE_IPINIP_GRE) ||
        (attributes.tunnelType.s32 == SAI_TUNNEL_TYPE_MPLS))
    {
        XP_SAI_LOG_INFO("[skip] SAI Tunnel type %d is not implemented.\n",
                        attributes.tunnelType.s32);
        // SONIC Workaround
        return SAI_STATUS_SUCCESS;
        //return SAI_STATUS_NOT_IMPLEMENTED;
    }
    else if (attributes.tunnelType.s32 == SAI_TUNNEL_TYPE_VXLAN)
    {
        /* Create a VXLAN tunnel */
        tnlTermTblEntry->tnlIntfId = saiTnlInfo->tnlIntfId;
        saiRetVal = xpSaiVxlanTunnelTerminationCreate(xpsDevId, tnlTermTblEntry);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiVxlanTunnelTerminationCreate: Failed to create VXLAN tunnel, error: %d.\n",
                           saiRetVal);
            return saiRetVal;
        }
    }
    else if (attributes.tunnelType.s32 == SAI_TUNNEL_TYPE_IPINIP)
    {
        saiRetVal =  xpSaiIpInIpTunnelCreate(xpsDevId, tnlTermTblEntry);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiIpInIpTunnelCreate: Failed to create IpinIp tunnel, error: %d.\n",
                           saiRetVal);
            return saiRetVal;
        }
    }
    else
    {
        XP_SAI_LOG_ERR("Wrong SAI Tunnel type passed!\n");
        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    /* Add tunnel term table entry associated with Tunnel */
    saiRetVal = xpSaiTunnelAddTermTableEntry(xpsDevId, attributes.tunnelId.oid,
                                             oid);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to add Tunnel termination entry %" PRIu64
                       " associated with Tunnel %" PRIu64 ", error: %d.\n",
                       oid, attributes.tunnelId.oid, saiRetVal);
        return saiRetVal;
    }
    *tunnel_term_table_entry_id = oid;


    /* Below function, retrives Tunnel HW Id and sets Learning mode. Check the
       need for this in M0. For now commenting as we are not populating
       any Tunnel HW ID DB */
#if 0
    learnMode = saiTnlInfo->inrSaMissCmd;
    saiRetVal = xpSaiTunnelBridgePortSetFdbLearningMode(xpsDevId, saiTnlId,
                                                        *tunnel_term_table_entry_id, learnMode, is_term_entry_nw);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("FDB Learning mode in TunnelBridgePort failed, error: %d\n",
                       saiRetVal);
    }
#endif
    return saiRetVal;
}

//Func: xpSaiRemoveTunnelTermTableEntry

sai_status_t xpSaiRemoveTunnelTermTableEntry(sai_object_id_t
                                             tunnel_term_table_entry_id)
{
    sai_status_t           saiRetVal       = SAI_STATUS_SUCCESS;
    XP_STATUS              retVal          = XP_NO_ERR;
    xpsDevice_t            xpsDevId        = 0;
    xpSaiTunnelEntryInfoT *tnlTermTblEntry = NULL;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    /* Check incoming parameters */
    if (!XDK_SAI_OBJID_TYPE_CHECK(tunnel_term_table_entry_id,
                                  SAI_OBJECT_TYPE_TUNNEL_TERM_TABLE_ENTRY))
    {
        XP_SAI_LOG_DBG("Wrong object type received (%lu).\n",
                       xpSaiObjIdTypeGet(tunnel_term_table_entry_id));
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    /* Get xpsDevId from tunnel_term_table_entry_id */
    xpsDevId = xpSaiObjIdSwitchGet(tunnel_term_table_entry_id);

    /* Fetch from state DB */
    retVal = xpSaiTunnelEntryGetCtxDb(xpsDevId, tunnel_term_table_entry_id,
                                      &tnlTermTblEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiTunnelEntryGetCtxDb: Failed to fetch from XPS DB for tnlTermTblEntry: %"
                       PRIu64 " | error: %d.\n",
                       tunnel_term_table_entry_id, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    if ((tnlTermTblEntry->tnlType == SAI_TUNNEL_TYPE_IPINIP_GRE) ||
        (tnlTermTblEntry->tnlType == SAI_TUNNEL_TYPE_MPLS))
    {
        XP_SAI_LOG_ERR("SAI Tunnel type %d is not implemented.\n",
                       tnlTermTblEntry->tnlType);
        return SAI_STATUS_NOT_IMPLEMENTED;
    }
    else if (tnlTermTblEntry->tnlType == SAI_TUNNEL_TYPE_VXLAN)
    {
        /* Remove a VXLAN tunnel */
        saiRetVal = xpSaiVxlanTunnelTerminationRemove(xpsDevId, tnlTermTblEntry);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiVxlanTunnelTerminationRemove: Failed to remove VXLAN tunnel, error: %d.\n",
                           saiRetVal);
            return saiRetVal;
        }
    }
    else if (tnlTermTblEntry->tnlType == SAI_TUNNEL_TYPE_IPINIP)
    {
        saiRetVal = xpSaiIpinIpTunnelRemove(xpsDevId, tnlTermTblEntry);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiIpinIpTunnelRemove: Failed to remove IPinIP tunnel, error: %d.\n",
                           saiRetVal);
            return saiRetVal;
        }
    }
    else
    {
        XP_SAI_LOG_ERR("Wrong SAI Tunnel type passed!\n");
        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    /* Remove termination table entry ids associated with tunnel */
    saiRetVal = xpSaiTunnelRemoveTermTableEntry(xpsDevId, tnlTermTblEntry->tnlId,
                                                tunnel_term_table_entry_id);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to remove Tunnel termination entry %" PRIu64
                       " associated with Tunnel %" PRIu64 ", error: %d.\n",
                       tunnel_term_table_entry_id, tnlTermTblEntry->tnlId, saiRetVal);
        return saiRetVal;
    }

    retVal = xpSaiTunnelTermEntryIdRelease(xpsDevId, tunnel_term_table_entry_id);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiTunnelTermEntryIdRelease: SAI object Tunnel termination table entry could not be removed, error: %d.\n",
                       retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetTunnelTermTableEntryAttribute

sai_status_t xpSaiSetTunnelTermTableEntryAttribute(sai_object_id_t
                                                   tunnel_term_table_entry_id, const sai_attribute_t *attr)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    if (NULL == attr)
    {
        XP_SAI_LOG_ERR("Invalid parameter received!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    /* Check incoming attributes */
    saiRetVal = xpSaiAttrCheck(1, attr,
                               TUNNEL_TERM_TABLE_ENTRY_VALIDATION_ARRAY_SIZE, tunnel_term_table_entry_attribs,
                               SAI_COMMON_API_SET);

    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiAttrCheck: Attribute check failed with error: %d.\n",
                       saiRetVal);
        return saiRetVal;
    }

    switch (attr->id)
    {
        default:
            {
                XP_SAI_LOG_ERR("Invalid parameter received %d.\n", attr->id);
                return SAI_STATUS_INVALID_PARAMETER;
            }
    }

    return saiRetVal;
}

//Func: xpSaiGetTunnelTermTableEntryAttribute

static sai_status_t xpSaiGetTunnelTermTableEntryAttribute(
    sai_object_id_t tunnel_term_table_entry_id,
    xpSaiTunnelEntryInfoT *tnlTermTblEntry, sai_attribute_t *attr)
{

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    if ((NULL == tnlTermTblEntry) || (NULL == attr))
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return SAI_STATUS_FAILURE;
    }

    switch (attr->id)
    {
        case SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_VR_ID:
            {
                attr->value.oid = tnlTermTblEntry->vrfId;
                break;
            }
        case SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_TYPE:
            {
                attr->value.s32 = tnlTermTblEntry->entryType;
                break;
            }
        case SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_DST_IP:
            {
                attr->value.ipaddr = tnlTermTblEntry->dstIpAddr;
                break;
            }
        case SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_SRC_IP:
            {
                attr->value.ipaddr = tnlTermTblEntry->srcIpAddr;
                break;
            }
        case SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_TUNNEL_TYPE:
            {
                attr->value.s32 = tnlTermTblEntry->tnlType;
                break;
            }
        case SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_ACTION_TUNNEL_ID:
            {
                attr->value.oid = tnlTermTblEntry->tnlId;
                break;
            }
        case SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_IP_ADDR_FAMILY:
            {
                attr->value.ipaddr.addr_family = tnlTermTblEntry->dstIpAddr.addr_family;
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Invalid parameter received %d.\n", attr->id);
                return SAI_STATUS_INVALID_PARAMETER;
            }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetTunnelTermTableEntryAttributes

sai_status_t xpSaiGetTunnelTermTableEntryAttributes(sai_object_id_t
                                                    tunnel_term_table_entry_id,
                                                    uint32_t attr_count,
                                                    sai_attribute_t *attr_list)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    xpsDevice_t  xpsDevId  = 0;
    uint32_t     count     = 0;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    if (NULL == attr_list)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return SAI_STATUS_FAILURE;
    }

    /* Check incoming attributes */
    saiRetVal = xpSaiAttrCheck(attr_count, attr_list,
                               TUNNEL_TERM_TABLE_ENTRY_VALIDATION_ARRAY_SIZE, tunnel_term_table_entry_attribs,
                               SAI_COMMON_API_GET);

    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiAttrCheck: Attribute check failed with error: %d.\n",
                       saiRetVal);
        return saiRetVal;
    }

    /* Get xpsDevId from tunnel_term_table_entry_id */
    xpsDevId = xpSaiObjIdSwitchGet(tunnel_term_table_entry_id);

    for (count = 0; count < attr_count; ++count)
    {
        XP_STATUS              retVal          = XP_NO_ERR;
        xpSaiTunnelEntryInfoT *tnlTermTblEntry = NULL;

        /* Fetch from state DB */
        retVal = xpSaiTunnelEntryGetCtxDb(xpsDevId, tunnel_term_table_entry_id,
                                          &tnlTermTblEntry);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpSaiTunnelEntryGetCtxDb: Failed to fetch from XPS DB for tnlTermTblEntry: %"
                           PRIu64 " | error: %d.\n",
                           tunnel_term_table_entry_id, retVal);
            return xpsStatus2SaiStatus(retVal);
        }

        saiRetVal = xpSaiGetTunnelTermTableEntryAttribute(tunnel_term_table_entry_id,
                                                          tnlTermTblEntry, &attr_list[count]);
        if (SAI_STATUS_SUCCESS != saiRetVal)
        {
            XP_SAI_LOG_ERR("xpSaiGetTunnelTermTableEntryAttribute failed, error: %d.\n",
                           saiRetVal);
            return saiRetVal;
        }
    }

    return saiRetVal;
}

//Func: xpSaiSetDefaultTunnelMapAttributeVals

static void xpSaiSetDefaultTunnelMapAttributeVals(xpSaiTunnelMapAttributesT*
                                                  attributes)
{
    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);
}

//Func: xpSaiUpdateTunnelMapAttributeVals

static sai_status_t xpSaiUpdateTunnelMapAttributeVals(const uint32_t attr_count,
                                                      const sai_attribute_t* attr_list, xpSaiTunnelMapAttributesT* attributes)
{
    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    for (uint32_t count = 0; count < attr_count; ++count)
    {
        switch (attr_list[count].id)
        {
            case SAI_TUNNEL_MAP_ATTR_TYPE:
                {
                    attributes->mapType = attr_list[count].value;
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Failed to set %d.\n", attr_list[count].id);
                }
        }
    }
    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiCreateTunnelMap

sai_status_t xpSaiCreateTunnelMap(sai_object_id_t *tunnel_map_id,
                                  sai_object_id_t switch_id,
                                  uint32_t attr_count, const sai_attribute_t *attr_list)
{
    sai_status_t    saiRetVal = SAI_STATUS_SUCCESS;
    XP_STATUS       retVal    = XP_NO_ERR;
    xpsDevice_t     xpsDevId  = 0;

    xpSaiTunnelMapAttributesT attributes;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    memset(&attributes, 0, sizeof(attributes));

    /* Check incoming attributes */
    saiRetVal = xpSaiAttrCheck(attr_count, attr_list,
                               TUNNEL_MAP_VALIDATION_ARRAY_SIZE, tunnel_map_attribs,
                               SAI_COMMON_API_CREATE);

    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiAttrCheck: Attribute check failed with error: %d.\n",
                       saiRetVal);
        return saiRetVal;
    }

    xpSaiSetDefaultTunnelMapAttributeVals(&attributes);
    saiRetVal = xpSaiUpdateTunnelMapAttributeVals(attr_count, attr_list,
                                                  &attributes);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiUpdateTunnelMapAttributeVals: Failed to update attributes with error: %d.\n",
                       saiRetVal);
        return saiRetVal;
    }

    if ((NULL == tunnel_map_id) || (NULL == attr_list))
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument.\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    /* Get xpsDevId from switch_id */
    xpsDevId = xpSaiObjIdSwitchGet(switch_id);

    if ((attributes.mapType.s32 == SAI_TUNNEL_MAP_TYPE_OECN_TO_UECN) ||
        (attributes.mapType.s32 == SAI_TUNNEL_MAP_TYPE_UECN_OECN_TO_OECN))
    {
        XP_SAI_LOG_ERR("SAI Tunnel map type (%d) is not supported!\n",
                       attributes.mapType.s32);
        return SAI_STATUS_NOT_SUPPORTED;
    }

    /* Allocate tunnel map Oid */
    retVal = xpSaiTunnelMapIdAllocate(xpsDevId, &attributes, tunnel_map_id);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiTunnelMapIdAllocate: SAI object Tunnel map could not be created, error %d.\n",
                       retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiRemoveTunnelMap

sai_status_t xpSaiRemoveTunnelMap(sai_object_id_t tunnel_map_id)
{
    XP_STATUS            retVal   = XP_NO_ERR;
    xpSaiTunnelMapInfoT *tnlMap   = NULL;
    xpsDevice_t          xpsDevId = 0;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    /* Check incoming parameters */
    if (!XDK_SAI_OBJID_TYPE_CHECK(tunnel_map_id, SAI_OBJECT_TYPE_TUNNEL_MAP))
    {
        XP_SAI_LOG_DBG("Wrong object type received (%lu).\n",
                       xpSaiObjIdTypeGet(tunnel_map_id));
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    /* Get xpsDevId from tunnel_map_id */
    xpsDevId = xpSaiObjIdSwitchGet(tunnel_map_id);

    /* Search corresponding object */
    retVal = xpSaiTunnelMapGetCtxDb(xpsDevId, tunnel_map_id, &tnlMap);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiTunnelMapGetCtxDb: Failed to fetch from XPS DB for tunnel_map_id: %"
                       PRIu64 " | error: %d.\n", tunnel_map_id, retVal);
        return retVal;
    }

    if (tnlMap->mapEntryCnt > 0)
    {
        XP_SAI_LOG_ERR("Need to remove all (%u) tunnel map entries associated with tunnel_map_id: %"
                       PRIu64 " first!\n", tnlMap->mapEntryCnt, tunnel_map_id);
        return SAI_STATUS_OBJECT_IN_USE;
    }

    retVal = xpSaiTunnelMapReleaseId(xpsDevId, tunnel_map_id);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiTunnelMapReleaseId: SAI object Tunnel map could not be removed, tunnel_map_id: %"
                       PRIu64 " | error: %d.\n", tunnel_map_id, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetTunnelMapAttribute

sai_status_t xpSaiSetTunnelMapAttribute(sai_object_id_t tunnel_map_id,
                                        const sai_attribute_t *attr)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    if (NULL == attr)
    {
        XP_SAI_LOG_ERR("Invalid parameter received!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    /* Check incoming attributes */
    saiRetVal = xpSaiAttrCheck(1, attr,
                               TUNNEL_MAP_VALIDATION_ARRAY_SIZE, tunnel_map_attribs,
                               SAI_COMMON_API_SET);

    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiAttrCheck: Attribute check failed with error: %d.\n",
                       saiRetVal);
        return saiRetVal;
    }

    switch (attr->id)
    {
        default:
            {
                XP_SAI_LOG_ERR("Invalid parameter received %d.\n", attr->id);
                return SAI_STATUS_INVALID_PARAMETER;
            }
    }

    return saiRetVal;
}

//Func: xpSaiTunnelMapGetAttrMapEntryList

static sai_status_t xpSaiTunnelMapGetAttrMapEntryList(sai_object_id_t
                                                      tunnel_map_id, sai_attribute_value_t* value)
{
    XP_STATUS            retVal   = XP_NO_ERR;
    xpsDevice_t          xpsDevId = 0;
    xpSaiTunnelMapInfoT *tnlMap   = NULL;
    uint32_t             count    = 0;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    if (NULL == value)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return SAI_STATUS_FAILURE;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(tunnel_map_id, SAI_OBJECT_TYPE_TUNNEL_MAP))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n",
                       xpSaiObjIdTypeGet(tunnel_map_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    /* Get xpsDevId from tunnel_id */
    xpsDevId = xpSaiObjIdSwitchGet(tunnel_map_id);

    /* Search corresponding object */
    retVal = xpSaiTunnelMapGetCtxDb(xpsDevId, tunnel_map_id, &tnlMap);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiTunnelMapGetCtxDb: Failed to fetch from XPS DB for tunnel_map_id: %"
                       PRIu64 " | error: %d.\n", tunnel_map_id, retVal);
        return retVal;
    }

    XP_SAI_LOG_DBG("Tunnel map: %" PRIu64 " associated with %d map entries.\n",
                   tunnel_map_id, tnlMap->mapEntryCnt);

    if (value->objlist.count < tnlMap->mapEntryCnt)
    {
        value->objlist.count = tnlMap->mapEntryCnt;
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    if ((tnlMap->mapEntryCnt != 0) && (NULL == value->objlist.list))
    {
        XP_SAI_LOG_ERR("Invalid parameters received.\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    for (count = 0; count < tnlMap->mapEntryCnt; ++count)
    {
        value->objlist.list[count] = tnlMap->mapEntryId[count];
    }

    value->objlist.count = tnlMap->mapEntryCnt;

    return xpsStatus2SaiStatus(retVal);
}

//Func: xpSaiGetTunnelMapAttribute

static sai_status_t xpSaiGetTunnelMapAttribute(sai_object_id_t tunnel_map_id,
                                               xpSaiTunnelMapInfoT *tnlMapEntry, sai_attribute_t *attr)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    if ((NULL == tnlMapEntry) || (NULL == attr))
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return SAI_STATUS_FAILURE;
    }

    switch (attr->id)
    {
        case SAI_TUNNEL_MAP_ATTR_TYPE:
            {
                attr->value.s32 = (sai_tunnel_map_type_t)xpSaiTunnelMapObjIdMapTypeGet(
                                      tunnel_map_id);
                break;
            }
        case SAI_TUNNEL_MAP_ATTR_ENTRY_LIST:
            {
                saiRetVal = xpSaiTunnelMapGetAttrMapEntryList(tunnel_map_id, &attr->value);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    if (saiRetVal != SAI_STATUS_BUFFER_OVERFLOW)
                    {
                        XP_SAI_LOG_ERR("xpSaiTunnelMapGetAttrMapEntryList: Failed to get (SAI_TUNNEL_MAP_ATTR_ENTRY_LIST), tunnel_map_id: %"
                                       PRIu64
                                       " | error: %d\n", tunnel_map_id, saiRetVal);
                    }
                    return saiRetVal;
                }
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Invalid parameter received %d.\n", attr->id);
                return SAI_STATUS_INVALID_PARAMETER;
            }
    }
    return saiRetVal;
}

//Func: xpSaiGetTunnelMapAttributes

sai_status_t xpSaiGetTunnelMapAttributes(sai_object_id_t tunnel_map_id,
                                         uint32_t attr_count, sai_attribute_t *attr_list)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    xpsDevice_t  xpsDevId  = 0;
    uint32_t     count     = 0;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    if (NULL == attr_list)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return SAI_STATUS_FAILURE;
    }

    /* Check incoming attributes */
    saiRetVal = xpSaiAttrCheck(attr_count, attr_list,
                               TUNNEL_MAP_VALIDATION_ARRAY_SIZE, tunnel_map_attribs,
                               SAI_COMMON_API_GET);

    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiAttrCheck: Attribute check failed with error: %d.\n",
                       saiRetVal);
        return saiRetVal;
    }

    /* Get xpsDevId from tunnel_map_id */
    xpsDevId = xpSaiObjIdSwitchGet(tunnel_map_id);

    for (count = 0; count < attr_count; ++count)
    {
        XP_STATUS            retVal      = XP_NO_ERR;
        xpSaiTunnelMapInfoT *tnlMapEntry = NULL;

        /* Fetch from state DB */
        retVal = xpSaiTunnelMapGetCtxDb(xpsDevId, tunnel_map_id, &tnlMapEntry);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpSaiTunnelMapGetCtxDb: Failed to fetch from XPS DB for tunnel_map_id: %"
                           PRIu64 " | error: %d.\n", tunnel_map_id, retVal);
            return xpsStatus2SaiStatus(retVal);
        }

        saiRetVal = xpSaiGetTunnelMapAttribute(tunnel_map_id, tnlMapEntry,
                                               &attr_list[count]);
        if (SAI_STATUS_SUCCESS != saiRetVal)
        {
            XP_SAI_LOG_ERR("xpSaiGetTunnelMapAttribute failed, error: %d.\n", saiRetVal);
            return saiRetVal;
        }
    }

    return saiRetVal;
}

//Func: xpSaiSetDefaultTunnelMapEntryAttributeVals

static void xpSaiSetDefaultTunnelMapEntryAttributeVals(
    xpSaiTunnelMapEntryAttributesT* attributes)
{
    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);
}

//Func: xpSaiUpdateTunnelMapEntryAttributeVals

static sai_status_t xpSaiUpdateTunnelMapEntryAttributeVals(
    const uint32_t attr_count, const sai_attribute_t* attr_list,
    xpSaiTunnelMapEntryAttributesT* attributes)
{
    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    for (uint32_t count = 0; count < attr_count; ++count)
    {
        switch (attr_list[count].id)
        {
            case SAI_TUNNEL_MAP_ENTRY_ATTR_TUNNEL_MAP_TYPE:
                {
                    attributes->mapType = attr_list[count].value;
                    break;
                }
            case SAI_TUNNEL_MAP_ENTRY_ATTR_TUNNEL_MAP:
                {
                    attributes->mapId = attr_list[count].value;
                    break;
                }
            case SAI_TUNNEL_MAP_ENTRY_ATTR_OECN_KEY:
                {
                    attributes->oecnKey = attr_list[count].value;
                    break;
                }
            case SAI_TUNNEL_MAP_ENTRY_ATTR_OECN_VALUE:
                {
                    attributes->oecnVal = attr_list[count].value;
                    break;
                }
            case SAI_TUNNEL_MAP_ENTRY_ATTR_UECN_KEY:
                {
                    attributes->uecnKey = attr_list[count].value;
                    break;
                }
            case SAI_TUNNEL_MAP_ENTRY_ATTR_UECN_VALUE:
                {
                    attributes->uecnVal = attr_list[count].value;
                    break;
                }
            case SAI_TUNNEL_MAP_ENTRY_ATTR_VLAN_ID_KEY:
                {
                    attributes->vlanIdKey = attr_list[count].value;
                    break;
                }
            case SAI_TUNNEL_MAP_ENTRY_ATTR_VLAN_ID_VALUE:
                {
                    attributes->vlanIdVal = attr_list[count].value;
                    break;
                }
            case SAI_TUNNEL_MAP_ENTRY_ATTR_VNI_ID_KEY:
                {
                    if ((attr_list[count].value.s32 < XP_SAI_TUNNEL_MAP_ENTRY_ATTR_VNI_ID_MIN) ||
                        (attr_list[count].value.s32 > XP_SAI_TUNNEL_MAP_ENTRY_ATTR_VNI_ID_MAX))
                    {
                        XP_SAI_LOG_ERR("Invalid VNI ID :%d \n", attr_list[count].value);
                        return SAI_STATUS_FAILURE;
                    }
                    else
                    {
                        attributes->vniIdKey = attr_list[count].value;
                    }
                    break;
                }
            case SAI_TUNNEL_MAP_ENTRY_ATTR_VNI_ID_VALUE:
                {
                    if ((attr_list[count].value.s32 < XP_SAI_TUNNEL_MAP_ENTRY_ATTR_VNI_ID_MIN) ||
                        (attr_list[count].value.s32 > XP_SAI_TUNNEL_MAP_ENTRY_ATTR_VNI_ID_MAX))
                    {
                        XP_SAI_LOG_ERR("Invalid VNI ID :%d \n", attr_list[count].value);
                        return SAI_STATUS_FAILURE;
                    }
                    else
                    {
                        attributes->vniIdVal = attr_list[count].value;
                    }
                    break;
                }
            case SAI_TUNNEL_MAP_ENTRY_ATTR_BRIDGE_ID_KEY:
                {
                    attributes->bridgeIdKey = attr_list[count].value;
                    break;
                }
            case SAI_TUNNEL_MAP_ENTRY_ATTR_BRIDGE_ID_VALUE:
                {
                    attributes->bridgeIdVal = attr_list[count].value;
                    break;
                }
            case SAI_TUNNEL_MAP_ENTRY_ATTR_VIRTUAL_ROUTER_ID_KEY:
                {
                    attributes->vrfIdKey = attr_list[count].value;
                    break;
                }
            case SAI_TUNNEL_MAP_ENTRY_ATTR_VIRTUAL_ROUTER_ID_VALUE:
                {
                    attributes->vrfIdVal = attr_list[count].value;
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Failed to set %d.\n", attr_list[count].id);
                }
        }
    }
    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiTunnelMapAddEntry

static sai_status_t xpSaiTunnelMapAddEntry(xpsDevice_t xpsDevId,
                                           sai_object_id_t tnlMapId, sai_object_id_t entryId)
{
    XP_STATUS            retVal   = XP_NO_ERR;
    xpsScope_t           scope    = xpSaiScopeFromDevGet(xpsDevId);
    xpSaiTunnelMapInfoT *entry    = NULL;
    xpSaiTunnelMapInfoT *newEntry = NULL;


    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(tnlMapId, SAI_OBJECT_TYPE_TUNNEL_MAP))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).\n",
                       xpSaiObjIdTypeGet(tnlMapId));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    /* Search for the node in global SAI TUNNEL MAP DB */
    retVal = xpSaiTunnelMapGetCtxDb(xpsDevId, tnlMapId, &entry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiTunnelMapGetCtxDb: Failed to fetch from XPS DB for tnlMapId: %"
                       PRIu64 " | error: %d.\n", tnlMapId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    if (xpsDAIsCtxGrowthNeeded(entry->mapEntryCnt, XP_SAI_TUNNEL_THRESHOLD))
    {
        /* Double the entry list array size */
        retVal = xpSaiDynamicArrayGrow((void**)&newEntry, (void*)entry,
                                       sizeof(xpSaiTunnelMapInfoT),
                                       sizeof(sai_object_id_t), entry->mapEntryCnt,
                                       XP_SAI_TUNNEL_THRESHOLD);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpSaiDynamicArrayGrow: Dynamic array grow failed, error: %d.\n",
                           retVal);
            return xpsStatus2SaiStatus(retVal);
        }

        /* Remove the corresponding state from SAI TUNNEL MAP DB and add new state */
        retVal = xpSaiTunnelMapDeleteCtxDb(xpsDevId, tnlMapId);
        if (retVal != XP_NO_ERR)
        {
            xpsStateHeapFree((void*)newEntry);
            XP_SAI_LOG_ERR("xpSaiTunnelMapDeleteCtxDb: Deletion of data failed for tnlMapId: %"
                           PRIu64 " | error: %d.\n", tnlMapId, retVal);
            return xpsStatus2SaiStatus(retVal);
        }

        /* Insert the new state */
        retVal = xpsStateInsertData(scope, xpSaiTunnelMapStateDbHndl, (void*)newEntry);
        if (retVal != XP_NO_ERR)
        {
            xpsStateHeapFree((void*)newEntry);
            XP_SAI_LOG_ERR("xpsStateInsertData: Insertion of data failed, error: %d.\n",
                           retVal);
            return retVal;
        }

        entry = newEntry;
    }

    entry->mapEntryId[entry->mapEntryCnt++] = entryId;

    return xpsStatus2SaiStatus(retVal);
}

//Func: xpSaiTunnelMapRemoveEntry

static sai_status_t xpSaiTunnelMapRemoveEntry(xpsDevice_t xpsDevId,
                                              sai_object_id_t tnlMapId, sai_object_id_t entryId)
{
    XP_STATUS            retVal           = XP_NO_ERR;
    xpsScope_t           scope            = xpSaiScopeFromDevGet(xpsDevId);
    xpSaiTunnelMapInfoT *entry            = NULL;
    xpSaiTunnelMapInfoT *newEntry         = NULL;
    sai_uint8_t          isEnrtyIdPresent = 0;
    sai_uint32_t         count            = 0;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(tnlMapId, SAI_OBJECT_TYPE_TUNNEL_MAP))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).\n",
                       xpSaiObjIdTypeGet(tnlMapId));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    /* Search for the node in global SAI TUNNEL DB */
    retVal = xpSaiTunnelMapGetCtxDb(xpsDevId, tnlMapId, &entry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiTunnelMapGetCtxDb: Failed to fetch from XPS DB for tnlMapId: %"
                       PRIu64 " | error: %d.\n", tnlMapId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    /* Remove the Entry Id from associated Tunnel Map */
    for (count = 0; count < entry->mapEntryCnt; ++count)
    {
        if (!isEnrtyIdPresent)
        {
            if (entry->mapEntryId[count] == entryId)
            {
                isEnrtyIdPresent = 1;
            }
            continue;
        }
        else
        {
            entry->mapEntryId[count-1] = entry->mapEntryId[count];
        }
    }

    if (!isEnrtyIdPresent)
    {
        return xpsStatus2SaiStatus(XP_ERR_INVALID_ARG);
    }

    /* Decrement the entryCnt count */
    entry->mapEntryCnt--;

    if (xpsDAIsCtxShrinkNeeded(entry->mapEntryCnt, XP_SAI_TUNNEL_THRESHOLD))
    {
        /* Shrink the entryId array size */
        retVal = xpSaiDynamicArrayShrink((void**)&newEntry, (void*)entry,
                                         sizeof(xpSaiTunnelMapInfoT),
                                         sizeof(sai_object_id_t), entry->mapEntryCnt,
                                         XP_SAI_TUNNEL_THRESHOLD);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpSaiDynamicArrayShrink: Dynamic array shrink failed, error: %d.\n",
                           retVal);
            return xpsStatus2SaiStatus(retVal);
        }

        /* Remove the corresponding state from SAI TUNNEL DB and add new state */
        retVal = xpSaiTunnelMapDeleteCtxDb(xpsDevId, tnlMapId);
        if (retVal != XP_NO_ERR)
        {
            xpsStateHeapFree((void*)newEntry);
            XP_SAI_LOG_ERR("xpSaiTunnelMapDeleteCtxDb: Deletion of data failed for tnlMapId: %"
                           PRIu64 " | error: %d.\n", tnlMapId, retVal);
            return xpsStatus2SaiStatus(retVal);
        }

        /* Insert the new state */
        retVal = xpsStateInsertData(scope, xpSaiTunnelMapStateDbHndl, (void*)newEntry);
        if (retVal != XP_NO_ERR)
        {
            xpsStateHeapFree((void*)newEntry);
            XP_SAI_LOG_ERR("xpsStateInsertData: Insertion of data failed, error: %d.\n",
                           retVal);
            return retVal;
        }
    }

    return xpsStatus2SaiStatus(retVal);
}

//Func: xpSaiCreateTunnelMapEntry

sai_status_t xpSaiCreateTunnelMapEntry(sai_object_id_t *tunnel_map_entry_id,
                                       sai_object_id_t switch_id,
                                       uint32_t attr_count, const sai_attribute_t *attr_list)
{
    sai_status_t    saiRetVal = SAI_STATUS_SUCCESS;
    XP_STATUS       retVal    = XP_NO_ERR;
    sai_object_id_t oid       = SAI_NULL_OBJECT_ID;
    xpsDevice_t     xpsDevId  = 0;
    int32_t         mapType   = 0;

    xpSaiTunnelMapEntryAttributesT attributes;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    memset(&attributes, 0, sizeof(attributes));

    /* Check incoming attributes */
    saiRetVal = xpSaiAttrCheck(attr_count, attr_list,
                               TUNNEL_MAP_ENTRY_VALIDATION_ARRAY_SIZE, tunnel_map_entry_attribs,
                               SAI_COMMON_API_CREATE);

    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiAttrCheck: Attribute check failed with error: %d.\n",
                       saiRetVal);
        return saiRetVal;
    }

    xpSaiSetDefaultTunnelMapEntryAttributeVals(&attributes);
    saiRetVal = xpSaiUpdateTunnelMapEntryAttributeVals(attr_count, attr_list,
                                                       &attributes);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiUpdateTunnelMapEntryAttributeVals: Failed to update attributes with error: %d.\n",
                       saiRetVal);
        return saiRetVal;
    }

    if ((NULL == tunnel_map_entry_id) || (NULL == attr_list))
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument.\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    /* Get xpsDevId from switch_id */
    xpsDevId = xpSaiObjIdSwitchGet(switch_id);

    mapType = attributes.mapType.s32;

    if ((mapType == SAI_TUNNEL_MAP_TYPE_OECN_TO_UECN) ||
        (mapType == SAI_TUNNEL_MAP_TYPE_UECN_OECN_TO_OECN)
       )
    {
        XP_SAI_LOG_ERR("SAI Tunnel map type (%d) is not supported!\n", mapType);
        return SAI_STATUS_NOT_SUPPORTED;
    }

    /* Allocate tunnel map entry Oid */
    retVal = xpSaiTunnelMapEntryIdAllocate(xpsDevId, &attributes, &oid);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiTunnelMapEntryIdAllocate: SAI object Tunnel map entry could not be created, error %d.\n",
                       retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    /* Add tunnel map entries associated with tunnel map */
    saiRetVal = xpSaiTunnelMapAddEntry(xpsDevId, attributes.mapId.oid, oid);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to add Tunnel map entry %" PRIu64
                       " associated with Tunnel map %" PRIu64 ", error: %d.\n",
                       oid, attributes.mapId.oid, saiRetVal);
        return saiRetVal;
    }

    *tunnel_map_entry_id = oid;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiRemoveTunnelMapEntry

sai_status_t xpSaiRemoveTunnelMapEntry(sai_object_id_t tunnel_map_entry_id)
{
    sai_status_t              saiRetVal    = SAI_STATUS_SUCCESS;
    XP_STATUS                 retVal       = XP_NO_ERR;
    xpSaiTunnelMapEntryInfoT *pTnlMapEntry = NULL;
    xpsDevice_t               xpsDevId     = 0;
    int32_t                   mapType      = 0;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    /* Check incoming parameters */
    if (!XDK_SAI_OBJID_TYPE_CHECK(tunnel_map_entry_id,
                                  SAI_OBJECT_TYPE_TUNNEL_MAP_ENTRY))
    {
        XP_SAI_LOG_DBG("Wrong object type received (%lu).\n",
                       xpSaiObjIdTypeGet(tunnel_map_entry_id));
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    /* Get xpsDevId from tunnel_map_entry_id */
    xpsDevId = xpSaiObjIdSwitchGet(tunnel_map_entry_id);

    /* Fetch from state DB */
    retVal = xpSaiTunnelMapEntryGetCtxDb(xpsDevId, tunnel_map_entry_id,
                                         &pTnlMapEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiTunnelMapEntryGetCtxDb: Failed to fetch from XPS DB for tunnel_map_entry_id: %"
                       PRIu64 ", error: %d.\n",
                       tunnel_map_entry_id, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    if (pTnlMapEntry->isConfgInHW || pTnlMapEntry->refCnt)
    {
        XP_SAI_LOG_ERR("Tunnels still ref Map entry: %"
                       PRIu64 ", error: %d.\n",
                       tunnel_map_entry_id, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    mapType = pTnlMapEntry->mapType;

    if ((mapType == SAI_TUNNEL_MAP_TYPE_OECN_TO_UECN) ||
        (mapType == SAI_TUNNEL_MAP_TYPE_UECN_OECN_TO_OECN))
    {
        XP_SAI_LOG_ERR("SAI Tunnel map type (%d) is not supported!\n", mapType);
        return SAI_STATUS_NOT_SUPPORTED;
    }

    /* Remove tunnel map entry ids associated with tunnel map */
    saiRetVal = xpSaiTunnelMapRemoveEntry(xpsDevId, pTnlMapEntry->mapId,
                                          tunnel_map_entry_id);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to remove Tunnel map entry %" PRIu64
                       " associated with Tunnel map %" PRIu64 ", error: %d.\n",
                       tunnel_map_entry_id, pTnlMapEntry->mapId, saiRetVal);
        return saiRetVal;
    }

    retVal = xpSaiTunnelMapEntryIdRelease(xpsDevId, tunnel_map_entry_id);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiTunnelMapEntryIdRelease: SAI object Tunnel map entry could not be removed, error %d.\n",
                       retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetTunnelMapEntryAttribute

sai_status_t xpSaiSetTunnelMapEntryAttribute(sai_object_id_t
                                             tunnel_map_entry_id, const sai_attribute_t *attr)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    if (NULL == attr)
    {
        XP_SAI_LOG_ERR("Invalid parameter received!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    /* Check incoming attributes */
    saiRetVal = xpSaiAttrCheck(1, attr,
                               TUNNEL_MAP_ENTRY_VALIDATION_ARRAY_SIZE, tunnel_map_entry_attribs,
                               SAI_COMMON_API_SET);

    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiAttrCheck: Attribute check failed with error: %d.\n",
                       saiRetVal);
        return saiRetVal;
    }

    switch (attr->id)
    {
        default:
            {
                XP_SAI_LOG_ERR("Invalid parameter received %d.\n", attr->id);
                return SAI_STATUS_INVALID_PARAMETER;
            }
    }

    return saiRetVal;
}

//Func: xpSaiGetTunnelMapEntryAttribute

static sai_status_t xpSaiGetTunnelMapEntryAttribute(sai_object_id_t
                                                    tunnel_map_entry_id,
                                                    xpSaiTunnelMapEntryInfoT *tnlMapEntry, sai_attribute_t *attr)
{
    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    if ((NULL == tnlMapEntry) || (NULL == attr))
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return SAI_STATUS_FAILURE;
    }

    switch (attr->id)
    {
        case SAI_TUNNEL_MAP_ENTRY_ATTR_TUNNEL_MAP_TYPE:
            {
                attr->value.s32 = tnlMapEntry->mapType;
                break;
            }
        case SAI_TUNNEL_MAP_ENTRY_ATTR_TUNNEL_MAP:
            {
                attr->value.oid = tnlMapEntry->mapId;
                break;
            }
        case SAI_TUNNEL_MAP_ENTRY_ATTR_VLAN_ID_KEY:
            {
                attr->value.u16 = tnlMapEntry->vlanIdKey;
                break;
            }
        case SAI_TUNNEL_MAP_ENTRY_ATTR_VLAN_ID_VALUE:
            {
                attr->value.u16 = tnlMapEntry->vlanId;
                break;
            }
        case SAI_TUNNEL_MAP_ENTRY_ATTR_VNI_ID_KEY:
            {
                attr->value.u32 = tnlMapEntry->vniIdKey;
                break;
            }
        case SAI_TUNNEL_MAP_ENTRY_ATTR_VNI_ID_VALUE:
            {
                attr->value.u32 = tnlMapEntry->vniId;
                break;
            }
        case SAI_TUNNEL_MAP_ENTRY_ATTR_BRIDGE_ID_KEY:
            {
                attr->value.oid = tnlMapEntry->bridgeIdKey;
                break;
            }
        case SAI_TUNNEL_MAP_ENTRY_ATTR_BRIDGE_ID_VALUE:
            {
                attr->value.oid = tnlMapEntry->bridgeId;
                break;
            }
        case SAI_TUNNEL_MAP_ENTRY_ATTR_VIRTUAL_ROUTER_ID_KEY:
            {
                attr->value.oid = tnlMapEntry->vrfIdKey;
                break;
            }
        case SAI_TUNNEL_MAP_ENTRY_ATTR_VIRTUAL_ROUTER_ID_VALUE:
            {
                attr->value.oid = tnlMapEntry->vrfId;
                break;
            }
        case SAI_TUNNEL_MAP_ENTRY_ATTR_OECN_KEY:
        case SAI_TUNNEL_MAP_ENTRY_ATTR_OECN_VALUE:
        case SAI_TUNNEL_MAP_ENTRY_ATTR_UECN_KEY:
        case SAI_TUNNEL_MAP_ENTRY_ATTR_UECN_VALUE:
            {
                XP_SAI_LOG_ERR("Attribute %d is not supported.\n", attr->id);
                return SAI_STATUS_NOT_SUPPORTED;
            }
        default:
            {
                XP_SAI_LOG_ERR("Invalid parameter received %d.\n", attr->id);
                return SAI_STATUS_INVALID_PARAMETER;
            }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetTunnelMapEntryAttributes

sai_status_t xpSaiGetTunnelMapEntryAttributes(sai_object_id_t
                                              tunnel_map_entry_id, uint32_t attr_count, sai_attribute_t *attr_list)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    xpsDevice_t  xpsDevId  = 0;
    uint32_t     count     = 0;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    if (NULL == attr_list)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return SAI_STATUS_FAILURE;
    }

    /* Check incoming attributes */
    saiRetVal = xpSaiAttrCheck(attr_count, attr_list,
                               TUNNEL_MAP_ENTRY_VALIDATION_ARRAY_SIZE, tunnel_map_entry_attribs,
                               SAI_COMMON_API_GET);

    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiAttrCheck: Attribute check failed with error: %d.\n",
                       saiRetVal);
        return saiRetVal;
    }

    /* Get xpsDevId from tunnel_map_entry_id */
    xpsDevId = xpSaiObjIdSwitchGet(tunnel_map_entry_id);

    for (count = 0; count < attr_count; ++count)
    {
        XP_STATUS                 retVal      = XP_NO_ERR;
        xpSaiTunnelMapEntryInfoT *tnlMapEntry = NULL;

        /* Fetch from state DB */
        retVal = xpSaiTunnelMapEntryGetCtxDb(xpsDevId, tunnel_map_entry_id,
                                             &tnlMapEntry);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpSaiTunnelMapEntryGetCtxDb: Failed to fetch from XPS DB for tunnel_map_entry_id: %"
                           PRIu64 ", error: %d.\n",
                           tunnel_map_entry_id, retVal);
            return xpsStatus2SaiStatus(retVal);
        }

        saiRetVal = xpSaiGetTunnelMapEntryAttribute(tunnel_map_entry_id, tnlMapEntry,
                                                    &attr_list[count]);
        if (SAI_STATUS_SUCCESS != saiRetVal)
        {
            XP_SAI_LOG_ERR("xpSaiGetTunnelMapEntryAttribute failed, error: %d.\n",
                           saiRetVal);
            return saiRetVal;
        }
    }

    return saiRetVal;
}

//Func: xpSaiTunnelOnBridgePortAdded

sai_status_t xpSaiTunnelOnBridgePortAdded(sai_object_id_t bridgeId,
                                          sai_object_id_t bridgePortId, sai_object_id_t tunnelId)
{
    XP_STATUS                 retVal       = XP_NO_ERR;
    xpSaiTunnelBridgeInfoT    *entry       = NULL;
    xpSaiTunnelBridgeInfoT    *newEntry    = NULL;
    xpsDevice_t               xpsDevId     = 0;
    xpsScope_t                scope        = 0;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    /* Check incoming parameters */
    if (!XDK_SAI_OBJID_TYPE_CHECK(bridgeId, SAI_OBJECT_TYPE_BRIDGE) ||
        !XDK_SAI_OBJID_TYPE_CHECK(bridgePortId, SAI_OBJECT_TYPE_BRIDGE_PORT) ||
        !XDK_SAI_OBJID_TYPE_CHECK(tunnelId, SAI_OBJECT_TYPE_TUNNEL))
    {
        XP_SAI_LOG_DBG("Wrong object type received!\n");
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }
    /* Get xpsDevId from tunnel_map_entry_id */
    xpsDevId = xpSaiObjIdSwitchGet(tunnelId);

    /* Get scope from xpsDevId */
    scope = xpSaiScopeFromDevGet(xpsDevId);

    retVal = xpSaiTunnelBridgeInfoInsertCtxDb(xpsDevId, tunnelId, &entry);
    if (retVal == XP_ERR_KEY_EXISTS)
    {
        retVal = xpSaiTunnelBridgeInfoGetCtxDb(xpsDevId, tunnelId, &entry);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpSaiTunnelBridgeInfoGetCtxDb: Failed to fetch from XPS DB for tunnelId: %"
                           PRIu64 " | error: %d.\n", tunnelId, retVal);
            return xpsStatus2SaiStatus(retVal);
        }

    }
    else if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiTunnelBridgeInfoInsertCtxDb: Unable to allocate XPS state Heap, error: %d.\n",
                       retVal);
        return retVal;
    }

    if (xpsDAIsCtxGrowthNeeded(entry->brPortCnt, XP_SAI_TUNNEL_THRESHOLD))
    {
        /* Double the entry list array size */
        retVal = xpSaiDynamicArrayGrow((void**)&newEntry, (void*)entry,
                                       sizeof(xpSaiTunnelBridgeInfoT),
                                       sizeof(xpSaiTnlBrPort), entry->brPortCnt,
                                       XP_SAI_TUNNEL_THRESHOLD);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpSaiDynamicArrayGrow: Dynamic array grow failed, error: %d.\n",
                           retVal);
            return xpsStatus2SaiStatus(retVal);
        }

        /* Remove the corresponding state from SAI TUNNEL DB and add new state */
        retVal = xpSaiTunnelBridgeInfoDeleteCtxDb(xpsDevId, tunnelId);
        if (retVal != XP_NO_ERR)
        {
            xpsStateHeapFree((void*)newEntry);
            XP_SAI_LOG_ERR("xpSaiTunnelBridgeInfoDeleteCtxDb: Deletion of data failed for tunnelId: %"
                           PRIu64 " | error: %d.\n", tunnelId, retVal);
            return xpsStatus2SaiStatus(retVal);
        }

        /* Insert the new state */
        retVal = xpsStateInsertData(scope, xpSaiTunnelBridgeStateDbHndl,
                                    (void*)newEntry);
        if (retVal != XP_NO_ERR)
        {
            xpsStateHeapFree((void*)newEntry);
            XP_SAI_LOG_ERR("xpsStateInsertData: Insertion of data failed, error: %d.\n",
                           retVal);
            return retVal;
        }

        entry = newEntry;
    }

    entry->tnlBrPortList[entry->brPortCnt].brPortId = bridgePortId;
    entry->tnlBrPortList[entry->brPortCnt].brId = bridgeId;
    entry->tnlBrPortList[entry->brPortCnt].isTnlOnBrPortAdded = true;
    entry->brPortCnt++;

    return xpsStatus2SaiStatus(retVal);
}

//Func: xpSaiTunnelOnBridgePortRemoved

sai_status_t xpSaiTunnelOnBridgePortRemoved(sai_object_id_t bridgeId,
                                            sai_object_id_t bridgePortId, sai_object_id_t tunnelId)
{
    XP_STATUS         retVal            = XP_NO_ERR;
    xpSaiTunnelBridgeInfoT *entry       = NULL;
    xpSaiTunnelBridgeInfoT *newEntry    = NULL;
    xpsDevice_t       xpsDevId          = 0;
    xpsScope_t        scope             = 0;
    sai_uint8_t       isBrPortIdPresent = 0;
    sai_uint32_t      count             = 0;

    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    /* Check incoming parameters */
    if (!XDK_SAI_OBJID_TYPE_CHECK(bridgeId, SAI_OBJECT_TYPE_BRIDGE) ||
        !XDK_SAI_OBJID_TYPE_CHECK(bridgePortId, SAI_OBJECT_TYPE_BRIDGE_PORT) ||
        !XDK_SAI_OBJID_TYPE_CHECK(tunnelId, SAI_OBJECT_TYPE_TUNNEL))
    {
        XP_SAI_LOG_DBG("Wrong object type received!\n");
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    /* Get xpsDevId from tunnel_map_entry_id */
    xpsDevId = xpSaiObjIdSwitchGet(tunnelId);

    /* Get scope from xpsDevId */
    scope = xpSaiScopeFromDevGet(xpsDevId);

    /* Search for the node in global SAI TUNNEL DB */
    retVal = xpSaiTunnelBridgeInfoGetCtxDb(xpsDevId, tunnelId, &entry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiTunnelBridgeInfoGetCtxDb: Failed to fetch from XPS DB for tunnelId: %"
                       PRIu64 " | error: %d.\n", tunnelId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    /* Remove the Bridge Port Id from Tunnel DB */
    for (count = 0; count < entry->brPortCnt; ++count)
    {
        if (!isBrPortIdPresent)
        {
            if (entry->tnlBrPortList[count].brPortId == bridgePortId)
            {
                isBrPortIdPresent = 1;
            }
            continue;
        }
        else
        {
            entry->tnlBrPortList[count-1].brPortId = entry->tnlBrPortList[count].brPortId;
            entry->tnlBrPortList[count-1].brId = entry->tnlBrPortList[count].brId;
        }
    }

    if (!isBrPortIdPresent)
    {
        return xpsStatus2SaiStatus(XP_ERR_INVALID_ARG);
    }

    /* Decrement the brPortCnt count */
    entry->brPortCnt--;

    if (xpsDAIsCtxShrinkNeeded(entry->brPortCnt, XP_SAI_TUNNEL_THRESHOLD))
    {
        /* Shrink the entryId array size */
        retVal = xpSaiDynamicArrayShrink((void**)&newEntry, (void*)entry,
                                         sizeof(xpSaiTunnelBridgeInfoT),
                                         sizeof(xpSaiTnlBrPort), entry->brPortCnt,
                                         XP_SAI_TUNNEL_THRESHOLD);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpSaiDynamicArrayShrink: Dynamic array shrink failed, error: %d.\n",
                           retVal);
            return xpsStatus2SaiStatus(retVal);
        }

        /* Remove the corresponding state from SAI TUNNEL DB and add new state */
        retVal = xpSaiTunnelBridgeInfoDeleteCtxDb(xpsDevId, tunnelId);
        if (retVal != XP_NO_ERR)
        {
            xpsStateHeapFree((void*)newEntry);
            XP_SAI_LOG_ERR("xpSaiTunnelBridgeInfoDeleteCtxDb: Deletion of data failed for tunnelId: %"
                           PRIu64 " | error: %d.\n", tunnelId, retVal);
            return xpsStatus2SaiStatus(retVal);
        }

        /* Insert the new state */
        retVal = xpsStateInsertData(scope, xpSaiTunnelBridgeStateDbHndl,
                                    (void*)newEntry);
        if (retVal != XP_NO_ERR)
        {
            xpsStateHeapFree((void*)newEntry);
            XP_SAI_LOG_ERR("xpsStateInsertData: Insertion of data failed, error: %d.\n",
                           retVal);
            return retVal;
        }
    }

    return xpsStatus2SaiStatus(retVal);
}

sai_status_t xpSaiTunnelOnNextHopChanged(uint32_t nextHopId)
{
    XP_STATUS           retVal         = XP_NO_ERR;
    sai_status_t        saiRetVal      = SAI_STATUS_SUCCESS;
    xpsDevice_t         xpsDevId       = 0;
    xpSaiTunnelEntryInfoT *curDbEntry  = NULL;
    xpSaiTunnelBridgeInfoT *tunBridgeEntry = NULL;

    xpsL3NextHopEntry_t nextHopEntry;
    //ipv4Addr_t          localIp;
    //ipv4Addr_t          mcEndpointIp;

    memset(&nextHopEntry, 0, sizeof(xpsL3NextHopEntry_t));

    /*TODO fetch NH details from SAI NH DB instaead of xps*/
    saiRetVal = xpSaiNextHopGetXpsNextHop(nextHopId, &nextHopEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiNextHopGetXpsNextHop: Failed to get Next Hop entry, nhopId: %d | error: %d.\n",
                       nextHopId, saiRetVal);
        return saiRetVal;
    }

    retVal = xpsStateGetNextData(XP_SCOPE_DEFAULT, xpSaiTunnelTermEntryStateDbHndl,
                                 (xpsDbKey_t)NULL, (void **)&curDbEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get first tunnel entry from DB, error: %d.\n",
                       retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    /* Loop through tunnel entries and find the one with the same tunnel interface ID. */
    while (curDbEntry != NULL)
    {
        if ((curDbEntry->nextHopId == nextHopId) &&
            (nextHopEntry.pktCmd != XP_PKTCMD_DROP) &&
            (nextHopEntry.pktCmd != XP_PKTCMD_TRAP) &&
            (memcmp(&nextHopEntry, &curDbEntry->nextHopEntry,
                    sizeof(xpsL3NextHopEntry_t)) != 0))
        {
            /* Get xpsDevId from tunnelId */
            xpsDevId = xpSaiObjIdSwitchGet(curDbEntry->tnlId);
            retVal = xpSaiTunnelBridgeInfoGetCtxDb(xpsDevId, curDbEntry->tnlId,
                                                   &tunBridgeEntry);
            if (retVal != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("xpSaiTunnelBridgeInfoGetCtxDb: Failed to fetch from XPS DB for tunnelId: %"
                               PRIu64 " | error: %d.\n", curDbEntry->tnlId, retVal);
                return xpsStatus2SaiStatus(retVal);
            }
            /*Update the tunnel next hop data*/
            retVal = xpsVxlanSetTunnelNextHopData(xpsDevId, curDbEntry->tnlIntfId,
                                                  curDbEntry->nextHopId);
            if (retVal != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("xpsVxlanSetTunnelNextHopData: Failed to set tunnel next hop data, tnlIntfId: %d | error: %d.\n",
                               curDbEntry->tnlIntfId, retVal);
                return xpsStatus2SaiStatus(retVal);
            }

#if 0 //P2MP tunnel not supported
            for (uint32_t count = 0; count < tunBridgeEntry->brPortCnt; ++count)
            {
                if (!tunBridgeEntry->tnlBrPortList[count].isTnlBrPortL2McGroupMbr)
                {
                    /* Set tunnel next hop data */
                    retVal = xpsVxlanSetTunnelNextHopData(xpsDevId, curDbEntry->tnlIntfId,
                                                          curDbEntry->nextHopId);
                    if (retVal != XP_NO_ERR)
                    {
                        XP_SAI_LOG_ERR("xpsVxlanSetTunnelNextHopData: Failed to set tunnel next hop data, tnlIntfId: %d | error: %d.\n",
                                       curDbEntry->tnlIntfId, retVal);
                        return xpsStatus2SaiStatus(retVal);
                    }
                }
                else
                {
                    xpsIpMcOIFData_t mcData;

                    memset(&mcData, 0, sizeof(xpsIpMcOIFData_t));

                    /* Set local/remote IPs addresses for xps tunnel */
                    xpSaiIpCopyAndReverse(localIp, (uint8_t*)&curDbEntry->dstIpAddr.addr.ip4,
                                          SAI_IP_ADDR_FAMILY_IPV4);
                    xpSaiIpCopyAndReverse(mcEndpointIp,
                                          (uint8_t*)&tunBridgeEntry->tnlBrPortList[count].tnlEndpointIp.addr.ip4,
                                          SAI_IP_ADDR_FAMILY_IPV4);

                    /* Initialize the tunnel params */
                    mcData.tunnelData.type = XP_IP_VXLAN_TUNNEL;
                    COPY_IPV4_ADDR_T(mcData.tunnelData.lclEpIpAddr, localIp);
                    COPY_IPV4_ADDR_T(mcData.tunnelData.rmtEpIpAddr, mcEndpointIp);

                    /* Add a VXLAN tunnel Mcast entry to a device */
                    mcData.l3IntfId = nextHopEntry.nextHop.l3InterfaceId;
                    mcData.egressIntfId = nextHopEntry.nextHop.egressIntfId;

                    /* Update the tunnel with OIF information */
                    retVal = xpsIpTnlSetMcOIF(xpsDevId, curDbEntry->tnlIntfId, &mcData);
                    if (retVal != XP_NO_ERR)
                    {
                        XP_SAI_LOG_ERR("xpsIpTnlSetMcOIF: Failed to set MCast OIF data for tunnelIntf: %d | error: %d",
                                       curDbEntry->tnlIntfId, retVal);
                        return retVal;
                    }
                }
            }
#endif //P2MP tunnel not supported
            memcpy(&curDbEntry->nextHopEntry, &nextHopEntry, sizeof(xpsL3NextHopEntry_t));
        }
        retVal = xpsStateGetNextData(XP_SCOPE_DEFAULT, xpSaiTunnelTermEntryStateDbHndl,
                                     (xpsDbKey_t)curDbEntry, (void **)&curDbEntry);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to get next tunnel entry from DB, error: %d.\n", retVal);
            return xpsStatus2SaiStatus(retVal);
        }
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiTunnelGetTunnelInterfaceListForTunnelId(xpsDevice_t xpsDevId,
                                                          sai_object_id_t tunnelId, uint32_t termEntryCount,
                                                          xpsInterfaceId_t* tnlIntfIdList)
{
    XP_STATUS retVal   = XP_NO_ERR;

    uint32_t count = 0;
    xpSaiTunnelInfoT *tunnelInfo = NULL;
    xpSaiTunnelEntryInfoT *tnlTermTblEntry = NULL;

    if (!XDK_SAI_OBJID_TYPE_CHECK(tunnelId, SAI_OBJECT_TYPE_TUNNEL))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).\n",
                       xpSaiObjIdTypeGet(tunnelId));
        return SAI_STATUS_INVALID_PARAMETER;
    }
    else if (termEntryCount == 0)
    {
        XP_SAI_LOG_ERR("No HW tunnel interface created !\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }
    /* Search for tunnelInfo node in global SAI TUNNEL DB with tunnelId key*/
    retVal = xpSaiTunnelGetCtxDb(xpsDevId, tunnelId, &tunnelInfo);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiTunnelGetCtxDb: Failed to fetch from XPS DB for tunnelId: %"
                       PRIu64 " | error: %d.\n", tunnelId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }
    for (count = 0; count < termEntryCount; count++)
    {
        /* Fetch from state DB */
        retVal = xpSaiTunnelEntryGetCtxDb(xpsDevId, tunnelInfo->entryId[count],
                                          &tnlTermTblEntry);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpSaiTunnelEntryGetCtxDb: Failed to fetch from XPS DB for : %"
                           PRIu64 " | error: %d.\n", tunnelInfo->entryId[count], retVal);
            return xpsStatus2SaiStatus(retVal);
        }
        tnlIntfIdList[count] = tnlTermTblEntry->tnlIntfId;
    }
    return xpsStatus2SaiStatus(retVal);
}

sai_status_t xpSaiTunnelGetEgressEncapInfoFromTunnelIdandBridgeId(
    xpsDevice_t xpsDevId, sai_object_id_t tunnelId, sai_object_id_t bridgeId,
    uint32_t* servInstanceId)
{
    XP_STATUS retVal   = XP_NO_ERR;
    xpSaiTunnelInfoT *entry = NULL;
    xpSaiTunnelMapInfoT      *pTnlMap      = NULL;
    xpSaiTunnelMapEntryInfoT *pTnlMapEntry = NULL;
    int32_t                   mapType      = 0;
    uint32_t        tnlMapId = 0;
    uint32_t count = 0;
    /* Check incoming parameters */
    if (!XDK_SAI_OBJID_TYPE_CHECK(bridgeId, SAI_OBJECT_TYPE_BRIDGE) ||
        !XDK_SAI_OBJID_TYPE_CHECK(tunnelId, SAI_OBJECT_TYPE_TUNNEL))
    {
        XP_SAI_LOG_DBG("Wrong object type received!\n");
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }
    /* Get xpsDevId from tunnel_map_entry_id */
    xpsDevId = xpSaiObjIdSwitchGet(tunnelId);

    /* Search for the node in global SAI TUNNEL DB */
    retVal = xpSaiTunnelGetCtxDb(xpsDevId, tunnelId, &entry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiTunnelGetCtxDb: Failed to fetch from XPS DB for tunnelId: %"
                       PRIu64 " | error: %d.\n", tunnelId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    /*traverse through the encapmap list and find vni encap data for the bdId*/
    for (count = 0; count < entry->ecpMapListCnt; ++count)
    {
        /* Search for the node in global SAI TUNNEL MAP DB */
        retVal = xpSaiTunnelMapGetCtxDb(xpsDevId, entry->ecpMapList[count], &pTnlMap);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpSaiTunnelMapGetCtxDb: Failed to fetch from XPS DB for tnlMapId: %"
                           PRIu64 " | error: %d.\n", tnlMapId, retVal);
            return xpsStatus2SaiStatus(retVal);
        }
        mapType = pTnlMap->mapType;

        if (mapType == SAI_TUNNEL_MAP_TYPE_BRIDGE_IF_TO_VNI)
        {
            uint32_t entryCnt = 0;

            for (entryCnt = 0; entryCnt < pTnlMap->mapEntryCnt; ++entryCnt)
            {
                /* Search for the node in global SAI TUNNEL MAP ENTRY DB */
                retVal = xpSaiTunnelMapEntryGetCtxDb(xpsDevId, pTnlMap->mapEntryId[entryCnt],
                                                     &pTnlMapEntry);
                if (retVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("xpSaiTunnelMapEntryGetCtxDb: Failed to fetch from XPS DB for tnlMapEntryId: %"
                                   PRIu64 " | error: %d.\n", pTnlMap->mapEntryId[entryCnt], retVal);
                    return xpsStatus2SaiStatus(retVal);
                }

                if (bridgeId == pTnlMapEntry->bridgeIdKey)
                {
                    *servInstanceId = pTnlMapEntry->vniId;
                    return xpsStatus2SaiStatus(retVal);
                }
            }
        }
        else if (mapType == SAI_TUNNEL_MAP_TYPE_VLAN_ID_TO_VNI)
        {
            uint32_t entryCnt = 0;

            for (entryCnt = 0; entryCnt < pTnlMap->mapEntryCnt; ++entryCnt)
            {
                /* Search for the node in global SAI TUNNEL MAP ENTRY DB */
                retVal = xpSaiTunnelMapEntryGetCtxDb(xpsDevId, pTnlMap->mapEntryId[entryCnt],
                                                     &pTnlMapEntry);
                if (retVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("xpSaiTunnelMapEntryGetCtxDb: Failed to fetch from XPS DB for tnlMapEntryId: %"
                                   PRIu64 " | error: %d.\n", pTnlMap->mapEntryId[entryCnt], retVal);
                    return xpsStatus2SaiStatus(retVal);
                }

                if (bridgeId == pTnlMapEntry->vlanIdKey)
                {
                    *servInstanceId = pTnlMapEntry->vniId;
                    return xpsStatus2SaiStatus(retVal);
                }
            }
        }
        else if (mapType == SAI_TUNNEL_MAP_TYPE_VIRTUAL_ROUTER_ID_TO_VNI)
        {
            /* This map is not used now */
        }
    }
    return xpsStatus2SaiStatus(retVal);
}
sai_status_t xpSaiTunnelGetHwTunnelIntfIdByTunnIdAndRemoteVtepIp(
    xpsDevice_t xpsDevId, sai_object_id_t tunnelId, sai_ip_address_t remoteVtepIp,
    xpsInterfaceId_t *tnlIntfId)
{
    XP_STATUS retVal   = XP_NO_ERR;

    uint32_t count = 0;
    xpSaiTunnelInfoT *tunnelInfo = NULL;
    xpSaiTunnelEntryInfoT *tnlTermTblEntry = NULL;

    if (!XDK_SAI_OBJID_TYPE_CHECK(tunnelId, SAI_OBJECT_TYPE_TUNNEL))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).\n",
                       xpSaiObjIdTypeGet(tunnelId));
        return SAI_STATUS_INVALID_PARAMETER;
    }
    else if (remoteVtepIp.addr.ip4 == 0)
    {
        XP_SAI_LOG_ERR("Invalid remoteEndpointIp !\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    /* Search for tunnelInfo node in global SAI TUNNEL DB with tunnelId key*/
    retVal = xpSaiTunnelGetCtxDb(xpsDevId, tunnelId, &tunnelInfo);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiTunnelGetCtxDb: Failed to fetch from XPS DB for tunnelId: %"
                       PRIu64 " | error: %d.\n", tunnelId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }
    uint8_t remoteVtepIpFound = 0;
    for (count = 0; count < XP_SAI_TUNNEL_THRESHOLD; count++)
    {
        /* Fetch from state DB */
        retVal = xpSaiTunnelEntryGetCtxDb(xpsDevId, tunnelInfo->entryId[count],
                                          &tnlTermTblEntry);
        if (retVal != XP_NO_ERR)
        {
            if (retVal == XP_ERR_KEY_NOT_FOUND)
            {
                XP_SAI_LOG_INFO("xpSaiTunnelEntryGetCtxDb: Failed to fetch from XPS DB for tnlTermTblEntry: %"
                                PRIu64 " | error: %d.\n", tunnelInfo->entryId[count], retVal);
                continue;
            }
            else
            {
                XP_SAI_LOG_ERR("xpSaiTunnelEntryGetCtxDb: Failed to fetch from XPS DB for tnlTermTblEntry: %"
                               PRIu64 " | error: %d.\n", tunnelInfo->entryId[count], retVal);
                return xpsStatus2SaiStatus(retVal);
            }

        }
        if (tnlTermTblEntry->srcIpAddr.addr.ip4 == remoteVtepIp.addr.ip4)
        {
            remoteVtepIpFound = 1;
            *tnlIntfId = tnlTermTblEntry->tnlIntfId;
            return xpsStatus2SaiStatus(retVal);
        }
    }
    if (remoteVtepIpFound == 0)
    {
        XP_SAI_LOG_ERR(" xpSaiTunnelGetHwTunnelIntfIdByTunnIdAndRemoteVtepIp:TunnelTermEntry not created for tunnelId: %"
                       PRIu64"\n", tunnelId);
    }
    return SAI_STATUS_ITEM_NOT_FOUND;
}
//Func: xpSaiTunnelIntfOnMcMemberAdded

sai_status_t xpSaiTunnelIntfOnMcMemberAdded(sai_object_id_t tunnelId,
                                            sai_object_id_t brPortId, sai_object_id_t l2McGroupId,
                                            sai_ip_address_t epIpAddr)
{
    sai_status_t        saiRetVal = SAI_STATUS_SUCCESS;
    XP_STATUS           retVal    = XP_NO_ERR;
    xpSaiTunnelInfoT   *entry     = NULL;
    xpSaiTunnelBridgeInfoT *entryBridge = NULL;
    xpsDevice_t         xpsDevId  = 0;
    sai_uint32_t        count     = 0;
    ipv4Addr_t          localIp;
    ipv4Addr_t          remoteIp;
    xpsInterfaceId_t    tnlIntfId = XPS_INTF_INVALID_ID;
    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    /* Check incoming parameters */
    if (!XDK_SAI_OBJID_TYPE_CHECK(tunnelId, SAI_OBJECT_TYPE_TUNNEL) ||
        !XDK_SAI_OBJID_TYPE_CHECK(brPortId, SAI_OBJECT_TYPE_BRIDGE_PORT) ||
        !XDK_SAI_OBJID_TYPE_CHECK(l2McGroupId, SAI_OBJECT_TYPE_L2MC_GROUP))
    {
        XP_SAI_LOG_DBG("Wrong object type received!\n");
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    /* Get xpsDevId from tunnelId */
    xpsDevId = xpSaiObjIdSwitchGet(tunnelId);

    /* Search for the node in global SAI TUNNEL DB */
    retVal = xpSaiTunnelGetCtxDb(xpsDevId, tunnelId, &entry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiTunnelGetCtxDb: Failed to fetch from XPS DB for tunnelId: %"
                       PRIu64 " | error: %d.\n", tunnelId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }
    retVal = xpSaiTunnelBridgeInfoGetCtxDb(xpsDevId, tunnelId, &entryBridge);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiTunnelBridgeInfoGetCtxDb: Failed to fetch from XPS DB for tunnelId: %"
                       PRIu64 " | error: %d.\n", tunnelId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    saiRetVal = xpSaiTunnelGetHwTunnelIntfIdByTunnIdAndRemoteVtepIp(xpsDevId,
                                                                    tunnelId, epIpAddr, &tnlIntfId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiXpsTunnelIntfByTunnelObjIdGet: Failed to fetch from XPS DB for tunnelId: %"
                       PRIu64 " | error: %d.\n", tunnelId, saiRetVal)
        return saiRetVal;
    }
    if (tnlIntfId != XPS_INTF_INVALID_ID)
    {
        /* Set local/remote IPs addresses for xps tunnel */
        xpSaiIpCopyAndReverse(localIp, (uint8_t*)&entry->tnlSrcIp.addr.ip4,
                              SAI_IP_ADDR_FAMILY_IPV4);
        xpSaiIpCopyAndReverse(remoteIp, (uint8_t*)&epIpAddr.addr.ip4,
                              SAI_IP_ADDR_FAMILY_IPV4);

        /* Add tunnel interface to L2MC group */
        saiRetVal = xpSaiL2McGroupAddVxTunnel(l2McGroupId, tnlIntfId);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiL2McGroupAddVxTunnel: Failed to add L2MC group member, l2McGroupId %"
                           PRIu64 " | error: %d.\n",
                           l2McGroupId, saiRetVal);
        }

#if 0 //P2MP tunnel not supported
        /* If the command is drop or trap the NH entry will contain invalid egress data so just do nothing here.
         * When NH is unresolved with L3 router VLAN egress interface we'll get the notification when the
         * NH is resolved so will program the NH data when such event occurs.
         */
        if ((entry->nextHopEntry.pktCmd != XP_PKTCMD_DROP) &&
            (entry->nextHopEntry.pktCmd != XP_PKTCMD_TRAP))
        {

            xpsIpMcOIFData_t mcData;

            memset(&mcData, 0, sizeof(xpsIpMcOIFData_t));

            /* Initialize the tunnel params */
            mcData.tunnelData.type = XP_IP_VXLAN_TUNNEL;
            COPY_IPV4_ADDR_T(mcData.tunnelData.lclEpIpAddr, localIp);
            COPY_IPV4_ADDR_T(mcData.tunnelData.rmtEpIpAddr, remoteIp);

            /* Add a VXLAN tunnel Mcast entry to a device */
            mcData.l3IntfId = entry->nextHopEntry.nextHop.l3InterfaceId;
            mcData.egressIntfId = entry->nextHopEntry.nextHop.egressIntfId;

            /* Update the tunnel with OIF information */
            retVal = xpsIpTnlSetMcOIF(xpsDevId, tnlIntfId, &mcData);
            if (retVal != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("xpsIpTnlSetMcOIF: Failed to set MCast OIF data for tunnelIntf: %d | error: %d",
                               tnlIntfId, retVal);
                return retVal;
            }
        }
#endif //P2MP tunnel not supported
    }
    for (count = 0; count < entryBridge->brPortCnt; ++count)
    {
        if (entryBridge->tnlBrPortList[count].brPortId == brPortId)
        {
            entryBridge->tnlBrPortList[count].isTnlBrPortL2McGroupMbr = true;
            entryBridge->tnlBrPortList[count].l2McGroupId = l2McGroupId;
            memcpy(&entryBridge->tnlBrPortList[count].tnlEndpointIp, &epIpAddr,
                   sizeof(sai_ip_address_t));
        }
    }

    return xpsStatus2SaiStatus(retVal);
}

//Func: xpSaiTunnelIntfOnMcMemberRemoved

sai_status_t xpSaiTunnelIntfOnMcMemberRemoved(sai_object_id_t tunnelId,
                                              sai_object_id_t brPortId, sai_object_id_t l2McGroupId,
                                              sai_ip_address_t epIpAddr)
{
    sai_status_t      saiRetVal = SAI_STATUS_SUCCESS;
    XP_STATUS         retVal    = XP_NO_ERR;
    xpSaiTunnelBridgeInfoT *entryBridge = NULL;
    xpsDevice_t       xpsDevId  = 0;
    sai_uint32_t      count     = 0;
    xpsInterfaceId_t  tnlIntfId = XPS_INTF_INVALID_ID;
    XP_SAI_LOG_DBG("%s %d:\n", __FUNCNAME__, __LINE__);

    /* Check incoming parameters */
    if (!XDK_SAI_OBJID_TYPE_CHECK(tunnelId, SAI_OBJECT_TYPE_TUNNEL) ||
        !XDK_SAI_OBJID_TYPE_CHECK(brPortId, SAI_OBJECT_TYPE_BRIDGE_PORT) ||
        !XDK_SAI_OBJID_TYPE_CHECK(l2McGroupId, SAI_OBJECT_TYPE_L2MC_GROUP))
    {
        XP_SAI_LOG_DBG("Wrong object type received!\n");
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    /* Get xpsDevId from tunnelId */
    xpsDevId = xpSaiObjIdSwitchGet(tunnelId);
    saiRetVal = xpSaiTunnelGetHwTunnelIntfIdByTunnIdAndRemoteVtepIp(xpsDevId,
                                                                    tunnelId, epIpAddr, &tnlIntfId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiXpsTunnelIntfByTunnelObjIdGet: Failed to fetch from XPS DB for tunnelId: %"
                       PRIu64 " | error: %d.\n", tunnelId, saiRetVal)
        return saiRetVal;
    }

    if (tnlIntfId != XPS_INTF_INVALID_ID)
    {
        /* Add tunnel interface to L2MC group */
        saiRetVal = xpSaiL2McGroupRemoveVxTunnel(l2McGroupId, tnlIntfId);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiL2McGroupRemoveVxTunnel: Failed to remove L2MC group member, l2McGroupId %"
                           PRIu64 " | error: %d.\n",
                           l2McGroupId, saiRetVal);
        }
    }

    retVal = xpSaiTunnelBridgeInfoGetCtxDb(xpsDevId, tunnelId, &entryBridge);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiTunnelBridgeInfoGetCtxDb: Failed to fetch from XPS DB for tunnelId: %"
                       PRIu64 " | error: %d.\n", tunnelId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    for (count = 0; count < entryBridge->brPortCnt; ++count)
    {
        if (entryBridge->tnlBrPortList[count].brPortId == brPortId)
        {
            entryBridge->tnlBrPortList[count].isTnlBrPortL2McGroupMbr = false;
            entryBridge->tnlBrPortList[count].l2McGroupId = SAI_NULL_OBJECT_ID;
            memset(&entryBridge->tnlBrPortList[count].tnlEndpointIp, 0,
                   sizeof(sai_ip_address_t));
        }
    }

    return xpsStatus2SaiStatus(retVal);
}
//Func: xpSaiTunnelRvtepLearnHandler
/* Uncomennt when point to multipoint tunnel support will be added to profile */
#if 0
XP_STATUS xpSaiTunnelRvtepLearnHandler(xpsDevice_t xpsDevId, xphRxHdr *rxHdr,
                                       void *buf, uint16_t bufSize, void* userData)
{
    XP_STATUS          retVal = XP_NO_ERR;
    bool               isLclVtepAdded = false;
    uint8_t            srcMacAddr[XP_MAC_ADDR_LEN];
    uint8_t            dstMacAddr[XP_MAC_ADDR_LEN];
    uint8_t           *origBuf;
    xpsInterfaceId_t   tnlIntfId;
    xpsL2EncapType_e   type;
    xpsL3RouteEntry_t  l3RouteEntry;
    uint32_t           nHopId = 0;
    uint8_t            vni[3];
    uint32_t           tnlVni;
    uint32_t           destPort, vxlanPort;
    uint32_t           bdId;
    uint32_t           vrfId;
    uint32_t           prefixIndex = 0xFFFFFFFF;
    ipv4Addr_t         srcIp, dstIp, lclEpIpAddr, rmtEpIpAddr;
    uint32_t           eVif = 0, iVif = 0, etagOffset = 0;
    xpEtagsStruct      etagsStruct;
    xpsFdbEntry_t      fdbEntry;
    uint32_t           fdbIdx = 0;
    xpsHashIndexList_t hashList = { 0, { 0xFFFFFFFF, } };

    origBuf = (uint8_t *)buf;

    /* Parse the packet fields */
    memcpy(&dstMacAddr, buf, XP_MAC_ADDR_LEN);

    buf = (uint8_t *)buf + XP_MAC_ADDR_LEN;
    memcpy(&srcMacAddr, buf, XP_MAC_ADDR_LEN);

    buf = (uint8_t *)buf + XP_MAC_ADDR_LEN;

    if (0 == xpDecodeEtags((uint8_t*)buf, &etagsStruct))
    {
        etagOffset = XP_ETAG_HEADER_LEN;

        iVif = etagsStruct.ingressECidExt;
        iVif = (iVif << 4) | (etagsStruct.ingressECidBaseMSB & 0xF);
        iVif = (iVif << 8) | (etagsStruct.ingressECidBaseLSB & 0xFF);
        eVif = etagsStruct.egressECidExt;
        eVif = (eVif << 4) | (etagsStruct.egressECidBaseMSB & 0xF);
        eVif = (eVif << 8) | (etagsStruct.egressECidBaseLSB & 0xFF);
    }

    buf = (uint8_t *)buf + etagOffset + XP_VLAN_ID_OFFSET;

    bdId = ntohs(*(uint32_t *)buf) & 0x0FFF;

    buf = origBuf + XP_ETH_TAG_HDR_LEN + XP_IP_HDR_SA_OFFSET + etagOffset;

    memcpy(&srcIp, buf, sizeof(ipv4Addr_t));

    buf = (uint8_t *)buf + sizeof(ipv4Addr_t);

    memcpy(&dstIp, buf, sizeof(ipv4Addr_t));

    buf = origBuf + XP_ETH_TAG_HDR_LEN + XP_IP_HDR_LEN + XP_UDP_DST_PORT_OFFSET +
          etagOffset;

    destPort = ntohs(*(uint16_t *)buf);

    retVal = xpsVxlanGetUdpPort(xpsDevId, &vxlanPort);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsVxlanGetUdpPort: Failed to get UDP port, error: %d.\n",
                       retVal);
        return retVal;
    }

    if (destPort != vxlanPort)
    {
        XP_SAI_LOG_ERR("Only VXLAN tunnel type is supported.\n");
        return XP_ERR_INVALID_VALUE;
    }
    else
    {
        type = XP_L2_ENCAP_VXLAN;
    }

    buf = origBuf + XP_ETH_TAG_HDR_LEN + etagOffset + XP_IP_HDR_LEN + XP_UDP_HDR_LEN
          + XP_TNL_VNI_OFFSET;

    memcpy(&vni, buf, XP_TNL_VNI_LEN);

    tnlVni = vni[2] + (vni[1] << 8) + (vni[0] << 16);

    retVal = xpsL3GetIntfVrf(xpsDevId, XPS_INTF_MAP_BD_TO_INTF(bdId), &vrfId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsL3GetIntfVrf: Failed to get VRF ID, error: %d.\n", retVal);
        return retVal;
    }

    xpSaiIpCopyAndReverse(lclEpIpAddr, dstIp, SAI_IP_ADDR_FAMILY_IPV4);
    xpSaiIpCopyAndReverse(rmtEpIpAddr, srcIp, SAI_IP_ADDR_FAMILY_IPV4);

    /* Check is a Vxlan local Vtep entry added to a device. */
    retVal = xpsVxlanIsLocalVtepAdded(xpsDevId, lclEpIpAddr, &isLclVtepAdded);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsVxlanIsLocalVtepAdded: Failed to check is VXLAN local Vtep entry added to a device, error: %d.\n",
                       retVal);
        return retVal;
    }

    if (!isLclVtepAdded)
    {
        return XP_ERR_PM_HWLOOKUP_FAIL;
    }

    /* Perform src ip lookup to obtain the nHopId */
    memset(&l3RouteEntry, 0, sizeof(xpsL3RouteEntry_t));
    memcpy(l3RouteEntry.ipv4Addr, rmtEpIpAddr, sizeof(ipv4Addr_t));
    l3RouteEntry.vrfId = vrfId;

    retVal = xpsL3FindIpRouteLpmEntry(xpsDevId, &l3RouteEntry, &prefixIndex);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsL3FindIpRouteLpmEntry: Failed to find IP route entry, error: %d.\n",
                       retVal);
        return retVal;
    }

    nHopId = l3RouteEntry.nhId;

    /* Create a VXLAN tunnel interface */
    retVal = xpsVxlanCreateTunnelInterface(lclEpIpAddr, rmtEpIpAddr, &tnlIntfId);
    if ((retVal != XP_NO_ERR) && (retVal != XP_ERR_KEY_EXISTS))
    {
        XP_SAI_LOG_ERR("xpsVxlanCreateTunnelInterface: Failed to create VXLAN Tunnel Interface, error: %d.\n",
                       retVal);
        return retVal;
    }

    if (retVal == XP_ERR_KEY_EXISTS)
    {
        XP_SAI_LOG_DBG("VXLAN Tunnel Interface (%d) is already created, error: %d.\n",
                       tnlIntfId, retVal);
        return XP_NO_ERR;
    }

    /* Add a VXLAN tunnel to a device */
    retVal = xpsVxlanAddTunnelEntry(xpsDevId, tnlIntfId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsVxlanAddTunnelEntry: Failed to add VXLAN Tunnel entry, tnlIntfId: %d, error: %d.\n",
                       tnlIntfId, retVal);
        xpsVxlanDestroyTunnelInterface(tnlIntfId);
        return retVal;
    }

    /* Set tunnel next hop data */
    retVal = xpsVxlanSetTunnelNextHopData(xpsDevId, tnlIntfId, nHopId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsVxlanSetTunnelNextHopData: Failed to set tunnel next hop data, error: %d.\n",
                       retVal);
        xpsVxlanRemoveTunnelEntry(xpsDevId, tnlIntfId);
        xpsVxlanDestroyTunnelInterface(tnlIntfId);
        return retVal;
    }

    /* Add tunnel interface to the Bridge domain (VLAN or .1D Bridge) */
    retVal = xpsVlanAddEndpoint(xpsDevId, bdId, tnlIntfId, type, tnlVni);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsVlanAddEndpoint: Failed to add endpoint Tunnel %d to the bridge domain %d, error: %d\n",
                       tnlIntfId, bdId, retVal);
        xpsVxlanRemoveTunnelEntry(xpsDevId, tnlIntfId);
        xpsVxlanDestroyTunnelInterface(tnlIntfId);
        return retVal;
    }

    memset(&fdbEntry, 0, sizeof(fdbEntry));

    fdbEntry.vlanId = (xpsVlan_t)bdId;
    memcpy(fdbEntry.macAddr, srcMacAddr, sizeof(fdbEntry.macAddr));
    xpSaiMacReverse(fdbEntry.macAddr);
    fdbEntry.pktCmd = XP_PKTCMD_FWD;
    fdbEntry.isStatic = false;
    fdbEntry.intfId = rxHdr->ingressPortNum;
    fdbEntry.serviceInstId = tnlVni;

    /* Need to do a lookup in the FDB table not to add the same entry in hw */
    retVal = xpsFdbFindEntry(xpsDevId, &fdbEntry, &fdbIdx);
    if ((retVal != XP_NO_ERR) && (retVal != XP_ERR_PM_HWLOOKUP_FAIL))
    {
        XP_SAI_LOG_ERR("xpsFdbFindEntry: FDB table lookup failed, error: %d.\n",
                       retVal);
        return retVal;
    }

    if (retVal == XP_ERR_PM_HWLOOKUP_FAIL)
    {
        retVal = xpsFdbAddEntry(xpsDevId, &fdbEntry, &hashList);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpsFdbAddEntry: Failed to add FDB entry, error: %d.\n", retVal);
            return retVal;
        }
    }

    return retVal;
}
#endif

sai_status_t xpSaiTunnelBridgePortSetFdbLearningMode(xpsDevice_t xpsDevId,
                                                     sai_object_id_t saiTnlId,
                                                     xpPktCmd_e learnMode)
{
    XP_STATUS                   retVal     = XP_NO_ERR;
    xpSaiTunnelInfoT            *saiTnlInfo = NULL;

    /*Get the term entry Ids from the tunnelId*/
    retVal = xpSaiTunnelGetCtxDb(xpsDevId, saiTnlId, &saiTnlInfo);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiTunnelGetCtxDb: Failed to fetch from XPS DB for tunnel_id: %"
                       PRIu64 " | error: %d.\n", saiTnlId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    if (saiTnlInfo->tunnelType == SAI_TUNNEL_TYPE_VXLAN)
    {
        retVal = xpsVxlanTunnelLearnModeSet(xpsDevId, saiTnlInfo->tnlIntfId, learnMode);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpsVxlanTunnelLearnModeSet Failed: %"
                           PRIu64 " | error: %d.\n", saiTnlId, retVal);
            return xpsStatus2SaiStatus(retVal);
        }
    }
    else
    {
        XP_SAI_LOG_ERR("Support only Vxlan: %" PRIu64 "\n", saiTnlId);
        return SAI_STATUS_FAILURE;
    }

    saiTnlInfo->inrSaMissCmd = learnMode;

    return xpsStatus2SaiStatus(retVal);
}

sai_status_t xpSaiTunnelGetTunnelId(uint32_t ingressVif,
                                    sai_object_id_t *saiTnlId)
{
    XP_STATUS          status    = XP_NO_ERR;
    xpSaiTunnelEntryInfoT *curTnlTermDbEntry = NULL;
    status = xpsStateGetNextData(XP_SCOPE_DEFAULT, xpSaiTunnelTermEntryStateDbHndl,
                                 (xpsDbKey_t)NULL, (void **)&curTnlTermDbEntry);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get first tunnel term entry from DB, error: %d.\n",
                       status);
        return xpsStatus2SaiStatus(status);
    }
    /* Loop through tunnel term entries and find the one with the same xps tunnel interface ID. */
    while (curTnlTermDbEntry != NULL)
    {
        if (curTnlTermDbEntry->tnlIntfId == ingressVif)
        {
            *saiTnlId = curTnlTermDbEntry->tnlId;
            break;
        }
        status = xpsStateGetNextData(XP_SCOPE_DEFAULT, xpSaiTunnelTermEntryStateDbHndl,
                                     (xpsDbKey_t)curTnlTermDbEntry, (void **)&curTnlTermDbEntry);
        if (status != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to get next tunnel term entry from DB, error: %d.\n",
                           status);
            return xpsStatus2SaiStatus(status);
        }
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiTunnelGetTunnelIntf(xpsDevice_t xpsDevId,
                                      sai_object_id_t tunnelId,
                                      uint32_t *tunIntf)
{
    XP_STATUS         retVal  = XP_NO_ERR;
    xpSaiTunnelInfoT *pTunnel = NULL;

    retVal = xpSaiTunnelGetCtxDb(xpsDevId, tunnelId, &pTunnel);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiTunnelGetCtxDb: Failed to fetch from XPS DB for tunnelId: %"
                       PRIu64 " | error: %d.\n", tunnelId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    if (!pTunnel)
    {
        return XP_ERR_NULL_POINTER;
    }

    *tunIntf = pTunnel->tnlIntfId;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiTunnelInit

XP_STATUS xpSaiTunnelInit(xpsDevice_t xpsDevId)
{
    XP_STATUS  retVal = XP_NO_ERR;
    xpsScope_t scope  = xpSaiScopeFromDevGet(xpsDevId);

    retVal = xpsAllocatorInitIdAllocator(scope, XP_SAI_ALLOC_TUNNEL,
                                         XP_SAI_TUNNEL_MAX_IDS,
                                         XP_SAI_TUNNEL_RANGE_START);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsAllocatorInitIdAllocator: Failed to initialize SAI Tunnel ID allocator, error: %d.\n",
                       retVal);
        return retVal;
    }

    retVal = xpsAllocatorInitIdAllocator(scope, XP_SAI_ALLOC_TUNNEL_TERM_ENTRY,
                                         XP_SAI_TUNNEL_TERM_ENTRY_MAX_IDS,
                                         XP_SAI_TUNNEL_TERM_ENTRY_RANGE_START);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsAllocatorInitIdAllocator: Failed to initialize SAI Tunnel termination table entry ID allocator, error: %d.\n",
                       retVal);
        return retVal;
    }
    retVal = xpsAllocatorInitIdAllocator(scope, XP_SAI_ALLOC_TUNNEL_MAP,
                                         XP_SAI_TUNNEL_MAP_MAX_IDS,
                                         XP_SAI_TUNNEL_MAP_RANGE_START);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsAllocatorInitIdAllocator: Failed to initialize SAI Tunnel map ID allocator, error: %d.\n",
                       retVal);
        return retVal;
    }

    retVal = xpsAllocatorInitIdAllocator(scope, XP_SAI_ALLOC_TUNNEL_MAP_ENTRY,
                                         XP_SAI_TUNNEL_MAP_ENTRY_MAX_IDS,
                                         XP_SAI_TUNNEL_MAP_ENTRY_RANGE_START);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsAllocatorInitIdAllocator: Failed to initialize SAI Tunnel map entry ID allocator, error: %d.\n",
                       retVal);
        return retVal;
    }

    /* Create a global SAI TUNNEL DB */
    xpSaiTunnelStateDbHndl = XPSAI_TUNNEL_DB_HNDL;

    retVal = xpsStateRegisterDb(scope, "SAI TUNNEL DB", XPS_GLOBAL,
                                &xpSaiTunnelCtxKeyComp, xpSaiTunnelStateDbHndl);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateRegisterDb: Failed to Register SAI TUNNEL DB, error: %d.\n",
                       retVal);
        return retVal;
    }
    /* Create a global SAI TUNNEL DB */
    xpSaiTunnelBridgeStateDbHndl = XPSAI_TUNNEL_BRIDGE_DB_HNDL;

    retVal = xpsStateRegisterDb(scope, "SAI TUNNEL BRIDGE DB", XPS_GLOBAL,
                                &xpSaiTunnelBridgeCtxKeyComp, xpSaiTunnelBridgeStateDbHndl);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateRegisterDb: Failed to Register SAI TUNNEL DB, error: %d.\n",
                       retVal);
        return retVal;
    }
    /* Create a global SAI TUNNEL TERMTABLE ENTRY DB */
    xpSaiTunnelTermEntryStateDbHndl = XPSAI_TUNNEL_TERMTABLE_ENTRY_DB_HNDL;

    retVal = xpsStateRegisterDb(scope, "SAI TUNNEL TERMTABLE ENTRY DB", XPS_GLOBAL,
                                &xpSaiTunnelEntryCtxKeyComp, xpSaiTunnelTermEntryStateDbHndl);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateRegisterDb: Failed to Register SAI TUNNEL TERMTABLE ENTRY DB, error: %d.\n",
                       retVal);
        return retVal;
    }

    /* Create a global SAI TUNNEL MAP DB */
    xpSaiTunnelMapStateDbHndl = XPSAI_TUNNEL_MAP_DB_HNDL;

    retVal = xpsStateRegisterDb(scope, "SAI TUNNEL MAP DB", XPS_GLOBAL,
                                &xpSaiTunnelMapCtxKeyComp, xpSaiTunnelMapStateDbHndl);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateRegisterDb: Failed to Register SAI TUNNEL MAP DB, error: %d.\n",
                       retVal);
        return retVal;
    }

    /* Create a global SAI TUNNEL MAP ENTRY DB */
    xpSaiTunnelMapEntryStateDbHndl = XPSAI_TUNNEL_MAP_ENTRY_DB_HNDL;

    retVal = xpsStateRegisterDb(scope, "SAI TUNNEL MAP ENTRY DB", XPS_GLOBAL,
                                &xpSaiTunnelMapEntryCtxKeyComp, xpSaiTunnelMapEntryStateDbHndl);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateRegisterDb: Failed to Register SAI TUNNEL ENTRY MAP DB, error: %d.\n",
                       retVal);
        return retVal;
    }

    return retVal;
}

//Func: xpSaiTunnelDeInit

XP_STATUS xpSaiTunnelDeInit(xpsDevice_t xpsDevId)
{
    XP_STATUS  retVal = XP_NO_ERR;
    xpsScope_t scope  = xpSaiScopeFromDevGet(xpsDevId);

    /* Uncomennt when point to multipoint tunnel support will be added to profile */
#if 0
    retVal = xpsUnregisterRemoteVtepLearnHandler();
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsUnregisterRemoteVtepLearnHandler: Failed to unregister RVTEP handler, error: %d.\n",
                       retVal);
        return retVal;
    }
#endif

    /* Remove a global SAI TUNNEL DB */
    retVal = xpsStateDeRegisterDb(scope, &xpSaiTunnelStateDbHndl);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateDeRegisterDb: De-Register SAI TUNNEL DB failed, error: %d.\n",
                       retVal);
        return retVal;
    }

    /* Remove a global SAI TUNNEL BRIDGE DB */
    retVal = xpsStateDeRegisterDb(scope, &xpSaiTunnelBridgeStateDbHndl);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateDeRegisterDb: De-Register SAI TUNNEL Bridge DB failed, error: %d.\n",
                       retVal);
        return retVal;
    }
    /* Remove a global SAI TUNNEL TERMTABLE ENTRY DB */
    retVal = xpsStateDeRegisterDb(scope, &xpSaiTunnelTermEntryStateDbHndl);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateDeRegisterDb: De-Register SAI TUNNEL TERMTABLE ENTRY DB failed, error: %d.\n",
                       retVal);
        return retVal;
    }

    /* Remove a global SAI TUNNEL MAP DB */
    retVal = xpsStateDeRegisterDb(scope, &xpSaiTunnelMapStateDbHndl);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateDeRegisterDb: De-Register SAI TUNNEL MAP DB failed, error: %d.\n",
                       retVal);
        return retVal;
    }

    /* Remove a global SAI TUNNEL MAP ENTRY DB */
    retVal = xpsStateDeRegisterDb(scope, &xpSaiTunnelMapEntryStateDbHndl);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateDeRegisterDb: De-Register SAI TUNNEL MAP ENTRY DB failed, error: %d.\n",
                       retVal);
        return retVal;
    }

    return retVal;
}

//Func: xpSaiTunnelApiInit

XP_STATUS xpSaiTunnelApiInit(uint64_t flag,
                             const sai_service_method_table_t* adapHostServiceMethodTable)
{
    XP_STATUS    retVal    = XP_NO_ERR;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    _xpSaiTunnelApi = (sai_tunnel_api_t *) xpMalloc(sizeof(sai_tunnel_api_t));
    if (NULL == _xpSaiTunnelApi)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for _xpSaiTunnelApi.\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }

    _xpSaiTunnelApi->create_tunnel_map = xpSaiCreateTunnelMap;
    _xpSaiTunnelApi->remove_tunnel_map = xpSaiRemoveTunnelMap;
    _xpSaiTunnelApi->set_tunnel_map_attribute = xpSaiSetTunnelMapAttribute;
    _xpSaiTunnelApi->get_tunnel_map_attribute = xpSaiGetTunnelMapAttributes;
    _xpSaiTunnelApi->create_tunnel = xpSaiCreateTunnel;
    _xpSaiTunnelApi->remove_tunnel = xpSaiRemoveTunnel;
    _xpSaiTunnelApi->set_tunnel_attribute = xpSaiSetTunnelAttribute;
    _xpSaiTunnelApi->get_tunnel_attribute = xpSaiGetTunnelAttributes;
    _xpSaiTunnelApi->get_tunnel_stats = xpSaiTunnelGetStats;
    _xpSaiTunnelApi->clear_tunnel_stats = xpSaiTunnelClearStats;
    _xpSaiTunnelApi->create_tunnel_term_table_entry =
        xpSaiCreateTunnelTermTableEntry;
    _xpSaiTunnelApi->remove_tunnel_term_table_entry =
        xpSaiRemoveTunnelTermTableEntry;
    _xpSaiTunnelApi->set_tunnel_term_table_entry_attribute =
        xpSaiSetTunnelTermTableEntryAttribute;
    _xpSaiTunnelApi->get_tunnel_term_table_entry_attribute =
        xpSaiGetTunnelTermTableEntryAttributes;
    _xpSaiTunnelApi->create_tunnel_map_entry = xpSaiCreateTunnelMapEntry;
    _xpSaiTunnelApi->remove_tunnel_map_entry = xpSaiRemoveTunnelMapEntry;
    _xpSaiTunnelApi->set_tunnel_map_entry_attribute =
        xpSaiSetTunnelMapEntryAttribute;
    _xpSaiTunnelApi->get_tunnel_map_entry_attribute =
        xpSaiGetTunnelMapEntryAttributes;

    _xpSaiTunnelApi->get_tunnel_stats_ext = (sai_get_tunnel_stats_ext_fn)
                                            xpSaiStubGenericApi;

    saiRetVal = xpSaiApiRegister(SAI_API_TUNNEL, (void*)_xpSaiTunnelApi);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiApiRegister: Failed to register TUNNEL API, error: %d.\n",
                       saiRetVal);
        return XP_ERR_ARRAY_OUT_OF_BOUNDS;
    }

    return retVal;
}

//Func: xpSaiTunnelApiDeinit

XP_STATUS xpSaiTunnelApiDeinit()
{
    XP_STATUS retVal = XP_NO_ERR;

    xpFree(_xpSaiTunnelApi);
    _xpSaiTunnelApi = NULL;

    return retVal;
}
