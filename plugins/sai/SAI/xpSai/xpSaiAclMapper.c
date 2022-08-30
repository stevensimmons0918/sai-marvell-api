// xpSaiAclMapper.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpSaiAclMapper.h"
#include "xpsAllocator.h"

XP_SAI_LOG_REGISTER_API(SAI_API_ACL);


//#define XP_SAI_ACL_PER_TBL_COUNTER_ALLOC_ID(tableId) (uint16_t(XP_SAI_ALLOC_ACL_COUNTER_START + (tableId)))

extern bool xpSaiValidateEntryListAttributes(sai_acl_entry_attr_t attr);
static xpsDbHandle_t sXpSaiAclStaticTableDbHndl = XPS_STATE_INVALID_DB_HANDLE;
static xpsDbHandle_t sXpSaiAclTableIdArrayDbHndl = XPS_STATE_INVALID_DB_HANDLE;
static xpsDbHandle_t sXpSaiAclCounterIdArrayDbHndl =
    XPS_STATE_INVALID_DB_HANDLE;
static xpsDbHandle_t sXpSaiAclEntryIdArrayDbHndl = XPS_STATE_INVALID_DB_HANDLE;
static xpsDbHandle_t sXpSaiAclRangeIdDbHndl = XPS_STATE_INVALID_DB_HANDLE;
static xpsDbHandle_t sXpSaiAclTableGrpIdArrayDbHndl =
    XPS_STATE_INVALID_DB_HANDLE;
static xpsDbHandle_t sXpSaiAclTableGrpMembIdArrayDbHndl =
    XPS_STATE_INVALID_DB_HANDLE;
static xpsDbHandle_t sXpSaiAclRsnCodeToQueueMapArrayDbHndl =
    XPS_STATE_INVALID_DB_HANDLE;
sai_uint32_t gValidAclV4KeyNum;
sai_uint32_t gValidAclV6KeyNum;
sai_uint32_t validAclV4KeyLoc[XPS_PCL_MAX_NUM_FLDS];
sai_uint32_t validAclV6KeyLoc[XPS_PCL_MAX_NUM_FLDS];

static int32_t xpSaiAclTableIdKeyComp(void* key1, void* key2)
{
    /* Key is dependent on tableId */
    return ((((xpSaiAclTableIdMappingT *) key1)->tableId) - (((
                                                                  xpSaiAclTableIdMappingT *) key2)->tableId));
}

static int32_t xpSaiAclTableGrpIdKeyComp(void* key1, void* key2)
{
    /* Key is dependent on groupId */
    return ((((xpSaiAclTableGrpIdMappingT *) key1)->groupId) - (((
                                                                     xpSaiAclTableGrpIdMappingT *) key2)->groupId));
}

static int32_t xpSaiAclTableGrpMembIdKeyComp(void* key1, void* key2)
{
    /* Key is dependent on groupId */
    return ((((xpSaiAclTableGrpMembIdMappingT *) key1)->memberId) - (((
                                                                          xpSaiAclTableGrpMembIdMappingT *) key2)->memberId));
}

static int32_t xpSaiAclCounterIdKeyComp(void* key1, void* key2)
{
    /* Key is dependent on counterId */
    return ((((xpSaiAclCounterIdMappingT *) key1)->counterId) - (((
                                                                      xpSaiAclCounterIdMappingT *) key2)->counterId));
}

static int32_t xpSaiAclEntryIdKeyComp(void* key1, void* key2)
{
    /* Key is dependent on entryId */
    return ((((xpSaiAclEntryIdMappingT *) key1)->entryId) - (((
                                                                  xpSaiAclEntryIdMappingT *) key2)->entryId));
}

static int32_t xpSaiAclRangeIdKeyComp(void* key1, void* key2)
{
    /* Key is dependent on entryId */
    return ((((xpSaiAclRangeIdMappingT *) key1)->entryId) - (((
                                                                  xpSaiAclRangeIdMappingT *) key2)->entryId));
}
static int32_t xpSaiAclRsnCodeToQueueMapKeyComp(void* key1, void* key2)
{
    /* Key is dependent on Reason Code */
    return ((((xpSaiAclRsnCodeToQueueMapInfo_t *) key1)->rsnCode) - (((
                                                                          xpSaiAclRsnCodeToQueueMapInfo_t *) key2)->rsnCode));
}

XP_STATUS xpSaiAclCounterByEntryGet(sai_uint32_t devId, sai_uint32_t entryId,
                                    sai_uint32_t *counterId);
XP_STATUS xpSaiAclCounterMappingGet(xpsDevice_t devId, sai_uint32_t counterId,
                                    xpSaiAclCounterIdMappingT **info);
sai_status_t xpSaiAclCounterUpdate(xpsDevice_t devId, sai_uint32_t counterId,
                                   sai_uint32_t tableId,
                                   sai_acl_stage_t stage, sai_uint32_t ruleId);

static sai_status_t xpSaiAclMapperAllocatorsInit()
{
    XP_STATUS retVal = XP_NO_ERR;

    retVal = xpsAllocatorInitIdAllocator(XP_SCOPE_DEFAULT, XP_SAI_ALLOC_ACL_TBL,
                                         XP_SAI_ACL_TBL_MAX_IDS, XP_SAI_ACL_TBL_RANGE_START);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to initialize SAI ACL Table ID allocator\n");
        return xpsStatus2SaiStatus(retVal);
    }


    retVal = xpsAllocatorInitIdAllocator(XP_SCOPE_DEFAULT, XP_SAI_ALLOC_ACL_ENTRY,
                                         XP_SAI_ACL_ENTRY_MAX_IDS, XP_SAI_ACL_ENTRY_RANGE_START);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to initialize SAI ACL Entry ID allocator\n");
        return xpsStatus2SaiStatus(retVal);
    }

    retVal = xpsAllocatorInitIdAllocator(XP_SCOPE_DEFAULT,
                                         XP_SAI_ALLOC_ACL_TCP_UDP_PORT_RANGE,
                                         XP_SAI_ACL_RANGE_MAX_IDS, XP_SAI_ACL_RANGE_RANGE_START);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to initialize SAI ACL Entry ID allocator\n");
        return xpsStatus2SaiStatus(retVal);
    }

    //for(uint32_t i = 0; i < XP_SAI_ACL_TABLE_NUMBER; i++)
    //{
    retVal = xpsAllocatorInitIdAllocator(XP_SCOPE_DEFAULT,
                                         XP_SAI_ALLOC_ACL_COUNTER_START,
                                         XP_SAI_ACL_PER_TBL_COUNTER_MAX_IDS, XP_SAI_ACL_PER_TBL_COUNTER_RANGE_START);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to initialize SAI ACL Per Table Counter ID allocator\n");
        return xpsStatus2SaiStatus(retVal);
    }
    //}

    retVal = xpsAllocatorInitIdAllocator(XP_SCOPE_DEFAULT, XP_SAI_ALLOC_ACL_TBL_GRP,
                                         XP_SAI_ACL_TBL_GRP_MAX_IDS, XP_SAI_ACL_TBL_GRP_RANGE_START);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to initialize SAI ACL Table Group ID allocator\n");
        return xpsStatus2SaiStatus(retVal);
    }

    retVal = xpsAllocatorInitIdAllocator(XP_SCOPE_DEFAULT,
                                         XP_SAI_ALLOC_ACL_TBL_GRP_MEMBER,
                                         XP_SAI_ACL_TBL_GRP_MEMBER_MAX_IDS, XP_SAI_ACL_TBL_GRP_MEMBER_RANGE_START);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to initialize SAI ACL Table Group Member ID allocator\n");
        return xpsStatus2SaiStatus(retVal);
    }

    /* Reserve ACL rule-Ids for ERSPAN2 per mirror session per type.
       Note here only IDs are reserved and actual rules are configured based
       on mirror_create from user.*/
    uint8_t type = XPS_ACL_TABLE_TYPE_IPV4;
    uint32_t ruleId = 0xFFFFFFFF;
    for (uint32_t analyzerIndex = 0 ; analyzerIndex < XP_MIRROR_MAX_USER_SESSION+1;
         analyzerIndex++)
    {
        for (type = XPS_ACL_TABLE_TYPE_ARP; type <= XPS_ACL_TABLE_TYPE_IPV6; type++)
        {
            ruleId = (analyzerIndex* (XPS_ACL_TABLE_TYPE_IPV6+1)) + type;
            retVal = xpsAllocatorAllocateWithId(XP_SCOPE_DEFAULT, XP_SAI_ALLOC_ACL_ENTRY,
                                                ruleId);
            if (retVal != XP_NO_ERR)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Could not allocate ACL ruleId id, error %d\n", retVal);
                return retVal;
            }
        }
    }

    return SAI_STATUS_SUCCESS;
}


sai_status_t xpSaiAclMapperGetTableGroupStateData(xpsDevice_t devId,
                                                  sai_object_id_t tableGrpObjId, xpSaiAclTableGrpIdMappingT **ppTblGrpEntry)
{
    XP_STATUS                   xpsStatus = XP_NO_ERR;
    xpSaiAclTableGrpIdMappingT  key;

    key.groupId = tableGrpObjId;

    // Get the table group state data base
    xpsStatus = xpsStateSearchDataForDevice(devId, sXpSaiAclTableGrpIdArrayDbHndl,
                                            (xpsDbKey_t)&key, (void**)ppTblGrpEntry);
    if (xpsStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get the acl table group state data base, status: %d\n",
                       xpsStatus);
        return xpsStatus2SaiStatus(xpsStatus);
    }

    if (*ppTblGrpEntry == NULL)
    {
        XP_SAI_LOG_ERR("Acl table group state entry not found, %" PRId64 "\n",
                       tableGrpObjId);
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiAclMapperBindToTableGroup(xpsDevice_t devId,
                                            sai_object_id_t tableGrpObjId, sai_object_id_t boundedObjId)
{
    sai_status_t                saiStatus = SAI_STATUS_SUCCESS;
    XP_STATUS xpStatus = XP_NO_ERR;
    xpSaiAclTableGrpIdMappingT *pTblGrpEntry = NULL;
    uint32_t tableId = 0xFFFFFFFF;
    uint32_t xpsIntfId = 0xFFFFFFFF;

    // Get the table group state data
    saiStatus = xpSaiAclMapperGetTableGroupStateData(devId, tableGrpObjId,
                                                     &pTblGrpEntry);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get the acl table group state data base, status: %d\n",
                       saiStatus);
        return saiStatus;
    }

    if (pTblGrpEntry->bindPointCount >= SAI_ACL_BIND_POINT_LIST_SIZE)
    {
        XP_SAI_LOG_ERR("Acl table group bind point list array full\n");
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    pTblGrpEntry->bindPointList[pTblGrpEntry->bindPointCount] = boundedObjId;
    pTblGrpEntry->bindPointCount += 1;

    sai_uint32_t memberId    = 0;
    xpSaiAclTableGrpMembIdMappingT *keyFormat  = NULL;
    for (uint32_t i = 0; i < pTblGrpEntry->memberCount; i++)
    {
        memberId = (sai_uint32_t)xpSaiObjIdValueGet(pTblGrpEntry->memberList[i]);

        saiStatus = xpSaiAclMapperAclGroupMemberAttributesGet(memberId, &keyFormat);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpsStateSearchDataForDevice failed with xpStatus: %d\n",
                           saiStatus);
            return saiStatus;
        }

        tableId = keyFormat->tableId;
        xpsIntfId = xpSaiObjIdValueGet(boundedObjId);

        if (XDK_SAI_OBJID_TYPE_CHECK(boundedObjId, SAI_OBJECT_TYPE_LAG))
        {
            xpStatus = xpsAclSetLagAcl(devId, xpsIntfId, tableId, pTblGrpEntry->groupId);
            if (xpStatus != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR(" LAG bind Failed %d tblId %d \n", xpsIntfId, tableId);
                return xpsStatus2SaiStatus(xpStatus);
            }
        }
        else if (XDK_SAI_OBJID_TYPE_CHECK(boundedObjId, SAI_OBJECT_TYPE_PORT))
        {
            xpStatus = xpsAclSetPortAcl(devId, xpsIntfId, tableId, pTblGrpEntry->groupId);
            if (xpStatus != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR(" Port bind Failed port %d tblId %d \n", xpsIntfId, tableId);
                return xpsStatus2SaiStatus(xpStatus);
            }
        }
        else if (XDK_SAI_OBJID_TYPE_CHECK(boundedObjId, SAI_OBJECT_TYPE_VLAN))
        {
            xpStatus = xpsAclSetVlanAcl(devId, xpsIntfId, tableId, pTblGrpEntry->groupId);
            if (xpStatus != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR(" VLan bind Failed %d tblId %d \n", xpsIntfId, tableId);
                return xpsStatus2SaiStatus(xpStatus);
            }
        }
        else if (XDK_SAI_OBJID_TYPE_CHECK(boundedObjId,
                                          SAI_OBJECT_TYPE_ROUTER_INTERFACE))
        {
            xpSaiRouterInterfaceDbEntryT *pRifEntry = NULL;

            saiStatus = xpSaiRouterInterfaceDbInfoGet(boundedObjId, &pRifEntry);
            if (saiStatus != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("xpSaiRouterInterfaceDbInfoGet() failed with error code: %d!\n",
                               saiStatus);
                return saiStatus;
            }

            saiStatus = xpSaiAclRouterAclIdSet(pRifEntry->l3IntfId, pTblGrpEntry->groupId,
                                               tableId, true);
            if (saiStatus != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("xpSaiAclRouterAclIdSet failed with error %d \n", saiStatus);
                return saiStatus;
            }
        }
        else
        {
            return SAI_STATUS_FAILURE;
        }
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiAclMapperUnBindFromTableGroup(xpsDevice_t devId,
                                                sai_object_id_t tableGrpObjId,
                                                sai_object_id_t boundedObjId)
{
    sai_status_t                saiStatus = SAI_STATUS_SUCCESS;
    XP_STATUS xpStatus = XP_NO_ERR;
    xpSaiAclTableGrpIdMappingT *pTblGrpEntry = NULL;
    uint32_t tableId = 0xFFFFFFFF;
    uint32_t xpsIntfId = 0xFFFFFFFF;
    uint32_t i, j=0;

    // Get the table group state data
    saiStatus = xpSaiAclMapperGetTableGroupStateData(devId, tableGrpObjId,
                                                     &pTblGrpEntry);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get the acl table group state data base, status: %d\n",
                       saiStatus);
        return saiStatus;
    }

    if (pTblGrpEntry->bindPointCount == 0)
    {
        XP_SAI_LOG_ERR("Acl table group bind point list array empty\n");
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    for (i = 0; i< pTblGrpEntry->bindPointCount; i++)
    {
        if (pTblGrpEntry->bindPointList[i] == boundedObjId)
        {
            j=i;
            for (; j< pTblGrpEntry->bindPointCount-1; j++)
            {
                pTblGrpEntry->bindPointList[j] = pTblGrpEntry->bindPointList[j+1];
            }
            pTblGrpEntry->bindPointList[j] = SAI_NULL_OBJECT_ID;
            break;
        }
    }
    pTblGrpEntry->bindPointCount -= 1;

    sai_uint32_t memberId    = 0;
    xpSaiAclTableGrpMembIdMappingT *keyFormat  = NULL;
    for (uint32_t i = 0; i < pTblGrpEntry->memberCount; i++)
    {
        memberId = (sai_uint32_t)xpSaiObjIdValueGet(pTblGrpEntry->memberList[i]);

        saiStatus = xpSaiAclMapperAclGroupMemberAttributesGet(memberId, &keyFormat);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpsStateSearchDataForDevice failed with xpStatus: %d\n",
                           saiStatus);
            return saiStatus;
        }

        tableId = keyFormat->tableId;
        xpsIntfId = xpSaiObjIdValueGet(boundedObjId);
        if (XDK_SAI_OBJID_TYPE_CHECK(boundedObjId, SAI_OBJECT_TYPE_LAG))
        {
            xpStatus = xpsAclLagUnbind(devId, xpsIntfId, pTblGrpEntry->groupId, tableId);
            if (xpStatus != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR(" LAG unbind Failed  %d tblId %d \n", xpsIntfId, tableId);
                return xpsStatus2SaiStatus(xpStatus);
            }
        }
        else if (XDK_SAI_OBJID_TYPE_CHECK(boundedObjId, SAI_OBJECT_TYPE_PORT))
        {
            xpStatus = xpsAclPortUnbind(devId, xpsIntfId, pTblGrpEntry->groupId, tableId);
            if (xpStatus != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR(" Port unbind Failed port %d tblId %d \n", xpsIntfId, tableId);
                return xpsStatus2SaiStatus(xpStatus);
            }
        }
        else if (XDK_SAI_OBJID_TYPE_CHECK(boundedObjId, SAI_OBJECT_TYPE_VLAN))
        {
            xpStatus = xpsAclVlanUnbind(devId, xpsIntfId, pTblGrpEntry->groupId, tableId);
            if (xpStatus != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR(" VLan unbind Failed %d tblId %d \n", xpsIntfId, tableId);
                return xpsStatus2SaiStatus(xpStatus);
            }
        }
        else if (XDK_SAI_OBJID_TYPE_CHECK(boundedObjId,
                                          SAI_OBJECT_TYPE_ROUTER_INTERFACE))
        {
            xpSaiRouterInterfaceDbEntryT *pRifEntry = NULL;

            saiStatus = xpSaiRouterInterfaceDbInfoGet(boundedObjId, &pRifEntry);
            if (saiStatus != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("xpSaiRouterInterfaceDbInfoGet() failed with error code: %d!\n",
                               saiStatus);
                return saiStatus;
            }

            saiStatus = xpSaiAclRouterAclIdSet(pRifEntry->l3IntfId, pTblGrpEntry->groupId,
                                               tableId, false);
            if (saiStatus != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("xpSaiAclRouterAclIdSet failed with error %d \n", saiStatus);
                return saiStatus;
            }
        }
        else
        {
            return SAI_STATUS_FAILURE;
        }
    }
    return SAI_STATUS_SUCCESS;
}

