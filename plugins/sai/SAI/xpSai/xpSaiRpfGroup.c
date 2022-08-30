// xpSaiRpfGroup.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpSaiRpfGroup.h"
#include "xpsMulticast.h"
#include "xpSaiSwitch.h"
#include "xpSaiValidationArrays.h"

XP_SAI_LOG_REGISTER_API(SAI_API_RPF_GROUP);

static sai_rpf_group_api_t* _xpSaiRpfGroupApi;

#define XP_SAI_RPF_GRP_MAX_IDS             48 * 1024 // 48K RPF groups
#define XP_SAI_RPF_GRP_RANGE_START         0
#define XP_SAI_RPF_GRP_MEMBER_MAX_IDS      XP_SAI_RPF_GRP_MAX_IDS * 128
#define XP_SAI_RPF_GRP_MEMBER_RANGE_START  0

static xpsDbHandle_t xpSaiRpfGroupStateDbHndl = XPS_STATE_INVALID_DB_HANDLE;
static xpsDbHandle_t xpSaiRpfGroupMcEntryHwIdxDbHndl =
    XPS_STATE_INVALID_DB_HANDLE;
static xpsDbHandle_t xpSaiRpfGroupMemberStateDbHndl =
    XPS_STATE_INVALID_DB_HANDLE;

static int32_t saiRpfGroupCtxKeyComp(void* key1, void* key2)
{
    return ((((xpSaiRpfGroupContextDbEntry *) key1)->groupIdOid) - (((
                                                                         xpSaiRpfGroupContextDbEntry *) key2)->groupIdOid));
}

static int32_t saiRpfGroupMcEntryHwIdxKeyComp(void* key1, void* key2)
{
    return ((((xpSaiRpfGroupMcEntryHwIdxDbEntry *) key1)->groupIdOid) - (((
                                                                              xpSaiRpfGroupMcEntryHwIdxDbEntry *) key2)->groupIdOid));
}

static int32_t saiRpfGroupMemberCtxKeyComp(void* key1, void* key2)
{
    return ((((xpSaiRpfGroupMemberContextDbEntry *) key1)->groupMemberIdOid) - (((
            xpSaiRpfGroupMemberContextDbEntry *) key2)->groupMemberIdOid));
}

static sai_status_t xpSaiGetRpfGroupCtxDb(xpsDevice_t xpsDevId,
                                          sai_object_id_t groupIdOid,
                                          xpSaiRpfGroupContextDbEntry **entry)
{
    xpSaiRpfGroupContextDbEntry key;

    memset(&key, 0, sizeof(key));
    key.groupIdOid = groupIdOid;

    /* Search for corresponding object */
    return xpSaiGetCtxDb(xpSaiScopeFromDevGet(xpsDevId), xpSaiRpfGroupStateDbHndl,
                         (void*) &key, (void**) entry);
}

static sai_status_t xpSaiRemoveRpfGroupCtxDb(xpsDevice_t xpsDevId,
                                             sai_object_id_t groupIdOid)
{
    xpSaiRpfGroupContextDbEntry key;

    memset(&key, 0, sizeof(key));
    key.groupIdOid = groupIdOid;

    /* Remove the corresponding state */
    return xpSaiRemoveCtxDb(xpSaiScopeFromDevGet(xpsDevId),
                            xpSaiRpfGroupStateDbHndl, (void*) &key);
}

static sai_status_t xpSaiGetRpfGroupMcEntryHwIdxDb(xpsDevice_t xpsDevId,
                                                   sai_object_id_t groupIdOid,
                                                   xpSaiRpfGroupMcEntryHwIdxDbEntry **entry)
{
    xpSaiRpfGroupMcEntryHwIdxDbEntry key;

    memset(&key, 0, sizeof(key));
    key.groupIdOid = groupIdOid;

    /* Search for corresponding object */
    return xpSaiGetCtxDb(xpSaiScopeFromDevGet(xpsDevId),
                         xpSaiRpfGroupMcEntryHwIdxDbHndl, (void*) &key, (void**) entry);
}

static sai_status_t xpSaiRemoveRpfGroupMcEntryHwIdxDb(xpsDevice_t xpsDevId,
                                                      sai_object_id_t groupIdOid)
{
    xpSaiRpfGroupMcEntryHwIdxDbEntry key;

    memset(&key, 0, sizeof(key));
    key.groupIdOid = groupIdOid;

    /* Remove the corresponding state */
    return xpSaiRemoveCtxDb(xpSaiScopeFromDevGet(xpsDevId),
                            xpSaiRpfGroupMcEntryHwIdxDbHndl, (void*) &key);
}

static sai_status_t xpSaiGetRpfGroupMemberCtxDb(xpsDevice_t xpsDevId,
                                                sai_object_id_t groupMemberIdOid,
                                                xpSaiRpfGroupMemberContextDbEntry **entry)
{
    xpSaiRpfGroupMemberContextDbEntry key;

    memset(&key, 0, sizeof(key));
    key.groupMemberIdOid = groupMemberIdOid;

    return xpSaiGetCtxDb(xpSaiScopeFromDevGet(xpsDevId),
                         xpSaiRpfGroupMemberStateDbHndl,
                         (void*) &key, (void**) entry);
}

static sai_status_t xpSaiRemoveRpfGroupMemberCtxDb(xpsDevice_t xpsDevId,
                                                   sai_object_id_t groupMemberIdOid)
{
    xpSaiRpfGroupMemberContextDbEntry key;

    memset(&key, 0, sizeof(key));
    key.groupMemberIdOid = groupMemberIdOid;

    /* Remove the corresponding state */
    return xpSaiRemoveCtxDb(xpSaiScopeFromDevGet(xpsDevId),
                            xpSaiRpfGroupMemberStateDbHndl, (void*) &key);
}

