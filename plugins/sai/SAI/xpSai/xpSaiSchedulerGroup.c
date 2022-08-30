// xpSaiSchedulerGroup.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpSai.h"
#include "xpSaiValidationArrays.h"

XP_SAI_LOG_REGISTER_API(SAI_API_SCHEDULER_GROUP);

sai_scheduler_group_api_t* _xpSaiSchedulerGroupApi;

// Scheduler group DB handler
xpsDbHandle_t schedulerGrpDbHandle = XPSAI_SCHEDULER_GROUP_DB_HNDL;

//Func: xpSaiSchedulerGroupKeyComp

static sai_int32_t xpSaiSchedulerGroupKeyComp(void *key1, void *key2)
{
    sai_int64_t val1 = ((xpSaiSchedulerGrp_t*)key1)->schedulerGrpObjId;
    sai_int64_t val2 = ((xpSaiSchedulerGrp_t*)key2)->schedulerGrpObjId;

    if (val1 > val2)
    {
        return 1;
    }
    else if (val1 < val2)
    {
        return -1;
    }

    return 0;
}

//Func: xpSaiSchedulerGroupInit

XP_STATUS xpSaiSchedulerGroupInit(xpsDevice_t xpDevId)
{
    XP_STATUS xpStatus = XP_NO_ERR;

    /* Register global scheduler group state database */
    xpStatus = xpsStateRegisterDb(XP_SCOPE_DEFAULT, "Sai Scheduler Group DB",
                                  XPS_GLOBAL, &xpSaiSchedulerGroupKeyComp, schedulerGrpDbHandle);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to register sai scheduler group state database, xpStatus: %d\n",
                       xpStatus);
        return xpStatus;
    }

    return XP_NO_ERR;
}

//Func: xpSaiSchedulerGroupDeInit

XP_STATUS xpSaiSchedulerGroupDeInit(xpsDevice_t xpDevId)
{
    XP_STATUS xpStatus = XP_NO_ERR;

    /* De-Register global scheduler group state database */
    xpStatus = xpsStateDeRegisterDb(XP_SCOPE_DEFAULT, &schedulerGrpDbHandle);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to deregister sai scheduler group state database, xpStatus: %d\n",
                       xpStatus);
        return xpStatus;
    }

    return XP_NO_ERR;
}

//Func: xpSaiSchedulerGroupCreateStateData

static XP_STATUS xpSaiSchedulerGroupCreateStateData(sai_object_id_t
                                                    schedulerGrpObjId, uint32_t maxChilds, xpSaiSchedulerGrp_t **ppSchedulerGrpInfo)
{
    XP_STATUS xpStatus = XP_NO_ERR;

    /* Allocate memory for scheduler group */
    xpStatus = xpsStateHeapMalloc(sizeof(xpSaiSchedulerGrp_t),
                                  (void**)ppSchedulerGrpInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Couldn't allocate heap memory | xpStatus: %d\n",
                       xpStatus);
        return xpStatus;
    }
    memset(*ppSchedulerGrpInfo, 0x0, sizeof(xpSaiSchedulerGrp_t));

    /* Populate key value */
    (*ppSchedulerGrpInfo)->schedulerGrpObjId = schedulerGrpObjId;

    /* Insert the state into the database */
    xpStatus = xpsStateInsertData(XP_SCOPE_DEFAULT, schedulerGrpDbHandle,
                                  (void*)*ppSchedulerGrpInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Couldn't insert state into data base | xpStatus: %d\n",
                       xpStatus);
        xpsStateHeapFree((void*)*ppSchedulerGrpInfo);
        return xpStatus;
    }

    /* Allocate memory for child list */
    (*ppSchedulerGrpInfo)->childList.list  = (sai_object_id_t*)xpMalloc(sizeof(
                                                                            sai_object_id_t) * maxChilds);
    (*ppSchedulerGrpInfo)->childList.count = 0;
    memset((*ppSchedulerGrpInfo)->childList.list, 0x0,
           sizeof(sai_object_id_t)*maxChilds);

    return XP_NO_ERR;
}

//Func: xpSaiSchedulerGroupRemoveStateData

