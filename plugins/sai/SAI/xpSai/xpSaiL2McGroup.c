// xpSaiL2McGroup.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpSaiL2McGroup.h"
#include "xpsMulticast.h"
#include "xpSaiSwitch.h"
#include "xpSaiBridge.h"
#include "xpSaiValidationArrays.h"

XP_SAI_LOG_REGISTER_API(SAI_API_L2MC_GROUP);

#define XP_SAI_L2MC_GRP_MAX_IDS             48 * 1024 // 48K multicast groups
#define XP_SAI_L2MC_GRP_RANGE_START         0
#define XP_SAI_L2MC_GRP_MEMBER_MAX_IDS      XP_SAI_L2MC_GRP_MAX_IDS * 128
#define XP_SAI_L2MC_GRP_MEMBER_RANGE_START  0

static sai_l2mc_group_api_t* _xpSaiL2McGroupApi;

static xpsDbHandle_t xpSaiL2McGroupStateDbHndl = XPS_STATE_INVALID_DB_HANDLE;
static xpsDbHandle_t xpSaiL2McGroupMemberStateDbHndl =
    XPS_STATE_INVALID_DB_HANDLE;

/* default L2 Multicast Group to be passed to XPS when entry action is not FORWARD */
static uint32_t defaultL2McGroupId;

sai_status_t xpSaiRemoveL2McGroupMember(sai_object_id_t l2mc_group_member_id);

static int32_t saiL2McGroupCtxKeyComp(void* key1, void* key2)
{
    return ((((xpSaiL2McGroupContextDbEntry *) key1)->l2mcGroupIdOid) - (((
                                                                              xpSaiL2McGroupContextDbEntry *) key2)->l2mcGroupIdOid));
}

static int32_t saiL2McGroupMemberCtxKeyComp(void* key1, void* key2)
{
    return ((((xpSaiL2McGroupMemberContextDbEntry *) key1)->l2mcGroupMemberIdOid)
            - (((xpSaiL2McGroupMemberContextDbEntry *) key2)->l2mcGroupMemberIdOid));
}

static sai_status_t xpSaiGetL2McGroupCtxDb(xpsDevice_t xpsDevId,
                                           sai_object_id_t l2mcGroupIdOid,
                                           xpSaiL2McGroupContextDbEntry **entry)
{
    xpSaiL2McGroupContextDbEntry key;

    memset(&key, 0, sizeof(key));
    key.l2mcGroupIdOid = l2mcGroupIdOid;

    /* Search for corresponding object */
    return xpSaiGetCtxDb(xpSaiScopeFromDevGet(xpsDevId), xpSaiL2McGroupStateDbHndl,
                         (void*) &key, (void**) entry);
}

static sai_status_t xpSaiRemoveL2McGroupCtxDb(xpsDevice_t xpsDevId,
                                              sai_object_id_t l2mcGroupIdOid)
{
    xpSaiL2McGroupContextDbEntry key;

    memset(&key, 0, sizeof(key));
    key.l2mcGroupIdOid = l2mcGroupIdOid;

    /* Remove the corresponding state */
    return xpSaiRemoveCtxDb(xpSaiScopeFromDevGet(xpsDevId),
                            xpSaiL2McGroupStateDbHndl, (void*) &key);
}

static sai_status_t xpSaiGetL2McGroupMemberCtxDb(xpsDevice_t xpsDevId,
                                                 sai_object_id_t l2mcGroupMemberIdOid,
                                                 xpSaiL2McGroupMemberContextDbEntry **entry)
{
    xpSaiL2McGroupMemberContextDbEntry key;

    memset(&key, 0, sizeof(key));
    key.l2mcGroupMemberIdOid = l2mcGroupMemberIdOid;

    return xpSaiGetCtxDb(xpSaiScopeFromDevGet(xpsDevId),
                         xpSaiL2McGroupMemberStateDbHndl,
                         (void*) &key, (void**) entry);
}

static sai_status_t xpSaiRemoveL2McGroupMemberCtxDb(xpsDevice_t xpsDevId,
                                                    sai_object_id_t l2mcGroupMemberIdOid)
{
    xpSaiL2McGroupMemberContextDbEntry key;

    memset(&key, 0, sizeof(key));
    key.l2mcGroupMemberIdOid = l2mcGroupMemberIdOid;

    /* Remove the corresponding state */
    return xpSaiRemoveCtxDb(xpSaiScopeFromDevGet(xpsDevId),
                            xpSaiL2McGroupMemberStateDbHndl, (void*) &key);
}

static sai_status_t xpSaiL2McGroupReplaceItem(xpsDevice_t xpsDevId,
                                              sai_object_id_t l2mcGroupIdOid, xpSaiL2McGroupContextDbEntry *newEntry)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    XP_STATUS xpRetVal = XP_NO_ERR;

    if (newEntry == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument (newEntry: NULL)\n");
        return XP_ERR_INVALID_INPUT;
    }

    /* Remove the corresponding state */
    retVal = xpSaiRemoveL2McGroupCtxDb(xpsDevId, l2mcGroupIdOid);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiRemoveL2McGroupCtxDb failed (retVal: %d)\n", retVal);
        return retVal;
    }

    /* Insert the new state */
    xpRetVal = xpsStateInsertData(xpSaiScopeFromDevGet(xpsDevId),
                                  xpSaiL2McGroupStateDbHndl, (void*) newEntry);
    if (xpRetVal != XP_NO_ERR)
    {
        xpsStateHeapFree((void*) newEntry);
        XP_SAI_LOG_ERR("Failed to insert data (retVal: %d)\n", xpRetVal);
        return xpsStatus2SaiStatus(xpRetVal);
    }

    return retVal;
}

static sai_status_t xpSaiL2McGroupAddElement(xpsDevice_t xpsDevId,
                                             sai_object_id_t l2mcGroupIdOid, sai_object_id_t l2mcGroupMemberIdOid)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    xpSaiL2McGroupContextDbEntry *entry = NULL;
    xpSaiL2McGroupContextDbEntry *newEntry = NULL;

    /* Search for corresponding object */
    retVal = xpSaiGetL2McGroupCtxDb(xpsDevId, l2mcGroupIdOid, &entry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiGetL2McGroupCtxDb failed (retVal: %d)\n", retVal);
        return retVal;
    }

    if (xpSaiCtxGrowthNeeded(entry->numItems,
                             XP_SAI_DEFAULT_MAX_L2MC_GROUP_MEMBERS))
    {
        /* Extend the array size */
        XP_STATUS xpRetVal = xpSaiDynamicArrayGrow((void**)&newEntry, (void*)entry,
                                                   sizeof(xpSaiL2McGroupContextDbEntry), sizeof(sai_object_id_t),
                                                   entry->numItems, XP_SAI_DEFAULT_MAX_L2MC_GROUP_MEMBERS);
        if (xpRetVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpSaiDynamicArrayGrow failed (retVal: %d)\n", xpRetVal);
            return xpsStatus2SaiStatus(xpRetVal);
        }

        /* Replace the corresponding state */
        retVal = xpSaiL2McGroupReplaceItem(xpsDevId, l2mcGroupIdOid, newEntry);
        if (retVal != XP_NO_ERR)
        {
            xpsStateHeapFree((void*) newEntry);
            XP_SAI_LOG_ERR("xpSaiL2McGroupReplaceItem failed. (retVal: %d)\n", retVal);
            return retVal;
        }

        entry = newEntry;
    }

    entry->l2mcGroupMemberIdOid[entry->numItems] = l2mcGroupMemberIdOid;
    entry->numItems++;

    return retVal;
}

static sai_status_t xpSaiL2McGroupDelElement(xpsDevice_t xpsDevId,
                                             sai_object_id_t l2mcGroupIdOid, sai_object_id_t l2mcGroupMemberIdOid)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    xpSaiL2McGroupContextDbEntry *entry = NULL;
    xpSaiL2McGroupContextDbEntry *newEntry = NULL;
    bool found = false;

    /* Search for corresponding object */
    retVal = xpSaiGetL2McGroupCtxDb(xpsDevId, l2mcGroupIdOid, &entry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiGetL2McGroupCtxDb failed (retVal: %d)\n", retVal);
        return retVal;
    }

    /* Remove the bridge port Id from the table */
    for (uint32_t i = 0; i < entry->numItems; i++)
    {
        if (!found)
        {
            found = (entry->l2mcGroupMemberIdOid[i] == l2mcGroupMemberIdOid) ? true : false;
            continue;
        }
        else
        {
            entry->l2mcGroupMemberIdOid[i - 1] = entry->l2mcGroupMemberIdOid[i];
        }
    }

    if (!found)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    /* Decrement the numPorts count */
    entry->numItems--;
    if (xpSaiCtxShrinkNeeded(entry->numItems,
                             XP_SAI_DEFAULT_MAX_L2MC_GROUP_MEMBERS))
    {
        /* Shrink the queueList array size */
        XP_STATUS xpRetVal = xpSaiDynamicArrayShrink((void**)&newEntry, (void*)entry,
                                                     sizeof(xpSaiL2McGroupContextDbEntry), sizeof(sai_object_id_t),
                                                     entry->numItems, XP_SAI_DEFAULT_MAX_L2MC_GROUP_MEMBERS);
        if (xpRetVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpSaiDynamicArrayShrink failed (retVal: %d)\n", xpRetVal);
            return xpsStatus2SaiStatus(xpRetVal);
        }

        /* Replace the corresponding state */
        retVal = xpSaiL2McGroupReplaceItem(xpsDevId, l2mcGroupIdOid, newEntry);
        if (retVal != XP_NO_ERR)
        {
            xpsStateHeapFree((void*) newEntry);
            XP_SAI_LOG_ERR("xpSaiL2McGroupReplaceItem failed. (retVal: %d)\n", retVal);
            return retVal;
        }
    }

    return retVal;
}