//This function allocates virtual table group ID
sai_status_t xpSaiAclMapperCreateTableGrpId(xpsDevice_t devId,
                                            sai_uint32_t *tblGrpId, xpSaiAclTableGrpAttributesT *attibutes)
{
    XP_STATUS                   retVal     = XP_NO_ERR;
    sai_uint32_t                groupId    = 0;
    xpSaiAclTableGrpIdMappingT *keyFormat  = NULL;

    if (tblGrpId == NULL)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpsAllocatorAllocateId(XP_SCOPE_DEFAULT, XP_SAI_ALLOC_ACL_TBL_GRP,
                                    &groupId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to allocate table group id retVal %d \n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    retVal = xpsStateHeapMalloc(sizeof(xpSaiAclTableGrpIdMappingT),
                                (void**)&keyFormat);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to allocate memory for the device %u \n", devId);
        return xpsStatus2SaiStatus(retVal);
    }

    memset(keyFormat, 0, sizeof(xpSaiAclTableGrpIdMappingT));

    keyFormat->groupId = groupId;
    keyFormat->stage = attibutes->stage;
    memcpy(keyFormat->bp_list, attibutes->bp_list, sizeof(keyFormat->bp_list));
    keyFormat->type = attibutes->type;

    /* Insert SAI ACL Table Group entry structure into DB */
    retVal = xpsStateInsertDataForDevice(devId, sXpSaiAclTableGrpIdArrayDbHndl,
                                         (void*)keyFormat);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to add DB to the device %u \n", devId);
        xpsStateHeapFree((void*)keyFormat);
        return xpsStatus2SaiStatus(retVal);
    }

    *tblGrpId = groupId;

    return SAI_STATUS_SUCCESS;
}

//This function deallocates virtual table group ID
sai_status_t xpSaiAclMapperRemoveTableGrpId(xpsDevice_t devId,
                                            sai_uint32_t tblGrpId)
{
    XP_STATUS                   retVal     = XP_NO_ERR;
    xpSaiAclTableGrpIdMappingT *keyFormat  = NULL;
    xpSaiAclTableGrpIdMappingT  searchKeyFormat;

    memset(&searchKeyFormat, 0, sizeof(xpSaiAclTableGrpIdMappingT));
    searchKeyFormat.groupId = tblGrpId;

    retVal = xpsAllocatorReleaseId(XP_SCOPE_DEFAULT, XP_SAI_ALLOC_ACL_TBL_GRP,
                                   tblGrpId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to deallocate table group id :%d \n", tblGrpId);
        return xpsStatus2SaiStatus(retVal);
    }

    // Remove the corresponding state
    if ((retVal = xpsStateDeleteDataForDevice(devId, sXpSaiAclTableGrpIdArrayDbHndl,
                                              &searchKeyFormat, (void **)&keyFormat)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to delete DB for device %u \n", devId);
        return xpsStatus2SaiStatus(retVal);
    }

    xpsStateHeapFree((void*)keyFormat);

    return xpsStatus2SaiStatus(retVal);
}

//This function allocates virtual table group member ID
sai_status_t xpSaiAclMapperCreateTableGrpMembId(xpsDevice_t devId,
                                                sai_uint32_t *grpMembId, xpSaiAclTableGrpMembAttributesT *attibutes)
{
    XP_STATUS                       retVal     = XP_NO_ERR;
    sai_uint32_t                    memberId   = 0;
    xpSaiAclTableGrpMembIdMappingT *keyFormat  = NULL;

    if (grpMembId == NULL)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpsAllocatorAllocateId(XP_SCOPE_DEFAULT,
                                    XP_SAI_ALLOC_ACL_TBL_GRP_MEMBER, &memberId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to allocate table group member id!\n");
        return xpsStatus2SaiStatus(retVal);
    }

    retVal = xpsStateHeapMalloc(sizeof(xpSaiAclTableGrpMembIdMappingT),
                                (void**)&keyFormat);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to allocate memory for the device %u \n", devId);
        return xpsStatus2SaiStatus(retVal);
    }

    memset(keyFormat, 0, sizeof(xpSaiAclTableGrpMembIdMappingT));

    keyFormat->memberId = memberId;
    keyFormat->groupId = (sai_uint32_t)xpSaiObjIdValueGet(attibutes->groupId);
    keyFormat->tableId = (sai_uint32_t)xpSaiObjIdValueGet(attibutes->tableId);
    keyFormat->priority = attibutes->priority;

    /* Insert SAI ACL Table Group Member entry structure into DB */
    retVal = xpsStateInsertDataForDevice(devId, sXpSaiAclTableGrpMembIdArrayDbHndl,
                                         (void*)keyFormat);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to add DB to the device %u \n", devId);
        xpsStateHeapFree((void*)keyFormat);
        return xpsStatus2SaiStatus(retVal);
    }

    *grpMembId = memberId;

    return SAI_STATUS_SUCCESS;
}

//This function deallocates virtual table group member ID
sai_status_t xpSaiAclMapperRemoveTableGrpMembId(xpsDevice_t devId,
                                                sai_uint32_t grpMembId)
{
    XP_STATUS                       retVal     = XP_NO_ERR;
    xpSaiAclTableGrpMembIdMappingT *keyFormat  = NULL;
    xpSaiAclTableGrpMembIdMappingT  searchKeyFormat;

    memset(&searchKeyFormat, 0, sizeof(xpSaiAclTableGrpMembIdMappingT));
    searchKeyFormat.memberId = grpMembId;

    retVal = xpsAllocatorReleaseId(XP_SCOPE_DEFAULT,
                                   XP_SAI_ALLOC_ACL_TBL_GRP_MEMBER, grpMembId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to deallocate table group member id :%d \n",
                       grpMembId);
        return xpsStatus2SaiStatus(retVal);
    }

    // Remove the corresponding state
    if ((retVal = xpsStateDeleteDataForDevice(devId,
                                              sXpSaiAclTableGrpMembIdArrayDbHndl,
                                              &searchKeyFormat, (void **)&keyFormat)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to delete DB for device %u \n", devId);
        return xpsStatus2SaiStatus(retVal);
    }

    xpsStateHeapFree((void*)keyFormat);

    return xpsStatus2SaiStatus(retVal);
}

sai_uint32_t xpSaiAclEntryPriNumMaxGet()
{
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;

    sai_uint32_t profileId = 0;
    sai_int32_t  value = 0;

    saiStatus = xpSaiSwitchProfileIdGet(&profileId);
    if (saiStatus == SAI_STATUS_SUCCESS)
    {
        saiStatus = xpSaiSwitchProfileIntValueGet(profileId,
                                                  XP_SAI_ENTRY_PRI_NUM_MAX_PARAM, &value);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            return XP_SAI_DEFAULT_ENTRY_PRI_NUM_MAX;
        }

        return (sai_uint32_t)value;
    }

    return XP_SAI_DEFAULT_ENTRY_PRI_NUM_MAX;
}