static XP_STATUS xpSaiSchedulerGroupRemoveStateData(sai_object_id_t
                                                    schedulerGrpObjId)
{
    xpSaiSchedulerGrp_t *pSchedulerGrpInfo = NULL;
    xpSaiSchedulerGrp_t  key               = { 0 };
    XP_STATUS            xpStatus          = XP_NO_ERR;

    /* Populate the key */
    key.schedulerGrpObjId = schedulerGrpObjId;

    /* Delete the corresponding state from state database */
    xpStatus = xpsStateDeleteData(XP_SCOPE_DEFAULT, schedulerGrpDbHandle,
                                  (xpsDbKey_t)&key, (void**)&pSchedulerGrpInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Couldn't not delete scheduler state data, xpStatus: %d\n",
                       xpStatus);
        return xpStatus;
    }

    if (!pSchedulerGrpInfo)
    {
        XP_SAI_LOG_ERR("Error: Could not find entry\n");
        return XP_ERR_NOT_FOUND;
    }

    /* Free the memory allocated for child list */
    xpStatus = xpsStateHeapFree(pSchedulerGrpInfo->childList.list);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Couldn't free memory allocated for child list, xpStatus: %d\n",
                       xpStatus);
        return xpStatus;
    }

    /* Free the memory allocated for the corresponding state */
    xpStatus = xpsStateHeapFree(pSchedulerGrpInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Couldn't free heap memory | xpStatus: %d\n", xpStatus);
        return xpStatus;
    }

    return XP_NO_ERR;
}

//Func: xpSaiSchedulerGroupGetStateData

static XP_STATUS xpSaiSchedulerGroupGetStateData(sai_object_id_t
                                                 schedulerGrpObjId, xpSaiSchedulerGrp_t **ppSchedulerGrpInfo)
{
    xpSaiSchedulerGrp_t key      = { 0 };
    XP_STATUS           xpStatus = XP_NO_ERR;

    /* Populate the key */
    key.schedulerGrpObjId = schedulerGrpObjId;

    /* Retrieve the corresponding state from state database */
    xpStatus = xpsStateSearchData(XP_SCOPE_DEFAULT, schedulerGrpDbHandle,
                                  (xpsDbKey_t)&key, (void**)ppSchedulerGrpInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to retrieve the state data | xpStatus: %d\n",
                       xpStatus);
        return xpStatus;
    }

    if (!(*ppSchedulerGrpInfo))
    {
        XP_SAI_LOG_ERR("Error: Scheduler group entry not found\n");
        return XP_ERR_NOT_FOUND;
    }

    return XP_NO_ERR;
}

//Func:  xpSaiSchedulerGroupBindChildToParent

XP_STATUS xpSaiSchedulerGroupBindChildToParent(sai_object_id_t parentObjId,
                                               sai_object_id_t childObjId)
{
    xpSaiSchedulerGrp_t  *pSchedulerGrpInfo = NULL;
    XP_STATUS             xpStatus = XP_NO_ERR;
    uint32_t              idx = 0;

    /* Get the parent scheduler group state database */
    xpStatus = xpSaiSchedulerGroupGetStateData(parentObjId, &pSchedulerGrpInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to get scheduler group state data | xpStatus: %d\n",
                       xpStatus);
        return xpStatus;
    }

    idx = pSchedulerGrpInfo->childList.count;

    /* Update the parent scheduler group child list */
    pSchedulerGrpInfo->childList.list[idx] = childObjId;
    pSchedulerGrpInfo->childList.count++;

    /* Update the parent scheduler group child count */
    pSchedulerGrpInfo->childCount++;

    return XP_NO_ERR;
}

//Func: xpSaiSchedulerGroupUnbindChildFromParent

XP_STATUS xpSaiSchedulerGroupUnbindChildFromParent(sai_object_id_t parentObjId,
                                                   sai_object_id_t childObjId)
{
    xpSaiSchedulerGrp_t  *pSchedulerGrpInfo = NULL;
    sai_uint32_t          nodePresent       = 0;
    XP_STATUS             xpStatus          = XP_NO_ERR;

    /* Get the parent scheduler group state database */
    xpStatus = xpSaiSchedulerGroupGetStateData(parentObjId, &pSchedulerGrpInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to get scheduler group state data | xpStatus: %d\n",
                       xpStatus);
        return xpStatus;
    }

    for (uint16_t idx = 0; idx < pSchedulerGrpInfo->childCount; idx++)
    {
        if (!nodePresent)
        {
            if (pSchedulerGrpInfo->childList.list[idx] == childObjId)
            {
                nodePresent = 1;
            }
            continue;
        }
        else
        {
            pSchedulerGrpInfo->childList.list[idx-1] =
                pSchedulerGrpInfo->childList.list[idx];
        }
    }

    if (nodePresent)
    {
        /* Decrement the count */
        pSchedulerGrpInfo->childList.count -= 1;
        pSchedulerGrpInfo->childCount -= 1;

    }

    return XP_NO_ERR;
}

//Func: xpSaiSchedulerGroupGetNodeIdAtLevelAndIndex