static sai_status_t xpSaiL2McGroupValidateElement(xpsDevice_t xpsDevId,
                                                  sai_object_id_t l2mcGroupIdOid, sai_object_id_t brPortIdOid)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    xpSaiL2McGroupContextDbEntry *entry = NULL;
    xpSaiL2McGroupMemberContextDbEntry *memberEntry = NULL;

    /* Search for corresponding object */
    retVal = xpSaiGetL2McGroupCtxDb(xpsDevId, l2mcGroupIdOid, &entry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiGetL2McGroupCtxDb failed (retVal: %d)\n", retVal);
        return retVal;
    }

    /* Remove the bridge port Id from the table */
    for (uint32_t i = 0; i < entry->numItems; i++)
    {
        retVal = xpSaiGetL2McGroupMemberCtxDb(xpsDevId, entry->l2mcGroupMemberIdOid[i],
                                              &memberEntry);
        if (retVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Unable to retreive context data for L2 Multicast group member (groupMemberIdOid: %"
                           PRIu64 ", retVal: %d)\n", entry->l2mcGroupMemberIdOid[i], retVal);

            return retVal;
        }

        if (memberEntry->brPortIdOid == brPortIdOid)
        {
            XP_SAI_LOG_ERR("member Already present in group... (groupMemberIdOid: %" PRIu64
                           ",%" PRIu64 " retVal: %d)\n", entry->l2mcGroupMemberIdOid[i],
                           memberEntry->brPortIdOid, retVal);
            return SAI_STATUS_ITEM_ALREADY_EXISTS;
        }
    }

    return SAI_STATUS_ITEM_NOT_FOUND;
}

static sai_status_t xpSaiL2McGroupHwEntryAddElement(xpsDevice_t xpsDevId,
                                                    sai_object_id_t l2mcGroupIdOid, xpSaiL2McGroupHwEntry hwEntry)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    xpSaiL2McGroupContextDbEntry *entry = NULL;
    xpSaiL2McGroupContextDbEntry *newEntry = NULL;

    /* Search for corresponding object */
    retVal = xpSaiGetL2McGroupCtxDb(xpsDevId, l2mcGroupIdOid, &entry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiGetL2McGroupCtxDb failed (retVal: %d)\n", retVal);
        return retVal;
    }

    if (xpSaiCtxGrowthNeeded(entry->numHwEntries,
                             XP_SAI_DEFAULT_MAX_L2MC_GROUP_ENTRIES))
    {
        /* Extend the array size */
        XP_STATUS xpRetVal = xpSaiDynamicArrayGrow((void**)&newEntry, (void*)entry,
                                                   sizeof(xpSaiL2McGroupContextDbEntry), sizeof(xpSaiL2McGroupHwEntry),
                                                   entry->numHwEntries, XP_SAI_DEFAULT_MAX_L2MC_GROUP_ENTRIES);
        if (xpRetVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpSaiDynamicArrayGrow failed (retVal: %d)\n", xpRetVal);
            return xpsStatus2SaiStatus(xpRetVal);
        }

        /* Replace the corresponding state */
        retVal = xpSaiL2McGroupReplaceItem(xpsDevId, l2mcGroupIdOid, newEntry);
        if (retVal != XP_NO_ERR)
        {
            xpsStateHeapFree((void*) newEntry);
            XP_SAI_LOG_ERR("xpSaiL2McGroupReplaceItem failed. (retVal: %d)\n", retVal);
            return retVal;
        }

        entry = newEntry;
    }

    entry->groupHwEntry[entry->numHwEntries].l2mcGroupId = hwEntry.l2mcGroupId;
    entry->groupHwEntry[entry->numHwEntries].bvId = hwEntry.bvId;
    entry->groupHwEntry[entry->numHwEntries].active = hwEntry.active;
    entry->numHwEntries++;

    return retVal;
}

static sai_status_t xpSaiL2McGroupHwEntryDelElement(xpsDevice_t xpsDevId,
                                                    sai_object_id_t l2mcGroupIdOid, xpSaiL2McGroupHwEntry hwEntry)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    xpSaiL2McGroupContextDbEntry *entry = NULL;
    xpSaiL2McGroupContextDbEntry *newEntry = NULL;
    bool found = false;

    /* Search for corresponding object */
    retVal = xpSaiGetL2McGroupCtxDb(xpsDevId, l2mcGroupIdOid, &entry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiGetL2McGroupCtxDb failed (retVal: %d)\n", retVal);
        return retVal;
    }

    /* Remove the bridge port Id from the table */
    for (uint32_t i = 0; i < entry->numHwEntries; i++)
    {
        if (!found)
        {
            found = (entry->groupHwEntry[i].l2mcGroupId == hwEntry.l2mcGroupId &&
                     entry->groupHwEntry[i].bvId == hwEntry.bvId) ? true : false;
            continue;
        }
        else
        {
            entry->groupHwEntry[i - 1].l2mcGroupId = entry->groupHwEntry[i].l2mcGroupId;
            entry->groupHwEntry[i - 1].bvId = entry->groupHwEntry[i].bvId;
            entry->groupHwEntry[i - 1].active = entry->groupHwEntry[i].active;
        }
    }

    if (!found)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    /* Decrement the numPorts count */
    entry->numHwEntries--;
    if (xpSaiCtxShrinkNeeded(entry->numHwEntries,
                             XP_SAI_DEFAULT_MAX_L2MC_GROUP_ENTRIES))
    {
        /* Shrink the queueList array size */
        XP_STATUS xpRetVal = xpSaiDynamicArrayShrink((void**)&newEntry, (void*)entry,
                                                     sizeof(xpSaiL2McGroupContextDbEntry), sizeof(xpSaiL2McGroupHwEntry),
                                                     entry->numHwEntries, XP_SAI_DEFAULT_MAX_L2MC_GROUP_ENTRIES);
        if (xpRetVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpSaiDynamicArrayShrink failed (retVal: %d)\n", xpRetVal);
            return xpsStatus2SaiStatus(xpRetVal);
        }

        /* Replace the corresponding state */
        retVal = xpSaiL2McGroupReplaceItem(xpsDevId, l2mcGroupIdOid, newEntry);
        if (retVal != XP_NO_ERR)
        {
            xpsStateHeapFree((void*) newEntry);
            XP_SAI_LOG_ERR("xpSaiL2McGroupReplaceItem failed. (retVal: %d)\n", retVal);
            return retVal;
        }
    }

    return retVal;
}

sai_status_t xpSaiCreateL2McGroupOnHardware(xpsDevice_t xpsDevId,
                                            xpVlan_t vlanId,
                                            uint32_t xpsVidx,
                                            uint32_t *ifListId)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpsScope_t scopeId = xpSaiScopeFromDevGet(xpsDevId);

    retVal = xpsMulticastCreateL2InterfaceListScope(scopeId, vlanId, ifListId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsMulticastCreateL2InterfaceList failed (vlanId: %u, retVal: %d)\n",
                       vlanId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    retVal = xpsMulticastIpMLLPairCreateFirstOnly(xpsDevId, *ifListId, xpsVidx);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsMulticastSetVidx failed (ifListId: %u, retVal: %d)\n",
                       *ifListId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    retVal = xpsMulticastIpRouteEntryCreate(xpsDevId, *ifListId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsMulticastIpRouteEntryCreate failed (ifListId: %u, retVal: %d)\n",
                       *ifListId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

#if 0
    retVal = xpsMulticastAddL2InterfaceListToDevice(xpsDevId, *ifListId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsMulticastAddL2InterfaceListToDevice failed (groupId: %u, retVal: %d)\n",
                       *ifListId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }
#endif
    XP_SAI_LOG_DBG("Group created on hardware (groupId: %u, vlanId: %u)\n",
                   *ifListId, vlanId);

    return xpsStatus2SaiStatus(retVal);
}

sai_status_t xpSaiRemoveL2McGroupFromHardware(xpsDevice_t xpsDevId,
                                              uint32_t ifListId)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpsScope_t scope = xpSaiScopeFromDevGet(xpsDevId);
#if 0
    retVal = xpsMulticastRemoveL2InterfaceListFromDevice(xpsDevId, ifListId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsMulticastRemoveL2InterfaceListFromDevice failed (groupId: %u, retVal: %d)\n",
                       ifListId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }
#endif
    retVal = xpsL2McReleaseOldIpMllPairIdx(xpsDevId, ifListId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsMulticastDestroyL2InterfaceList failed (groupId: %u, retVal: %d)\n",
                       ifListId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    retVal = xpsMulticastDestroyL2InterfaceListScope(scope, ifListId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsMulticastDestroyL2InterfaceList failed (groupId: %u, retVal: %d)\n",
                       ifListId, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    XP_SAI_LOG_DBG("Group removed from hardware (groupId: %u)\n", ifListId);

    return xpsStatus2SaiStatus(retVal);
}

sai_status_t xpSaiGetL2McGroupId(xpsDevice_t xpsDevId,
                                 sai_object_id_t groupIdOid, uint16_t vlanId, uint32_t *groupId)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    xpSaiL2McGroupContextDbEntry *entry = NULL;
    xpSaiL2McGroupHwEntry *groupEntryPtr = NULL;

    if (groupId == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument (groupId: NULL)\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(groupIdOid, SAI_OBJECT_TYPE_L2MC_GROUP))
    {
        XP_SAI_LOG_ERR("Wrong object type received (type: %u)\n",
                       xpSaiObjIdTypeGet(groupIdOid));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpSaiGetL2McGroupCtxDb(xpsDevId, groupIdOid, &entry);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiGetL2McGroupCtxDb failed (retVal: %d)\n", retVal);
        return retVal;
    }

    for (uint32_t i = 0; i < entry->numHwEntries; i++)
    {
        if (entry->groupHwEntry[i].bvId == vlanId)
        {
            groupEntryPtr = &entry->groupHwEntry[i];
            break;
        }
    }

    if (groupEntryPtr == NULL)
    {
        XP_SAI_LOG_ERR("Group does not exist on hardware (groupIdOid: %" PRIu64
                       ", vlanId: %u)\n", groupIdOid, vlanId);
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    *groupId = groupEntryPtr->l2mcGroupId;

    return retVal;
}

sai_status_t xpSaiGetL2McGroupIdOid(xpsDevice_t xpsDevId, uint32_t groupId,
                                    uint16_t vlanId, sai_object_id_t *groupIdOid)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    xpSaiL2McGroupContextDbEntry *entryNext = NULL;
    xpsScope_t scope = xpSaiScopeFromDevGet(xpsDevId);
    uint32_t count;
    uint32_t i = 0;

    retVal = xpSaiCountCtxDbObjects(scope, xpSaiL2McGroupStateDbHndl, &count);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiCountCtxDbObjects failed (retVal: %d)\n", retVal);
        return retVal;
    }

    for (i = 0; i < count; i++)
    {
        retVal = xpSaiGetNextCtxDb(scope, xpSaiL2McGroupStateDbHndl, entryNext,
                                   (void **) &entryNext);
        if (retVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to retrieve L2 Multicast Group object (retVal: %d)\n",
                           retVal);
            return retVal;
        }

        uint32_t idx;
        for (idx = 0; idx < entryNext->numHwEntries; idx++)
        {
            if ((entryNext->groupHwEntry[idx].l2mcGroupId == groupId)
                && (entryNext->groupHwEntry[idx].bvId == vlanId))
            {
                break;
            }
        }

        if (idx != entryNext->numHwEntries)
        {
            break;
        }
    }

    if (i == count)
    {
        XP_SAI_LOG_INFO("GroupId is not found (groupId: %u)\n", groupId);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    *groupIdOid = entryNext->l2mcGroupIdOid;

    return retVal;
}

