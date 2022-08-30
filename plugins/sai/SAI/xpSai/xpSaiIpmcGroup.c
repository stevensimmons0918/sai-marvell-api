// xpSaiIpmcGroup.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpSaiIpmcGroup.h"
#include "xpsMulticast.h"
#include "xpSaiSwitch.h"
#include "xpSaiBridge.h"
#include "xpSaiVlan.h"
#include "xpSaiValidationArrays.h"

XP_SAI_LOG_REGISTER_API(SAI_API_IPMC_GROUP);

#define XP_SAI_IPMC_GRP_MAX_IDS             48 * 1024 // 48K multicast groups
#define XP_SAI_IPMC_GRP_RANGE_START         0
#define XP_SAI_IPMC_GRP_MEMBER_MAX_IDS      XP_SAI_IPMC_GRP_MAX_IDS * 128
#define XP_SAI_IPMC_GRP_MEMBER_RANGE_START  0

static sai_ipmc_group_api_t* _xpSaiIpmcGroupApi;

static xpsDbHandle_t xpSaiIpmcGroupStateDbHndl = XPS_STATE_INVALID_DB_HANDLE;
static xpsDbHandle_t xpSaiIpmcGroupMemberStateDbHndl =
    XPS_STATE_INVALID_DB_HANDLE;
static xpsDbHandle_t xpSaiIpmcGroupVlanGroupStateDbHndl =
    XPS_STATE_INVALID_DB_HANDLE;

static int32_t saiIpmcGroupCtxKeyComp(void* key1, void* key2)
{
    return ((((xpSaiIpmcGroupContextDbEntry *) key1)->groupIdOid) - (((
                                                                          xpSaiIpmcGroupContextDbEntry *) key2)->groupIdOid));
}

static int32_t saiIpmcGroupMemberCtxKeyComp(void* key1, void* key2)
{
    return ((((xpSaiIpmcGroupMemberContextDbEntry *) key1)->groupMemberIdOid) - (((
            xpSaiIpmcGroupMemberContextDbEntry *) key2)->groupMemberIdOid));
}

static int32_t saiIpmcGroupVlanGroupCtxKeyComp(void* key1, void* key2)
{
    return ((((xpSaiIpmcGroupVlan2GroupContextDbEntry *) key1)->bvId) - (((
                                                                              xpSaiIpmcGroupVlan2GroupContextDbEntry *) key2)->bvId));
}

static sai_status_t xpSaiGetIpmcGroupVlanGroupCtxDb(xpsDevice_t xpsDevId,
                                                    uint16_t bvId,
                                                    xpSaiIpmcGroupVlan2GroupContextDbEntry **entry)
{
    xpSaiIpmcGroupVlan2GroupContextDbEntry key;

    memset(&key, 0, sizeof(key));
    key.bvId = bvId;

    return xpSaiGetCtxDb(xpSaiScopeFromDevGet(xpsDevId),
                         xpSaiIpmcGroupVlanGroupStateDbHndl,
                         (void*) &key, (void**) entry);
}

static sai_status_t xpSaiRemoveIpmcGroupVlanGroupCtxDb(xpsDevice_t xpsDevId,
                                                       uint16_t bvId)
{
    xpSaiIpmcGroupVlan2GroupContextDbEntry key;

    memset(&key, 0, sizeof(key));
    key.bvId = bvId;

    /* Remove the corresponding state */
    return xpSaiRemoveCtxDb(xpSaiScopeFromDevGet(xpsDevId),
                            xpSaiIpmcGroupVlanGroupStateDbHndl, (void*) &key);
}

static sai_status_t xpSaiIpmcGroupVlanGroupReplaceItem(xpsDevice_t xpsDevId,
                                                       uint16_t bvId, xpSaiIpmcGroupVlan2GroupContextDbEntry *newEntry)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    XP_STATUS xpRetVal = XP_NO_ERR;

    if (newEntry == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument (newEntry: NULL)\n");
        return XP_ERR_INVALID_INPUT;
    }

    /* Remove the corresponding state */
    retVal = xpSaiRemoveIpmcGroupVlanGroupCtxDb(xpsDevId, bvId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiRemoveIpmcGroupVlanGroupCtxDb failed (retVal: %d)\n",
                       retVal);
        return retVal;
    }

    /* Insert the new state */
    xpRetVal = xpsStateInsertData(xpSaiScopeFromDevGet(xpsDevId),
                                  xpSaiIpmcGroupVlanGroupStateDbHndl, (void*) newEntry);
    if (xpRetVal != XP_NO_ERR)
    {
        xpsStateHeapFree((void*) newEntry);
        XP_SAI_LOG_ERR("Failed to insert data (retVal: %d)\n", xpRetVal);
        return xpsStatus2SaiStatus(xpRetVal);
    }

    return retVal;
}

static sai_status_t xpSaiIpmcGroupVlanGroupAddElement(xpsDevice_t xpsDevId,
                                                      uint16_t bvId, uint32_t groupId)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    xpSaiIpmcGroupVlan2GroupContextDbEntry *entry = NULL;
    xpSaiIpmcGroupVlan2GroupContextDbEntry *newEntry = NULL;

    /* Search for corresponding object */
    retVal = xpSaiGetIpmcGroupVlanGroupCtxDb(xpsDevId, bvId, &entry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiGetIpmcGroupVlanGroupCtxDb failed (retVal: %d)\n", retVal);
        return retVal;
    }

    if (xpSaiCtxGrowthNeeded(entry->numItems,
                             XP_SAI_DEFAULT_MAX_IPMC_GROUP_MEMBERS))
    {
        /* Extend the array size */
        XP_STATUS xpRetVal = xpSaiDynamicArrayGrow((void**)&newEntry, (void*)entry,
                                                   sizeof(xpSaiIpmcGroupVlan2GroupContextDbEntry), sizeof(uint32_t),
                                                   entry->numItems, XP_SAI_DEFAULT_MAX_IPMC_GROUP_MEMBERS);
        if (xpRetVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpSaiDynamicArrayGrow failed (retVal: %d)\n", xpRetVal);
            return xpsStatus2SaiStatus(xpRetVal);
        }

        /* Replace the corresponding state */
        retVal = xpSaiIpmcGroupVlanGroupReplaceItem(xpsDevId, bvId, newEntry);
        if (retVal != XP_NO_ERR)
        {
            xpsStateHeapFree((void*) newEntry);
            XP_SAI_LOG_ERR("xpSaiIpmcGroupVlanGroupReplaceItem failed. (retVal: %d)\n",
                           retVal);
            return retVal;
        }

        entry = newEntry;
    }

    entry->l3mcGroupId[entry->numItems] = groupId;
    entry->numItems++;

    return retVal;
}