static sai_status_t xpSaiRpfGroupReplaceItem(xpsDevice_t xpsDevId,
                                             sai_object_id_t groupIdOid, xpSaiRpfGroupContextDbEntry *newEntry)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    XP_STATUS xpRetVal = XP_NO_ERR;

    if (newEntry == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument (newEntry: NULL)\n");
        return XP_ERR_INVALID_INPUT;
    }

    /* Remove the corresponding state */
    retVal = xpSaiRemoveRpfGroupCtxDb(xpsDevId, groupIdOid);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiRemoveRpfGroupCtxDb failed (retVal: %d)\n", retVal);
        return retVal;
    }

    /* Insert the new state */
    xpRetVal = xpsStateInsertData(xpSaiScopeFromDevGet(xpsDevId),
                                  xpSaiRpfGroupStateDbHndl, (void*) newEntry);
    if (xpRetVal != XP_NO_ERR)
    {
        xpsStateHeapFree((void*) newEntry);
        XP_SAI_LOG_ERR("Failed to insert data (retVal: %d)\n", xpRetVal);
        return xpsStatus2SaiStatus(xpRetVal);
    }

    return retVal;
}

static sai_status_t xpSaiRpfGroupAddElement(xpsDevice_t xpsDevId,
                                            sai_object_id_t groupIdOid, sai_object_id_t groupMemberIdOid)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    xpSaiRpfGroupContextDbEntry *entry = NULL;
    xpSaiRpfGroupContextDbEntry *newEntry = NULL;

    /* Search for corresponding object */
    retVal = xpSaiGetRpfGroupCtxDb(xpsDevId, groupIdOid, &entry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiGetRpfGroupCtxDb failed (retVal: %d)\n", retVal);
        return retVal;
    }

    if (xpSaiCtxGrowthNeeded(entry->numItems, XP_SAI_DEFAULT_MAX_RPF_GROUP_MEMBERS))
    {
        /* Extend the array size */
        XP_STATUS xpRetVal = xpSaiDynamicArrayGrow((void**)&newEntry, (void*)entry,
                                                   sizeof(xpSaiRpfGroupContextDbEntry), sizeof(sai_object_id_t),
                                                   entry->numItems, XP_SAI_DEFAULT_MAX_RPF_GROUP_MEMBERS);
        if (xpRetVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpSaiDynamicArrayGrow failed (retVal: %d)\n", xpRetVal);
            return xpsStatus2SaiStatus(xpRetVal);
        }

        /* Replace the corresponding state */
        retVal = xpSaiRpfGroupReplaceItem(xpsDevId, groupIdOid, newEntry);
        if (retVal != XP_NO_ERR)
        {
            xpsStateHeapFree((void*) newEntry);
            XP_SAI_LOG_ERR("xpSaiRpfGroupReplaceItem failed. (retVal: %d)\n", retVal);
            return retVal;
        }

        entry = newEntry;
    }

    entry->rpfGroupMemberIdOid[entry->numItems] = groupMemberIdOid;
    entry->numItems++;

    return retVal;
}

static sai_status_t xpSaiRpfGroupDelElement(xpsDevice_t xpsDevId,
                                            sai_object_id_t groupIdOid, sai_object_id_t groupMemberIdOid)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    xpSaiRpfGroupContextDbEntry *entry = NULL;
    xpSaiRpfGroupContextDbEntry *newEntry = NULL;
    bool found = false;

    /* Search for corresponding object */
    retVal = xpSaiGetRpfGroupCtxDb(xpsDevId, groupIdOid, &entry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiGetRpfGroupCtxDb failed (retVal: %d)\n", retVal);
        return retVal;
    }

    /* Remove the bridge port Id from the table */
    for (uint32_t i = 0; i < entry->numItems; i++)
    {
        if (!found)
        {
            found = (entry->rpfGroupMemberIdOid[i] == groupMemberIdOid) ? true : false;
            continue;
        }
        else
        {
            entry->rpfGroupMemberIdOid[i - 1] = entry->rpfGroupMemberIdOid[i];
        }
    }

    if (!found)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    /* Decrement the numPorts count */
    entry->numItems--;
    if (xpSaiCtxShrinkNeeded(entry->numItems, XP_SAI_DEFAULT_MAX_RPF_GROUP_MEMBERS))
    {
        /* Shrink the queueList array size */
        XP_STATUS xpRetVal = xpSaiDynamicArrayShrink((void**)&newEntry, (void*)entry,
                                                     sizeof(xpSaiRpfGroupContextDbEntry), sizeof(sai_object_id_t),
                                                     entry->numItems, XP_SAI_DEFAULT_MAX_RPF_GROUP_MEMBERS);
        if (xpRetVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpSaiDynamicArrayShrink failed (retVal: %d)\n", xpRetVal);
            return xpsStatus2SaiStatus(xpRetVal);
        }

        /* Replace the corresponding state */
        retVal = xpSaiRpfGroupReplaceItem(xpsDevId, groupIdOid, newEntry);
        if (retVal != XP_NO_ERR)
        {
            xpsStateHeapFree((void*) newEntry);
            XP_SAI_LOG_ERR("xpSaiRpfGroupReplaceItem failed. (retVal: %d)\n", retVal);
            return retVal;
        }
    }

    return retVal;
}

sai_status_t xpSaiGetRpfValue(sai_object_id_t rpfGroupId, uint32_t *rpfValue,
                              xpMulticastRpfCheckType_e *rpfType)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    xpSaiRpfGroupContextDbEntry *rpfGroupEntry = NULL;
    xpSaiRpfGroupMemberContextDbEntry *rpfGroupMemberEntry = NULL;
    xpSaiRouterInterfaceDbEntryT *pRouterInterfaceEntry = NULL;
    xpsDevice_t xpsDevId = xpSaiObjIdSwitchGet(rpfGroupId);

    retVal = xpSaiGetRpfGroupCtxDb(xpsDevId, rpfGroupId, &rpfGroupEntry);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Unable to retreive context data for RPF group (groupIdOid: %"
                       PRIu64 ", retVal: %d)\n",
                       rpfGroupId, retVal);
        return retVal;
    }

    if (rpfGroupEntry->numItems == 0)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    retVal = xpSaiGetRpfGroupMemberCtxDb(xpsDevId,
                                         rpfGroupEntry->rpfGroupMemberIdOid[0], &rpfGroupMemberEntry);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Unable to retreive context data for RPF group member (groupMemberIdOid: %"
                       PRIu64 ", retVal: %d)\n",
                       rpfGroupEntry->rpfGroupMemberIdOid[0], retVal);
        return retVal;
    }

    retVal = xpSaiRouterInterfaceDbInfoGet(rpfGroupMemberEntry->rpfInterfaceIdOid,
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
                *rpfValue = xpSaiObjIdValueGet(pRouterInterfaceEntry->portOid);
                *rpfType = XP_MC_RPF_CHECK_TYPE_PORT;
                break;
            }
        case SAI_ROUTER_INTERFACE_TYPE_VLAN:
            {
                *rpfValue = xpSaiObjIdValueGet(pRouterInterfaceEntry->vlanOid);
                *rpfType = XP_MC_RPF_CHECK_TYPE_BD;
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("RPF Group does not support current RIF type (type: %u)\n",
                               pRouterInterfaceEntry->rifType);
                return SAI_STATUS_INVALID_PARAMETER;
            }
    }

    return retVal;
}