sai_status_t xpSaiCreateL2McGroup(sai_object_id_t *l2mc_group_id,
                                  sai_object_id_t switch_id,
                                  uint32_t attr_count,
                                  const sai_attribute_t *attr_list)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    XP_STATUS xpRetVal = XP_NO_ERR;
    xpsDevice_t xpsDevId = xpSaiObjIdValueGet(switch_id);
    xpsScope_t scope = xpSaiScopeFromDevGet(xpsDevId);
    xpSaiL2McGroupContextDbEntry *entry = NULL;
    xpSaiL2McGroupContextDbEntry key;
    uint32_t id;

    if (l2mc_group_id == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpSaiAttrCheck(attr_count, attr_list, L2MC_GROUP_VALIDATION_ARRAY_SIZE,
                            l2mc_group_attribs, SAI_COMMON_API_CREATE);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Invalid attribute provided (retVal: %d)\n", retVal);
        return retVal;
    }

    xpRetVal = xpsAllocatorAllocateId(scope, XP_SAI_ALLOC_L2MC_GRP, &id);
    if (xpRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Unable to allocate L2 Multicast Group id (retVal: %d)\n",
                       xpRetVal);
        return xpsStatus2SaiStatus(xpRetVal);
    }

    if (xpSaiObjIdCreate(SAI_OBJECT_TYPE_L2MC_GROUP, xpsDevId, (sai_uint64_t) id,
                         l2mc_group_id) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to create SAI L2 Multicast Group object id (id: %u)\n",
                       id);
        return SAI_STATUS_FAILURE;
    }

    xpRetVal = xpsMulticastAllocateVidx(scope, &id);
    if (xpRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Unable to allocate L2 Multicast VIDX (retVal: %d)\n",
                       xpRetVal);
        return xpsStatus2SaiStatus(xpRetVal);
    }

    memset(&key, 0, sizeof(key));
    key.l2mcGroupIdOid = *l2mc_group_id;

    retVal = xpSaiInsertCtxDb(scope, xpSaiL2McGroupStateDbHndl, (void*) &key,
                              sizeof(key), (void **) &entry);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to insert L2 Multicast Group to context DB (groupIdOid: %"
                       PRIu64 ", retVal: %d)\n",
                       key.l2mcGroupIdOid, retVal);
        return retVal;
    }

    entry->l2mcGroupIdOid = *l2mc_group_id;
    entry->xpsVidx = id;

    XP_SAI_LOG_DBG("Group created (groupIdOid: %" PRIu64 ")\n",
                   entry->l2mcGroupIdOid);

    return retVal;
}

sai_status_t xpSaiRemoveL2McGroup(sai_object_id_t l2mc_group_id)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    XP_STATUS xpRetVal = XP_NO_ERR;
    xpsDevice_t xpsDevId = xpSaiObjIdSwitchGet(l2mc_group_id);
    xpSaiL2McGroupContextDbEntry *entry = NULL;
    xpPortList_t mvifPortList;

    if (!XDK_SAI_OBJID_TYPE_CHECK(l2mc_group_id, SAI_OBJECT_TYPE_L2MC_GROUP))
    {
        XP_SAI_LOG_ERR("Wrong object type received (type: %u)\n",
                       xpSaiObjIdTypeGet(l2mc_group_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpSaiGetL2McGroupCtxDb(xpsDevId, l2mc_group_id, &entry);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Unable to retreive context data for L2 Multicast group (groupIdOid: %"
                       PRIu64 ", retVal: %d)\n",
                       l2mc_group_id, retVal);
        return retVal;
    }

    /* for each member remove from the group */
    if (entry->numItems)
    {
        XP_SAI_LOG_ERR("Failed to remove L2 Multicast group as it is in use (groupIdOid: %"
                       PRIu64 ")\n", l2mc_group_id);
        return SAI_STATUS_OBJECT_IN_USE;
    }

    for (uint32_t i = 0; i < entry->numHwEntries; i++)
    {
        if (entry->groupHwEntry[i].active)
        {
            XP_SAI_LOG_ERR("MCGroup in use by L2MC entry. (retVal: %d) bvId :%d Cnt :%d\n",
                           xpRetVal, entry->groupHwEntry[i].bvId, entry->groupHwEntry[i].active);
            return SAI_STATUS_OBJECT_IN_USE;

#if 0
            retVal = xpSaiRemoveL2McGroupFromHardware(xpsDevId,
                                                      entry->groupHwEntry[i].l2mcGroupId);
            if (retVal != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Failed to remove L2 Multicast Group from hardware (groupId: %u, retVal: %d)\n",
                               entry->groupHwEntry[i].l2mcGroupId, retVal);
                return retVal;
            }
#endif
        }
    }

    xpRetVal = xpsAllocatorReleaseId(xpSaiScopeFromDevGet(xpsDevId),
                                     XP_SAI_ALLOC_L2MC_GRP, xpSaiObjIdValueGet(l2mc_group_id));
    if (xpRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Unable to release SAI L2 Multicast Group Id (retVal: %d)\n",
                       xpRetVal);
        return xpsStatus2SaiStatus(xpRetVal);
    }

    xpRetVal = xpsMulticastReleaseVidx(xpSaiScopeFromDevGet(xpsDevId),
                                       entry->xpsVidx);
    if (xpRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Unable to release L2 Multicast VIDX (retVal: %d)\n",
                       xpRetVal);
        return xpsStatus2SaiStatus(xpRetVal);
    }

    retVal = xpSaiRemoveL2McGroupCtxDb(xpsDevId, l2mc_group_id);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to remove L2 Multicast Group from context DB (groupIdOid: %"
                       PRIu64 ")\n", l2mc_group_id);
        return retVal;
    }

    XP_SAI_LOG_DBG("Group removed (groupIdOid: %" PRIu64 ")\n", l2mc_group_id);

    return retVal;
}