sai_status_t
xpSaiAclMapperInit(xpsDevice_t devId)
{
    XP_STATUS            retVal = XP_NO_ERR;
    sai_status_t         saiRetVal   = SAI_STATUS_SUCCESS;
    xpsDefineKeyConfig_t keyConfig;

    memset(&keyConfig, 0x0, sizeof(xpsDefineKeyConfig_t));

    saiRetVal = xpSaiAclMapperAllocatorsInit();
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        return saiRetVal;
    }

    if (sXpSaiAclTableIdArrayDbHndl == XPS_STATE_INVALID_DB_HANDLE)
    {
        sXpSaiAclTableIdArrayDbHndl = XPSAI_ACL_TABLE_ARRAY_DB_HNDL;
        retVal = xpsStateRegisterDb(XP_SCOPE_DEFAULT, "SAI ACL TableID Array",
                                    XPS_PER_DEVICE, &xpSaiAclTableIdKeyComp, sXpSaiAclTableIdArrayDbHndl);
        if (retVal != XP_NO_ERR)
        {
            sXpSaiAclTableIdArrayDbHndl = XPS_STATE_INVALID_DB_HANDLE;
            XP_SAI_LOG_ERR("Error: Failed to register state database\n");
            return xpsStatus2SaiStatus(retVal);
        }

        if (INIT_COLD == xpsGetInitType())
        {
            retVal = xpsStateInsertDbForDevice(devId, sXpSaiAclTableIdArrayDbHndl,
                                               xpSaiAclTableIdKeyComp);
            if (retVal != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Error: Failed to add DB for device %u \n", devId);
                return xpsStatus2SaiStatus(retVal);
            }
        }
    }

    if (sXpSaiAclTableGrpIdArrayDbHndl == XPS_STATE_INVALID_DB_HANDLE)
    {
        sXpSaiAclTableGrpIdArrayDbHndl = XPSAI_ACL_TABLEGROUP_ARRAY_DB_HNDL;
        retVal = xpsStateRegisterDb(XP_SCOPE_DEFAULT, "SAI ACL TableGroupID Array",
                                    XPS_PER_DEVICE, &xpSaiAclTableGrpIdKeyComp, sXpSaiAclTableGrpIdArrayDbHndl);
        if (retVal != XP_NO_ERR)
        {
            sXpSaiAclTableGrpIdArrayDbHndl = XPS_STATE_INVALID_DB_HANDLE;
            XP_SAI_LOG_ERR("Error: Failed to register state database\n");
            return xpsStatus2SaiStatus(retVal);
        }

        if (INIT_COLD == xpsGetInitType())
        {
            retVal = xpsStateInsertDbForDevice(devId, sXpSaiAclTableGrpIdArrayDbHndl,
                                               xpSaiAclTableGrpIdKeyComp);
            if (retVal != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Error: Failed to add DB for device %u \n", devId);
                return xpsStatus2SaiStatus(retVal);
            }
        }
    }

    if (sXpSaiAclTableGrpMembIdArrayDbHndl == XPS_STATE_INVALID_DB_HANDLE)
    {
        sXpSaiAclTableGrpMembIdArrayDbHndl = XPSAI_ACL_TABLEGROUP_MEMBER_ARRAY_DB_HNDL;
        retVal = xpsStateRegisterDb(XP_SCOPE_DEFAULT, "SAI ACL TableGroupMembID Array",
                                    XPS_PER_DEVICE, &xpSaiAclTableGrpMembIdKeyComp,
                                    sXpSaiAclTableGrpMembIdArrayDbHndl);
        if (retVal != XP_NO_ERR)
        {
            sXpSaiAclTableGrpMembIdArrayDbHndl = XPS_STATE_INVALID_DB_HANDLE;
            XP_SAI_LOG_ERR("Error: Failed to register state database\n");
            return xpsStatus2SaiStatus(retVal);
        }

        if (INIT_COLD == xpsGetInitType())
        {
            retVal = xpsStateInsertDbForDevice(devId, sXpSaiAclTableGrpMembIdArrayDbHndl,
                                               xpSaiAclTableGrpMembIdKeyComp);
            if (retVal != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Error: Failed to add DB for device %u \n", devId);
                return xpsStatus2SaiStatus(retVal);
            }
        }
    }

    if (sXpSaiAclCounterIdArrayDbHndl == XPS_STATE_INVALID_DB_HANDLE)
    {
        sXpSaiAclCounterIdArrayDbHndl = XPSAI_ACL_COUNTERID_ARRAY_DB_HNDL;
        retVal = xpsStateRegisterDb(XP_SCOPE_DEFAULT, "SAI ACL CounterID Array",
                                    XPS_PER_DEVICE, &xpSaiAclCounterIdKeyComp, sXpSaiAclCounterIdArrayDbHndl);
        if (retVal != XP_NO_ERR)
        {
            sXpSaiAclCounterIdArrayDbHndl = XPS_STATE_INVALID_DB_HANDLE;
            XP_SAI_LOG_ERR("Error: Failed to register state database\n");
            return xpsStatus2SaiStatus(retVal);
        }

        if (INIT_COLD == xpsGetInitType())
        {
            retVal = xpsStateInsertDbForDevice(devId, sXpSaiAclCounterIdArrayDbHndl,
                                               xpSaiAclCounterIdKeyComp);
            if (retVal != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Error: Failed to add DB for device %u \n", devId);
                return xpsStatus2SaiStatus(retVal);
            }
        }
    }

    if (sXpSaiAclEntryIdArrayDbHndl == XPS_STATE_INVALID_DB_HANDLE)
    {
        sXpSaiAclEntryIdArrayDbHndl = XPSAI_ACL_ENTRY_ID_ARRAY_DB_HNDL;
        retVal = xpsStateRegisterDb(XP_SCOPE_DEFAULT, "SAI ACL EntryID Array",
                                    XPS_PER_DEVICE, &xpSaiAclEntryIdKeyComp, sXpSaiAclEntryIdArrayDbHndl);
        if (retVal != XP_NO_ERR)
        {
            sXpSaiAclEntryIdArrayDbHndl = XPS_STATE_INVALID_DB_HANDLE;
            XP_SAI_LOG_ERR("Error: Failed to register state database\n");
            return xpsStatus2SaiStatus(retVal);
        }

        if (INIT_COLD == xpsGetInitType())
        {
            retVal = xpsStateInsertDbForDevice(devId, sXpSaiAclEntryIdArrayDbHndl,
                                               xpSaiAclEntryIdKeyComp);
            if (retVal != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Error: Failed to add DB for device %u \n", devId);
                return xpsStatus2SaiStatus(retVal);
            }
        }
    }
    if (sXpSaiAclRangeIdDbHndl == XPS_STATE_INVALID_DB_HANDLE)
    {
        sXpSaiAclRangeIdDbHndl = XPSAI_ACL_TCP_UDP_PORT_RANGE_DB_HNDL;
        retVal = xpsStateRegisterDb(XP_SCOPE_DEFAULT, "SAI ACL RangeID Array",
                                    XPS_PER_DEVICE, &xpSaiAclRangeIdKeyComp, sXpSaiAclRangeIdDbHndl);
        if (retVal != XP_NO_ERR)
        {
            sXpSaiAclRangeIdDbHndl = XPS_STATE_INVALID_DB_HANDLE;
            XP_SAI_LOG_ERR("Error: Failed to register state database\n");
            return xpsStatus2SaiStatus(retVal);
        }

        if (INIT_COLD == xpsGetInitType())
        {
            retVal = xpsStateInsertDbForDevice(devId, sXpSaiAclRangeIdDbHndl,
                                               xpSaiAclRangeIdKeyComp);
            if (retVal != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Error: Failed to add DB for device %u \n", devId);
                return xpsStatus2SaiStatus(retVal);
            }
        }
    }

    if (sXpSaiAclRsnCodeToQueueMapArrayDbHndl == XPS_STATE_INVALID_DB_HANDLE)
    {
        sXpSaiAclRsnCodeToQueueMapArrayDbHndl =
            XPSAI_ACL_RSN_CODE_TO_QUEUE_MAP_ARRAY_DB_HNDL;
        retVal = xpsStateRegisterDb(XP_SCOPE_DEFAULT,
                                    "SAI ACL Reason code to Queue Map Array", XPS_PER_DEVICE,
                                    &xpSaiAclRsnCodeToQueueMapKeyComp, sXpSaiAclRsnCodeToQueueMapArrayDbHndl);
        if (retVal != XP_NO_ERR)
        {
            sXpSaiAclRsnCodeToQueueMapArrayDbHndl = XPS_STATE_INVALID_DB_HANDLE;
            XP_SAI_LOG_ERR("Error: Failed to register state database\n");
            return xpsStatus2SaiStatus(retVal);
        }

        if (INIT_COLD == xpsGetInitType())
        {
            retVal = xpsStateInsertDbForDevice(devId, sXpSaiAclRsnCodeToQueueMapArrayDbHndl,
                                               xpSaiAclRsnCodeToQueueMapKeyComp);
            if (retVal != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Error: Failed to add DB for device %u \n", devId);
                return xpsStatus2SaiStatus(retVal);
            }
        }

    }

    keyConfig.numberOfUDB = 0;
    keyConfig.pclId0 = 0;
    keyConfig.pclId01 = 0;
    keyConfig.pclId1 = 0;
    keyConfig.lookup0NonIpKey = XPS_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    keyConfig.lookup0Ipv4Key = XPS_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E;
    keyConfig.lookup0Ipv6Key = XPS_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E;
    keyConfig.lookup1NonIpKey =XPS_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    keyConfig.lookup1Ipv4Key = XPS_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E;
    keyConfig.lookup1Ipv6Key = XPS_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E;
    // retVal = xpsAclTypeDefineKey(devId,  keyConfig);

    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to add DB for device %u \n", devId);
        return xpsStatus2SaiStatus(retVal);
    }

    return xpsStatus2SaiStatus(retVal);
}

sai_status_t
xpSaiAclMapperDeinit(xpsDevice_t devId)
{
    XP_STATUS                 retVal      = XP_NO_ERR;
    sai_status_t              saiRetVal   = SAI_STATUS_SUCCESS;
    xpSaiAclEntryIdMappingT*  infoEntry   = NULL;
    xpSaiAclTableIdMappingT   *pSaiAclTableAttribute = NULL;
    sai_uint32_t              countIdx    = 0;
    //sai_uint32_t              reasonCode  = 0;
    //sai_uint32_t              aclQueueId  = 0;
    xpsPolicerType_e             client = XP_INGRESS_FLOW_POLICER;
    //xpsInterfaceId_t          xpsIntf    = 0;
    sai_acl_action_data_t     action;


    retVal = xpsStateDeRegisterDb(XP_SCOPE_DEFAULT,
                                  &sXpSaiAclRsnCodeToQueueMapArrayDbHndl);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to deregister state database\n");
        return xpsStatus2SaiStatus(retVal);
    }

    retVal = xpsStateGetNextDataForDevice(XP_SCOPE_DEFAULT,
                                          sXpSaiAclEntryIdArrayDbHndl, (xpsDbKey_t)NULL, (void **)&infoEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to get next data from database\n");
        return xpsStatus2SaiStatus(retVal);
    }

    while (infoEntry != NULL)
    {
        for (countIdx = SAI_ACL_ENTRY_ATTR_ACTION_START;
             countIdx < SAI_ACL_ENTRY_ATTR_ACTION_END; countIdx++)
        {
            if (xpSaiValidateEntryListAttributes((sai_acl_entry_attr_t)countIdx) &&
                infoEntry->attrs.action[countIdx -
                                                 SAI_ACL_ENTRY_ATTR_ACTION_START].actionValue.enable)
            {
                if (infoEntry->attrs.action[countIdx-
                                            SAI_ACL_ENTRY_ATTR_ACTION_START].actionValue.parameter.objlist.list != NULL)
                {
                    if ((retVal = xpsStateHeapFree(infoEntry->attrs.action[countIdx -
                                                                                    SAI_ACL_ENTRY_ATTR_ACTION_START].actionValue.parameter.objlist.list)) !=
                        XP_NO_ERR)
                    {
                        XP_SAI_LOG_ERR("Failed to to free allocated memory\n");
                        return xpsStatus2SaiStatus(retVal);
                    }

                    infoEntry->attrs.action[countIdx -
                                                     SAI_ACL_ENTRY_ATTR_ACTION_START].actionValue.parameter.objlist.list = NULL;
                }
            }
        }
        // Delete Mapping between Reason code and Queue before entry delete.
#if 0 // SAI 1.2
        if (infoEntry->attrs.action[SAI_ACL_ENTRY_ATTR_ACTION_SET_CPU_QUEUE -
                                                                            SAI_ACL_ENTRY_ATTR_ACTION_START].actionValue.enable)
        {
            action = infoEntry->attrs.action[SAI_ACL_ENTRY_ATTR_ACTION_SET_CPU_QUEUE -
                                                                                     SAI_ACL_ENTRY_ATTR_ACTION_START].actionValue;
            saiRetVal = xpSaiQosQueueObjIdPortValueGet(action.parameter.oid, &xpsIntf,
                                                       &aclQueueId);
            if (SAI_STATUS_SUCCESS != saiRetVal)
            {
                XP_SAI_LOG_ERR("Failed to get Locat Queue ID |retVal = %d\n", saiRetVal);
                return saiRetVal;
            }

            if (infoEntry->attrs.action[SAI_ACL_ENTRY_ATTR_ACTION_SET_USER_TRAP_ID -
                                                                                   SAI_ACL_ENTRY_ATTR_ACTION_START].actionValue.enable)
            {
                action = infoEntry->attrs.action[SAI_ACL_ENTRY_ATTR_ACTION_SET_USER_TRAP_ID -
                                                                                            SAI_ACL_ENTRY_ATTR_ACTION_START].actionValue;

                saiRetVal = xpSaiHostInterfaceUserTrapReasonCodeByOid(action.parameter.oid,
                                                                      &reasonCode);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Could not get ACL reason code by trap OID\n");
                    return saiRetVal;
                }

            }
            else
            {
                // when reason code is not configured get it.
                reasonCode = XP_SAI_CONTROL_TRAP_RC_MIN + aclQueueId;
            }

            saiRetVal = xpSaiDeleteAclMapperRsnCodeToQueueMap(devId, reasonCode);
            if (SAI_STATUS_SUCCESS != saiRetVal)
            {
                XP_SAI_LOG_ERR("Failed to set reason code to default Queue |retVal = %d\n",
                               saiRetVal);
                return saiRetVal;
            }

        }
#endif

        // Deleting Poilcer Entry
        if (infoEntry->attrs.action[SAI_ACL_ENTRY_ATTR_ACTION_SET_POLICER -
                                                                          SAI_ACL_ENTRY_ATTR_ACTION_START].actionValue.enable)
        {
            action = infoEntry->attrs.action[SAI_ACL_ENTRY_ATTR_ACTION_SET_POLICER -
                                                                                   SAI_ACL_ENTRY_ATTR_ACTION_START].actionValue;
            /* Get table info */
            if ((saiRetVal = xpSaiAclTableAttributesGet(infoEntry->attrs.tableId,
                                                        &pSaiAclTableAttribute)) != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("%s:, Failed to get table %u related data\n", __FUNCNAME__,
                               infoEntry->attrs.tableId);
                return saiRetVal;
            }

            if (pSaiAclTableAttribute->stage == SAI_ACL_STAGE_INGRESS)
            {
                client = XP_INGRESS_FLOW_POLICER;
            }
            else
            {
                client = XP_EGRESS_FLOW_POLICER;
            }
            /* Unset the Acl to PolicerMap */
            if ((saiRetVal = xpSaiUnSetAclToPolicerMap(action.parameter.oid,
                                                       client)) != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Failed xpSaiUnSetAclToPolicerMap |Error: %d\n", saiRetVal);
                return saiRetVal;
            }

        }

        retVal = xpsStateGetNextDataForDevice(XP_SCOPE_DEFAULT,
                                              sXpSaiAclEntryIdArrayDbHndl, (xpsDbKey_t)infoEntry, (void **)&infoEntry);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Failed to get next data from database\n");
            return xpsStatus2SaiStatus(retVal);
        }

    }

    retVal = xpsStateDeRegisterDb(XP_SCOPE_DEFAULT, &sXpSaiAclEntryIdArrayDbHndl);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to deregister Entry state database\n");
        return xpsStatus2SaiStatus(retVal);
    }

    retVal = xpsStateDeRegisterDb(XP_SCOPE_DEFAULT, &sXpSaiAclRangeIdDbHndl);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to deregister Entry state database\n");
        return xpsStatus2SaiStatus(retVal);
    }
    retVal = xpsStateDeRegisterDb(XP_SCOPE_DEFAULT, &sXpSaiAclCounterIdArrayDbHndl);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to deregister Counter state database\n");
        return xpsStatus2SaiStatus(retVal);
    }

    retVal = xpsStateDeRegisterDb(XP_SCOPE_DEFAULT,
                                  &sXpSaiAclTableGrpMembIdArrayDbHndl);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to deregister TableGrpMemb state database\n");
        return xpsStatus2SaiStatus(retVal);
    }

    retVal = xpsStateDeRegisterDb(XP_SCOPE_DEFAULT,
                                  &sXpSaiAclTableGrpIdArrayDbHndl);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to deregister TableGrpMemb state database\n");
        return xpsStatus2SaiStatus(retVal);
    }

    retVal = xpsStateDeRegisterDb(XP_SCOPE_DEFAULT, &sXpSaiAclTableIdArrayDbHndl);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to deregister Table state database\n");
        return xpsStatus2SaiStatus(retVal);
    }

    return xpsStatus2SaiStatus(retVal);
}

sai_status_t
xpSaiAclMapperTableSizeSet(xpSaiAclTableIdMappingT *keyFormat,
                           sai_uint32_t tableSize, sai_uint32_t keySize)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;

    /* Check if table size is exceeding limits */
    if (tableSize > ((XP_SAI_ACL_TCAM_DB_SIZE * XP_SAI_ACL_NUM_DB) / keySize))
    {
        XP_SAI_LOG_ERR("Invalid input parameter. Table size is higher "
                       "than possible supported. Requested size is %u and allowed size is %u \n",
                       tableSize, ((XP_SAI_ACL_TCAM_DB_SIZE * XP_SAI_ACL_NUM_DB) / keySize));
        return SAI_STATUS_FAILURE;
    }

    if (tableSize == XP_SAI_ACL_TABLE_SIZE_DEFAULT)
    {
        /* Set table size for default case */
        keyFormat->tableSize = ((XP_SAI_ACL_TCAM_DB_SIZE * XP_SAI_ACL_NUM_DB) /
                                keySize);
    }
    else
    {
        /* Set table size */
        keyFormat->tableSize = tableSize;
    }


    return retVal;
}