sai_status_t xpSaiCreateRpfGroup(sai_object_id_t *rpf_group_id,
                                 sai_object_id_t switch_id,
                                 uint32_t attr_count,
                                 const sai_attribute_t *attr_list)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    XP_STATUS xpRetVal = XP_NO_ERR;
    xpsDevice_t xpsDevId = xpSaiObjIdValueGet(switch_id);
    xpSaiRpfGroupContextDbEntry key;
    xpSaiRpfGroupContextDbEntry *entry = NULL;
    xpSaiRpfGroupMcEntryHwIdxDbEntry key2;
    xpSaiRpfGroupMcEntryHwIdxDbEntry *entry2 = NULL;
    uint32_t id;

    if (rpf_group_id == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpSaiAttrCheck(attr_count, attr_list, RPF_GROUP_VALIDATION_ARRAY_SIZE,
                            rpf_group_attribs, SAI_COMMON_API_CREATE);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Invalid attribute provided (retVal: %d)\n", retVal);
        return retVal;
    }

    xpRetVal = xpsAllocatorAllocateId(xpSaiScopeFromDevGet(xpsDevId),
                                      XP_SAI_ALLOC_RPF_GRP, &id);
    if (xpRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Unable to allocate RPF Group id (retVal: %d)\n", xpRetVal);
        return xpsStatus2SaiStatus(xpRetVal);
    }

    if (xpSaiObjIdCreate(SAI_OBJECT_TYPE_RPF_GROUP, xpsDevId, id,
                         rpf_group_id) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to create SAI RPF Group object id (id: %u)\n", id);
        return SAI_STATUS_FAILURE;
    }

    memset(&key, 0, sizeof(key));
    key.groupIdOid = *rpf_group_id;

    retVal = xpSaiInsertCtxDb(xpSaiScopeFromDevGet(xpsDevId),
                              xpSaiRpfGroupStateDbHndl, (void*) &key, sizeof(key), (void **) &entry);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to insert RPF Group to context DB (groupIdOid: %" PRIu64
                       ", retVal: %d)\n",
                       key.groupIdOid, retVal);
        return retVal;
    }
    if (entry == NULL)
    {
        XP_SAI_LOG_ERR("Failed to insert RPF Group to context \n");
        return SAI_STATUS_FAILURE;
    }

    entry->groupIdOid = *rpf_group_id;

    memset(&key2, 0, sizeof(key2));
    key2.groupIdOid = *rpf_group_id;

    retVal = xpSaiInsertCtxDb(xpSaiScopeFromDevGet(xpsDevId),
                              xpSaiRpfGroupMcEntryHwIdxDbHndl, (void*) &key2, sizeof(key2),
                              (void **) &entry2);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to insert RPF Group to Mc Entry hwIndex DB (groupIdOid: %"
                       PRIu64 ", retVal: %d)\n",
                       key2.groupIdOid, retVal);
        return retVal;
    }
    if (entry2 == NULL)
    {
        XP_SAI_LOG_ERR("Failed to insert RPF Group to Mc Entry hwIndex DB \n");
        return SAI_STATUS_FAILURE;
    }

    entry2->groupIdOid = *rpf_group_id;

    XP_SAI_LOG_DBG("Group created (groupIdOid: %" PRIu64 ")\n", *rpf_group_id);

    return retVal;
}

sai_status_t xpSaiRemoveRpfGroup(sai_object_id_t rpf_group_id)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    XP_STATUS xpRetVal = XP_NO_ERR;
    xpsDevice_t xpsDevId = xpSaiObjIdSwitchGet(rpf_group_id);
    xpSaiRpfGroupContextDbEntry *entry = NULL;

    if (!XDK_SAI_OBJID_TYPE_CHECK(rpf_group_id, SAI_OBJECT_TYPE_RPF_GROUP))
    {
        XP_SAI_LOG_ERR("Wrong object type received (type: %u)\n",
                       xpSaiObjIdTypeGet(rpf_group_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpSaiGetRpfGroupCtxDb(xpsDevId, rpf_group_id, &entry);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Unable to retreive context data for RPF group (groupIdOid: %"
                       PRIu64 ", retVal: %d)\n",
                       rpf_group_id, retVal);
        return retVal;
    }

    /* for each member remove from the group */
    if (entry->numItems)
    {
        XP_SAI_LOG_ERR("Failed to remove RPF group as it is in use (groupIdOid: %"
                       PRIu64 ")\n", rpf_group_id);
        return SAI_STATUS_OBJECT_IN_USE;
    }

    xpRetVal = xpsAllocatorReleaseId(xpSaiScopeFromDevGet(xpsDevId),
                                     XP_SAI_ALLOC_L2MC_GRP, xpSaiObjIdValueGet(entry->groupIdOid));
    if (xpRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Unable to release SAI RPF Group Id (retVal: %d)\n", xpRetVal);
        return xpsStatus2SaiStatus(xpRetVal);
    }

    retVal = xpSaiRemoveRpfGroupCtxDb(xpsDevId, rpf_group_id);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to remove RPF Group from context DB (groupIdOid: %"
                       PRIu64 ")\n", rpf_group_id);
        return retVal;
    }

    retVal = xpSaiRemoveRpfGroupMcEntryHwIdxDb(xpsDevId, rpf_group_id);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to remove RPF Group from Mc Entry hwIndex DB (groupIdOid: %"
                       PRIu64 ")\n", rpf_group_id);
        return retVal;
    }

    return retVal;
}