sai_status_t xpSaiSetL2McGroupAttribute(sai_object_id_t l2mc_group_id,
                                        const sai_attribute_t *attr)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;

    if (attr == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(l2mc_group_id, SAI_OBJECT_TYPE_L2MC_GROUP))
    {
        XP_SAI_LOG_ERR("Wrong object type received (type: %u)\n",
                       xpSaiObjIdTypeGet(l2mc_group_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpSaiAttrCheck(1, attr, L2MC_GROUP_VALIDATION_ARRAY_SIZE,
                            l2mc_group_attribs, SAI_COMMON_API_SET);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Invalid attribute provided (retVal: %d)\n", retVal);
        return retVal;
    }

    switch (attr->id)
    {
        default:
            {
                XP_SAI_LOG_ERR("Failed to set %d\n", attr->id);
                return SAI_STATUS_INVALID_PARAMETER;
            }
    }

    return retVal;
}

static sai_status_t xpSaiGetL2McGroupAttrMemberList(xpSaiL2McGroupContextDbEntry
                                                    entry,
                                                    sai_object_list_t* pSaiObjList)
{
    if ((pSaiObjList == NULL) || (pSaiObjList->count == 0))
    {
        XP_SAI_LOG_ERR("Invalid parameters received\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (pSaiObjList->count < entry.numItems)
    {
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    pSaiObjList->count = entry.numItems;

    for (uint32_t i = 0; i < pSaiObjList->count; i++)
    {
        pSaiObjList->list[i] = entry.l2mcGroupMemberIdOid[i];
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiGetL2McGroupAttribute(sai_object_id_t l2mc_group_id,
                                        uint32_t attr_count,
                                        sai_attribute_t *attr_list)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    xpsDevice_t xpsDevId = xpSaiObjIdSwitchGet(l2mc_group_id);
    xpSaiL2McGroupContextDbEntry *entry = NULL;

    if (!XDK_SAI_OBJID_TYPE_CHECK(l2mc_group_id, SAI_OBJECT_TYPE_L2MC_GROUP))
    {
        XP_SAI_LOG_ERR("Wrong object type received (type: %u)\n",
                       xpSaiObjIdTypeGet(l2mc_group_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpSaiAttrCheck(attr_count, attr_list, L2MC_GROUP_VALIDATION_ARRAY_SIZE,
                            l2mc_group_attribs, SAI_COMMON_API_GET);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Invalid attribute provided (retVal: %d)\n", retVal);
        return retVal;
    }

    retVal = xpSaiGetL2McGroupCtxDb(xpsDevId, l2mc_group_id, &entry);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Unable to retreive context data for L2 Multicast group (groupIdOid: %"
                       PRIu64 ", retVal: %d)\n",
                       l2mc_group_id, retVal);
        return retVal;
    }

    for (uint32_t count = 0; count < attr_count; count++)
    {
        switch (attr_list[count].id)
        {
            case SAI_L2MC_GROUP_ATTR_L2MC_OUTPUT_COUNT:
                {
                    attr_list[count].value.u32 = entry->numItems;

                    break;
                }
            case SAI_L2MC_GROUP_ATTR_L2MC_MEMBER_LIST:
                {
                    retVal = xpSaiGetL2McGroupAttrMemberList(*entry,
                                                             &attr_list[count].value.objlist);
                    if (retVal != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Failed to get member list of L2 Multicast Group (groupIdOid: %"
                                       PRIu64 ", retVal: %d)\n",
                                       l2mc_group_id, retVal);
                        return retVal;
                    }

                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Failed to get %d\n", attr_list[count].id);
                    return SAI_STATUS_INVALID_PARAMETER;
                }
        }
    }

    return retVal;
}

void xpSaiUpdateL2McGroupMemberAttributeVals(const uint32_t attr_count,
                                             const sai_attribute_t *attr_list, xpSaiL2McGroupMemberAttributesT *attributes)
{
    if ((attributes == NULL) || (attr_list == NULL))
    {
        XP_SAI_LOG_ERR("Invalid input parameter\n");
        return;
    }

    for (uint32_t count = 0; count < attr_count; count++)
    {
        switch (attr_list[count].id)
        {
            case SAI_L2MC_GROUP_MEMBER_ATTR_L2MC_GROUP_ID:
                {
                    attributes->l2mcGroupId = attr_list[count].value.oid;
                    break;
                }
            case SAI_L2MC_GROUP_MEMBER_ATTR_L2MC_OUTPUT_ID:
                {
                    attributes->brPortId = attr_list[count].value.oid;
                    break;
                }
            case SAI_L2MC_GROUP_MEMBER_ATTR_L2MC_ENDPOINT_IP:
                {
                    memcpy(&attributes->tunnelEndpointIP, &attr_list[count].value.ipaddr,
                           sizeof(attributes->tunnelEndpointIP));
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Failed to set %d\n", attr_list[count].id);
                }
        }
    }
}

static sai_status_t xpSaiL2McGroupGetBridgeInfoByBridgePortId(
    sai_object_id_t brPortIdOid,
    xpsInterfaceId_t *intfId,
    sai_bridge_type_t *bv_type, xpsVlan_t *vlanId)
{
    xpsInterfaceId_t interfaceId = XPS_INTF_INVALID_ID;
    xpSaiBridgePort_t info;

    XP_STATUS retVal = xpSaiGetBridgePortById(brPortIdOid, &info);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiGetBridgePortById failed (brPortIdOid: %" PRIu64
                       ", retVal: %d)\n", brPortIdOid, retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    switch (info.brPortType)
    {
        case SAI_BRIDGE_PORT_TYPE_PORT:
            {
                interfaceId = xpSaiObjIdValueGet(info.brPortDescr.objId);
                break;
            }
        case SAI_BRIDGE_PORT_TYPE_SUB_PORT:
            {
                interfaceId = xpSaiObjIdValueGet(info.brPortDescr.objId);
                if (vlanId != NULL)
                {
                    *vlanId      = (xpsVlan_t)info.brPortDescr.subportId.vlanId;
                }
                break;
            }
        case SAI_BRIDGE_PORT_TYPE_TUNNEL:
            {
                /*For tunnel bridge port's HW intf Id need remoteEndpointIp and TunnelId*/
                interfaceId = XPS_INTF_INVALID_ID;
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Bridge port type is not supported (type: %u)\n",
                               info.brPortType);
                return SAI_STATUS_INVALID_PARAMETER;
            }
    }

    if (intfId != NULL)
    {
        *intfId = interfaceId;
    }

    if (bv_type != NULL)
    {
        *bv_type = xpSaiObjIdValueGet(info.bridgeId) ? SAI_BRIDGE_TYPE_1D :
                   SAI_BRIDGE_TYPE_1Q;
    }

    XP_SAI_LOG_DBG("intfId: %u, bridgeType: %u, bridgePortType: %u\n",
                   interfaceId, xpSaiObjIdValueGet(info.bridgeId) ? SAI_BRIDGE_TYPE_1D :
                   SAI_BRIDGE_TYPE_1Q,
                   info.brPortType);

    return xpsStatus2SaiStatus(retVal);
}

static sai_status_t xpSaiActivateL2McGroup(xpsDevice_t xpsDevId, xpsVlan_t bvId,
                                           xpSaiL2McGroupContextDbEntry *entry, uint32_t *groupId)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    xpSaiL2McGroupHwEntry *groupEntryPtr = NULL;
    xpsVlan_t vlanId = 0;
    for (uint32_t i = 0; i < entry->numHwEntries; i++)
    {
        if (entry->groupHwEntry[i].bvId == bvId)
        {
            groupEntryPtr = &entry->groupHwEntry[i];
            break;
        }
    }

    /* group is active, no need to apply it on hardware */
    if (groupEntryPtr != NULL)
    {
        if (groupId != NULL)
        {
            *groupId = groupEntryPtr->l2mcGroupId;
        }
        groupEntryPtr->active++;

        XP_SAI_LOG_DBG("Activated Group DONE (active_count: %d)",
                       groupEntryPtr->active);

        return retVal;
    }

    xpSaiL2McGroupHwEntry groupEntry;
    memset(&groupEntry, 0, sizeof(groupEntry));

    retVal = xpSaiCreateL2McGroupOnHardware(xpsDevId, bvId, entry->xpsVidx,
                                            &groupEntry.l2mcGroupId);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to create L2 Multicast Group on hardwarde (groupId: %u, bvId: %u, retVal: %d)\n",
                       groupEntry.l2mcGroupId, bvId, retVal);
        return retVal;
    }

    for (uint32_t i = 0; i < entry->numItems; i++)
    {
        xpsInterfaceId_t intfId;
        sai_bridge_type_t bv_type;
        sai_bridge_port_type_t bridgePortType;
        sai_object_id_t tunnelId;
        xpSaiL2McGroupMemberContextDbEntry *memberEntry = NULL;
        XP_STATUS xpRetVal = XP_NO_ERR;

        retVal = xpSaiGetL2McGroupMemberCtxDb(xpsDevId, entry->l2mcGroupMemberIdOid[i],
                                              &memberEntry);
        if (retVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Unable to retreive context data for L2 Multicast group member (groupMemberIdOid: %"
                           PRIu64 ", retVal: %d)\n",
                           entry->l2mcGroupMemberIdOid[i], retVal);

            return retVal;
        }

        retVal = xpSaiL2McGroupGetBridgeInfoByBridgePortId(memberEntry->brPortIdOid,
                                                           &intfId, &bv_type, &vlanId);
        if (retVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to get information about Bridge Port (brPortIdOid: %"
                           PRIu64 ", retVal: %d)\n",
                           memberEntry->brPortIdOid, retVal);
            return retVal;
        }
        bridgePortType = (sai_bridge_port_type_t)xpSaiBridgePortObjIdTypeGet(
                             memberEntry->brPortIdOid);

        if ((bridgePortType == SAI_BRIDGE_PORT_TYPE_TUNNEL) &&
            (bv_type == SAI_BRIDGE_TYPE_1D))
        {
            retVal = xpSaiGetTunnelIdByBridgePort(memberEntry->brPortIdOid, &tunnelId);
            if (retVal != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Failed to get tunnel object ID by bridge port (brPortIdOid: %"
                               PRIu64 ", retVal: %d)\n",
                               memberEntry->brPortIdOid, xpRetVal);
                return xpsStatus2SaiStatus(xpRetVal);
            }
            /*Tunnel HW interface Id can be fetched only with TunnelId and remoteVtepIp*/
            retVal = xpSaiTunnelGetHwTunnelIntfIdByTunnIdAndRemoteVtepIp(xpsDevId, tunnelId,
                                                                         memberEntry->tunnelEndpointIP, &intfId);
            if (retVal != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("xpSaiXpsTunnelIntfByTunnelObjIdGet: Failed to fetch from XPS DB for tunnelId: %"
                               PRIu64 " | error: %d.\n", tunnelId, retVal)
                return retVal;
            }

        }
        else if (bv_type == SAI_BRIDGE_TYPE_1D)
        {
            /*For bridgeport intfId is a combination of xps interfaceId and vlanId*/
            intfId = XPS_INTF_MAP_INTF_AND_VLAN_TO_BRIDGE_PORT(intfId, vlanId);
        }
        xpRetVal = xpsMulticastAddInterfaceToL2InterfaceList(xpsDevId,
                                                             groupEntry.l2mcGroupId, intfId);
        if (xpRetVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to add interface to L2 Interface list on hardware (brPortIdOid: %"
                           PRIu64 ", groupId: %u, intfId: %u, retVal: %d)\n",
                           memberEntry->brPortIdOid, groupEntry.l2mcGroupId, intfId, xpRetVal);
            return xpsStatus2SaiStatus(xpRetVal);
        }
    }

    if (groupId != NULL)
    {
        *groupId = groupEntry.l2mcGroupId;
    }

    groupEntry.bvId = bvId;
    groupEntry.active = 1;

    retVal = xpSaiL2McGroupHwEntryAddElement(xpsDevId, entry->l2mcGroupIdOid,
                                             groupEntry);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to add HW entry to L2 Multicast Group list (groupIdOid: %"
                       PRIu64 ", bvId: %u, groupId: %u, retVal: %d)\n",
                       entry->l2mcGroupIdOid, groupEntry.bvId, groupEntry.l2mcGroupId, retVal);
        return retVal;
    }

    XP_SAI_LOG_DBG("Activated Group (groupId: %u, bvId: %u, active_count: %d)\n",
                   groupEntry.l2mcGroupId, groupEntry.bvId, groupEntry.active);

    return retVal;
}

sai_status_t xpSaiActivateBridgeL2McGroup(xpsDevice_t xpsDevId,
                                          sai_object_id_t bridgeOid, sai_object_id_t groupIdOid, uint32_t *groupId)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    xpsVlan_t bdId = (xpsVlan_t)xpSaiObjIdValueGet(bridgeOid);
    xpSaiL2McGroupContextDbEntry *entry = NULL;
    sai_object_id_t l2mcGroupBridgeOid = SAI_NULL_OBJECT_ID;

    if (groupIdOid == SAI_NULL_OBJECT_ID)
    {
        return SAI_STATUS_SUCCESS;
    }

    XP_SAI_LOG_DBG("Activating Dot1D Group (groupIdOid: %" PRIu64 ", bridgeOid: %"
                   PRIu64 ")\n", groupIdOid, bridgeOid);

    retVal = xpSaiGetL2McGroupCtxDb(xpsDevId, groupIdOid, &entry);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiGetL2McGroupCtxDb failed (groupIdOid: %" PRIu64
                       ", retVal: %d)\n", groupIdOid, retVal);
        return retVal;
    }

    for (uint32_t i = 0; i < entry->numItems; i++)
    {
        sai_bridge_type_t bv_type;
        xpSaiL2McGroupMemberContextDbEntry *memberEntry = NULL;

        retVal = xpSaiGetL2McGroupMemberCtxDb(xpsDevId, entry->l2mcGroupMemberIdOid[i],
                                              &memberEntry);
        if (retVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Unable to retreive context data for L2 Multicast group member (groupMemberIdOid: %"
                           PRIu64 ", retVal: %d)\n",
                           entry->l2mcGroupMemberIdOid[i], retVal);

            return retVal;
        }

        retVal = xpSaiL2McGroupGetBridgeInfoByBridgePortId(memberEntry->brPortIdOid,
                                                           NULL, &bv_type, NULL);
        if (retVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to get information about Bridge Port (brPortIdOid: %"
                           PRIu64 ", retVal: %d)\n",
                           memberEntry->brPortIdOid, retVal);
            return retVal;
        }

        retVal = xpSaiGetBridgeIdByBridgePort(memberEntry->brPortIdOid,
                                              &l2mcGroupBridgeOid);
        if (SAI_STATUS_SUCCESS != retVal)
        {
            XP_SAI_LOG_ERR("Failed to check bridge membership for Bridge Port (brPortIdOid: %"
                           PRIu64 ", bridgeOid: %" PRIu64 ", retVal: %d)\n",
                           memberEntry->brPortIdOid, bridgeOid, retVal);
            return retVal;
        }

        if (l2mcGroupBridgeOid != bridgeOid)
        {
            XP_SAI_LOG_ERR("Bridge Port is not member of the Bridge (brPortIdOid: %" PRIu64
                           ", vlanIdOid: %" PRIu64 ")\n",
                           memberEntry->brPortIdOid, bridgeOid);
            return SAI_STATUS_ITEM_NOT_FOUND;
        }
    }

    retVal = xpSaiActivateL2McGroup(xpsDevId, bdId, entry, groupId);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to activate L2MC group (bdId: %u, retVal: %d)\n", bdId,
                       retVal);
        return retVal;
    }

    return retVal;
}