static sai_status_t xpSaiIpmcGroupVlanGroupDelElement(xpsDevice_t xpsDevId,
                                                      uint16_t bvId, uint32_t groupId)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    xpSaiIpmcGroupVlan2GroupContextDbEntry *entry = NULL;
    xpSaiIpmcGroupVlan2GroupContextDbEntry *newEntry = NULL;
    bool found = false;

    /* Search for corresponding object */
    retVal = xpSaiGetIpmcGroupVlanGroupCtxDb(xpsDevId, bvId, &entry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiGetIpmcGroupVlanGroupCtxDb failed (retVal: %d)\n", retVal);
        return retVal;
    }

    /* Remove the bridge port Id from the table */
    for (uint32_t i = 0; i < entry->numItems; i++)
    {
        if (!found)
        {
            found = (entry->l3mcGroupId[i] == groupId) ? true : false;
            continue;
        }
        else
        {
            entry->l3mcGroupId[i - 1] = entry->l3mcGroupId[i];
        }
    }

    if (!found)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    /* Decrement the numPorts count */
    entry->numItems--;
    if (xpSaiCtxShrinkNeeded(entry->numItems,
                             XP_SAI_DEFAULT_MAX_IPMC_GROUP_MEMBERS))
    {
        /* Shrink the queueList array size */
        XP_STATUS xpRetVal = xpSaiDynamicArrayShrink((void**)&newEntry, (void*)entry,
                                                     sizeof(xpSaiIpmcGroupVlan2GroupContextDbEntry), sizeof(uint32_t),
                                                     entry->numItems, XP_SAI_DEFAULT_MAX_IPMC_GROUP_MEMBERS);
        if (xpRetVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpSaiDynamicArrayShrink failed (retVal: %d)\n", xpRetVal);
            return xpsStatus2SaiStatus(xpRetVal);
        }

        /* Replace the corresponding state */
        retVal = xpSaiIpmcGroupVlanGroupReplaceItem(xpsDevId, bvId, newEntry);
        if (retVal != XP_NO_ERR)
        {
            xpsStateHeapFree((void*) newEntry);
            XP_SAI_LOG_ERR("xpSaiIpmcGroupVlanGroupReplaceItem failed. (retVal: %d)\n",
                           retVal);
            return retVal;
        }
    }

    return retVal;
}

static sai_status_t xpSaiGetIpmcGroupCtxDb(xpsDevice_t xpsDevId,
                                           sai_object_id_t groupIdOid,
                                           xpSaiIpmcGroupContextDbEntry **entry)
{
    xpSaiIpmcGroupContextDbEntry key;

    memset(&key, 0, sizeof(key));
    key.groupIdOid = groupIdOid;

    /* Search for corresponding object */
    return xpSaiGetCtxDb(xpSaiScopeFromDevGet(xpsDevId), xpSaiIpmcGroupStateDbHndl,
                         (void*) &key, (void**) entry);
}

static sai_status_t xpSaiRemoveIpmcGroupCtxDb(xpsDevice_t xpsDevId,
                                              sai_object_id_t groupIdOid)
{
    xpSaiIpmcGroupContextDbEntry key;

    memset(&key, 0, sizeof(key));
    key.groupIdOid = groupIdOid;

    /* Remove the corresponding state */
    return xpSaiRemoveCtxDb(xpSaiScopeFromDevGet(xpsDevId),
                            xpSaiIpmcGroupStateDbHndl, (void*) &key);
}

static sai_status_t xpSaiGetIpmcGroupMemberCtxDb(xpsDevice_t xpsDevId,
                                                 sai_object_id_t groupMemberIdOid,
                                                 xpSaiIpmcGroupMemberContextDbEntry **entry)
{
    xpSaiIpmcGroupMemberContextDbEntry key;

    memset(&key, 0, sizeof(key));
    key.groupMemberIdOid = groupMemberIdOid;

    return xpSaiGetCtxDb(xpSaiScopeFromDevGet(xpsDevId),
                         xpSaiIpmcGroupMemberStateDbHndl,
                         (void*) &key, (void**) entry);
}

static sai_status_t xpSaiRemoveIpmcGroupMemberCtxDb(xpsDevice_t xpsDevId,
                                                    sai_object_id_t groupMemberIdOid)
{
    xpSaiIpmcGroupMemberContextDbEntry key;

    memset(&key, 0, sizeof(key));
    key.groupMemberIdOid = groupMemberIdOid;

    /* Remove the corresponding state */
    return xpSaiRemoveCtxDb(xpSaiScopeFromDevGet(xpsDevId),
                            xpSaiIpmcGroupMemberStateDbHndl, (void*) &key);
}

static sai_status_t xpSaiIpmcGroupReplaceItem(xpsDevice_t xpsDevId,
                                              sai_object_id_t groupIdOid, xpSaiIpmcGroupContextDbEntry *newEntry)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    XP_STATUS xpRetVal = XP_NO_ERR;

    if (newEntry == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument (newEntry: NULL)\n");
        return XP_ERR_INVALID_INPUT;
    }

    /* Remove the corresponding state */
    retVal = xpSaiRemoveIpmcGroupCtxDb(xpsDevId, groupIdOid);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiRemoveIpmcGroupCtxDb failed (retVal: %d)\n", retVal);
        return retVal;
    }

    /* Insert the new state */
    xpRetVal = xpsStateInsertData(xpSaiScopeFromDevGet(xpsDevId),
                                  xpSaiIpmcGroupStateDbHndl, (void*) newEntry);
    if (xpRetVal != XP_NO_ERR)
    {
        xpsStateHeapFree((void*) newEntry);
        XP_SAI_LOG_ERR("Failed to insert data (retVal: %d)\n", xpRetVal);
        return xpsStatus2SaiStatus(xpRetVal);
    }

    return retVal;
}

static sai_status_t xpSaiIpmcGroupAddElement(xpsDevice_t xpsDevId,
                                             sai_object_id_t groupIdOid, sai_object_id_t groupMemberIdOid)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    xpSaiIpmcGroupContextDbEntry *entry = NULL;
    xpSaiIpmcGroupContextDbEntry *newEntry = NULL;

    /* Search for corresponding object */
    retVal = xpSaiGetIpmcGroupCtxDb(xpsDevId, groupIdOid, &entry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiGetIpmcGroupCtxDb failed (retVal: %d)\n", retVal);
        return retVal;
    }

    if (xpSaiCtxGrowthNeeded(entry->numItems,
                             XP_SAI_DEFAULT_MAX_IPMC_GROUP_MEMBERS))
    {
        /* Extend the array size */
        XP_STATUS xpRetVal = xpSaiDynamicArrayGrow((void**)&newEntry, (void*)entry,
                                                   sizeof(xpSaiIpmcGroupContextDbEntry), sizeof(sai_object_id_t),
                                                   entry->numItems, XP_SAI_DEFAULT_MAX_IPMC_GROUP_MEMBERS);
        if (xpRetVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpSaiDynamicArrayGrow failed (retVal: %d)\n", xpRetVal);
            return xpsStatus2SaiStatus(xpRetVal);
        }

        /* Replace the corresponding state */
        retVal = xpSaiIpmcGroupReplaceItem(xpsDevId, groupIdOid, newEntry);
        if (retVal != XP_NO_ERR)
        {
            xpsStateHeapFree((void*) newEntry);
            XP_SAI_LOG_ERR("xpSaiIpmcGroupReplaceItem failed. (retVal: %d)\n", retVal);
            return retVal;
        }

        entry = newEntry;
    }

    entry->groupMemberIdOid[entry->numItems] = groupMemberIdOid;
    entry->numItems++;

    return retVal;
}