sai_status_t
xpSaiAclMapperCounterIdGet(sai_uint32_t devId, sai_uint32_t tableId,
                           sai_uint32_t *counterId)
{
    XP_STATUS rc;
    xpSaiAclCounterIdMappingT *keyFormat;
    sai_uint32_t tblId = 0;

    if (counterId == NULL)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    // Get per table counter ID
    rc = xpsAllocatorAllocateId(XP_SCOPE_DEFAULT, XP_SAI_ALLOC_ACL_COUNTER_START,
                                counterId);
    if (rc != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("SAI ACL mapper counter ID allocation failed.\n");
        return xpsStatus2SaiStatus(rc);
    }

    // Add table id to the counter
    *counterId += tblId << XP_SAI_ACL_TBL_ID_SHIFT;

    rc = xpsStateHeapMalloc(sizeof(xpSaiAclCounterIdMappingT), (void**)&keyFormat);
    if (rc != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("SAI ACL mapper counter data allocation failed.\n");
        return xpsStatus2SaiStatus(rc);
    }
    if (keyFormat == NULL)
    {
        XP_SAI_LOG_ERR("SAI ACL mapper counter data allocation failed.\n");
        return SAI_STATUS_FAILURE;
    }

    memset(keyFormat, 0x00, sizeof(xpSaiAclCounterIdMappingT));

    keyFormat->counterId = *counterId;
    keyFormat->numEntries   = 0;

    /* Insert SAI ACL entry structure into DB */
    rc = xpsStateInsertDataForDevice(devId, sXpSaiAclCounterIdArrayDbHndl,
                                     (void*)keyFormat);
    if (rc != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to add DB to the device %u \n", devId);
        xpsStateHeapFree((void*)keyFormat);
        return xpsStatus2SaiStatus(rc);
    }

    return xpsStatus2SaiStatus(rc);
}

sai_status_t
xpSaiAclMapperCounterIdDelete(sai_uint32_t devId, sai_uint32_t counterId)
{
    XP_STATUS                  rc = XP_NO_ERR;
    xpSaiAclCounterIdMappingT  *infoCounter = NULL;
    xpSaiAclCounterIdMappingT  keyCounter;
    sai_uint32_t               entryId = 0;
    sai_uint32_t               tblId = 0;
    uint32_t                   rawCounterId = 0;

    rc = xpSaiAclCounterMappingGet(devId, counterId, &infoCounter);
    if (rc != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to find DB entry for counter %u\n", counterId);
        return xpsStatus2SaiStatus(rc);
    }

    if (infoCounter->numEntries != 0)
    {
        XP_SAI_LOG_ERR("Error: Can't delete counter, counter attached to some Entry(s) %u \n",
                       entryId);
        return SAI_STATUS_FAILURE;
    }


    if ((counterId >> XP_SAI_ACL_TBL_ID_SHIFT) != tblId)
    {
        XP_SAI_LOG_ERR("Error: Invalid counter %u\n", counterId);
        return xpsStatus2SaiStatus(XP_ERR_INVALID_ARG);
    }

    rawCounterId = counterId - (tblId << XP_SAI_ACL_TBL_ID_SHIFT);

    rc = xpsAllocatorReleaseId(XP_SCOPE_DEFAULT, XP_SAI_ALLOC_ACL_COUNTER_START,
                               rawCounterId);
    if (rc != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("SAI ACL mapper counter ID release failed.\n");
        return xpsStatus2SaiStatus(rc);
    }

    keyCounter.counterId = counterId;
    rc = xpsStateDeleteDataForDevice(devId, sXpSaiAclCounterIdArrayDbHndl,
                                     (xpsDbKey_t)&keyCounter, (void**)&infoCounter);
    if (rc != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to remove DB from device %u \n", devId);
        return xpsStatus2SaiStatus(rc);
    }
    if (infoCounter == NULL)
    {
        XP_SAI_LOG_ERR("Error: Failed to remove DB from device %u \n", devId);
        return SAI_STATUS_ITEM_NOT_FOUND;
    }
    memset(infoCounter, 0, sizeof(xpSaiAclCounterIdMappingT));

    xpsStateHeapFree((void*)infoCounter);

    return xpsStatus2SaiStatus(rc);
}

sai_status_t
xpSaiAclMapperCounterIdAttributesUpdate(sai_uint32_t counterId,
                                        xpSaiAclCounterAttributesT attributes)
{
    XP_STATUS                 rc = XP_NO_ERR;
    sai_uint32_t              devId = xpSaiGetDevId();
    xpSaiAclCounterIdMappingT *info;

    rc = xpSaiAclCounterMappingGet(devId, counterId, &info);
    if (rc != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to find DB entry for counter %u \n", counterId);
        return xpsStatus2SaiStatus(rc);
    }

    memcpy(&info->attr, &attributes, sizeof(xpSaiAclCounterAttributesT));

    return xpsStatus2SaiStatus(rc);
}

sai_status_t xpSaiCounterRefresh(sai_uint32_t counterId, sai_uint32_t tableId,
                                 sai_uint32_t entryId)
{
    sai_status_t               retVal      = SAI_STATUS_SUCCESS;
    sai_uint32_t               count       = 0;
    sai_uint32_t               ruleId      = 0;
    xpSaiAclTableIdMappingT    *pSaiAclTableAttribute = NULL;
    xpSaiAclEntryAttributesT   *pSaiAclEntryAttribute= NULL;
    xpsDevice_t     devId       = xpSaiGetDevId();

    retVal = xpSaiAclMapperEntryAttributesGet(entryId, &pSaiAclEntryAttribute);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get entry attributes from DB\n");
        return retVal;
    }

    retVal = xpSaiAclTableAttributesGet(tableId, &pSaiAclTableAttribute);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get table %u related data\n", tableId);
        return retVal;
    }

    for (count = 0; count < pSaiAclEntryAttribute->rule_count; count++)
    {
        retVal = xpSaiAclMapperRuleIdGet(devId, tableId,
                                         pSaiAclEntryAttribute->ruleMap[count].id, &ruleId);
        if (retVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Error: Failed to get TCAM ID, rc %u \n", retVal);
            return retVal;
        }

        retVal = xpSaiAclCounterUpdate(devId, counterId, tableId,
                                       pSaiAclTableAttribute->stage, ruleId);
        if (retVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to update a counter, rc %u\n", retVal);
            return retVal;
        }
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t
xpSaiAclMapperCounterIdAttributesGet(sai_uint32_t counterId,
                                     xpSaiAclCounterAttributesT *attributes)
{
    sai_uint32_t              devId = xpSaiGetDevId();
    sai_uint32_t              index = 0;
    xpSaiAclCounterIdMappingT *pSaiAclCounterIdMapping = NULL;
    XP_STATUS                 rc = XP_NO_ERR;
    sai_status_t              retVal = SAI_STATUS_SUCCESS;

    if (attributes == NULL)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    rc = xpSaiAclCounterMappingGet(devId, counterId, &pSaiAclCounterIdMapping);
    if (rc != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to find DB entry for counter %u \n", counterId);
        return xpsStatus2SaiStatus(rc);
    }

    if (!pSaiAclCounterIdMapping->attr.enablePacketCount)
    {
        XP_SAI_LOG_ERR("packet Counting is not Enable for counter %u \n", counterId);
        return SAI_STATUS_FAILURE;
    }

    for (index = 0; index < pSaiAclCounterIdMapping->numEntries; index++)
    {
        retVal = xpSaiCounterRefresh(counterId, pSaiAclCounterIdMapping->attr.tableId,
                                     pSaiAclCounterIdMapping->entryId[index]);
        if (retVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to update a counter, rc %u\n", retVal);
            return retVal;
        }
    }

    memcpy(attributes, &pSaiAclCounterIdMapping->attr,
           sizeof(xpSaiAclCounterAttributesT));

    return retVal;
}

sai_status_t
xpSaiAclMapperAddEntryInTable(sai_uint32_t tableId, sai_uint32_t entryId)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpSaiAclTableIdMappingT *keyFormat = NULL;
    xpSaiAclTableIdMappingT *newKeyFormat = NULL;
    xpSaiAclTableIdMappingT dbKey;
    xpsDevice_t devId = xpSaiGetDevId();

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    dbKey.tableId = tableId;

    /* Search corresponding table API */
    retVal = xpsStateSearchDataForDevice(devId, sXpSaiAclTableIdArrayDbHndl,
                                         (xpsDbKey_t)&dbKey, (void**)&keyFormat);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("XP_SAI_LOG_ERR() failed, error %d\n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    if (keyFormat == NULL)
    {
        XP_SAI_LOG_ERR("Key not found\n");
        return xpsStatus2SaiStatus(XP_ERR_KEY_NOT_FOUND);
    }

    if (xpsDAIsCtxGrowthNeeded(keyFormat->numEntries, XP_SAI_ENTRY_MAX))
    {
        // Double the queueList array size
        retVal = xpSaiDynamicArrayGrow((void**)&newKeyFormat, (void*)keyFormat,
                                       sizeof(xpSaiAclTableIdMappingT),
                                       sizeof(sai_uint32_t), keyFormat->numEntries,
                                       XP_SAI_ENTRY_MAX);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Dynamic array grow failed |Error code: %d\n", retVal);
            return xpsStatus2SaiStatus(retVal);
        }

        // Remove the corresponding state from Table db and add new state
        retVal = xpsStateDeleteDataForDevice(devId, sXpSaiAclTableIdArrayDbHndl,
                                             (xpsDbKey_t)&dbKey, (void**)&keyFormat);
        if (retVal != XP_NO_ERR)
        {
            xpsStateHeapFree((void*)newKeyFormat);
            XP_SAI_LOG_ERR("Deletion of data failed |Error code: %d\n", retVal);
            return xpsStatus2SaiStatus(retVal);
        }

        // Free the memory allocated for the corresponding state
        retVal = xpsStateHeapFree(keyFormat);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Freeing allocated memory failed |Error code: %d\n", retVal);
            xpsStateHeapFree((void*)newKeyFormat);
            return xpsStatus2SaiStatus(retVal);
        }

        // Insert the new state
        retVal = xpsStateInsertDataForDevice(devId, sXpSaiAclTableIdArrayDbHndl,
                                             (void*)newKeyFormat);
        if (retVal != XP_NO_ERR)
        {
            xpsStateHeapFree((void*)newKeyFormat);
            return xpsStatus2SaiStatus(retVal);
        }

        keyFormat = newKeyFormat;
    }

    keyFormat->entryId[keyFormat->numEntries++] = entryId;

    return xpsStatus2SaiStatus(retVal);
}

sai_status_t
xpSaiAclMapperRemoveEntryInTable(sai_uint32_t tableId, sai_uint32_t entryId)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpSaiAclTableIdMappingT *keyFormat = NULL;
    xpSaiAclTableIdMappingT *newKeyFormat = NULL;
    xpSaiAclTableIdMappingT dbKey;
    xpsDevice_t devId = xpSaiGetDevId();
    sai_uint8_t isEnrtyIdPresent = 0;

    dbKey.tableId = tableId;

    /* Search corresponding table API */
    if ((retVal = xpsStateSearchDataForDevice(devId, sXpSaiAclTableIdArrayDbHndl,
                                              (xpsDbKey_t)&dbKey, (void**)&keyFormat)) != XP_NO_ERR)
    {
        return xpsStatus2SaiStatus(retVal);
    }

    if (keyFormat == NULL)
    {
        return xpsStatus2SaiStatus(XP_ERR_KEY_NOT_FOUND);
    }

    // Remove the Entry Id from Table
    for (sai_uint16_t count = 0; count < keyFormat->numEntries; count++)
    {
        if (!isEnrtyIdPresent)
        {
            if (keyFormat->entryId[count] == entryId)
            {
                isEnrtyIdPresent = 1;
            }
            continue;
        }
        else
        {
            keyFormat->entryId[count-1] = keyFormat->entryId[count];
        }
    }

    if (!isEnrtyIdPresent)
    {
        // Counter already removed/not present in the entry
        return xpsStatus2SaiStatus(XP_NO_ERR);
    }

    // Decrement the numEntries count
    keyFormat->numEntries--;
    if (xpsDAIsCtxShrinkNeeded(keyFormat->numEntries, XP_SAI_ENTRY_MAX))
    {
        // Shrink the queueList array size
        if ((retVal = xpSaiDynamicArrayShrink((void**)&newKeyFormat, (void*)keyFormat,
                                              sizeof(xpSaiAclTableIdMappingT), sizeof(sai_uint32_t), keyFormat->numEntries,
                                              XP_SAI_ENTRY_MAX)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Dynamic array grow failed |Error code: %d\n", retVal);
            return xpsStatus2SaiStatus(retVal);
        }

        // Remove the corresponding state from sai table db and add new state
        if ((retVal = xpsStateDeleteDataForDevice(devId, sXpSaiAclTableIdArrayDbHndl,
                                                  (xpsDbKey_t)&dbKey, (void**)&keyFormat)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Deletion of data failed |Error code: %d\n", retVal);
            xpsStateHeapFree((void*)newKeyFormat);
            return xpsStatus2SaiStatus(retVal);
        }

        // Free the memory allocated for the corresponding state
        if ((retVal = xpsStateHeapFree(keyFormat)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Freeing allocated memory failed |Error code: %d\n", retVal);
            xpsStateHeapFree((void*)newKeyFormat);
            return xpsStatus2SaiStatus(retVal);
        }

        // Insert the new state
        if ((retVal = xpsStateInsertDataForDevice(devId, sXpSaiAclTableIdArrayDbHndl,
                                                  (void*)newKeyFormat)) != XP_NO_ERR)
        {
            xpsStateHeapFree((void*)newKeyFormat);
            return xpsStatus2SaiStatus(retVal);
        }
    }

    return xpsStatus2SaiStatus(retVal);
}

sai_status_t
xpSaiAclMapperAddEntryInCounter(sai_uint32_t counterId, sai_uint32_t entryId)
{
    XP_STATUS                 retVal = XP_NO_ERR;
    sai_status_t              saiRetVal = SAI_STATUS_SUCCESS;
    sai_uint32_t              index = 0;
    xpSaiAclCounterIdMappingT *keyFormat = NULL;
    xpSaiAclCounterIdMappingT *newKeyFormat = NULL;
    xpSaiAclCounterIdMappingT dbKey;
    xpSaiAclEntryAttributesT *pSaiAclEntryAttributes = NULL;
    xpsDevice_t devId = xpSaiGetDevId();

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    dbKey.counterId = counterId;

    /* Search corresponding table API */
    retVal = xpsStateSearchDataForDevice(devId, sXpSaiAclCounterIdArrayDbHndl,
                                         (xpsDbKey_t)&dbKey, (void**)&keyFormat);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("XP_SAI_LOG_ERR() failed, error %d\n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    if (keyFormat == NULL)
    {
        XP_SAI_LOG_ERR("Key not found\n");
        return xpsStatus2SaiStatus(XP_ERR_KEY_NOT_FOUND);
    }

    saiRetVal = xpSaiAclMapperEntryAttributesGet(entryId, &pSaiAclEntryAttributes);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get entry attributes from DB\n");
        return saiRetVal;
    }

    if (keyFormat->attr.tableOid != pSaiAclEntryAttributes->tableOid)
    {
        XP_SAI_LOG_ERR("Failed Counter and Entry is not belong to same table\n");
        return SAI_STATUS_FAILURE;
    }

    for (index = 0; index < keyFormat->numEntries; index++)
    {
        if (entryId == keyFormat->entryId[index])
        {
            return SAI_STATUS_SUCCESS;
        }
    }

    if (xpsDAIsCtxGrowthNeeded(keyFormat->numEntries, XP_SAI_ENTRY_MAX))
    {
        // Double the queueList array size
        retVal = xpSaiDynamicArrayGrow((void**)&newKeyFormat, (void*)keyFormat,
                                       sizeof(xpSaiAclCounterIdMappingT),
                                       sizeof(sai_uint32_t), keyFormat->numEntries,
                                       XP_SAI_ENTRY_MAX);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Dynamic array grow failed |Error code: %d\n", retVal);
            return xpsStatus2SaiStatus(retVal);
        }

        // Remove the corresponding state from counter db and add new state
        retVal = xpsStateDeleteDataForDevice(devId, sXpSaiAclCounterIdArrayDbHndl,
                                             (xpsDbKey_t)&dbKey, (void**)&keyFormat);
        if (retVal != XP_NO_ERR)
        {
            xpsStateHeapFree((void*)newKeyFormat);
            XP_SAI_LOG_ERR("Deletion of data failed |Error code: %d\n", retVal);
            return xpsStatus2SaiStatus(retVal);
        }

        // Free the memory allocated for the corresponding state
        retVal = xpsStateHeapFree(keyFormat);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Freeing allocated memory failed |Error code: %d\n", retVal);
            xpsStateHeapFree((void*)newKeyFormat);
            return xpsStatus2SaiStatus(retVal);
        }

        // Insert the new state
        retVal = xpsStateInsertDataForDevice(devId, sXpSaiAclCounterIdArrayDbHndl,
                                             (void*)newKeyFormat);
        if (retVal != XP_NO_ERR)
        {
            xpsStateHeapFree((void*)newKeyFormat);
            return xpsStatus2SaiStatus(retVal);
        }

        keyFormat = newKeyFormat;
    }

    keyFormat->entryId[keyFormat->numEntries++] = entryId;

    return xpsStatus2SaiStatus(retVal);
}

