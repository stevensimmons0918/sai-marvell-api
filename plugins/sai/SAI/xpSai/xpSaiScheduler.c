// xpSaiScheduler.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpSaiScheduler.h"
#include "xpSaiSchedulerGroup.h"
#include "xpSaiValidationArrays.h"
#include "cpssHalQos.h"

XP_SAI_LOG_REGISTER_API(SAI_API_SCHEDULER);

/* 16 port Scheduler profiles supported in falcon.
 * 16 queues per port. (16*16 = 256)
 * Additional profiles for Port and Level-A nodes.
 */
#define XP_SAI_SCHD_PFL_MAX_IDS     512

#define XP_SAI_SCHD_PFL_RANGE_START 1

sai_scheduler_api_t* _xpSaiSchedulerApi;

xpsDbHandle_t schedulerDbHandle = XPSAI_SCHEDULER_PROFILE_DB_HNDL;

//Func: xpSaiSchedulerKeyComp

static sai_int32_t xpSaiSchedulerKeyComp(void *key1, void *key2)
{
    sai_int64_t val1 = ((xpSaiSchedulerProfile_t*)key1)->schedObjId;
    sai_int64_t val2 = ((xpSaiSchedulerProfile_t*)key2)->schedObjId;

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

//Func: xpSaiCreateSchedulerStateData

XP_STATUS xpSaiCreateSchedulerStateData(sai_object_id_t schedObjId,
                                        xpSaiSchedulerProfile_t **schedInfo)
{
    XP_STATUS retVal = XP_NO_ERR;

    /* Create a new sai scheduler db */
    if ((retVal = xpsStateHeapMalloc(sizeof(xpSaiSchedulerProfile_t),
                                     (void**)schedInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Couldn't allocate heap memory | retVal: %d\n", retVal);
        return retVal;
    }
    memset(*schedInfo, 0x0, sizeof(xpSaiSchedulerProfile_t));

    /* Populate key value */
    (*schedInfo)->schedObjId = schedObjId;

    /* Insert the state into the database */
    if ((retVal = xpsStateInsertData(XP_SCOPE_DEFAULT, schedulerDbHandle,
                                     (void*)*schedInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Couldn't insert state into data base | retVal: %d\n",
                       retVal);
        xpsStateHeapFree((void*)*schedInfo);
        return retVal;
    }

    return XP_NO_ERR;
}

//Func: xpSaiRemoveSchedulerStateData

XP_STATUS xpSaiRemoveSchedulerStateData(sai_object_id_t schedObjId)
{
    xpSaiSchedulerProfile_t *schedInfo = NULL;
    xpSaiSchedulerProfile_t  schedKey = { 0 };
    XP_STATUS                retVal = XP_NO_ERR;

    schedKey.schedObjId = schedObjId;

    /* Delete the corresponding state */
    if ((retVal = xpsStateDeleteData(XP_SCOPE_DEFAULT, schedulerDbHandle,
                                     (xpsDbKey_t)&schedKey, (void**)&schedInfo))!= XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Couldn't not delete scheduler state data | retVal: %d\n",
                       retVal);
        return retVal;
    }

    if (!schedInfo)
    {
        XP_SAI_LOG_DBG("Error: Could not find entry\n");
        return XP_ERR_NOT_FOUND;
    }

    /* Free the memory allocated for the corresponding state */
    if ((retVal = xpsStateHeapFree(schedInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Couldn't free heap memory | retVal: %d\n", retVal);
        return retVal;
    }

    return XP_NO_ERR;
}

//Func: xpSaiGetSchedulerStateData

XP_STATUS xpSaiGetSchedulerStateData(sai_object_id_t schedObjId,
                                     xpSaiSchedulerProfile_t **schedInfo)
{
    xpSaiSchedulerProfile_t  schedInfoKey = { 0 };
    XP_STATUS                retVal = XP_NO_ERR;

    schedInfoKey.schedObjId = schedObjId;

    /* Retrieve the corresponding state from sai scheduler profile db */
    if ((retVal = xpsStateSearchData(XP_SCOPE_DEFAULT, schedulerDbHandle,
                                     (xpsDbKey_t)&schedInfoKey, (void**)schedInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to retrieve the data | retVal: %d\n", retVal);
        return retVal;
    }

    if (!(*schedInfo))
    {
        XP_SAI_LOG_ERR("Error: Scheduler profile entry not found\n");
        return XP_ERR_NOT_FOUND;
    }

    return XP_NO_ERR;
}

//Func: xpSaiGetSchedulerAttrInfo

XP_STATUS xpSaiGetSchedulerAttrInfo(sai_object_id_t schedObjId,
                                    xpSaiSchedulerAttributesT *schedAttr)
{
    xpSaiSchedulerProfile_t *schedInfo = NULL;
    XP_STATUS                retVal = XP_NO_ERR;

    /* Get the scheduler state data */
    if ((retVal = xpSaiGetSchedulerStateData(schedObjId, &schedInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Could not retrieve scheduler state info\n");
        return retVal;
    }

    /* Copy the scheduler attributes */
    memcpy(schedAttr, &schedInfo->attrInfo, sizeof(xpSaiSchedulerAttributesT));

    return XP_NO_ERR;
}

//Func: xpSaiCreateSchedulerProfile

sai_status_t xpSaiCreateSchedulerProfile(uint32_t *profileId)
{
    XP_STATUS xpsRetVal;

    if ((xpsRetVal = xpsAllocatorAllocateId(XP_SCOPE_DEFAULT, XP_SAI_ALLOC_SCHD_PFL,
                                            profileId)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to allocate Sai Scheduler Profile |Error code: %d\n",
                       xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }
    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiRemoveSchedulerProfile

sai_status_t xpSaiRemoveSchedulerProfile(uint32_t profileId)
{
    XP_STATUS xpsRetVal;

    if ((xpsRetVal = xpsAllocatorReleaseId(XP_SCOPE_DEFAULT, XP_SAI_ALLOC_SCHD_PFL,
                                           profileId)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to release Sai Scheduler Profile |Error code: %d\n",
                       xpsRetVal);
        return xpsStatus2SaiStatus(xpsRetVal);
    }
    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetDefaultSchedulerAttributeVals

void xpSaiSetDefaultSchedulerAttributeVals(xpSaiSchedulerAttributesT*
                                           attributes)
{
    memset(attributes, 0x0, sizeof(xpSaiSchedulerAttributesT));

    /* Scheduling algorithm [sai_scheduling_type_t], Default WRR */
    attributes->schedulingType.s32 = SAI_SCHEDULING_TYPE_DWRR;

    /* Scheduling algorithm weight, Range [1 - 100], Default Weight = 1 */
    attributes->schedulingWeight.u8 = 1;

    /* Shaper type [sai_meter_type_t], Default BYTES */
    attributes->meterType.s32 = SAI_METER_TYPE_BYTES;

    /* Guaranteed Bandwidth shape rate [bytes/sec], Value 0 to No Limit, Default 0 */
    attributes->minBandwidthRate.u64 = 0;

    /* Guaranteed Burst for Bandwidth shape rate [Bytes or Packets] */
    attributes->minBandwidthBurstRate.u64 = 0;

    /* Maximum Bandwidth shape rate [bytes/sec], Value 0 to No Limit, Default 0 */
    attributes->maxBandwidthRate.u64 = 0;

    /* Maximum Burst for Bandwidth shape rate [bytes or Packets] */
    attributes->maxBandwidthBurstRate.u64 = 0;
}

//Func: xpSaiUpdateSchedulerAttributeVals

void xpSaiUpdateSchedulerAttributeVals(const uint32_t attr_count,
                                       const sai_attribute_t* attr_list, xpSaiSchedulerAttributesT* attributes)
{
    for (uint32_t count = 0; count < attr_count; count++)
    {
        switch (attr_list[count].id)
        {
            case SAI_SCHEDULER_ATTR_SCHEDULING_TYPE:
                {
                    attributes->schedulingType = attr_list[count].value;
                    break;
                }
            case SAI_SCHEDULER_ATTR_SCHEDULING_WEIGHT:
                {
                    attributes->schedulingWeight = attr_list[count].value;
                    break;
                }
            case SAI_SCHEDULER_ATTR_METER_TYPE:
                {
                    attributes->meterType = attr_list[count].value;
                    break;
                }
            case SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_RATE:
                {
                    attributes->minBandwidthRate = attr_list[count].value;
                    break;
                }
            case SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_BURST_RATE:
                {
                    attributes->minBandwidthBurstRate = attr_list[count].value;
                    break;
                }
            case SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_RATE:
                {
                    attributes->maxBandwidthRate = attr_list[count].value;
                    break;
                }
            case SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_BURST_RATE:
                {
                    attributes->maxBandwidthBurstRate = attr_list[count].value;
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Failed to set %d\n", attr_list[count].id);
                }
        }
    }
}

//Func: xpSaiUpdateAttrListSchedulerVals

void xpSaiUpdateAttrListSchedulerVals(const xpSaiSchedulerAttributesT*
                                      attributes, const uint32_t attr_count, sai_attribute_t* attr_list)
{
    for (uint32_t count = 0; count < attr_count; count++)
    {
        switch (attr_list[count].id)
        {
            case SAI_SCHEDULER_ATTR_SCHEDULING_TYPE:
                {
                    attr_list[count].value = attributes->schedulingType;
                    break;
                }
            case SAI_SCHEDULER_ATTR_SCHEDULING_WEIGHT:
                {
                    attr_list[count].value = attributes->schedulingWeight;
                    break;
                }
            case SAI_SCHEDULER_ATTR_METER_TYPE:
                {
                    attr_list[count].value = attributes->meterType;
                    break;
                }
            case SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_RATE:
                {
                    attr_list[count].value = attributes->minBandwidthRate;
                    break;
                }
            case SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_BURST_RATE:
                {
                    attr_list[count].value = attributes->minBandwidthBurstRate;
                    break;
                }
            case SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_RATE:
                {
                    attr_list[count].value = attributes->maxBandwidthRate;
                    break;
                }
            case SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_BURST_RATE:
                {
                    attr_list[count].value = attributes->maxBandwidthBurstRate;
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Failed to set %d\n", attr_list[count].id);
                }
        }
    }
}

//Func: xpSaiCreateScheduler

sai_status_t xpSaiCreateScheduler(sai_object_id_t *scheduler_id,
                                  sai_object_id_t switch_id, uint32_t attr_count,
                                  const sai_attribute_t *attr_list)
{
    xpSaiSchedulerAttributesT attributes;
    xpSaiSchedulerProfile_t  *schedInfo  = NULL;
    sai_uint32_t              profileId  = 0;
    sai_status_t              retVal     = SAI_STATUS_SUCCESS;
    xpsDevice_t               xpDevId    = xpSaiObjIdSwitchGet(switch_id);
    XP_STATUS                 xpRetVal   = XP_NO_ERR;

    retVal = xpSaiAttrCheck(attr_count, attr_list,
                            SCHEDULER_VALIDATION_ARRAY_SIZE, scheduler_attribs,
                            SAI_COMMON_API_CREATE);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", retVal);
        return retVal;
    }

    xpSaiSetDefaultSchedulerAttributeVals(&attributes);
    xpSaiUpdateSchedulerAttributeVals(attr_count, attr_list, &attributes);

    /* Validate scheduler weight */
    if ((attributes.schedulingType.s32 != SAI_SCHEDULING_TYPE_STRICT) &&
        (!XPSAI_SCHEDULER_IS_WEIGHT_VALID(attributes.schedulingWeight.u8)))
    {
        XP_SAI_LOG_ERR("Error: Invalid weight %d provided\n",
                       attributes.schedulingWeight.u8);
        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }
    else if (attributes.schedulingType.s32 == SAI_SCHEDULING_TYPE_STRICT)
    {
        attributes.schedulingWeight.u8 = 1;
    }

    /* Validate scheduling type (strict/wrr/dwrr) */
    if (attributes.schedulingType.s32 == SAI_SCHEDULING_TYPE_WRR)
    {
        XP_SAI_LOG_ERR("Error: WRR scheduling type is not supported\n");
        return SAI_STATUS_NOT_SUPPORTED;
    }

    /* Validate scheduler meter type (packet/byte) */
    if (attributes.meterType.s32 != SAI_METER_TYPE_BYTES)
    {
        XP_SAI_LOG_ERR("Meter type %d is not supported. Device only supports metering based on bytes\n",
                       attributes.meterType.s32);
        return SAI_STATUS_NOT_SUPPORTED;
    }

    /* Create the scheduler profile */
    if ((retVal = xpSaiCreateSchedulerProfile(&profileId)) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Failed to create scheduler profile | retVal:%d\n",
                       retVal);
        return retVal;
    }

    /* Create scheduler profile object id */
    if (xpSaiObjIdCreate(SAI_OBJECT_TYPE_SCHEDULER, xpDevId,
                         (sai_uint64_t)profileId, scheduler_id) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Could not create SAI object\n");
        xpSaiRemoveSchedulerProfile(profileId);
        return SAI_STATUS_FAILURE;
    }

    /* Create state datebase */
    if ((xpRetVal = xpSaiCreateSchedulerStateData(*scheduler_id,
                                                  &schedInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Could not create scheduler state database\n");
        xpSaiRemoveSchedulerProfile(profileId);
        return xpsStatus2SaiStatus(xpRetVal);
    }

    /* Update the state data with scheduler attributes */
    schedInfo->attrInfo = attributes;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiRemoveScheduler

sai_status_t xpSaiRemoveScheduler(sai_object_id_t scheduler_id)
{
    xpSaiSchedulerProfile_t *pSchedulerInfo = NULL;
    sai_uint32_t             profileId      = 0;
    sai_status_t             retVal         = SAI_STATUS_SUCCESS;
    XP_STATUS                xpRetVal       = XP_NO_ERR;

    if (!XDK_SAI_OBJID_TYPE_CHECK(scheduler_id, SAI_OBJECT_TYPE_SCHEDULER))
    {
        XP_SAI_LOG_ERR("Wrong object 0x%" PRIx64 " passed!\n", scheduler_id);
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    /* Get the scheduler state data */
    xpRetVal = xpSaiGetSchedulerStateData(scheduler_id, &pSchedulerInfo);
    if (xpRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to get scheduler state data, xpStatus: %d\n",
                       xpRetVal);
        return xpsStatus2SaiStatus(xpRetVal);
    }

    if (pSchedulerInfo->numNodes != 0)
    {
        XP_SAI_LOG_ERR("Error: Scheduler is in use, unmap before remove\n");
        return SAI_STATUS_OBJECT_IN_USE;
    }

    /* Remove state datebase */
    if ((xpRetVal = xpSaiRemoveSchedulerStateData(scheduler_id)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Could not remove scheduler state data |retVal: %d\n",
                       xpRetVal);
        return xpsStatus2SaiStatus(xpRetVal);
    }

    /* Get the scheduler profile id */
    profileId = (sai_uint32_t)xpSaiObjIdValueGet(scheduler_id);

    /* Remove the scheduler profile */
    if ((retVal = xpSaiRemoveSchedulerProfile(profileId)) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Could not remove scheduler profile %d | retVal:%d\n",
                       profileId, retVal);
        return retVal;
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetSchedulerQueueSchedulingType

sai_status_t xpSaiSetSchedulerQueueSchedulingType(uint32_t xpDevId,
                                                  uint32_t xpPort, uint32_t queue, uint32_t schedType)
{
    GT_STATUS rc = GT_OK;

    switch (schedType)
    {
        case SAI_SCHEDULING_TYPE_STRICT:
            {
                /* Strict Priority Scheduling */
                if ((rc = cpssHalPortTxQueueSchedulerTypeSet(xpDevId, xpPort, queue,
                                                             CPSS_PORT_TX_SP_ARB_GROUP_E)) != GT_OK)
                {
                    XP_SAI_LOG_ERR("Error: Could not set queue scheduler type to strict | retVal: %d\n",
                                   rc);
                    return cpssStatus2SaiStatus(rc);
                }
                break;
            }
        case SAI_SCHEDULING_TYPE_DWRR:
            {
                /* Deficit Weighted Round-Robin Scheduling */
                /* using wrr arbitration group-0 for DWRR */
                if ((rc = cpssHalPortTxQueueSchedulerTypeSet(xpDevId, xpPort, queue,
                                                             CPSS_PORT_TX_WRR_ARB_GROUP_0_E)) != GT_OK)
                {
                    XP_SAI_LOG_ERR("Error: Could not set queue scheduler type to dwrr | retVal: %d\n",
                                   rc);
                    return cpssStatus2SaiStatus(rc);
                }
                break;
            }
        case SAI_SCHEDULING_TYPE_WRR:
            {
                /* Weighted Round-Robin Scheduling */
                XP_SAI_LOG_DBG("Weighted Round-Robin Scheduling not supported\n");
                return SAI_STATUS_NOT_SUPPORTED;
            }
        default:
            {
                XP_SAI_LOG_ERR("Error: Invalid scheduler type %d\n", schedType);
                break;
            }
    }

    return SAI_STATUS_SUCCESS;
}

#if 0
//Func: xpSaiSetSchedulerH1SchedulingType

XP_STATUS xpSaiSetSchedulerH1SchedulingType(xpsDevice_t xpDevId,
                                            uint32_t h1NodeKey, uint32_t schedType)
{
    xpTxqQueuePathToPort_t path;
    XP_STATUS xpRetVal = XP_NO_ERR;
    uint32_t priority = 0;

    /* Get the scheduler group node to port path */
    xpRetVal = xpSaiSchedulerGroupGetNodeToPortPath(xpDevId, h1NodeKey, &path);
    if (xpRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Could not get scheduler group node to port path | retVal: %d\n",
                       xpRetVal);
        return xpRetVal;
    }

    switch (schedType)
    {
        case SAI_SCHEDULING_TYPE_STRICT:
            {
                /* Strict Scheduling */
                xpRetVal = xpsQosSetH1SchedulerSP(xpDevId, path, 1);
                if (xpRetVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("Error: Could not set H1 scheduler type to strict | retVal: %d\n",
                                   xpRetVal);
                    return xpRetVal;
                }
                priority = (path.h1Num % XPSAI_SG_MAX_CHILDS_AT_LEVEL1) + 1;
                break;
            }
        case SAI_SCHEDULING_TYPE_DWRR:
            {
                /* Deficit Weighted Round-Robin Scheduling */
                xpRetVal = xpsQosSetH1SchedulerDWRR(xpDevId, path, 1);
                if (xpRetVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("Error: Could not set port scheduler type to dwrr | retVal: %d\n",
                                   xpRetVal);
                    return xpRetVal;
                }
                break;
            }
        case SAI_SCHEDULING_TYPE_WRR:
            {
                /* Weighted Round-Robin Scheduling */
                XP_SAI_LOG_ERR("Error: Weighted Round-Robin Scheduling not supported\n");
                return XP_ERR_NOT_SUPPORTED;
            }
        default:
            {
                XP_SAI_LOG_ERR("Error: Invalid scheduler type %d\n", schedType);
                break;
            }
    }

    /* Setting priority */
    xpRetVal = xpsQosSetH1SchedulerSPPriority(xpDevId, path, priority);
    if (xpRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Could not set H1 scheduler group priority | retVal: %d\n",
                       xpRetVal);
        return xpRetVal;
    }

    return XP_NO_ERR;
}

//Func: xpSaiSetSchedulerH2SchedulingType

XP_STATUS xpSaiSetSchedulerH2SchedulingType(xpsDevice_t xpDevId,
                                            uint32_t h2NodeKey, uint32_t schedType)
{
    xpTxqQueuePathToPort_t path;
    XP_STATUS xpRetVal = XP_NO_ERR;

    /* Get the scheduler group node to port path */
    xpRetVal = xpSaiSchedulerGroupGetNodeToPortPath(xpDevId, h2NodeKey, &path);
    if (xpRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Could not get scheduler group node to port path | retVal: %d\n",
                       xpRetVal);
        return xpRetVal;
    }

    switch (schedType)
    {
        case SAI_SCHEDULING_TYPE_STRICT:
            {
                /* Strict Scheduling */
                xpRetVal = xpsQosSetH2SchedulerSP(xpDevId, path, 1);
                if (xpRetVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("Error: Could not set H1 scheduler type to strict | retVal: %d\n",
                                   xpRetVal);
                    return xpRetVal;
                }
                break;
            }
        case SAI_SCHEDULING_TYPE_DWRR:
            {
                /* Deficit Weighted Round-Robin Scheduling */
                xpRetVal = xpsQosSetH2SchedulerDWRR(xpDevId, path, 1);
                if (xpRetVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("Error: Could not set port scheduler type to dwrr | retVal: %d\n",
                                   xpRetVal);
                    return xpRetVal;
                }
                break;
            }
        case SAI_SCHEDULING_TYPE_WRR:
            {
                /* Weighted Round-Robin Scheduling */
                XP_SAI_LOG_ERR("Error: Weighted Round-Robin Scheduling not supported\n");
                return XP_ERR_NOT_SUPPORTED;
            }
        default:
            {
                XP_SAI_LOG_ERR("Error: Invalid scheduler type %d\n", schedType);
                break;
            }
    }

    return XP_NO_ERR;
}
#endif

//Func: xpSaiSetSchedulerPortSchedulingType

XP_STATUS xpSaiSetSchedulerPortSchedulingType(uint32_t xpDevId, uint32_t xpPort,
                                              uint32_t schedType)
{
    XP_STATUS xpRetVal = XP_NO_ERR;

    switch (schedType)
    {
        case SAI_SCHEDULING_TYPE_STRICT:
            {
                /* Strict Scheduling */
                if ((xpRetVal = xpsQosSetPortSchedulerSP(xpDevId, xpPort, 1)) != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("Error: Could not set port scheduler type to strict | retVal: %d\n",
                                   xpRetVal);
                    return xpRetVal;
                }
                break;
            }
        case SAI_SCHEDULING_TYPE_DWRR:
            {
                /* Deficit Weighted Round-Robin Scheduling */
                if ((xpRetVal = xpsQosSetPortSchedulerDWRR(xpDevId, xpPort, 1)) != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("Error: Could not set port scheduler type to dwrr | retVal: %d\n",
                                   xpRetVal);
                    return xpRetVal;
                }
                break;
            }
        case SAI_SCHEDULING_TYPE_WRR:
            {
                /* Weighted Round-Robin Scheduling */
                XP_SAI_LOG_ERR("Weighted Round-Robin Scheduling not supported\n");
                return XP_ERR_NOT_SUPPORTED;
            }
        default:
            {
                XP_SAI_LOG_ERR("Error: Invalid scheduler type %d\n", schedType);
                break;
            }
    }

    return XP_NO_ERR;
}

//Func: xpSaiSetSchedulerAttrSchedulingType

sai_status_t xpSaiSetSchedulerAttrSchedulingType(sai_object_id_t scheduler_id,
                                                 sai_attribute_value_t value)
{
    xpSaiSchedulerProfile_t *schedInfo = NULL;
    sai_uint32_t             nodeKey   = 0;
    sai_uint32_t             queue     = 0;
    sai_uint32_t             xpPort    = 0;
    sai_status_t             saiStatus = SAI_STATUS_SUCCESS;
    XP_STATUS                xpRetVal  = XP_NO_ERR;

    /* Get the scheduler state data */
    if ((xpRetVal = xpSaiGetSchedulerStateData(scheduler_id,
                                               &schedInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Could not retrieve scheduler state info | retVal:%d\n",
                       xpRetVal);
        return xpsStatus2SaiStatus(xpRetVal);
    }

    xpsDevice_t xpDevId = xpSaiObjIdSwitchGet(scheduler_id);

    for (uint32_t count = 0; count < schedInfo->numNodes; count++)
    {
        nodeKey = schedInfo->nodeList[count].nodeIndex;

        switch (schedInfo->nodeList[count].nodeType)
        {
            case XPSAI_SCHEDULER_QUEUE_NODE:
                {
                    /* Get the port and queue from queue node index */
                    xpSaiQueueGetPortAndQueueFromKey(nodeKey, &xpPort, &queue);

                    saiStatus = xpSaiSetSchedulerQueueSchedulingType(xpDevId, xpPort, queue,
                                                                     value.s32);
                    if (saiStatus != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Error: Could not set queue scheduling type | saiStatus:%d\n",
                                       saiStatus);
                        return saiStatus;
                    }
                    break;
                }

            case XPSAI_SCHEDULER_LEVEL2_NODE:
            case XPSAI_SCHEDULER_LEVEL1_NODE:
            case XPSAI_SCHEDULER_LEVEL0_NODE:
                /* do nothing */
                /* HQoS is not supported */
                break;

            case XPSAI_SCHEDULER_PORT_NODE:
                {
                    xpRetVal = xpSaiSetSchedulerPortSchedulingType(xpDevId, nodeKey, value.s32);
                    if (xpRetVal != XP_NO_ERR)
                    {
                        XP_SAI_LOG_ERR("Error: Could not set port scheduling type | retVal:%d\n",
                                       xpRetVal);
                        return xpsStatus2SaiStatus(xpRetVal);
                    }
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Error: Invalid scheduler node type %d\n",
                                   schedInfo->nodeList[count].nodeType);
                    return SAI_STATUS_INVALID_PARAMETER;
                }
        }
    }

    /* Update the scheduler state database */
    schedInfo->attrInfo.schedulingType.s32 = value.s32;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetSchedulerAttrSchedulingWeight

sai_status_t xpSaiSetSchedulerAttrSchedulingWeight(sai_object_id_t scheduler_id,
                                                   sai_attribute_value_t value)
{
    xpSaiSchedulerProfile_t *schedInfo = NULL;
    sai_uint32_t             nodeKey = 0;
    sai_uint32_t             xpPort = 0;
    sai_uint32_t             queue = 0;
    XP_STATUS                xpRetVal = XP_NO_ERR;
    GT_STATUS                rc = GT_OK;

    /* Validate scheduler weight */
    if (!XPSAI_SCHEDULER_IS_WEIGHT_VALID(value.u8))
    {
        XP_SAI_LOG_ERR("Error: Invalid weight %d provided\n", value.u8);
        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    /* Get the scheduler state data */
    if ((xpRetVal = xpSaiGetSchedulerStateData(scheduler_id,
                                               &schedInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Could not retrieve scheduler state info | retVal:%d\n",
                       xpRetVal);
        return xpsStatus2SaiStatus(xpRetVal);
    }

    xpsDevice_t xpDevId = xpSaiObjIdSwitchGet(scheduler_id);

    for (uint32_t count = 0; count < schedInfo->numNodes; count++)
    {
        nodeKey = schedInfo->nodeList[count].nodeIndex;

        switch (schedInfo->nodeList[count].nodeType)
        {
            case XPSAI_SCHEDULER_QUEUE_NODE:
                {
                    /* Get the port and queue from queue node index */
                    xpSaiQueueGetPortAndQueueFromKey(nodeKey, &xpPort, &queue);

                    rc = cpssHalPortTxQueueWRRWeightSet(xpDevId, xpPort, queue, value.u8);
                    if (rc != GT_OK)
                    {
                        XP_SAI_LOG_ERR("Error: Could not set queue scheduler DWRR weight | retVal:%d",
                                       rc);
                        return cpssStatus2SaiStatus(rc);
                    }
                    break;
                }

            case XPSAI_SCHEDULER_LEVEL2_NODE:
            case XPSAI_SCHEDULER_LEVEL1_NODE:
            case XPSAI_SCHEDULER_LEVEL0_NODE:
                /* do nothing */
                /* HQoS is not supported */
                break;

            case XPSAI_SCHEDULER_PORT_NODE:
                {
                    xpRetVal = xpsQosSetPortSchedulerDWRRWeight(xpDevId, nodeKey, value.u8);
                    if (xpRetVal != XP_NO_ERR)
                    {
                        XP_SAI_LOG_ERR("Error: Could not set port scheduler DWRR weight | retVal:%d",
                                       xpRetVal);
                        return xpsStatus2SaiStatus(xpRetVal);
                    }
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Error: Invalid scheduler node type %d\n",
                                   schedInfo->nodeList[count].nodeType);
                    return SAI_STATUS_INVALID_PARAMETER;
                }
        }
    }

    /* Update the scheduler stat database */
    schedInfo->attrInfo.schedulingWeight.u8 = value.u8;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetSchedulerAttrMeterType

sai_status_t xpSaiSetSchedulerAttrMeterType(sai_object_id_t scheduler_id,
                                            sai_attribute_value_t value)
{
    xpSaiSchedulerProfile_t *schedInfo = NULL;
    XP_STATUS                xpRetVal;

    /* Get the scheduler state data */
    if ((xpRetVal = xpSaiGetSchedulerStateData(scheduler_id,
                                               &schedInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Could not retrieve scheduler state info | retVal:%d\n",
                       xpRetVal);
        return xpsStatus2SaiStatus(xpRetVal);
    }

    if (value.s32 != SAI_METER_TYPE_BYTES)
    {
        XP_SAI_LOG_ERR("Meter type %d is not supported. Device only supports metering based on bytes\n",
                       value.s32);
        return SAI_STATUS_NOT_SUPPORTED;
    }

    /* Update the scheduler stat database */
    schedInfo->attrInfo.meterType.s32 = value.s32;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetSchedulerQueueShaperMaxRate

sai_status_t xpSaiSetSchedulerQueueShaperMaxRate(uint32_t xpDevId,
                                                 uint32_t xpPort, uint32_t queue, uint64_t bandwidthRate,
                                                 uint64_t bandwidthBurstRate)
{
    GT_BOOL   enable = ((bandwidthRate > 0) &&
                        (bandwidthBurstRate > 0)) ? GT_TRUE : GT_FALSE;
    GT_STATUS rc = GT_OK;

    /* No limit if bandwidthRate is set to 0. In this case disable the queue max shaper */
    if ((rc = cpssHalPortTxQMaxRateEnableSet(xpDevId, xpPort, queue,
                                             enable)) != GT_OK)
    {
        XP_SAI_LOG_ERR("Error: Could not set queue max shaper enable | retVal: %d\n",
                       rc);
        return cpssStatus2SaiStatus(rc);
    }

    /* if shaper is disabled. no need to configure */
    if (enable == GT_FALSE)
    {
        return SAI_STATUS_SUCCESS;
    }

    /* Convert Bytes to Kbps */
    sai_uint32_t rateKbps = ((bandwidthRate * XP_BITS_IN_BYTE) /
                             XP_KBPS_TO_BITS_FACTOR);
    /* Convert burstSize bytes to 4K bytes */
    sai_uint32_t burstSize = bandwidthBurstRate / 4096;

    if ((rc = cpssHalPortTxQMaxRateSet(xpDevId, xpPort, queue, burstSize,
                                       &rateKbps)) != GT_OK)
    {
        XP_SAI_LOG_ERR("Error: Could not configure queue max shaper | retVal: %d\n",
                       rc);
        return cpssStatus2SaiStatus(rc);
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetSchedulerPortShaperRate

sai_status_t xpSaiSetSchedulerPortShaperRate(uint32_t xpDevId, uint32_t xpPort,
                                             uint64_t bandwidthRate, uint64_t bandwidthBurstRate)
{
    GT_BOOL   enable = ((bandwidthRate > 0) &&
                        (bandwidthBurstRate > 0)) ? GT_TRUE : GT_FALSE;
    GT_STATUS rc = GT_OK;

    /* No limit if bandwidthRate is set to 0. Disable port shaper */
    if ((rc = cpssHalPortShaperEnableSet(xpDevId, xpPort, enable)) != GT_OK)
    {
        XP_SAI_LOG_ERR("Error: Could not set port shaper enable | retVal: %d\n", rc);
        return cpssStatus2SaiStatus(rc);
    }

    /* if shaper is disabled. no need to configure */
    if (enable == GT_FALSE)
    {
        return SAI_STATUS_SUCCESS;
    }

    /* Convert Bytes to Kbps */
    sai_uint32_t rateKbps = ((bandwidthRate * XP_BITS_IN_BYTE) /
                             XP_KBPS_TO_BITS_FACTOR);
    /* Convert burstSize bytes to 4K bytes */
    sai_uint32_t burstSize = bandwidthBurstRate / 4096;

    /* Configure port shaper */
    if ((rc = cpssHalPortShaperRateSet(xpDevId, xpPort, burstSize,
                                       &rateKbps)) != GT_OK)
    {
        XP_SAI_LOG_ERR("Error: Could not configure port shaper | retVal: %d\n", rc);
        return cpssStatus2SaiStatus(rc);
    }

    return XP_NO_ERR;
}

//Func: xpSaiSetSchedulerAttrMaxBandwidthRate

sai_status_t xpSaiSetSchedulerAttrMaxBandwidthRate(sai_object_id_t scheduler_id,
                                                   sai_attribute_value_t value)
{
    xpSaiSchedulerProfile_t *schedInfo = NULL;
    sai_uint32_t             nodeKey = 0;
    sai_uint32_t             xpPort = 0;
    sai_uint32_t             queue = 0;
    XP_STATUS                xpRetVal;
    sai_status_t retVal;

    /* Get the scheduler state data */
    if ((xpRetVal = xpSaiGetSchedulerStateData(scheduler_id,
                                               &schedInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Could not retrieve scheduler state info | retVal:%d\n",
                       xpRetVal);
        return xpsStatus2SaiStatus(xpRetVal);
    }

    sai_uint64_t burstSize = schedInfo->attrInfo.maxBandwidthBurstRate.u64;
    xpsDevice_t  xpDevId   = xpSaiObjIdSwitchGet(scheduler_id);

    for (uint32_t count = 0; count < schedInfo->numNodes; count++)
    {
        nodeKey = schedInfo->nodeList[count].nodeIndex;

        switch (schedInfo->nodeList[count].nodeType)
        {
            case XPSAI_SCHEDULER_QUEUE_NODE:
                {
                    /* Get the port and queue from queue node index */
                    xpSaiQueueGetPortAndQueueFromKey(nodeKey, &xpPort, &queue);

                    retVal = xpSaiSetSchedulerQueueShaperMaxRate(xpDevId, xpPort, queue, value.u64,
                                                                 burstSize);
                    if (SAI_STATUS_SUCCESS != retVal)
                    {
                        XP_SAI_LOG_ERR("Error: Could not set queue shaper rate | retVal:%d", retVal);
                        return retVal;
                    }
                    break;
                }

            case XPSAI_SCHEDULER_LEVEL2_NODE:
            case XPSAI_SCHEDULER_LEVEL1_NODE:
            case XPSAI_SCHEDULER_LEVEL0_NODE:
                /* do nothing */
                /* HQoS is not supported */
                break;

            case XPSAI_SCHEDULER_PORT_NODE:
                {
                    retVal = xpSaiSetSchedulerPortShaperRate(xpDevId, nodeKey, value.u64,
                                                             burstSize);
                    if (SAI_STATUS_SUCCESS != retVal)
                    {
                        XP_SAI_LOG_ERR("Error: Could not set port shaper rate | retVal:%d", retVal);
                        return retVal;
                    }
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Error: Invalid scheduler node type %d\n",
                                   schedInfo->nodeList[count].nodeType);
                    return SAI_STATUS_INVALID_PARAMETER;
                }
        }
    }

    /* Update the scheduler stat database */
    schedInfo->attrInfo.maxBandwidthRate.u64 = value.u64;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetSchedulerAttrMaxBandwidthBurstRate

sai_status_t xpSaiSetSchedulerAttrMaxBandwidthBurstRate(
    sai_object_id_t scheduler_id, sai_attribute_value_t value)
{
    xpSaiSchedulerProfile_t *schedInfo = NULL;
    sai_uint32_t             nodeKey   = 0;
    sai_uint32_t             xpPort    = 0;
    sai_uint32_t             queue     = 0;
    XP_STATUS                xpRetVal  = XP_NO_ERR;
    sai_status_t retVal;

    /* Get the scheduler state data */
    if ((xpRetVal = xpSaiGetSchedulerStateData(scheduler_id,
                                               &schedInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Could not retrieve scheduler state info | retVal:%d\n",
                       xpRetVal);
        return xpsStatus2SaiStatus(xpRetVal);
    }

    sai_uint64_t rateBytes = schedInfo->attrInfo.maxBandwidthRate.u64;
    xpsDevice_t  xpDevId   = xpSaiObjIdSwitchGet(scheduler_id);

    for (uint32_t count = 0; count < schedInfo->numNodes; count++)
    {
        nodeKey = schedInfo->nodeList[count].nodeIndex;

        switch (schedInfo->nodeList[count].nodeType)
        {
            case XPSAI_SCHEDULER_QUEUE_NODE:
                {
                    /* Get the port and queue from queue node index */
                    xpSaiQueueGetPortAndQueueFromKey(nodeKey, &xpPort, &queue);

                    retVal = xpSaiSetSchedulerQueueShaperMaxRate(xpDevId, xpPort, queue, rateBytes,
                                                                 value.s32);
                    if (SAI_STATUS_SUCCESS != retVal)
                    {
                        XP_SAI_LOG_ERR("Error: Could not set queue shaper burst size | retVal:%d",
                                       retVal);
                        return retVal;
                    }
                    break;
                }

            case XPSAI_SCHEDULER_LEVEL2_NODE:
            case XPSAI_SCHEDULER_LEVEL1_NODE:
            case XPSAI_SCHEDULER_LEVEL0_NODE:
                /* do nothing */
                /* HQoS is not supported */
                break;

            case XPSAI_SCHEDULER_PORT_NODE:
                {
                    retVal = xpSaiSetSchedulerPortShaperRate(xpDevId, nodeKey, rateBytes,
                                                             value.s32);
                    if (SAI_STATUS_SUCCESS != retVal)
                    {
                        XP_SAI_LOG_ERR("Error: Could not set port shaper burst size | retVal:%d",
                                       retVal);
                        return retVal;
                    }
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Error: Invalid scheduler node type %d\n",
                                   schedInfo->nodeList[count].nodeType);
                    return SAI_STATUS_INVALID_PARAMETER;
                }
        }
    }

    /* Update the scheduler stat database */
    schedInfo->attrInfo.maxBandwidthBurstRate.u64 = value.u64;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetSchedulerQueueShaperMinRate

sai_status_t xpSaiSetSchedulerQueueShaperMinRate(uint32_t xpDevId,
                                                 uint32_t xpPort, uint32_t queue, uint64_t bandwidthRate,
                                                 uint64_t bandwidthBurstRate)
{
    GT_BOOL   enable = ((bandwidthRate > 0) &&
                        (bandwidthBurstRate > 0)) ? GT_TRUE : GT_FALSE;
    GT_STATUS rc = GT_OK;

    /* No limit if bandwidthRate is set to 0. In this case disable the queue slow shaper */
    if ((rc = cpssHalPortTxQMinRateEnableSet(xpDevId, xpPort, queue,
                                             enable)) != GT_OK)
    {
        XP_SAI_LOG_ERR("Error: Could not set queue min shaper enable | retVal: %d\n",
                       rc);
        return cpssStatus2SaiStatus(rc);
    }

    /* if shaper is disabled. no need to configure */
    if (enable == GT_FALSE)
    {
        return SAI_STATUS_SUCCESS;
    }

    /* Convert Bytes to Kbps */
    sai_uint32_t rateKbps = ((bandwidthRate * XP_BITS_IN_BYTE) /
                             XP_KBPS_TO_BITS_FACTOR);
    /* Convert burstSize bytes to 4K bytes */
    sai_uint32_t burstSize = bandwidthBurstRate / 4096;

    if ((rc = cpssHalPortTxQMinRateSet(xpDevId, xpPort, queue, burstSize,
                                       &rateKbps)) != GT_OK)
    {
        XP_SAI_LOG_ERR("Error: Could not configure queue min shaper | retVal: %d\n",
                       rc);
        return cpssStatus2SaiStatus(rc);
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetSchedulerAttrMinBandwidthRate

sai_status_t xpSaiSetSchedulerAttrMinBandwidthRate(sai_object_id_t scheduler_id,
                                                   sai_attribute_value_t value)
{
    xpSaiSchedulerProfile_t *schedInfo = NULL;
    sai_uint32_t             nodeKey = 0;
    sai_uint32_t             xpPort = 0;
    sai_uint32_t             queue = 0;
    XP_STATUS                xpRetVal;
    sai_status_t retVal;

    /* Get the scheduler state data */
    if ((xpRetVal = xpSaiGetSchedulerStateData(scheduler_id,
                                               &schedInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Could not retrieve scheduler state info | retVal:%d\n",
                       xpRetVal);
        return xpsStatus2SaiStatus(xpRetVal);
    }

    sai_uint64_t burstSize = schedInfo->attrInfo.minBandwidthBurstRate.u64;
    xpsDevice_t  xpDevId   = xpSaiObjIdSwitchGet(scheduler_id);

    for (uint32_t count = 0; count < schedInfo->numNodes; count++)
    {
        nodeKey = schedInfo->nodeList[count].nodeIndex;

        switch (schedInfo->nodeList[count].nodeType)
        {
            case XPSAI_SCHEDULER_QUEUE_NODE:
                {
                    /* Get the port and queue from queue node index */
                    xpSaiQueueGetPortAndQueueFromKey(nodeKey, &xpPort, &queue);

                    retVal = xpSaiSetSchedulerQueueShaperMinRate(xpDevId, xpPort, queue, value.u64,
                                                                 burstSize);
                    if (SAI_STATUS_SUCCESS != retVal)
                    {
                        XP_SAI_LOG_ERR("Error: Could not set queue shaper rate | retVal:%d", retVal);
                        return retVal;
                    }
                    break;
                }

            case XPSAI_SCHEDULER_LEVEL2_NODE:
            case XPSAI_SCHEDULER_LEVEL1_NODE:
            case XPSAI_SCHEDULER_LEVEL0_NODE:
                /* do nothing */
                /* HQoS is not supported */
                break;

            case XPSAI_SCHEDULER_PORT_NODE:
                {
                    if (burstSize != 0 || value.u64 != 0)
                    {
                        XP_SAI_LOG_ERR("Error: Min Shaper at port is not supported\n");
                        return SAI_STATUS_NOT_SUPPORTED;
                    }
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Error: Invalid scheduler node type %d\n",
                                   schedInfo->nodeList[count].nodeType);
                    return SAI_STATUS_INVALID_PARAMETER;
                }
        }
    }

    /* Update the scheduler stat database */
    schedInfo->attrInfo.minBandwidthRate.u64 = value.u64;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetSchedulerAttrMinBandwidthBurstRate

sai_status_t xpSaiSetSchedulerAttrMinBandwidthBurstRate(
    sai_object_id_t scheduler_id, sai_attribute_value_t value)
{
    xpSaiSchedulerProfile_t *schedInfo = NULL;
    sai_uint32_t             nodeKey   = 0;
    sai_uint32_t             xpPort    = 0;
    sai_uint32_t             queue     = 0;
    XP_STATUS                xpRetVal  = XP_NO_ERR;
    sai_status_t retVal;

    /* Get the scheduler state data */
    if ((xpRetVal = xpSaiGetSchedulerStateData(scheduler_id,
                                               &schedInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Could not retrieve scheduler state info | retVal:%d\n",
                       xpRetVal);
        return xpsStatus2SaiStatus(xpRetVal);
    }

    sai_uint64_t rateBytes = schedInfo->attrInfo.minBandwidthRate.u64;
    xpsDevice_t  xpDevId   = xpSaiObjIdSwitchGet(scheduler_id);

    for (uint32_t count = 0; count < schedInfo->numNodes; count++)
    {
        nodeKey = schedInfo->nodeList[count].nodeIndex;

        switch (schedInfo->nodeList[count].nodeType)
        {
            case XPSAI_SCHEDULER_QUEUE_NODE:
                {
                    /* Get the port and queue from queue node index */
                    xpSaiQueueGetPortAndQueueFromKey(nodeKey, &xpPort, &queue);

                    retVal = xpSaiSetSchedulerQueueShaperMinRate(xpDevId, xpPort, queue, rateBytes,
                                                                 value.s32);
                    if (SAI_STATUS_SUCCESS != retVal)
                    {
                        XP_SAI_LOG_ERR("Error: Could not set queue shaper burst size | retVal:%d",
                                       retVal);
                        return retVal;
                    }
                    break;
                }

            case XPSAI_SCHEDULER_LEVEL2_NODE:
            case XPSAI_SCHEDULER_LEVEL1_NODE:
            case XPSAI_SCHEDULER_LEVEL0_NODE:
                /* do nothing */
                /* HQoS is not supported */
                break;

            case XPSAI_SCHEDULER_PORT_NODE:
                {
                    if (rateBytes != 0 || value.u32 != 0)
                    {
                        XP_SAI_LOG_ERR("Error: Min Shaper at port is not supported\n");
                        return SAI_STATUS_NOT_SUPPORTED;
                    }
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Error: Invalid scheduler node type %d\n",
                                   schedInfo->nodeList[count].nodeType);
                    return SAI_STATUS_INVALID_PARAMETER;
                }
        }
    }

    /* Update the scheduler stat database */
    schedInfo->attrInfo.minBandwidthBurstRate.u64 = value.u64;

    return SAI_STATUS_SUCCESS;
}
//Func: xpSaiSetSchedulerAttribute

sai_status_t xpSaiSetSchedulerAttribute(sai_object_id_t scheduler_id,
                                        const sai_attribute_t *attr)
{
    sai_status_t retVal;

    retVal = xpSaiAttrCheck(1, attr,
                            SCHEDULER_VALIDATION_ARRAY_SIZE, scheduler_attribs,
                            SAI_COMMON_API_SET);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", retVal);
        return retVal;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(scheduler_id, SAI_OBJECT_TYPE_SCHEDULER))
    {
        XP_SAI_LOG_ERR("Wrong object type received (%u)\n",
                       xpSaiObjIdTypeGet(scheduler_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    switch (attr->id)
    {
        case SAI_SCHEDULER_ATTR_SCHEDULING_TYPE:
            {
                retVal = xpSaiSetSchedulerAttrSchedulingType(scheduler_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_SCHEDULER_ATTR_SCHEDULING_TYPE)\n");
                    return retVal;
                }
                break;
            }
        case SAI_SCHEDULER_ATTR_SCHEDULING_WEIGHT:
            {
                retVal = xpSaiSetSchedulerAttrSchedulingWeight(scheduler_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_SCHEDULER_ATTR_SCHEDULING_WEIGHT)\n");
                    return retVal;
                }
                break;
            }
        case SAI_SCHEDULER_ATTR_METER_TYPE:
            {
                retVal = xpSaiSetSchedulerAttrMeterType(scheduler_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_SCHEDULER_ATTR_METER_TYPE)\n");
                    return retVal;
                }
                break;
            }
        case SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_RATE:
            {
                retVal = xpSaiSetSchedulerAttrMaxBandwidthRate(scheduler_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_RATE)\n");
                    return retVal;
                }
                break;
            }
        case SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_BURST_RATE:
            {
                retVal = xpSaiSetSchedulerAttrMaxBandwidthBurstRate(scheduler_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_BURST_RATE)\n");
                    return retVal;
                }
                break;
            }
        case SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_RATE:
            {
                retVal = xpSaiSetSchedulerAttrMinBandwidthRate(scheduler_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_RATE)\n");
                    return retVal;
                }
                break;
            }
        case SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_BURST_RATE:
            {
                retVal = xpSaiSetSchedulerAttrMinBandwidthBurstRate(scheduler_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_BURST_RATE)\n");
                    return retVal;
                }
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Unknown attribute %d\n", attr->id);
                return SAI_STATUS_INVALID_PARAMETER;
            }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetSchedulerAttribute

static sai_status_t xpSaiGetSchedulerAttribute(const xpSaiSchedulerAttributesT*
                                               attributes, sai_attribute_t *attr, uint32_t attr_index)
{
    switch (attr->id)
    {
        case SAI_SCHEDULER_ATTR_SCHEDULING_TYPE:
            {
                attr->value = attributes->schedulingType;
                break;
            }
        case SAI_SCHEDULER_ATTR_SCHEDULING_WEIGHT:
            {
                attr->value = attributes->schedulingWeight;
                break;
            }
        case SAI_SCHEDULER_ATTR_METER_TYPE:
            {
                attr->value = attributes->meterType;
                break;
            }
        case SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_RATE:
            {
                attr->value = attributes->minBandwidthRate;
                break;
            }
        case SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_BURST_RATE:
            {
                attr->value = attributes->minBandwidthBurstRate;
                break;
            }
        case SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_RATE:
            {
                attr->value = attributes->maxBandwidthRate;
                break;
            }
        case SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_BURST_RATE:
            {
                attr->value = attributes->maxBandwidthBurstRate;
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Attribute %d is unknown\n", attr->id);
                return SAI_STATUS_UNKNOWN_ATTRIBUTE_0 + SAI_STATUS_CODE(attr_index);
            }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetSchedulerAttributes

static sai_status_t xpSaiGetSchedulerAttributes(sai_object_id_t scheduler_id,
                                                uint32_t attr_count, sai_attribute_t *attr_list)
{
    xpSaiSchedulerProfile_t *schedInfo = NULL;
    XP_STATUS                xpRetVal;
    sai_status_t             retVal;

    retVal = xpSaiAttrCheck(attr_count, attr_list,
                            SCHEDULER_VALIDATION_ARRAY_SIZE, scheduler_attribs,
                            SAI_COMMON_API_GET);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", retVal);
        return retVal;
    }

    /* Validate scheduler object id */
    if (!XDK_SAI_OBJID_TYPE_CHECK(scheduler_id, SAI_OBJECT_TYPE_SCHEDULER))
    {
        XP_SAI_LOG_ERR("Wrong object type received (%u)\n",
                       xpSaiObjIdTypeGet(scheduler_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    /* Get the scheduler state data */
    if ((xpRetVal = xpSaiGetSchedulerStateData(scheduler_id,
                                               &schedInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Could not retrieve scheduler state info | retVal:%d\n",
                       xpRetVal);
        return xpsStatus2SaiStatus(xpRetVal);
    }

    xpSaiUpdateAttrListSchedulerVals(&schedInfo->attrInfo, attr_count, attr_list);

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiBulkGetSchedulerAttributes

sai_status_t xpSaiBulkGetSchedulerAttributes(sai_object_id_t id,
                                             uint32_t *attr_count, sai_attribute_t *attr_list)
{
    sai_status_t            saiRetVal  = SAI_STATUS_SUCCESS;
    XP_STATUS               xpRetVal   = XP_NO_ERR;
    uint32_t                idx        = 0;
    uint32_t                maxcount   = 0;
    xpSaiSchedulerProfile_t *schedInfo = NULL;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    /* Check incoming parameters */
    if (!XDK_SAI_OBJID_TYPE_CHECK(id, SAI_OBJECT_TYPE_SCHEDULER))
    {
        XP_SAI_LOG_ERR("Wrong object type received (%u)\n", xpSaiObjIdTypeGet(id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if ((attr_count == NULL) || (attr_list == NULL))
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiMaxCountSchedulerAttribute(&maxcount);
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

    /* Get the scheduler state data */
    xpRetVal = xpSaiGetSchedulerStateData(id, &schedInfo);
    if (xpRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Could not retrieve scheduler state info | xpRetVal:%d\n",
                       xpRetVal);
        return xpsStatus2SaiStatus(xpRetVal);
    }

    for (uint32_t count = 0; count < maxcount; count++)
    {
        attr_list[idx].id = SAI_SCHEDULER_ATTR_START + count;
        saiRetVal = xpSaiGetSchedulerAttribute(&schedInfo->attrInfo, &attr_list[idx],
                                               count);

        if (saiRetVal == SAI_STATUS_SUCCESS)
        {
            idx++;
        }
    }
    *attr_count = idx;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSchedulerBindNodeIdxToScheduler

XP_STATUS xpSaiSchedulerBindNodeIdxToScheduler(sai_object_id_t schedulerObjId,
                                               xpSaiSchedulerNodeType_e nodeType, uint32_t nodeIdx)
{
    xpSaiSchedulerProfile_t *pNewSchedulerInfo = NULL;
    xpSaiSchedulerProfile_t *pSchedulerInfo    = NULL;
    XP_STATUS                xpStatus          = XP_NO_ERR;

    /* Get the scheduler state data */
    xpStatus = xpSaiGetSchedulerStateData(schedulerObjId, &pSchedulerInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Could not get scheduler state data\n");
        return xpStatus;
    }

    if (xpsDAIsCtxGrowthNeeded(pSchedulerInfo->numNodes,
                               XPSAI_SCHEDULER_MAP_LIST_SIZE))
    {
        /* Double the queueList array size */
        if ((xpStatus = xpSaiDynamicArrayGrow((void**)&pNewSchedulerInfo,
                                              (void*)pSchedulerInfo, sizeof(xpSaiSchedulerProfile_t),
                                              sizeof(xpSaiSchedulerMapList_t), pSchedulerInfo->numNodes,
                                              XPSAI_SCHEDULER_MAP_LIST_SIZE)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Failed to grow dynamic array size, xpStatus: %d\n",
                           xpStatus);
            return xpStatus;
        }

        /* Remove the corresponding state from database and add new state */
        xpStatus = xpSaiRemoveSchedulerStateData(schedulerObjId);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Could not remove scheduler state data, xpStatus: %d\n",
                           xpStatus);
            xpsStateHeapFree((void*)pNewSchedulerInfo);
            return xpStatus;
        }

        /* Insert the new state */
        xpStatus = xpsStateInsertData(XP_SCOPE_DEFAULT, schedulerDbHandle,
                                      (void*)pNewSchedulerInfo);
        if (xpStatus != XP_NO_ERR)
        {
            xpsStateHeapFree((void*)pNewSchedulerInfo);
            return xpStatus;
        }

        pSchedulerInfo = pNewSchedulerInfo;
    }

    pSchedulerInfo->nodeList[pSchedulerInfo->numNodes].nodeIndex = nodeIdx;
    pSchedulerInfo->nodeList[pSchedulerInfo->numNodes].nodeType = nodeType;
    pSchedulerInfo->numNodes++;

    return XP_NO_ERR;
}

//Func: xpSaiSchedulerUnbindNodeIdxFromScheduler

XP_STATUS xpSaiSchedulerUnbindNodeIdxFromScheduler(sai_object_id_t
                                                   schedulerObjId, xpSaiSchedulerNodeType_e nodeType, uint32_t nodeIdx)
{
    xpSaiSchedulerProfile_t *pNewSchedulerInfo = NULL;
    xpSaiSchedulerProfile_t *pSchedulerInfo    = NULL;
    sai_uint8_t              nodePresent       = 0;
    XP_STATUS                xpStatus          = XP_NO_ERR;

    /* Get the scheduler state data */
    xpStatus = xpSaiGetSchedulerStateData(schedulerObjId, &pSchedulerInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Could not get scheduler state data, xpStatus: %d\n",
                       xpStatus);
        return xpStatus;
    }

    /* Remove the node from scheduler node list */
    for (uint16_t count = 0; count < pSchedulerInfo->numNodes; count++)
    {
        if (!nodePresent)
        {
            if ((pSchedulerInfo->nodeList[count].nodeType == nodeType) &&
                (pSchedulerInfo->nodeList[count].nodeIndex == nodeIdx))
            {
                nodePresent = 1;
            }
            continue;
        }
        else
        {
            pSchedulerInfo->nodeList[count-1] = pSchedulerInfo->nodeList[count];
        }
    }

    if (!nodePresent)
    {
        XP_SAI_LOG_ERR("Error: Could not remove node from scheduler node list\n");
        return XP_ERR_INVALID_ARG;
    }

    /* Decrement the scheduler node list count */
    pSchedulerInfo->numNodes--;

    if (xpsDAIsCtxShrinkNeeded(pSchedulerInfo->numNodes,
                               XPSAI_SCHEDULER_MAP_LIST_SIZE))
    {
        /* Shrink the queueList array size */
        if ((xpStatus = xpSaiDynamicArrayShrink((void**)&pNewSchedulerInfo,
                                                (void*)pSchedulerInfo, sizeof(xpSaiSchedulerProfile_t),
                                                sizeof(xpSaiSchedulerMapList_t), pSchedulerInfo->numNodes,
                                                XPSAI_SCHEDULER_MAP_LIST_SIZE)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Failed to shrinnk dynamic array size, xpStatus: %d\n",
                           xpStatus);
            return xpStatus;
        }

        /* Remove the corresponding state from database and add new state */
        xpStatus = xpSaiRemoveSchedulerStateData(schedulerObjId);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Could not remove scheduler state data, xpStatus: %d\n",
                           xpStatus);
            xpsStateHeapFree((void*)pNewSchedulerInfo);
            return xpStatus;
        }

        /* Insert the new state */
        xpStatus = xpsStateInsertData(XP_SCOPE_DEFAULT, schedulerDbHandle,
                                      (void*)pNewSchedulerInfo);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Could not insert new scheduler state data, xpStatus: %d\n",
                           xpStatus);
            xpsStateHeapFree((void*)pNewSchedulerInfo);
            return xpStatus;
        }
    }

    return XP_NO_ERR;
}

//Func: xpSaiSchedulerBindPortToScheduler

XP_STATUS xpSaiSchedulerBindPortToScheduler(sai_object_id_t schedObjId,
                                            sai_uint32_t xpPort)
{
    XP_STATUS xpStatus = XP_NO_ERR;

    /* Bind port node to scheduler */
    xpStatus = xpSaiSchedulerBindNodeIdxToScheduler(schedObjId,
                                                    XPSAI_SCHEDULER_PORT_NODE, xpPort);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to bind port node to scheduler, xpStatus: %d\n",
                       xpStatus);
        return xpStatus;
    }

    return XP_NO_ERR;
}

//Func: xpSaiSchedulerUnbindPortFromScheduler

XP_STATUS xpSaiSchedulerUnbindPortFromScheduler(sai_object_id_t schedObjId,
                                                sai_uint32_t xpPort)
{
    XP_STATUS xpStatus = XP_NO_ERR;

    /* Unbind port node to scheduler */
    xpStatus = xpSaiSchedulerUnbindNodeIdxFromScheduler(schedObjId,
                                                        XPSAI_SCHEDULER_PORT_NODE, xpPort);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to unbind port node from scheduler, xpStatus: %d\n",
                       xpStatus);
        return xpStatus;
    }

    return XP_NO_ERR;
}

//Func: xpSaiSchedulerBindQueueToScheduler

XP_STATUS xpSaiSchedulerBindQueueToScheduler(sai_object_id_t schedObjId,
                                             sai_object_id_t queueObjId)
{
    sai_uint32_t queueKey = 0;
    sai_uint32_t queue    = 0;
    sai_uint32_t port     = 0;
    XP_STATUS    xpStatus = XP_NO_ERR;

    /* Get the port and queue number from queue object id */
    xpSaiQosQueueObjIdPortValueGet(queueObjId, &port, &queue);

    /* Get the queue key from port and queue */
    xpSaiQueueGetKeyFromPortAndQueue(port, queue, &queueKey);

    /* Bind queue node to scheduler */
    xpStatus = xpSaiSchedulerBindNodeIdxToScheduler(schedObjId,
                                                    XPSAI_SCHEDULER_QUEUE_NODE, queueKey);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to bind queue node to scheduler, xpStatus: %d\n",
                       xpStatus);
        return xpStatus;
    }

    return XP_NO_ERR;
}

//Func: xpSaiSchedulerUnbindQueueFromScheduler

XP_STATUS xpSaiSchedulerUnbindQueueFromScheduler(sai_object_id_t schedObjId,
                                                 sai_object_id_t queueObjId)
{
    sai_uint32_t queueKey = 0;
    sai_uint32_t queue    = 0;
    sai_uint32_t port     = 0;
    XP_STATUS    xpStatus = XP_NO_ERR;

    /* Get the port and queue number from queue object id */
    xpSaiQosQueueObjIdPortValueGet(queueObjId, &port, &queue);

    /* Get the queue key from port and queue */
    xpSaiQueueGetKeyFromPortAndQueue(port, queue, &queueKey);

    /* Unbind port node to scheduler */
    xpStatus = xpSaiSchedulerUnbindNodeIdxFromScheduler(schedObjId,
                                                        XPSAI_SCHEDULER_QUEUE_NODE, queueKey);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to unbind queue node from scheduler, xpStatus: %d\n",
                       xpStatus);
        return xpStatus;
    }

    return XP_NO_ERR;
}

//Func: xpSaiSchedulerBindSchedulerGrpToScheduler

XP_STATUS xpSaiSchedulerBindSchedulerGrpToScheduler(sai_object_id_t schedObjId,
                                                    sai_object_id_t schedGrpObjId)
{
    xpSaiSchedulerNodeType_e nodeType;
    sai_uint32_t nodeKey   = xpSaiObjIdValueGet(schedGrpObjId);
    sai_uint32_t nodeLevel = 0;
    XP_STATUS    xpStatus  = XP_NO_ERR;

    /* Get the scheduler group node level from key */
    xpSaiSchedulerGroupGetLevelAndNodeIdxFromKey(nodeKey, &nodeLevel, NULL);

    switch (nodeLevel)
    {
        case 0:
            nodeType = XPSAI_SCHEDULER_LEVEL0_NODE;
            break;
        case 1:
            nodeType = XPSAI_SCHEDULER_LEVEL1_NODE;
            break;
        case 2:
            nodeType = XPSAI_SCHEDULER_LEVEL2_NODE;
            break;
        default:
            XP_SAI_LOG_ERR("Error: Ivalid scheduler group node level %d\n", nodeLevel);
            return XP_ERR_INVALID_VALUE;
    }

    /* Bind the scheduler group node to scheduler */
    xpStatus = xpSaiSchedulerBindNodeIdxToScheduler(schedObjId, nodeType, nodeKey);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to bind the scheduler group node to scheduler, xpStatus: %d\n",
                       xpStatus);
        return xpStatus;
    }

    return XP_NO_ERR;
}

//Func: xpSaiSchedulerUnbindSchedulerGrpFromScheduler

XP_STATUS xpSaiSchedulerUnbindSchedulerGrpFromScheduler(
    sai_object_id_t schedObjId, sai_object_id_t schedGrpObjId)
{
    xpSaiSchedulerNodeType_e nodeType;
    sai_uint32_t nodeKey   = xpSaiObjIdValueGet(schedGrpObjId);
    sai_uint32_t nodeLevel = 0;
    XP_STATUS    xpStatus  = XP_NO_ERR;

    /* Get the scheduler group node level from key */
    xpSaiSchedulerGroupGetLevelAndNodeIdxFromKey(nodeKey, &nodeLevel, NULL);

    switch (nodeLevel)
    {
        case 0:
            nodeType = XPSAI_SCHEDULER_LEVEL0_NODE;
            break;
        case 1:
            nodeType = XPSAI_SCHEDULER_LEVEL1_NODE;
            break;
        case 2:
            nodeType = XPSAI_SCHEDULER_LEVEL2_NODE;
            break;
        default:
            XP_SAI_LOG_ERR("Error: Ivalid scheduler group node level %d\n", nodeLevel);
            return XP_ERR_INVALID_VALUE;
    }

    /* Unbind the scheduler group node from scheduler */
    xpStatus = xpSaiSchedulerUnbindNodeIdxFromScheduler(schedObjId, nodeType,
                                                        nodeKey);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to unbind scheduler group node from scheduler, xpStatus: %d\n",
                       xpStatus);
        return xpStatus;
    }

    return XP_NO_ERR;
}

//Func: xpSaiSchedulerInit

XP_STATUS xpSaiSchedulerInit(xpsDevice_t xpSaiDevId)
{
    XP_STATUS xpsRetVal = XP_NO_ERR;

    xpsRetVal = xpsAllocatorInitIdAllocator(XP_SCOPE_DEFAULT, XP_SAI_ALLOC_SCHD_PFL,
                                            XP_SAI_SCHD_PFL_MAX_IDS, XP_SAI_SCHD_PFL_RANGE_START);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to initialize SAI Scheduler Profile ID allocator\n");
        return xpsRetVal;
    }

    // Create a Global Sai Scheduler Profile Db
    xpsRetVal = xpsStateRegisterDb(XP_SCOPE_DEFAULT, "Sai Scheduler Profile Db",
                                   XPS_GLOBAL, &xpSaiSchedulerKeyComp, schedulerDbHandle);
    if (xpsRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to Register Sai Scheduler Profile Db\n");
        return xpsRetVal;
    }

    return XP_NO_ERR;
}

//Func: xpSaiSchedulerDeInit

XP_STATUS xpSaiSchedulerDeInit(xpsDevice_t xpSaiDevId)
{
    XP_STATUS xpsRetVal = XP_NO_ERR;

    // Remove Global Sai Scheduler Profile Db
    if ((xpsRetVal = xpsStateDeRegisterDb(XP_SCOPE_DEFAULT,
                                          &schedulerDbHandle)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("De-Register Sai Scheduler Profile Db handler failed\n");
        return xpsRetVal;
    }

    return XP_NO_ERR;
}

//Func: xpSaiSchedulerApiInit

XP_STATUS xpSaiSchedulerApiInit(uint64_t flag,
                                const sai_service_method_table_t* adapHostServiceMethodTable)
{
    XP_STATUS retVal = XP_NO_ERR;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Calling xpSaiSchedulerApiInit\n");

    _xpSaiSchedulerApi = (sai_scheduler_api_t *) xpMalloc(sizeof(
                                                              sai_scheduler_api_t));

    if (NULL == _xpSaiSchedulerApi)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for _xpSaiSchedulerApi\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }

    _xpSaiSchedulerApi->create_scheduler = xpSaiCreateScheduler;
    _xpSaiSchedulerApi->remove_scheduler = xpSaiRemoveScheduler;
    _xpSaiSchedulerApi->set_scheduler_attribute  = xpSaiSetSchedulerAttribute;
    _xpSaiSchedulerApi->get_scheduler_attribute  = xpSaiGetSchedulerAttributes;

    saiRetVal = xpSaiApiRegister(SAI_API_SCHEDULER, (void*)_xpSaiSchedulerApi);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Failed to register scheduler API\n");
        return XP_ERR_ARRAY_OUT_OF_BOUNDS;
    }

    return retVal;
}

//Func: xpSaiSchedulerApiDeinit

XP_STATUS xpSaiSchedulerApiDeinit()
{
    XP_STATUS retVal = XP_NO_ERR;

    XP_SAI_LOG_DBG("Calling xpSaiSchedulerApiDeinit\n");

    xpFree(_xpSaiSchedulerApi);
    _xpSaiSchedulerApi = NULL;

    return retVal;
}

sai_status_t xpSaiMaxCountSchedulerAttribute(uint32_t *count)
{
    if (!count)
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }
    *count = SAI_SCHEDULER_ATTR_END;

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiCountSchedulerObjects(uint32_t *count)
{
    XP_STATUS  retVal  = XP_NO_ERR;

    if (!count)
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpsStateGetCount(XP_SCOPE_DEFAULT, schedulerDbHandle, count);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get Count. return Value : %d\n", retVal);
    }
    return xpsStatus2SaiStatus(retVal);
}

sai_status_t xpSaiGetSchedulerObjectList(uint32_t *object_count,
                                         sai_object_key_t *object_list)
{
    XP_STATUS       retVal      = XP_NO_ERR;
    sai_status_t    saiRetVal   = SAI_STATUS_SUCCESS;
    uint32_t        objCount    = 0;

    xpSaiSchedulerProfile_t   *pSchedulerNext = NULL;

    saiRetVal = xpSaiCountSchedulerObjects(&objCount);
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
        retVal = xpsStateGetNextData(XP_SCOPE_DEFAULT, schedulerDbHandle,
                                     pSchedulerNext, (void **)&pSchedulerNext);
        if (retVal != XP_NO_ERR || pSchedulerNext == NULL)
        {
            XP_SAI_LOG_ERR("Failed to retrieve queue object, error %d\n", retVal);
            return SAI_STATUS_FAILURE;
        }

        object_list[i].key.object_id = pSchedulerNext->schedObjId;
    }

    return SAI_STATUS_SUCCESS;
}