static sai_status_t xpSaiIpmcGroupDelElement(xpsDevice_t xpsDevId,
                                             sai_object_id_t groupIdOid, sai_object_id_t groupMemberIdOid)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    xpSaiIpmcGroupContextDbEntry *entry = NULL;
    xpSaiIpmcGroupContextDbEntry *newEntry = NULL;
    bool found = false;

    /* Search for corresponding object */
    retVal = xpSaiGetIpmcGroupCtxDb(xpsDevId, groupIdOid, &entry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiGetIpmcGroupCtxDb failed (retVal: %d)\n", retVal);
        return retVal;
    }

    /* Remove the bridge port Id from the table */
    for (uint32_t i = 0; i < entry->numItems; i++)
    {
        if (!found)
        {
            found = (entry->groupMemberIdOid[i] == groupMemberIdOid) ? true : false;
            continue;
        }
        else
        {
            entry->groupMemberIdOid[i - 1] = entry->groupMemberIdOid[i];
        }
    }

    if (!found)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    /* Decrement the numPorts count */
    entry->numItems--;
    if (xpSaiCtxShrinkNeeded(entry->numItems,
                             XP_SAI_DEFAULT_MAX_IPMC_GROUP_MEMBERS))
    {
        /* Shrink the queueList array size */
        XP_STATUS xpRetVal = xpSaiDynamicArrayShrink((void**)&newEntry, (void*)entry,
                                                     sizeof(xpSaiIpmcGroupContextDbEntry), sizeof(sai_object_id_t),
                                                     entry->numItems, XP_SAI_DEFAULT_MAX_IPMC_GROUP_MEMBERS);
        if (xpRetVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpSaiDynamicArrayShrink failed (retVal: %d)\n", xpRetVal);
            return xpsStatus2SaiStatus(xpRetVal);
        }

        /* Replace the corresponding state */
        retVal = xpSaiIpmcGroupReplaceItem(xpsDevId, groupIdOid, newEntry);
        if (retVal != XP_NO_ERR)
        {
            xpsStateHeapFree((void*) newEntry);
            XP_SAI_LOG_ERR("xpSaiIpmcGroupReplaceItem failed. (retVal: %d)\n", retVal);
            return retVal;
        }
    }

    return retVal;
}

sai_status_t xpSaiCreateIpmcGroup(sai_object_id_t *ipmc_group_id,
                                  sai_object_id_t switch_id,
                                  uint32_t attr_count,
                                  const sai_attribute_t *attr_list)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    XP_STATUS xpRetVal = XP_NO_ERR;
    xpsDevice_t xpsDevId = xpSaiObjIdValueGet(switch_id);
    xpsMcL3InterfaceListId_t ifListId = 0;
    xpSaiIpmcGroupContextDbEntry key;
    xpSaiIpmcGroupContextDbEntry *entry = NULL;

    if (ipmc_group_id == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpSaiAttrCheck(attr_count, attr_list, IPMC_GROUP_VALIDATION_ARRAY_SIZE,
                            ipmc_group_attribs, SAI_COMMON_API_CREATE);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Invalid attribute provided (retVal: %d)\n", retVal);
        return retVal;
    }

    xpRetVal = xpsMulticastCreateL3InterfaceListScope(xpSaiScopeFromDevGet(
                                                          xpsDevId), &ifListId);
    if (xpRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsMulticastCreateL3InterfaceListScope failed (xpRetVal: %d)\n",
                       xpRetVal);
        return xpsStatus2SaiStatus(xpRetVal);
    }

    xpRetVal = xpsMulticastAddL3InterfaceListToDevice(xpsDevId, ifListId);
    if (xpRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsMulticastAddL3InterfaceListToDevice failed (ifListId: %u, xpRetVal: %d)\n",
                       ifListId, xpRetVal);
        return xpsStatus2SaiStatus(xpRetVal);
    }

    if (xpSaiObjIdCreate(SAI_OBJECT_TYPE_IPMC_GROUP, xpsDevId, ifListId,
                         ipmc_group_id) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to create SAI IP Multicast Group object id (intfListId: %u)\n",
                       ifListId);
        return SAI_STATUS_FAILURE;
    }

    memset(&key, 0, sizeof(key));
    key.groupIdOid = *ipmc_group_id;

    retVal = xpSaiInsertCtxDb(xpSaiScopeFromDevGet(xpsDevId),
                              xpSaiIpmcGroupStateDbHndl, (void*) &key, sizeof(key), (void **) &entry);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to insert IP Multicast Group to context DB (groupIdOid: %"
                       PRIu64 ", retVal: %d)\n",
                       key.groupIdOid, retVal);
        return retVal;
    }

    entry->groupIdOid = *ipmc_group_id;

    XP_SAI_LOG_DBG("Group created (groupIdOid: %" PRIu64 ")\n", *ipmc_group_id);

    return retVal;
}

