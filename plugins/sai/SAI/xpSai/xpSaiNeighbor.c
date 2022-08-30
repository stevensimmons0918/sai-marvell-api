// xpSaiNeighbor.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpSai.h"
#include "xpSaiVrf.h"
#include "xpSaiUtil.h"
#include "xpSaiValidationArrays.h"

XP_SAI_LOG_REGISTER_API(SAI_API_NEIGHBOR);

#define XP_SAI_NEIGHBOR_FDB_MISS_PKTCMD          XP_PKTCMD_DROP
#define XP_SAI_NEIGHBOR_FDB_DEFAULT_MEMBER_COUNT 4
/**
 * @brief SAI Neighbor FDB Data Base Entry struct
 *
 * This structure is intended to serve as a fast access point to a
 * SAI neighbor entry during egress interface resolution which happens
 * when a FDB event occurs on the same router Bridge/VLAN the neighbor entry
 * is using.
 */
typedef struct xpSaiNeighborFdbEntry_t
{
    /** Bridge/VLAN object ID as a key */
    sai_object_id_t bv_id;
    /** MAC address as a key */
    sai_mac_t mac;

    /** List of SAI neighbors associated with these MAC and Bridge/VLAN */
    uint32_t numItems;
    xpSaiNeighborEntry_t* neighborsList[XP_SAI_NEIGHBOR_FDB_DEFAULT_MEMBER_COUNT];

} xpSaiNeighborFdbEntry_t;

static sai_neighbor_api_t* _xpSaiNeighborApi;
xpsDbHandle_t gsaiNeighborDbHdle = XPS_STATE_INVALID_DB_HANDLE;
xpsDbHandle_t gsaiNeighborFdbDbHdle = XPS_STATE_INVALID_DB_HANDLE;

static pthread_mutex_t gSaiNeighborLock = PTHREAD_MUTEX_INITIALIZER;

static inline sai_status_t xpSaiNeighborInitLock()
{
    if (pthread_mutex_init(&gSaiNeighborLock, NULL) != 0)
    {
        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

static inline sai_status_t xpSaiNeighborDeinitLock()
{
    if (pthread_mutex_destroy(&gSaiNeighborLock) != 0)
    {
        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

static inline void xpSaiNeighborLock()
{
    pthread_mutex_lock(&gSaiNeighborLock);
}

static inline void xpSaiNeighborUnlock()
{
    pthread_mutex_unlock(&gSaiNeighborLock);
}

/**
 * \brief Gets Neighbor FDB entry from state DB by Bridge/VLAN OID and MAC address
 * \param [in] xpsDevId    device identifier
 * \param [in] bv_id       Bridge/VLAN OID
 * \param [in] mac         MAC address
 * \param [out] entry      pointer to Neighbor FDB entry found
 * \return sai_status_t
 *
 * The pointer to Neighbor FDB entry returned points to the entry in the state DB
 */
static sai_status_t xpSaiGetNeighborFdbEntry(xpsDevice_t xpsDevId,
                                             sai_object_id_t bv_id,
                                             sai_mac_t mac,
                                             xpSaiNeighborFdbEntry_t **entry)
{
    xpSaiNeighborFdbEntry_t key;

    memset(&key, 0, sizeof(key));
    key.bv_id = bv_id;
    memcpy(key.mac, mac, XP_MAC_ADDR_LEN);

    /* Search for corresponding object */
    return xpSaiGetCtxDb(xpSaiScopeFromDevGet(xpsDevId), gsaiNeighborFdbDbHdle,
                         (void*) &key, (void**) entry);
}

/**
 * \brief Removes Neighbor FDB entry from state DB by Bridge/VLAN OID and MAC address
 * \param [in] xpsDevId    device identifier
 * \param [in] bv_id       Bridge/VLAN OID
 * \param [in] mac         MAC address
 * \return sai_status_t
 */
static sai_status_t xpSaiRemoveNeighborFdbEntry(xpsDevice_t xpsDevId,
                                                sai_object_id_t bv_id,
                                                sai_mac_t mac)
{
    xpSaiNeighborFdbEntry_t key;

    memset(&key, 0, sizeof(key));
    key.bv_id = bv_id;
    memcpy(key.mac, mac, XP_MAC_ADDR_LEN);

    /* Remove the corresponding state */
    return xpSaiRemoveCtxDb(xpSaiScopeFromDevGet(xpsDevId), gsaiNeighborFdbDbHdle,
                            (void*) &key);
}

/**
 * \brief Replaces Neighbor FDB entry pointed by Bridge/VLAN OID and MAC address
 *        with the new one in state DB.
 * \param [in] xpsDevId     device identifier
 * \param [in] bv_id        Bridge/VLAN OID
 * \param [in] mac          MAC address
 * \param [in] newFdbEntry  pointer to the new Neighbor FDB entry
 * \return sai_status_t
 */
static sai_status_t xpSaiNeighborFdbReplaceEntry(xpsDevice_t xpsDevId,
                                                 sai_object_id_t bv_id,
                                                 sai_mac_t mac,
                                                 xpSaiNeighborFdbEntry_t *newFdbEntry)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    XP_STATUS xpRetVal = XP_NO_ERR;

    if (newFdbEntry == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument (newFdbEntry: NULL)\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    /* Remove the corresponding state */
    retVal = xpSaiRemoveNeighborFdbEntry(xpsDevId, bv_id, mac);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to remove FDB Neighbour entry with MAC:"
                       XP_SAI_MAC_ADDR_FMT " and bv_id: 0x%" PRIx64 " (retVal: %d)\n",
                       XP_SAI_MAC_ADDR_ARGS(mac), bv_id, retVal);
        return retVal;
    }

    /* Insert the new state */
    xpRetVal = xpsStateInsertData(xpSaiScopeFromDevGet(xpsDevId),
                                  gsaiNeighborFdbDbHdle, (void*) newFdbEntry);
    if (xpRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to insert FDB Neighbor entry with MAC:"
                       XP_SAI_MAC_ADDR_FMT " and bv_id: 0x%" PRIx64 " (retVal: %d)\n",
                       XP_SAI_MAC_ADDR_ARGS(newFdbEntry->mac), newFdbEntry->bv_id, retVal);
        return xpsStatus2SaiStatus(xpRetVal);
    }

    return retVal;
}

/**
 * \brief Adds a pointer to Neighbor entry to an Neighbor FDB entry pointed by Bridge/VLAN OID and MAC address
 * \param [in] xpsDevId     device identifier
 * \param [in] bv_id        Bridge/VLAN OID
 * \param [in] mac          MAC address
 * \param [in] neighbor     pointer to the Neighbor entry to be added
 * \return sai_status_t
 *
 * Searches for Neighbor FDB entry with these Bridge/VLAN OID and MAC and adds
 * a pointer to Neighbor entry there. If there is no Neighbor FDB created
 * for such Bridge/VLAN - MAC pair yet it will create such and add it to the DB
 * and then will add the pointer to Neighbor to this Neighbor FDB entry just created.
 * If there is no enough buffer for the new Neighbor pointer in the Neighbor FDB entry
 * found the function will create a new Neighbor FDB more memory allocated and replace the
 * old entry with this new bigger one.
 */
static sai_status_t xpSaiNeighborFdbAddEntry(xpsDevice_t xpsDevId,
                                             sai_object_id_t bv_id,
                                             sai_mac_t mac,
                                             xpSaiNeighborEntry_t* neighbor)
{
    sai_status_t             retVal   = SAI_STATUS_SUCCESS;
    xpSaiNeighborFdbEntry_t *entry    = NULL;
    xpSaiNeighborFdbEntry_t *newEntry = NULL;

    if (neighbor == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument (neighbor: NULL)\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpSaiGetNeighborFdbEntry(xpsDevId, bv_id, mac, &entry);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        xpSaiNeighborFdbEntry_t key;

        memset(&key, 0, sizeof(key));

        key.bv_id = bv_id;
        memcpy(key.mac, mac, XP_MAC_ADDR_LEN);

        /* No entry found for these MAC and VLAN. Need to create one.*/
        retVal = xpSaiInsertCtxDb(xpsDevId, gsaiNeighborFdbDbHdle, (void*)&key,
                                  sizeof(key), (void**)&entry);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to insert FDB Neighbor entry with MAC:"
                           XP_SAI_MAC_ADDR_FMT " and bv_id: 0x%" PRIx64 " (retVal: %d)\n",
                           XP_SAI_MAC_ADDR_ARGS(mac), bv_id, retVal);
            return retVal;
        }
    }

    /* Verify whether the neigbor entry is already present in the list. */
    for (uint32_t i = 0; i < entry->numItems; i++)
    {
        if (entry->neighborsList[i] == neighbor)
        {
            if (neighbor->type == SAI_IP_ADDR_FAMILY_IPV4)
            {
                XP_SAI_LOG_ERR("Neighbor: " XP_SAI_IP_ADDR_FMT
                               " is already present in the neighbors list of FDB Neighbor entry with MAC:"
                               XP_SAI_MAC_ADDR_FMT " and bv_id: 0x%" PRIx64 " (retVal: %d)\n",
                               XP_SAI_XP_IP_ADDR_ARGS(neighbor->ipv4Addr), XP_SAI_MAC_ADDR_ARGS(entry->mac),
                               entry->bv_id, retVal);
            }
            else
            {
                XP_SAI_LOG_ERR("Neighbor:" XP_SAI_IPv6_ADDR_FMT
                               " is already present in the neighbors list of FDB Neighbor entry with MAC:"
                               XP_SAI_MAC_ADDR_FMT " and bv_id: 0x%" PRIx64 " (retVal: %d)\n",
                               XP_SAI_XP_IPv6_ADDR_ARGS(neighbor->ipv6Addr), XP_SAI_MAC_ADDR_ARGS(entry->mac),
                               entry->bv_id, retVal);
            }
            return SAI_STATUS_ITEM_ALREADY_EXISTS;
        }
    }

    if (xpSaiCtxGrowthNeeded(entry->numItems,
                             XP_SAI_NEIGHBOR_FDB_DEFAULT_MEMBER_COUNT))
    {
        /* Extend the array size */
        XP_STATUS xpRetVal = xpSaiDynamicArrayGrow((void**)&newEntry, (void*)entry,
                                                   sizeof(xpSaiNeighborFdbEntry_t), sizeof(xpSaiNeighborEntry_t *),
                                                   entry->numItems, XP_SAI_NEIGHBOR_FDB_DEFAULT_MEMBER_COUNT);
        if (xpRetVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed extend dynamic array for FDB Neighbor entry with MAC:"
                           XP_SAI_MAC_ADDR_FMT " and bv_id: 0x%" PRIx64 " (retVal: %d)\n",
                           XP_SAI_MAC_ADDR_ARGS(entry->mac), entry->bv_id, xpRetVal);
            return xpsStatus2SaiStatus(xpRetVal);
        }

        /* Replace the corresponding state */
        retVal = xpSaiNeighborFdbReplaceEntry(xpsDevId, entry->bv_id, entry->mac,
                                              newEntry);
        if (retVal != SAI_STATUS_SUCCESS)
        {
            xpsStateHeapFree((void*)newEntry);
            XP_SAI_LOG_ERR("Failed to replace FDB entry (retVal: %d)\n", retVal);
            return retVal;
        }

        entry = newEntry;
    }

    entry->neighborsList[entry->numItems] = neighbor;
    entry->numItems++;

    return retVal;
}

/**
 * \brief Removes a pointer to Neighbor entry from an Neighbor FDB entry pointed by Bridge/VLAN OID and MAC address
 * \param [in] xpsDevId     device identifier
 * \param [in] bv_id        Bridge/VLAN OID
 * \param [in] mac          MAC address
 * \param [in] neighbor     pointer to the Neighbor entry to be added
 * \return sai_status_t
 *
 * Searches for Neighbor FDB entry with these Bridge/VLAN OID and MAC and removes
 * a pointer to Neighbor entry from there. If the pointer to the neighbor to be removed
 * is the last one in Neighbor FDB entry the Neighbor FDB entry is removed from DB.
 * If there is no too much buffer in the Neighbor FDB entry after Neighbor
 * pointer removal the function will create a new Neighbor FDB entry with less
 * memory allocated and replace the old entry with this new smaller one.
 */
static sai_status_t xpSaiNeighborFdbRemoveEntry(xpsDevice_t xpsDevId,
                                                sai_object_id_t bv_id,
                                                sai_mac_t mac,
                                                xpSaiNeighborEntry_t* neighbor)
{
    sai_status_t            retVal    = SAI_STATUS_SUCCESS;
    xpSaiNeighborFdbEntry_t *entry    = NULL;
    xpSaiNeighborFdbEntry_t *newEntry = NULL;
    bool found = false;

    if (neighbor == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument (neighbor: NULL)\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpSaiGetNeighborFdbEntry(xpsDevId, bv_id, mac, &entry);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to find FDB Neighbor entry with MAC:" XP_SAI_MAC_ADDR_FMT
                       " and bv_id: 0x%" PRIx64 " (retVal: %d)\n",
                       XP_SAI_MAC_ADDR_ARGS(mac), bv_id, retVal);
        return retVal;
    }

    /* Search for neighbor and remove it from the list if found. */
    for (uint32_t i = 0; i < entry->numItems; i++)
    {
        if (!found)
        {
            found = (entry->neighborsList[i] == neighbor) ? true : false;
            continue;
        }
        else
        {
            /* Neighbor has been found - remove it by shifting all other neighbors following this one. */
            entry->neighborsList[i - 1] = entry->neighborsList[i];
        }
    }

    if (!found)
    {
        if (neighbor->type == SAI_IP_ADDR_FAMILY_IPV4)
        {
            XP_SAI_LOG_ERR("No Neighbor: " XP_SAI_IP_ADDR_FMT
                           " found in the neighbors list of FDB Neighbor entry with MAC:"
                           XP_SAI_MAC_ADDR_FMT " and bv_id: 0x%" PRIx64 " (retVal: %d)\n",
                           XP_SAI_XP_IP_ADDR_ARGS(neighbor->ipv4Addr), XP_SAI_MAC_ADDR_ARGS(entry->mac),
                           entry->bv_id, retVal);
        }
        else
        {
            XP_SAI_LOG_ERR("No Neighbor:" XP_SAI_IPv6_ADDR_FMT
                           " found in the neighbors list of FDB Neighbor entry with MAC:"
                           XP_SAI_MAC_ADDR_FMT " and bv_id: 0x%" PRIx64 " (retVal: %d)\n",
                           XP_SAI_XP_IPv6_ADDR_ARGS(neighbor->ipv6Addr), XP_SAI_MAC_ADDR_ARGS(entry->mac),
                           entry->bv_id, retVal);
        }
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    /* Decrement the count of neighbors in the list. */
    entry->numItems--;

    if (!entry->numItems)
    {
        /* No more Neighbor entries stored. Remove the Neighbor FDB entry. */
        retVal = xpSaiRemoveNeighborFdbEntry(xpsDevId, bv_id, mac);
        if (retVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to remove FDB Neighbour entry with MAC:"
                           XP_SAI_MAC_ADDR_FMT " and bv_id: 0x%" PRIx64 " (retVal: %d)\n",
                           XP_SAI_MAC_ADDR_ARGS(mac), bv_id, retVal);
            return retVal;
        }

        XP_SAI_LOG_DBG("Removed FDB Neighbour entry with MAC:" XP_SAI_MAC_ADDR_FMT
                       " and bv_id: 0x%" PRIx64 " (retVal: %d)\n",
                       XP_SAI_MAC_ADDR_ARGS(mac), bv_id, retVal);

        return SAI_STATUS_SUCCESS;
    }

    if (xpSaiCtxShrinkNeeded(entry->numItems,
                             XP_SAI_NEIGHBOR_FDB_DEFAULT_MEMBER_COUNT))
    {
        /* Shrink the array size */
        XP_STATUS xpRetVal = xpSaiDynamicArrayShrink((void**)&newEntry, (void*)entry,
                                                     sizeof(xpSaiNeighborFdbEntry_t), sizeof(xpSaiNeighborEntry_t *),
                                                     entry->numItems, XP_SAI_NEIGHBOR_FDB_DEFAULT_MEMBER_COUNT);
        if (xpRetVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed extend dynamic array for FDB Neighbor entry with MAC:"
                           XP_SAI_MAC_ADDR_FMT " and bv_id: 0x%" PRIx64 " (retVal: %d)\n",
                           XP_SAI_MAC_ADDR_ARGS(entry->mac), entry->bv_id, xpRetVal);
            return xpsStatus2SaiStatus(xpRetVal);
        }

        /* Replace the corresponding state */
        retVal = xpSaiNeighborFdbReplaceEntry(xpsDevId, entry->bv_id, entry->mac,
                                              newEntry);
        if (retVal != SAI_STATUS_SUCCESS)
        {
            xpsStateHeapFree((void*)newEntry);
            XP_SAI_LOG_ERR("Failed to relace FDB Neighbor entry (retVal: %d)\n", retVal);
            return retVal;
        }
    }

    return SAI_STATUS_SUCCESS;
}