sai_status_t xpSaiActivateVlanL2McGroup(xpsDevice_t xpsDevId,
                                        sai_object_id_t vlanIdOid, sai_object_id_t groupIdOid, uint32_t *groupId)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    xpVlan_t vlanId = (uint16_t)xpSaiObjIdValueGet(vlanIdOid);
    xpSaiL2McGroupContextDbEntry *entry = NULL;

    /* Relevant only for Dot1Q Vlans */
    if ((vlanId > XPS_VLANID_MAX) || (groupIdOid == SAI_NULL_OBJECT_ID))
    {
        return SAI_STATUS_SUCCESS;
    }

    XP_SAI_LOG_DBG("Activating Dot1Q Group (groupIdOid: %" PRIu64 ", vlanIdOid: %"
                   PRIu64 ")\n", groupIdOid, vlanIdOid);

    retVal = xpSaiGetL2McGroupCtxDb(xpsDevId, groupIdOid, &entry);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiGetL2McGroupCtxDb failed (groupIdOid: %" PRIu64
                       ", retVal: %d)\n", groupIdOid, retVal);
        return retVal;
    }

    for (uint32_t i = 0; i < entry->numItems; i++)
    {
        sai_bridge_type_t bv_type;
        xpSaiL2McGroupMemberContextDbEntry *memberEntry = NULL;

        retVal = xpSaiGetL2McGroupMemberCtxDb(xpsDevId, entry->l2mcGroupMemberIdOid[i],
                                              &memberEntry);
        if (retVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Unable to retreive context data for L2 Multicast group member (groupMemberIdOid: %"
                           PRIu64 ", retVal: %d)\n",
                           entry->l2mcGroupMemberIdOid[i], retVal);

            return retVal;
        }

        retVal = xpSaiL2McGroupGetBridgeInfoByBridgePortId(memberEntry->brPortIdOid,
                                                           NULL, &bv_type, NULL);
        if (retVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to get information about Bridge Port (brPortIdOid: %"
                           PRIu64 ", retVal: %d)\n",
                           memberEntry->brPortIdOid, retVal);
            return retVal;
        }

        if (bv_type != SAI_BRIDGE_TYPE_1Q)
        {
            XP_SAI_LOG_ERR("Bridge Port type is invalid (brPortIdOid: %" PRIu64
                           ", bv_type: %u)\n",
                           memberEntry->brPortIdOid, bv_type);
            return SAI_STATUS_INVALID_PARAMETER;
        }

        retVal = xpSaiBridgePortIsVlanMember(vlanIdOid, memberEntry->brPortIdOid);
        if (retVal != SAI_STATUS_SUCCESS)
        {
            if (retVal == SAI_STATUS_ITEM_NOT_FOUND)
            {
                XP_SAI_LOG_ERR("Bridge Port is not member of the Vlan (brPortIdOid: %" PRIu64
                               ", vlanIdOid: %" PRIu64 ")\n",
                               memberEntry->brPortIdOid, vlanIdOid);
            }
            else
            {
                XP_SAI_LOG_ERR("Failed to check vlan membership for Bridge Port (brPortIdOid: %"
                               PRIu64 ", vlanIdOid: %" PRIu64 ", retVal: %d)\n",
                               memberEntry->brPortIdOid, vlanIdOid, retVal);
            }
            return retVal;
        }
    }

    retVal = xpSaiActivateL2McGroup(xpsDevId, vlanId, entry, groupId);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to activate L2MC group (vlanId: %u, retVal: %d)\n",
                       vlanId, retVal);
        return retVal;
    }

    return retVal;
}