sai_status_t xpSaiRemoveIpmcGroup(sai_object_id_t ipmc_group_id)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    XP_STATUS xpRetVal = XP_NO_ERR;
    xpsMcL3InterfaceListId_t ifListId = xpSaiObjIdValueGet(ipmc_group_id);
    xpsDevice_t xpsDevId = xpSaiObjIdSwitchGet(ipmc_group_id);
    xpSaiIpmcGroupContextDbEntry *entry = NULL;

    if (!XDK_SAI_OBJID_TYPE_CHECK(ipmc_group_id, SAI_OBJECT_TYPE_IPMC_GROUP))
    {
        XP_SAI_LOG_ERR("Wrong object type received (type: %u)\n",
                       xpSaiObjIdTypeGet(ipmc_group_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpSaiGetIpmcGroupCtxDb(xpsDevId, ipmc_group_id, &entry);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Unable to retreive context data for IP Multicast group (groupIdOid: %"
                       PRIu64 ", retVal: %d)\n",
                       ipmc_group_id, retVal);
        return retVal;
    }

    /* for each member remove from the group */
    if (entry->numItems)
    {
        XP_SAI_LOG_ERR("Failed to remove IP Multicast group as it is in use (groupIdOid: %"
                       PRIu64 ")\n", ipmc_group_id);
        return SAI_STATUS_OBJECT_IN_USE;
    }

    xpRetVal = xpsMulticastRemoveL3InterfaceListFromDevice(xpsDevId, ifListId);
    if (xpRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsMulticastRemoveL3InterfaceListFromDevice failed (ifListId: %u, xpRetVal: %d)\n",
                       ifListId, xpRetVal);
        return xpsStatus2SaiStatus(xpRetVal);
    }

    xpRetVal = xpsMulticastDestroyL3InterfaceListScope(xpSaiScopeFromDevGet(
                                                           xpsDevId), ifListId);
    if (xpRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsMulticastDestroyL3InterfaceListScope failed (ifListId: %u, xpRetVal: %d)\n",
                       ifListId, xpRetVal);
        return xpsStatus2SaiStatus(xpRetVal);
    }

    retVal = xpSaiRemoveIpmcGroupCtxDb(xpsDevId, ipmc_group_id);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to remove IP Multicast Group from context DB (groupIdOid: %"
                       PRIu64 ")\n", ipmc_group_id);
        return retVal;
    }

    return retVal;
}