sai_status_t xpSaiSetRpfGroupAttribute(sai_object_id_t rpf_group_id,
                                       const sai_attribute_t *attr)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;

    if (attr == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(rpf_group_id, SAI_OBJECT_TYPE_RPF_GROUP))
    {
        XP_SAI_LOG_ERR("Wrong object type received (type: %u)\n",
                       xpSaiObjIdTypeGet(rpf_group_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpSaiAttrCheck(1, attr, RPF_GROUP_VALIDATION_ARRAY_SIZE,
                            rpf_group_attribs, SAI_COMMON_API_SET);
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

static sai_status_t xpSaiRpfGroupMcEntryHwIdxReplaceItem(xpsDevice_t xpsDevId,
                                                         sai_object_id_t groupIdOid, xpSaiRpfGroupMcEntryHwIdxDbEntry *newEntry)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    XP_STATUS xpRetVal = XP_NO_ERR;

    if (newEntry == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument (newEntry: NULL)\n");
        return XP_ERR_INVALID_INPUT;
    }

    /* Remove the corresponding state */
    retVal = xpSaiRemoveRpfGroupMcEntryHwIdxDb(xpsDevId, groupIdOid);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiRemoveRpfGroupMcEntryHwIdxDb failed (retVal: %d)\n",
                       retVal);
        return retVal;
    }

    /* Insert the new state */
    xpRetVal = xpsStateInsertData(xpSaiScopeFromDevGet(xpsDevId),
                                  xpSaiRpfGroupMcEntryHwIdxDbHndl, (void*) newEntry);
    if (xpRetVal != XP_NO_ERR)
    {
        xpsStateHeapFree((void*) newEntry);
        XP_SAI_LOG_ERR("Failed to insert data (retVal: %d)\n", xpRetVal);
        return xpsStatus2SaiStatus(xpRetVal);
    }

    return retVal;
}

sai_status_t xpSaiRpfGroupMcEntryHwIdxAddElement(xpsDevice_t xpsDevId,
                                                 sai_object_id_t groupIdOid, uint32_t mcEntryHwIndex)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    xpSaiRpfGroupMcEntryHwIdxDbEntry *entry = NULL;
    xpSaiRpfGroupMcEntryHwIdxDbEntry *newEntry = NULL;

    /* Search for corresponding object */
    retVal = xpSaiGetRpfGroupMcEntryHwIdxDb(xpsDevId, groupIdOid, &entry);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiGetRpfGroupCtxDb failed (retVal: %d)\n", retVal);
        return retVal;
    }

    if (xpSaiCtxGrowthNeeded(entry->numItems,
                             XP_SAI_DEFAULT_MAX_RPF_GROUP_MC_ENTRY_HW_INDEX))
    {
        /* Extend the array size */
        XP_STATUS xpRetVal = xpSaiDynamicArrayGrow((void**)&newEntry, (void*)entry,
                                                   sizeof(xpSaiRpfGroupMcEntryHwIdxDbEntry), sizeof(uint32_t),
                                                   entry->numItems, XP_SAI_DEFAULT_MAX_RPF_GROUP_MC_ENTRY_HW_INDEX);
        if (xpRetVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpSaiDynamicArrayGrow failed (retVal: %d)\n", xpRetVal);
            return xpsStatus2SaiStatus(xpRetVal);
        }

        /* Replace the corresponding state */
        retVal = xpSaiRpfGroupMcEntryHwIdxReplaceItem(xpsDevId, groupIdOid, newEntry);
        if (retVal != SAI_STATUS_SUCCESS)
        {
            xpsStateHeapFree((void*) newEntry);
            XP_SAI_LOG_ERR("xpSaiRpfGroupMcEntryHwIdxReplaceItem failed. (retVal: %d)\n",
                           retVal);
            return retVal;
        }

        entry = newEntry;
    }

    entry->rpfGroupMcEntryHwIdx[entry->numItems] = mcEntryHwIndex;
    entry->numItems++;

    return retVal;
}

sai_status_t xpSaiRpfGroupMcEntryHwIdxDelElement(xpsDevice_t xpsDevId,
                                                 sai_object_id_t groupIdOid, uint32_t mcEntryHwIndex)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    xpSaiRpfGroupMcEntryHwIdxDbEntry *entry = NULL;
    xpSaiRpfGroupMcEntryHwIdxDbEntry *newEntry = NULL;
    bool found = false;

    /* Search for corresponding object */
    retVal = xpSaiGetRpfGroupMcEntryHwIdxDb(xpsDevId, groupIdOid, &entry);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiGetRpfGroupMcEntryHwIdxDb failed (retVal: %d)\n", retVal);
        return retVal;
    }

    for (uint32_t i = 0; i < entry->numItems; i++)
    {
        if (!found)
        {
            found = (entry->rpfGroupMcEntryHwIdx[i] == mcEntryHwIndex) ? true : false;
            continue;
        }
        else
        {
            entry->rpfGroupMcEntryHwIdx[i - 1] = entry->rpfGroupMcEntryHwIdx[i];
        }
    }

    if (!found)
    {
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    /* Decrement the numPorts count */
    entry->numItems--;
    if (xpSaiCtxShrinkNeeded(entry->numItems,
                             XP_SAI_DEFAULT_MAX_RPF_GROUP_MC_ENTRY_HW_INDEX))
    {
        /* Shrink the queueList array size */
        XP_STATUS xpRetVal = xpSaiDynamicArrayShrink((void**)&newEntry, (void*)entry,
                                                     sizeof(xpSaiRpfGroupMcEntryHwIdxDbEntry), sizeof(uint32_t),
                                                     entry->numItems, XP_SAI_DEFAULT_MAX_RPF_GROUP_MC_ENTRY_HW_INDEX);
        if (xpRetVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpSaiDynamicArrayShrink failed (retVal: %d)\n", xpRetVal);
            return xpsStatus2SaiStatus(xpRetVal);
        }

        /* Replace the corresponding state */
        retVal = xpSaiRpfGroupMcEntryHwIdxReplaceItem(xpsDevId, groupIdOid, newEntry);
        if (retVal != SAI_STATUS_SUCCESS)
        {
            xpsStateHeapFree((void*) newEntry);
            XP_SAI_LOG_ERR("xpSaiRpfGroupMcEntryHwIdxReplaceItem failed. (retVal: %d)\n",
                           retVal);
            return retVal;
        }
    }

    return retVal;
}