sai_status_t
xpSaiAclMapperRemoveEntryInCounter(sai_uint32_t counterId, sai_uint32_t entryId)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpSaiAclCounterIdMappingT *keyFormat = NULL;
    xpSaiAclCounterIdMappingT *newKeyFormat = NULL;
    xpSaiAclCounterIdMappingT dbKey;
    xpsDevice_t devId = xpSaiGetDevId();
    sai_uint8_t isEnrtyIdPresent = 0;

    dbKey.counterId = counterId;

    /* Search corresponding table API */
    if ((retVal = xpsStateSearchDataForDevice(devId, sXpSaiAclCounterIdArrayDbHndl,
                                              (xpsDbKey_t)&dbKey, (void**)&keyFormat)) != XP_NO_ERR)
    {
        return xpsStatus2SaiStatus(retVal);
    }

    if (keyFormat == NULL)
    {
        return xpsStatus2SaiStatus(XP_ERR_KEY_NOT_FOUND);
    }

    // Remove the Entry Id from Table
    for (sai_uint16_t count = 0; count < keyFormat->numEntries; count++)
    {
        if (!isEnrtyIdPresent)
        {
            if (keyFormat->entryId[count] == entryId)
            {
                isEnrtyIdPresent = 1;
            }
            continue;
        }
        else
        {
            keyFormat->entryId[count-1] = keyFormat->entryId[count];
        }
    }

    if (!isEnrtyIdPresent)
    {
        // Counter already removed/not present in the entry
        return xpsStatus2SaiStatus(XP_NO_ERR);
    }

    // Decrement the numEntries count
    keyFormat->numEntries--;
    if (xpsDAIsCtxShrinkNeeded(keyFormat->numEntries, XP_SAI_ENTRY_MAX))
    {
        // Shrink the queueList array size
        if ((retVal = xpSaiDynamicArrayShrink((void**)&newKeyFormat, (void*)keyFormat,
                                              sizeof(xpSaiAclCounterIdMappingT), sizeof(sai_uint32_t), keyFormat->numEntries,
                                              XP_SAI_ENTRY_MAX)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Dynamic array grow failed |Error code: %d\n", retVal);
            return xpsStatus2SaiStatus(retVal);
        }

        // Remove the corresponding state from sai counter db and add new state
        if ((retVal = xpsStateDeleteDataForDevice(devId, sXpSaiAclCounterIdArrayDbHndl,
                                                  (xpsDbKey_t)&dbKey, (void**)&keyFormat)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Deletion of data failed |Error code: %d\n", retVal);
            xpsStateHeapFree((void*)newKeyFormat);
            return xpsStatus2SaiStatus(retVal);
        }

        // Free the memory allocated for the corresponding state
        if ((retVal = xpsStateHeapFree(keyFormat)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Freeing allocated memory failed |Error code: %d\n", retVal);
            xpsStateHeapFree((void*)newKeyFormat);
            return xpsStatus2SaiStatus(retVal);
        }

        // Insert the new state
        if ((retVal = xpsStateInsertDataForDevice(devId, sXpSaiAclCounterIdArrayDbHndl,
                                                  (void*)newKeyFormat)) != XP_NO_ERR)
        {
            xpsStateHeapFree((void*)newKeyFormat);
            return xpsStatus2SaiStatus(retVal);
        }
    }

    return xpsStatus2SaiStatus(retVal);
}


sai_status_t
xpSaiAclMapperEntryCreate(sai_uint32_t* entryId)
{
    XP_STATUS                rc         = XP_NO_ERR;
    xpSaiAclEntryIdMappingT* pEntry     = NULL;

    sai_uint32_t devId = xpSaiGetDevId();

    rc = xpsAllocatorAllocateId(XP_SCOPE_DEFAULT, XP_SAI_ALLOC_ACL_ENTRY, entryId);
    if (rc != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not allocate ACL entry id, error %d\n", rc);
        return xpsStatus2SaiStatus(rc);
    }

    rc = xpsStateHeapMalloc(sizeof(xpSaiAclEntryIdMappingT), (void**)&pEntry);
    if (rc != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("SAI ACL mapper entry data allocation failed.\n");
        return xpsStatus2SaiStatus(rc);
    }

    if (pEntry == NULL)
    {
        XP_SAI_LOG_ERR("SAI ACL mapper entry data allocation failed.\n");
        return SAI_STATUS_NO_MEMORY;
    }

    memset(pEntry, 0x00, sizeof(xpSaiAclEntryIdMappingT));

    pEntry->entryId = *entryId;

    /* Insert SAI ACL entry structure into DB */
    rc = xpsStateInsertDataForDevice(devId, sXpSaiAclEntryIdArrayDbHndl,
                                     (void*)pEntry);
    if (rc != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to add entry to DB, device %u \n", devId);
        xpsStateHeapFree((void*)pEntry);
        return xpsStatus2SaiStatus(rc);
    }

    return SAI_STATUS_SUCCESS;
}


sai_status_t
xpSaiAclMapperAclRangeCreate(sai_uint32_t* entryId)
{
    XP_STATUS                rc         = XP_NO_ERR;
    xpSaiAclRangeIdMappingT* pEntry     = NULL;

    sai_uint32_t devId = xpSaiGetDevId();

    rc = xpsAllocatorAllocateId(XP_SCOPE_DEFAULT,
                                XP_SAI_ALLOC_ACL_TCP_UDP_PORT_RANGE, entryId);
    if (rc != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not allocate ACL range id, error %d\n", rc);
        return xpsStatus2SaiStatus(rc);
    }

    rc = xpsStateHeapMalloc(sizeof(xpSaiAclRangeIdMappingT), (void**)&pEntry);
    if (rc != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("SAI ACL mapper range data allocation failed.\n");
        return xpsStatus2SaiStatus(rc);
    }

    if (pEntry == NULL)
    {
        XP_SAI_LOG_ERR("SAI ACL mapper range data allocation failed.\n");
        return SAI_STATUS_NO_MEMORY;
    }

    memset(pEntry, 0x00, sizeof(xpSaiAclRangeIdMappingT));

    pEntry->entryId = *entryId;

    /* Insert SAI ACL range structure into DB */
    rc = xpsStateInsertDataForDevice(devId, sXpSaiAclRangeIdDbHndl, (void*)pEntry);
    if (rc != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to add entry to DB, device %u \n", devId);
        xpsStateHeapFree((void*)pEntry);
        return xpsStatus2SaiStatus(rc);
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t
xpSaiAclTableAttributesGet(sai_uint32_t tableId,
                           xpSaiAclTableIdMappingT **pSaiAclTableAttribute)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpSaiAclTableIdMappingT dbKey;
    xpsDevice_t devId = xpSaiGetDevId();

    dbKey.tableId = tableId;

    /* Search corresponding table API */
    if ((retVal = xpsStateSearchDataForDevice(devId, sXpSaiAclTableIdArrayDbHndl,
                                              (xpsDbKey_t)&dbKey, (void**)pSaiAclTableAttribute)) != XP_NO_ERR)
    {
        return xpsStatus2SaiStatus(retVal);
    }

    if (*pSaiAclTableAttribute == NULL)
    {
        XP_SAI_LOG_ERR("Key is not valid, not found.\n");
        return SAI_STATUS_INVALID_OBJECT_ID;
    }
    return xpsStatus2SaiStatus(retVal);
}

sai_status_t
xpSaiAclStaticTableMappingGet(sai_uint32_t tableId,
                              xpSaiAclStaticTableMappingT **pSaiAclStaticTableMappingT)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpSaiAclStaticTableMappingT dbKey;
    xpsDevice_t devId = xpSaiGetDevId();

    dbKey.tableId = tableId;

    /* Search corresponding table API */
    if ((retVal = xpsStateSearchDataForDevice(devId, sXpSaiAclStaticTableDbHndl,
                                              (xpsDbKey_t)&dbKey, (void**)pSaiAclStaticTableMappingT)) != XP_NO_ERR)
    {
        return xpsStatus2SaiStatus(retVal);
    }

    if (*pSaiAclStaticTableMappingT == NULL)
    {
        XP_SAI_LOG_ERR("Key is not valid, not found.\n");
        return SAI_STATUS_ITEM_NOT_FOUND;;
    }
    return xpsStatus2SaiStatus(retVal);
}

sai_status_t
xpSaiAclMapperEntryAttributesSet(sai_uint32_t entry_id,
                                 xpSaiAclEntryAttributesT* pAttributes)
{
    XP_STATUS                rc     = XP_NO_ERR;
    xpSaiAclEntryIdMappingT* pEntry = NULL;
    static xpSaiAclEntryIdMappingT  key;

    sai_uint32_t devId = xpSaiGetDevId();

    if (pAttributes == NULL)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    memset(&key, 0x00, sizeof(key));
    key.entryId = entry_id;

    rc = xpsStateSearchDataForDevice(devId, sXpSaiAclEntryIdArrayDbHndl,
                                     (xpsDbKey_t)&key, (void**)&pEntry);
    if ((rc != XP_NO_ERR) || (pEntry == NULL))
    {
        XP_SAI_LOG_ERR("Could not find entry %u in DB\n", entry_id);
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    /* Assign values */
    memcpy(&pEntry->attrs, pAttributes, sizeof(xpSaiAclEntryAttributesT));

    return SAI_STATUS_SUCCESS;
}

sai_status_t
xpSaiAclMapperEntryAttributesGet(sai_uint32_t entry_id,
                                 xpSaiAclEntryAttributesT** pAttributes)
{
    XP_STATUS                retVal  = XP_NO_ERR;
    xpSaiAclEntryIdMappingT* pEntry = NULL;
    static xpSaiAclEntryIdMappingT  key;
    sai_uint32_t             devId = xpSaiGetDevId();

    memset(&key, 0x00, sizeof(key));
    key.entryId = entry_id;

    retVal = xpsStateSearchDataForDevice(devId, sXpSaiAclEntryIdArrayDbHndl,
                                         (xpsDbKey_t)&key, (void**)&pEntry);
    if (retVal != XP_NO_ERR)
    {
        return xpsStatus2SaiStatus(retVal);
    }

    if (pEntry == NULL)
    {
        return xpsStatus2SaiStatus(XP_ERR_KEY_NOT_FOUND);
    }

    *pAttributes = &pEntry->attrs;

    return SAI_STATUS_SUCCESS;
}

sai_status_t
xpSaiAclMapperAclRangeAttributesSet(sai_uint32_t entry_id,
                                    xpSaiAclRangeAttributesT* pAttributes)
{
    XP_STATUS                rc     = XP_NO_ERR;
    xpSaiAclRangeIdMappingT* pEntry = NULL;
    static xpSaiAclRangeIdMappingT  key;

    sai_uint32_t devId = xpSaiGetDevId();

    if (pAttributes == NULL)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    memset(&key, 0x00, sizeof(key));
    key.entryId = entry_id;

    rc = xpsStateSearchDataForDevice(devId, sXpSaiAclRangeIdDbHndl,
                                     (xpsDbKey_t)&key, (void**)&pEntry);
    if ((rc != XP_NO_ERR) || (pEntry == NULL))
    {
        XP_SAI_LOG_ERR("Could not find entry %u in DB\n", entry_id);
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    /* Assign values */
    memcpy(&pEntry->attrs, pAttributes, sizeof(xpSaiAclRangeAttributesT));

    return SAI_STATUS_SUCCESS;
}


sai_status_t
xpSaiAclMapperAclRangeAttributesGet(sai_uint32_t entry_id,
                                    xpSaiAclRangeAttributesT** pAttributes)
{
    XP_STATUS                retVal  = XP_NO_ERR;
    xpSaiAclRangeIdMappingT* pEntry = NULL;
    static xpSaiAclRangeIdMappingT  key;
    sai_uint32_t             devId = xpSaiGetDevId();

    memset(&key, 0x00, sizeof(key));
    key.entryId = entry_id;

    retVal = xpsStateSearchDataForDevice(devId, sXpSaiAclRangeIdDbHndl,
                                         (xpsDbKey_t)&key, (void**)&pEntry);
    if (retVal != XP_NO_ERR)
    {
        return xpsStatus2SaiStatus(retVal);
    }

    if (pEntry == NULL)
    {
        return xpsStatus2SaiStatus(XP_ERR_KEY_NOT_FOUND);
    }

    *pAttributes = &pEntry->attrs;

    return SAI_STATUS_SUCCESS;
}

sai_status_t
xpSaiAclMapperAclRangeDelete(xpsDevice_t devId, sai_uint32_t entryId)
{
    XP_STATUS                   xpStatus    = XP_NO_ERR;
    xpSaiAclRangeIdMappingT*    infoEntry   = NULL;
    static xpSaiAclRangeIdMappingT     key;
    //sai_uint32_t                countIdx = 0;

    memset(&key, 0x00, sizeof(key));
    key.entryId = entryId;

    xpStatus = xpsStateDeleteDataForDevice(devId, sXpSaiAclRangeIdDbHndl,
                                           (xpsDbKey_t)&key, (void**)&infoEntry);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to delete DB entry for entry %u \n", entryId);
        return xpsStatus2SaiStatus(xpStatus);
    }

    xpsStateHeapFree((void*)infoEntry);

    xpStatus = xpsAllocatorReleaseId(XP_SCOPE_DEFAULT,
                                     XP_SAI_ALLOC_ACL_TCP_UDP_PORT_RANGE, entryId);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not release ACL range id, error %d\n", xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t
xpSaiAclMapperTableAttributesSet(sai_uint32_t tableId,
                                 xpSaiAclTableAttributesT* pAttributes, sai_uint32_t keySize)
{
    sai_status_t            retVal = SAI_STATUS_SUCCESS;
    xpSaiAclTableIdMappingT *keyFormat = NULL;
    xpSaiAclTableIdMappingT dbKey;
    xpsDevice_t             devId = xpSaiGetDevId();

    if (pAttributes == NULL)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    dbKey.tableId = tableId;

    /* Search corresponding table API */
    retVal = xpsStateSearchDataForDevice(devId, sXpSaiAclTableIdArrayDbHndl,
                                         (xpsDbKey_t)&dbKey, (void**)&keyFormat);
    if (retVal != XP_NO_ERR)
    {
        return retVal;
    }

    if (keyFormat == NULL)
    {
        return XP_ERR_KEY_NOT_FOUND;
    }

    /* Assign values */
    keyFormat->stage = pAttributes->stage;

    //Currently supporting only one bind-type per table
    if (SAI_ACL_STAGE_INGRESS == pAttributes->stage)
    {
        if (retVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Failed to determine ACL type, error %d \n", retVal);
            return retVal;
        }
    }


    if (pAttributes->aclBindPoint == SAI_ACL_BIND_POINT_TYPE_SWITCH)
    {
        keyFormat->isSwitchAclEn = TRUE;
    }
    keyFormat->tableSize = pAttributes->tableSize;
    memcpy(keyFormat->match, pAttributes->match, sizeof(keyFormat->match));

    return SAI_STATUS_SUCCESS;
}

static sai_status_t xpSaiGetAclMapperTableAttribute(xpSaiAclTableIdMappingT
                                                    *keyFormat, sai_attribute_t *attr, sai_uint32_t attr_index)
{
    sai_status_t              saiRetVal    = SAI_STATUS_SUCCESS;
    sai_uint32_t              devId        = 0;
    XP_STATUS                 retVal       = XP_NO_ERR;

    switch (attr->id)
    {
        case SAI_ACL_TABLE_ATTR_ACL_STAGE:
            {
                attr->value.s32 = keyFormat->stage;
                break;
            }
        case SAI_ACL_TABLE_ATTR_ACL_BIND_POINT_TYPE_LIST:
            {
                attr->value.s32list.count= keyFormat->bindPointCount;
                memcpy(attr->value.s32list.list, keyFormat->bp_list,
                       sizeof(sai_uint32_t)*keyFormat->bindPointCount);
                break;
            }
        case SAI_ACL_TABLE_ATTR_SIZE:
            {
                attr->value.u32 = keyFormat->tableSize;
                break;
            }
        case SAI_ACL_TABLE_ATTR_AVAILABLE_ACL_ENTRY:
            {
                sai_uint32_t availableEntries;

                retVal = xpsAclGetAvailableEntries(devId, keyFormat->tableId,
                                                   &availableEntries);
                if (retVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("Error: Failed to get available entries for the table %u \n",
                                   keyFormat->tableId);
                    return xpsStatus2SaiStatus(retVal);
                }
                attr->value.u32 = availableEntries;
                break;
            }
        case SAI_ACL_TABLE_ATTR_AVAILABLE_ACL_COUNTER:
            {
                xpSaiTableEntryCountDbEntry *entryCountCtxPtr = NULL;
                saiRetVal = xpSaiGetTableEntryCountCtxDb(devId, &entryCountCtxPtr);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Error: Failed to get Table Entry Count DB\n");
                    return saiRetVal;
                }
                if (entryCountCtxPtr->aclCounters <= entryCountCtxPtr->max_aclCounters)
                {
                    attr->value.u32 = entryCountCtxPtr->max_aclCounters -
                                      entryCountCtxPtr->aclCounters;
                }
                else
                {
                    XP_SAI_LOG_WARNING("Used count more than maximum count.\n");
                    attr->value.u32 = 0;
                }
                break;
            }

        default:
            {
                sai_uint32_t matchId = attr->id - SAI_ACL_TABLE_ATTR_FIELD_START;

                if (matchId >= sizeof(keyFormat->match))
                {
                    XP_SAI_LOG_ERR("Table attribute %s(%d) is out of range\n",
                                   xpSaiTableAttrNameGet((sai_acl_table_attr_t)attr->id),
                                   attr->id);

                    return SAI_STATUS_UNKNOWN_ATTRIBUTE_0 + SAI_STATUS_CODE(attr_index);
                }

                if (keyFormat->match[matchId] == FALSE)
                {
                    XP_SAI_LOG_INFO("Table attribute %s(%d) is not set\n",
                                    xpSaiTableAttrNameGet((sai_acl_table_attr_t)attr->id),
                                    attr->id);
                }
                attr->value.booldata = keyFormat->match[matchId];
                break;
            }
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiGetAclMapperTableAttributes(sai_uint32_t tableId,
                                              sai_uint32_t attr_count, sai_attribute_t *attr_list)
{
    XP_STATUS               retVal     = XP_NO_ERR;
    sai_status_t            saiRetVal  = SAI_STATUS_SUCCESS;
    xpSaiAclTableIdMappingT *keyFormat = NULL;
    xpSaiAclTableIdMappingT dbKey;

    if (attr_list == NULL)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    dbKey.tableId = tableId;

    /* Search corresponding table API */
    if ((retVal = xpsStateSearchDataForDevice(xpSaiGetDevId(),
                                              sXpSaiAclTableIdArrayDbHndl, (xpsDbKey_t)&dbKey,
                                              (void**)&keyFormat)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateSearchDataForDevice failed with retVal: %d\n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }
    if (keyFormat == NULL)
    {
        XP_SAI_LOG_ERR("Key is not valid, not found.\n");
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    for (sai_uint32_t count = 0; count < attr_count; count++)
    {
        saiRetVal = xpSaiGetAclMapperTableAttribute(keyFormat, &attr_list[count],
                                                    count);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            return saiRetVal;
        }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiBulkGetAclMapperTableAttributes

sai_status_t xpSaiBulkGetAclMapperTableAttributes(sai_uint32_t tableId,
                                                  sai_uint32_t *attr_count, sai_attribute_t *attr_list)
{
    XP_STATUS               retVal     = XP_NO_ERR;
    sai_status_t            saiRetVal  = SAI_STATUS_SUCCESS;
    sai_uint32_t            idx        = 0;
    sai_uint32_t            count      = 0;
    sai_uint32_t            maxcount   = 0;
    xpSaiAclTableIdMappingT dbKey;
    xpSaiAclTableIdMappingT *keyFormat = NULL;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    /* Check incoming parameters */
    if ((attr_count == NULL) || (attr_list == NULL))
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiMaxCountAclTableAttribute(&maxcount);
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

    dbKey.tableId = tableId;

    /* Search corresponding table API */
    retVal = xpsStateSearchDataForDevice(xpSaiGetDevId(),
                                         sXpSaiAclTableIdArrayDbHndl, (xpsDbKey_t)&dbKey, (void**)&keyFormat);
    if (retVal != XP_NO_ERR)
    {
        return xpsStatus2SaiStatus(retVal);
    }
    if (keyFormat == NULL)
    {
        return xpsStatus2SaiStatus(XP_ERR_KEY_NOT_FOUND);
    }

    for (sai_uint32_t i = 0; i < maxcount; i++)
    {
        if (count > SAI_ACL_TABLE_ATTR_ACL_ACTION_TYPE_LIST &&
            count < SAI_ACL_TABLE_ATTR_FIELD_START)
        {
            count = SAI_ACL_TABLE_ATTR_FIELD_START;
        }

        attr_list[idx].id = SAI_ACL_TABLE_ATTR_START + count;
        saiRetVal = xpSaiGetAclMapperTableAttribute(keyFormat, &attr_list[idx], count);

        if (saiRetVal == SAI_STATUS_SUCCESS)
        {
            idx++;
        }
        count++;
    }
    *attr_count = idx;

    return SAI_STATUS_SUCCESS;
}

sai_status_t
xpSaiAclMapperTableAttributesDelete(sai_uint32_t tableId)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    xpSaiAclTableIdMappingT *keyFormat = NULL;
    xpSaiAclTableIdMappingT dbKey;
    xpsDevice_t devId = xpSaiGetDevId();

    dbKey.tableId = tableId;

    /* Search corresponding table API */
    retVal = xpsStateSearchDataForDevice(devId, sXpSaiAclTableIdArrayDbHndl,
                                         (xpsDbKey_t)&dbKey, (void**)&keyFormat);
    if (retVal != XP_NO_ERR)
    {
        return retVal;
    }

    if (keyFormat == NULL)
    {
        return XP_ERR_KEY_NOT_FOUND;
    }

    memset(keyFormat->match, 0x00, sizeof(keyFormat->match));

    return retVal;
}

sai_status_t
xpSaiAclMapperRuleIdGet(xpsDevice_t devId, sai_uint32_t tableId,
                        sai_uint32_t entryId,
                        sai_uint32_t *ruleId)
{
    XP_STATUS xpStatus = XP_NO_ERR;

    if (ruleId == NULL)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    return xpsStatus2SaiStatus(xpStatus);
}

sai_status_t
xpSaiAclMapperEntryKeyDeleteWithoutCntr(xpsDevice_t devId, sai_uint32_t entryId)
{
    XP_STATUS                   xpStatus    = XP_NO_ERR;
    xpSaiAclEntryIdMappingT*    infoEntry   = NULL;
    static xpSaiAclEntryIdMappingT     key;

    memset(&key, 0x00, sizeof(key));
    key.entryId = entryId;

    xpStatus = xpsStateDeleteDataForDevice(devId, sXpSaiAclEntryIdArrayDbHndl,
                                           (xpsDbKey_t)&key, (void**)&infoEntry);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to delete DB entry for entry %u \n", entryId);
        return xpsStatus2SaiStatus(xpStatus);
    }
    xpStatus = xpsAllocatorReleaseId(XP_SCOPE_DEFAULT, XP_SAI_ALLOC_ACL_ENTRY,
                                     entryId);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not release ACL entry id, error %d\n", xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t
xpSaiAclMapperEntryDelete(xpsDevice_t devId, sai_uint32_t entryId)
{
    XP_STATUS                   xpStatus    = XP_NO_ERR;
    xpSaiAclEntryIdMappingT*    infoEntry   = NULL;
    static xpSaiAclEntryIdMappingT     key;
    sai_uint32_t                countIdx = 0;

    memset(&key, 0x00, sizeof(key));
    key.entryId = entryId;

    xpStatus = xpsStateDeleteDataForDevice(devId, sXpSaiAclEntryIdArrayDbHndl,
                                           (xpsDbKey_t)&key, (void**)&infoEntry);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to delete DB entry for entry %u \n", entryId);
        return xpsStatus2SaiStatus(xpStatus);
    }

    if (infoEntry != NULL)
    {
        for (countIdx = SAI_ACL_ENTRY_ATTR_ACTION_START;
             countIdx < SAI_ACL_ENTRY_ATTR_ACTION_END; countIdx++)
        {
            if (xpSaiValidateEntryListAttributes((sai_acl_entry_attr_t)countIdx) &&
                infoEntry->attrs.action[countIdx -
                                                 SAI_ACL_ENTRY_ATTR_ACTION_START].actionValue.enable)
            {
                if (infoEntry->attrs.action[countIdx-
                                            SAI_ACL_ENTRY_ATTR_ACTION_START].actionValue.parameter.objlist.list != NULL)
                {
                    if ((xpStatus = xpsStateHeapFree(infoEntry->attrs.action[countIdx -
                                                                                      SAI_ACL_ENTRY_ATTR_ACTION_START].actionValue.parameter.objlist.list)) !=
                        XP_NO_ERR)
                    {
                        XP_SAI_LOG_ERR("Failed to to free allocated memory\n");
                        return xpsStatus2SaiStatus(xpStatus);
                    }

                    infoEntry->attrs.action[countIdx -
                                                     SAI_ACL_ENTRY_ATTR_ACTION_START].actionValue.parameter.objlist.list = NULL;
                }
            }
        }

        for (countIdx = SAI_ACL_ENTRY_ATTR_FIELD_START;
             countIdx < SAI_ACL_ENTRY_ATTR_FIELD_END; countIdx++)
        {

            if (xpSaiValidateEntryListAttributes((sai_acl_entry_attr_t)countIdx) &&
                infoEntry->attrs.match[countIdx -
                                                SAI_ACL_ENTRY_ATTR_FIELD_START].entryValue.enable)
            {
                if (infoEntry->attrs.match[countIdx-
                                           SAI_ACL_ENTRY_ATTR_FIELD_START].entryValue.data.objlist.list != NULL)
                {
                    if ((xpStatus = xpsStateHeapFree(infoEntry->attrs.match[countIdx -
                                                                                     SAI_ACL_ENTRY_ATTR_FIELD_START].entryValue.data.objlist.list)) != XP_NO_ERR)
                    {
                        XP_SAI_LOG_ERR("Failed to to free allocated memory\n");
                        return xpsStatus2SaiStatus(xpStatus);
                    }

                    infoEntry->attrs.match[countIdx -
                                                    SAI_ACL_ENTRY_ATTR_FIELD_START].entryValue.data.objlist.list = NULL;
                }
            }
        }

        xpsStateHeapFree((void*)infoEntry);
    }

    xpStatus = xpsAllocatorReleaseId(XP_SCOPE_DEFAULT, XP_SAI_ALLOC_ACL_ENTRY,
                                     entryId);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Could not release ACL entry id, error %d\n", xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t
xpSaiAclMapperTableIdMappingShow(void)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    xpSaiAclTableIdMappingT *keyFormat = NULL;
    xpSaiAclTableIdMappingT dbKey;
    xpsDevice_t devId = xpSaiGetDevId();

    retVal = xpsStateGetNextDataForDevice(devId, sXpSaiAclTableIdArrayDbHndl, NULL,
                                          (void**)&keyFormat);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Get data failed\n");
        return SAI_STATUS_FAILURE;
    }

    while (keyFormat != NULL)
    {
        XP_SAI_LOG_DBG("Table %u   stage = %u \n",
                       keyFormat->tableId, keyFormat->stage);

        /* Get next node */
        retVal = xpsStateGetNextDataForDevice(devId, sXpSaiAclTableIdArrayDbHndl,
                                              (xpsDbKey_t)&dbKey, (void**)&keyFormat);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Get data failed\n");
            return SAI_STATUS_FAILURE;
        }
    }

    return retVal;
}

sai_status_t
xpSaiAclMapperEntryIdMappingShow(xpsDevice_t devId, sai_uint32_t tableId)
{
    XP_STATUS xpStatus = XP_NO_ERR;

    //TODO
    return xpsStatus2SaiStatus(xpStatus);
}

XP_STATUS xpSaiAclCounterMappingGet(xpsDevice_t devId, sai_uint32_t counterId,
                                    xpSaiAclCounterIdMappingT **info)
{
    XP_STATUS rc = XP_NO_ERR;
    xpSaiAclCounterIdMappingT key;

    key.counterId = counterId;
    if ((rc = xpsStateSearchDataForDevice(devId, sXpSaiAclCounterIdArrayDbHndl,
                                          (xpsDbKey_t)&key, (void**)info)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("SAI ACL mapper search failed for counter %u.\n", counterId);
        return rc;
    }

    /* interface context found ? */
    if (*info == NULL)
    {
        XP_SAI_LOG_ERR("SAI ACL mapper failed to find counter %u.\n", counterId);
        return XP_ERR_NOT_FOUND;
    }

    return rc;
}

sai_status_t xpSaiAclCounterUpdate(xpsDevice_t devId, sai_uint32_t counterId,
                                   sai_uint32_t tableId,
                                   sai_acl_stage_t stage, sai_uint32_t ruleId)
{
    sai_status_t                saiRetVal           = SAI_STATUS_SUCCESS;
    sai_uint64_t                pktCounterValue     = 0;
    sai_uint64_t                byteCounterValue    = 0;
    xpSaiAclCounterIdMappingT*  infoCounterPtr       = NULL;
    XP_STATUS           retVal              = XP_NO_ERR;

    saiRetVal = xpSaiAclCounterMappingGet(devId, counterId, &infoCounterPtr);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("No counter %u found \n", counterId);
        return saiRetVal;
    }
    retVal = xpsAclGetCounterValue(devId, counterId, &byteCounterValue,
                                   &pktCounterValue);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsAclGetCounterValue: returned with error %d\n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }
    infoCounterPtr->attr.packets += pktCounterValue;
    infoCounterPtr->attr.bytes += byteCounterValue;

    return SAI_STATUS_SUCCESS;
}

static sai_status_t xpSaiGetAclMapperTableGrpAttribute(
    xpSaiAclTableGrpIdMappingT *keyFormat, sai_attribute_t *attr,
    sai_uint32_t attr_index)
{
    switch (attr->id)
    {
        case SAI_ACL_TABLE_GROUP_ATTR_ACL_STAGE:
            {
                attr->value.u32 = (sai_uint32_t)(keyFormat->stage);
                break;
            }

        case SAI_ACL_TABLE_GROUP_ATTR_ACL_BIND_POINT_TYPE_LIST:
            {
                if (attr->value.s32list.count > 1)
                {
                    XP_SAI_LOG_ERR("Currently supporting only one bind point type per table!\n");
                    return (SAI_STATUS_INVALID_ATTR_VALUE_0 + SAI_STATUS_CODE(attr_index));
                }

                if (attr->value.s32list.count < 1)
                {
                    attr->value.s32list.count = 1;
                    XP_SAI_LOG_ERR("Invalid list items count: %d!\n", attr->value.s32list.count);
                    return (SAI_STATUS_INVALID_ATTR_VALUE_0 + SAI_STATUS_CODE(attr_index));
                }
                attr->value.s32list.list[0] = keyFormat->bp_list[0];
                break;
            }

        case SAI_ACL_TABLE_GROUP_ATTR_TYPE:
            {
                attr->value.u32 = (sai_uint32_t)(keyFormat->type);
                break;
            }

        default:
            {
                XP_SAI_LOG_ERR("Invalid attribute: %d!\n", attr->id);
                return (SAI_STATUS_UNKNOWN_ATTRIBUTE_0 + SAI_STATUS_CODE(attr_index));
            }
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiGetAclMapperTableGrpAttributes(sai_uint32_t groupId,
                                                 sai_uint32_t attr_count, sai_attribute_t *attr_list)
{
    XP_STATUS                   xpsStatus = XP_NO_ERR;
    sai_status_t                saiRetVal = SAI_STATUS_SUCCESS;
    xpSaiAclTableGrpIdMappingT *keyFormat = NULL;
    xpSaiAclTableGrpIdMappingT  dbKey;

    if (attr_list == NULL || !attr_count)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    dbKey.groupId = groupId;

    /* Search corresponding table API */
    xpsStatus = xpsStateSearchDataForDevice(xpSaiGetDevId(),
                                            sXpSaiAclTableGrpIdArrayDbHndl, (xpsDbKey_t)&dbKey, (void**)&keyFormat);
    if (xpsStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateSearchDataForDevice failed with xpsStatus: %d\n",
                       xpsStatus);
        return xpsStatus2SaiStatus(xpsStatus);
    }
    if (keyFormat == NULL)
    {
        XP_SAI_LOG_ERR("Key not found!\n");
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    for (sai_uint32_t count = 0; count < attr_count; count++)
    {
        saiRetVal = xpSaiGetAclMapperTableGrpAttribute(keyFormat, &attr_list[count],
                                                       count);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiGetAclMapperTableGrpAttribute() failed with saiRetVal: %d!\n",
                           saiRetVal);
            return saiRetVal;
        }
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t xpSaiGetAclMapperTableGrpMembAttribute(
    xpSaiAclTableGrpMembIdMappingT *keyFormat, sai_attribute_t *attr,
    sai_uint32_t attr_index)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    switch (attr->id)
    {
        case SAI_ACL_TABLE_GROUP_MEMBER_ATTR_ACL_TABLE_GROUP_ID:
            {
                saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_ACL_TABLE_GROUP, xpSaiGetDevId(),
                                             (sai_uint64_t)keyFormat->groupId, &attr->value.oid);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("SAI object could not be created, error %d\n", saiRetVal);
                    return saiRetVal;
                }
                break;
            }

        case SAI_ACL_TABLE_GROUP_MEMBER_ATTR_ACL_TABLE_ID:
            {
                saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_ACL_TABLE, xpSaiGetDevId(),
                                             (sai_uint64_t)keyFormat->tableId, &attr->value.oid);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("SAI object could not be created, error %d\n", saiRetVal);
                    return saiRetVal;
                }
                break;
            }

        case SAI_ACL_TABLE_GROUP_MEMBER_ATTR_PRIORITY:
            {
                attr->value.u32 = keyFormat->priority;
                break;
            }

        default:
            {
                XP_SAI_LOG_ERR("Invalid attribute: %d!\n", attr->id);
                return (SAI_STATUS_UNKNOWN_ATTRIBUTE_0 + SAI_STATUS_CODE(attr_index));
            }
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiGetAclMapperTableGrpMembAttributes(sai_uint32_t memberId,
                                                     sai_uint32_t attr_count, sai_attribute_t *attr_list)
{
    XP_STATUS                       xpsStatus = XP_NO_ERR;
    sai_status_t                    saiRetVal = SAI_STATUS_SUCCESS;
    xpSaiAclTableGrpMembIdMappingT *keyFormat = NULL;
    xpSaiAclTableGrpMembIdMappingT  dbKey;

    if (attr_list == NULL || !attr_count)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    dbKey.memberId = memberId;

    /* Search corresponding table API */
    xpsStatus = xpsStateSearchDataForDevice(xpSaiGetDevId(),
                                            sXpSaiAclTableGrpMembIdArrayDbHndl, (xpsDbKey_t)&dbKey, (void**)&keyFormat);
    if (xpsStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateSearchDataForDevice failed with xpStatus: %d\n",
                       xpsStatus);
        return xpsStatus2SaiStatus(xpsStatus);
    }
    if (keyFormat == NULL)
    {
        XP_SAI_LOG_ERR("Key not found!\n");
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    for (sai_uint32_t count = 0; count < attr_count; count++)
    {
        saiRetVal = xpSaiGetAclMapperTableGrpMembAttribute(keyFormat, &attr_list[count],
                                                           count);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiGetAclMapperTableGrpMembAttribute() failed with saiRetVal: %d!\n",
                           saiRetVal);
            return saiRetVal;
        }
    }

    return SAI_STATUS_SUCCESS;
}