// Func: saiConvertNeighborEntryToXps

static sai_status_t saiConvertNeighborEntryToXps(const sai_neighbor_entry_t*
                                                 neighbor_entry,
                                                 xpsL3HostEntry_t *saiNeighborEntry)
{
    uint32_t ipV4Addr = 0;
    memset(saiNeighborEntry, 0, sizeof(xpsL3HostEntry_t));

    if (SAI_IP_ADDR_FAMILY_IPV4 == neighbor_entry->ip_address.addr_family)
    {
        saiNeighborEntry->type = XP_PREFIX_TYPE_IPV4;

        ipV4Addr = neighbor_entry->ip_address.addr.ip4;
        memcpy(saiNeighborEntry->ipv4Addr, &ipV4Addr,
               sizeof(saiNeighborEntry->ipv4Addr));
    }
    else if (SAI_IP_ADDR_FAMILY_IPV6 == neighbor_entry->ip_address.addr_family)
    {
        saiNeighborEntry->type = XP_PREFIX_TYPE_IPV6;
        memcpy(saiNeighborEntry->ipv6Addr, &neighbor_entry->ip_address.addr.ip6,
               sizeof(ipv6Addr_t));
    }
    else
    {
        XP_SAI_LOG_ERR("Invalid addr family %d\n",
                       neighbor_entry->ip_address.addr_family);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    return SAI_STATUS_SUCCESS;
}

// Func: Get the XPS L3 host entry using the neighbor DB entry

sai_status_t xpSaiNeighborGetXpsHostEntry(const xpSaiNeighborEntry_t *nbrEntry,
                                          xpsL3HostEntry_t *xpsHostEntry)
{
    xpsInterfaceId_t l3InterfaceId;
    uint32_t         vrfId;
    XP_STATUS        xpsRetVal;
    sai_status_t     saiRetVal = SAI_STATUS_SUCCESS;
    xpsDevice_t      xpsDevId;

    if ((xpsHostEntry == NULL) || (nbrEntry == NULL))
    {
        XP_SAI_LOG_ERR("Invalid arguments.\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsDevId = xpSaiObjIdSwitchGet(nbrEntry->rifId);

    saiRetVal = xpSaiGetRouterInterfaceL3IntfId(nbrEntry->rifId, &l3InterfaceId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiGetRouterInterfaceL3IntfId() failed with saiRetVal : %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    xpsRetVal = xpSaiVrfIntfVrfIdGet(l3InterfaceId, &vrfId);
    if (xpsRetVal)
    {
        XP_SAI_LOG_ERR("Error in xpSaiVrfIntfVrfIdGet: error code: %d\n", xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    // Populate the key.
    xpsHostEntry->vrfId = vrfId;
    if (nbrEntry->type == SAI_IP_ADDR_FAMILY_IPV4)
    {
        xpsHostEntry->type = XP_PREFIX_TYPE_IPV4;
        memcpy(xpsHostEntry->ipv4Addr, nbrEntry->ipv4Addr,
               sizeof(xpsHostEntry->ipv4Addr));
    }
    else
    {
        xpsHostEntry->type = XP_PREFIX_TYPE_IPV6;
        memcpy(xpsHostEntry->ipv6Addr, nbrEntry->ipv6Addr,
               sizeof(xpsHostEntry->ipv6Addr));
    }

    // Get XPS host entry.
    XP_STATUS xpsStatus = xpsL3GetIpHostEntry(xpsDevId, xpsHostEntry);
    if (xpsStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsL3GetIpHostEntry: Failed for devId=%i with status=%i.\n",
                       xpsDevId, xpsStatus);
        return xpsStatus2SaiStatus(xpsStatus);
    }

    return SAI_STATUS_SUCCESS;
}

/**
 * Update the XPS L3 host entry
 */
static sai_status_t xpSaiNeighborUpdateXpsHostEntry(xpsL3HostEntry_t
                                                    *xpsHostEntry)
{
    xpsDevice_t xpsDevId = xpSaiGetDevId();
    XP_STATUS xpsStatus;

    xpsStatus = xpsL3UpdateIpHostEntry(xpsDevId, xpsHostEntry);
    if (xpsStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiNeighborUpdateXpsHostEntry: Failed to update XPS IP host entry\n");
        return xpsStatus2SaiStatus(xpsStatus);
    }

    return SAI_STATUS_SUCCESS;
}

// Func: Notify a neighbor change to all the interested modules

static sai_status_t xpSaiNeighborNotify(const xpSaiNeighborChangeNotif_t *notif)
{
    // Notify SAI Next Hop.
    return xpSaiNextHopHandleNeighborChangeNotification(notif);
}


static sai_status_t xpSaiNeighborFlushStateDb(void)
{
    XP_STATUS status = XP_NO_ERR;
    xpSaiNeighborEntry_t *curDbEntry = NULL;
    xpSaiNeighborEntry_t *nextDbEntry = NULL;
    xpSaiNeighborEntry_t keyEntry;
    xpSaiNeighborFdbEntry_t *curFdbDbEntry = NULL;
    xpSaiNeighborFdbEntry_t *nextFdbDbEntry = NULL;
    xpSaiNeighborFdbEntry_t keyFdbEntry;


    /* Flush Neighbor state DB data. */
    memset(&keyEntry, 0, sizeof(keyEntry));

    status = xpsStateGetNextData(XP_SCOPE_DEFAULT, gsaiNeighborDbHdle,
                                 (xpsDbKey_t)NULL, (void **)&curDbEntry);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get first entry from DB\n");
        return xpsStatus2SaiStatus(status);
    }

    while (curDbEntry != NULL)
    {
        /* Get the next entry beforehead as the current entry shall be removed from state DB */
        nextDbEntry = NULL;
        status = xpsStateGetNextData(XP_SCOPE_DEFAULT, gsaiNeighborDbHdle,
                                     (xpsDbKey_t)curDbEntry, (void **)&nextDbEntry);
        if (status != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to get first entry from DB\n");
            return xpsStatus2SaiStatus(status);
        }

        status = xpsStateDeleteData(XP_SCOPE_DEFAULT, gsaiNeighborDbHdle,
                                    (xpsDbKey_t)curDbEntry, (void **)&curDbEntry);
        if (status != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to delete state entry from DB\n");
            return status;
        }

        status = xpsStateHeapFree(curDbEntry);
        if (status != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to free memory from heap\n");
            return status;
        }

        /* Move to next */
        curDbEntry = nextDbEntry;
    }

    /* Flush Neighbor FDB state DB data. */
    memset(&keyFdbEntry, 0, sizeof(keyFdbEntry));

    status = xpsStateGetNextData(XP_SCOPE_DEFAULT, gsaiNeighborFdbDbHdle,
                                 (xpsDbKey_t)NULL, (void **)&curFdbDbEntry);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get first entry from Neighbor FDB DB\n");
        return xpsStatus2SaiStatus(status);
    }

    while (curFdbDbEntry != NULL)
    {
        /* Get the next entry beforehand as the current entry shall be removed from state DB */
        nextFdbDbEntry = NULL;
        status = xpsStateGetNextData(XP_SCOPE_DEFAULT, gsaiNeighborFdbDbHdle,
                                     (xpsDbKey_t)curFdbDbEntry, (void **)&nextFdbDbEntry);
        if (status != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to get first entry from Neighbor FDB DB\n");
            return xpsStatus2SaiStatus(status);
        }

        status = xpsStateDeleteData(XP_SCOPE_DEFAULT, gsaiNeighborFdbDbHdle,
                                    (xpsDbKey_t)curFdbDbEntry, (void **)&curFdbDbEntry);
        if (status != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to delete state entry from Neighbor FDB DB\n");
            return status;
        }

        status = xpsStateHeapFree(curFdbDbEntry);
        if (status != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to free memory from heap\n");
            return status;
        }

        /* Move to next */
        curFdbDbEntry = nextFdbDbEntry;
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t xpSaiNotifyDeleteForAllNeighbors(void)
{
    XP_STATUS                   status      = XP_NO_ERR;
    sai_status_t                saiStatus;
    xpSaiNeighborEntry_t       *curDbEntry  = NULL;
    xpSaiNeighborChangeNotif_t  notif;
    sai_neighbor_entry_t        neighborKey;

    memset(&notif, 0, sizeof(notif));
    memset(&neighborKey, 0, sizeof(neighborKey));

    status = xpsStateGetNextData(XP_SCOPE_DEFAULT, gsaiNeighborDbHdle,
                                 (xpsDbKey_t)NULL, (void **)&curDbEntry);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get first entry from DB\n");
        return xpsStatus2SaiStatus(status);
    }

    while (curDbEntry != NULL)
    {
        neighborKey.rif_id = curDbEntry->rifId;
        neighborKey.ip_address.addr_family = curDbEntry->type;
        if (curDbEntry->type == SAI_IP_ADDR_FAMILY_IPV4)
        {
            memcpy(&neighborKey.ip_address.addr.ip4, curDbEntry->ipv4Addr,
                   sizeof(curDbEntry->ipv4Addr));
        }
        else // IPv6 case
        {
            memcpy(&neighborKey.ip_address.addr.ip6, curDbEntry->ipv6Addr,
                   sizeof(curDbEntry->ipv6Addr));
        }

        /* Notify the delete operation */
        notif.notifType = XP_SAI_REMOVED;
        notif.neighborKey = &neighborKey;
        saiStatus = xpSaiNeighborNotify(&notif);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to notify neighbor delete\n");
            /* Continue to next neighbor */
        }

        status = xpsStateGetNextData(XP_SCOPE_DEFAULT, gsaiNeighborDbHdle,
                                     (xpsDbKey_t)curDbEntry, (void **)&curDbEntry);
        if (status != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to get first entry from DB\n");
            return xpsStatus2SaiStatus(status);
        }
    }

    return SAI_STATUS_SUCCESS;
}

/*
 * Resolves final packet command using neighbor pktCmd setting and fdb pktCmd
 */
static xpsPktCmd_e xpSaiNeighborResolvePktCmd(xpsPktCmd_e neighPktCmd,
                                              xpsPktCmd_e fdbPktCmd)
{
    /*
     * In SAI pipeline, FDB comes after neighbor table. Following is implemented:
     *
     *      Neighbor        FDB         Final pktCmd
     *      -----------------------------------------
     *      FWD             FWD         FWD
     *      FWD             DROP        DROP
     *      FWD             TRAP        TRAP
     *      FWD             FWD_MIR     FWD_MIR
     *      TRAP            FWD         TRAP
     *      TRAP            TRAP        TRAP
     *      TRAP            DROP        TRAP
     *      TRAP            FWD_MIR     TRAP
     *      DROP            FWD         DROP
     *      DROP            DROP        DROP
     *      DROP            TRAP        DROP
     *      DROP            FWD_MIR     DROP
     *      FWD_MIR         FWD         FWD_MIR
     *      FWD_MIR         DROP        DROP
     *      FWD_MIR         TRAP        TRAP
     *      FWD_MIR         FWD_MIR     FWD_MIR
     */

    /* Generalizing above decisions: */
    if (neighPktCmd == XP_PKTCMD_FWD)
    {
        return fdbPktCmd;
    }

    if (neighPktCmd == XP_PKTCMD_TRAP || neighPktCmd == XP_PKTCMD_DROP)
    {
        return neighPktCmd;
    }

    if (neighPktCmd == XP_PKTCMD_FWD_MIRROR)
    {
        if (fdbPktCmd == XP_PKTCMD_FWD)
        {
            return XP_PKTCMD_FWD_MIRROR;
        }
        else
        {
            return fdbPktCmd;
        }
    }

    /* Shouldnt reach here */
    return XP_PKTCMD_FWD;
}

//Func: xpSaiSetDefaultNeighborAttributeVals

void xpSaiSetDefaultNeighborAttributeVals(xpSaiNeighborAttributesT* attributes)
{
    XP_SAI_LOG_DBG("Calling xpSaiSetDefaultNeighborAttributeVals\n");
    attributes->packetAction.s32 = SAI_PACKET_ACTION_FORWARD;
    attributes->noHostRoute.booldata = FALSE;
}

//Func: xpSaiUpdateNeighborAttributeVals

sai_status_t xpSaiUpdateNeighborAttributeVals(const uint32_t attr_count,
                                              const sai_attribute_t* attr_list, xpSaiNeighborAttributesT* attributes)
{
    XP_SAI_LOG_DBG("Calling xpSaiUpdateNeighborAttributeVals\n");

    for (uint32_t count = 0; count < attr_count; count++)
    {
        switch (attr_list[count].id)
        {
            case SAI_NEIGHBOR_ENTRY_ATTR_DST_MAC_ADDRESS:
                {
                    memcpy(&(attributes->dstMacAddress.mac), attr_list[count].value.mac,
                           sizeof(macAddr_t));
                    break;
                }
            case SAI_NEIGHBOR_ENTRY_ATTR_PACKET_ACTION:
                {
                    attributes->packetAction.s32 = (uint8_t)(attr_list[count].value.s32);
                    break;
                }
            case SAI_NEIGHBOR_ENTRY_ATTR_NO_HOST_ROUTE:
                {
                    attributes->noHostRoute.booldata = (bool)(attr_list[count].value.booldata);
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Failed to set %d\n", attr_list[count].id);
                    return SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
                }
        }
    }
    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiUpdateAttrListNeighborVals

void xpSaiUpdateAttrListNeighborVals(xpSaiNeighborAttributesT attributes,
                                     const uint32_t attr_count, sai_attribute_t* attr_list)
{
    XP_SAI_LOG_DBG("Calling xpSaiUpdateAttrListNeighborVals\n");
    for (uint32_t count = 0; count < attr_count; count++)
    {
        switch (attr_list[count].id)
        {
            case SAI_NEIGHBOR_ENTRY_ATTR_DST_MAC_ADDRESS:
                {
                    attr_list[count].value = attributes.dstMacAddress;
                    break;
                }
            case SAI_NEIGHBOR_ENTRY_ATTR_PACKET_ACTION:
                {
                    attr_list[count].value = attributes.packetAction;
                    break;
                }
            case SAI_NEIGHBOR_ENTRY_ATTR_NO_HOST_ROUTE:
                {
                    attr_list[count].value = attributes.noHostRoute;
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Failed to set %d\n", attr_list[count].id);
                }
        }

    }

}



//Func: xpSaiSetNeighborAttrDstMacAddress

sai_status_t xpSaiSetNeighborAttrDstMacAddress(const sai_neighbor_entry_t
                                               *neighbor_entry,  sai_attribute_value_t value)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpsDevice_t xpsDevId = xpSaiGetDevId();
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    xpSaiNeighborEntry_t saiL3NeighbourEntry;
    xpSaiNeighborEntry_t saiL3NeighborKey;
    xpSaiNeighborEntry_t *saiL3FoundNeighborEntry = NULL;
    xpsInterfaceId_t l3InterfaceId = 0;
    xpsL3HostEntry_t xpsL3NeighborEntry;
    uint32_t  vrfId;
    xpsInterfaceType_e xpsIntfType;
    xpSaiNeighborChangeNotif_t notif;
    sai_fdb_entry_t saiFdbKey;
    xpsFdbEntry_t xpsFdbEntry;
    sai_attribute_value_t vlanIdValue;
    xpsInterfaceId_t initialEgressIntfId, finalEgressIntfId;
    xpsPktCmd_e initialPktCmd, finalPktCmd;
    xpSaiNeighborState_t finalState = XP_SAI_NEIGHBOR_FULLY_RESOLVED;
    xpSaiNeighborState_t initialState;
    sai_attribute_value_t fdbUniCastMissAction;
    uint8_t bCastMac[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

    xpSaiRouterInterfaceDbEntryT *pRifEntry = NULL;

    memset(&notif, 0, sizeof(notif));
    memset(&saiFdbKey, 0, sizeof(saiFdbKey));
    memset(&xpsFdbEntry, 0, sizeof(xpsFdbEntry));

    XP_SAI_LOG_DBG("Calling xpSaiSetNeighborAttrDstMacAddress\n");

    memset(&saiL3NeighbourEntry, 0, sizeof(xpSaiNeighborEntry_t));

    if (!XDK_SAI_OBJID_TYPE_CHECK(neighbor_entry->rif_id,
                                  SAI_OBJECT_TYPE_ROUTER_INTERFACE))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n",
                       xpSaiObjIdTypeGet(neighbor_entry->rif_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiGetSwitchAttrFdbUnicastMissAction(&fdbUniCastMissAction);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get (SAI_SWITCH_ATTR_FDB_UNICAST_MISS_PACKET_ACTION) | saiRetVal : %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    if ((fdbUniCastMissAction.s32 != SAI_PACKET_ACTION_FORWARD) &&
        (fdbUniCastMissAction.s32 != SAI_PACKET_ACTION_DROP))
    {
        XP_SAI_LOG_ERR(" (SAI_SWITCH_ATTR_FDB_UNICAST_MISS_PACKET_ACTION) Not Supported| Val : %d\n",
                       fdbUniCastMissAction.s32);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiRouterInterfaceDbInfoGet(neighbor_entry->rif_id, &pRifEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiRouterInterfaceDbInfoGet() failed with error code: %d!\n",
                       saiRetVal);
        return saiRetVal;
    }

    l3InterfaceId = pRifEntry->l3IntfId;

    saiRetVal = saiConvertNeighborEntryToXps(neighbor_entry, &xpsL3NeighborEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        return saiRetVal;
    }

    retVal = xpsL3GetIntfVrf(xpsDevId, l3InterfaceId, &vrfId);
    if (retVal)
    {
        XP_SAI_LOG_ERR("Error in xpsL3GetIntfVrf: error code: %d\n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    xpsL3NeighborEntry.vrfId = vrfId;

    if (neighbor_entry->ip_address.addr_family == SAI_IP_ADDR_FAMILY_IPV4)
    {
        saiL3NeighbourEntry.type = SAI_IP_ADDR_FAMILY_IPV4;
        memcpy(saiL3NeighbourEntry.ipv4Addr, xpsL3NeighborEntry.ipv4Addr,
               XP_IPV4_ADDR_LEN);
    }
    else
    {
        saiL3NeighbourEntry.type = SAI_IP_ADDR_FAMILY_IPV6;
        memcpy(saiL3NeighbourEntry.ipv6Addr, xpsL3NeighborEntry.ipv6Addr,
               XP_IPV6_ADDR_LEN);
    }

    saiL3NeighbourEntry.rifId =
        neighbor_entry->rif_id;   /* VRF and IP:: Keys to Get the sai-neighborEntry */

    saiRetVal = xpSaiNeighborEntryGet(&saiL3NeighbourEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not find SAI L3 neighbour entry\n");
        return saiRetVal;
    }

    /* Store the initial values for comparison later for notifications */
    finalPktCmd = initialPktCmd = saiL3NeighbourEntry.resolvedPktCmd;
    finalEgressIntfId = initialEgressIntfId = saiL3NeighbourEntry.xpsEgressIntf;
    initialState = saiL3NeighbourEntry.state;

    retVal = xpsInterfaceGetType(saiL3NeighbourEntry.l3IntfId, &xpsIntfType);
    if (retVal != XP_NO_ERR)
    {
        return xpsStatus2SaiStatus(retVal);
    }

    if (memcmp(saiL3NeighbourEntry.macDa, value.mac, XP_MAC_ADDR_LEN) == 0)
    {
        return SAI_STATUS_SUCCESS;
    }

    /*
     * As per SAI pipeline, FDB entry is looked up after neighbor table if it is
     * vlan router interface. look up the FDB table for the new dest mac address.
     * Update the neighbor parameters from the FDB entry.
     */
    if (xpsIntfType == XPS_VLAN_ROUTER)
    {
        if (pRifEntry->rifType == SAI_ROUTER_INTERFACE_TYPE_BRIDGE)
        {
            saiFdbKey.bv_id = pRifEntry->brOid;
        }
        else
        {
            saiRetVal = xpSaiGetRouterInterfaceAttrVlanId(neighbor_entry->rif_id,
                                                          &vlanIdValue);
            if (saiRetVal != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Error in xpSaiGetRouterInterfaceAttrVlanId: error code: %d\n",
                               saiRetVal);
                return saiRetVal;
            }

            saiFdbKey.bv_id = vlanIdValue.oid;
        }

        memcpy(&saiFdbKey.mac_address, &value.mac, sizeof(saiFdbKey.mac_address));

        saiRetVal = xpSaiFdbGetXpsEntry(&saiFdbKey, &xpsFdbEntry);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            /*
             * If the entry is not found yet, assume FDB pktCmd is XP_SAI_NEIGHBOR_FDB_MISS_PKTCMD and resolve
             */
            finalPktCmd = xpSaiNeighborResolvePktCmd(saiL3NeighbourEntry.userPktCmd,
                                                     XP_SAI_NEIGHBOR_FDB_MISS_PKTCMD);
            finalState = XP_SAI_NEIGHBOR_PHYS_INTF_UNRESOLVED;
            finalEgressIntfId = XPS_INTF_INVALID_ID;
        }
        else
        {
            /* FDB Entry found. Inherit the properties from it. */
            finalPktCmd = xpSaiNeighborResolvePktCmd(saiL3NeighbourEntry.userPktCmd,
                                                     xpsFdbEntry.pktCmd);
            finalEgressIntfId = xpsFdbEntry.intfId;
        }
    }

    /*
     * Update the HW if host entry is present
     */
    if (saiL3NeighbourEntry.noHostEntry == FALSE)
    {
        /*
         * If FDB is empty for this DMAC, l3IntfId and egressIntfId is set to
         * 0xFFFFFFFF if pkt action was DROP, else set to l3IntfId.
         */
        retVal = xpsL3GetIpHostEntry(xpsDevId, &xpsL3NeighborEntry);
        if (retVal)
        {
            XP_SAI_LOG_ERR("Error in xpsL3GetIpHostEntry: error code: %d\n", retVal);
            return xpsStatus2SaiStatus(retVal);
        }

        /*Update the current Rif Id*/
        xpsL3NeighborEntry.nhEntry.nextHop.l3InterfaceId = l3InterfaceId;

        xpSaiMacCopy(xpsL3NeighborEntry.nhEntry.nextHop.macDa, value.mac);
        xpsL3NeighborEntry.nhEntry.pktCmd = finalPktCmd;

        /*finalEgressIntfId is 0xFFFFFFFF when FDB is empty
          or set to valid value based on FDB get.*/
        xpsL3NeighborEntry.nhEntry.nextHop.egressIntfId = finalEgressIntfId;

        if ((xpsIntfType == XPS_VLAN_ROUTER) &&
            (xpsL3NeighborEntry.nhEntry.nextHop.egressIntfId == XPS_INTF_INVALID_ID) &&
            (finalState == XP_SAI_NEIGHBOR_PHYS_INTF_UNRESOLVED))
        {
            if (!(memcmp(&xpsL3NeighborEntry.nhEntry.nextHop.macDa, bCastMac,
                         XP_MAC_ADDR_LEN)) ||
                (fdbUniCastMissAction.s32 == SAI_PACKET_ACTION_FORWARD))
            {
                xpsL3NeighborEntry.nhEntry.pktCmd = XP_PKTCMD_FWD;
                xpsL3NeighborEntry.nhEntry.nextHop.egressIntfId = l3InterfaceId;
                finalEgressIntfId = l3InterfaceId;
                finalPktCmd = XP_PKTCMD_FWD;
            }
        }

        retVal = xpsL3UpdateIpHostEntry(xpsDevId, &xpsL3NeighborEntry);
        if (retVal)
        {
            XP_SAI_LOG_ERR("Error in xpsL3UpdateIpHostEntry: error code: %d\n", retVal);
            return xpsStatus2SaiStatus(retVal);
        }
    }

    /*
     * Notify the change to the interested modules
     */
    notif.neighborKey = neighbor_entry;
    notif.notifType = XP_SAI_PROPERTY_CHANGED;

    memcpy(notif.props.macDa, value.mac, sizeof(notif.props.macDa));
    notif.props.macDaValid = true;

    if (initialPktCmd != finalPktCmd)
    {
        notif.props.pktCmd = finalPktCmd;
        notif.props.pktCmdValid = true;
    }

    if (initialEgressIntfId != finalEgressIntfId)
    {
        notif.props.egressIntf = finalEgressIntfId;
        notif.props.egrIntfValid = true;
    }

    if (initialState != finalState)
    {
        notif.props.state = finalState;
        notif.props.stateValid = true;
    }

    saiRetVal = xpSaiNeighborNotify(&notif);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to notify for neighbor change\n");
        return saiRetVal;
    }

    memcpy(&saiL3NeighborKey, &saiL3NeighbourEntry, sizeof(xpSaiNeighborEntry_t));
    saiRetVal = xpSaiGetCtxDb(xpSaiScopeFromDevGet(xpsDevId), gsaiNeighborDbHdle,
                              (void*) &saiL3NeighborKey, (void**) &saiL3FoundNeighborEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not find SAI L3 neighbour entry in DB, error code: %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    memcpy(saiL3FoundNeighborEntry->macDa, value.mac, sizeof(macAddr_t));
    saiL3FoundNeighborEntry->state = finalState;
    saiL3FoundNeighborEntry->resolvedPktCmd = finalPktCmd;
    saiL3FoundNeighborEntry->xpsEgressIntf = finalEgressIntfId;

    if ((xpsIntfType == XPS_VLAN_ROUTER) &&
        (saiL3NeighbourEntry.noHostEntry == FALSE))
    {
        /* MAC changed so need to remove the old FDB Neighbor entry and add the new one */
        saiRetVal = xpSaiNeighborFdbRemoveEntry(xpsDevId, saiFdbKey.bv_id,
                                                saiL3NeighborKey.macDa, saiL3FoundNeighborEntry);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to remove neighbor from FDB neighbor entry, retVal: %d\n",
                           saiRetVal);
            return saiRetVal;
        }

        saiRetVal = xpSaiNeighborFdbAddEntry(xpsDevId, saiFdbKey.bv_id,
                                             saiL3FoundNeighborEntry->macDa, saiL3FoundNeighborEntry);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to add neighbor to FDB neighbor entry, retVal: %d\n",
                           saiRetVal);
            return saiRetVal;
        }
    }

    return retVal;
}


//Func: xpSaiSetNeighborAttrPacketAction
sai_status_t xpSaiSetNeighborAttrPacketAction(const sai_neighbor_entry_t
                                              *neighbor_entry,  sai_attribute_value_t value)
{
    XP_STATUS retVal = XP_NO_ERR;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    xpsDevice_t xpsDevId = xpSaiGetDevId();
    xpsInterfaceId_t l3InterfaceId = 0;
    xpsL3HostEntry_t xpsL3NeighborEntry;
    xpSaiNeighborEntry_t saiL3NeighbourEntry;
    xpsFdbEntry_t getFdbEntry;
    uint32_t  vrfId;
    xpsPktCmd_e inputPktCmd, initialPktCmd, finalPktCmd;
    xpsInterfaceType_e intfType;
    sai_attribute_value_t vlanId;
    xpSaiNeighborChangeNotif_t notif;

    xpSaiRouterInterfaceDbEntryT *pRifEntry = NULL;

    memset(&notif, 0, sizeof(notif));

    XP_SAI_LOG_DBG("Calling xpSaiSetNeighborAttrPacketAction\n");

    memset(&saiL3NeighbourEntry, 0, sizeof(xpSaiNeighborEntry_t));

    if (!XDK_SAI_OBJID_TYPE_CHECK(neighbor_entry->rif_id,
                                  SAI_OBJECT_TYPE_ROUTER_INTERFACE))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n",
                       xpSaiObjIdTypeGet(neighbor_entry->rif_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiRouterInterfaceDbInfoGet(neighbor_entry->rif_id, &pRifEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiRouterInterfaceDbInfoGet() failed with error code: %d!\n",
                       saiRetVal);
        return saiRetVal;
    }

    l3InterfaceId = pRifEntry->l3IntfId;

    saiRetVal = saiConvertNeighborEntryToXps(neighbor_entry, &xpsL3NeighborEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        return saiRetVal;
    }

    retVal = xpsL3GetIntfVrf(xpsDevId, l3InterfaceId, &vrfId);
    if (retVal)
    {
        XP_SAI_LOG_ERR("Error in xpsL3GetIntfVrf: error code: %d\n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    xpsL3NeighborEntry.vrfId = vrfId;

    saiL3NeighbourEntry.rifId =
        neighbor_entry->rif_id;   /*VRF and IP:: Keys to Get the sai-neighborEntry*/

    if (neighbor_entry->ip_address.addr_family == SAI_IP_ADDR_FAMILY_IPV4)
    {
        saiL3NeighbourEntry.type = SAI_IP_ADDR_FAMILY_IPV4;
        memcpy(saiL3NeighbourEntry.ipv4Addr, xpsL3NeighborEntry.ipv4Addr,
               XP_IPV4_ADDR_LEN);
    }
    else
    {
        saiL3NeighbourEntry.type = SAI_IP_ADDR_FAMILY_IPV6;
        memcpy(saiL3NeighbourEntry.ipv6Addr, xpsL3NeighborEntry.ipv6Addr,
               XP_IPV6_ADDR_LEN);
    }

    saiRetVal = xpSaiNeighborEntryGet(&saiL3NeighbourEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not find SAI L3 neighbour entry\n");
        return saiRetVal;
    }

    if ((saiRetVal = xpSaiConvertSaiPacketAction2xps((sai_packet_action_t)value.s32,
                                                     &inputPktCmd)) != SAI_STATUS_SUCCESS)
    {
        return saiRetVal;
    }

    if (inputPktCmd == saiL3NeighbourEntry.userPktCmd)
    {
        return SAI_STATUS_SUCCESS;
    }

    /* Initialize final pktCmd from input pktCmd. */
    finalPktCmd = inputPktCmd;
    initialPktCmd = saiL3NeighbourEntry.resolvedPktCmd;

    retVal = (XP_STATUS) xpsInterfaceGetType(l3InterfaceId, &intfType);
    if (retVal)
    {
        XP_SAI_LOG_ERR("Error in xpsInterfaceGetType: error code: %d\n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    if (intfType == XPS_VLAN_ROUTER)
    {
        xpSaiMacCopy(getFdbEntry.macAddr, saiL3NeighbourEntry.macDa);

        if (pRifEntry->rifType == SAI_ROUTER_INTERFACE_TYPE_BRIDGE)
        {
            getFdbEntry.vlanId = (xpVlan_t)xpSaiObjIdValueGet(pRifEntry->brOid);
        }
        else
        {
            saiRetVal = xpSaiGetRouterInterfaceAttrVlanId(neighbor_entry->rif_id, &vlanId);
            if (saiRetVal)
            {
                XP_SAI_LOG_ERR("Error in xpSaiGetRouterInterfaceAttrVlanId: error code: %d\n",
                               saiRetVal);
                return saiRetVal;
            }
            getFdbEntry.vlanId = (xpVlan_t)xpSaiObjIdValueGet(vlanId.oid);
        }

        XP_SAI_LOG_DBG("MAC Addr " FMT_MAC " vlan %d\n", PRI_MAC(getFdbEntry.macAddr),
                       getFdbEntry.vlanId);

        if ((retVal = xpsFdbGetEntry(xpsDevId, &getFdbEntry)) != XP_NO_ERR)
        {
            /*
             * If the entry is not found yet, assume FDB pktCmd is XP_SAI_NEIGHBOR_FDB_MISS_PKTCMD and resolve
             */
            finalPktCmd = xpSaiNeighborResolvePktCmd(inputPktCmd,
                                                     XP_SAI_NEIGHBOR_FDB_MISS_PKTCMD);
        }

        else
        {
            finalPktCmd = xpSaiNeighborResolvePktCmd(inputPktCmd, getFdbEntry.pktCmd);
        }
        XP_SAI_LOG_DBG("fdbEntry.intId/portId %d \n", getFdbEntry.intfId);
    }

    if (saiL3NeighbourEntry.noHostEntry == FALSE)
    {
        retVal = xpsL3GetIpHostEntry(xpsDevId, &xpsL3NeighborEntry);
        if (retVal)
        {
            XP_SAI_LOG_ERR("Error in xpsL3GetIpHostEntry: error code: %d\n", retVal);
            return xpsStatus2SaiStatus(retVal);
        }

        xpsL3NeighborEntry.nhEntry.pktCmd = finalPktCmd;
        retVal = xpsL3UpdateIpHostEntry(xpsDevId, &xpsL3NeighborEntry);
        if (retVal)
        {
            XP_SAI_LOG_ERR("Error in xpsL3UpdateIpHostEntry: error code: %d\n", retVal);
            return xpsStatus2SaiStatus(retVal);
        }
    }

    /*
     * Notify the changes to the interested modules.
     */
    if (initialPktCmd != finalPktCmd)
    {
        notif.neighborKey = neighbor_entry;
        notif.notifType = XP_SAI_PROPERTY_CHANGED;

        notif.props.pktCmd = finalPktCmd;
        notif.props.pktCmdValid = true;

        saiRetVal = xpSaiNeighborNotify(&notif);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to notify for neighbor change on packet action\n");
            return saiRetVal;
        }
    }

    if ((saiRetVal = xpSaiConvertSaiPacketAction2xps((sai_packet_action_t)value.s32,
                                                     &inputPktCmd))!= SAI_STATUS_SUCCESS)
    {
        return saiRetVal;
    }
    saiL3NeighbourEntry.userPktCmd = inputPktCmd;
    saiL3NeighbourEntry.resolvedPktCmd = finalPktCmd;
    saiRetVal = xpSaiNeighborEntrySet(&saiL3NeighbourEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not update SAI L3 neighbour entry\n");
        return saiRetVal;
    }

    return saiRetVal;
}

static int32_t xpSaiNeighborEntryKeyComp(void* key1, void* key2)
{
    if (((xpSaiNeighborEntry_t*)key1)->rifId > ((xpSaiNeighborEntry_t*)key2)->rifId)
    {
        return 1;
    }
    else
    {
        if (((xpSaiNeighborEntry_t *)key1)->rifId < ((xpSaiNeighborEntry_t *)
                                                     key2)->rifId)
        {
            return -1;
        }
        else
        {
            if (((xpSaiNeighborEntry_t *)key1)->type > ((xpSaiNeighborEntry_t *)key2)->type)
            {
                return 1;
            }
            else
            {
                if (((xpSaiNeighborEntry_t *)key1)->type < ((xpSaiNeighborEntry_t *)key2)->type)
                {
                    return -1;
                }
                else
                {
                    if (((xpSaiNeighborEntry_t *)key1)->type == SAI_IP_ADDR_FAMILY_IPV4)
                    {
                        return COMPARE_IPV4_ADDR_T((((xpSaiNeighborEntry_t *)key1)->ipv4Addr),
                                                   (((xpSaiNeighborEntry_t *)key2)->ipv4Addr));
                    }
                    else
                    {
                        return COMPARE_IPV6_ADDR_T((((xpSaiNeighborEntry_t *)key1)->ipv6Addr),
                                                   (((xpSaiNeighborEntry_t *)key2)->ipv6Addr));
                    }
                }
            }
        }
    }
}

static int32_t xpSaiNeighborFdbEntryKeyComp(void* key1, void* key2)
{
    if (((xpSaiNeighborFdbEntry_t*)key1)->bv_id > ((xpSaiNeighborFdbEntry_t*)
                                                   key2)->bv_id)
    {
        return 1;
    }
    else if ((((xpSaiNeighborFdbEntry_t*)key1)->bv_id < ((xpSaiNeighborFdbEntry_t*)
                                                         key2)->bv_id))
    {
        return -1;
    }
    else
    {
        return COMPARE_MAC_ADDR_T(((xpSaiNeighborFdbEntry_t*)key1)->mac,
                                  ((xpSaiNeighborFdbEntry_t*)key2)->mac);
    }

}

XP_STATUS xpSaiNeighborInit(xpsDevice_t xpSaiDevId)
{
    XP_STATUS retVal = XP_NO_ERR;
    gsaiNeighborDbHdle = XPSAI_NEIGHBOUR_DB_HNDL;
    retVal = xpsStateRegisterDb(XP_SCOPE_DEFAULT, "Neighbor Db", XPS_GLOBAL,
                                &xpSaiNeighborEntryKeyComp, gsaiNeighborDbHdle);
    if (retVal != XP_NO_ERR)
    {
        gsaiNeighborDbHdle = XPS_STATE_INVALID_DB_HANDLE;
        XP_SAI_LOG_ERR("Failed to register SAI Neighbour DB\n");
        return retVal;
    }

    gsaiNeighborFdbDbHdle = XPSAI_NEIGHBOUR_FDB_DB_HNDL;
    retVal = xpsStateRegisterDb(XP_SCOPE_DEFAULT, "Neighbor FDB DB", XPS_GLOBAL,
                                &xpSaiNeighborFdbEntryKeyComp, gsaiNeighborFdbDbHdle);
    if (retVal != XP_NO_ERR)
    {
        gsaiNeighborFdbDbHdle = XPS_STATE_INVALID_DB_HANDLE;
        XP_SAI_LOG_ERR("Failed to register SAI Neighbour FDB DB\n");
        return retVal;
    }

    return retVal;
}


//Func: xpSaiCreateNeighborEntry

sai_status_t xpSaiHandleCreateNeighborEntry(const sai_neighbor_entry_t
                                            *neighbor_entry, uint32_t attr_count, const sai_attribute_t *attr_list)
{
    XP_STATUS retVal = XP_NO_ERR;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    xpsDevice_t xpsDevId = xpSaiGetDevId();
    xpSaiNeighborAttributesT attributes;
    xpsL3HostEntry_t xpsL3NeighborEntry;
    xpsFdbEntry_t getFdbEntry;
    xpsInterfaceType_e intfType;
    sai_attribute_value_t vlanId;
    sai_attribute_value_t  egressIntfId;
    xpsHashIndexList_t *indexList = NULL;
    uint32_t  vrfId;
    xpsInterfaceId_t l3InterfaceId = 0;
    xpSaiNeighborState_t neighState =
        XP_SAI_NEIGHBOR_FULLY_RESOLVED; /* Initial state */
    xpsPktCmd_e inputPktCmd, finalPktCmd;
    const sai_attribute_t *attr = NULL;
    int index;
    uint8_t nullMac[XP_MAC_ADDR_LEN];
    uint8_t bCastMac[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    xpSaiTableEntryCountDbEntry *entryCountCtxPtr = NULL;
    int newEntry = 0;

    sai_attribute_value_t fdbUniCastMissAction;

    memset(nullMac, 0, XP_MAC_ADDR_LEN*sizeof(uint8_t));

    indexList = (xpsHashIndexList_t *)xpMalloc(sizeof(xpsHashIndexList_t));
    if (!indexList)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for indexList\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }
    memset(indexList, 0, sizeof(xpsHashIndexList_t));

    //Global State maintainance
    xpSaiNeighborEntry_t saiL3NeighborKey;
    xpSaiNeighborEntry_t *saiL3NeighborEntry = NULL;

    xpSaiRouterInterfaceDbEntryT *pRifEntry = NULL;

    /* Notifcation for the interested */
    xpSaiNeighborChangeNotif_t notif;

    memset(&notif, 0, sizeof(notif));

    if (neighbor_entry == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument");
        xpFree(indexList);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiAttrCheck(attr_count, attr_list,
                               NEIGHBOR_ENTRY_VALIDATION_ARRAY_SIZE, neighbor_entry_attribs,
                               SAI_COMMON_API_CREATE);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiRetVal);
        xpFree(indexList);
        return saiRetVal;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(neighbor_entry->rif_id,
                                  SAI_OBJECT_TYPE_ROUTER_INTERFACE))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n",
                       xpSaiObjIdTypeGet(neighbor_entry->rif_id));
        xpFree(indexList);
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    if (xpSaiRouterInterfaceVirtualCheck(neighbor_entry->rif_id))
    {
        XP_SAI_LOG_ERR("Virtual RIF OID received\n");
        xpFree(indexList);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    /* Loopback processing */
    if (xpSaiRouterInterfaceLoopbackIdCheck(neighbor_entry->rif_id))
    {
        /* SAI requires router interface to be passed on neighbor entry creation.
         * However, this router interface might be loopback type and there are
         * no use-cases for loopback fastpath
         */
        XP_SAI_LOG_ERR("Loopback router interface received.\n"
                       "Please create IP2ME route using SAI Route API.\n");
        xpFree(indexList);
        return SAI_STATUS_FAILURE;
    }

    saiRetVal = xpSaiRouterInterfaceDbInfoGet(neighbor_entry->rif_id, &pRifEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiRouterInterfaceDbInfoGet() failed with error code: %d!\n",
                       saiRetVal);
        xpFree(indexList);
        return saiRetVal;
    }

    l3InterfaceId = pRifEntry->l3IntfId;

    xpSaiSetDefaultNeighborAttributeVals(&attributes);

    attr = xpSaiFindAttrById(SAI_NEIGHBOR_ENTRY_ATTR_DST_MAC_ADDRESS, attr_count,
                             attr_list, NULL);
    if (NULL == attr)
    {
        xpFree(indexList);
        return SAI_STATUS_FAILURE;
    }

    /* Get neighbor attributes */
    saiRetVal = xpSaiUpdateNeighborAttributeVals(attr_count, attr_list,
                                                 &attributes);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Invalid Attribute-Id, error %d\n", saiRetVal);
        xpFree(indexList);
        return saiRetVal;
    }

    saiRetVal = xpSaiGetSwitchAttrFdbUnicastMissAction(&fdbUniCastMissAction);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get (SAI_SWITCH_ATTR_FDB_UNICAST_MISS_PACKET_ACTION) | saiRetVal : %d\n",
                       saiRetVal);
        xpFree(indexList);
        return saiRetVal;
    }

    if ((fdbUniCastMissAction.s32 != SAI_PACKET_ACTION_FORWARD) &&
        (fdbUniCastMissAction.s32 != SAI_PACKET_ACTION_DROP))
    {
        XP_SAI_LOG_ERR(" (SAI_SWITCH_ATTR_FDB_UNICAST_MISS_PACKET_ACTION) Not Supported| Val : %d\n",
                       fdbUniCastMissAction.s32);
        xpFree(indexList);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (attributes.packetAction.s32 == SAI_PACKET_ACTION_FORWARD)
    {
        if (memcmp(attributes.dstMacAddress.mac, &nullMac, XP_MAC_ADDR_LEN) == 0)
        {
            xpFree(indexList);
            return SAI_STATUS_INVALID_ATTR_VALUE_0;
        }
    }

    saiRetVal = saiConvertNeighborEntryToXps(neighbor_entry, &xpsL3NeighborEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not convert a neighbor, error %d\n", saiRetVal);
        xpFree(indexList);
        return saiRetVal;
    }

    retVal = xpsL3GetIntfVrf(xpsDevId, l3InterfaceId, &vrfId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error in xpsL3GetIntfVrf: error code: %d\n", retVal);
        xpFree(indexList);
        return xpsStatus2SaiStatus(retVal);
    }

    xpsL3NeighborEntry.vrfId = vrfId;

    xpSaiMacCopy(xpsL3NeighborEntry.nhEntry.nextHop.macDa,
                 attributes.dstMacAddress.mac);

    saiRetVal = xpSaiConvertSaiPacketAction2xps((sai_packet_action_t)
                                                attributes.packetAction.s32, &inputPktCmd);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error in xpSaiConvertSaiPacketAction2xps: error code: %d\n",
                       retVal);
        xpFree(indexList);
        return saiRetVal;
    }
    /* Init final pkt cmd and input pktCmd */
    finalPktCmd = inputPktCmd;

    retVal = xpsInterfaceGetType(l3InterfaceId, &intfType);
    if (retVal)
    {
        XP_SAI_LOG_ERR("Error in xpsInterfaceGetType: error code: %d\n", retVal);
        xpFree(indexList);
        return xpsStatus2SaiStatus(retVal);
    }

    switch (intfType)
    {
        case XPS_PORT_ROUTER:  /* For Normal l3 interface over l2 */
            {
                saiRetVal = xpSaiGetRouterInterfaceAttrPortId(neighbor_entry->rif_id,
                                                              &egressIntfId);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Error in xpSaiGetRouterInterfaceAttrPortId: error code: %d\n",
                                   saiRetVal);
                    xpFree(indexList);
                    return saiRetVal;
                }
                xpsL3NeighborEntry.nhEntry.nextHop.egressIntfId = (xpsInterfaceId_t)
                                                                  xpSaiObjIdValueGet(egressIntfId.oid);
                break;
            }

        case XPS_VLAN_ROUTER: //For Interface vlan
            {
                memcpy(getFdbEntry.macAddr, xpsL3NeighborEntry.nhEntry.nextHop.macDa,
                       sizeof(macAddr_t));

                if (pRifEntry->rifType == SAI_ROUTER_INTERFACE_TYPE_BRIDGE)
                {
                    getFdbEntry.vlanId = (xpVlan_t)xpSaiObjIdValueGet(pRifEntry->brOid);
                }
                else
                {
                    saiRetVal = xpSaiGetRouterInterfaceAttrVlanId(neighbor_entry->rif_id, &vlanId);
                    if (saiRetVal != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Error in xpSaiGetRouterInterfaceAttrVlanId: error code: %d\n",
                                       saiRetVal);
                        xpFree(indexList);
                        return saiRetVal;
                    }
                    getFdbEntry.vlanId = (xpVlan_t)xpSaiObjIdValueGet(vlanId.oid);
                }

                XP_SAI_LOG_DBG("MAC Addr " FMT_MAC " vlan %d\n",
                               PRI_MAC(getFdbEntry.macAddr),
                               getFdbEntry.vlanId);

                retVal = xpsFdbGetEntry(xpsDevId, &getFdbEntry);
                if (retVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_NOTICE("fdb get failed for mac" FMT_MAC " vlan %d\n",
                                      PRI_MAC(getFdbEntry.macAddr),
                                      getFdbEntry.vlanId);
                    /*
                     * In SAI pipeline, if FDB entry is not available after vlan router
                     * interface, packet is supposed to be flood into the vlan members.
                     * This is not possible with XPS. So, assume the FDB pktcmd to be trap
                     * and resolve the final pktCmd
                     */
                    XP_SAI_LOG_DBG("Getting FDB entry failed. Setting pktcmd drop for neighbor\n");
                    finalPktCmd = xpSaiNeighborResolvePktCmd(inputPktCmd,
                                                             XP_SAI_NEIGHBOR_FDB_MISS_PKTCMD);
                    neighState = XP_SAI_NEIGHBOR_PHYS_INTF_UNRESOLVED;
                    xpsL3NeighborEntry.nhEntry.nextHop.egressIntfId = XPS_INTF_INVALID_ID;
                }
                else
                {
                    /* FDB entry available. Inherit the parameters */
                    xpsL3NeighborEntry.nhEntry.nextHop.egressIntfId = getFdbEntry.intfId;

                    if (pRifEntry->rifType == SAI_ROUTER_INTERFACE_TYPE_BRIDGE)
                    {
                        xpsL3NeighborEntry.nhEntry.nextHop.egressIntfId =
                            XPS_INTF_MAP_INTF_AND_VLAN_TO_BRIDGE_PORT(getFdbEntry.intfId,
                                                                      getFdbEntry.serviceInstId);
                    }

                    finalPktCmd = xpSaiNeighborResolvePktCmd(inputPktCmd, getFdbEntry.pktCmd);
                }

                XP_SAI_LOG_DBG("fdbEntry.intId/portId %d \n", getFdbEntry.intfId);
                break;
            }

        case XPS_SUBINTERFACE_ROUTER: //For Sub-Interface
            {
                saiRetVal = xpSaiGetRouterInterfaceAttrPortId(neighbor_entry->rif_id,
                                                              &egressIntfId);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Error in xpSaiGetRouterInterfaceAttrPortId: error code: %d\n",
                                   saiRetVal);
                    xpFree(indexList);
                    return saiRetVal;
                }

                xpsL3NeighborEntry.nhEntry.nextHop.egressIntfId = (xpsInterfaceId_t)
                                                                  xpSaiObjIdValueGet(egressIntfId.oid);

                saiRetVal = xpSaiGetRouterInterfaceAttrVlanId(neighbor_entry->rif_id, &vlanId);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Error in xpSaiGetRouterInterfaceAttrVlanId: error code: %d\n",
                                   saiRetVal);
                    xpFree(indexList);
                    return saiRetVal;
                }

                xpsL3NeighborEntry.nhEntry.serviceInstId = (uint32_t)xpSaiObjIdValueGet(
                                                               vlanId.oid);
                break;
            }

        default:
            XP_SAI_LOG_ERR("Invalid interface type: %d!\n", intfType);
            xpFree(indexList);
            return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsL3NeighborEntry.nhEntry.nextHop.l3InterfaceId = l3InterfaceId;
    xpsL3NeighborEntry.nhEntry.pktCmd = finalPktCmd;

    XP_SAI_LOG_DBG("###### Before xpsL3AddIpHostEntry , programming neighbor data in H/W \n\n");
    XP_SAI_LOG_DBG("vrfId: %d \n", xpsL3NeighborEntry.vrfId);
    XP_SAI_LOG_DBG("Ip Prefix type: %d \n", xpsL3NeighborEntry.type);

    if (xpsL3NeighborEntry.type == XP_PREFIX_TYPE_IPV4)
    {
        XP_SAI_LOG_DBG("Ipv4Address " FMT_IP4 "\n",
                       PRI_IP4(xpsL3NeighborEntry.ipv4Addr));
    }
    else
    {
        XP_SAI_LOG_DBG("Ipv6Address " FMT_IP6 "\n",
                       PRI_IP6(xpsL3NeighborEntry.ipv6Addr));
    }

    XP_SAI_LOG_DBG("Packet command: %d \n", xpsL3NeighborEntry.nhEntry.pktCmd);
    XP_SAI_LOG_DBG("L3 Interface Id: %d \n",
                   xpsL3NeighborEntry.nhEntry.nextHop.l3InterfaceId);
    XP_SAI_LOG_DBG("MAC Addr " FMT_MAC " \n",
                   PRI_MAC(xpsL3NeighborEntry.nhEntry.nextHop.macDa));

    XP_SAI_LOG_DBG("Egress Int/Port Id: 0x%x \n",
                   xpsL3NeighborEntry.nhEntry.nextHop.egressIntfId);

    if (!xpSaiVrfExists(xpsL3NeighborEntry.vrfId))
    {
        XP_SAI_LOG_ERR("VRF does not exists, vrfId %d", xpsL3NeighborEntry.vrfId);
        xpFree(indexList);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (attributes.noHostRoute.booldata == FALSE)
    {
        retVal = xpsL3FindIpHostEntry(xpsDevId, &xpsL3NeighborEntry, &index);
        if (retVal == XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpsL3NeighborEntry: Entry: " FMT_IP4
                           " is already present for device=%d at index =%d \n",
                           PRI_IP4(xpsL3NeighborEntry.ipv4Addr), xpsDevId,
                           index);
            //xpFree(indexList);
            //return SAI_STATUS_ITEM_ALREADY_EXISTS;

            //ToDo :In scaled scenario, ceratin entries are not cleared properly, so removing below before add.
            //Did as a quick Fix , Need to revisit
            retVal = xpsL3RemoveIpHostEntry(xpsDevId, &xpsL3NeighborEntry);
            if (retVal != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("xpsL3NeighborEntry: Failed!!! in Removing and Existing Entry while adding new entry\n");
                xpFree(indexList);
                return  SAI_STATUS_FAILURE;
            }
        }
        if ((intfType == XPS_VLAN_ROUTER) &&
            (xpsL3NeighborEntry.nhEntry.nextHop.egressIntfId == XPS_INTF_INVALID_ID) &&
            (neighState == XP_SAI_NEIGHBOR_PHYS_INTF_UNRESOLVED))
        {
            if (!(memcmp(&xpsL3NeighborEntry.nhEntry.nextHop.macDa, bCastMac,
                         XP_MAC_ADDR_LEN)) ||
                (fdbUniCastMissAction.s32 == SAI_PACKET_ACTION_FORWARD))
            {
                xpsL3NeighborEntry.nhEntry.pktCmd = XP_PKTCMD_FWD;
                xpsL3NeighborEntry.nhEntry.nextHop.egressIntfId = l3InterfaceId;
                finalPktCmd = XP_PKTCMD_FWD;
            }
        }
        if ((retVal = xpsL3AddIpHostEntry(xpsDevId, &xpsL3NeighborEntry,
                                          indexList)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("%s: Error in inserting host entry(%u): error code: %d\n",
                           __func__, xpsL3NeighborEntry.ipv4Addr, retVal);
            xpFree(indexList);
            return xpsStatus2SaiStatus(retVal);
        }

        retVal = xpSaiVrfHostsNumInc(xpsDevId, xpsL3NeighborEntry.vrfId);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpSaiVrfHostsNumInc failed, retVal : %d \n", retVal);
            xpFree(indexList);
            return xpsStatus2SaiStatus(retVal);
        }

        saiRetVal = xpSaiGetTableEntryCountCtxDb(xpsDevId, &entryCountCtxPtr);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("%s: Failed to get Table Entry Count DB , retVal : %d \n",
                           __FUNCNAME__, saiRetVal);
            xpFree(indexList);
            return saiRetVal;
        }
        if (xpsL3NeighborEntry.type == XP_PREFIX_TYPE_IPV4)
        {
            entryCountCtxPtr->ipv4HostEntries++;
        }
        else if (xpsL3NeighborEntry.type == XP_PREFIX_TYPE_IPV6)
        {
            entryCountCtxPtr->ipv6HostEntries++;
        }
        entryCountCtxPtr->arpEntries++;

    }

    /* Global Programming for neighbor, using ipaddr and VrfId as key */
    memset(&saiL3NeighborKey, 0, sizeof(xpSaiNeighborEntry_t));

    if (xpsL3NeighborEntry.type == XP_PREFIX_TYPE_IPV4)
    {
        saiL3NeighborKey.type = SAI_IP_ADDR_FAMILY_IPV4;
        memcpy(&saiL3NeighborKey.ipv4Addr, xpsL3NeighborEntry.ipv4Addr,
               sizeof(ipv4Addr_t));
    }
    else
    {
        saiL3NeighborKey.type = SAI_IP_ADDR_FAMILY_IPV6;
        memcpy(&saiL3NeighborKey.ipv6Addr, xpsL3NeighborEntry.ipv6Addr,
               sizeof(ipv6Addr_t));
    }

    saiL3NeighborKey.rifId = neighbor_entry->rif_id;

    /* Search for the node in global neighbor Tree database */
    /*The neighbour DB is unified DB for both SAI neigh and NH
        here neigh details to be filled, and NHid will be filled by Nh create*/
    retVal = xpsStateSearchData(XP_SCOPE_DEFAULT, gsaiNeighborDbHdle,
                                (xpsDbKey_t)&saiL3NeighborKey, (void **) &saiL3NeighborEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error in xpsStateSearchData an entry: error code: %d\n",
                       retVal);
        xpFree(indexList);
        return xpsStatus2SaiStatus(retVal);
    }

    if (saiL3NeighborEntry !=NULL)
    {
        newEntry = 0;
    }
    else
    {
        newEntry = 1;
        /* Allocate space for the neighbor entry data */
        if ((retVal = (XP_STATUS)xpsStateHeapMalloc(sizeof(xpSaiNeighborEntry_t),
                                                    (void**)&saiL3NeighborEntry)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpsStateHeapMalloc failed: retVal %d \n", retVal);
            xpFree(indexList);
            return xpsStatus2SaiStatus(retVal);
        }
        memset(saiL3NeighborEntry, 0, sizeof(xpSaiNeighborEntry_t));
    }
    /* update other details*/
    if (xpsL3NeighborEntry.type == XP_PREFIX_TYPE_IPV4)
    {
        saiL3NeighborEntry->type = SAI_IP_ADDR_FAMILY_IPV4;
        memcpy(saiL3NeighborEntry->ipv4Addr, xpsL3NeighborEntry.ipv4Addr,
               sizeof(ipv4Addr_t));
    }
    else
    {
        saiL3NeighborEntry->type = SAI_IP_ADDR_FAMILY_IPV6;
        memcpy(saiL3NeighborEntry->ipv6Addr, xpsL3NeighborEntry.ipv6Addr,
               sizeof(ipv6Addr_t));
    }

    saiL3NeighborEntry->rifId = neighbor_entry->rif_id;
    memcpy(saiL3NeighborEntry->macDa, xpsL3NeighborEntry.nhEntry.nextHop.macDa,
           sizeof(macAddr_t));
    saiL3NeighborEntry->userPktCmd = inputPktCmd;
    saiL3NeighborEntry->resolvedPktCmd = finalPktCmd;
    saiL3NeighborEntry->state = neighState;
    saiL3NeighborEntry->noHostEntry = attributes.noHostRoute.booldata;
    saiL3NeighborEntry->l3IntfId = l3InterfaceId;
    saiL3NeighborEntry->xpsEgressIntf =
        xpsL3NeighborEntry.nhEntry.nextHop.egressIntfId;

    XP_SAI_LOG_NOTICE("Created new neighbor Data in Global S/W DB nhId: %d type %d rif %"
                      PRIx64 " ",
                      saiL3NeighborEntry->nhId, saiL3NeighborEntry->type, saiL3NeighborEntry->rifId);
    if (saiL3NeighborEntry->type == SAI_IP_ADDR_FAMILY_IPV4)
    {
        XP_SAI_LOG_NOTICE("ip4addr" FMT_IP4 "\n",
                          PRI_IP4(saiL3NeighborEntry->ipv4Addr));
    }
    else
    {
        XP_SAI_LOG_NOTICE("ip6addr" FMT_IP6 "\n",
                          PRI_IP6(saiL3NeighborEntry->ipv6Addr));
    }
    if (newEntry)
    {
        saiL3NeighborEntry->nhId = INVALID_INDEX;
        /* Insert the neighbor entry structure into the databse, using the Ip addr and rif_id as key */
        if ((retVal = xpsStateInsertData(XP_SCOPE_DEFAULT, gsaiNeighborDbHdle,
                                         saiL3NeighborEntry)) != XP_NO_ERR)
        {
            // Free Allocated Memory
            xpsStateHeapFree((void*)saiL3NeighborEntry);
            xpFree(indexList);
            return retVal;
        }

        XP_SAI_LOG_NOTICE("Inserting neighbor Data in Global S/W DB success::saiL3NeighborEntry->nhId: %d \n",
                          saiL3NeighborEntry->nhId);
    }
    else
    {
        XP_SAI_LOG_NOTICE("xpsRBTSearchNode Success: updated\n");
    }

    if ((intfType == XPS_VLAN_ROUTER) && (attributes.noHostRoute.booldata == FALSE))
    {
        /* Get the vlan from router intf */
        sai_object_id_t bvOid = SAI_NULL_OBJECT_ID;
        xpsInterfaceId_t rifBdId = XPS_INTF_MAP_INTF_TO_BD(l3InterfaceId);

        if (!rifBdId)
        {
            XP_SAI_LOG_ERR("Failed to map L3 interface: %u to BD\n", l3InterfaceId);
            xpFree(indexList);
            return SAI_STATUS_INVALID_PARAMETER;
        }

        /* Get the vlan OID */
        saiRetVal = xpSaiBridgeVlanObjIdCreate(xpsDevId, rifBdId, &bvOid);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to create BV OID for BD: %u, retVal: %d\n", rifBdId,
                           saiRetVal);
            xpFree(indexList);
            return saiRetVal;
        }

        saiRetVal = xpSaiNeighborFdbAddEntry(xpsDevId, bvOid,
                                             attributes.dstMacAddress.mac, saiL3NeighborEntry);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to add neighbor to FDB neighbor entry, retVal: %d\n",
                           saiRetVal);
            xpFree(indexList);
            return saiRetVal;
        }
    }

    /* Update all other modules which are interested */
    notif.neighborKey = neighbor_entry;
    notif.notifType = XP_SAI_CREATED;

    /* Set the properties */
    notif.props.egressIntf = xpsL3NeighborEntry.nhEntry.nextHop.egressIntfId;
    notif.props.egrIntfValid = true;
    notif.props.state = neighState;
    notif.props.stateValid = true;
    notif.props.pktCmd = finalPktCmd;
    notif.props.pktCmdValid = true;
    memcpy(notif.props.macDa, attributes.dstMacAddress.mac,
           sizeof(notif.props.macDa));
    notif.props.macDaValid = true;

    saiRetVal = xpSaiNeighborNotify(&notif);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed for notify on neighbor create\n");
        xpFree(indexList);
        return saiRetVal;
    }

    xpFree(indexList);
    return saiRetVal;
}