XP_STATUS xpSaiSchedulerGroupGetFreeNodeIdx(xpDevice_t xpDevId,
                                            sai_object_id_t parentObjId, uint32_t level, uint32_t *pNodeIdx)
{
    xpSaiSchedulerGrp_t *pSchedulerGrpInfo = NULL;
    XP_STATUS            xpStatus          = XP_NO_ERR;

    if (level > 0)
    {
        /* Get the scheduler group state data */
        xpStatus = xpSaiSchedulerGroupGetStateData(parentObjId, &pSchedulerGrpInfo);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Failed to get scheduler group state data, xpStatus: %d\n",
                           xpStatus);
            return xpStatus;
        }
    }

    uint32_t parentNodeIdx = (uint32_t)xpSaiObjIdValueGet(parentObjId);

    switch (level)
    {
        case 0: /* Level-0 */
            {
                *pNodeIdx = parentNodeIdx;
                break;
            }
        case 1: /* Level-1 */
            {
                /* check for max childs */
                if (pSchedulerGrpInfo->childCount >= XPSAI_SG_MAX_CHILDS_AT_LEVEL0)
                {
                    XP_SAI_LOG_ERR("Error: Max childs for a given parent scheduler group can be %d\n",
                                   XPSAI_SG_MAX_CHILDS_AT_LEVEL0);
                    return XP_ERR_OUT_OF_MEM;
                }

                *pNodeIdx = parentNodeIdx * XPSAI_SG_MAX_CHILDS_AT_LEVEL0 +
                            pSchedulerGrpInfo->childCount;
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Error: Invalid level %d\n", level);
                return XP_ERR_INVALID_INPUT;
            }
    }

    return xpStatus;
}

//Func: xpSaiSchedulerGroupGetKeyFromLevelAndNodeIdx

XP_STATUS xpSaiSchedulerGroupGetKeyFromLevelAndNodeIdx(uint32_t level,
                                                       uint32_t nodeIdx, uint32_t* key)
{
    uint32_t keyVal = 0;

    keyVal = keyVal | (level << XPSAI_SC_LEVEL_OFFSET);
    keyVal = keyVal | (nodeIdx << XPSAI_SC_NODE_INDEX_OFFSET);

    *key = keyVal;

    return XP_NO_ERR;
}

//Func: xpSaiSchedulerGroupGetLevelAndNodeIdxFromKey

XP_STATUS xpSaiSchedulerGroupGetLevelAndNodeIdxFromKey(uint32_t key,
                                                       uint32_t* level, uint32_t* nodeIdx)
{
    if (level)
    {
        *level = ((key & XPSAI_SC_LEVEL_VAL_MASK) >> XPSAI_SC_LEVEL_OFFSET);
    }

    if (nodeIdx)
    {
        *nodeIdx = ((key & XPSAI_SC_NODE_INDEX_VAL_MASK) >> XPSAI_SC_NODE_INDEX_OFFSET);
    }

    return XP_NO_ERR;
}

//Func: xpSaiSchedulerGroupGetNodeToPortPath

XP_STATUS xpSaiSchedulerGroupGetNodeToPortPath(xpsDevice_t xpDevId,
                                               uint32_t nodeKey, xpTxqQueuePathToPort_t* pPath)
{
    xpSaiSchedulerGrp_t *pSchedulerGrpInfo = NULL;
    sai_object_id_t      schedulerGrpObjId = SAI_NULL_OBJECT_ID;
    sai_status_t         saiStatus         = SAI_STATUS_SUCCESS;
    XP_STATUS            xpStatus          = XP_NO_ERR;

    /* Get scheduler group object id */
    saiStatus = xpSaiObjIdCreate(SAI_OBJECT_TYPE_SCHEDULER_GROUP, xpDevId, nodeKey,
                                 &schedulerGrpObjId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Could not get scheduler group object id, saiStatus: %d\n",
                       saiStatus);
        return XP_ERR_INVALID_VALUE;
    }

    /* Get the parent scheduler group state database */
    xpStatus = xpSaiSchedulerGroupGetStateData(schedulerGrpObjId,
                                               &pSchedulerGrpInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to get scheduler group state data | xpStatus: %d\n",
                       xpStatus);
        return xpStatus;
    }

    memset(pPath, 0, sizeof(xpTxqQueuePathToPort_t));

    switch (pSchedulerGrpInfo->level)
    {
        case 0:
            {
                uint32_t portKey = xpSaiObjIdValueGet(pSchedulerGrpInfo->portId);

                /* Get the Port node index from key */
                xpSaiSchedulerGroupGetLevelAndNodeIdxFromKey(portKey, NULL, &pPath->portNum);
                break;
            }
        case 1:
            {
                uint32_t h2Key   = xpSaiObjIdValueGet(pSchedulerGrpInfo->schedulerGrpObjId);
                uint32_t portKey = xpSaiObjIdValueGet(pSchedulerGrpInfo->portId);

                /* Get the H1 node index from key */
                xpSaiSchedulerGroupGetLevelAndNodeIdxFromKey(h2Key, NULL, &pPath->h2Num);

                /* Get the Port node index from key */
                xpSaiSchedulerGroupGetLevelAndNodeIdxFromKey(portKey, NULL, &pPath->portNum);
                break;
            }
        case 2:
            {
                uint32_t h1Key   = xpSaiObjIdValueGet(pSchedulerGrpInfo->schedulerGrpObjId);
                uint32_t h2Key   = xpSaiObjIdValueGet(pSchedulerGrpInfo->parentNode);
                uint32_t portKey = xpSaiObjIdValueGet(pSchedulerGrpInfo->portId);

                /* Get the H1 node index from key */
                xpSaiSchedulerGroupGetLevelAndNodeIdxFromKey(h1Key, NULL, &pPath->h1Num);

                /* Get the H2 node index from key */
                xpSaiSchedulerGroupGetLevelAndNodeIdxFromKey(h2Key, NULL, &pPath->h2Num);

                /* Get the Port node index from key */
                xpSaiSchedulerGroupGetLevelAndNodeIdxFromKey(portKey, NULL, &pPath->portNum);
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Error: Invalid scheduler group level %d\n",
                               pSchedulerGrpInfo->level);
                return XP_ERR_INVALID_VALUE;
            }
    }

    return XP_NO_ERR;
}