sai_status_t xpSaiDeActivateVlanL2McGroup(xpsDevice_t xpsDevId,
                                          sai_object_id_t bvIdOid, sai_object_id_t groupIdOid)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    xpSaiL2McGroupContextDbEntry *entry = NULL;
    xpVlan_t bvId = (uint16_t)xpSaiObjIdValueGet(bvIdOid);
    xpSaiL2McGroupHwEntry *groupEntryPtr = NULL;

    if (groupIdOid == SAI_NULL_OBJECT_ID)
    {
        return SAI_STATUS_SUCCESS;
    }

    XP_SAI_LOG_DBG("DeActivating Group (groupIdOid: %" PRIu64 ")\n", groupIdOid);

    retVal = xpSaiGetL2McGroupCtxDb(xpsDevId, groupIdOid, &entry);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        if (retVal == SAI_STATUS_ITEM_NOT_FOUND)
        {
            XP_SAI_LOG_DBG("L2 Multicast group no more exist (groupIdOid: %" PRIu64
                           ", retVal: %d)\n", groupIdOid, retVal);
        }
        else
        {
            XP_SAI_LOG_ERR("xpSaiGetL2McGroupCtxDb failed (groupIdOid: %" PRIu64
                           ", retVal: %d)\n", groupIdOid, retVal);
        }
        return retVal;
    }

    for (uint32_t i = 0; i < entry->numHwEntries; i++)
    {
        if (entry->groupHwEntry[i].bvId == bvId)
        {
            groupEntryPtr = &entry->groupHwEntry[i];
            break;
        }
    }

    if (groupEntryPtr == NULL)
    {
        XP_SAI_LOG_DBG("L2 Multicast HW group no more exist (groupIdOid: %" PRIu64
                       ", retVal: %d)\n", groupIdOid, retVal);
        return SAI_STATUS_SUCCESS;
    }

    /* someone still use the group, no need to clear the hardware */
    if (groupEntryPtr->active > 1)
    {
        groupEntryPtr->active--;
        XP_SAI_LOG_NOTICE("DeActivating Group DONE (active_count: %d)",
                          groupEntryPtr->active);
        return retVal;
    }

    for (uint32_t i = 0; i < entry->numItems; i++)
    {
        xpsInterfaceId_t intfId;
        xpsVlan_t vlanId = 0;
        sai_bridge_type_t bv_type;
        sai_bridge_port_type_t bridgePortType;
        sai_object_id_t tunnelId;
        XP_STATUS xpRetVal = XP_NO_ERR;
        xpSaiL2McGroupMemberContextDbEntry *memberEntry = NULL;

        retVal = xpSaiGetL2McGroupMemberCtxDb(xpsDevId, entry->l2mcGroupMemberIdOid[i],
                                              &memberEntry);
        if (retVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Unable to retreive context data for L2 Multicast group member (groupMemberIdOid: %"
                           PRIu64 ", retVal: %d)\n",
                           entry->l2mcGroupMemberIdOid[i], retVal);

            return retVal;
        }

        retVal = xpSaiL2McGroupGetBridgeInfoByBridgePortId(memberEntry->brPortIdOid,
                                                           &intfId, &bv_type, &vlanId);
        if (retVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to get information about Bridge Port (brPortIdOid: %"
                           PRIu64 ", retVal: %d)\n",
                           memberEntry->brPortIdOid, retVal);
            return retVal;
        }
        bridgePortType = (sai_bridge_port_type_t)xpSaiBridgePortObjIdTypeGet(
                             memberEntry->brPortIdOid);
        if ((bridgePortType == SAI_BRIDGE_PORT_TYPE_TUNNEL) &&
            (bv_type == SAI_BRIDGE_TYPE_1D))
        {
            retVal = xpSaiGetTunnelIdByBridgePort(memberEntry->brPortIdOid, &tunnelId);
            if (retVal != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Failed to get tunnel object ID by bridge port (brPortIdOid: %"
                               PRIu64 ", retVal: %d)\n",
                               memberEntry->brPortIdOid, xpRetVal);
                return xpsStatus2SaiStatus(xpRetVal);
            }
            /*Tunnel HW interface Id can be fetched only with TunnelId and remoteVtepIp*/
            retVal = xpSaiTunnelGetHwTunnelIntfIdByTunnIdAndRemoteVtepIp(xpsDevId, tunnelId,
                                                                         memberEntry->tunnelEndpointIP, &intfId);
            if (retVal != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("xpSaiXpsTunnelIntfByTunnelObjIdGet: Failed to fetch from XPS DB for tunnelId: %"
                               PRIu64 " | error: %d.\n", tunnelId, retVal)
                return retVal;
            }
        }
        else if (bv_type == SAI_BRIDGE_TYPE_1D)
        {
            /*For bridgeport intfId is a combination of xps interfaceId and vlanId*/
            intfId = XPS_INTF_MAP_INTF_AND_VLAN_TO_BRIDGE_PORT(intfId, vlanId);
        }
        XP_SAI_LOG_DBG("Removing interface from group (groupId: %u, intfId: %u)\n",
                       groupEntryPtr->l2mcGroupId, intfId);

        xpRetVal = xpsMulticastRemoveInterfaceFromL2InterfaceList(xpsDevId,
                                                                  groupEntryPtr->l2mcGroupId, intfId);
        if (xpRetVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to remove inteface from L2 Interface list on hardware (groupId: %u, intfId: %u, retVal: %d)\n",
                           groupEntryPtr->l2mcGroupId, intfId, xpRetVal);
            return xpsStatus2SaiStatus(xpRetVal);
        }
    }

    retVal = xpSaiRemoveL2McGroupFromHardware(xpsDevId, groupEntryPtr->l2mcGroupId);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to remove L2 Multicast Group from hardware (groupId: %u, retVal: %d)\n",
                       groupEntryPtr->l2mcGroupId, retVal);
        return retVal;
    }

    XP_SAI_LOG_DBG("DeActivated Group (groupId: %u, active_count: %d)\n",
                   groupEntryPtr->l2mcGroupId, groupEntryPtr->active);

    retVal = xpSaiL2McGroupHwEntryDelElement(xpsDevId, entry->l2mcGroupIdOid,
                                             *groupEntryPtr);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to remove HW entry from L2 Multicast Group list (groupIdOid: %"
                       PRIu64 ", bvId: %u, groupId: %u, retVal: %d)\n",
                       entry->l2mcGroupIdOid, groupEntryPtr->bvId, groupEntryPtr->l2mcGroupId, retVal);
        return retVal;
    }

    return retVal;
}

sai_status_t xpSaiL2McGroupAddVxTunnel(sai_object_id_t l2mcGroupId,
                                       xpsInterfaceId_t tnlIntfId)
{
    xpsDevice_t xpsDevId = xpSaiObjIdSwitchGet(l2mcGroupId);
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    XP_STATUS xpRetVal = XP_NO_ERR;
    xpSaiL2McGroupContextDbEntry *groupEntry = NULL;

    if (!XDK_SAI_OBJID_TYPE_CHECK(l2mcGroupId, SAI_OBJECT_TYPE_L2MC_GROUP))
    {
        XP_SAI_LOG_DBG("Wrong object type received!\n");
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    retVal = xpSaiGetL2McGroupCtxDb(xpsDevId, l2mcGroupId, &groupEntry);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        if (retVal == SAI_STATUS_ITEM_NOT_FOUND)
        {
            XP_SAI_LOG_ERR("L2 Multicast group does not exist (groupIdOid: %" PRIu64
                           ", retVal: %d)\n",
                           l2mcGroupId, retVal);
        }
        else
        {
            XP_SAI_LOG_ERR("Unable to retreive context data for L2 Multicast group (groupIdOid: %"
                           PRIu64 ", retVal: %d)\n",
                           l2mcGroupId, retVal);
        }
        return retVal;
    }

    for (uint32_t i = 0; i < groupEntry->numHwEntries; i++)
    {
        if (groupEntry->groupHwEntry[i].active)
        {
            XP_SAI_LOG_DBG("Adding interface to group on hardware (groupId: %u, tnlIntfId: %u)\n",
                           groupEntry->groupHwEntry[i].l2mcGroupId, tnlIntfId);

            xpRetVal = xpsMulticastAddInterfaceToL2InterfaceList(xpsDevId,
                                                                 groupEntry->groupHwEntry[i].l2mcGroupId, tnlIntfId);
            if (xpRetVal != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Failed to add interface to L2 Interface list on hardware (groupId: %u, tnlIntfId: %u, retVal: %d)\n",
                               groupEntry->groupHwEntry[i].l2mcGroupId, tnlIntfId, xpRetVal);
                return xpsStatus2SaiStatus(xpRetVal);
            }
        }
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiL2McGroupRemoveVxTunnel(sai_object_id_t l2mcGroupId,
                                          xpsInterfaceId_t tnlIntfId)
{
    xpsDevice_t xpsDevId = xpSaiObjIdSwitchGet(l2mcGroupId);
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    XP_STATUS xpRetVal = XP_NO_ERR;
    xpSaiL2McGroupContextDbEntry *groupEntry = NULL;

    if (!XDK_SAI_OBJID_TYPE_CHECK(l2mcGroupId, SAI_OBJECT_TYPE_L2MC_GROUP))
    {
        XP_SAI_LOG_DBG("Wrong object type received!\n");
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    retVal = xpSaiGetL2McGroupCtxDb(xpsDevId, l2mcGroupId, &groupEntry);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiGetL2McGroupCtxDb failed (retVal: %d)\n", retVal);
        return retVal;
    }

    for (uint32_t i = 0; i < groupEntry->numHwEntries; i++)
    {
        if (groupEntry->groupHwEntry[i].active)
        {
            XP_SAI_LOG_DBG("Removing interface from group on hardware (groupId: %u, tnlIntfId: %u)\n",
                           groupEntry->groupHwEntry[i].l2mcGroupId, tnlIntfId);

            xpRetVal = xpsMulticastRemoveInterfaceFromL2InterfaceList(xpsDevId,
                                                                      groupEntry->groupHwEntry[i].l2mcGroupId, tnlIntfId);
            if (xpRetVal != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Failed to remove interface from L2 Interface list on hardware (groupId: %u, tnlIntfId: %u, retVal: %d)\n",
                               groupEntry->groupHwEntry[i].l2mcGroupId, tnlIntfId, xpRetVal);
                return xpsStatus2SaiStatus(xpRetVal);
            }
        }
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiCreateL2McGroupMember(sai_object_id_t *l2mc_group_member_id,
                                        sai_object_id_t switch_id,
                                        uint32_t attr_count,
                                        const sai_attribute_t *attr_list)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    XP_STATUS xpRetVal = XP_NO_ERR;
    xpsDevice_t xpsDevId = xpSaiObjIdValueGet(switch_id);
    xpSaiL2McGroupMemberContextDbEntry *entry = NULL;
    xpSaiL2McGroupMemberContextDbEntry key;
    xpSaiL2McGroupContextDbEntry *groupEntry = NULL;
    xpSaiL2McGroupMemberAttributesT attributes;
    uint32_t id;
    xpsInterfaceId_t intfId;
    sai_bridge_type_t bridgeType;
    sai_bridge_port_type_t bridgePortType;
    xpsVlan_t vlanId = 0;
    sai_object_id_t tunnelId;
    if (l2mc_group_member_id == NULL)
    {
        XP_SAI_LOG_ERR("Invalid input parameter\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpSaiAttrCheck(attr_count, attr_list,
                            L2MC_GROUP_MEMBER_VALIDATION_ARRAY_SIZE,
                            l2mc_group_member_attribs, SAI_COMMON_API_CREATE);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Invalid attribute provided (retVal: %d)\n", retVal);
        return retVal;
    }

    xpSaiUpdateL2McGroupMemberAttributeVals(attr_count, attr_list, &attributes);

    if (!XDK_SAI_OBJID_TYPE_CHECK(attributes.l2mcGroupId,
                                  SAI_OBJECT_TYPE_L2MC_GROUP))
    {
        XP_SAI_LOG_ERR("Wrong object type received (type: %u)\n",
                       xpSaiObjIdTypeGet(attributes.l2mcGroupId));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(attributes.brPortId, SAI_OBJECT_TYPE_BRIDGE_PORT))
    {
        XP_SAI_LOG_ERR("Wrong object type received (type: %u)\n",
                       xpSaiObjIdTypeGet(attributes.brPortId));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpSaiL2McGroupGetBridgeInfoByBridgePortId(attributes.brPortId, &intfId,
                                                       &bridgeType, &vlanId);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get information about Bridge Port (brPortIdOid: %"
                       PRIu64 ", retVal: %d)\n",
                       attributes.brPortId, retVal);
        return retVal;
    }

    retVal = xpSaiGetL2McGroupCtxDb(xpsDevId, attributes.l2mcGroupId, &groupEntry);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        if (retVal == SAI_STATUS_ITEM_NOT_FOUND)
        {
            XP_SAI_LOG_ERR("L2 Multicast group does not exist (groupIdOid: %" PRIu64
                           ", retVal: %d)\n",
                           attributes.l2mcGroupId, retVal);
        }
        else
        {
            XP_SAI_LOG_ERR("Unable to retreive context data for L2 Multicast group (groupIdOid: %"
                           PRIu64 ", retVal: %d)\n",
                           attributes.l2mcGroupId, retVal);
        }
        return retVal;
    }

    retVal = xpSaiL2McGroupValidateElement(xpsDevId, groupEntry->l2mcGroupIdOid,
                                           attributes.brPortId);
    if (retVal == SAI_STATUS_ITEM_ALREADY_EXISTS)
    {
        return retVal;
    }

    for (uint32_t i = 0; i < groupEntry->numHwEntries; i++)
    {
        if (groupEntry->groupHwEntry[i].active)
        {
            XP_SAI_LOG_DBG("Adding interface to group on hardware (groupId: %u, intfId: %u)\n",
                           groupEntry->groupHwEntry[i].l2mcGroupId, intfId);

            bridgePortType = (sai_bridge_port_type_t)xpSaiBridgePortObjIdTypeGet(
                                 attributes.brPortId);

            if (bridgePortType == SAI_BRIDGE_PORT_TYPE_TUNNEL)
            {
                retVal = xpSaiGetTunnelIdByBridgePort(attributes.brPortId, &tunnelId);
                if (retVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to get tunnel object ID by bridge port (brPortIdOid: %"
                                   PRIu64 ", retVal: %d)\n",
                                   attributes.brPortId, xpRetVal);
                    return xpsStatus2SaiStatus(xpRetVal);
                }

                retVal = xpSaiTunnelIntfOnMcMemberAdded(tunnelId, attributes.brPortId,
                                                        attributes.l2mcGroupId, attributes.tunnelEndpointIP);
                if (retVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to add L2MC data to tunnel DB (tunnelId: %" PRIu64
                                   ", brPortIdOid: %" PRIu64 ", groupId: %" PRIu64 ", retVal: %d)\n",
                                   tunnelId, attributes.brPortId, attributes.l2mcGroupId, xpRetVal);
                    return xpsStatus2SaiStatus(xpRetVal);
                }
            }
            else
            {
                if (bridgeType == SAI_BRIDGE_TYPE_1D)
                {
                    /*For bridgeport intfId is a combination of xps interfaceId and vlanId*/
                    intfId = XPS_INTF_MAP_INTF_AND_VLAN_TO_BRIDGE_PORT(intfId, vlanId);
                }
                xpRetVal = xpsMulticastAddInterfaceToL2InterfaceList(xpsDevId,
                                                                     groupEntry->groupHwEntry[i].l2mcGroupId, intfId);
                if (xpRetVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("Failed to add interface to L2 Interface list on hardware (brPortIdOid: %"
                                   PRIu64 ", groupId: %u, intfId: %u, retVal: %d)\n",
                                   attributes.brPortId, groupEntry->groupHwEntry[i].l2mcGroupId, intfId, xpRetVal);
                    return xpsStatus2SaiStatus(xpRetVal);
                }
            }
        }
    }

    /* allocate unique id for L2 Multicast Group Member */
    xpRetVal = xpsAllocatorAllocateId(xpSaiScopeFromDevGet(xpsDevId),
                                      XP_SAI_ALLOC_L2MC_GRP_MEMBER, &id);
    if (xpRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Unable to allocate L2 Multicast Group Member id (retVal: %d)\n",
                       xpRetVal);
        return xpsStatus2SaiStatus(xpRetVal);
    }

    memset(&key, 0, sizeof(key));

    /* generate OID based on unique XPS id */
    if (xpSaiObjIdCreate(SAI_OBJECT_TYPE_L2MC_GROUP_MEMBER, xpsDevId,
                         (sai_uint64_t) id, &key.l2mcGroupMemberIdOid) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to create SAI L2 Multicast Group Member object id (id: %u)\n",
                       id);
        return SAI_STATUS_FAILURE;
    }

    retVal = xpSaiInsertCtxDb(xpSaiScopeFromDevGet(xpsDevId),
                              xpSaiL2McGroupMemberStateDbHndl, (void*) &key, sizeof(key), (void **) &entry);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to insert L2 Multicast Group member to context DB (l2mcGroupMemberIdOid: %"
                       PRIu64 ")\n",
                       key.l2mcGroupMemberIdOid);
        return retVal;
    }

    entry->l2mcGroupMemberIdOid = key.l2mcGroupMemberIdOid;
    entry->l2mcGroupIdOid = attributes.l2mcGroupId;
    entry->brPortIdOid = attributes.brPortId;
    memcpy(&entry->tunnelEndpointIP, &attributes.tunnelEndpointIP,
           sizeof(entry->tunnelEndpointIP));

    retVal = xpSaiL2McGroupAddElement(xpsDevId, entry->l2mcGroupIdOid,
                                      entry->l2mcGroupMemberIdOid);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to add interface to L2 Multicast Group list (groupIdOid: %"
                       PRIu64 ", groupMemberIdOid: %" PRIu64 ", retVal: %d)\n",
                       entry->l2mcGroupIdOid, entry->l2mcGroupMemberIdOid, retVal);
        return retVal;
    }

    *l2mc_group_member_id = entry->l2mcGroupMemberIdOid;

    XP_SAI_LOG_DBG("Group member created (groupIdOid: %" PRIu64
                   ", groupMemberIdOid: %" PRIu64 ", brPortIdOid: %" PRIu64 ")\n",
                   entry->l2mcGroupIdOid, entry->l2mcGroupMemberIdOid, entry->brPortIdOid);

    return retVal;
}