sai_status_t xpSaiCreateNeighborEntry(const sai_neighbor_entry_t
                                      *neighbor_entry, uint32_t attr_count, const sai_attribute_t *attr_list)
{
    sai_status_t status;

    xpSaiNeighborLock();
    status = xpSaiHandleCreateNeighborEntry(neighbor_entry, attr_count, attr_list);
    xpSaiNeighborUnlock();

    return status;
}

//Func: xpSaiRemoveNeighborEntry

sai_status_t xpSaiHandleRemoveNeighborEntry(const sai_neighbor_entry_t
                                            *neighbor_entry)
{
    XP_STATUS retVal = XP_NO_ERR;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    xpsDevice_t xpsDevId = xpSaiGetDevId();
    xpsL3HostEntry_t xpsL3NeighborEntry;
    xpSaiNeighborChangeNotif_t notif;
    xpSaiNeighborEntry_t saiL3NeighbourEntryKey;
    xpSaiNeighborEntry_t *saiL3NeighborEntryData = NULL;
    xpsInterfaceType_e intfType;
    uint32_t vrfId;
    xpsInterfaceId_t l3InterfaceId = 0;
    xpSaiTableEntryCountDbEntry *entryCountCtxPtr = NULL;

    memset(&saiL3NeighbourEntryKey, 0, sizeof(xpSaiNeighborEntry_t));
    memset(&notif, 0, sizeof(notif));

    if (neighbor_entry == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(neighbor_entry->rif_id,
                                  SAI_OBJECT_TYPE_ROUTER_INTERFACE))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n",
                       xpSaiObjIdTypeGet(neighbor_entry->rif_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiGetRouterInterfaceL3IntfId(neighbor_entry->rif_id,
                                                &l3InterfaceId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiGetRouterInterfaceL3IntfId() failed with saiRetVal : %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    /* Loopback processing */
    if (xpSaiRouterInterfaceLoopbackIdCheck(neighbor_entry->rif_id))
    {
        /* Router interface is passed on neighbor entry remove.
         * However, this router interface might be loopback type and there are
         * no use-cases for loopback fastpath
         */
        XP_SAI_LOG_ERR("Loopback router interface received.\n"
                       "Please remove IP2ME route using SAI Route API.\n");
        return SAI_STATUS_FAILURE;
    }

    saiRetVal = saiConvertNeighborEntryToXps(neighbor_entry, &xpsL3NeighborEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error in saiConvertNeighborEntryToXps: error code: %d\n",
                       retVal);
        return saiRetVal;
    }

    retVal = xpsL3GetIntfVrf(xpsDevId, l3InterfaceId, &vrfId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error in xpsL3GetIntfVrf: error code: %d\n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    xpsL3NeighborEntry.vrfId = vrfId;

    if (neighbor_entry->ip_address.addr_family == SAI_IP_ADDR_FAMILY_IPV4)
    {
        saiL3NeighbourEntryKey.type = SAI_IP_ADDR_FAMILY_IPV4;
        memcpy(saiL3NeighbourEntryKey.ipv4Addr, xpsL3NeighborEntry.ipv4Addr,
               XP_IPV4_ADDR_LEN);
    }
    else
    {
        saiL3NeighbourEntryKey.type = SAI_IP_ADDR_FAMILY_IPV6;
        memcpy(saiL3NeighbourEntryKey.ipv6Addr, xpsL3NeighborEntry.ipv6Addr,
               XP_IPV6_ADDR_LEN);
    }

    saiL3NeighbourEntryKey.rifId = neighbor_entry->rif_id;

    saiRetVal = xpSaiNeighborEntryGet(&saiL3NeighbourEntryKey);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not find SAI L3 neighbour entry\n");
        return saiRetVal;
    }

    if (saiL3NeighbourEntryKey.noHostEntry == FALSE)
    {
        xpSaiMacCopy(xpsL3NeighborEntry.nhEntry.nextHop.macDa,
                     saiL3NeighbourEntryKey.macDa);
        if ((retVal = xpsL3RemoveIpHostEntry(xpsDevId,
                                             &xpsL3NeighborEntry)) !=XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("%s: Error in removing an entry: error code: %d\n", __FUNCNAME__,
                           retVal);
            return xpsStatus2SaiStatus(retVal);
        }

        retVal = xpSaiVrfHostsNumDec(xpsDevId, xpsL3NeighborEntry.vrfId);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpSaiVrfHostsNumDec failed, retVal : %d \n", retVal);
            return xpsStatus2SaiStatus(retVal);
        }

        saiRetVal = xpSaiGetTableEntryCountCtxDb(xpsDevId, &entryCountCtxPtr);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("%s: Failed to get Table Entry Count DB , retVal : %d \n",
                           __FUNCNAME__, saiRetVal);
            return saiRetVal;
        }
        if (xpsL3NeighborEntry.type == XP_PREFIX_TYPE_IPV4)
        {
            entryCountCtxPtr->ipv4HostEntries--;
        }
        else if (xpsL3NeighborEntry.type == XP_PREFIX_TYPE_IPV6)
        {
            entryCountCtxPtr->ipv6HostEntries--;
        }
        entryCountCtxPtr->arpEntries--;

    }

    /*
    * Notify the removal to all the interested modules.
    */
    notif.neighborKey = neighbor_entry;
    notif.notifType = XP_SAI_REMOVED;

    saiRetVal = xpSaiNeighborNotify(&notif);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to notify on neighbor removal\n");
        return saiRetVal;
    }

    retVal = xpsStateDeleteData(XP_SCOPE_DEFAULT, gsaiNeighborDbHdle,
                                (xpsDbKey_t)&saiL3NeighbourEntryKey, (void **)&saiL3NeighborEntryData);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to delete state entry from DB\n");
        return xpsStatus2SaiStatus(retVal);
    }

    retVal = xpsInterfaceGetType(l3InterfaceId, &intfType);
    if (retVal)
    {
        XP_SAI_LOG_ERR("Failed to get interface type for L3 interface: %u, retVal: %d\n",
                       l3InterfaceId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    if ((intfType == XPS_VLAN_ROUTER) &&
        (saiL3NeighbourEntryKey.noHostEntry == FALSE))
    {
        /* Get the vlan from router intf */
        sai_object_id_t bvOid = SAI_NULL_OBJECT_ID;
        xpsInterfaceId_t rifBdId = XPS_INTF_MAP_INTF_TO_BD(l3InterfaceId);

        if (!rifBdId)
        {
            XP_SAI_LOG_ERR("Failed to map L3 interface: %u to BD\n", l3InterfaceId);
            xpsStateHeapFree(saiL3NeighborEntryData);
            return SAI_STATUS_INVALID_PARAMETER;
        }

        /* Get the vlan OID */
        saiRetVal = xpSaiBridgeVlanObjIdCreate(xpsDevId, rifBdId, &bvOid);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to create BV OID for BD: %u, retVal: %d\n", rifBdId,
                           saiRetVal);
            xpsStateHeapFree(saiL3NeighborEntryData);
            return saiRetVal;
        }

        saiRetVal = xpSaiNeighborFdbRemoveEntry(xpsDevId, bvOid,
                                                saiL3NeighborEntryData->macDa, saiL3NeighborEntryData);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to remove neighbor from FDB neighbor entry, retVal: %d\n",
                           saiRetVal);
            xpsStateHeapFree(saiL3NeighborEntryData);
            return saiRetVal;
        }
    }

    retVal = xpsStateHeapFree(saiL3NeighborEntryData);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to free memory from heap\n");
        return xpsStatus2SaiStatus(retVal);
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiRemoveNeighborEntry(const sai_neighbor_entry_t
                                      *neighbor_entry)
{
    sai_status_t status;

    xpSaiNeighborLock();
    status = xpSaiHandleRemoveNeighborEntry(neighbor_entry);
    xpSaiNeighborUnlock();

    return status;
}