//Func: xpSaiSchedulerGroupConfigureScheduler

XP_STATUS xpSaiSchedulerGroupConfigureScheduler(sai_object_id_t schedGrpObjId,
                                                sai_object_id_t schedObjId)
{
    xpSaiSchedulerGrp_t  *pSchedulerGrpInfo = NULL;
    XP_STATUS             xpStatus          = XP_NO_ERR;
    sai_status_t          saiStatus         = SAI_STATUS_SUCCESS;
    sai_uint32_t          nodeKey           = xpSaiObjIdValueGet(schedGrpObjId);
    sai_uint32_t          count             = 0;
    sai_uint32_t          level             = 0;
    sai_attribute_value_t value;

    /* Get the scheduler group state database */
    xpStatus = xpSaiSchedulerGroupGetStateData(schedGrpObjId, &pSchedulerGrpInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to get scheduler group state data, xpStatus: %d\n",
                       xpStatus);
        return xpStatus;
    }

    /* Get the level from key */
    xpSaiSchedulerGroupGetLevelAndNodeIdxFromKey(nodeKey, &level, NULL);

    /* Set scheduler group's scheduling type, weight and shapers */
    switch (level)
    {
        case 0:
            {
                /* do nothing */
                /* port level shaping parameters are configured via port scheduler profile */
                break;
            }
        case 1:
            {
                value.oid = schedObjId;
                /* Apply the scheduler profile to child queue nodes */
                for (count = 0; count < pSchedulerGrpInfo->childCount; count++)
                {
                    XP_SAI_LOG_DBG("Set scheduler profile Parent %ld child %ld\n", schedGrpObjId,
                                   pSchedulerGrpInfo->childList.list[count]);
                    saiStatus = xpSaiSetQueueAttrSchedulerProfileId(
                                    pSchedulerGrpInfo->childList.list[count], value);
                    if (saiStatus != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Error: Failed to set scheduler profile to queue, saiStatus: %d\n",
                                       saiStatus);
                        return XP_ERR_FAILED;
                    }
                }

                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Error: Invalid scheduler group level %d\n", level);
                return XP_ERR_INVALID_VALUE;
            }
    }
    return XP_NO_ERR;
}

//Func: xpSaiSchedulerGroupSetSchedulerProfileId

XP_STATUS xpSaiSchedulerGroupSetSchedulerProfileId(sai_object_id_t
                                                   schedGrpObjId, sai_object_id_t schedObjId)
{
    xpSaiSchedulerGrp_t *pSchedulerGrpInfo = NULL;
    XP_STATUS            xpStatus          = XP_NO_ERR;

    /* Get the scheduler group state database */
    xpStatus = xpSaiSchedulerGroupGetStateData(schedGrpObjId, &pSchedulerGrpInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to get scheduler group state data, xpStatus: %d\n",
                       xpStatus);
        return xpStatus;
    }

    /* Check if already programmed */
    if (pSchedulerGrpInfo->schedulerProfileId == schedObjId)
    {
        return XP_NO_ERR;
    }

    /* Configure the scheduler on scheduler group node */
    xpStatus = xpSaiSchedulerGroupConfigureScheduler(schedGrpObjId, schedObjId);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to configure the scheduler on scheduler group, xpStatus: %d\n",
                       xpStatus);
        return xpStatus;
    }

    if (pSchedulerGrpInfo->schedulerProfileId != SAI_NULL_OBJECT_ID)
    {
        /* Unbind the scheduler group node from existing scheduler */
        xpStatus = xpSaiSchedulerUnbindSchedulerGrpFromScheduler(
                       pSchedulerGrpInfo->schedulerProfileId, schedGrpObjId);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Failed to unbind the scheduler group node from scheduler, xpStatus: %d\n",
                           xpStatus);
            return xpStatus;
        }
    }

    if (schedObjId != SAI_NULL_OBJECT_ID)
    {
        /* Bind the scheduler group node to new scheduler */
        xpStatus = xpSaiSchedulerBindSchedulerGrpToScheduler(schedObjId, schedGrpObjId);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Failed to bind the scheduler group node to scheduler, xpStatus: %d\n",
                           xpStatus);
            return xpStatus;
        }
    }

    /* Update the scheduler group state data with new scheduler profile */
    pSchedulerGrpInfo->schedulerProfileId = schedObjId;

    return XP_NO_ERR;
}