sai_status_t xpSaiRpfGroupMcEntryHwIdxGetRpfGroupOId(xpsDevice_t xpsDevId,
                                                     uint32_t mcEntryHwIndex, sai_object_id_t *rpfGroupId)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    sai_object_id_t groupIdOid = 0;
    xpSaiRpfGroupMcEntryHwIdxDbEntry *entry = NULL;
    xpsScope_t scope = xpSaiScopeFromDevGet(xpsDevId);
    uint32_t count, i, j;

    retVal = xpSaiCountCtxDbObjects(scope, xpSaiRpfGroupMcEntryHwIdxDbHndl, &count);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiCountCtxDbObjects failed (retVal: %d)\n", retVal);
        return retVal;
    }

    for (i = 0; i < count; i++)
    {
        retVal = xpSaiGetNextCtxDb(scope, xpSaiRpfGroupMcEntryHwIdxDbHndl, entry,
                                   (void **) &entry);
        if (retVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to retrieve xpSaiRpfGroupMcEntryHwIdxDbEntry object (retVal: %d)\n",
                           retVal);
            return retVal;
        }
        if (entry == NULL)
        {
            XP_SAI_LOG_ERR("Failed to retrieve xpSaiRpfGroupMcEntryHwIdxDbEntry \n");
            return SAI_STATUS_FAILURE;
        }

        for (j = 0; j < entry->numItems; j++)
        {
            if (entry->rpfGroupMcEntryHwIdx[j] == mcEntryHwIndex)
            {
                *rpfGroupId = groupIdOid;
                return SAI_STATUS_SUCCESS;
            }
        }
    }

    XP_SAI_LOG_ERR("multicast entry hwIndex doesn't belong to any rpfGroup object (retVal: %d)\n",
                   retVal);
    return SAI_STATUS_ITEM_NOT_FOUND;
}

sai_status_t xpSaiRpfGroupMcEntryHwIdxReplaceHwIdx(xpsDevice_t xpsDevId,
                                                   uint32_t mcEntryHwIndex, uint32_t newMcEntryHwIndex)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    xpSaiRpfGroupMcEntryHwIdxDbEntry *entry = NULL;
    xpsScope_t scope = xpSaiScopeFromDevGet(xpsDevId);
    uint32_t count, i, j;

    retVal = xpSaiCountCtxDbObjects(scope, xpSaiRpfGroupMcEntryHwIdxDbHndl, &count);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiCountCtxDbObjects failed (retVal: %d)\n", retVal);
        return retVal;
    }

    for (i = 0; i < count; i++)
    {
        retVal = xpSaiGetNextCtxDb(scope, xpSaiRpfGroupMcEntryHwIdxDbHndl, entry,
                                   (void **) &entry);
        if (retVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to retrieve xpSaiRpfGroupMcEntryHwIdxDbEntry object (retVal: %d)\n",
                           retVal);
            return retVal;
        }
        if (entry == NULL)
        {
            XP_SAI_LOG_ERR("Failed to retrieve xpSaiRpfGroupMcEntryHwIdxDbEntry \n");
            return SAI_STATUS_FAILURE;
        }

        for (j = 0; j < entry->numItems; j++)
        {
            if (entry->rpfGroupMcEntryHwIdx[j] == mcEntryHwIndex)
            {
                entry->rpfGroupMcEntryHwIdx[j] = newMcEntryHwIndex;
                //Mc Entry can not be part of two rpfGroup so it is safe to return once the matching entry found.
                return SAI_STATUS_SUCCESS;
            }
        }
    }

    XP_SAI_LOG_ERR("multicast entry hwIndex doesn't belong to any rpfGroup object (retVal: %d)\n",
                   retVal);
    return SAI_STATUS_ITEM_NOT_FOUND;
}