//Func: xpSaiSetNeighborAttribute

sai_status_t xpSaiHandleSetNeighborAttributes(const sai_neighbor_entry_t
                                              *neighbor_entry, const sai_attribute_t *attr)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;

    if (neighbor_entry == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpSaiAttrCheck(1, attr,
                            NEIGHBOR_ENTRY_VALIDATION_ARRAY_SIZE, neighbor_entry_attribs,
                            SAI_COMMON_API_SET);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", retVal);
        return retVal;
    }

    switch (attr->id)
    {
        case SAI_NEIGHBOR_ENTRY_ATTR_DST_MAC_ADDRESS:
            {
                retVal = xpSaiSetNeighborAttrDstMacAddress(neighbor_entry, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_NEIGHBOR_ENTRY_ATTR_DST_MAC_ADDRESS)\n");
                    return retVal;
                }
                break;
            }
        case SAI_NEIGHBOR_ENTRY_ATTR_PACKET_ACTION:
            {
                retVal = xpSaiSetNeighborAttrPacketAction(neighbor_entry, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_NEIGHBOR_ENTRY_ATTR_PACKET_ACTION)\n");
                    return retVal;
                }
                break;
            }
        case  SAI_NEIGHBOR_ENTRY_ATTR_NO_HOST_ROUTE:
            {
                XP_SAI_LOG_DBG("Attribute %d is not supported.\n", attr->id);
                return  SAI_STATUS_ATTR_NOT_SUPPORTED_0;
            }
        case  SAI_NEIGHBOR_ENTRY_ATTR_META_DATA:
            {
                XP_SAI_LOG_DBG("Attribute %d is not supported.\n", attr->id);
                return SAI_STATUS_ATTR_NOT_SUPPORTED_0;
            }
        default:
            {
                XP_SAI_LOG_ERR("Unknown attribute %d\n", attr->id);
                return SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
            }
    }
    return  retVal;
}