//Func:  xpSaiSchedulerGroupCreate

sai_status_t xpSaiSchedulerGroupCreate(sai_object_id_t switchObjId,
                                       xpSaiSchedulerGroupAttributesT *attributes,
                                       sai_object_id_t *pSchedulerGrpObjId)
{
    xpSaiSchedulerGrp_t  *pSchedulerGrpInfo = NULL;
    sai_uint32_t          nodeIdx           = 0;
    sai_uint32_t          key               = 0;
    sai_status_t          saiStatus         = SAI_STATUS_SUCCESS;
    XP_STATUS             xpStatus          = XP_NO_ERR;

    /* Get the device id from switch object id */
    xpsDevice_t xpDevId = xpSaiObjIdSwitchGet(switchObjId);

    /* Get a free node index for given level */
    xpStatus = xpSaiSchedulerGroupGetFreeNodeIdx(xpDevId,
                                                 attributes->parentNode.oid, attributes->level.u8, &nodeIdx);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Could not get a free node index at level %d, xpStatus: %d\n",
                       attributes->level.u8, xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    /* Get the scheduler group key from level and node index */
    xpStatus = xpSaiSchedulerGroupGetKeyFromLevelAndNodeIdx(attributes->level.u8,
                                                            (uint16_t)nodeIdx, &key);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Could not get key from level and node index, xpStatus: %d\n",
                       xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    /* Create scheduler group object id */
    saiStatus = xpSaiObjIdCreate(SAI_OBJECT_TYPE_SCHEDULER_GROUP, xpDevId, key,
                                 pSchedulerGrpObjId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Could not create scheduler group object id, saiStatus: %d\n",
                       saiStatus);
        return saiStatus;
    }

    /* Create state datebase */
    xpStatus = xpSaiSchedulerGroupCreateStateData(*pSchedulerGrpObjId,
                                                  attributes->maxChilds.u8, &pSchedulerGrpInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Could not create scheduler group state database, xpStatus: %d\n",
                       xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    pSchedulerGrpInfo->portId             = attributes->portId.oid;
    pSchedulerGrpInfo->level              = attributes->level.u8;
    pSchedulerGrpInfo->maxChilds          = attributes->maxChilds.u8;
    pSchedulerGrpInfo->parentNode         = attributes->parentNode.oid;

    xpStatus = xpSaiSchedulerGroupSetSchedulerProfileId(*pSchedulerGrpObjId,
                                                        attributes->schedulerProfileId.oid);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to set scheduler profile id, xpStatus: %d\n",
                       xpStatus);
        xpSaiSchedulerGroupRemoveStateData(*pSchedulerGrpObjId);
        return xpsStatus2SaiStatus(xpStatus);
    }

    /* Update the parent scheduler group's child list and count, if node level is greater than 0 */
    if (attributes->level.u8 > 0)
    {
        /* Bind child scheduler group node to parent scheduler group node */
        xpStatus = xpSaiSchedulerGroupBindChildToParent(attributes->parentNode.oid,
                                                        *pSchedulerGrpObjId);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Failed to bind child node to parent node, xpStatus: %d\n",
                           xpStatus);
            xpSaiSchedulerGroupRemoveStateData(*pSchedulerGrpObjId);
            return xpsStatus2SaiStatus(xpStatus);
        }
    }

    /* Add scheduler group to port's scheduler group list */
    xpStatus = xpSaiPortBindSchedulerGroupToPort(attributes->portId.oid, key);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Could not add scheduler group to port's "
                       "scheduler group list, port:%d, key:%d, xpStatus:%d\n",
                       (uint32_t)xpSaiObjIdValueGet(attributes->portId.oid), key, xpStatus);
        xpSaiSchedulerGroupRemoveStateData(*pSchedulerGrpObjId);
        return xpsStatus2SaiStatus(xpStatus);
    }

    return SAI_STATUS_SUCCESS;
}

//Func:  xpSaiSchedulerGroupRemove