sai_status_t xpSaiRpfGroupMcEntryHwIdxUpdateRpfValue(xpsDevice_t xpsDevId,
                                                     sai_object_id_t groupIdOid)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    xpSaiRpfGroupMcEntryHwIdxDbEntry *entry = NULL;
    uint32_t rpfValue = 0, index;
    xpMulticastRpfCheckType_e rpfType = XP_MC_RPF_CHECK_TYPE_PORT;
    xpMulticastRpfFailCmd_e rpfFailCmd = XP_MC_RPF_FAIL_CMD_DEFER_TO_EGRESS;
    XP_STATUS xpRetVal = XP_NO_ERR;

    /* Search for corresponding object */
    retVal = xpSaiGetRpfGroupMcEntryHwIdxDb(xpsDevId, groupIdOid, &entry);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiGetRpfGroupMcEntryHwIdxDb failed (retVal: %d)\n", retVal);
        return retVal;
    }

    retVal = xpSaiGetRpfValue(groupIdOid, &rpfValue, &rpfType);
    if ((retVal != SAI_STATUS_SUCCESS) && (retVal != SAI_STATUS_ITEM_NOT_FOUND))
    {
        XP_SAI_LOG_ERR("xpSaiGetRpfValue failed (retVal: %u)\n", retVal);
        return retVal;
    }

    for (uint32_t i = 0; i < entry->numItems; i++)
    {
        index = (entry->rpfGroupMcEntryHwIdx[i] & ~XP_SAI_RPF_GROUP_HW_IDX_IPV6);
        if (entry->rpfGroupMcEntryHwIdx[i] & XP_SAI_RPF_GROUP_HW_IDX_IPV6)
        {
            xpRetVal = xpsMulticastSetIpv6MulticastRouteEntryRpfParams(xpsDevId, index,
                                                                       rpfType, rpfValue, rpfFailCmd);
        }
        else
        {
            xpRetVal = xpsMulticastSetIpv4MulticastRouteEntryRpfParams(xpsDevId, index,
                                                                       rpfType, rpfValue, rpfFailCmd);
        }

        if (xpRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to update rfpParams on hardware (retVal: %u)\n",
                           xpRetVal);
            return xpsStatus2SaiStatus(xpRetVal);
        }
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t xpSaiGetRpfGroupAttrMemberList(xpSaiRpfGroupContextDbEntry
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
        pSaiObjList->list[i] = entry.rpfGroupMemberIdOid[i];
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiGetRpfGroupAttribute(sai_object_id_t rpf_group_id,
                                       uint32_t attr_count,
                                       sai_attribute_t *attr_list)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    xpSaiRpfGroupContextDbEntry *entry = NULL;
    xpsDevice_t xpsDevId = xpSaiObjIdSwitchGet(rpf_group_id);

    if (!XDK_SAI_OBJID_TYPE_CHECK(rpf_group_id, SAI_OBJECT_TYPE_RPF_GROUP))
    {
        XP_SAI_LOG_ERR("Wrong object type received (type: %u)\n",
                       xpSaiObjIdTypeGet(rpf_group_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpSaiGetRpfGroupCtxDb(xpsDevId, rpf_group_id, &entry);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Unable to retreive context data for RPF group (groupIdOid: %"
                       PRIu64 ", retVal: %d)\n",
                       rpf_group_id, retVal);
        return retVal;
    }

    for (uint32_t count = 0; count < attr_count; count++)
    {
        switch (attr_list[count].id)
        {
            case SAI_RPF_GROUP_ATTR_RPF_INTERFACE_COUNT:
                {
                    attr_list[count].value.u32 = entry->numItems;

                    break;
                }
            case SAI_RPF_GROUP_ATTR_RPF_MEMBER_LIST:
                {
                    retVal = xpSaiGetRpfGroupAttrMemberList(*entry,
                                                            &attr_list[count].value.objlist);
                    if (retVal != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Failed to get member list of RPF Group (groupIdOid: %" PRIu64
                                       ", retVal: %d)\n",
                                       rpf_group_id, retVal);
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

void xpSaiUpdateRpfGroupMemberAttributeVals(const uint32_t attr_count,
                                            const sai_attribute_t *attr_list, xpSaiRpfGroupMemberAttributesT *attributes)
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
            case SAI_RPF_GROUP_MEMBER_ATTR_RPF_GROUP_ID:
                {
                    attributes->groupId = attr_list[count].value.oid;
                    break;
                }
            case SAI_RPF_GROUP_MEMBER_ATTR_RPF_INTERFACE_ID:
                {
                    attributes->rpfInterfaceId = attr_list[count].value.oid;
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Failed to set %d\n", attr_list[count].id);
                }
        }
    }
}

sai_status_t xpSaiCreateRpfGroupMember(sai_object_id_t *rpf_group_member_id,
                                       sai_object_id_t switch_id,
                                       uint32_t attr_count,
                                       const sai_attribute_t *attr_list)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    XP_STATUS xpRetVal = XP_NO_ERR;
    xpsDevice_t xpsDevId = xpSaiObjIdSwitchGet(switch_id);
    xpSaiRpfGroupMemberAttributesT attributes;
    xpSaiRpfGroupMemberContextDbEntry *entry = NULL;
    xpSaiRpfGroupMemberContextDbEntry key;
    xpSaiRpfGroupContextDbEntry *groupEntry = NULL;
    uint32_t id;

    if (rpf_group_member_id == NULL)
    {
        XP_SAI_LOG_ERR("Invalid input parameter\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpSaiAttrCheck(attr_count, attr_list,
                            RPF_GROUP_MEMBER_VALIDATION_ARRAY_SIZE,
                            rpf_group_member_attribs, SAI_COMMON_API_CREATE);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Invalid attribute provided (retVal: %d)\n", retVal);
        return retVal;
    }

    xpSaiUpdateRpfGroupMemberAttributeVals(attr_count, attr_list, &attributes);

    if (!XDK_SAI_OBJID_TYPE_CHECK(attributes.groupId, SAI_OBJECT_TYPE_RPF_GROUP))
    {
        XP_SAI_LOG_ERR("Wrong object type received (type: %u)\n",
                       xpSaiObjIdTypeGet(attributes.groupId));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(attributes.rpfInterfaceId,
                                  SAI_OBJECT_TYPE_ROUTER_INTERFACE))
    {
        XP_SAI_LOG_ERR("Wrong object type received (type: %u)\n",
                       xpSaiObjIdTypeGet(attributes.rpfInterfaceId));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpSaiGetRpfGroupCtxDb(xpsDevId, attributes.groupId, &groupEntry);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        if (retVal == SAI_STATUS_ITEM_NOT_FOUND)
        {
            XP_SAI_LOG_ERR("RPF group does not exist (groupIdOid: %" PRIu64
                           ", retVal: %d)\n",
                           attributes.groupId, retVal);
        }
        else
        {
            XP_SAI_LOG_ERR("Unable to retreive context data for RPF group (groupIdOid: %"
                           PRIu64 ", retVal: %d)\n",
                           attributes.groupId, retVal);
        }
        return retVal;
    }

    if (groupEntry->numItems)
    {
        XP_SAI_LOG_ERR("RPF group size is limited to one member\n");
        return SAI_STATUS_NOT_SUPPORTED;
    }

    /* allocate unique id for RPF Group Member */
    xpRetVal = xpsAllocatorAllocateId(xpSaiScopeFromDevGet(xpsDevId),
                                      XP_SAI_ALLOC_RPF_GRP_MEMBER, &id);
    if (xpRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Unable to allocate RPF Group Member id (retVal: %d)\n",
                       xpRetVal);
        return xpsStatus2SaiStatus(xpRetVal);
    }

    /* generate OID based on unique XPS id */
    if (xpSaiObjIdCreate(SAI_OBJECT_TYPE_RPF_GROUP_MEMBER, xpsDevId,
                         (sai_uint64_t) id, rpf_group_member_id) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to create SAI RPF Group Member object id (id: %u)\n",
                       id);
        return SAI_STATUS_FAILURE;
    }

    memset(&key, 0, sizeof(key));
    key.groupMemberIdOid = *rpf_group_member_id;

    retVal = xpSaiInsertCtxDb(xpSaiScopeFromDevGet(xpsDevId),
                              xpSaiRpfGroupMemberStateDbHndl, (void*) &key, sizeof(key), (void **) &entry);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to insert RPF Group member to context DB (groupMemberIdOid: %"
                       PRIu64 ")\n",
                       key.groupMemberIdOid);
        return retVal;
    }
    if (entry == NULL)
    {
        XP_SAI_LOG_ERR("Failed to insert RPF Group member \n");
        return SAI_STATUS_FAILURE;
    }

    entry->groupMemberIdOid = key.groupMemberIdOid;
    entry->groupIdOid = attributes.groupId;
    entry->rpfInterfaceIdOid = attributes.rpfInterfaceId;

    retVal = xpSaiRpfGroupAddElement(xpsDevId, entry->groupIdOid,
                                     entry->groupMemberIdOid);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to add interface to RPF Group list (groupIdOid: %" PRIu64
                       ", groupMemberIdOid: %" PRIu64 ", retVal: %d)\n",
                       entry->groupIdOid, entry->groupMemberIdOid, retVal);
        return retVal;
    }

    retVal = xpSaiRpfGroupMcEntryHwIdxUpdateRpfValue(xpsDevId, entry->groupIdOid);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to update RPF params to mcEntry associated with groupIdOid: %"
                       PRIu64 ", retVal: %d)\n",
                       entry->groupIdOid, retVal);
        return retVal;
    }

    return retVal;
}