sai_status_t xpSaiSetNeighborAttributes(const sai_neighbor_entry_t
                                        *neighbor_entry, const sai_attribute_t *attr)
{
    sai_status_t status;

    xpSaiNeighborLock();
    status = xpSaiHandleSetNeighborAttributes(neighbor_entry, attr);
    xpSaiNeighborUnlock();

    return status;
}

//Func: xpSaiGetNeighborAttrDestMacAddr

sai_status_t xpSaiGetNeighborAttrDestMacAddr(const sai_neighbor_entry_t
                                             *neighbor_entry, sai_attribute_value_t* value)
{
    XP_STATUS retVal = XP_NO_ERR;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    xpsDevice_t xpsDevId = xpSaiGetDevId();
    xpsInterfaceId_t l3InterfaceId = 0;
    xpsL3HostEntry_t xpsL3NeighborEntry;
    xpSaiNeighborEntry_t saiL3NeighbourEntry;
    uint32_t  vrfId;

    memset(&saiL3NeighbourEntry, 0, sizeof(xpSaiNeighborEntry_t));

    XP_SAI_LOG_DBG("Calling xpSaiGetNeighborAttrDestMacAddr\n");

    memset(&saiL3NeighbourEntry, 0, sizeof(xpSaiNeighborEntry_t));

    if (!XDK_SAI_OBJID_TYPE_CHECK(neighbor_entry->rif_id,
                                  SAI_OBJECT_TYPE_ROUTER_INTERFACE))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n",
                       xpSaiObjIdTypeGet(neighbor_entry->rif_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiGetRouterInterfaceL3IntfId(neighbor_entry->rif_id,
                                                &l3InterfaceId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiGetRouterInterfaceL3IntfId() failed with saiRetVal : %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    saiRetVal = saiConvertNeighborEntryToXps(neighbor_entry, &xpsL3NeighborEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        return saiRetVal;
    }

    retVal = xpsL3GetIntfVrf(xpsDevId, l3InterfaceId, &vrfId);
    if (retVal)
    {
        XP_SAI_LOG_ERR("Error in xpsL3GetIntfVrf: error code: %d\n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    saiL3NeighbourEntry.rifId = neighbor_entry->rif_id;

    if (neighbor_entry->ip_address.addr_family == SAI_IP_ADDR_FAMILY_IPV4)
    {
        saiL3NeighbourEntry.type = SAI_IP_ADDR_FAMILY_IPV4;
        memcpy(saiL3NeighbourEntry.ipv4Addr, xpsL3NeighborEntry.ipv4Addr,
               XP_IPV4_ADDR_LEN);
    }
    else
    {
        saiL3NeighbourEntry.type = SAI_IP_ADDR_FAMILY_IPV6;
        memcpy(saiL3NeighbourEntry.ipv6Addr, xpsL3NeighborEntry.ipv6Addr,
               XP_IPV6_ADDR_LEN);
    }

    saiRetVal = xpSaiNeighborEntryGet(&saiL3NeighbourEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not find SAI L3 neighbour entry\n");
        return saiRetVal;
    }

    memcpy(value->mac, saiL3NeighbourEntry.macDa, sizeof(macAddr_t));
    return xpsStatus2SaiStatus(retVal);

}

//Func: xpSaiGetNeighborAttrPacketAction

sai_status_t xpSaiGetNeighborAttrPacketAction(const sai_neighbor_entry_t
                                              *neighbor_entry, sai_attribute_value_t* value)
{
    XP_STATUS retVal = XP_NO_ERR;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    xpsDevice_t xpsDevId = xpSaiGetDevId();
    xpsInterfaceId_t l3InterfaceId = 0;
    xpsL3HostEntry_t xpsL3NeighborEntry;
    xpSaiNeighborEntry_t saiL3NeighbourEntry;
    uint32_t  vrfId;
    sai_packet_action_t action = SAI_PACKET_ACTION_DROP;

    XP_SAI_LOG_DBG("Calling xpSaiGetNeighborAttrPacketAction\n");

    memset(&saiL3NeighbourEntry, 0, sizeof(xpSaiNeighborEntry_t));

    if (!XDK_SAI_OBJID_TYPE_CHECK(neighbor_entry->rif_id,
                                  SAI_OBJECT_TYPE_ROUTER_INTERFACE))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n",
                       xpSaiObjIdTypeGet(neighbor_entry->rif_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiGetRouterInterfaceL3IntfId(neighbor_entry->rif_id,
                                                &l3InterfaceId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiGetRouterInterfaceL3IntfId() failed with saiRetVal : %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    saiRetVal = saiConvertNeighborEntryToXps(neighbor_entry, &xpsL3NeighborEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        return saiRetVal;
    }

    retVal = xpsL3GetIntfVrf(xpsDevId, l3InterfaceId, &vrfId);
    if (retVal)
    {
        XP_SAI_LOG_ERR("Error in xpsL3GetIntfVrf: error code: %d\n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    saiL3NeighbourEntry.rifId = neighbor_entry->rif_id;

    if (neighbor_entry->ip_address.addr_family == SAI_IP_ADDR_FAMILY_IPV4)
    {
        saiL3NeighbourEntry.type = SAI_IP_ADDR_FAMILY_IPV4;
        memcpy(saiL3NeighbourEntry.ipv4Addr, xpsL3NeighborEntry.ipv4Addr,
               XP_IPV4_ADDR_LEN);
    }
    else
    {
        saiL3NeighbourEntry.type = SAI_IP_ADDR_FAMILY_IPV6;
        memcpy(saiL3NeighbourEntry.ipv6Addr, xpsL3NeighborEntry.ipv6Addr,
               XP_IPV6_ADDR_LEN);
    }

    saiRetVal = xpSaiNeighborEntryGet(&saiL3NeighbourEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not find SAI L3 neighbour entry\n");
        return saiRetVal;
    }

    saiRetVal = xpSaiConvertXpsPacketAction2Sai(saiL3NeighbourEntry.userPktCmd,
                                                &action);
    if (saiRetVal)
    {
        return saiRetVal;
    }

    value->s32 = action;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetNeighborAttrNoHostRoute

sai_status_t xpSaiGetNeighborAttrNoHostRoute(const sai_neighbor_entry_t
                                             *neighbor_entry, sai_attribute_value_t* value)
{
    XP_STATUS retVal = XP_NO_ERR;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    xpsDevice_t xpsDevId = xpSaiGetDevId();
    xpsInterfaceId_t l3InterfaceId = 0;
    xpsL3HostEntry_t xpsL3NeighborEntry;
    xpSaiNeighborEntry_t saiL3NeighbourEntry;
    uint32_t  vrfId;

    XP_SAI_LOG_DBG("Calling xpSaiGetNeighborAttrNoHostRoute\n");

    memset(&saiL3NeighbourEntry, 0, sizeof(xpSaiNeighborEntry_t));

    if (!XDK_SAI_OBJID_TYPE_CHECK(neighbor_entry->rif_id,
                                  SAI_OBJECT_TYPE_ROUTER_INTERFACE))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n",
                       xpSaiObjIdTypeGet(neighbor_entry->rif_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiGetRouterInterfaceL3IntfId(neighbor_entry->rif_id,
                                                &l3InterfaceId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiGetRouterInterfaceL3IntfId() failed with saiRetVal : %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    saiRetVal = saiConvertNeighborEntryToXps(neighbor_entry, &xpsL3NeighborEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        return saiRetVal;
    }

    retVal = xpsL3GetIntfVrf(xpsDevId, l3InterfaceId, &vrfId);
    if (retVal)
    {
        XP_SAI_LOG_ERR("Error in xpsL3GetIntfVrf: error code: %d\n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    saiL3NeighbourEntry.rifId = neighbor_entry->rif_id;

    if (neighbor_entry->ip_address.addr_family == SAI_IP_ADDR_FAMILY_IPV4)
    {
        saiL3NeighbourEntry.type = SAI_IP_ADDR_FAMILY_IPV4;
        memcpy(saiL3NeighbourEntry.ipv4Addr, xpsL3NeighborEntry.ipv4Addr,
               XP_IPV4_ADDR_LEN);
    }
    else
    {
        saiL3NeighbourEntry.type = SAI_IP_ADDR_FAMILY_IPV6;
        memcpy(saiL3NeighbourEntry.ipv6Addr, xpsL3NeighborEntry.ipv6Addr,
               XP_IPV6_ADDR_LEN);
    }

    saiRetVal = xpSaiNeighborEntryGet(&saiL3NeighbourEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not find SAI L3 neighbour entry\n");
        return saiRetVal;
    }

    value->booldata = saiL3NeighbourEntry.noHostEntry;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetNeighborAttribute

static sai_status_t xpSaiGetNeighborAttribute(const sai_neighbor_entry_t
                                              *neighbor_entry, sai_attribute_t* attr, uint32_t attr_index)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    switch (attr->id)
    {
        case SAI_NEIGHBOR_ENTRY_ATTR_DST_MAC_ADDRESS:
            {
                saiRetVal = xpSaiGetNeighborAttrDestMacAddr(neighbor_entry, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_NEIGHBOR_ENTRY_ATTR_DST_MAC_ADDRESS)\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_NEIGHBOR_ENTRY_ATTR_PACKET_ACTION:
            {
                saiRetVal = xpSaiGetNeighborAttrPacketAction(neighbor_entry, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_NEIGHBOR_ENTRY_ATTR_PACKET_ACTION)\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_NEIGHBOR_ENTRY_ATTR_NO_HOST_ROUTE:
            {

                saiRetVal = xpSaiGetNeighborAttrNoHostRoute(neighbor_entry, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_NEIGHBOR_ENTRY_ATTR_NO_HOST_ROUTE)\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_NEIGHBOR_ENTRY_ATTR_META_DATA:
            {
                XP_SAI_LOG_DBG("Attribute %d is not supported.\n", attr->id);
                return SAI_STATUS_ATTR_NOT_SUPPORTED_0 + SAI_STATUS_CODE(attr_index);
            }
        default:
            {
                XP_SAI_LOG_ERR("Failed to set %d unknown attribute\n", attr->id);
                return SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
            }
    }

    return saiRetVal;
}

//Func: xpSaiGetNeighborAttributes

static sai_status_t xpSaiHandleGetNeighborAttributes(const sai_neighbor_entry_t*
                                                     neighbor_entry, uint32_t attr_count, sai_attribute_t *attr_list)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;

    if (neighbor_entry == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpSaiAttrCheck(attr_count, attr_list,
                            NEIGHBOR_ENTRY_VALIDATION_ARRAY_SIZE, neighbor_entry_attribs,
                            SAI_COMMON_API_GET);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", retVal);
        return retVal;
    }

    for (uint32_t count = 0; count < attr_count; count++)
    {
        retVal = xpSaiGetNeighborAttribute(neighbor_entry, &attr_list[count], count);
        if (SAI_STATUS_SUCCESS != retVal)
        {
            XP_SAI_LOG_ERR("xpSaiGetNeighborAttribute failed\n");
            return retVal;
        }
    }

    return retVal;
}

static sai_status_t xpSaiGetNeighborAttributes(const sai_neighbor_entry_t*
                                               neighbor_entry, uint32_t attr_count, sai_attribute_t *attr_list)
{
    sai_status_t status;

    xpSaiNeighborLock();
    status = xpSaiHandleGetNeighborAttributes(neighbor_entry, attr_count,
                                              attr_list);
    xpSaiNeighborUnlock();

    return status;
}

//Func: xpSaiBulkGetNeighborAttributes

sai_status_t xpSaiBulkGetNeighborAttributes(const sai_neighbor_entry_t*
                                            neighbor_entry, uint32_t *attr_count, sai_attribute_t *attr_list)
{
    sai_status_t     saiRetVal  = SAI_STATUS_SUCCESS;
    uint32_t         idx        = 0;
    uint32_t         maxcount   = 0;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    /* Check incoming parameters */
    if ((neighbor_entry == NULL) || (attr_count == NULL) || (attr_list == NULL))
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiMaxCountNeighborAttribute(&maxcount);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not get max attribute count!\n");
        return SAI_STATUS_FAILURE;
    }

    if (*attr_count < maxcount)
    {
        *attr_count = maxcount;
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    for (uint32_t count = 0; count < maxcount; count++)
    {
        attr_list[idx].id = SAI_NEIGHBOR_ENTRY_ATTR_START + count;
        saiRetVal = xpSaiGetNeighborAttribute(neighbor_entry, &attr_list[idx], count);

        if (saiRetVal == SAI_STATUS_SUCCESS)
        {
            idx++;
        }
    }
    *attr_count = idx;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiRemoveAllNeighborEntries

sai_status_t xpSaiHandleRemoveAllNeighborEntries(sai_object_id_t switch_id)
{
    XP_STATUS         retVal   = XP_NO_ERR;
    sai_status_t      saiStatus = SAI_STATUS_SUCCESS;
    xpsL3HostEntry_t  hostKey;

    memset(&hostKey, 0, sizeof(hostKey));

    xpsDevice_t xpsDevId = xpSaiGetDevId();

    hostKey.type = XP_PREFIX_TYPE_IPV4;

    retVal = xpsL3FlushIpHostEntries(xpsDevId, &hostKey);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to flush IPv4 ip hosts, error %d", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    hostKey.type = XP_PREFIX_TYPE_IPV6;

    retVal = xpsL3FlushIpHostEntries(xpsDevId, &hostKey);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to flush IPv6 ip hosts, error %d", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    retVal = xpSaiVrfHostsClear(xpsDevId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVrfHostsClear failed, retVal : %d \n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    /* Notify the delete for all neighbors */
    saiStatus = xpSaiNotifyDeleteForAllNeighbors();
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to notify delete for all neighbors");
        return saiStatus;
    }

    /* Flush the state DB */
    saiStatus = xpSaiNeighborFlushStateDb();
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to flush neighbor state DB");
        return saiStatus;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiRemoveAllNeighborEntries(sai_object_id_t switch_id)
{
    sai_status_t status;

    xpSaiNeighborLock();
    status = xpSaiHandleRemoveAllNeighborEntries(switch_id);
    xpSaiNeighborUnlock();

    return status;
}

//Func: xpSaiNeighborApiInit

XP_STATUS xpSaiNeighborApiInit(uint64_t flag,
                               const sai_service_method_table_t* adapHostServiceMethodTable)
{
    XP_STATUS retVal = XP_NO_ERR;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Calling xpSaiNeighborApiInit\n");

    saiRetVal = xpSaiNeighborInitLock();
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Failed to init SAI neighbor lock\n");
        return XP_ERR_RESOURCE_NOT_AVAILABLE;
    }

    _xpSaiNeighborApi = (sai_neighbor_api_t *) xpMalloc(sizeof(sai_neighbor_api_t));
    if (NULL == _xpSaiNeighborApi)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for _xpSaiNeighborApi\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }

    _xpSaiNeighborApi->create_neighbor_entry = xpSaiCreateNeighborEntry;
    _xpSaiNeighborApi->remove_neighbor_entry = xpSaiRemoveNeighborEntry;
    _xpSaiNeighborApi->set_neighbor_entry_attribute = xpSaiSetNeighborAttributes;
    _xpSaiNeighborApi->get_neighbor_entry_attribute = xpSaiGetNeighborAttributes;
    _xpSaiNeighborApi->remove_all_neighbor_entries = xpSaiRemoveAllNeighborEntries;

    saiRetVal = xpSaiApiRegister(SAI_API_NEIGHBOR, (void*)_xpSaiNeighborApi);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Failed to register neighbor API\n");
        return XP_ERR_ARRAY_OUT_OF_BOUNDS;
    }

    return retVal;
}


//Func: xpSaiNeighborApiDeinit

XP_STATUS xpSaiNeighborApiDeinit()
{
    XP_STATUS retVal = XP_NO_ERR;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Calling xpSaiNeighborApiDeinit\n");

    saiRetVal = xpSaiNeighborDeinitLock();
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Failed to deinit SAI neighbor lock\n");
        return XP_ERR_RESOURCE_NOT_AVAILABLE;
    }
    xpFree(_xpSaiNeighborApi);
    _xpSaiNeighborApi = NULL;

    return retVal;
}

sai_status_t xpSaiNeighborEntryGetFirst(xpSaiNeighborEntry_t *entry)
{
    if (entry == NULL)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    // Search for the node in global neighbor Tree database.
    xpSaiNeighborEntry_t *saiL3NeighborEntry = NULL;
    XP_STATUS status = xpsStateGetNextData(XP_SCOPE_DEFAULT, gsaiNeighborDbHdle,
                                           (xpsDbKey_t)NULL, (void **)&saiL3NeighborEntry);
    if (status != XP_NO_ERR)
    {
        return xpsStatus2SaiStatus(status);
    }

    if (saiL3NeighborEntry == NULL)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    memcpy(entry, saiL3NeighborEntry, sizeof(*entry));

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiNeighborEntryGetNext(xpSaiNeighborEntry_t *entry,
                                       xpSaiNeighborEntry_t *nextEntry)
{
    XP_STATUS retVal = XP_NO_ERR;
    //Global State maintainance
    xpSaiNeighborEntry_t *saiL3NeighborEntry = NULL;

    if ((entry == NULL)  || (nextEntry == NULL))
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    /* Search for the node in global neighbor Tree database */
    retVal = xpsStateGetNextData(XP_SCOPE_DEFAULT, gsaiNeighborDbHdle,
                                 (xpsDbKey_t)entry, (void **)&saiL3NeighborEntry);
    if (retVal != XP_NO_ERR)
    {
        return xpsStatus2SaiStatus(retVal);
    }

    if (saiL3NeighborEntry == NULL)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    memcpy(nextEntry, saiL3NeighborEntry, sizeof(*nextEntry));

    return SAI_STATUS_SUCCESS;
}

// Given the IP anf VRF ID of the neighbor gets the neighbour entry.
sai_status_t xpSaiNeighborEntryGet(xpSaiNeighborEntry_t *entry)
{
    XP_STATUS retVal = XP_NO_ERR;
    //Global State maintainance
    xpSaiNeighborEntry_t saiL3NeighborKey;
    xpSaiNeighborEntry_t *saiL3NeighborEntry = NULL;

    if (entry == NULL)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    memcpy(&saiL3NeighborKey, entry, sizeof(xpSaiNeighborEntry_t));

    /* Search for the node in global neighbor Tree database */
    retVal = xpsStateSearchData(XP_SCOPE_DEFAULT, gsaiNeighborDbHdle,
                                (xpsDbKey_t)&saiL3NeighborKey, (void **)&saiL3NeighborEntry);
    if (retVal != XP_NO_ERR)
    {
        return xpsStatus2SaiStatus(retVal);
    }

    if (saiL3NeighborEntry == NULL)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    memcpy(entry, saiL3NeighborEntry, sizeof(*entry));

    return SAI_STATUS_SUCCESS;
}

// Given the IP anf VRF ID of the neighbor finds the entry and sets other neighbour data to it.
sai_status_t xpSaiNeighborEntrySet(xpSaiNeighborEntry_t *entry)
{
    XP_STATUS retVal = XP_NO_ERR;
    //Global State maintainance
    xpSaiNeighborEntry_t saiL3NeighborKey;
    xpSaiNeighborEntry_t *saiL3NeighborEntry = NULL;

    if (entry == NULL)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    memcpy(&saiL3NeighborKey, entry, sizeof(xpSaiNeighborEntry_t));

    /* Search for the node in global neighbor Tree database */
    retVal = xpsStateSearchData(XP_SCOPE_DEFAULT, gsaiNeighborDbHdle,
                                (xpsDbKey_t)&saiL3NeighborKey, (void **)&saiL3NeighborEntry);
    if (retVal != XP_NO_ERR)
    {
        return xpsStatus2SaiStatus(retVal);
    }

    if (saiL3NeighborEntry == NULL)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    memcpy(saiL3NeighborEntry, entry, sizeof(*saiL3NeighborEntry));

    return SAI_STATUS_SUCCESS;
}

static sai_status_t xpSaiNeighborHandleFdbNotifPerNeighbor(
    xpSaiNeighborEntry_t *nbrEntry, const xpSaiFdbChangeNotif_t *fdbNotif)
{
    xpsL3HostEntry_t    xpsHostEntry;
    sai_status_t        saiStatus;
    XP_STATUS retVal = XP_NO_ERR;

    // Collect initial values.
    xpsPktCmd_e newPktCmd, initPktCmd;
    xpsInterfaceId_t newEgrIntfId, initEgrIntf;

    newPktCmd = initPktCmd = nbrEntry->resolvedPktCmd;
    newEgrIntfId = initEgrIntf = nbrEntry->xpsEgressIntf;
    sai_attribute_value_t fdbUniCastMissAction;
    xpsInterfaceType_e intfType;
    uint8_t bCastMac[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    xpSaiNeighborState_t neighState = XP_SAI_NEIGHBOR_FULLY_RESOLVED;

    retVal = xpsInterfaceGetType(nbrEntry->l3IntfId, &intfType);
    if (retVal)
    {
        XP_SAI_LOG_ERR("Failed to get interface type for L3 interface: %u, retVal: %d\n",
                       nbrEntry->l3IntfId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    saiStatus = xpSaiGetSwitchAttrFdbUnicastMissAction(&fdbUniCastMissAction);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get (SAI_SWITCH_ATTR_FDB_UNICAST_MISS_PACKET_ACTION) | saiStatus : %d\n",
                       saiStatus);
        return saiStatus;
    }

    if ((fdbUniCastMissAction.s32 != SAI_PACKET_ACTION_FORWARD) &&
        (fdbUniCastMissAction.s32 != SAI_PACKET_ACTION_DROP))
    {
        XP_SAI_LOG_ERR(" (SAI_SWITCH_ATTR_FDB_UNICAST_MISS_PACKET_ACTION) Not Supported| Val : %d\n",
                       fdbUniCastMissAction.s32);
        return SAI_STATUS_INVALID_PARAMETER;
    }


    // Get the new values.
    switch (fdbNotif->notifType)
    {
        case XP_SAI_CREATED:
        case XP_SAI_PROPERTY_CHANGED:
            {
                // Get new available values.
                if (fdbNotif->props.egrIntfValid)
                {
                    newEgrIntfId = fdbNotif->props.egrIntfId;
                }

                if (fdbNotif->props.pktCmdValid)
                {
                    newPktCmd = xpSaiNeighborResolvePktCmd((xpsPktCmd_e)nbrEntry->userPktCmd,
                                                           fdbNotif->props.pktCmd);
                }

                break;
            }
        case XP_SAI_REMOVED:
            {
                // If the entry is not found yet, assume FDB pktCmd is XP_SAI_NEIGHBOR_FDB_MISS_PKTCMD and resolve
                newPktCmd = xpSaiNeighborResolvePktCmd(nbrEntry->userPktCmd,
                                                       XP_SAI_NEIGHBOR_FDB_MISS_PKTCMD);
                newEgrIntfId = XPS_INTF_INVALID_ID;
                neighState = XP_SAI_NEIGHBOR_PHYS_INTF_UNRESOLVED;

                break;
            }
    }

    xpSaiNeighborChangeNotif_t nbrNotif;
    memset(&nbrNotif, '\0', sizeof(nbrNotif));
    // Update the XPS host entry.
    if (!nbrEntry->noHostEntry)
    {
        memset(&xpsHostEntry, '\0', sizeof(xpsHostEntry));
        saiStatus = xpSaiNeighborGetXpsHostEntry(nbrEntry, &xpsHostEntry);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiNeighborGetXpsHostEntry: Failed with status=%i.\n",
                           saiStatus);
            return saiStatus;
        }

        xpsHostEntry.nhEntry.pktCmd = newPktCmd;
        xpsHostEntry.nhEntry.nextHop.egressIntfId = newEgrIntfId;
        /* service instanceId is not set , thus use the vlan Id or the bridge Id */
        if (fdbNotif->props.serviceInstId == XPS_INTF_INVALID_ID)
        {
            xpsHostEntry.nhEntry.serviceInstId = (uint32_t)xpSaiObjIdValueGet(
                                                     fdbNotif->entry->bv_id);
        }
        else
        {
            nbrNotif.props.serviceInstIdValid = true;
            nbrNotif.props.serviceInstId = fdbNotif->props.serviceInstId;
            xpsHostEntry.nhEntry.serviceInstId = fdbNotif->props.serviceInstId;
        }
        xpsHostEntry.nhEntry.nextHop.l3InterfaceId = nbrEntry->l3IntfId;
        xpSaiMacCopy(xpsHostEntry.nhEntry.nextHop.macDa, nbrEntry->macDa);

        if ((intfType == XPS_VLAN_ROUTER) &&
            (xpsHostEntry.nhEntry.nextHop.egressIntfId == XPS_INTF_INVALID_ID) &&
            (neighState == XP_SAI_NEIGHBOR_PHYS_INTF_UNRESOLVED))
        {
            if (!(memcmp(&xpsHostEntry.nhEntry.nextHop.macDa, bCastMac, XP_MAC_ADDR_LEN)) ||
                (fdbUniCastMissAction.s32 == SAI_PACKET_ACTION_FORWARD))
            {
                xpsHostEntry.nhEntry.pktCmd = XP_PKTCMD_FWD;
                xpsHostEntry.nhEntry.nextHop.egressIntfId = nbrEntry->l3IntfId;
                newEgrIntfId = nbrEntry->l3IntfId;
                newPktCmd = XP_PKTCMD_FWD;
            }
        }

        saiStatus = xpSaiNeighborUpdateXpsHostEntry(&xpsHostEntry);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiNeighborUpdateXpsHostEntry: Failed with status=%i.\n",
                           saiStatus);
            return saiStatus;
        }
    }

    /* Update the DB entry with new values */
    nbrEntry->resolvedPktCmd = newPktCmd;
    nbrEntry->xpsEgressIntf = newEgrIntfId;
    nbrEntry->state = neighState;
    saiStatus = xpSaiNeighborEntrySet(nbrEntry);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiNeighborEntrySet: Failed with status=%i.\n", saiStatus);
        return saiStatus;
    }

    // Notify the neighbor changes to the interested modules.

    sai_neighbor_entry_t saiNeighborEntry;
    memset(&saiNeighborEntry, '\0', sizeof(saiNeighborEntry));

    saiNeighborEntry.switch_id = fdbNotif->entry->switch_id;
    saiNeighborEntry.rif_id = nbrEntry->rifId;
    switch (nbrEntry->type)
    {
        case SAI_IP_ADDR_FAMILY_IPV4:
            saiNeighborEntry.ip_address.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
            xpSaiIpCopy((uint8_t*)&saiNeighborEntry.ip_address.addr.ip4,
                        (uint8_t*)&nbrEntry->ipv4Addr,
                        SAI_IP_ADDR_FAMILY_IPV4);
            break;

        case SAI_IP_ADDR_FAMILY_IPV6:
            saiNeighborEntry.ip_address.addr_family = SAI_IP_ADDR_FAMILY_IPV6;
            xpSaiIpCopy((uint8_t*)&saiNeighborEntry.ip_address.addr.ip6,
                        (uint8_t*)&nbrEntry->ipv6Addr,
                        SAI_IP_ADDR_FAMILY_IPV6);
            break;

        default:
            XP_SAI_LOG_ERR("Invalid SAI IP address family.\n");
            return SAI_STATUS_INVALID_PARAMETER;
            break;
    }

    nbrNotif.neighborKey = &saiNeighborEntry;
    nbrNotif.notifType = XP_SAI_PROPERTY_CHANGED;

    nbrNotif.props.macDaValid = true;
    memcpy(nbrNotif.props.macDa, nbrEntry->macDa, sizeof(nbrNotif.props.macDa));

    nbrNotif.props.stateValid = true;
    nbrNotif.props.state = neighState;

    if (initPktCmd != newPktCmd)
    {
        nbrNotif.props.pktCmdValid = true;
        nbrNotif.props.pktCmd = newPktCmd;
    }

    if (initEgrIntf != newEgrIntfId)
    {
        nbrNotif.props.egrIntfValid = true;
        nbrNotif.props.egressIntf = newEgrIntfId;
    }

    saiStatus = xpSaiNeighborNotify(&nbrNotif);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiNeighborNotify: Failed with status=%i.\n", saiStatus);
        return saiStatus;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiNeighborFdbNotificationHandler(const xpSaiFdbChangeNotif_t
                                                 *notif)
{
    sai_status_t            retVal   = SAI_STATUS_SUCCESS;
    xpSaiNeighborFdbEntry_t *entry   = NULL;

    if ((notif == NULL) || (notif->entry == NULL))
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument either for notif or notif->entry\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpSaiGetNeighborFdbEntry(xpSaiObjIdValueGet(notif->entry->switch_id),
                                      notif->entry->bv_id, (uint8_t*)notif->entry->mac_address, &entry);
    XP_SAI_LOG_INFO("xpSaiNeighborFdbNotificationHandler xpSaiGetNeighborFdbEntry ret %d bv_id %"
                    PRIx64 "mac" FMT_MAC "\n",
                    retVal,
                    notif->entry->bv_id,
                    PRI_MAC(notif->entry->mac_address));
    if (retVal == SAI_STATUS_ITEM_NOT_FOUND)
    {
        // No corresponding FDB Neighbor entry is a normal case so just return success.
        return SAI_STATUS_SUCCESS;
    }
    else if (retVal != SAI_STATUS_SUCCESS)
    {
        return retVal;
    }

    // Loop through all the neighbors added to this FDB neighbor entry
    for (uint32_t i = 0; i < entry->numItems; i++)
    {
        if (entry->neighborsList[i])
        {
            retVal = xpSaiNeighborHandleFdbNotifPerNeighbor(entry->neighborsList[i], notif);
            if (retVal != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("xpSaiNeighborHandleFdbNotifPerNeighbor: Failed with status=%i.\n",
                               retVal);
            }
        }
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiNeighborHandleFdbNotification(const xpSaiFdbChangeNotif_t
                                                *notif)
{
    sai_status_t status;

    xpSaiNeighborLock();
    status = xpSaiNeighborFdbNotificationHandler(notif);
    xpSaiNeighborUnlock();

    return status;
}

sai_status_t xpSaiMaxCountNeighborAttribute(uint32_t *count)
{
    if (!count)
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }
    *count = SAI_NEIGHBOR_ENTRY_ATTR_END;

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiCountNeighborObjects(uint32_t *count)
{
    XP_STATUS  retVal  = XP_NO_ERR;

    if (!count)
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpsStateGetCount(XP_SCOPE_DEFAULT, gsaiNeighborDbHdle, count);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get Count. return Value : %d\n", retVal);
    }
    return xpsStatus2SaiStatus(retVal);
}

sai_status_t xpSaiGetNeighborObjectList(uint32_t *object_count,
                                        sai_object_key_t *object_list)
{
    XP_STATUS       retVal      = XP_NO_ERR;
    sai_status_t    saiRetVal   = SAI_STATUS_SUCCESS;
    uint32_t        objCount    = 0;
    xpsDevice_t     devId       = xpSaiGetDevId();

    xpSaiNeighborEntry_t   *pNeighborNext    = NULL;

    saiRetVal = xpSaiCountNeighborObjects(&objCount);
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
        retVal = xpsStateGetNextData(XP_SCOPE_DEFAULT, gsaiNeighborDbHdle,
                                     pNeighborNext, (void **)&pNeighborNext);
        if (retVal != XP_NO_ERR || pNeighborNext == NULL)
        {
            XP_SAI_LOG_ERR("Failed to retrieve neighbor object, error %d\n", retVal);
            return SAI_STATUS_FAILURE;
        }

        saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_NEIGHBOR_ENTRY, devId,
                                     (sai_uint64_t)pNeighborNext->l3IntfId,
                                     &object_list[i].key.neighbor_entry.rif_id);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("SAI objectId could not be created, error %d\n", saiRetVal);
            return saiRetVal;
        }

        object_list[i].key.neighbor_entry.ip_address.addr_family = pNeighborNext->type;

        if (pNeighborNext->type == SAI_IP_ADDR_FAMILY_IPV4)
        {
            memcpy(&object_list[i].key.neighbor_entry.ip_address.addr.ip4,
                   &pNeighborNext->ipv4Addr, sizeof(pNeighborNext->ipv4Addr));
        }
        else if (pNeighborNext->type == SAI_IP_ADDR_FAMILY_IPV6)
        {
            memcpy(&object_list[i].key.neighbor_entry.ip_address.addr.ip6,
                   &pNeighborNext->ipv6Addr, sizeof(pNeighborNext->ipv6Addr));
        }
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiUpdateNbr(xpsDevice_t devId, xpsPktCmd_e unknownUcCmd)
{
    XP_STATUS   xpsStatus = XP_NO_ERR;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    xpSaiNeighborEntry_t *curDbEntry = NULL;
    xpSaiNeighborEntry_t *nextDbEntry = NULL;
    xpSaiNeighborEntry_t keyEntry;
    xpsL3HostEntry_t xpsL3NeighborEntry;
    uint32_t vrfId;
    xpsInterfaceType_e xpsIntfType;
    xpSaiNeighborChangeNotif_t  notif;
    /* Flush Neighbor state DB data. */
    memset(&keyEntry, 0, sizeof(keyEntry));

    xpsStatus = xpsStateGetNextData(XP_SCOPE_DEFAULT, gsaiNeighborDbHdle,
                                    (xpsDbKey_t)NULL, (void **)&curDbEntry);
    if (xpsStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get first entry from DB\n");
        return xpsStatus2SaiStatus(xpsStatus);
    }

    while (curDbEntry != NULL)
    {
        /* Get the next entry beforehead as the current entry shall be removed from state DB */
        nextDbEntry = NULL;
        xpsStatus = xpsStateGetNextData(XP_SCOPE_DEFAULT, gsaiNeighborDbHdle,
                                        (xpsDbKey_t)curDbEntry, (void **)&nextDbEntry);
        if (xpsStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to get first entry from DB\n");
            return xpsStatus2SaiStatus(xpsStatus);
        }

        xpsStatus = xpsInterfaceGetType(curDbEntry->l3IntfId, &xpsIntfType);
        if (xpsStatus != XP_NO_ERR)
        {
            return xpsStatus2SaiStatus(xpsStatus);
        }

        memset(&xpsL3NeighborEntry, 0, sizeof(xpsL3HostEntry_t));

        if (xpsIntfType == XPS_VLAN_ROUTER &&
            curDbEntry->state == XP_SAI_NEIGHBOR_PHYS_INTF_UNRESOLVED)
        {
            xpsStatus = xpsL3GetIntfVrf(devId, curDbEntry->l3IntfId, &vrfId);
            if (xpsStatus)
            {
                XP_SAI_LOG_ERR("Error in xpsL3GetIntfVrf: error code: %d\n", xpsStatus);
                return xpsStatus2SaiStatus(xpsStatus);
            }

            xpsL3NeighborEntry.vrfId = vrfId;

            if (curDbEntry->type == SAI_IP_ADDR_FAMILY_IPV4)
            {
                xpsL3NeighborEntry.type = XP_PREFIX_TYPE_IPV4;
                memcpy(xpsL3NeighborEntry.ipv4Addr, curDbEntry->ipv4Addr, XP_IPV4_ADDR_LEN);
            }
            else
            {
                xpsL3NeighborEntry.type = XP_PREFIX_TYPE_IPV6;
                memcpy(xpsL3NeighborEntry.ipv6Addr, curDbEntry->ipv6Addr, XP_IPV6_ADDR_LEN);
            }
            xpsStatus = xpsL3GetIpHostEntry(devId, &xpsL3NeighborEntry);
            if (xpsStatus)
            {
                XP_SAI_LOG_ERR("Error in xpsL3GetIpHostEntry: error code: %d\n", xpsStatus);
                return xpsStatus2SaiStatus(xpsStatus);
            }

            /*Update the current Rif Id*/
            xpsL3NeighborEntry.nhEntry.nextHop.l3InterfaceId = curDbEntry->l3IntfId;
            xpsL3NeighborEntry.nhEntry.pktCmd = unknownUcCmd;
            if (unknownUcCmd == XP_PKTCMD_FWD)
            {
                xpsL3NeighborEntry.nhEntry.nextHop.egressIntfId = curDbEntry->l3IntfId;
            }
            else if (unknownUcCmd == XP_PKTCMD_DROP)
            {
                xpsL3NeighborEntry.nhEntry.nextHop.egressIntfId = XPS_INTF_INVALID_ID;
            }
            else
            {
                XP_SAI_LOG_ERR("UnHandled Cmd code: %d\n", unknownUcCmd);
                return SAI_STATUS_FAILURE;
            }

            xpsStatus = xpsL3UpdateIpHostEntry(devId, &xpsL3NeighborEntry);
            if (xpsStatus)
            {
                XP_SAI_LOG_ERR("Error in xpsL3UpdateIpHostEntry: error code: %d\n", xpsStatus);
                return xpsStatus2SaiStatus(xpsStatus);
            }

            sai_neighbor_entry_t saiNeighborEntry;
            memset(&saiNeighborEntry, '\0', sizeof(saiNeighborEntry));
            //saiNeighborEntry.switch_id = fdbNotif->entry->switch_id;
            saiNeighborEntry.rif_id = curDbEntry->rifId;
            switch (curDbEntry->type)
            {
                case SAI_IP_ADDR_FAMILY_IPV4:
                    saiNeighborEntry.ip_address.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
                    xpSaiIpCopy((uint8_t*)&saiNeighborEntry.ip_address.addr.ip4,
                                (uint8_t*)&curDbEntry->ipv4Addr,
                                SAI_IP_ADDR_FAMILY_IPV4);
                    break;

                case SAI_IP_ADDR_FAMILY_IPV6:
                    saiNeighborEntry.ip_address.addr_family = SAI_IP_ADDR_FAMILY_IPV6;
                    xpSaiIpCopy((uint8_t*)&saiNeighborEntry.ip_address.addr.ip6,
                                (uint8_t*)&curDbEntry->ipv6Addr,
                                SAI_IP_ADDR_FAMILY_IPV6);
                    break;

                default:
                    XP_SAI_LOG_ERR("Invalid SAI IP address family.\n");
                    return SAI_STATUS_INVALID_PARAMETER;
                    break;
            }

            memset(&notif, 0, sizeof(notif));
            notif.neighborKey = &saiNeighborEntry;

            notif.notifType = XP_SAI_PROPERTY_CHANGED;

            memcpy(notif.props.macDa, curDbEntry->macDa, sizeof(notif.props.macDa));
            notif.props.macDaValid = true;

            notif.props.pktCmd = unknownUcCmd;
            notif.props.pktCmdValid = true;
            notif.props.egressIntf = xpsL3NeighborEntry.nhEntry.nextHop.egressIntfId;
            notif.props.egrIntfValid = true;

            notif.props.state = curDbEntry->state;
            notif.props.stateValid = true;

            saiRetVal = xpSaiNeighborNotify(&notif);
            if (saiRetVal != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Failed to notify for neighbor change\n");
                return saiRetVal;
            }
        }
        /* Move to next */
        curDbEntry = nextDbEntry;
    }
    return SAI_STATUS_SUCCESS;
}