sai_status_t
xpSaiAclMapperAclGroupMemberAttributesGet(sai_uint32_t entry_id,
                                          xpSaiAclTableGrpMembIdMappingT** pAttributes)
{
    XP_STATUS                retVal  = XP_NO_ERR;
    xpSaiAclTableGrpMembIdMappingT* pEntry = NULL;
    static xpSaiAclTableGrpMembIdMappingT  key;
    sai_uint32_t             devId = xpSaiGetDevId();

    memset(&key, 0x00, sizeof(key));
    key.memberId = entry_id;

    retVal = xpsStateSearchDataForDevice(devId, sXpSaiAclTableGrpMembIdArrayDbHndl,
                                         (xpsDbKey_t)&key, (void**)&pEntry);
    if (retVal != XP_NO_ERR)
    {
        return xpsStatus2SaiStatus(retVal);
    }

    if (pEntry == NULL)
    {
        return xpsStatus2SaiStatus(XP_ERR_KEY_NOT_FOUND);
    }

    *pAttributes = pEntry;

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiGetACLMapperTableGrpTableIdList(sai_object_id_t tableGrpObjId,
                                                  sai_uint32_t *count, sai_uint32_t *tableIdList)
{
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;
    xpSaiAclTableGrpIdMappingT *pTblGrpEntry = NULL;
    sai_uint32_t memberId    = 0;
    xpSaiAclTableGrpMembIdMappingT *keyFormat  = NULL;


    if ((tableIdList == NULL) || (count == NULL))
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    // Get the table group state data
    saiStatus = xpSaiAclMapperGetTableGroupStateData(xpSaiGetDevId(), tableGrpObjId,
                                                     &pTblGrpEntry);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get the acl table group state data base, status: %d\n",
                       saiStatus);
        return saiStatus;
    }

    for (uint32_t i = 0; i < pTblGrpEntry->memberCount; i++)
    {
        memberId = (sai_uint32_t)xpSaiObjIdValueGet(pTblGrpEntry->memberList[i]);

        saiStatus = xpSaiAclMapperAclGroupMemberAttributesGet(memberId, &keyFormat);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpsStateSearchDataForDevice failed with xpStatus: %d\n",
                           saiStatus);
            return saiStatus;
        }
        tableIdList[i] = keyFormat->tableId;
    }

    *count = pTblGrpEntry->memberCount;

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiCountAclMapperTableObjects(sai_uint32_t *count)
{
    XP_STATUS  retVal  = XP_NO_ERR;

    if (!count)
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpsStateGetCountForDevice(sXpSaiAclTableIdArrayDbHndl, xpSaiGetDevId(),
                                       count);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get Count. return Value : %d\n", retVal);
    }
    return xpsStatus2SaiStatus(retVal);
}