sai_status_t xpSaiSchedulerGroupRemove(sai_object_id_t schedulerGrpObjId)
{
    xpSaiSchedulerGrp_t  *pSchedulerGrpInfo = NULL;
    sai_uint32_t          key               = 0;
    XP_STATUS             xpStatus          = XP_NO_ERR;

    /* Get the scheduler group state database */
    xpStatus = xpSaiSchedulerGroupGetStateData(schedulerGrpObjId,
                                               &pSchedulerGrpInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to get scheduler group state data | xpStatus: %d\n",
                       xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    key = (sai_uint32_t)xpSaiObjIdValueGet(schedulerGrpObjId);

    /* Remove scheduler group from port's scheduler group list */
    xpStatus = xpSaiPortUnbindSchedulerGroupFromPort(pSchedulerGrpInfo->portId,
                                                     key);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Could not remove scheduler group from port's "
                       "scheduler group list, port:%d, key:%d, xpStatus:%d\n",
                       (uint32_t)xpSaiObjIdValueGet(pSchedulerGrpInfo->portId), key, xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    /* Set scheduler profile to default */
    xpStatus = xpSaiSchedulerGroupSetSchedulerProfileId(schedulerGrpObjId,
                                                        SAI_NULL_OBJECT_ID);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to set scheduler profile to default, xpStatus: %d\n",
                       xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    /* Remove state datebase */
    xpStatus = xpSaiSchedulerGroupRemoveStateData(schedulerGrpObjId);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Could not remove scheduler group state data, xpStatus: %d\n",
                       xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetDefaultSchedulerGroupAttributeVals

void xpSaiSetDefaultSchedulerGroupAttributeVals(xpSaiSchedulerGroupAttributesT*
                                                attributes)
{
    memset(attributes, 0, sizeof(xpSaiSchedulerGroupAttributesT));
}

//Func: xpSaiUpdateSchedulerGroupAttributeVals

void xpSaiUpdateSchedulerGroupAttributeVals(const uint32_t attr_count,
                                            const sai_attribute_t* attr_list, xpSaiSchedulerGroupAttributesT* attributes)
{
    for (uint32_t count = 0; count < attr_count; count++)
    {
        switch (attr_list[count].id)
        {
            case SAI_SCHEDULER_GROUP_ATTR_CHILD_COUNT:
                {
                    attributes->childCount = attr_list[count].value;
                    break;
                }
            case SAI_SCHEDULER_GROUP_ATTR_CHILD_LIST:
                {
                    attributes->childList = attr_list[count].value;
                    break;
                }
            case SAI_SCHEDULER_GROUP_ATTR_PORT_ID:
                {
                    attributes->portId = attr_list[count].value;
                    break;
                }
            case SAI_SCHEDULER_GROUP_ATTR_LEVEL:
                {
                    attributes->level = attr_list[count].value;
                    break;
                }
            case SAI_SCHEDULER_GROUP_ATTR_MAX_CHILDS:
                {
                    attributes->maxChilds = attr_list[count].value;
                    break;
                }
            case SAI_SCHEDULER_GROUP_ATTR_SCHEDULER_PROFILE_ID:
                {
                    attributes->schedulerProfileId = attr_list[count].value;
                    break;
                }
            case SAI_SCHEDULER_GROUP_ATTR_PARENT_NODE:
                {
                    attributes->parentNode = attr_list[count].value;
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Error: Unknown attribute %d\n", attr_list[count].id);
                }
        }
    }
}

//Func: xpSaiUpdateAttrListSchedulerGroupVals

void xpSaiUpdateAttrListSchedulerGroupVals(const xpSaiSchedulerGroupAttributesT*
                                           attributes, const uint32_t attr_count, sai_attribute_t* attr_list)
{
    for (uint32_t count = 0; count < attr_count; count++)
    {
        switch (attr_list[count].id)
        {
            case SAI_SCHEDULER_GROUP_ATTR_CHILD_COUNT:
                {
                    attr_list[count].value = attributes->childCount;
                    break;
                }
            case SAI_SCHEDULER_GROUP_ATTR_CHILD_LIST:
                {
                    attr_list[count].value.objlist.count = attributes->childList.objlist.count;
                    memcpy(attr_list[count].value.objlist.list, attributes->childList.objlist.list,
                           sizeof(sai_object_id_t) * attributes->childCount.u32);
                    break;
                }
            case SAI_SCHEDULER_GROUP_ATTR_PORT_ID:
                {
                    attr_list[count].value = attributes->portId;
                    break;
                }
            case SAI_SCHEDULER_GROUP_ATTR_LEVEL:
                {
                    attr_list[count].value = attributes->level;
                    break;
                }
            case SAI_SCHEDULER_GROUP_ATTR_MAX_CHILDS:
                {
                    attr_list[count].value = attributes->maxChilds;
                    break;
                }
            case SAI_SCHEDULER_GROUP_ATTR_SCHEDULER_PROFILE_ID:
                {
                    attr_list[count].value = attributes->schedulerProfileId;
                    break;
                }
            case SAI_SCHEDULER_GROUP_ATTR_PARENT_NODE:
                {
                    attr_list[count].value = attributes->parentNode;
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Error: Unknown attribute %d\n", attr_list[count].id);
                }
        }
    }
}

//Func: xpSaiSetSchedulerGroupAttrSchedulerProfileId

sai_status_t xpSaiSetSchedulerGroupAttrSchedulerProfileId(
    sai_object_id_t scheduler_group_id, sai_attribute_value_t value)
{
    XP_STATUS    xpStatus  = XP_NO_ERR;
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;
    sai_object_id_t schedulerObjId;

    /* Validate scheduler group object id */
    if (!XDK_SAI_OBJID_TYPE_CHECK(scheduler_group_id,
                                  SAI_OBJECT_TYPE_SCHEDULER_GROUP))
    {
        XP_SAI_LOG_ERR("Error: Wrong object type received (%u)\n",
                       xpSaiObjIdTypeGet(scheduler_group_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    /* Validate scheduler object id */
    if ((value.oid != SAI_NULL_OBJECT_ID) &&
        (!XDK_SAI_OBJID_TYPE_CHECK(value.oid, SAI_OBJECT_TYPE_SCHEDULER)))
    {
        XP_SAI_LOG_ERR("Error: Wrong object type received (%u)\n",
                       xpSaiObjIdTypeGet(value.oid));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    /* Get the default Scheduler object */
    saiStatus = xpSaiSwitchGetDefaultScheduler(&schedulerObjId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Could not Get the Default Scheduler, saiStatus: %d\n",
                       saiStatus);
        return saiStatus;
    }

    /* set default scheduler group if object id is NULL */
    if (value.oid == SAI_NULL_OBJECT_ID)
    {
        xpStatus = xpSaiSchedulerGroupSetSchedulerProfileId(scheduler_group_id,
                                                            schedulerObjId);
    }
    else
    {
        xpStatus = xpSaiSchedulerGroupSetSchedulerProfileId(scheduler_group_id,
                                                            value.oid);
    }

    if (xpStatus != XP_NO_ERR)
    {
        return xpsStatus2SaiStatus(xpStatus);
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiCreateSchedulerGroup

sai_status_t xpSaiCreateSchedulerGroup(sai_object_id_t *scheduler_group_id,
                                       sai_object_id_t switch_id,
                                       uint32_t attr_count,
                                       const sai_attribute_t *attr_list)
{
    xpSaiSchedulerGroupAttributesT attributes;
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Calling xpSaiCreateSchedulerGroup\n");

    /* Validation check */
    saiStatus = xpSaiAttrCheck(attr_count, attr_list,
                               SCHEDULER_GROUP_VALIDATION_ARRAY_SIZE,
                               scheduler_group_attribs,
                               SAI_COMMON_API_CREATE);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Attribute check failed with error %d\n", saiStatus);
        return saiStatus;
    }

    xpSaiSetDefaultSchedulerGroupAttributeVals(&attributes);
    xpSaiUpdateSchedulerGroupAttributeVals(attr_count, attr_list, &attributes);

    XP_SAI_LOG_ERR("Error: Initialize with own required hierarchy is not supported\n");

    return SAI_STATUS_NOT_SUPPORTED;
}

//Func: xpSaiRemoveSchedulerGroup

sai_status_t xpSaiRemoveSchedulerGroup(sai_object_id_t scheduler_group_id)
{
    XP_SAI_LOG_DBG("Calling xpSaiRemoveSchedulerGroup\n");

    XP_SAI_LOG_ERR("Error: Initialize with own required hierarchy is not supported\n");

    return SAI_STATUS_NOT_SUPPORTED;
}

//Func: xpSaiSetSchedulerGroupAttribute

sai_status_t xpSaiSetSchedulerGroupAttribute(sai_object_id_t scheduler_group_id,
                                             const sai_attribute_t *attr)
{
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;

    saiStatus = xpSaiAttrCheck(1, attr, SCHEDULER_GROUP_VALIDATION_ARRAY_SIZE,
                               scheduler_group_attribs, SAI_COMMON_API_SET);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Attribute check failed with error %d\n", saiStatus);
        return saiStatus;
    }

    switch (attr->id)
    {
        case SAI_SCHEDULER_GROUP_ATTR_CHILD_COUNT:
        case SAI_SCHEDULER_GROUP_ATTR_CHILD_LIST:
        case SAI_SCHEDULER_GROUP_ATTR_PORT_ID:
        case SAI_SCHEDULER_GROUP_ATTR_LEVEL:
        case SAI_SCHEDULER_GROUP_ATTR_MAX_CHILDS:
            {
                XP_SAI_LOG_ERR("Error: Create-Only/Read-Only attribute %d\n", attr->id);
                return SAI_STATUS_INVALID_ATTRIBUTE_0 + SAI_STATUS_CODE(attr->id);
            }
        case SAI_SCHEDULER_GROUP_ATTR_SCHEDULER_PROFILE_ID:
            {
                /* Scheucler ID */
                saiStatus = xpSaiSetSchedulerGroupAttrSchedulerProfileId(scheduler_group_id,
                                                                         attr->value);
                if (SAI_STATUS_SUCCESS != saiStatus)
                {
                    XP_SAI_LOG_ERR("Error: Failed to set (SAI_SCHEDULER_GROUP_ATTR_SCHEDULER_PROFILE_ID)\n");
                    return saiStatus;
                }
                break;
            }
        case SAI_SCHEDULER_GROUP_ATTR_PARENT_NODE:
            {
                /* Scheduler group parent node */
                XP_SAI_LOG_ERR("Error: Scheduler group hierarchy restructuring is not supported\n");
                return SAI_STATUS_ATTR_NOT_SUPPORTED_0 + SAI_STATUS_CODE(attr->id);
            }
        default:
            {
                XP_SAI_LOG_ERR("Error: Unknown attribute %d received\n", attr->id);
                return SAI_STATUS_UNKNOWN_ATTRIBUTE_0 + SAI_STATUS_CODE(attr->id);
            }
    }

    return saiStatus;
}

//Func: xpSaiGetSchedulerGroupAttribute

sai_status_t xpSaiGetSchedulerGroupAttribute(sai_object_id_t scheduler_group_id,
                                             uint32_t attr_count, sai_attribute_t *attr_list)
{
    xpSaiSchedulerGroupAttributesT attributes;
    xpSaiSchedulerGrp_t  *pSchedulerGrpInfo = NULL;
    sai_status_t          saiStatus = SAI_STATUS_SUCCESS;
    XP_STATUS             xpStatus = XP_NO_ERR;

    memset(&attributes, 0, sizeof(xpSaiSchedulerGroupAttributesT));

    saiStatus = xpSaiAttrCheck(attr_count, attr_list,
                               SCHEDULER_GROUP_VALIDATION_ARRAY_SIZE,
                               scheduler_group_attribs, SAI_COMMON_API_GET);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Attribute check failed with error %d\n", saiStatus);
        return saiStatus;
    }

    /* Get the scheduler group state database */
    xpStatus = xpSaiSchedulerGroupGetStateData(scheduler_group_id,
                                               &pSchedulerGrpInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to get scheduler group state data | xpStatus: %d\n",
                       xpStatus);
        return xpStatus;
    }

    attributes.childCount.u32          = pSchedulerGrpInfo->childCount;
    attributes.portId.oid              = pSchedulerGrpInfo->portId;
    attributes.level.u8                = pSchedulerGrpInfo->level;
    attributes.maxChilds.u8            = pSchedulerGrpInfo->maxChilds;
    attributes.schedulerProfileId.oid  = pSchedulerGrpInfo->schedulerProfileId;
    attributes.parentNode.oid          = pSchedulerGrpInfo->parentNode;
    attributes.childList.objlist.count = pSchedulerGrpInfo->childList.count;
    attributes.childList.objlist.list  = pSchedulerGrpInfo->childList.list;

    xpSaiUpdateAttrListSchedulerGroupVals(&attributes, attr_count, attr_list);

    return saiStatus;
}

//Func: xpSaiSchedulerGroupApiInit

XP_STATUS xpSaiSchedulerGroupApiInit(uint64_t flag,
                                     const sai_service_method_table_t* adapHostServiceMethodTable)
{
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Calling xpSaiSchedulerGroupApiInit\n");

    _xpSaiSchedulerGroupApi = (sai_scheduler_group_api_t *) xpMalloc(sizeof(
                                                                         sai_scheduler_group_api_t));
    if (NULL == _xpSaiSchedulerGroupApi)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for _xpSaiSchedulerGroupApi\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }

    _xpSaiSchedulerGroupApi->create_scheduler_group         =
        xpSaiCreateSchedulerGroup;
    _xpSaiSchedulerGroupApi->remove_scheduler_group         =
        xpSaiRemoveSchedulerGroup;
    _xpSaiSchedulerGroupApi->set_scheduler_group_attribute  =
        xpSaiSetSchedulerGroupAttribute;
    _xpSaiSchedulerGroupApi->get_scheduler_group_attribute  =
        xpSaiGetSchedulerGroupAttribute;

    saiStatus = xpSaiApiRegister(SAI_API_SCHEDULER_GROUP,
                                 (void*)_xpSaiSchedulerGroupApi);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Failed to register scheduler group API\n");
        return XP_ERR_ARRAY_OUT_OF_BOUNDS;
    }

    return XP_NO_ERR;
}

//Func: xpSaiSchedulerGroupApiDeinit

XP_STATUS xpSaiSchedulerGroupApiDeinit()
{
    XP_STATUS saiStatus = XP_NO_ERR;

    XP_SAI_LOG_DBG("Calling xpSaiSchedulerGroupApiDeinit\n");

    xpFree(_xpSaiSchedulerGroupApi);
    _xpSaiSchedulerGroupApi = NULL;

    return saiStatus;
}