sai_status_t xpSaiSetIpmcGroupAttribute(sai_object_id_t ipmc_group_id,
                                        const sai_attribute_t *attr)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;

    if (attr == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(ipmc_group_id, SAI_OBJECT_TYPE_IPMC_GROUP))
    {
        XP_SAI_LOG_ERR("Wrong object type received (type: %u)\n",
                       xpSaiObjIdTypeGet(ipmc_group_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpSaiAttrCheck(1, attr, IPMC_GROUP_VALIDATION_ARRAY_SIZE,
                            ipmc_group_attribs, SAI_COMMON_API_SET);
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

static sai_status_t xpSaiGetIpmcGroupAttrMemberList(xpSaiIpmcGroupContextDbEntry
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
        pSaiObjList->list[i] = entry.groupMemberIdOid[i];
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiGetIpmcGroupAttribute(sai_object_id_t ipmc_group_id,
                                        uint32_t attr_count,
                                        sai_attribute_t *attr_list)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    xpSaiIpmcGroupContextDbEntry *entry = NULL;
    xpsDevice_t xpsDevId = xpSaiObjIdSwitchGet(ipmc_group_id);

    if (!XDK_SAI_OBJID_TYPE_CHECK(ipmc_group_id, SAI_OBJECT_TYPE_IPMC_GROUP))
    {
        XP_SAI_LOG_ERR("Wrong object type received (type: %u)\n",
                       xpSaiObjIdTypeGet(ipmc_group_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpSaiGetIpmcGroupCtxDb(xpsDevId, ipmc_group_id, &entry);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Unable to retreive context data for IP Multicast group (groupIdOid: %"
                       PRIu64 ", retVal: %d)\n",
                       ipmc_group_id, retVal);
        return retVal;
    }

    for (uint32_t count = 0; count < attr_count; count++)
    {
        switch (attr_list[count].id)
        {
            case SAI_IPMC_GROUP_ATTR_IPMC_OUTPUT_COUNT:
                {
                    attr_list[count].value.u32 = entry->numItems;

                    break;
                }
            case SAI_IPMC_GROUP_ATTR_IPMC_MEMBER_LIST:
                {
                    retVal = xpSaiGetIpmcGroupAttrMemberList(*entry,
                                                             &attr_list[count].value.objlist);
                    if (retVal != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Failed to get member list of IP Multicast Group (groupIdOid: %"
                                       PRIu64 ", retVal: %d)\n",
                                       ipmc_group_id, retVal);
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

void xpSaiUpdateIpmcGroupMemberAttributeVals(const uint32_t attr_count,
                                             const sai_attribute_t *attr_list, xpSaiIpmcGroupMemberAttributesT *attributes)
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
            case SAI_IPMC_GROUP_MEMBER_ATTR_IPMC_GROUP_ID:
                {
                    attributes->groupId = attr_list[count].value.oid;
                    break;
                }
            case SAI_IPMC_GROUP_MEMBER_ATTR_IPMC_OUTPUT_ID:
                {
                    attributes->outputId = attr_list[count].value.oid;
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Failed to set %d\n", attr_list[count].id);
                }
        }
    }
}

sai_status_t xpSaiIpmcGroupOnVlanMemberUpdateCallback(xpsDevice_t xpsDevId,
                                                      uint32_t portId, uint16_t vlanId, bool create)
{
    XP_STATUS xpRetVal = XP_NO_ERR;
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    xpSaiIpmcGroupVlan2GroupContextDbEntry *vlanGroupEntry = NULL;

    retVal = xpSaiGetIpmcGroupVlanGroupCtxDb(xpsDevId, vlanId, &vlanGroupEntry);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        if (retVal == SAI_STATUS_ITEM_NOT_FOUND)
        {
            /* Not member of IPMC group */
            return SAI_STATUS_SUCCESS;
        }
        else
        {
            XP_SAI_LOG_ERR("Unable to retreive context data for IP Multicast group vlan group (bvId: %u, retVal: %d)\n",
                           vlanId, retVal);
            return retVal;
        }
    }

    if (create)
    {
        xpRetVal = xpsMulticastAddInterfaceToL2InterfaceList(xpsDevId,
                                                             vlanGroupEntry->l2mcGroupId, portId);
        if (xpRetVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to add interface to L2 Interface list on hardware (groupId: %u, intfId: %u, xpRetVal: %d)\n",
                           vlanGroupEntry->l2mcGroupId, portId, xpRetVal);
            return xpsStatus2SaiStatus(xpRetVal);
        }
    }
    else
    {
        xpRetVal = xpsMulticastRemoveInterfaceFromL2InterfaceList(xpsDevId,
                                                                  vlanGroupEntry->l2mcGroupId, portId);
        if (xpRetVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to remove interface from L2 Interface list on hardware (groupId: %u, intfId: %u, xpRetVal: %d)\n",
                           vlanGroupEntry->l2mcGroupId, portId, xpRetVal);
            return xpsStatus2SaiStatus(xpRetVal);
        }
    }

    return retVal;
}

sai_status_t xpSaiCreateIpmcGroupMember(sai_object_id_t *ipmc_group_member_id,
                                        sai_object_id_t switch_id,
                                        uint32_t attr_count,
                                        const sai_attribute_t *attr_list)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    XP_STATUS xpRetVal = XP_NO_ERR;
    xpsMcL3InterfaceListId_t ifListId;
    xpsMcL2InterfaceListId_t ifL2ListId = 0;
    xpsDevice_t xpsDevId = xpSaiObjIdSwitchGet(switch_id);
    xpSaiIpmcGroupMemberAttributesT attributes;
    xpSaiIpmcGroupMemberContextDbEntry *entry = NULL;
    xpSaiIpmcGroupMemberContextDbEntry key;
    xpSaiIpmcGroupContextDbEntry *groupEntry = NULL;
    xpSaiRouterInterfaceDbEntryT *pRouterInterfaceEntry = NULL;
    uint16_t vlanId;
    uint32_t id;

    if (ipmc_group_member_id == NULL)
    {
        XP_SAI_LOG_ERR("Invalid input parameter\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpSaiAttrCheck(attr_count, attr_list,
                            IPMC_GROUP_MEMBER_VALIDATION_ARRAY_SIZE,
                            ipmc_group_member_attribs, SAI_COMMON_API_CREATE);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Invalid attribute provided (retVal: %d)\n", retVal);
        return retVal;
    }

    xpSaiUpdateIpmcGroupMemberAttributeVals(attr_count, attr_list, &attributes);

    if (!XDK_SAI_OBJID_TYPE_CHECK(attributes.groupId, SAI_OBJECT_TYPE_IPMC_GROUP))
    {
        XP_SAI_LOG_ERR("Wrong object type received (type: %u)\n",
                       xpSaiObjIdTypeGet(attributes.groupId));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(attributes.outputId,
                                  SAI_OBJECT_TYPE_ROUTER_INTERFACE)
        && !XDK_SAI_OBJID_TYPE_CHECK(attributes.outputId, SAI_OBJECT_TYPE_TUNNEL))
    {
        XP_SAI_LOG_ERR("Wrong object type received (type: %u)\n",
                       xpSaiObjIdTypeGet(attributes.outputId));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpSaiGetIpmcGroupCtxDb(xpsDevId, attributes.groupId, &groupEntry);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        if (retVal == SAI_STATUS_ITEM_NOT_FOUND)
        {
            XP_SAI_LOG_ERR("IP Multicast group does not exist (groupIdOid: %" PRIu64
                           ", retVal: %d)\n",
                           attributes.groupId, retVal);
        }
        else
        {
            XP_SAI_LOG_ERR("Unable to retreive context data for IP Multicast group (groupIdOid: %"
                           PRIu64 ", retVal: %d)\n",
                           attributes.groupId, retVal);
        }
        return retVal;
    }

    if (XDK_SAI_OBJID_TYPE_CHECK(attributes.outputId, SAI_OBJECT_TYPE_TUNNEL))
    {
        /* TODO SAI_OBJECT_TYPE_TUNNEL */
        return SAI_STATUS_NOT_IMPLEMENTED;
    }

    ifListId = xpSaiObjIdValueGet(attributes.groupId);

    retVal = xpSaiRouterInterfaceDbInfoGet(attributes.outputId,
                                           &pRouterInterfaceEntry);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiRouterInterfaceDbInfoGet() failed (retVal: %d)\n", retVal);
        return retVal;
    }

    if (groupEntry->numItems == 0)
    {
        groupEntry->ipmcRifType = pRouterInterfaceEntry->rifType;
    }
    else if (pRouterInterfaceEntry->rifType != groupEntry->ipmcRifType)
    {
        XP_SAI_LOG_ERR("IPMC Group do not support RIFs with mixed types (groupRifType: %u, rifType: %u)\n",
                       groupEntry->ipmcRifType, pRouterInterfaceEntry->rifType);
        return SAI_STATUS_NOT_SUPPORTED;
    }

    switch (pRouterInterfaceEntry->rifType)
    {
        case SAI_ROUTER_INTERFACE_TYPE_PORT:
            {
                xpRetVal = xpsMulticastAddInterfaceToL3InterfaceList(xpSaiScopeFromDevGet(
                                                                         xpsDevId), ifListId, pRouterInterfaceEntry->l3IntfId);
                if (xpRetVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("xpsMulticastAddInterfaceToL3InterfaceList failed (ifListId: %u, l3IntfId: %u, xpRetVal: %d)\n",
                                   ifListId, pRouterInterfaceEntry->l3IntfId, xpRetVal);
                    return xpsStatus2SaiStatus(xpRetVal);
                }

                break;
            }
        case SAI_ROUTER_INTERFACE_TYPE_VLAN:
            {
                xpSaiIpmcGroupVlan2GroupContextDbEntry *vlanGroupEntry = NULL;

                vlanId = xpSaiObjIdValueGet(pRouterInterfaceEntry->vlanOid);
                xpsInterfaceId_t *intfList  = NULL;
                uint16_t numOfIntfs = 0;
                bool found = true;

                retVal = xpSaiGetIpmcGroupVlanGroupCtxDb(xpsDevId, vlanId, &vlanGroupEntry);
                if (retVal != SAI_STATUS_SUCCESS)
                {
                    if (retVal == SAI_STATUS_ITEM_NOT_FOUND)
                    {
                        found = false;
                    }
                    else
                    {
                        XP_SAI_LOG_ERR("Unable to retreive context data for IP Multicast group vlan group (bvId: %u, retVal: %d)\n",
                                       vlanId, retVal);
                        return retVal;
                    }
                }

                if (!found)
                {
                    retVal = xpSaiCreateL2McGroupOnHardware(xpsDevId, vlanId, 0xFFFF, &ifL2ListId);
                    if (retVal != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("xpSaiCreateL2McGroupOnHardware failed (ifL2ListId: %u, bvId: %u, retVal: %d)\n",
                                       ifL2ListId, vlanId, retVal);
                        return retVal;
                    }

                    xpRetVal = xpsVlanGetInterfaceList(xpsDevId, vlanId, &intfList, &numOfIntfs);
                    if (xpRetVal != XP_NO_ERR)
                    {
                        XP_SAI_LOG_ERR("xpsVlanGetInterfaceList failed (bvId: %u, xpRetVal: %d)\n",
                                       vlanId, xpRetVal);
                        return xpsStatus2SaiStatus(xpRetVal);
                    }

                    for (uint16_t portIndx = 0; portIndx < numOfIntfs; portIndx++)
                    {
                        XP_SAI_LOG_DBG("Adding interface to group (ifL2ListId: %u, intfId: %u)\n",
                                       ifL2ListId, intfList[portIndx]);

                        xpRetVal = xpsMulticastAddInterfaceToL2InterfaceList(xpsDevId, ifL2ListId,
                                                                             intfList[portIndx]);
                        if (xpRetVal != XP_NO_ERR)
                        {
                            XP_SAI_LOG_ERR("Failed to add interface to L2 Interface list on hardware (groupId: %u, intfId: %u, xpRetVal: %d)\n",
                                           ifL2ListId, intfList[portIndx], xpRetVal);
                            return xpsStatus2SaiStatus(xpRetVal);
                        }
                    }
                }
                else
                {
                    ifL2ListId = vlanGroupEntry->l2mcGroupId;
                }

                xpRetVal = xpsMulticastAddBridgingInterfaceToL3InterfaceList(xpsDevId, ifListId,
                                                                             ifL2ListId);
                if (xpRetVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("Failed to add bridging interface to L3 Interface list on hardware (ifListId: %u, ifL2ListId: %u, xpRetVal: %d)\n",
                                   ifListId, ifL2ListId, xpRetVal);
                    return xpsStatus2SaiStatus(xpRetVal);
                }

                if (!found)
                {
                    xpSaiIpmcGroupVlan2GroupContextDbEntry vlanGroupKey;

                    memset(&vlanGroupKey, 0, sizeof(vlanGroupKey));

                    vlanGroupKey.bvId = vlanId;

                    retVal = xpSaiInsertCtxDb(xpSaiScopeFromDevGet(xpsDevId),
                                              xpSaiIpmcGroupVlanGroupStateDbHndl, (void*) &vlanGroupKey,
                                              sizeof(vlanGroupKey), (void **) &vlanGroupEntry);
                    if (retVal != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Failed to insert IP Multicast Group Vlan Group to context DB (bvId: %u)\n",
                                       vlanGroupKey.bvId);
                        return retVal;
                    }

                    vlanGroupEntry->bvId = vlanId;
                    vlanGroupEntry->l2mcGroupId = ifL2ListId;
                }

                retVal = xpSaiIpmcGroupVlanGroupAddElement(xpsDevId, vlanId, ifListId);
                if (retVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to add interface to IP Multicast Group list (groupId: %u, bvId: %u, retVal: %d)\n",
                                   ifListId, vlanId, retVal);
                    return retVal;
                }

                break;
            }
        default:
            {
                return SAI_STATUS_NOT_IMPLEMENTED;
            }
    }

    /* allocate unique id for IP Multicast Group Member */
    xpRetVal = xpsAllocatorAllocateId(xpSaiScopeFromDevGet(xpsDevId),
                                      XP_SAI_ALLOC_IPMC_GRP_MEMBER, &id);
    if (xpRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Unable to allocate IP Multicast Group Member id (retVal: %d)\n",
                       xpRetVal);
        return xpsStatus2SaiStatus(xpRetVal);
    }

    /* generate OID based on unique XPS id */
    if (xpSaiObjIdCreate(SAI_OBJECT_TYPE_IPMC_GROUP_MEMBER, xpsDevId,
                         (sai_uint64_t) id, ipmc_group_member_id) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to create SAI IP Multicast Group Member object id (id: %u)\n",
                       id);
        return SAI_STATUS_FAILURE;
    }

    memset(&key, 0, sizeof(key));
    key.groupMemberIdOid = *ipmc_group_member_id;

    retVal = xpSaiInsertCtxDb(xpSaiScopeFromDevGet(xpsDevId),
                              xpSaiIpmcGroupMemberStateDbHndl, (void*) &key, sizeof(key), (void **) &entry);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to insert IP Multicast Group member to context DB (groupMemberIdOid: %"
                       PRIu64 ")\n",
                       key.groupMemberIdOid);
        return retVal;
    }

    entry->groupMemberIdOid = key.groupMemberIdOid;
    entry->groupIdOid = attributes.groupId;
    entry->outputIdOid = attributes.outputId;

    retVal = xpSaiIpmcGroupAddElement(xpsDevId, entry->groupIdOid,
                                      entry->groupMemberIdOid);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to add interface to IP Multicast Group list (groupIdOid: %"
                       PRIu64 ", groupMemberIdOid: %" PRIu64 ", retVal: %d)\n",
                       entry->groupIdOid, entry->groupMemberIdOid, retVal);
        return retVal;
    }

    return retVal;
}