sai_status_t xpSaiCountAclMapperTableGrpObjects(sai_uint32_t *count)
{
    XP_STATUS  retVal  = XP_NO_ERR;

    if (!count)
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpsStateGetCountForDevice(sXpSaiAclTableGrpIdArrayDbHndl,
                                       xpSaiGetDevId(), count);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get Count. return Value : %d\n", retVal);
    }
    return xpsStatus2SaiStatus(retVal);
}

sai_status_t xpSaiCountAclMapperTableGrpMembObjects(sai_uint32_t *count)
{
    XP_STATUS  retVal  = XP_NO_ERR;

    if (!count)
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpsStateGetCountForDevice(sXpSaiAclTableGrpMembIdArrayDbHndl,
                                       xpSaiGetDevId(), count);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get Count. return Value : %d\n", retVal);
    }
    return xpsStatus2SaiStatus(retVal);
}

sai_status_t xpSaiCountAclMapperCounterObjects(sai_uint32_t *count)
{
    XP_STATUS  retVal  = XP_NO_ERR;

    if (!count)
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpsStateGetCountForDevice(sXpSaiAclCounterIdArrayDbHndl,
                                       xpSaiGetDevId(), count);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get Count. return Value : %d\n", retVal);
    }
    return xpsStatus2SaiStatus(retVal);
}

sai_status_t xpSaiCountAclMapperEntryObjects(sai_uint32_t *count)
{
    XP_STATUS  retVal  = XP_NO_ERR;

    if (!count)
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpsStateGetCountForDevice(sXpSaiAclEntryIdArrayDbHndl, xpSaiGetDevId(),
                                       count);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get Count. return Value : %d\n", retVal);
    }
    return xpsStatus2SaiStatus(retVal);
}