sai_status_t xpSaiRemoveRpfGroupMember(sai_object_id_t rpf_group_member_id)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    XP_STATUS xpRetVal = XP_NO_ERR;
    xpSaiRpfGroupMemberContextDbEntry *entry = NULL;
    xpsDevice_t xpsDevId = xpSaiObjIdSwitchGet(rpf_group_member_id);

    if (!XDK_SAI_OBJID_TYPE_CHECK(rpf_group_member_id,
                                  SAI_OBJECT_TYPE_RPF_GROUP_MEMBER))
    {
        XP_SAI_LOG_ERR("Wrong object type received (type: %u)\n",
                       xpSaiObjIdTypeGet(rpf_group_member_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpSaiGetRpfGroupMemberCtxDb(xpsDevId, rpf_group_member_id, &entry);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Unable to retreive context data for RPF group member (groupMemberIdOid: %"
                       PRIu64 ", retVal: %d)\n",
                       rpf_group_member_id, retVal);
        return retVal;
    }

    xpRetVal = xpsAllocatorReleaseId(xpSaiScopeFromDevGet(xpsDevId),
                                     XP_SAI_ALLOC_RPF_GRP_MEMBER, xpSaiObjIdValueGet(rpf_group_member_id));
    if (xpRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Unable to release RpfGroupMember Id (retVal: %d)\n", xpRetVal);
        return xpsStatus2SaiStatus(xpRetVal);
    }

    retVal = xpSaiRpfGroupDelElement(xpsDevId, entry->groupIdOid,
                                     entry->groupMemberIdOid);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to remove interface from RPF Group list (groupIdOid: %"
                       PRIu64 ", groupMemberIdOid: %" PRIu64 ", retVal: %d)\n",
                       entry->groupIdOid, entry->groupMemberIdOid, retVal);
        return retVal;
    }

    retVal = xpSaiRemoveRpfGroupMemberCtxDb(xpsDevId, rpf_group_member_id);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to remove RPF Group member context DB (groupMemberIdOid: %"
                       PRIu64 ")\n", rpf_group_member_id);
        return retVal;
    }

    retVal = xpSaiRpfGroupMcEntryHwIdxUpdateRpfValue(xpsDevId, entry->groupIdOid);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to remove RPF params to mcEntry associated with groupIdOid: %"
                       PRIu64 ", retVal: %d)\n",
                       entry->groupIdOid, retVal);
        return retVal;
    }

    return retVal;
}