sai_status_t xpSaiRemoveL2McGroupMember(sai_object_id_t l2mc_group_member_id)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    XP_STATUS xpRetVal = XP_NO_ERR;
    xpsDevice_t xpsDevId = xpSaiObjIdSwitchGet(l2mc_group_member_id);
    xpSaiL2McGroupMemberContextDbEntry *entry = NULL;
    xpSaiL2McGroupContextDbEntry *groupEntry = NULL;
    xpsInterfaceId_t intfId;
    sai_bridge_type_t bridgeType;
    sai_bridge_port_type_t bridgePortType;
    sai_object_id_t tunnelId;
    xpsVlan_t vlanId = 0;
    if (!XDK_SAI_OBJID_TYPE_CHECK(l2mc_group_member_id,
                                  SAI_OBJECT_TYPE_L2MC_GROUP_MEMBER))
    {
        XP_SAI_LOG_ERR("Wrong object type received (type: %u)\n",
                       xpSaiObjIdTypeGet(l2mc_group_member_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpSaiGetL2McGroupMemberCtxDb(xpsDevId, l2mc_group_member_id, &entry);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Unable to retreive context data for L2 Multicast group member (groupMemberIdOid: %"
                       PRIu64 ", retVal: %d)\n",
                       l2mc_group_member_id, retVal);
        return retVal;
    }

    retVal = xpSaiL2McGroupGetBridgeInfoByBridgePortId(entry->brPortIdOid, &intfId,
                                                       &bridgeType, &vlanId);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get information about Bridge Port (brPortIdOid: %"
                       PRIu64 ", retVal: %d)\n",
                       entry->brPortIdOid, retVal);
        return retVal;
    }

    retVal = xpSaiGetL2McGroupCtxDb(xpsDevId, entry->l2mcGroupIdOid, &groupEntry);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiGetL2McGroupCtxDb failed (retVal: %d)\n", retVal);
        return retVal;
    }

    for (uint32_t i = 0; i < groupEntry->numHwEntries; i++)
    {
        if (groupEntry->groupHwEntry[i].active)
        {
            XP_SAI_LOG_DBG("Removing interface from group on hardware (groupId: %u, intfId: %u)\n",
                           groupEntry->groupHwEntry[i].l2mcGroupId, intfId);

            bridgePortType = (sai_bridge_port_type_t)xpSaiBridgePortObjIdTypeGet(
                                 entry->brPortIdOid);

            if (bridgePortType == SAI_BRIDGE_PORT_TYPE_TUNNEL)
            {
                retVal = xpSaiGetTunnelIdByBridgePort(entry->brPortIdOid, &tunnelId);
                if (retVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to get tunnel object ID by bridge port (brPortIdOid: %"
                                   PRIu64 ", retVal: %d)\n",
                                   entry->brPortIdOid, xpRetVal);
                    return xpsStatus2SaiStatus(xpRetVal);
                }
                retVal = xpSaiTunnelIntfOnMcMemberRemoved(tunnelId, entry->brPortIdOid,
                                                          entry->l2mcGroupIdOid, entry->tunnelEndpointIP);
                if (retVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to remove L2MC data from tunnel DB (tunnelId: %" PRIu64
                                   ", brPortIdOid: %" PRIu64 ", groupId: %" PRIu64 ", retVal: %d)\n", tunnelId,
                                   entry->brPortIdOid, entry->l2mcGroupIdOid, retVal);
                    return xpsStatus2SaiStatus(xpRetVal);
                }
            }
            else
            {
                if (bridgeType == SAI_BRIDGE_TYPE_1D)
                {
                    /*For bridgeport intfId is a combination of xps interfaceId and vlanId*/
                    intfId = XPS_INTF_MAP_INTF_AND_VLAN_TO_BRIDGE_PORT(intfId, vlanId);
                }
                xpRetVal = xpsMulticastRemoveInterfaceFromL2InterfaceList(xpsDevId,
                                                                          groupEntry->groupHwEntry[i].l2mcGroupId, intfId);
                if (xpRetVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("Failed to remove interface from L2 Interface list on hardware (brPortIdOid: %"
                                   PRIu64 ", groupId: %u, intfId: %u, retVal: %d)\n",
                                   entry->brPortIdOid, groupEntry->groupHwEntry[i].l2mcGroupId, intfId, xpRetVal);
                    return xpsStatus2SaiStatus(xpRetVal);
                }
            }
        }
    }

    retVal = xpSaiL2McGroupDelElement(xpsDevId, entry->l2mcGroupIdOid,
                                      entry->l2mcGroupMemberIdOid);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to remove interface from L2 Multicast Group list (groupIdOid: %"
                       PRIu64 ", groupMemberIdOid: %" PRIu64 ", retVal: %d)\n",
                       entry->l2mcGroupIdOid, entry->l2mcGroupMemberIdOid, retVal);
        return retVal;
    }

    xpRetVal = xpsAllocatorReleaseId(xpSaiScopeFromDevGet(xpsDevId),
                                     XP_SAI_ALLOC_L2MC_GRP_MEMBER, xpSaiObjIdValueGet(l2mc_group_member_id));
    if (xpRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Unable to release L2McGroupMember Id (retVal: %d)\n", xpRetVal);
        return xpsStatus2SaiStatus(xpRetVal);
    }

    XP_SAI_LOG_DBG("Group member removed (groupIdOid: %" PRIu64
                   ", groupMemberIdOid: %" PRIu64 ", brPortIdOid: %" PRIu64 ")\n",
                   entry->l2mcGroupIdOid, entry->l2mcGroupMemberIdOid, entry->brPortIdOid);

    retVal = xpSaiRemoveL2McGroupMemberCtxDb(xpsDevId, l2mc_group_member_id);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to remove L2 Multicast Group member context DB (groupMemberIdOid: %"
                       PRIu64 ")\n", l2mc_group_member_id);
        return retVal;
    }

    return retVal;
}