sai_status_t xpSaiGetAclMapperTableObjectList(sai_uint32_t *object_count,
                                              sai_object_key_t *object_list)
{
    XP_STATUS       retVal      = XP_NO_ERR;
    sai_status_t    saiRetVal   = SAI_STATUS_SUCCESS;
    sai_uint32_t    objCount    = 0;
    xpsDevice_t     devId       = xpSaiGetDevId();

    xpSaiAclTableIdMappingT   *pAclTableNext    = NULL;

    saiRetVal = xpSaiCountAclMapperTableObjects(&objCount);
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

    for (sai_uint32_t i = 0; i < *object_count; i++)
    {
        retVal = xpsStateGetNextDataForDevice(xpSaiGetDevId(),
                                              sXpSaiAclTableIdArrayDbHndl, pAclTableNext, (void **)&pAclTableNext);
        if (retVal != XP_NO_ERR || pAclTableNext == NULL)
        {
            XP_SAI_LOG_ERR("Failed to retrieve AclMapperTable object, error %d\n", retVal);
            return SAI_STATUS_FAILURE;
        }

        saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_ACL_TABLE, devId,
                                     (sai_uint64_t)pAclTableNext->tableId, &object_list[i].key.object_id);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("SAI objectId could not be created, error %d\n", saiRetVal);
            return saiRetVal;
        }
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiGetAclMapperCounterObjectList(sai_uint32_t *object_count,
                                                sai_object_key_t *object_list)
{
    XP_STATUS       retVal      = XP_NO_ERR;
    sai_status_t    saiRetVal   = SAI_STATUS_SUCCESS;
    sai_uint32_t    objCount    = 0;
    xpsDevice_t     devId       = xpSaiGetDevId();

    xpSaiAclCounterIdMappingT   *pAclCounterNext    = NULL;

    saiRetVal = xpSaiCountAclMapperCounterObjects(&objCount);
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

    for (sai_uint32_t i = 0; i < *object_count; i++)
    {
        retVal = xpsStateGetNextDataForDevice(xpSaiGetDevId(),
                                              sXpSaiAclCounterIdArrayDbHndl, pAclCounterNext, (void **)&pAclCounterNext);
        if (retVal != XP_NO_ERR || pAclCounterNext == NULL)
        {
            XP_SAI_LOG_ERR("Failed to retrieve AclMapperCounter object, error %d\n",
                           retVal);
            return SAI_STATUS_FAILURE;
        }

        saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_ACL_COUNTER, devId,
                                     (sai_uint64_t)pAclCounterNext->counterId, &object_list[i].key.object_id);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("SAI objectId could not be created, error %d\n", saiRetVal);
            return saiRetVal;
        }
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiGetAclMapperEntryObjectList(sai_uint32_t *object_count,
                                              sai_object_key_t *object_list)
{
    XP_STATUS       retVal      = XP_NO_ERR;
    sai_status_t    saiRetVal   = SAI_STATUS_SUCCESS;
    sai_uint32_t    objCount    = 0;
    xpsDevice_t     devId       = xpSaiGetDevId();

    xpSaiAclEntryIdMappingT   *pAclEntryNext    = NULL;

    saiRetVal = xpSaiCountAclMapperEntryObjects(&objCount);
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

    for (sai_uint32_t i = 0; i < *object_count; i++)
    {
        retVal = xpsStateGetNextDataForDevice(devId, sXpSaiAclEntryIdArrayDbHndl,
                                              pAclEntryNext, (void **)&pAclEntryNext);
        if (retVal != XP_NO_ERR || pAclEntryNext == NULL)
        {
            XP_SAI_LOG_ERR("Failed to retrieve AclMapperEntry object, error %d\n", retVal);
            return SAI_STATUS_FAILURE;
        }

        saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_ACL_ENTRY, devId,
                                     (sai_uint64_t)pAclEntryNext->entryId, &object_list[i].key.object_id);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("SAI objectId could not be created, error %d\n", saiRetVal);
            return saiRetVal;
        }
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiAddAclMapperRsnCodeToQueueMap(sai_uint32_t xpsDevId,
                                                sai_uint32_t rsnCode, sai_uint32_t queueId)
{
    XP_STATUS xpsRetVal = XP_NO_ERR;
    xpSaiAclRsnCodeToQueueMapInfo_t aclRsnCodeToQueueMapInfo;
    xpSaiAclRsnCodeToQueueMapInfo_t *pAclRsnCodeToQueueMapInfo = NULL;

    aclRsnCodeToQueueMapInfo.rsnCode = rsnCode;

    /* Retrieve the corresponding state from sai acl reason Code to Queue Map info db */
    if ((xpsRetVal = xpsStateSearchDataForDevice(xpsDevId,
                                                 sXpSaiAclRsnCodeToQueueMapArrayDbHndl, (xpsDbKey_t)&aclRsnCodeToQueueMapInfo,
                                                 (void**)&pAclRsnCodeToQueueMapInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to retrieve the sai acl reason code to Queue Map info data |Error: %d\n",
                       xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    if (!pAclRsnCodeToQueueMapInfo)
    {
        /* Create a new sai acl reason Code to Queue Map info db */
        if ((xpsRetVal = xpsStateHeapMalloc(sizeof(xpSaiAclRsnCodeToQueueMapInfo_t),
                                            (void**)&pAclRsnCodeToQueueMapInfo)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Heap Allocation for sai acl reason code to Queue Map db failed |Error: %d\n",
                           xpsRetVal);
            return xpsStatus2SaiStatus(xpsRetVal);
        }
        memset(pAclRsnCodeToQueueMapInfo, 0, sizeof(xpSaiAclRsnCodeToQueueMapInfo_t));

        pAclRsnCodeToQueueMapInfo->rsnCode = rsnCode;
        pAclRsnCodeToQueueMapInfo->queueId = queueId;

        if ((xpsRetVal = xpsStateInsertDataForDevice(xpsDevId,
                                                     sXpSaiAclRsnCodeToQueueMapArrayDbHndl,
                                                     (void*)pAclRsnCodeToQueueMapInfo)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to insert sai acl reason code to Queue Map info data |Error: %d\n",
                           xpsRetVal);
            /* Free the memory allocated for the corresponding state */
            if ((xpsRetVal = xpsStateHeapFree(pAclRsnCodeToQueueMapInfo)) != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Freeing allocated memory failed |Error: %d\n", xpsRetVal);
                return xpsStatus2SaiStatus(xpsRetVal);
            }

            return xpsStatus2SaiStatus(xpsRetVal);
        }

    }
    else
    {
        if (pAclRsnCodeToQueueMapInfo->queueId != queueId)
        {
            XP_SAI_LOG_ERR("Reason Code %d already mapped with Queue %d |Error: %d\n",
                           pAclRsnCodeToQueueMapInfo->rsnCode,
                           pAclRsnCodeToQueueMapInfo->queueId, xpsRetVal);
            return SAI_STATUS_FAILURE;
        }
    }
    /* Increment the ref count */
    pAclRsnCodeToQueueMapInfo->refCount += 1;

    if (pAclRsnCodeToQueueMapInfo->refCount == 1)
    {
        xpsRetVal = xpsQosCpuScAddReasonCodeToCpuQueueMap(xpsDevId, XP_MCPU, rsnCode,
                                                          queueId);
        if (xpsRetVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Could not map reason code %d to queue %d, error %d", rsnCode,
                           queueId, xpsRetVal);
            return xpsStatus2SaiStatus(xpsRetVal);
        }

    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiDeleteAclMapperRsnCodeToQueueMap(sai_uint32_t xpsDevId,
                                                   sai_uint32_t rsnCode)
{
    XP_STATUS xpsRetVal = XP_NO_ERR;
    xpSaiAclRsnCodeToQueueMapInfo_t aclRsnCodeToQueueMapInfo;
    xpSaiAclRsnCodeToQueueMapInfo_t *pAclRsnCodeToQueueMapInfo = NULL;

    aclRsnCodeToQueueMapInfo.rsnCode = rsnCode;

    /* Retrieve the corresponding state from sai acl reason Code to Queue Map info db */
    if ((xpsRetVal = xpsStateSearchDataForDevice(xpsDevId,
                                                 sXpSaiAclRsnCodeToQueueMapArrayDbHndl, (xpsDbKey_t)&aclRsnCodeToQueueMapInfo,
                                                 (void**)&pAclRsnCodeToQueueMapInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to retrieve the sai acl reason code to Queue Map info data |Error: %d\n",
                       xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    if (!pAclRsnCodeToQueueMapInfo)
    {
        XP_SAI_LOG_ERR("SAI ACL reason code to Queue Map info Db not found |Error : %d\n",
                       xpsRetVal);
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    /* Decrement the ref count */
    pAclRsnCodeToQueueMapInfo->refCount -= 1;

    if (pAclRsnCodeToQueueMapInfo->refCount == 0)
    {
        xpsRetVal = xpsQosCpuScAddReasonCodeToCpuQueueMap(xpsDevId, XP_MCPU, rsnCode,
                                                          0);
        if (xpsRetVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Could not remove map fore reason code %d , error %d", rsnCode,
                           xpsRetVal);
            return xpsStatus2SaiStatus(xpsRetVal);
        }

        /* Remove the corresponding state from sai acl reason Code to Queue Map info db */
        if ((xpsRetVal = xpsStateDeleteDataForDevice(xpsDevId,
                                                     sXpSaiAclRsnCodeToQueueMapArrayDbHndl, (xpsDbKey_t)&aclRsnCodeToQueueMapInfo,
                                                     (void**)&pAclRsnCodeToQueueMapInfo)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Deletion of data failed |Error: %d\n", xpsRetVal);
            return xpsStatus2SaiStatus(xpsRetVal);
        }

        /* Free the memory allocated for the corresponding state */
        if ((xpsRetVal = xpsStateHeapFree(pAclRsnCodeToQueueMapInfo)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Freeing allocated memory failed |Error: %d\n", xpsRetVal);
            return (xpsRetVal);
        }
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t
xpSaiAclGetAcmCounterValue(xpsDevice_t devId, sai_uint32_t tableId,
                           sai_acl_stage_t stage, sai_uint32_t seVal, sai_uint32_t ruleId,
                           sai_uint64_t *pktCounterValue, sai_uint64_t *byteCounterValue)
{
    //TODO
    return SAI_STATUS_SUCCESS;
}

sai_status_t
xpSaiAclMapperInsertTableId(xpsDevice_t devId,
                            xpSaiAclTableIdMappingT  *keyFormat)
{
    XP_STATUS      retVal = XP_NO_ERR;

    /* Insert SAI ACL entry structure into DB */
    retVal = xpsStateInsertDataForDevice(devId, sXpSaiAclTableIdArrayDbHndl,
                                         (void*)keyFormat);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to add DB to the device %u \n", devId);
        return xpsStatus2SaiStatus(retVal);
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t
xpSaiAclMapperRemoveTableId(xpsDevice_t devId, uint32_t tableId)
{
    XP_STATUS      retVal = XP_NO_ERR;
    xpSaiAclTableIdMappingT  *infoEntry = NULL;
    xpSaiAclTableIdMappingT  key;

    key.tableId = tableId;

    retVal = xpsStateDeleteDataForDevice(devId, sXpSaiAclTableIdArrayDbHndl,
                                         (xpsDbKey_t)&key, (void**)&infoEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to add DB to the device %u \n", devId);
        return xpsStatus2SaiStatus(retVal);
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t
xpSaicustAclMapperTableAttributesSet(sai_uint32_t tableId,
                                     xpSaiAclTableAttributesT* pAttributes, sai_uint32_t keySize)
{
    sai_status_t            retVal = SAI_STATUS_SUCCESS;
    xpSaiAclTableIdMappingT *keyFormat = NULL;
    xpSaiAclTableIdMappingT dbKey;
    xpsDevice_t             devId = xpSaiGetDevId();

    if (pAttributes == NULL)
    {
        XP_SAI_LOG_ERR("Invalid input parameter.\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    dbKey.tableId = tableId;

    /* Search corresponding table API */
    retVal = xpsStateSearchDataForDevice(devId, sXpSaiAclTableIdArrayDbHndl,
                                         (xpsDbKey_t)&dbKey, (void**)&keyFormat);
    if (retVal != XP_NO_ERR)
    {
        return retVal;
    }

    if (keyFormat == NULL)
    {
        return XP_ERR_KEY_NOT_FOUND;
    }

    /* Assign values */
    keyFormat->stage = pAttributes->stage;
    keyFormat->tableSize = pAttributes->tableSize;
    memcpy(keyFormat->match, pAttributes->match, sizeof(keyFormat->match));

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiAclUpdateMemberInfo(sai_object_id_t aclObjId,
                                      xpsInterfaceId_t xpsIntfId, bool bindAcl)
{
    XP_STATUS retVal = XP_NO_ERR;
    uint32_t aclId = (sai_uint32_t)xpSaiObjIdValueGet(aclObjId);
    uint32_t count = 0;
    uint32_t xpsPortId = 0;
    uint32_t tableIdList[SAI_ACL_TBL_GROUP_SIZE];//should the size be max no of tables possible in a group
    xpsInterfaceType_e intfType;
    xpsLagPortIntfList_t portList;
    memset(&portList, 0, sizeof(portList));

    if (aclObjId == SAI_NULL_OBJECT_ID)
    {
        return SAI_STATUS_SUCCESS;
    }

    if (XDK_SAI_OBJID_TYPE_CHECK(aclObjId, SAI_OBJECT_TYPE_ACL_TABLE_GROUP))
    {
        //get the tables that are in this table group
        if (xpSaiGetACLMapperTableGrpTableIdList(aclObjId, &count,
                                                 tableIdList) != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Getting table Id list for group %d.\n", aclId);
            return SAI_STATUS_FAILURE;
        }
    }
    else if (XDK_SAI_OBJID_TYPE_CHECK(aclObjId, SAI_OBJECT_TYPE_ACL_TABLE))
    {
        count = 1;
        tableIdList[0] = aclId;
        aclId = 0;
    }
    else
    {
        XP_SAI_LOG_ERR("Binding to objectId %d not allowed.\n", aclObjId);
        return SAI_STATUS_FAILURE;
    }

    retVal = xpsInterfaceGetType(xpsIntfId, &intfType);
    if (retVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get type for interface %u, error: %d\n", xpsIntfId, retVal);
        return retVal;
    }

    memset(&portList, 0, sizeof(portList));
    if (XPS_LAG == intfType)
    {
        retVal = xpsLagGetPortIntfList(xpsIntfId, &portList);
        if (retVal != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Get ports failed, lag interface(%d)", xpsIntfId);
            return retVal;
        }
    }
    else if (XPS_PORT == intfType)
    {
        portList.portIntf[0] = xpsIntfId;
        portList.size = 1;
    }

    for (sai_uint32_t indx = 0; indx<count; indx++)
    {
        //update binding for all the tables in the tablegroup
        for (uint32_t i = 0; i < portList.size; i++)
        {
            xpsPortId = portList.portIntf[i];
            retVal = xpsAclUpdatePortIngressAcl(xpSaiGetDevId(), xpsPortId,
                                                tableIdList[indx], aclId,
                                                bindAcl);
            if (retVal != XP_NO_ERR)
            {
                return xpsStatus2SaiStatus(retVal);
            }
        }
    }
    return SAI_STATUS_SUCCESS;
}