sai_status_t xpSaiRemoveIpmcGroupMember(sai_object_id_t ipmc_group_member_id)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    XP_STATUS xpRetVal = XP_NO_ERR;
    xpSaiIpmcGroupMemberContextDbEntry *entry = NULL;
    xpSaiRouterInterfaceDbEntryT *pRouterInterfaceEntry = NULL;
    xpsDevice_t xpsDevId = xpSaiObjIdSwitchGet(ipmc_group_member_id);
    xpsMcL3InterfaceListId_t ifListId = 0;

    if (!XDK_SAI_OBJID_TYPE_CHECK(ipmc_group_member_id,
                                  SAI_OBJECT_TYPE_IPMC_GROUP_MEMBER))
    {
        XP_SAI_LOG_ERR("Wrong object type received (type: %u)\n",
                       xpSaiObjIdTypeGet(ipmc_group_member_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpSaiGetIpmcGroupMemberCtxDb(xpsDevId, ipmc_group_member_id, &entry);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Unable to retreive context data for IP Multicast group member (groupMemberIdOid: %"
                       PRIu64 ", retVal: %d)\n",
                       ipmc_group_member_id, retVal);
        return retVal;
    }

    ifListId = xpSaiObjIdValueGet(entry->groupIdOid);

    retVal = xpSaiRouterInterfaceDbInfoGet(entry->outputIdOid,
                                           &pRouterInterfaceEntry);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiRouterInterfaceDbInfoGet() failed (retVal: %d)\n", retVal);
        return retVal;
    }

    switch (pRouterInterfaceEntry->rifType)
    {
        case SAI_ROUTER_INTERFACE_TYPE_PORT:
            {
                xpRetVal = xpsMulticastRemoveInterfaceFromL3InterfaceList(xpsDevId, ifListId,
                                                                          pRouterInterfaceEntry->l3IntfId);
                if (xpRetVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("xpsMulticastRemoveInterfaceFromL3InterfaceList failed (ifListId: %u, l3IntfId: %u, xpRetVal: %d)\n",
                                   ifListId, pRouterInterfaceEntry->l3IntfId, xpRetVal);
                    return xpsStatus2SaiStatus(xpRetVal);
                }

                break;
            }
        case SAI_ROUTER_INTERFACE_TYPE_VLAN:
            {
                xpSaiIpmcGroupVlan2GroupContextDbEntry *vlanGroupEntry = NULL;

                uint16_t vlanId = xpSaiObjIdValueGet(pRouterInterfaceEntry->vlanOid);
                uint16_t ifL2ListId;
                xpsInterfaceId_t *intfList  = NULL;
                uint16_t numOfIntfs = 0;

                retVal = xpSaiGetIpmcGroupVlanGroupCtxDb(xpsDevId, vlanId, &vlanGroupEntry);
                if (retVal != SAI_STATUS_SUCCESS)
                {
                    if (retVal == SAI_STATUS_ITEM_NOT_FOUND)
                    {
                        break;
                    }
                    else
                    {
                        XP_SAI_LOG_ERR("Unable to retreive context data for IP Multicast group vlan group (bvId: %u, retVal: %d)\n",
                                       vlanId, retVal);
                        return retVal;
                    }
                }

                ifL2ListId = vlanGroupEntry->l2mcGroupId;

                xpRetVal = xpsMulticastRemoveBridgingInterfaceFromL3InterfaceList(xpsDevId,
                                                                                  ifListId, ifL2ListId);
                if (xpRetVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("Failed to remove bridging interface from L3 Interface list on hardware (ifListId: %u, ifL2ListId: %u, xpRetVal: %d)\n",
                                   ifListId, ifL2ListId, xpRetVal);
                    return xpsStatus2SaiStatus(xpRetVal);
                }

                // last item, remove hardware configuration
                if (vlanGroupEntry->numItems == 1)
                {
                    xpRetVal = xpsVlanGetInterfaceList(xpsDevId, vlanId, &intfList, &numOfIntfs);
                    if (xpRetVal != XP_NO_ERR)
                    {
                        XP_SAI_LOG_ERR("xpsVlanGetInterfaceList failed (bvId: %u, xpRetVal: %d)\n",
                                       vlanId, xpRetVal);
                        return xpsStatus2SaiStatus(xpRetVal);
                    }

                    for (uint16_t portIndx = 0; portIndx < numOfIntfs; portIndx++)
                    {
                        XP_SAI_LOG_DBG("Removing interface from group (ifL2ListId: %u, intfId: %u)\n",
                                       ifL2ListId, intfList[portIndx]);

                        xpRetVal = xpsMulticastRemoveInterfaceFromL2InterfaceList(xpsDevId, ifL2ListId,
                                                                                  intfList[portIndx]);
                        if (xpRetVal != XP_NO_ERR)
                        {
                            XP_SAI_LOG_ERR("Failed to add interface to L2 Interface list on hardware (groupId: %u, intfId: %u, xpRetVal: %d)\n",
                                           ifL2ListId, intfList[portIndx], xpRetVal);
                            return xpsStatus2SaiStatus(xpRetVal);
                        }
                    }

                    retVal = xpSaiRemoveL2McGroupFromHardware(xpsDevId, ifL2ListId);
                    if (retVal != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("xpSaiRemoveL2McGroupFromHardware failed (ifL2ListId: %u, retVal: %d)\n",
                                       ifL2ListId, retVal);
                        return retVal;
                    }

                    retVal = xpSaiRemoveIpmcGroupVlanGroupCtxDb(xpsDevId, vlanId);
                    if (retVal != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Failed to remove IP Multicast Group Vlan Group from context DB (bvId: %u)\n",
                                       vlanId);
                        return retVal;
                    }

                    break;
                }

                retVal = xpSaiIpmcGroupVlanGroupDelElement(xpsDevId, vlanGroupEntry->bvId,
                                                           ifListId);
                if (retVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to remove interface from IP Multicast Group list (groupId: %u, bvId: %u, retVal: %d)\n",
                                   ifListId, vlanGroupEntry->bvId, retVal);
                    return retVal;
                }

                break;
            }
        default:
            {
                return SAI_STATUS_NOT_IMPLEMENTED;
            }
    }

    xpRetVal = xpsAllocatorReleaseId(xpSaiScopeFromDevGet(xpsDevId),
                                     XP_SAI_ALLOC_IPMC_GRP_MEMBER, xpSaiObjIdValueGet(ipmc_group_member_id));
    if (xpRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Unable to release IpmcGroupMember Id (retVal: %d)\n", xpRetVal);
        return xpsStatus2SaiStatus(xpRetVal);
    }

    retVal = xpSaiIpmcGroupDelElement(xpsDevId, entry->groupIdOid,
                                      entry->groupMemberIdOid);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to remove interface from IP Multicast Group list (groupIdOid: %"
                       PRIu64 ", groupMemberIdOid: %" PRIu64 ", retVal: %d)\n",
                       entry->groupIdOid, entry->groupMemberIdOid, retVal);
        return retVal;
    }

    retVal = xpSaiRemoveIpmcGroupMemberCtxDb(xpsDevId, ipmc_group_member_id);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to remove IP Multicast Group member context DB (groupMemberIdOid: %"
                       PRIu64 ")\n", ipmc_group_member_id);
        return retVal;
    }

    return retVal;
}