sai_status_t xpSaiSetL2McGroupMemberAttribute(sai_object_id_t
                                              l2mc_group_member_id,
                                              const sai_attribute_t *attr)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;

    if (attr == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpSaiAttrCheck(1, attr, L2MC_GROUP_MEMBER_VALIDATION_ARRAY_SIZE,
                            l2mc_group_member_attribs, SAI_COMMON_API_SET);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Invalid attribute provided (retVal: %d)\n", retVal);
        return retVal;
    }

    switch (attr->id)
    {
        default:
            {
                XP_SAI_LOG_ERR("Failed to set %d\n", attr->id);
                return SAI_STATUS_INVALID_PARAMETER;
            }
    }

    return retVal;
}

sai_status_t xpSaiGetL2McGroupMemberAttribute(sai_object_id_t
                                              l2mc_group_member_id,
                                              uint32_t attr_count,
                                              sai_attribute_t *attr_list)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    xpsDevice_t xpsDevId = xpSaiObjIdSwitchGet(l2mc_group_member_id);
    xpSaiL2McGroupMemberContextDbEntry *entry = NULL;

    retVal = xpSaiAttrCheck(attr_count, attr_list,
                            L2MC_GROUP_MEMBER_VALIDATION_ARRAY_SIZE,
                            l2mc_group_member_attribs, SAI_COMMON_API_GET);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Invalid attribute provided (retVal: %d)\n", retVal);
        return retVal;
    }

    retVal = xpSaiGetL2McGroupMemberCtxDb(xpsDevId, l2mc_group_member_id, &entry);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Unable to retreive context data for L2 Multicast group member (groupMemberIdOid: %"
                       PRIu64 ", retVal: %d)\n",
                       l2mc_group_member_id, retVal);
        return retVal;
    }

    for (uint32_t count = 0; count < attr_count; count++)
    {
        switch (attr_list[count].id)
        {
            case SAI_L2MC_GROUP_MEMBER_ATTR_L2MC_GROUP_ID:
                {
                    attr_list[count].value.oid = entry->l2mcGroupIdOid;
                    break;
                }
            case SAI_L2MC_GROUP_MEMBER_ATTR_L2MC_OUTPUT_ID:
                {
                    attr_list[count].value.oid = entry->brPortIdOid;
                    break;
                }
            case SAI_L2MC_GROUP_MEMBER_ATTR_L2MC_ENDPOINT_IP:
                {
                    memcpy(&attr_list[count].value.ipaddr, &entry->tunnelEndpointIP,
                           sizeof(attr_list[count].value.ipaddr));
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Failed to get %d\n", attr_list[count].id);
                    return SAI_STATUS_INVALID_PARAMETER;
                }
        }
    }

    return retVal;
}

sai_status_t xpSaiL2McGroupApiInit(uint64_t flag,
                                   const sai_service_method_table_t* adapHostServiceMethodTable)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;

    _xpSaiL2McGroupApi = (sai_l2mc_group_api_t *) xpMalloc(sizeof(
                                                               sai_l2mc_group_api_t));

    if (_xpSaiL2McGroupApi == NULL)
    {
        XP_SAI_LOG_ERR("Failed to allocate _xpSaiL2McGroupApi\n");
        return SAI_STATUS_NO_MEMORY;
    }

    _xpSaiL2McGroupApi->create_l2mc_group = xpSaiCreateL2McGroup;
    _xpSaiL2McGroupApi->remove_l2mc_group = xpSaiRemoveL2McGroup;
    _xpSaiL2McGroupApi->set_l2mc_group_attribute = xpSaiSetL2McGroupAttribute;
    _xpSaiL2McGroupApi->get_l2mc_group_attribute = xpSaiGetL2McGroupAttribute;
    _xpSaiL2McGroupApi->create_l2mc_group_member = xpSaiCreateL2McGroupMember;
    _xpSaiL2McGroupApi->remove_l2mc_group_member = xpSaiRemoveL2McGroupMember;
    _xpSaiL2McGroupApi->set_l2mc_group_member_attribute =
        xpSaiSetL2McGroupMemberAttribute;
    _xpSaiL2McGroupApi->get_l2mc_group_member_attribute =
        xpSaiGetL2McGroupMemberAttribute;

    retVal = xpSaiApiRegister(SAI_API_L2MC_GROUP, (void*)_xpSaiL2McGroupApi);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to register L2MC GROUP API\n");
        return retVal;
    }

    return retVal;
}

sai_status_t xpSaiL2McGroupApiDeinit()
{
    xpFree(_xpSaiL2McGroupApi);
    _xpSaiL2McGroupApi = NULL;

    return SAI_STATUS_SUCCESS;
}

uint32_t xpSaiGetDefaultL2McGroupId(void)
{
    return defaultL2McGroupId;
}

sai_status_t xpSaiL2McCreateDefaultGroup(xpsDevice_t xpsDevId)
{
    uint16_t defaultVlan = 1;

    return xpSaiCreateL2McGroupOnHardware(xpsDevId, defaultVlan, 0,
                                          &defaultL2McGroupId);
}

/*This is called in switch init*/
sai_status_t xpSaiL2McGroupInit(xpsDevice_t xpsDevId)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpsScope_t scope = xpSaiScopeFromDevGet(xpsDevId);

    /* Create global SAI L2MC GROUP DB */
    xpSaiL2McGroupStateDbHndl = XPSAI_L2MC_GROUP_STATE_DB_HNDL;
    if ((retVal = xpsStateRegisterDb(scope, "SAI L2MC GROUP DB", XPS_GLOBAL,
                                     &saiL2McGroupCtxKeyComp,
                                     xpSaiL2McGroupStateDbHndl)) != XP_NO_ERR)
    {
        xpSaiL2McGroupStateDbHndl = XPS_STATE_INVALID_DB_HANDLE;
        XP_SAI_LOG_ERR("Could not create SAI L2MC GROUP DB\n");
        return xpsStatus2SaiStatus(retVal);
    }

    /* Create global SAI L2MC GROUP PORT DB */
    xpSaiL2McGroupMemberStateDbHndl = XPSAI_L2MC_GROUP_PORT_STATE_DB_HNDL;
    if ((retVal = xpsStateRegisterDb(scope, "SAI L2MC GROUP PORT DB", XPS_GLOBAL,
                                     &saiL2McGroupMemberCtxKeyComp,
                                     xpSaiL2McGroupMemberStateDbHndl)) != XP_NO_ERR)
    {
        xpSaiL2McGroupMemberStateDbHndl = XPS_STATE_INVALID_DB_HANDLE;
        XP_SAI_LOG_ERR("Could not create SAI L2MC GROUP PORT DB\n");
        return xpsStatus2SaiStatus(retVal);
    }

    retVal = xpsAllocatorInitIdAllocator(scope, XP_SAI_ALLOC_L2MC_GRP,
                                         XP_SAI_L2MC_GRP_MAX_IDS, XP_SAI_L2MC_GRP_RANGE_START);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to initialize SAI L2MC GROUP ID allocator\n");
        return xpsStatus2SaiStatus(retVal);
    }

    retVal = xpsAllocatorInitIdAllocator(scope, XP_SAI_ALLOC_L2MC_GRP_MEMBER,
                                         XP_SAI_L2MC_GRP_MEMBER_MAX_IDS, XP_SAI_L2MC_GRP_MEMBER_RANGE_START);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to initialize SAI L2MC GROUP ID allocator\n");
        return xpsStatus2SaiStatus(retVal);
    }

    return xpsStatus2SaiStatus(retVal);
}

sai_status_t xpSaiL2McGroupDeInit(xpsDevice_t xpsDevId)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpsScope_t scope = xpSaiScopeFromDevGet(xpsDevId);

    /* Purge global SAI L2MC GROUP DB */
    if ((retVal = xpsStateDeRegisterDb(scope,
                                       &xpSaiL2McGroupStateDbHndl)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not purge SAI L2MC GROUP DB\n");
        return xpsStatus2SaiStatus(retVal);
    }

    /* Purge global SAI L2MC GROUP PORT DB */
    if ((retVal = xpsStateDeRegisterDb(scope,
                                       &xpSaiL2McGroupMemberStateDbHndl)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not purge SAI L2MC GROUP PORT DB\n");
        return xpsStatus2SaiStatus(retVal);
    }

    return xpsStatus2SaiStatus(retVal);
}