sai_status_t xpSaiSetRpfGroupMemberAttribute(sai_object_id_t
                                             rpf_group_member_id,
                                             const sai_attribute_t *attr)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;

    if (attr == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(rpf_group_member_id,
                                  SAI_OBJECT_TYPE_RPF_GROUP_MEMBER))
    {
        XP_SAI_LOG_ERR("Wrong object type received (type: %u)\n",
                       xpSaiObjIdTypeGet(rpf_group_member_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpSaiAttrCheck(1, attr, RPF_GROUP_MEMBER_VALIDATION_ARRAY_SIZE,
                            rpf_group_member_attribs, SAI_COMMON_API_SET);
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

sai_status_t xpSaiGetRpfGroupMemberAttribute(sai_object_id_t
                                             rpf_group_member_id,
                                             uint32_t attr_count,
                                             sai_attribute_t *attr_list)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    xpsDevice_t xpsDevId = xpSaiObjIdSwitchGet(rpf_group_member_id);
    xpSaiRpfGroupMemberContextDbEntry *entry = NULL;

    if (!XDK_SAI_OBJID_TYPE_CHECK(rpf_group_member_id,
                                  SAI_OBJECT_TYPE_RPF_GROUP_MEMBER))
    {
        XP_SAI_LOG_ERR("Wrong object type received (type: %u)\n",
                       xpSaiObjIdTypeGet(rpf_group_member_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpSaiAttrCheck(attr_count, attr_list,
                            RPF_GROUP_MEMBER_VALIDATION_ARRAY_SIZE,
                            rpf_group_member_attribs, SAI_COMMON_API_GET);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Invalid attribute provided (retVal: %d)\n", retVal);
        return retVal;
    }

    retVal = xpSaiGetRpfGroupMemberCtxDb(xpsDevId, rpf_group_member_id, &entry);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Unable to retreive context data for RPF group member (groupMemberIdOid: %"
                       PRIu64 ", retVal: %d)\n",
                       rpf_group_member_id, retVal);
        return retVal;
    }

    for (uint32_t count = 0; count < attr_count; count++)
    {
        switch (attr_list[count].id)
        {
            case SAI_RPF_GROUP_MEMBER_ATTR_RPF_GROUP_ID:
                {
                    attr_list[count].value.oid = entry->groupIdOid;
                    break;
                }
            case SAI_RPF_GROUP_MEMBER_ATTR_RPF_INTERFACE_ID:
                {
                    attr_list[count].value.oid = entry->rpfInterfaceIdOid;
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


sai_status_t xpSaiRpfGroupApiInit(uint64_t flag,
                                  const sai_service_method_table_t* adapHostServiceMethodTable)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;

    _xpSaiRpfGroupApi = (sai_rpf_group_api_t *) xpMalloc(sizeof(
                                                             sai_rpf_group_api_t));

    if (_xpSaiRpfGroupApi == NULL)
    {
        XP_SAI_LOG_ERR("Failed to allocate _xpSaiRpfGroupApi\n");
        return SAI_STATUS_NO_MEMORY;
    }

    _xpSaiRpfGroupApi->create_rpf_group = xpSaiCreateRpfGroup;
    _xpSaiRpfGroupApi->remove_rpf_group = xpSaiRemoveRpfGroup;
    _xpSaiRpfGroupApi->set_rpf_group_attribute = xpSaiSetRpfGroupAttribute;
    _xpSaiRpfGroupApi->get_rpf_group_attribute = xpSaiGetRpfGroupAttribute;
    _xpSaiRpfGroupApi->create_rpf_group_member = xpSaiCreateRpfGroupMember;
    _xpSaiRpfGroupApi->remove_rpf_group_member = xpSaiRemoveRpfGroupMember;
    _xpSaiRpfGroupApi->set_rpf_group_member_attribute =
        xpSaiSetRpfGroupMemberAttribute;
    _xpSaiRpfGroupApi->get_rpf_group_member_attribute =
        xpSaiGetRpfGroupMemberAttribute;

    retVal = xpSaiApiRegister(SAI_API_RPF_GROUP, (void*)_xpSaiRpfGroupApi);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to register Rpf API\n");
        return retVal;
    }

    return retVal;
}

sai_status_t xpSaiRpfGroupApiDeinit()
{
    xpFree(_xpSaiRpfGroupApi);
    _xpSaiRpfGroupApi = NULL;

    return SAI_STATUS_SUCCESS;
}

/*This is called in switch init*/
sai_status_t xpSaiRpfGroupInit(xpsDevice_t xpsDevId)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpsScope_t scope = xpSaiScopeFromDevGet(xpsDevId);

    /* Create global SAI RPF GROUP DB */
    xpSaiRpfGroupStateDbHndl = XPSAI_RPF_GROUP_STATE_DB_HNDL;
    if ((retVal = xpsStateRegisterDb(scope, "SAI RPF GROUP DB", XPS_GLOBAL,
                                     &saiRpfGroupCtxKeyComp,
                                     xpSaiRpfGroupStateDbHndl)) != XP_NO_ERR)
    {
        xpSaiRpfGroupStateDbHndl = XPS_STATE_INVALID_DB_HANDLE;
        XP_SAI_LOG_ERR("Could not create SAI RPF GROUP DB\n");
        return xpsStatus2SaiStatus(retVal);
    }

    /* Create global SAI RPF GROUP MC ENTRY HW INDEX DB */
    xpSaiRpfGroupMcEntryHwIdxDbHndl = XPSAI_RPF_GROUP_MC_ENTRY_HW_IDX_STATE_DB_HNDL;
    if ((retVal = xpsStateRegisterDb(scope, "SAI RPF GROUP MC ENTRY HW INDEX DB",
                                     XPS_GLOBAL,
                                     &saiRpfGroupMcEntryHwIdxKeyComp, xpSaiRpfGroupMcEntryHwIdxDbHndl)) != XP_NO_ERR)
    {
        xpSaiRpfGroupMcEntryHwIdxDbHndl = XPS_STATE_INVALID_DB_HANDLE;
        XP_SAI_LOG_ERR("Could not create SAI RPF GROUP MC ENTRY HW INDEX DB\n");
        return xpsStatus2SaiStatus(retVal);
    }

    /* Create global SAI RPF GROUP PORT DB */
    xpSaiRpfGroupMemberStateDbHndl = XPSAI_RPF_GROUP_PORT_STATE_DB_HNDL;
    if ((retVal = xpsStateRegisterDb(scope, "SAI RPF GROUP PORT DB", XPS_GLOBAL,
                                     &saiRpfGroupMemberCtxKeyComp,
                                     xpSaiRpfGroupMemberStateDbHndl)) != XP_NO_ERR)
    {
        xpSaiRpfGroupMemberStateDbHndl = XPS_STATE_INVALID_DB_HANDLE;
        XP_SAI_LOG_ERR("Could not create SAI RPF GROUP PORT DB\n");
        return xpsStatus2SaiStatus(retVal);
    }

    retVal = xpsAllocatorInitIdAllocator(scope, XP_SAI_ALLOC_RPF_GRP,
                                         XP_SAI_RPF_GRP_MAX_IDS, XP_SAI_RPF_GRP_RANGE_START);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to initialize SAI RPF GROUP ID allocator\n");
        return xpsStatus2SaiStatus(retVal);
    }

    retVal = xpsAllocatorInitIdAllocator(scope, XP_SAI_ALLOC_RPF_GRP_MEMBER,
                                         XP_SAI_RPF_GRP_MEMBER_MAX_IDS, XP_SAI_RPF_GRP_MEMBER_RANGE_START);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to initialize SAI RPF GROUP ID allocator\n");
        return xpsStatus2SaiStatus(retVal);
    }

    return xpsStatus2SaiStatus(retVal);
}

sai_status_t xpSaiRpfGroupDeInit(xpsDevice_t xpsDevId)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpsScope_t scope = xpSaiScopeFromDevGet(xpsDevId);

    /* Purge global SAI RPF GROUP DB */
    if ((retVal = xpsStateDeRegisterDb(scope,
                                       &xpSaiRpfGroupStateDbHndl)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not purge SAI RPF GROUP DB\n");
        return xpsStatus2SaiStatus(retVal);
    }

    /* Purge global SAI RPF GROUP MC ENTRY HW INDEX DB */
    if ((retVal = xpsStateDeRegisterDb(scope,
                                       &xpSaiRpfGroupMcEntryHwIdxDbHndl)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not purge SAI RPF GROUP MC ENTRY HW INDEX DB\n");
        return xpsStatus2SaiStatus(retVal);
    }

    /* Purge global SAI RPF GROUP PORT DB */
    if ((retVal = xpsStateDeRegisterDb(scope,
                                       &xpSaiRpfGroupMemberStateDbHndl)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not purge SAI RPF GROUP PORT DB\n");
        return xpsStatus2SaiStatus(retVal);
    }

    return xpsStatus2SaiStatus(retVal);
}