sai_status_t xpSaiSetIpmcGroupMemberAttribute(sai_object_id_t
                                              ipmc_group_member_id,
                                              const sai_attribute_t *attr)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;

    if (attr == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(ipmc_group_member_id,
                                  SAI_OBJECT_TYPE_IPMC_GROUP_MEMBER))
    {
        XP_SAI_LOG_ERR("Wrong object type received (type: %u)\n",
                       xpSaiObjIdTypeGet(ipmc_group_member_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpSaiAttrCheck(1, attr, IPMC_GROUP_MEMBER_VALIDATION_ARRAY_SIZE,
                            ipmc_group_member_attribs, SAI_COMMON_API_SET);
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

sai_status_t xpSaiGetIpmcGroupMemberAttribute(sai_object_id_t
                                              ipmc_group_member_id,
                                              uint32_t attr_count,
                                              sai_attribute_t *attr_list)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    xpsDevice_t xpsDevId = xpSaiObjIdSwitchGet(ipmc_group_member_id);
    xpSaiIpmcGroupMemberContextDbEntry *entry = NULL;

    if (!XDK_SAI_OBJID_TYPE_CHECK(ipmc_group_member_id,
                                  SAI_OBJECT_TYPE_IPMC_GROUP_MEMBER))
    {
        XP_SAI_LOG_ERR("Wrong object type received (type: %u)\n",
                       xpSaiObjIdTypeGet(ipmc_group_member_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpSaiAttrCheck(attr_count, attr_list,
                            IPMC_GROUP_MEMBER_VALIDATION_ARRAY_SIZE,
                            ipmc_group_member_attribs, SAI_COMMON_API_GET);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Invalid attribute provided (retVal: %d)\n", retVal);
        return retVal;
    }

    retVal = xpSaiGetIpmcGroupMemberCtxDb(xpsDevId, ipmc_group_member_id, &entry);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Unable to retreive context data for IP Multicast group member (groupMemberIdOid: %"
                       PRIu64 ", retVal: %d)\n",
                       ipmc_group_member_id, retVal);
        return retVal;
    }

    for (uint32_t count = 0; count < attr_count; count++)
    {
        switch (attr_list[count].id)
        {
            case SAI_IPMC_GROUP_MEMBER_ATTR_IPMC_GROUP_ID:
                {
                    attr_list[count].value.oid = entry->groupIdOid;
                    break;
                }
            case SAI_IPMC_GROUP_MEMBER_ATTR_IPMC_OUTPUT_ID:
                {
                    attr_list[count].value.oid = entry->outputIdOid;
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

sai_status_t xpSaiIpmcGroupApiInit(uint64_t flag,
                                   const sai_service_method_table_t* adapHostServiceMethodTable)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;

    _xpSaiIpmcGroupApi = (sai_ipmc_group_api_t *) xpMalloc(sizeof(
                                                               sai_ipmc_group_api_t));

    if (_xpSaiIpmcGroupApi == NULL)
    {
        XP_SAI_LOG_ERR("Failed to allocate _xpSaiIpmcGroupApi\n");
        return SAI_STATUS_NO_MEMORY;
    }

    _xpSaiIpmcGroupApi->create_ipmc_group = xpSaiCreateIpmcGroup;
    _xpSaiIpmcGroupApi->remove_ipmc_group = xpSaiRemoveIpmcGroup;
    _xpSaiIpmcGroupApi->set_ipmc_group_attribute = xpSaiSetIpmcGroupAttribute;
    _xpSaiIpmcGroupApi->get_ipmc_group_attribute = xpSaiGetIpmcGroupAttribute;
    _xpSaiIpmcGroupApi->create_ipmc_group_member = xpSaiCreateIpmcGroupMember;
    _xpSaiIpmcGroupApi->remove_ipmc_group_member = xpSaiRemoveIpmcGroupMember;
    _xpSaiIpmcGroupApi->set_ipmc_group_member_attribute =
        xpSaiSetIpmcGroupMemberAttribute;
    _xpSaiIpmcGroupApi->get_ipmc_group_member_attribute =
        xpSaiGetIpmcGroupMemberAttribute;

    retVal = xpSaiApiRegister(SAI_API_IPMC_GROUP, (void*)_xpSaiIpmcGroupApi);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to register IPMC GROUP API\n");
        return retVal;
    }

    return retVal;
}

sai_status_t xpSaiIpmcGroupApiDeinit()
{
    xpFree(_xpSaiIpmcGroupApi);
    _xpSaiIpmcGroupApi = NULL;

    return SAI_STATUS_SUCCESS;
}

/*This is called in switch init*/
sai_status_t xpSaiIpmcGroupInit(xpsDevice_t xpsDevId)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpsScope_t scope = xpSaiScopeFromDevGet(xpsDevId);

    /* Create global SAI IPMC GROUP DB */
    xpSaiIpmcGroupStateDbHndl = XPSAI_IPMC_GROUP_STATE_DB_HNDL;
    if ((retVal = xpsStateRegisterDb(scope, "SAI IPMC GROUP DB", XPS_GLOBAL,
                                     &saiIpmcGroupCtxKeyComp,
                                     xpSaiIpmcGroupStateDbHndl)) != XP_NO_ERR)
    {
        xpSaiIpmcGroupStateDbHndl = XPS_STATE_INVALID_DB_HANDLE;
        XP_SAI_LOG_ERR("Could not create SAI IPMC GROUP DB\n");
        return xpsStatus2SaiStatus(retVal);
    }

    /* Create global SAI IPMC GROUP PORT DB */
    xpSaiIpmcGroupMemberStateDbHndl = XPSAI_IPMC_GROUP_PORT_STATE_DB_HNDL;
    if ((retVal = xpsStateRegisterDb(scope, "SAI IPMC GROUP PORT DB", XPS_GLOBAL,
                                     &saiIpmcGroupMemberCtxKeyComp,
                                     xpSaiIpmcGroupMemberStateDbHndl)) != XP_NO_ERR)
    {
        xpSaiIpmcGroupMemberStateDbHndl = XPS_STATE_INVALID_DB_HANDLE;
        XP_SAI_LOG_ERR("Could not create SAI IPMC GROUP PORT DB\n");
        return xpsStatus2SaiStatus(retVal);
    }

    /* Create global SAI IPMC GROUP VLAN GROUP DB */
    xpSaiIpmcGroupVlanGroupStateDbHndl = XPSAI_IPMC_GROUP_VLAN_GROUP_STATE_DB_HNDL;
    if ((retVal = xpsStateRegisterDb(scope, "SAI IPMC GROUP VLAN GROUP DB",
                                     XPS_GLOBAL, &saiIpmcGroupVlanGroupCtxKeyComp,
                                     xpSaiIpmcGroupVlanGroupStateDbHndl)) != XP_NO_ERR)
    {
        xpSaiIpmcGroupVlanGroupStateDbHndl = XPS_STATE_INVALID_DB_HANDLE;
        XP_SAI_LOG_ERR("Could not create SAI IPMC GROUP VLAN GROUP DB\n");
        return xpsStatus2SaiStatus(retVal);
    }

    retVal = xpsAllocatorInitIdAllocator(scope, XP_SAI_ALLOC_IPMC_GRP,
                                         XP_SAI_IPMC_GRP_MAX_IDS, XP_SAI_IPMC_GRP_RANGE_START);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to initialize SAI IPMC GROUP ID allocator\n");
        return xpsStatus2SaiStatus(retVal);
    }

    retVal = xpsAllocatorInitIdAllocator(scope, XP_SAI_ALLOC_IPMC_GRP_MEMBER,
                                         XP_SAI_IPMC_GRP_MEMBER_MAX_IDS, XP_SAI_IPMC_GRP_MEMBER_RANGE_START);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to initialize SAI IPMC GROUP ID allocator\n");
        return xpsStatus2SaiStatus(retVal);
    }

    return xpsStatus2SaiStatus(retVal);
}

sai_status_t xpSaiIpmcGroupDeInit(xpsDevice_t xpsDevId)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpsScope_t scope = xpSaiScopeFromDevGet(xpsDevId);

    /* Purge global SAI IPMC GROUP DB */
    if ((retVal = xpsStateDeRegisterDb(scope,
                                       &xpSaiIpmcGroupStateDbHndl)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not purge SAI IPMC GROUP DB\n");
        return xpsStatus2SaiStatus(retVal);
    }

    /* Purge global SAI IPMC GROUP PORT DB */
    if ((retVal = xpsStateDeRegisterDb(scope,
                                       &xpSaiIpmcGroupMemberStateDbHndl)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not purge SAI IPMC GROUP PORT DB\n");
        return xpsStatus2SaiStatus(retVal);
    }

    /* Purge global SAI IPMC GROUP VLAN GROUP DB */
    if ((retVal = xpsStateDeRegisterDb(scope,
                                       &xpSaiIpmcGroupVlanGroupStateDbHndl)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not purge SAI IPMC GROUP VLAN GROUP DB\n");
        return xpsStatus2SaiStatus(retVal);
    }

    return xpsStatus2SaiStatus(retVal);
}
