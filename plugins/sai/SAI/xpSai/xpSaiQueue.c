/* xpSaiQueue.c */
/************************************************************************/
/*  Copyright (c) Marvell [2000-2020] All rights reserved.              */
/*  Unpublished - rights reserved under the Copyright Laws of the       */
/*  United States.  Use, duplication, or disclosure by the              */
/*  Government is subject to restrictions as set forth in               */
/*  subparagraph (c)(1)(ii) of the Rights in Technical Data and         */
/*  Computer Software clause at 252.227-7013.                           */
/************************************************************************/
#include "xpSaiQueue.h"
#include "xpSaiValidationArrays.h"
#include "xpSaiStub.h"
#include "cpssDxChPortTx.h"
#include "xpSaiProfileMng.h"
#include "cpssHalQos.h"
#include "cpssHalUtil.h"
#include "cpssHalCounter.h"
#include "xpSaiHostInterface.h"

XP_SAI_LOG_REGISTER_API(SAI_API_QUEUE);

sai_queue_api_t* _xpSaiQueueApi;

xpsDbHandle_t queueQosDbHandle = XPSAI_QUEUE_QOS_DB_HNDL;

xpSaiPortProfile_t *schedulerProfileList[XP_MAX_DEVICES];

static sai_status_t xpSaiQueueIsWredConfigured(sai_object_id_t queueObjId,
                                               bool *wredConfiguredPtr);
static sai_status_t xpSaiQueueIsBufferConfigured(sai_object_id_t queueObjId,
                                                 bool *bufferConfiguredPtr,
                                                 sai_uint32_t *sizePtrInPages);

static sai_int32_t xpSaiQueueKeyComp(void *key1, void *key2)
{
    sai_int64_t val1 = ((xpSaiQueueQosInfo_t*)key1)->queueId;
    sai_int64_t val2 = ((xpSaiQueueQosInfo_t*)key2)->queueId;

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

XP_STATUS xpSaiQueueStateCreate(sai_object_id_t queueId,
                                xpSaiQueueQosInfo_t **ppQueueQosInfo)
{
    XP_STATUS retVal = XP_NO_ERR;

    /* Create a new sai queue qos db */
    if ((retVal = xpsStateHeapMalloc(sizeof(xpSaiQueueQosInfo_t),
                                     (void**)ppQueueQosInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Couldn't allocate heap memory | retVal: %d\n", retVal);
        return retVal;
    }
    memset(*ppQueueQosInfo, 0x0, sizeof(xpSaiQueueQosInfo_t));

    (*ppQueueQosInfo)->queueId = queueId;

    /* Insert the state into the database */
    if ((retVal = xpsStateInsertData(XP_SCOPE_DEFAULT, queueQosDbHandle,
                                     (void*)*ppQueueQosInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Couldn't insert state into data base | retVal: %d\n",
                       retVal);
        xpsStateHeapFree((void*)*ppQueueQosInfo);
        return retVal;
    }

    return XP_NO_ERR;
}

XP_STATUS xpSaiQueueStateRemove(sai_object_id_t queueId)
{
    xpSaiQueueQosInfo_t *pQueueInfo = NULL;
    xpSaiQueueQosInfo_t  queueInfoKey;
    XP_STATUS            retVal = XP_NO_ERR;

    queueInfoKey.queueId = queueId;
    /* Delete the corresponding state */
    if ((retVal = xpsStateDeleteData(XP_SCOPE_DEFAULT, queueQosDbHandle,
                                     (xpsDbKey_t)&queueInfoKey, (void**)&pQueueInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Couldn't not delete queue state data | retVal: %d\n",
                       retVal);
        return retVal;
    }

    /* Free the memory allocated for the corresponding state */
    if ((retVal = xpsStateHeapFree(pQueueInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Couldn't free heap memory | retVal: %d\n", retVal);
        return retVal;
    }

    return XP_NO_ERR;
}

//Func: xpSaiGetQueueQosInfo

XP_STATUS xpSaiGetQueueQosInfo(sai_object_id_t queueObjId,
                               xpSaiQueueQosInfo_t **ppQueueQosInfo)
{
    xpSaiQueueQosInfo_t queueQosInfoKey;
    XP_STATUS retVal = XP_NO_ERR;

    queueQosInfoKey.queueId = queueObjId;
    /* Retrieve the corresponding state from sai queue qos db */
    if ((retVal = xpsStateSearchData(XP_SCOPE_DEFAULT, queueQosDbHandle,
                                     (xpsDbKey_t)&queueQosInfoKey, (void**)ppQueueQosInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to retrieve the sai queue qos data |retVal:%d\n",
                       retVal);
        return retVal;
    }

    if (*ppQueueQosInfo == NULL)
    {
        XP_SAI_LOG_ERR("Error: Entry not found |retVal: %d\n", retVal);
        return XP_ERR_KEY_NOT_FOUND;
    }

    return XP_NO_ERR;
}

//Func: xpSaiGetQueueDeviceId

XP_STATUS xpSaiGetQueueDeviceId(sai_object_id_t queueObjId,
                                xpsDevice_t *xpDevId)
{
    xpSaiQueueQosInfo_t *pQueueQosInfo = NULL;
    XP_STATUS            retVal = XP_NO_ERR;

    /* Get the queue qos information */
    if ((retVal = xpSaiGetQueueQosInfo(queueObjId, &pQueueQosInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Couldn't get sai queue state info |retVal: %d\n",
                       retVal);
        return retVal;
    }

    /* Get the device id from switch object id */
    *xpDevId = (xpsDevice_t)xpSaiObjIdSwitchGet(pQueueQosInfo->switchId);

    return XP_NO_ERR;
}

//Func: xpSaiQueueValidateAttributeValue

static sai_status_t xpSaiQueueValidateAttributeValue(xpsDevice_t xpDevId,
                                                     sai_uint32_t portNum, sai_attr_id_t attrId, sai_attribute_value_t attrValue,
                                                     sai_uint32_t count)
{
    switch (attrId)
    {
        case SAI_QUEUE_ATTR_TYPE:
            {
                /* Validate queue type */
                if ((attrValue.s32 < SAI_QUEUE_TYPE_ALL) &&
                    (attrValue.s32 > SAI_QUEUE_TYPE_MULTICAST))
                {
                    XP_SAI_LOG_ERR("Error: Invalid queue type %d provided\n", attrValue.s32);
                    return SAI_STATUS_INVALID_ATTR_VALUE_0 + SAI_STATUS_CODE(count);
                }
                break;
            }
        case SAI_QUEUE_ATTR_PORT:
            {
                /* Validate port object */
                if (!XDK_SAI_OBJID_TYPE_CHECK(attrValue.oid, SAI_OBJECT_TYPE_PORT))
                {
                    XP_SAI_LOG_ERR("Error: Invalid object type %u \n",
                                   xpSaiObjIdTypeGet(attrValue.oid));
                    return SAI_STATUS_INVALID_ATTR_VALUE_0 + SAI_STATUS_CODE(count);
                }
                break;
            }
        case SAI_QUEUE_ATTR_INDEX:
            {
                XP_STATUS xpStatus = XP_NO_ERR;
                uint32_t numOfQs = 0;

                /* Get the max queues per port */
                xpStatus = xpsQosAqmGetNumberOfQueuesPerPort(xpDevId, portNum, &numOfQs);
                if (xpStatus != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("Error: Failed to get max queues per port, xpStatus: %d\n",
                                   xpStatus);
                    return xpsStatus2SaiStatus(xpStatus);
                }

                /* Validate queue index */
                if (attrValue.u8 > numOfQs)
                {
                    XP_SAI_LOG_ERR("Error: Invalid queue index %d \n", attrValue.u8);
                    return SAI_STATUS_INVALID_ATTR_VALUE_0 + SAI_STATUS_CODE(count);
                }
                break;
            }
        case SAI_QUEUE_ATTR_PARENT_SCHEDULER_NODE:
            {
                /* Validate queue's parent scheduler node object */
                if ((!XDK_SAI_OBJID_TYPE_CHECK(attrValue.oid, SAI_OBJECT_TYPE_PORT)) &&
                    (!XDK_SAI_OBJID_TYPE_CHECK(attrValue.oid, SAI_OBJECT_TYPE_SCHEDULER_GROUP)))
                {
                    XP_SAI_LOG_ERR("Error: Invalid queue's parent scheduler node object %u \n",
                                   xpSaiObjIdTypeGet(attrValue.oid));
                    return SAI_STATUS_INVALID_ATTR_VALUE_0 + SAI_STATUS_CODE(count);
                }
                break;
            }
        case SAI_QUEUE_ATTR_WRED_PROFILE_ID:
            {
                /* Validate queue's wred object */
                if ((attrValue.oid != SAI_NULL_OBJECT_ID) &&
                    (!XDK_SAI_OBJID_TYPE_CHECK(attrValue.oid, SAI_OBJECT_TYPE_WRED)))
                {
                    XP_SAI_LOG_ERR("Wrong Invalid queue's wred object %u\n",
                                   xpSaiObjIdTypeGet(attrValue.oid));
                    return SAI_STATUS_INVALID_ATTR_VALUE_0 + SAI_STATUS_CODE(count);
                }
                break;
            }
        case SAI_QUEUE_ATTR_BUFFER_PROFILE_ID:
            {
                /* Validate queue's buffer profile object */
                if ((attrValue.oid != SAI_NULL_OBJECT_ID) &&
                    (!XDK_SAI_OBJID_TYPE_CHECK(attrValue.oid, SAI_OBJECT_TYPE_BUFFER_PROFILE)))
                {
                    XP_SAI_LOG_ERR("Wrong Invalid queue's buffer profile object %u\n",
                                   xpSaiObjIdTypeGet(attrValue.oid));
                    return SAI_STATUS_INVALID_ATTR_VALUE_0 + SAI_STATUS_CODE(count);
                }
                break;
            }
        case SAI_QUEUE_ATTR_SCHEDULER_PROFILE_ID:
            {
                /* Validate queue's scheduler profile object */
                if ((attrValue.oid != SAI_NULL_OBJECT_ID) &&
                    (!XDK_SAI_OBJID_TYPE_CHECK(attrValue.oid, SAI_OBJECT_TYPE_SCHEDULER)))
                {
                    XP_SAI_LOG_ERR("Wrong Invalid queue's scheduler profile object %u\n",
                                   xpSaiObjIdTypeGet(attrValue.oid));
                    return SAI_STATUS_INVALID_ATTR_VALUE_0 + SAI_STATUS_CODE(count);
                }
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Unknown attribute %d\n", attrId);
                return SAI_STATUS_UNKNOWN_ATTRIBUTE_0 + SAI_STATUS_CODE(count);
            }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiQueueGetPortAndQueueFromKey

XP_STATUS xpSaiQueueGetPortAndQueueFromKey(uint32_t key, uint32_t* port,
                                           uint32_t* queue)
{
    if (port)
    {
        *port = (sai_uint16_t)((key & SAI_QOS_Q_OBJID_PORT_MASK) >>
                               SAI_QOS_Q_OBJID_VALUE_BITS);;
    }

    if (queue)
    {
        *queue = (sai_uint16_t)(key & SAI_QOS_Q_OBJID_VALUE_MASK);
    }

    return XP_NO_ERR;
}

//Func: xpSaiQueueGetKeyFromPortAndQueue

XP_STATUS xpSaiQueueGetKeyFromPortAndQueue(uint32_t port, uint32_t queue,
                                           uint32_t* key)
{
    uint32_t keyVal = 0;

    keyVal = keyVal | (port << SAI_QOS_Q_OBJID_VALUE_BITS);
    keyVal = keyVal | queue;

    *key = keyVal;

    return XP_NO_ERR;
}

//Func: xpSaiQosQueueObjIdCreate

sai_status_t xpSaiQosQueueObjIdCreate(sai_object_type_t sai_object_type,
                                      sai_uint32_t xpsIntfId, sai_uint32_t localQId, sai_object_id_t *queue_id)
{
    sai_uint64_t    xpsQueueId  = SAI_NULL_OBJECT_ID;
    xpsDevice_t     devId       = xpSaiGetDevId();

    xpsQueueId = (sai_uint64_t) xpsIntfId << SAI_QOS_Q_OBJID_VALUE_BITS;
    xpsQueueId |= (sai_uint64_t) localQId;

    if (xpSaiObjIdCreate(sai_object_type, devId, xpsQueueId,
                         queue_id) != SAI_STATUS_SUCCESS)
    {
        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiQosQueueObjIdPortValueGet

sai_status_t xpSaiQosQueueObjIdPortValueGet(sai_object_id_t queue_id,
                                            sai_uint32_t *xpsIntfId, sai_uint32_t *localQId)
{
    *localQId = (sai_uint32_t)(queue_id & SAI_QOS_Q_OBJID_VALUE_MASK);
    *xpsIntfId = (sai_uint32_t)((queue_id & SAI_QOS_Q_OBJID_PORT_MASK) >>
                                SAI_QOS_Q_OBJID_VALUE_BITS);

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetDefaultQueueAttributeVals

void xpSaiSetDefaultQueueAttributeVals(xpSaiQueueAttributesT* attributes)
{
    memset(attributes, 0, sizeof(xpSaiQueueAttributesT));
}

//Func: xpSaiUpdateQueueAttributeVals

void xpSaiUpdateQueueAttributeVals(const uint32_t attr_count,
                                   const sai_attribute_t* attr_list, xpSaiQueueAttributesT* attributes)
{
    for (uint32_t count = 0; count < attr_count; count++)
    {
        switch (attr_list[count].id)
        {
            case SAI_QUEUE_ATTR_TYPE:
                {
                    attributes->type = attr_list[count].value;
                    break;
                }
            case SAI_QUEUE_ATTR_PORT:
                {
                    attributes->port = attr_list[count].value;
                    break;
                }
            case SAI_QUEUE_ATTR_INDEX:
                {
                    attributes->index = attr_list[count].value;
                    break;
                }
            case SAI_QUEUE_ATTR_PARENT_SCHEDULER_NODE:
                {
                    attributes->parentSchedulerNode = attr_list[count].value;
                    break;
                }
            case SAI_QUEUE_ATTR_WRED_PROFILE_ID:
                {
                    attributes->wredProfileId = attr_list[count].value;
                    break;
                }
            case SAI_QUEUE_ATTR_BUFFER_PROFILE_ID:
                {
                    attributes->bufferProfileId = attr_list[count].value;
                    break;
                }
            case SAI_QUEUE_ATTR_SCHEDULER_PROFILE_ID:
                {
                    attributes->schedulerProfileId = attr_list[count].value;
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Unknown attribute %d received\n", attr_list[count].id);
                }
        }
    }
}

//Func: xpSaiUpdateAttrListQueueVals

void xpSaiUpdateAttrListQueueVals(const xpSaiQueueAttributesT* attributes,
                                  const uint32_t attr_count, sai_attribute_t* attr_list)
{
    XP_SAI_LOG_DBG("Calling xpSaiUpdateAttrListQueueVals\n");
    for (uint32_t count = 0; count < attr_count; count++)
    {
        switch (attr_list[count].id)
        {
            case SAI_QUEUE_ATTR_TYPE:
                {
                    attr_list[count].value = attributes->type;
                    break;
                }
            case SAI_QUEUE_ATTR_PORT:
                {
                    attr_list[count].value = attributes->port;
                    break;
                }
            case SAI_QUEUE_ATTR_INDEX:
                {
                    attr_list[count].value = attributes->index;
                    break;
                }
            case SAI_QUEUE_ATTR_PARENT_SCHEDULER_NODE:
                {
                    attr_list[count].value = attributes->parentSchedulerNode;
                    break;
                }
            case SAI_QUEUE_ATTR_WRED_PROFILE_ID:
                {
                    attr_list[count].value = attributes->wredProfileId;
                    break;
                }
            case SAI_QUEUE_ATTR_BUFFER_PROFILE_ID:
                {
                    attr_list[count].value = attributes->bufferProfileId;
                    break;
                }
            case SAI_QUEUE_ATTR_SCHEDULER_PROFILE_ID:
                {
                    attr_list[count].value = attributes->schedulerProfileId;
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Failed to set %d\n", attr_list[count].id);
                }
        }
    }
}

//Func: xpSaiGetQueueAttrType

sai_status_t xpSaiGetQueueAttrType(sai_object_id_t queue_id,
                                   sai_attribute_value_t* value)
{
    xpSaiQueueQosInfo_t *pQueueQosInfo = NULL;
    XP_STATUS            retVal = XP_NO_ERR;

    /* Get the queue qos information */
    if ((retVal = xpSaiGetQueueQosInfo(queue_id, &pQueueQosInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Couldn't get sai queue state info |retVal: %d\n",
                       retVal);
        if (retVal == XP_ERR_KEY_NOT_FOUND)
        {
            return SAI_STATUS_INVALID_OBJECT_ID;
        }
        return xpsStatus2SaiStatus(retVal);
    }

    value->s32 = pQueueQosInfo->type;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetQueueAttrPort

sai_status_t xpSaiGetQueueAttrPort(sai_object_id_t queue_id,
                                   sai_attribute_value_t* value)
{
    sai_object_id_t portObjId   = SAI_NULL_OBJECT_ID;
    sai_status_t    retVal      = SAI_STATUS_SUCCESS;
    sai_uint32_t    xpIntfId    = 0;
    sai_uint32_t    queueNum    = 0;
    xpsDevice_t     devId       = xpSaiGetDevId();

    xpSaiQosQueueObjIdPortValueGet(queue_id, &xpIntfId, &queueNum);

    if ((retVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_PORT, devId,
                                   (sai_uint64_t)xpIntfId, &portObjId)) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Couldn't create port object id | retVal: %d\n", retVal);
        return retVal;
    }

    value->oid = portObjId;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetQueueAttrIndex

sai_status_t xpSaiGetQueueAttrIndex(sai_object_id_t queue_id,
                                    sai_attribute_value_t* value)
{
    sai_uint32_t xpIntfId = 0;
    sai_uint32_t queueNum = 0;

    xpSaiQosQueueObjIdPortValueGet(queue_id, &xpIntfId, &queueNum);

    value->u8 = queueNum;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetQueueAttrParentSchedulerNode

sai_status_t xpSaiSetQueueAttrParentSchedulerNode(sai_object_id_t queue_id,
                                                  sai_attribute_value_t value)
{
    xpSaiQueueQosInfo_t *pQueueQosInfo = NULL;
    sai_object_id_t      parentSchedNodeObjId = value.oid;
    XP_STATUS            retVal = XP_NO_ERR;

    /* Validate parent scheduler node object id */
    if ((!XDK_SAI_OBJID_TYPE_CHECK(parentSchedNodeObjId, SAI_OBJECT_TYPE_PORT)) &&
        (!XDK_SAI_OBJID_TYPE_CHECK(parentSchedNodeObjId,
                                   SAI_OBJECT_TYPE_SCHEDULER_GROUP)))
    {
        XP_SAI_LOG_ERR("Error: Wrong object type received (%u)\n",
                       xpSaiObjIdTypeGet(parentSchedNodeObjId));
        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    /* Get the queue qos information */
    if ((retVal = xpSaiGetQueueQosInfo(queue_id, &pQueueQosInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Couldn't get sai queue state info |retVal: %d\n",
                       retVal);
        if (retVal == XP_ERR_KEY_NOT_FOUND)
        {
            return SAI_STATUS_INVALID_OBJECT_ID;
        }
        return xpsStatus2SaiStatus(retVal);
    }

    if (XDK_SAI_OBJID_TYPE_CHECK(parentSchedNodeObjId,
                                 SAI_OBJECT_TYPE_SCHEDULER_GROUP) == true)
    {
        /* Update the parent scheduler group child list in case of HQoS */
        if ((retVal = xpSaiSchedulerGroupBindChildToParent(parentSchedNodeObjId,
                                                           queue_id)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Could not add queue to parent scheduler group | retVal:%d\n",
                           retVal);
            return xpsStatus2SaiStatus(retVal);
        }
    }

    /* Update state database */
    pQueueQosInfo->parentSchedulerNode = value.oid;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetQueueAttrParentSchedulerNode

sai_status_t xpSaiGetQueueAttrParentSchedulerNode(sai_object_id_t queue_id,
                                                  sai_attribute_value_t* value)
{
    xpSaiQueueQosInfo_t *pQueueQosInfo = NULL;
    XP_STATUS            retVal = XP_NO_ERR;

    /* Get the queue qos information */
    if ((retVal = xpSaiGetQueueQosInfo(queue_id, &pQueueQosInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Couldn't get sai queue state info |retVal: %d\n",
                       retVal);
        if (retVal == XP_ERR_KEY_NOT_FOUND)
        {
            return SAI_STATUS_INVALID_OBJECT_ID;
        }
        return xpsStatus2SaiStatus(retVal);
    }

    value->oid = pQueueQosInfo->parentSchedulerNode;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetQueueAttrWredProfileId

sai_status_t xpSaiSetQueueAttrWredProfileId(sai_object_id_t queue_id,
                                            sai_attribute_value_t value)
{
    XP_STATUS       xpRetVal  = XP_NO_ERR;
    sai_uint32_t    portNum   = 0;
    sai_uint32_t    queueNum  = 0;
    sai_status_t    saiStatus = SAI_STATUS_SUCCESS;
    sai_object_id_t wredObjId = value.oid;
    xpSaiWred_t * pWredInfo;
    bool          buffProfileConfigured =false;
    sai_uint32_t  buffersizeInPages = 0;

    /* Validate Wred object */
    saiStatus = xpSaiWredValidateWredObject(wredObjId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    sai_uint32_t greenProfileId, yellowProfileId, redProfileId;

    /* Get the Aqm Queue Green, Yellow and Red profile Ids */
    xpSaiWredObjIdValueGet(wredObjId, &greenProfileId, &yellowProfileId,
                           &redProfileId);

    xpSaiQosQueueObjIdPortValueGet(queue_id, &portNum, &queueNum);
    if (queueNum >= XPS_MAX_QUEUES_PER_PORT)
    {
        XP_SAI_LOG_ERR("Invalid queueNum :%d \n", queueNum);
        return SAI_STATUS_FAILURE;
    }

    /* Get the device type */
    XP_DEV_TYPE_T devType = xpSaiSwitchDevTypeGet();
    xpsDevice_t   xpDevId = xpSaiObjIdSwitchGet(queue_id);

    if (IS_DEVICE_FALCON(xpSaiSwitchDevTypeGet()))
    {
        if (wredObjId != SAI_NULL_OBJECT_ID)
        {
            XP_SAI_LOG_DBG("Try to set WRED profile id on queue %d port%d\n", queueNum,
                           portNum);
            /* Check if wred object exist */
            xpRetVal = xpSaiWredGetState(wredObjId, &pWredInfo);
            if (xpRetVal != XP_NO_ERR)
            {
                return SAI_STATUS_INVALID_OBJECT_ID;
            }

            xpRetVal = xpSaiProfileMngConfigureWredAndEcn(xpDevId, portNum, queueNum,
                                                          pWredInfo);
            if (xpRetVal != XP_NO_ERR)
            {
                return SAI_STATUS_INVALID_OBJECT_ID;
            }
        }
        else
        {

            saiStatus = xpSaiQueueIsBufferConfigured(queue_id, &buffProfileConfigured,
                                                     &buffersizeInPages);
            if (SAI_STATUS_SUCCESS != saiStatus)
            {
                XP_SAI_LOG_ERR("Failed to get (SAI_QUEUE_ATTR_WRED_PROFILE_ID)\n");
                return saiStatus;
            }

            if (true==buffProfileConfigured)
            {
                XP_SAI_LOG_DBG("Reapply buffer\n");
                /* Configure reserved/dedicated buffers */
                xpRetVal = xpSaiProfileMngConfigureGuaranteedLimitAndAlfa(xpDevId, portNum,
                                                                          queueNum, &buffersizeInPages, NULL);
                if (xpRetVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("Failed to configure queue page tail drop threshold, xpStatus: %d\n",
                                   xpRetVal);
                    return xpsStatus2SaiStatus(xpRetVal);
                }
            }
        }
    }
    else if (devType == XP80 || devType == XP80B0)
    {
        /* Bind the profile to Queue */
        if ((xpRetVal = xpsQosAqmBindAqmQProfileToQueue(xpDevId, portNum, queueNum,
                                                        greenProfileId)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Failed to bind aqm queue profile to queue %ld| retVal: %d\n",
                           (long int)queue_id, xpRetVal);
            return xpRetVal;
        }
    }
    else
    {
        /* Bind Green Aqm Queue Profile to Queue */
        if ((xpRetVal = xpsQosAqmBindAqmQProfileToColorAwareQueue(xpDevId, portNum,
                                                                  queueNum, XP_COLOR_TYPE_GREEN, greenProfileId)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Failed to bind green aqm queue profile to queue %ld| retVal: %d\n",
                           (long int)queue_id, xpRetVal);
            return xpRetVal;
        }

        /* Bind Yellow Aqm Queue Profile to Queue */
        if ((xpRetVal = xpsQosAqmBindAqmQProfileToColorAwareQueue(xpDevId, portNum,
                                                                  queueNum, XP_COLOR_TYPE_YELLOW, yellowProfileId)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Failed to bind yellow aqm queue profile to queue %ld| retVal: %d\n",
                           (long int)queue_id, xpRetVal);
            return xpRetVal;
        }

        /* Bind Red Aqm Queue Profile to Queue */
        if ((xpRetVal = xpsQosAqmBindAqmQProfileToColorAwareQueue(xpDevId, portNum,
                                                                  queueNum, XP_COLOR_TYPE_RED, redProfileId)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Failed to bind red aqm queue profile to queue %ld| retVal: %d\n",
                           (long int)queue_id, xpRetVal);
            return xpRetVal;
        }
    }

    xpSaiQueueQosInfo_t *pQueueQosInfo = NULL;

    /* Get the Queue state database */
    if ((xpRetVal = xpSaiGetQueueQosInfo(queue_id, &pQueueQosInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get Queue state database |retVal:%d\n", xpRetVal);
        if (xpRetVal == XP_ERR_KEY_NOT_FOUND)
        {
            return SAI_STATUS_INVALID_OBJECT_ID;
        }
        return xpsStatus2SaiStatus(xpRetVal);
    }

    /* Unbind queue from current wred */
    if (pQueueQosInfo->wredObjId != SAI_NULL_OBJECT_ID)
    {
        saiStatus = xpSaiWredUnbindQueueToWred(pQueueQosInfo->wredObjId, queue_id);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Error: Failed to unbind unbind queue from current wred, saiStatus: %d\n",
                           saiStatus);
            return saiStatus;
        }
    }

    /* Bind queue to new wred */
    if (wredObjId != SAI_NULL_OBJECT_ID)
    {
        saiStatus = xpSaiWredBindQueueToWred(wredObjId, queue_id);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Error: Failed to unbind bind queue to new wred, saiStatus: %d\n",
                           saiStatus);
            return saiStatus;
        }
    }

    /* Update the Queue state database */
    pQueueQosInfo->wredObjId = wredObjId;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetQueueAttrWredProfileId

sai_status_t xpSaiGetQueueAttrWredProfileId(sai_object_id_t queue_id,
                                            sai_attribute_value_t* value)
{
    xpSaiQueueQosInfo_t *pQueueQosInfo = NULL;
    XP_STATUS xpsRetVal = XP_NO_ERR;

    /* Get the Queue state datebase */
    if ((xpsRetVal = xpSaiGetQueueQosInfo(queue_id, &pQueueQosInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get Queue state database |Queue:%d retVal:%d\n",
                       (int)xpSaiObjIdValueGet(queue_id), xpsRetVal);
        if (xpsRetVal == XP_ERR_KEY_NOT_FOUND)
        {
            return SAI_STATUS_INVALID_OBJECT_ID;
        }
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    value->oid = pQueueQosInfo->wredObjId;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiQueueApplyBufferProfile

sai_status_t xpSaiQueueApplyBufferProfile(sai_object_id_t queueObjId,
                                          sai_object_id_t profileObjId)
{
    xpSaiBufferProfile_t *pProfileInfo   = NULL;
    //sai_uint32_t          isLossy        = 0;
    sai_uint32_t          tailDropPageTh = 0;
    sai_uint32_t          dynPoolId      = 0;
    sai_uint32_t          enableDynPool  = 0;
    sai_status_t          saiStatus      = SAI_STATUS_SUCCESS;
    xpsDevice_t           xpsDevId       = 0;
    GT_STATUS             rc             =GT_OK;
    XP_STATUS             xpStatus       = XP_NO_ERR;
    xpsDevType_t          devType        = XP_DEV_TYPE_TOTAL;
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT   alfa;
    GT_BOOL                conversionError=GT_FALSE;
    bool                   wredProfileConfigured=false;

    if (profileObjId == SAI_NULL_OBJECT_ID)
    {
        enableDynPool  = 0;
        dynPoolId      = 0;

        /* Convert default tail drop thresholds from bytes to pages */
        tailDropPageTh = XPSAI_BUFFER_PAGE_LENGTH_MAX_THRESHOLD;
        alfa = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E;
    }
    else
    {
        /* Get the buffer profile state data */
        saiStatus = xpSaiBufferProfileGetStateData(profileObjId, &pProfileInfo);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            return saiStatus;
        }

        /* Get the dynamic pool id */
        xpSaiBufferPoolGetDynamicPoolId(pProfileInfo->poolObjId, &dynPoolId);

        if (pProfileInfo->thMode == SAI_BUFFER_PROFILE_THRESHOLD_MODE_DYNAMIC)
        {
            enableDynPool  = 1;
        }

        /* Convert tail drop thresholds from bytes to pages */
        tailDropPageTh = XPSAI_BYTES_TO_PAGES(pProfileInfo->bufferSize);
    }

    /* Get the device from queue object */
    xpStatus = xpSaiGetQueueDeviceId(queueObjId, &xpsDevId);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to get the device id, xpStatus: %d\n", xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    sai_uint32_t portNum = 0, qNum = 0;

    /* Get the port and queue information from queue object id */
    xpSaiQosQueueObjIdPortValueGet(queueObjId, &portNum, &qNum);
    if (qNum >= XPS_MAX_QUEUES_PER_PORT)
    {
        XP_SAI_LOG_ERR("Invalid queueNum :%d \n", qNum);
        return SAI_STATUS_FAILURE;
    }

    if ((profileObjId == SAI_NULL_OBJECT_ID) || ((pProfileInfo->xoffTh == 0) &&
                                                 (pProfileInfo->xonTh == 0))) // Lossy
    {
        if (profileObjId != SAI_NULL_OBJECT_ID)
        {
            if (SAI_BUFFER_PROFILE_THRESHOLD_MODE_STATIC != pProfileInfo->thMode)
            {
                DYN_TH_2_ALFA(pProfileInfo->sharedDynamicTh, alfa, conversionError);
                if (GT_TRUE==conversionError)
                {
                    XP_SAI_LOG_ERR("Failed to configure alfa from sharedDynamicTh: %d\n",
                                   pProfileInfo->sharedDynamicTh);
                    return SAI_STATUS_INVALID_PARAMETER;
                }
            }
            else
            {
                alfa = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E;
            }
        }

        saiStatus = xpSaiQueueIsWredConfigured(queueObjId, &wredProfileConfigured);
        if (SAI_STATUS_SUCCESS != saiStatus)
        {
            XP_SAI_LOG_ERR("Failed to get (SAI_QUEUE_ATTR_WRED_PROFILE_ID)\n");
            return saiStatus;
        }

        if (IS_DEVICE_FALCON(xpSaiSwitchDevTypeGet()))
        {
            /*In Falcon WRED and tail drop use same profile .WRED configuration override TD*/
            if (true ==wredProfileConfigured)
            {
                XP_SAI_LOG_DBG("Not appliyng   buffer  profile size on queue %d port%d since WRED configured\n",
                               qNum, portNum);
            }

        }

        XP_SAI_LOG_DBG("Try to set buffer  profile id (TD) on queue %d port%d\n", qNum,
                       portNum);

        /* Configure reserved/dedicated buffers */
        xpStatus = xpSaiProfileMngConfigureGuaranteedLimitAndAlfa(xpsDevId, portNum,
                                                                  qNum, wredProfileConfigured?NULL:&tailDropPageTh, &alfa);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to configure queue page tail drop threshold, xpStatus: %d\n",
                           xpStatus);
            return xpsStatus2SaiStatus(xpStatus);
        }

        //isLossy = 1;
    }
    else // Lossless
    {
        //isLossy = 0;
    }

    xpStatus = xpSalGetDeviceType(xpsDevId, &devType);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to retrieve devType from devId %d!\n", xpsDevId);
        return xpsStatus2SaiStatus(xpStatus);
    }

    /* Disable dequeue before changing dynamic pool */
    rc = cpssHalPortTxQueueTxEnableSet(xpsDevId, portNum, qNum, GT_FALSE);
    if (rc != GT_OK)
    {
        XP_SAI_LOG_ERR("Failed to set enqueue disable, port: %d, queue: %d\n", portNum,
                       qNum);
        return cpssStatus2SaiStatus(rc);
    }

    uint32_t queueDepth = 1,
             pollQueueLengthCount = 0xFF; // In any case to avoid infinite loop

    while (pollQueueLengthCount && queueDepth)
    {
        usleep(100);
        xpStatus = xpsQosQcGetCurrentQueuePageDepth(xpsDevId, portNum, qNum,
                                                    &queueDepth);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to get current queue page depth, port: %d, queue: %d\n",
                           portNum, qNum);
            return xpsStatus2SaiStatus(xpStatus);
        }
        pollQueueLengthCount -= 1;
    }

    /* Configure the shared buffers */
    /* Bind the queue to dynamic pool */

    rc = cpssHalPortTxTcMapToSharedEgressPoolSet(xpsDevId, qNum, dynPoolId);
    if (rc != GT_OK)
    {
        XP_SAI_LOG_ERR("Failed to bind queue to dynamic pool, xpStatus: %d\n", rc);
        return cpssStatus2SaiStatus(rc);
    }

    /* Set queue dynamic pool enable */
    xpStatus = xpsQosAqmSetQueueDynamicPoolEnable(xpsDevId, portNum, qNum,
                                                  enableDynPool);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to set queue dynamic pool enable, xpStatus: %d\n",
                       xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    /*Do not enable deque if it is total throttle profile*/
    if (GT_FALSE==xpSaiBufferTotalThrottleGet(pProfileInfo))
    {
        /* Enable enqueue */
        rc = cpssHalPortTxQueueTxEnableSet(xpsDevId, portNum, qNum, GT_TRUE);
        if (rc != GT_OK)
        {
            XP_SAI_LOG_ERR("Failed to set enqueue disable, port: %d, queue: %d\n", portNum,
                           qNum);
            return cpssStatus2SaiStatus(rc);
        }
    }
    else
    {
        XP_SAI_LOG_DBG("Deque from port %d queue %d is disabled.\n", qNum, portNum);
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetQueueAttrBufferProfileId

sai_status_t xpSaiSetQueueAttrBufferProfileId(sai_object_id_t queueObjId,
                                              sai_attribute_value_t value)
{
    xpSaiQueueQosInfo_t *pQueueQosInfo = NULL;
    sai_object_id_t      profileObjId  = value.oid;
    sai_status_t         saiStatus     = SAI_STATUS_SUCCESS;
    XP_STATUS            xpStatus      = XP_NO_ERR;

    /* Validate buffer profile object */
    saiStatus = xpSaiBufferProfileValidateProfileObject(profileObjId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    /* Get the queue state data */
    xpStatus = xpSaiGetQueueQosInfo(queueObjId, &pQueueQosInfo);
    if (xpStatus != XP_NO_ERR)
    {
        if (xpStatus == XP_ERR_KEY_NOT_FOUND)
        {
            return SAI_STATUS_INVALID_OBJECT_ID;
        }
        return xpsStatus2SaiStatus(xpStatus);
    }

    if (pQueueQosInfo->bufferProfileId == profileObjId)
    {
        /* Return if already programmed */
        return SAI_STATUS_SUCCESS;
    }

    if (pQueueQosInfo->bufferProfileId != SAI_NULL_OBJECT_ID)
    {
        /* Unbind queue from previous buffer profile and free the previously allocated buffer pool memory */
        saiStatus = xpSaiBufferRemoveQueueFromBufferProfile(
                        pQueueQosInfo->bufferProfileId, queueObjId);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Error: Failed to unbind queue from buffer profile, saiStatus: %d\n",
                           saiStatus);
            return saiStatus;
        }
    }

    if (profileObjId != SAI_NULL_OBJECT_ID)
    {
        /* Bind queue to buffer profile and allocate guaranteed memory from new buffer pool */
        saiStatus = xpSaiBufferAddQueueToBufferProfile(profileObjId, queueObjId);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Error: Failed to bind queue to buffer profile, saiStatus: %d\n",
                           saiStatus);
            return saiStatus;
        }
    }

    /* Configure buffer profile on queue */
    saiStatus = xpSaiQueueApplyBufferProfile(queueObjId, profileObjId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Failed to configure buffer profile on queue, saiStatus: %d\n",
                       saiStatus);
        return saiStatus;
    }

    /* Update state data */
    pQueueQosInfo->bufferProfileId = profileObjId;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetQueueAttrBufferProfileId

sai_status_t xpSaiGetQueueAttrBufferProfileId(sai_object_id_t queue_id,
                                              sai_attribute_value_t* value)
{
    xpSaiQueueQosInfo_t *pQueueQosInfo = NULL;
    XP_STATUS xpsRetVal = XP_NO_ERR;

    /* Get the port qos information */
    if ((xpsRetVal = xpSaiGetQueueQosInfo(queue_id, &pQueueQosInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get SAI Queue QOS Info |Queue:%d retVal:%d\n",
                       (int)xpSaiObjIdValueGet(queue_id), xpsRetVal);
        if (xpsRetVal == XP_ERR_KEY_NOT_FOUND)
        {
            return SAI_STATUS_INVALID_OBJECT_ID;
        }
        return xpsStatus2SaiStatus(xpsRetVal);
    }
    value->oid = pQueueQosInfo->bufferProfileId;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetQueueAttrSchedulerProfileId

sai_status_t xpSaiGetQueueAttrSchedulerProfileId(sai_object_id_t queue_id,
                                                 sai_attribute_value_t* value)
{
    xpSaiQueueQosInfo_t *pQueueQosInfo = NULL;
    XP_STATUS            xpsRetVal = XP_NO_ERR;

    /* Get the queue entry from state database */
    if ((xpsRetVal = xpSaiGetQueueQosInfo(queue_id, &pQueueQosInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get the queue entry from state database |Queue:%d retVal:%d\n",
                       (int)xpSaiObjIdValueGet(queue_id), xpsRetVal);
        if (xpsRetVal == XP_ERR_KEY_NOT_FOUND)
        {
            return SAI_STATUS_INVALID_OBJECT_ID;
        }
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    value->oid = pQueueQosInfo->schedObjId;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiQueueSetSchedulerInfo

sai_status_t xpSaiQueueSetSchedulerInfo(sai_object_id_t queueObjId,
                                        sai_object_id_t schedObjId)
{
    xpSaiSchedulerAttributesT schedAttrInfo;
    sai_uint32_t schedulingType = 0;
    sai_uint32_t weight         = 0;
    sai_uint64_t maxBurstSize   = 0;
    sai_uint64_t maxRateBps     = 0;
    sai_uint64_t minBurstSize   = 0;
    sai_uint64_t minRateBps     = 0;
    xpsDevice_t  xpDevId        = 0;
    sai_uint32_t xpPort         = 0;
    sai_uint32_t queueNum       = 0;
    XP_STATUS    xpRetVal       = XP_NO_ERR;
    GT_STATUS    rc             = GT_OK;
    sai_status_t saiStatus      = SAI_STATUS_SUCCESS;

    sai_object_id_t       port_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t       queue_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t       portSchedulerObjId[XPSAI_MAX_TC_VALUE];

    xpSaiPortQosInfo_t   *pPortQosInfo = NULL;
    sai_attribute_value_t value;
    uint32_t              qNum = 0;
    //uint32_t              profileIdx = 0;
    uint32_t              newProfileIdx = 0;

    /* Get the device id from queue object id */
    if ((xpRetVal = xpSaiGetQueueDeviceId(queueObjId, &xpDevId)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Could not device id from queue object id | retVal:%d\n",
                       xpRetVal);
        return xpsStatus2SaiStatus(xpRetVal);
    }

    /* Get port and queue info from queue object id */
    xpSaiQosQueueObjIdPortValueGet(queueObjId, &xpPort, &queueNum);
    if (queueNum >= XPS_MAX_QUEUES_PER_PORT)
    {
        XP_SAI_LOG_ERR("Invalid queueNum :%d \n", queueNum);
        return SAI_STATUS_FAILURE;
    }

    if (xpSaiGetQueueAttrPort(queueObjId, &value) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get queue attribute\n");
        return SAI_STATUS_FAILURE;
    }
    port_id = value.oid;

    /* Get the port qos state information */
    if ((xpRetVal = xpSaiGetPortQosInfo(port_id, &pPortQosInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Couldn't get port qos state, portNum:%d, retVal:%d\n",
                       xpPort, xpRetVal);
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    memset(portSchedulerObjId, 0, sizeof(sai_object_id_t) * XPSAI_MAX_TC_VALUE);

    /* read the current list of Scheduler profile of all queues */
    for (qNum = 0; qNum < XPS_MAX_QUEUES_PER_PORT; qNum++)
    {
        if ((pPortQosInfo->qListBitMap[qNum / SIZEOF_BYTE]) & (1 <<
                                                               (qNum % SIZEOF_BYTE)))
        {
            if (xpSaiQosQueueObjIdCreate(SAI_OBJECT_TYPE_QUEUE, xpPort, qNum,
                                         &queue_id) != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Failed to create Queue object id for queue %d\n", qNum);
                return SAI_STATUS_FAILURE;
            }
            xpSaiGetQueueAttrSchedulerProfileId(queue_id, &value);
            portSchedulerObjId[qNum] = value.oid;
        }
        else
        {
            portSchedulerObjId[qNum] = SAI_NULL_OBJECT_ID;
        }
    }
#ifdef NOT_WARM_RESTART
    rc = cpssHalBindPortToSchedulerProfileGet(xpDevId, xpPort, &profileIdx);
    if (rc != GT_OK)
    {
        XP_SAI_LOG_ERR("Error: Could not get scheduler profile for port %d | retVal:%d",
                       xpPort, rc);
        return cpssStatus2SaiStatus(rc);
    }

    /* Loop through schdeduler profile list and clear the refCount of old profile */
    saiStatus = xpSaiProfileMgrUnbindProfileToPort(schedulerProfileList[xpDevId],
                                                   profileIdx, xpPort);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }
#endif

    /* update new scheduler profile */
    portSchedulerObjId[queueNum] = schedObjId;

    /* Find a match for new scheduler profile */
    saiStatus = xpSaiProfileMgrFindMatch(schedulerProfileList[xpDevId],
                                         XPSAI_SCHEDULER_MAX_PROFILES, portSchedulerObjId, &newProfileIdx);
    if (saiStatus == SAI_STATUS_SUCCESS)
    {
        /* bind the new profile index to port */
        if ((rc = cpssHalBindPortToSchedulerProfileSet(xpDevId, xpPort,
                                                       newProfileIdx)) != GT_OK)
        {
            XP_SAI_LOG_ERR("Error: Could not set scheduler profile for port %d | retVal:%d",
                           xpPort, rc);
            return cpssStatus2SaiStatus(rc);
        }
    }
    else
    {
        /* If match is not found, find a free index and configure a new HW scheduler profile */
        saiStatus = xpSaiProfileMgrFindFreeIdx(schedulerProfileList[xpDevId],
                                               XPSAI_SCHEDULER_MAX_PROFILES, &newProfileIdx);
        if (saiStatus == SAI_STATUS_SUCCESS)
        {
            /* bind the free profile index to port */
            if ((rc = cpssHalBindPortToSchedulerProfileSet(xpDevId, xpPort,
                                                           newProfileIdx)) != GT_OK)
            {
                XP_SAI_LOG_ERR("Error: Could not set scheduler profile for port %d | retVal:%d",
                               xpPort, rc);
                return cpssStatus2SaiStatus(rc);
            }

            for (qNum = 0; qNum < XPS_MAX_QUEUES_PER_PORT; qNum++)
            {
                schedulerProfileList[xpDevId][newProfileIdx].profileObjId[qNum] =
                    portSchedulerObjId[qNum];
                if (schedulerProfileList[xpDevId][newProfileIdx].profileObjId[qNum] ==
                    SAI_NULL_OBJECT_ID)
                {
                    /* Set scheduling type and weight to default */
                    schedulingType = XPSAI_SCHEDULER_DEFAULT_SCHEDULING;
                    weight = XPSAI_SCHEDULER_DEFAULT_WEIGHT;
                }
                else
                {
                    /* Get the scheduler attributes */
                    if (xpSaiGetSchedulerAttrInfo(
                            schedulerProfileList[xpDevId][newProfileIdx].profileObjId[qNum],
                            &schedAttrInfo) != XP_NO_ERR)
                    {
                        XP_SAI_LOG_ERR("Failed to get the scheduler attributes\n");
                        return SAI_STATUS_FAILURE;
                    }
                    schedulingType = schedAttrInfo.schedulingType.s32;
                    weight = schedAttrInfo.schedulingWeight.u8;
                }

                /* Set the queue scheduling type */
                if ((saiStatus = xpSaiSetSchedulerQueueSchedulingType(xpDevId, xpPort, qNum,
                                                                      schedulingType)) != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Error: Could not set scheduler queue scheduling type | saiStatus:%d\n",
                                   saiStatus);
                    return saiStatus;
                }

                /* Set the queue scheduler DWRR weight */
                if ((rc = cpssHalPortTxQueueWRRWeightSet(xpDevId, xpPort, qNum,
                                                         weight)) != GT_OK)
                {
                    XP_SAI_LOG_ERR("Error: Could not set queue scheduler DWRR weight | retVal:%d",
                                   rc);
                    return cpssStatus2SaiStatus(rc);
                }
            }
        }
        else
            /* neither matching profile nor free index is found, return error */
        {
            XP_SAI_LOG_ERR("Failed to set Scheduler profile to Queue %d. Only 16 HW scheduler profiles are supported",
                           queueNum);
            return SAI_STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    /* Configure queue shaper */
    if (schedObjId == SAI_NULL_OBJECT_ID)
    {
        maxRateBps        = 0;
        maxBurstSize      = 0;
        minRateBps        = 0;
        minBurstSize      = 0;
    }
    else
    {
        /* Get the scheduler attributes */
        if (xpSaiGetSchedulerAttrInfo(schedObjId, &schedAttrInfo) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to get the scheduler attributes\n");
            return SAI_STATUS_FAILURE;
        }
        maxRateBps     = schedAttrInfo.maxBandwidthRate.u64;
        maxBurstSize   = schedAttrInfo.maxBandwidthBurstRate.u64;
        minRateBps     = schedAttrInfo.minBandwidthRate.u64;
        minBurstSize   = schedAttrInfo.minBandwidthBurstRate.u64;
    }

    if ((saiStatus = xpSaiSetSchedulerQueueShaperMaxRate(xpDevId, xpPort, queueNum,
                                                         maxRateBps, maxBurstSize)) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Could not set scheduler queue shaper rate | retVal: %d\n",
                       saiStatus);
        return saiStatus;
    }

    if ((saiStatus = xpSaiSetSchedulerQueueShaperMinRate(xpDevId, xpPort, queueNum,
                                                         minRateBps, minBurstSize)) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Could not set scheduler queue min shaper rate | retVal: %d\n",
                       saiStatus);
        return saiStatus;
    }

    xpSaiProfileMgrBindProfileToPort(schedulerProfileList[xpDevId], newProfileIdx,
                                     xpPort);

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetQueueAttrSchedulerProfileId

sai_status_t xpSaiSetQueueAttrSchedulerProfileId(sai_object_id_t queueObjId,
                                                 sai_attribute_value_t value)
{
    xpSaiQueueQosInfo_t *pQueueQosInfo = NULL;
    sai_object_id_t      schedObjId = value.oid;
    sai_status_t         saiRetVal;
    XP_STATUS            xpsRetVal;

    /* Validate scheduler object id */
    if ((value.oid != SAI_NULL_OBJECT_ID) &&
        (!XDK_SAI_OBJID_TYPE_CHECK(schedObjId, SAI_OBJECT_TYPE_SCHEDULER)))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n",
                       xpSaiObjIdTypeGet(schedObjId));
        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    /* Get the sai queue qos information */
    if ((xpsRetVal = xpSaiGetQueueQosInfo(queueObjId, &pQueueQosInfo)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Could not get queue state database |retVal:%d\n",
                       xpsRetVal);
        if (xpsRetVal == XP_ERR_KEY_NOT_FOUND)
        {
            return SAI_STATUS_INVALID_OBJECT_ID;
        }
        return xpsStatus2SaiStatus(xpsRetVal);
    }

    if (pQueueQosInfo->schedObjId == schedObjId)
    {
        /* Return if already programmed */
        return SAI_STATUS_SUCCESS;
    }

    /* Configure the scheduler on queue */
    if ((saiRetVal = xpSaiQueueSetSchedulerInfo(queueObjId,
                                                schedObjId)) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed xpSaiQueueSetSchedulerInfo: error code: %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    if (pQueueQosInfo->schedObjId != SAI_NULL_OBJECT_ID)
    {
        /* Unbind the queue from existing scheduler */
        if ((xpsRetVal = xpSaiSchedulerUnbindQueueFromScheduler(
                             pQueueQosInfo->schedObjId, queueObjId)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Failed to unbind the queue from scheduler, retVal: %d\n",
                           xpsRetVal);
            return xpsStatus2SaiStatus(xpsRetVal);
        }
    }

    if (schedObjId != SAI_NULL_OBJECT_ID)
    {
        /* Bind the queue to new scheduler */
        if ((xpsRetVal = xpSaiSchedulerBindQueueToScheduler(schedObjId,
                                                            queueObjId)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Failed to bind the queue to scheduler, retVal: %d\n",
                           xpsRetVal);
            return xpsStatus2SaiStatus(xpsRetVal);
        }
    }

    /* Update the state */
    pQueueQosInfo->schedObjId = schedObjId;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiCreateQueue

sai_status_t xpSaiCreateQueue(sai_object_id_t *queue_id,
                              sai_object_id_t switch_id, uint32_t attr_count,
                              const sai_attribute_t *attr_list)
{
    xpSaiQueueAttributesT  attributes;
    xpSaiQueueQosInfo_t   *pQueueInfo = NULL;
    sai_status_t           saiStatus  = SAI_STATUS_SUCCESS;
    XP_STATUS              xpStatus   = XP_NO_ERR;
    XP_STATUS              xpRetVal   = XP_NO_ERR;
    GT_STATUS              rc         = GT_OK;
    sai_uint32_t           portNum    = 0;
    xpsDevice_t            xpDevId    = 0;

    /* Validate attributes */
    saiStatus = xpSaiAttrCheck(attr_count, attr_list,
                               QUEUE_VALIDATION_ARRAY_SIZE, queue_attribs,
                               SAI_COMMON_API_CREATE);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiStatus);
        return saiStatus;
    }

    xpSaiSetDefaultQueueAttributeVals(&attributes);
    xpSaiUpdateQueueAttributeVals(attr_count, attr_list, &attributes);

    xpDevId  = xpSaiObjIdSwitchGet(switch_id);
    portNum = (xpsInterfaceId_t)xpSaiObjIdValueGet(attributes.port.oid);

    /* Validate input attribute values */
    for (uint32_t count = 0; count < attr_count; count++)
    {
        saiStatus = xpSaiQueueValidateAttributeValue(xpDevId, portNum,
                                                     attr_list[count].id, attr_list[count].value, count);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            return saiStatus;
        }
    }

    /* Create queue object */
    xpSaiQosQueueObjIdCreate(SAI_OBJECT_TYPE_QUEUE, portNum, attributes.index.u8,
                             queue_id);

    /* Create state datebase */
    xpStatus = xpSaiQueueStateCreate(*queue_id, &pQueueInfo);
    if (xpStatus != XP_NO_ERR)
    {
        return xpsStatus2SaiStatus(xpStatus);
    }

    if (attributes.index.u8 < XPS_MAX_QUEUES_PER_PORT)
    {
        /* Set parent node */
        if (XDK_SAI_OBJID_TYPE_CHECK(attributes.parentSchedulerNode.oid,
                                     SAI_OBJECT_TYPE_SCHEDULER_GROUP) == true)
        {
            /* Update the parent scheduler group child list in case of HQoS */
            xpStatus = xpSaiSchedulerGroupBindChildToParent(
                           attributes.parentSchedulerNode.oid, *queue_id);
            if (xpStatus != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Error: Could not add queue to parent scheduler group, xpStatus:%d\n",
                               xpStatus);
                xpRetVal = xpSaiQueueStateRemove(*queue_id);
                if (xpRetVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("Error: Could not remove the queue state, xpRetVal:%d\n",
                                   xpRetVal);
                    return xpsStatus2SaiStatus(xpRetVal);
                }
                return xpsStatus2SaiStatus(xpStatus);
            }
        }

        /* Set wred profile */
        saiStatus = xpSaiSetQueueAttrWredProfileId(*queue_id, attributes.wredProfileId);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Error: Failed to set wred profile on queue, saiStatus: %d\n",
                           saiStatus);
            xpRetVal = xpSaiQueueStateRemove(*queue_id);
            if (xpRetVal != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Error: Could not remove the queue state, xpRetVal:%d\n",
                               xpRetVal);
                return xpsStatus2SaiStatus(xpRetVal);
            }
            return saiStatus;
        }

        /* Set scheduler profile */
        saiStatus = xpSaiSetQueueAttrSchedulerProfileId(*queue_id,
                                                        attributes.schedulerProfileId);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Error: Failed to set scheduler profile on queue, saiStatus: %d\n",
                           saiStatus);
            xpRetVal = xpSaiQueueStateRemove(*queue_id);
            if (xpRetVal != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Error: Could not remove the queue state, xpRetVal:%d\n",
                               xpRetVal);
                return xpsStatus2SaiStatus(xpRetVal);
            }
            return saiStatus;
        }

        /* Set buffer profile */
        saiStatus = xpSaiSetQueueAttrBufferProfileId(*queue_id,
                                                     attributes.bufferProfileId);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Error: Failed to set buffer profile on queue, saiStatus: %d\n",
                           saiStatus);
            xpRetVal = xpSaiQueueStateRemove(*queue_id);
            if (xpRetVal != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Error: Could not remove the queue state, xpRetVal:%d\n",
                               xpRetVal);
                return xpsStatus2SaiStatus(xpRetVal);
            }
            return saiStatus;
        }
    }
    /* Add the queue to port queue list */
    xpStatus = xpSaiPortAddQueueToPortQList(attributes.port.oid,
                                            attributes.index.u8);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Could not add the queue to port queue list xpStatus:%d\n",
                       xpStatus);
        xpRetVal = xpSaiQueueStateRemove(*queue_id);
        if (xpRetVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Could not remove the queue state, xpRetVal:%d\n",
                           xpRetVal);
            return xpsStatus2SaiStatus(xpRetVal);
        }
        return xpsStatus2SaiStatus(xpStatus);
    }

    /*WA - Disabling hardware update for queues 8 - 23 (UNICAST and MULTICAST) type
     * as a workaround for SONIC SNMP queue stats testcases. These are dummy queues
     * and have software states but dont have any presence in ASIC*/

    /* Enable enqueue */
    if (attributes.index.u8 < XPS_MAX_QUEUES_PER_PORT)
    {
        rc = cpssHalPortTxQueueTxEnableSet(xpDevId, portNum, attributes.index.u8,
                                           GT_TRUE);
        if (rc != GT_OK)
        {
            XP_SAI_LOG_ERR("Failed to set enqueue enable, port: %d, queue: %d\n", portNum,
                           attributes.index.u8);
            xpRetVal = xpSaiQueueStateRemove(*queue_id);
            if (xpRetVal != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Error: Could not remove the queue state, xpRetVal:%d\n",
                               xpRetVal);
                return xpsStatus2SaiStatus(xpRetVal);
            }
            return cpssStatus2SaiStatus(rc);
        }
    }
    /* Update state data */
    pQueueInfo->switchId            = switch_id;
    pQueueInfo->type                = (sai_queue_type_t)attributes.type.s32;
    pQueueInfo->parentSchedulerNode = attributes.parentSchedulerNode.oid;
    pQueueInfo->schedObjId          = attributes.schedulerProfileId.oid;
    pQueueInfo->wredObjId           = attributes.wredProfileId.oid;
    pQueueInfo->bufferProfileId     = attributes.bufferProfileId.oid;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiRemoveQueue

sai_status_t xpSaiRemoveQueue(sai_object_id_t queue_id)
{
    sai_attribute_value_t  value;
    xpSaiQueueQosInfo_t   *pQueueInfo = NULL;
    sai_object_id_t        portObjId  = SAI_NULL_OBJECT_ID;
    sai_uint32_t           portNum    = 0, qNum = 0;
    sai_status_t           saiStatus  = SAI_STATUS_SUCCESS;
    XP_STATUS              xpStatus   = XP_NO_ERR;
    GT_STATUS              rc         = GT_OK;
    xpsDevice_t            xpDevId    = xpSaiObjIdSwitchGet(queue_id);

    /* Validate queue object */
    if (!XDK_SAI_OBJID_TYPE_CHECK(queue_id, SAI_OBJECT_TYPE_QUEUE))
    {
        XP_SAI_LOG_DBG("Wrong object type received (%u)\n",
                       xpSaiObjIdTypeGet(queue_id));
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    /* Check if queue exists */
    xpStatus = xpSaiGetQueueQosInfo(queue_id, &pQueueInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Queue does not exist, xpStatus: %d\n", xpStatus);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    /* Get the interface and queue info from queue object id */
    xpSaiQosQueueObjIdPortValueGet(queue_id, &portNum, &qNum);
    if (qNum > 23)
    {
        XP_SAI_LOG_ERR("Invalid queueNum :%d \n", qNum);
        return SAI_STATUS_FAILURE;
    }

    /*WA - Disabling hardware update for queues 8 - 23 (UNICAST and MULTICAST) type
     * as a workaround for SONIC SNMP queue stats testcases. These are dummy queues
     * and have software states but dont have any presence in ASIC*/

    /* Disable enqueue */
    if (qNum < XPS_MAX_QUEUES_PER_PORT)
    {
        rc = cpssHalPortTxQueueTxEnableSet(xpDevId, portNum, qNum, GT_FALSE);
        if (rc != GT_OK)
        {
            XP_SAI_LOG_ERR("Error: Could not disable enqueue, port: %d, queue: %d, rc: %d\n",
                           portNum, qNum, rc);
            return cpssStatus2SaiStatus(rc);
        }
    }
    /* Create port object id */
    saiStatus = xpSaiObjIdCreate(SAI_OBJECT_TYPE_PORT, xpDevId,
                                 (sai_uint64_t)portNum, &portObjId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Couldn't create port object id, saiStatus: %d\n",
                       saiStatus);
        return saiStatus;
    }

    /* Remove the queue from port queue list */
    xpStatus = xpSaiPortRemoveQueueFromPortQList(portObjId, qNum);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Could not remove the queue from port queue list, xpStatus:%d\n",
                       xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    if (qNum < XPS_MAX_QUEUES_PER_PORT)
    {
        /* Set parent node */
        if (XDK_SAI_OBJID_TYPE_CHECK(pQueueInfo->parentSchedulerNode,
                                     SAI_OBJECT_TYPE_SCHEDULER_GROUP) == true)
        {
            /* Update the parent scheduler group child list in case of HQoS */
            xpStatus = xpSaiSchedulerGroupUnbindChildFromParent(
                           pQueueInfo->parentSchedulerNode, queue_id);
            if (xpStatus != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Error: Could not remove queue to parent scheduler group, xpStatus:%d\n",
                               xpStatus);
                return xpsStatus2SaiStatus(xpStatus);
            }
        }

        value.oid  = SAI_NULL_OBJECT_ID;

        /* Set wred profile */
        saiStatus = xpSaiSetQueueAttrWredProfileId(queue_id, value);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Error: Failed to set wred profile on queue, saiStatus: %d\n",
                           saiStatus);
            xpSaiQueueStateRemove(queue_id);
            return saiStatus;
        }

        /* Set scheduler profile */
        saiStatus = xpSaiSetQueueAttrSchedulerProfileId(queue_id, value);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Error: Failed to set scheduler profile on queue, saiStatus: %d\n",
                           saiStatus);
            xpSaiQueueStateRemove(queue_id);
            return saiStatus;
        }

        /* Set buffer profile */
        saiStatus = xpSaiSetQueueAttrBufferProfileId(queue_id, value);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Error: Failed to set buffer profile on queue, saiStatus: %d\n",
                           saiStatus);
            xpSaiQueueStateRemove(queue_id);
            return saiStatus;
        }
    }
    /* Remove state datebase */
    xpStatus = xpSaiQueueStateRemove(queue_id);
    if (xpStatus != XP_NO_ERR)
    {
        return xpsStatus2SaiStatus(xpStatus);
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetQueueAttribute

sai_status_t xpSaiSetQueueAttribute(sai_object_id_t queue_id,
                                    const sai_attribute_t *attr)
{
    xpSaiQueueQosInfo_t *pQueueQosInfo = NULL;
    sai_status_t         retVal        = SAI_STATUS_SUCCESS;
    XP_STATUS            xpStatus      = XP_NO_ERR;

    retVal = xpSaiAttrCheck(1, attr, QUEUE_VALIDATION_ARRAY_SIZE, queue_attribs,
                            SAI_COMMON_API_SET);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", retVal);
        return retVal;
    }

    /* Validate queue object */
    if (!XDK_SAI_OBJID_TYPE_CHECK(queue_id, SAI_OBJECT_TYPE_QUEUE))
    {
        XP_SAI_LOG_DBG("Wrong object type received (%u)\n",
                       xpSaiObjIdTypeGet(queue_id));
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    /* Check if queue exists */
    xpStatus = xpSaiGetQueueQosInfo(queue_id, &pQueueQosInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Queue does not exist, xpStatus: %d\n", xpStatus);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    switch (attr->id)
    {
        case SAI_QUEUE_ATTR_PARENT_SCHEDULER_NODE:
            {
                retVal = xpSaiSetQueueAttrParentSchedulerNode(queue_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_QUEUE_ATTR_PARENT_SCHEDULER_NODE)\n");
                    return retVal;
                }
                break;
            }
        case SAI_QUEUE_ATTR_WRED_PROFILE_ID:
            {
                retVal = xpSaiSetQueueAttrWredProfileId(queue_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_QUEUE_ATTR_WRED_PROFILE_ID)\n");
                    return retVal;
                }
                break;
            }
        case SAI_QUEUE_ATTR_BUFFER_PROFILE_ID:
            {
                retVal = xpSaiSetQueueAttrBufferProfileId(queue_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_QUEUE_ATTR_BUFFER_PROFILE_ID)\n");
                    return retVal;
                }
                break;
            }
        case SAI_QUEUE_ATTR_SCHEDULER_PROFILE_ID:
            {
                retVal = xpSaiSetQueueAttrSchedulerProfileId(queue_id, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_QUEUE_ATTR_SCHEDULER_PROFILE_ID)\n");
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
    return retVal;
}

//Func: xpSaiGetQueueAttribute

static sai_status_t xpSaiGetQueueAttribute(sai_object_id_t queue_id,
                                           sai_attribute_t *attr, uint32_t attr_index)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    switch (attr->id)
    {
        case SAI_QUEUE_ATTR_TYPE:
            {
                saiRetVal = xpSaiGetQueueAttrType(queue_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_QUEUE_ATTR_TYPE)\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_QUEUE_ATTR_PORT:
            {
                saiRetVal = xpSaiGetQueueAttrPort(queue_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_QUEUE_ATTR_PORT)\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_QUEUE_ATTR_INDEX:
            {
                saiRetVal = xpSaiGetQueueAttrIndex(queue_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_QUEUE_ATTR_INDEX)\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_QUEUE_ATTR_PARENT_SCHEDULER_NODE:
            {
                saiRetVal = xpSaiGetQueueAttrParentSchedulerNode(queue_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_QUEUE_ATTR_PARENT_SCHEDULER_NODE)\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_QUEUE_ATTR_WRED_PROFILE_ID:
            {
                saiRetVal = xpSaiGetQueueAttrWredProfileId(queue_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_QUEUE_ATTR_WRED_PROFILE_ID)\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_QUEUE_ATTR_BUFFER_PROFILE_ID:
            {
                saiRetVal = xpSaiGetQueueAttrBufferProfileId(queue_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_QUEUE_ATTR_BUFFER_PROFILE_ID)\n");
                    return saiRetVal;
                }
                break;
            }
        case SAI_QUEUE_ATTR_SCHEDULER_PROFILE_ID:
            {
                saiRetVal = xpSaiGetQueueAttrSchedulerProfileId(queue_id, &attr->value);
                if (SAI_STATUS_SUCCESS != saiRetVal)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_QUEUE_ATTR_SCHEDULER_PROFILE_ID)\n");
                    return saiRetVal;
                }
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Invalid parameter received %d\n", attr->id);
                return SAI_STATUS_INVALID_PARAMETER;
            }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetQueueAttributes

static sai_status_t xpSaiGetQueueAttributes(sai_object_id_t queue_id,
                                            uint32_t attr_count, sai_attribute_t *attr_list)
{
    xpSaiQueueQosInfo_t *pQueueQosInfo = NULL;
    sai_status_t         saiStatus     = SAI_STATUS_SUCCESS;
    XP_STATUS            xpStatus      = XP_NO_ERR;

    saiStatus = xpSaiAttrCheck(attr_count, attr_list,
                               QUEUE_VALIDATION_ARRAY_SIZE, queue_attribs,
                               SAI_COMMON_API_GET);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiStatus);
        return saiStatus;
    }

    /* Validate queue object */
    if (!XDK_SAI_OBJID_TYPE_CHECK(queue_id, SAI_OBJECT_TYPE_QUEUE))
    {
        XP_SAI_LOG_ERR("Wrong object type received (%u)\n",
                       xpSaiObjIdTypeGet(queue_id));
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    /* Check if queue exists */
    xpStatus = xpSaiGetQueueQosInfo(queue_id, &pQueueQosInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Queue does not exist, xpStatus: %d\n", xpStatus);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    for (uint32_t count = 0; count < attr_count; count++)
    {
        saiStatus = xpSaiGetQueueAttribute(queue_id, &attr_list[count], count);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiGetQueueAttribute failed with saiStatus : %d\n",
                           saiStatus);
            return saiStatus;
        }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiBulkGetQueueAttributes

sai_status_t xpSaiBulkGetQueueAttributes(sai_object_id_t id,
                                         uint32_t *attr_count, sai_attribute_t *attr_list)
{
    sai_status_t     saiRetVal  = SAI_STATUS_SUCCESS;
    uint32_t         idx        = 0;
    uint32_t         maxcount   = 0;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    /* Check incoming parameters */
    if (!XDK_SAI_OBJID_TYPE_CHECK(id, SAI_OBJECT_TYPE_QUEUE))
    {
        XP_SAI_LOG_DBG("Wrong object type received (%u)\n", xpSaiObjIdTypeGet(id));
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    if ((attr_count == NULL) || (attr_list == NULL))
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiMaxCountQueueAttribute(&maxcount);
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
        attr_list[idx].id = SAI_QUEUE_ATTR_START + count;
        saiRetVal = xpSaiGetQueueAttribute(id, &attr_list[idx], count);

        if (saiRetVal == SAI_STATUS_SUCCESS)
        {
            idx++;
        }
    }
    *attr_count = idx;

    return SAI_STATUS_SUCCESS;
}

static sai_status_t xpSaiClearQueueStatsDB(uint32_t number_of_counters,
                                           const sai_stat_id_t *counter_ids,
                                           xpSaiQueueQosInfo_t *pQueueQosInfo)
{
    if ((counter_ids == NULL) || (pQueueQosInfo == NULL))
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    for (uint32_t idx = 0; idx < number_of_counters; idx++)
    {
        switch (counter_ids[idx])
        {
            /* clear tx packet count */
            case SAI_QUEUE_STAT_PACKETS:
                {
                    pQueueQosInfo->passPkts = 0;
                    break;
                }
            /* clear tx byte count */
            case SAI_QUEUE_STAT_BYTES:
                {
                    pQueueQosInfo->passBytes = 0;
                    break;
                }
            /* clear dropped packet count */
            case SAI_QUEUE_STAT_DROPPED_PACKETS:
                {
                    pQueueQosInfo->dropPkts = 0;
                    break;
                }
            /* clear dropped byte count */
            case SAI_QUEUE_STAT_DROPPED_BYTES:
                {
                    pQueueQosInfo->dropBytes = 0;
                    break;
                }
            /* clear watermark queue occupancy in bytes */
            case SAI_QUEUE_STAT_WATERMARK_BYTES:
            /* clear watermark queue shared occupancy in bytes */
            case SAI_QUEUE_STAT_SHARED_WATERMARK_BYTES:
                {
                    pQueueQosInfo->watermark = 0;
                    break;
                }
            /* clear green color tx packets count */
            case SAI_QUEUE_STAT_GREEN_PACKETS:
            /* clear green color tx bytes count */
            case SAI_QUEUE_STAT_GREEN_BYTES:
            /* clear green color dropped packets count */
            case SAI_QUEUE_STAT_GREEN_DROPPED_PACKETS:
            /* clear green color dropped packets count */
            case SAI_QUEUE_STAT_GREEN_DROPPED_BYTES:
            /* clear yellow color tx packets count */
            case SAI_QUEUE_STAT_YELLOW_PACKETS:
            /* clear yellow color tx bytes count */
            case SAI_QUEUE_STAT_YELLOW_BYTES:
            /* clear yellow color drooped packets count */
            case SAI_QUEUE_STAT_YELLOW_DROPPED_PACKETS:
            /* clear yellow color dropped bytes count */
            case SAI_QUEUE_STAT_YELLOW_DROPPED_BYTES:
            /* clear red color tx packets count */
            case SAI_QUEUE_STAT_RED_PACKETS:
            /* clear red color tx bytes count */
            case SAI_QUEUE_STAT_RED_BYTES:
            /* clear red color dropped packets count */
            case SAI_QUEUE_STAT_RED_DROPPED_PACKETS:
            /* clear red color drooped bytes count */
            case SAI_QUEUE_STAT_RED_DROPPED_BYTES:
            /* clear WRED green color dropped packets count */
            case SAI_QUEUE_STAT_GREEN_WRED_DROPPED_PACKETS:
            /* clear WRED green color dropped bytes count */
            case SAI_QUEUE_STAT_GREEN_WRED_DROPPED_BYTES:
            /* clear WRED yellow color dropped packets count */
            case SAI_QUEUE_STAT_YELLOW_WRED_DROPPED_PACKETS:
            /* clear WRED yellow color dropped bytes count */
            case SAI_QUEUE_STAT_YELLOW_WRED_DROPPED_BYTES:
            /* clear WRED red color dropped packets count */
            case SAI_QUEUE_STAT_RED_WRED_DROPPED_PACKETS:
            /* clear WRED red color dropped bytes count */
            case SAI_QUEUE_STAT_RED_WRED_DROPPED_BYTES:
                {
                    XP_SAI_LOG_DBG("Attribute %d not supported\n", counter_ids[idx]);
                    //saiStatus = SAI_STATUS_ATTR_NOT_SUPPORTED_0 + SAI_STATUS_CODE(idx);
                    break;
                }
            /* clear WRED dropped packets count */
            case SAI_QUEUE_STAT_WRED_DROPPED_PACKETS:
                {
                    pQueueQosInfo->wredDropPkts = 0;
                    break;
                }
            /* clear WRED dropped bytes count */
            case SAI_QUEUE_STAT_WRED_DROPPED_BYTES:
                {
                    pQueueQosInfo->wredDropBytes = 0;
                    break;
                }
            /* clear current queue occupancy in bytes */
            case SAI_QUEUE_STAT_CURR_OCCUPANCY_BYTES:
            /* clear current queue shared occupancy in bytes */
            case SAI_QUEUE_STAT_SHARED_CURR_OCCUPANCY_BYTES:
                {
                    XP_SAI_LOG_DBG("Attribute %d not supported\n", counter_ids[idx]);
                    //saiStatus = SAI_STATUS_ATTR_NOT_SUPPORTED_0 + SAI_STATUS_CODE(idx);
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Undefined attribute %d\n", counter_ids[idx]);
                    //saiStatus = SAI_STATUS_UNKNOWN_ATTRIBUTE_0 + SAI_STATUS_CODE(idx);
                }
        }
    }
    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiQueueStatisticsStoreDB

sai_status_t xpSaiQueueStatisticsStoreDB(sai_object_id_t queue_id,
                                         xpSaiQueueQosInfo_t **ppQueueQosInfo)
{
    xpSaiQueueQosInfo_t *pQueueQosInfo = NULL;
    XP_STATUS            xpStatus      = XP_NO_ERR;
    sai_uint32_t         portNum;
    sai_uint32_t         queueNum;
    sai_uint64_t         passBytes     = 0;
    sai_uint64_t         passPkts      = 0;
    sai_uint64_t         dropBytes     = 0;
    sai_uint64_t         dropPkts      = 0;
    sai_uint64_t         watermark     = 0;
    GT_STATUS            rc;
    sai_status_t         saiStatus = SAI_STATUS_SUCCESS;

    /* Get the port and queue from queue object */
    xpSaiQosQueueObjIdPortValueGet(queue_id, &portNum, &queueNum);

    xpsDevice_t xpsDevId = xpSaiObjIdSwitchGet(queue_id);

    xpStatus = xpSaiGetQueueQosInfo(queue_id, &pQueueQosInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Queue does not exist, xpStatus: %d\n", xpStatus);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }
    *ppQueueQosInfo = pQueueQosInfo;

    if (queueNum < XPS_MAX_QUEUES_PER_PORT)
    {
        uint32_t cpuPortNum = 0;
        uint64_t newPolicerdropPkts = 0;

        /*
            COPP use Egress Policer.
            Because the Egress Policer engine is after the PRE-Q engine.
            So the CPU port queue CNC pass count is need to subminus the Egress Policer drop count,
            and drop count is need to plus the Egress Policer drop counter.
            This is a workaround.
        */

        xpStatus = xpsGlobalSwitchControlGetCpuPhysicalPortNum(xpsDevId, &cpuPortNum);
        if (XP_NO_ERR != xpStatus)
        {
            XP_SAI_LOG_ERR("xpsGlobalSwitchControlGetCpuPhysicalPortNum call failed! Error %d\n",
                           xpStatus);
            return xpsStatus2SaiStatus(xpStatus);
        }
        // reading the counter of policer should be before the reading the Queue CNC.
        // Otherwise, "queue CNC counter" - "policer counter" will be a vary great value(a negative value)
        if (portNum == cpuPortNum)
        {
            saiStatus = xpSaiHostInterfaceTrapQueueDropCounter(queueNum,
                                                               &newPolicerdropPkts);
            if (SAI_STATUS_SUCCESS != saiStatus)
            {
                XP_SAI_LOG_ERR("Error : can get cpu queue drop packets \n");
                return saiStatus;
            }
        }

        rc = cpssHalCncQueueStatGet(xpsDevId, portNum, queueNum,
                                    &passBytes, &passPkts, &dropBytes, &dropPkts);
        if (rc != GT_OK)
        {
            XP_SAI_LOG_ERR("Failed to get queue stats | retVal = %d\n", rc);
            return cpssStatus2SaiStatus(rc);
        }
        rc = cpssHalCncQueueWatermarkGet(xpsDevId, portNum, queueNum, &watermark);
        if (rc != GT_OK)
        {
            XP_SAI_LOG_ERR("Failed to get queue watermark | retVal = %d\n", rc);
            return cpssStatus2SaiStatus(rc);
        }
        pQueueQosInfo->passPkts  += passPkts;
        pQueueQosInfo->passBytes += passBytes;
        pQueueQosInfo->dropPkts  += dropPkts;
        pQueueQosInfo->dropBytes += dropBytes;
        if (watermark > pQueueQosInfo->watermark)
        {
            pQueueQosInfo->watermark = watermark;
        }

        pQueueQosInfo->pg_passPkts  += passPkts;
        pQueueQosInfo->pg_passBytes += passBytes;
        pQueueQosInfo->pg_dropPkts  += dropPkts;
        pQueueQosInfo->pg_dropBytes += dropBytes;

        if (pQueueQosInfo->wredObjId != SAI_NULL_OBJECT_ID)
        {
            pQueueQosInfo->wredDropPkts  += dropPkts;
            pQueueQosInfo->wredDropBytes += dropBytes;

            pQueueQosInfo->port_wredDropPkts  += dropPkts;
            pQueueQosInfo->port_wredDropBytes += dropBytes;
        }

        if (portNum == cpuPortNum)
        {
            uint64_t deltaPolicerdropPkts = 0;

            if (newPolicerdropPkts < pQueueQosInfo->CoppPolicerDropPktsOfLastRead)
            {
                XP_SAI_LOG_WARNING("WARN: Copp counter should not be increased \n");
            }
            else
            {
                deltaPolicerdropPkts = newPolicerdropPkts -
                                       pQueueQosInfo->CoppPolicerDropPktsOfLastRead;
            }

            if (pQueueQosInfo->passPkts < deltaPolicerdropPkts)
            {
                XP_SAI_LOG_WARNING("WARN: CPU queue counter should not be less than Copp counter \n");
            }
            else
            {
                pQueueQosInfo->passPkts -= deltaPolicerdropPkts;
                pQueueQosInfo->dropPkts += deltaPolicerdropPkts;
            }

            pQueueQosInfo->CoppPolicerDropPktsOfLastRead = newPolicerdropPkts;

            // Policer not support "Bytes" currently.
            pQueueQosInfo->passBytes = 0;
            pQueueQosInfo->dropBytes = 0;
        }
    }
    else
    {
        return SAI_STATUS_SUCCESS;
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiClearQueueStatsInReadClearMode

sai_status_t xpSaiClearQueueStatsInReadClearMode(sai_object_id_t queue_id,
                                                 uint32_t number_of_counters, const sai_stat_id_t *counter_ids)
{
    xpSaiQueueQosInfo_t *pQueueQosInfo = NULL;
    sai_status_t         saiStatus     = SAI_STATUS_SUCCESS;
    sai_uint32_t portNum = 0;
    sai_uint32_t queueNum = 0;

    /* Get the port and queue from queue object */
    xpSaiQosQueueObjIdPortValueGet(queue_id, &portNum, &queueNum);

    if ((number_of_counters < 1) || (!counter_ids))
    {
        XP_SAI_LOG_ERR("%s invalid parameters received.\n", __FUNCNAME__);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(queue_id, SAI_OBJECT_TYPE_QUEUE))
    {
        XP_SAI_LOG_DBG("Wrong object type received (%u)\n",
                       xpSaiObjIdTypeGet(queue_id));
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    /* read queue all type statistics to XPS State DB, this operation will clear statistics in HW */
    saiStatus = xpSaiQueueStatisticsStoreDB(queue_id, &pQueueQosInfo);
    if (SAI_STATUS_SUCCESS != saiStatus)
    {
        XP_SAI_LOG_ERR("Could not store the statistics for the port %u queue %u.\n",
                       portNum, queueNum);
        return  saiStatus;
    }

    /* Clear specified stats in xpSai DB */
    saiStatus = xpSaiClearQueueStatsDB(number_of_counters, counter_ids,
                                       pQueueQosInfo);

    return saiStatus;
}
//Func: xpSaiGetQueueStatsInReadClearMode

static sai_status_t xpSaiGetQueueStatsInReadClearMode(sai_object_id_t queue_id,
                                                      uint32_t number_of_counters, const sai_stat_id_t *counter_ids,
                                                      uint64_t *counters, bool is_read_clear)
{
    xpSaiQueueQosInfo_t *pQueueQosInfo = NULL;
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;
    sai_uint32_t portNum;
    sai_uint32_t queueNum;
    GT_STATUS rc;

    if ((number_of_counters < 1) || (counter_ids == NULL) || (counters == NULL))
    {
        XP_SAI_LOG_ERR("%s invalid parameters received.\n", __FUNCNAME__);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(queue_id, SAI_OBJECT_TYPE_QUEUE))
    {
        XP_SAI_LOG_DBG("Wrong object type received (%u)\n",
                       xpSaiObjIdTypeGet(queue_id));
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    /* Get the port and queue from queue object */
    xpSaiQosQueueObjIdPortValueGet(queue_id, &portNum, &queueNum);

    xpsDevice_t xpsDevId = xpSaiObjIdSwitchGet(queue_id);

    saiStatus = xpSaiQueueStatisticsStoreDB(queue_id, &pQueueQosInfo);
    if (SAI_STATUS_SUCCESS != saiStatus)
    {
        XP_SAI_LOG_ERR("Could not store the statistics for the port %u queue %u.\n",
                       portNum, queueNum);
        return saiStatus;
    }

    for (uint32_t idx = 0; idx < number_of_counters; idx++)
    {
        if (queueNum >= XPS_MAX_QUEUES_PER_PORT)
        {
            /*queues 8-23  (UNICAST and MULTICAST) statistic is not supported ,just return zero*/
            counters[idx]=0;
            continue;
        }

        switch (counter_ids[idx])
        {
            /* get tx packets count */
            case SAI_QUEUE_STAT_PACKETS:
                {
                    counters[idx] = pQueueQosInfo->passPkts;
                    break;
                }
            /* get tx bytes count */
            case SAI_QUEUE_STAT_BYTES:
                {
                    counters[idx] = pQueueQosInfo->passBytes;
                    break;
                }
            /* get dropped packets count */
            case SAI_QUEUE_STAT_DROPPED_PACKETS:
                {
                    counters[idx] = pQueueQosInfo->dropPkts;
                    break;
                }
            /* get dropped bytes count */
            case SAI_QUEUE_STAT_DROPPED_BYTES:
                {
                    counters[idx] = pQueueQosInfo->dropBytes;
                    break;
                }
            /* get WRED dropped packets count */
            case SAI_QUEUE_STAT_WRED_DROPPED_PACKETS:
                {
                    counters[idx] = pQueueQosInfo->wredDropPkts;
                    break;
                }
            /* get/set WRED dropped bytes count */
            case SAI_QUEUE_STAT_WRED_DROPPED_BYTES:
                {
                    counters[idx] = pQueueQosInfo->wredDropBytes;
                    break;
                }
            case SAI_QUEUE_STAT_CURR_OCCUPANCY_BYTES:
                {
                    sai_uint32_t count = 0;

                    rc = cpssHalPortTx4TcBufNumberGet(xpsDevId, portNum, queueNum, &count);
                    if (rc != GT_OK)
                    {
                        XP_SAI_LOG_ERR("Failed to get (SAI_QUEUE_STAT_CURR_OCCUPANCY_BYTES) | retVal = %d\n",
                                       rc);
                        return cpssStatus2SaiStatus(rc);
                    }
                    counters[idx] = count * XPSAI_BUFFER_PAGE_SIZE_BYTES;
                    break;
                }
            /* get current queue shared occupancy in bytes */
            case SAI_QUEUE_STAT_SHARED_CURR_OCCUPANCY_BYTES:
                {
                    xpSaiBufferProfile_t *pProfileInfo  = NULL;
                    sai_uint32_t          count         = 0;

                    counters[idx] = 0;

                    rc = cpssHalPortTx4TcBufNumberGet(xpsDevId, portNum, queueNum, &count);
                    if (rc != GT_OK)
                    {
                        XP_SAI_LOG_ERR("Failed to get (SAI_QUEUE_STAT_CURR_OCCUPANCY_BYTES) | retVal = %d\n",
                                       rc);
                        return cpssStatus2SaiStatus(rc);
                    }

                    /* If have no buffer profile then use SAI_QUEUE_STAT_CURR_OCCUPANCY_BYTES */
                    if (pQueueQosInfo->bufferProfileId == SAI_NULL_OBJECT_ID)
                    {
                        counters[idx] = count * XPSAI_BUFFER_PAGE_SIZE_BYTES;
                        break;
                    }

                    /* Get the buffer profile state data */
                    saiStatus = xpSaiBufferProfileGetStateData(pQueueQosInfo->bufferProfileId,
                                                               &pProfileInfo);
                    if (saiStatus != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Error: Failed to get buffer profile state data, saiStatus = %d\n",
                                       saiStatus);
                    }
                    else if ((count * XPSAI_BUFFER_PAGE_SIZE_BYTES) > pProfileInfo->bufferSize)
                    {
                        /* Current shared occupancy = current occupancy - reserved/guaranteed buffer size */
                        counters[idx] = ((count * XPSAI_BUFFER_PAGE_SIZE_BYTES) -
                                         pProfileInfo->bufferSize);
                    }
                    break;
                }
            /* get watermark queue occupancy in bytes */
            case SAI_QUEUE_STAT_WATERMARK_BYTES:
                {
                    counters[idx] = pQueueQosInfo->watermark * XPSAI_BUFFER_PAGE_SIZE_BYTES;
                    break;
                }
            /* get watermark queue shared occupancy in bytes */
            case SAI_QUEUE_STAT_SHARED_WATERMARK_BYTES:
                {
                    xpSaiBufferProfile_t *pProfileInfo  = NULL;

                    counters[idx] = 0;

                    if (pQueueQosInfo->bufferProfileId == SAI_NULL_OBJECT_ID)
                    {
                        counters[idx] = pQueueQosInfo->watermark * XPSAI_BUFFER_PAGE_SIZE_BYTES;
                        break;
                    }

                    /* Get the buffer profile state data */
                    saiStatus = xpSaiBufferProfileGetStateData(pQueueQosInfo->bufferProfileId,
                                                               &pProfileInfo);
                    if (saiStatus != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Error: Failed to get buffer profile state data, saiStatus = %d\n",
                                       saiStatus);
                    }
                    else if ((pQueueQosInfo->watermark * XPSAI_BUFFER_PAGE_SIZE_BYTES) >
                             pProfileInfo->bufferSize)
                    {
                        /* Current shared occupancy = current occupancy - reserved/guaranteed buffer size */
                        counters[idx] = ((pQueueQosInfo->watermark * XPSAI_BUFFER_PAGE_SIZE_BYTES) -
                                         pProfileInfo->bufferSize);
                    }
                    break;
                }
            /* get green color tx packets coun */
            case SAI_QUEUE_STAT_GREEN_PACKETS:
            /* get green color tx bytes count */
            case SAI_QUEUE_STAT_GREEN_BYTES:
            /* get green color dropped packets count */
            case SAI_QUEUE_STAT_GREEN_DROPPED_PACKETS:
            /* get green color dropped packets count */
            case SAI_QUEUE_STAT_GREEN_DROPPED_BYTES:
            /* get yellow color tx packets count */
            case SAI_QUEUE_STAT_YELLOW_PACKETS:
            /* get yellow color tx bytes count */
            case SAI_QUEUE_STAT_YELLOW_BYTES:
            /* get yellow color drooped packets count */
            case SAI_QUEUE_STAT_YELLOW_DROPPED_PACKETS:
            /* get yellow color dropped bytes count */
            case SAI_QUEUE_STAT_YELLOW_DROPPED_BYTES:
            /* get red color tx packets count */
            case SAI_QUEUE_STAT_RED_PACKETS:
            /* get red color tx bytes count */
            case SAI_QUEUE_STAT_RED_BYTES:
            /* get red color dropped packets count */
            case SAI_QUEUE_STAT_RED_DROPPED_PACKETS:
            /* get red color drooped bytes count */
            case SAI_QUEUE_STAT_RED_DROPPED_BYTES:
            /* get WRED green color dropped packets count */
            case SAI_QUEUE_STAT_GREEN_WRED_DROPPED_PACKETS:
            /* get WRED green color dropped bytes count */
            case SAI_QUEUE_STAT_GREEN_WRED_DROPPED_BYTES:
            /* get WRED yellow color dropped packets count */
            case SAI_QUEUE_STAT_YELLOW_WRED_DROPPED_PACKETS:
            /* get WRED yellow color dropped bytes count */
            case SAI_QUEUE_STAT_YELLOW_WRED_DROPPED_BYTES:
            /* get WRED red color dropped packets count */
            case SAI_QUEUE_STAT_RED_WRED_DROPPED_PACKETS:
            /* get WRED red color dropped bytes count */
            case SAI_QUEUE_STAT_RED_WRED_DROPPED_BYTES:
                {
                    XP_SAI_LOG_DBG("Attribute %d not supported\n", counter_ids[idx]);
                    //saiStatus = SAI_STATUS_ATTR_NOT_SUPPORTED_0 + SAI_STATUS_CODE(idx);
                    counters[idx] = 0;
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Undefined attribute %d\n", counter_ids[idx]);
                    counters[idx] = 0;
                    // saiStatus = SAI_STATUS_UNKNOWN_ATTRIBUTE_0 + SAI_STATUS_CODE(idx);
                }
        }

        XP_SAI_LOG_DBG("xpSaiGetQueueStats port %d queueNum %d counter_ids[%d]=%d value 0x%08x%08x\n",
                       portNum, queueNum, idx,
                       counter_ids[idx], LONG_INT_FORMAT(counters[idx]));

    }

    if (is_read_clear)
    {
        xpSaiClearQueueStatsDB(number_of_counters, counter_ids, pQueueQosInfo);
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetQueueStats

sai_status_t xpSaiGetQueueStats(sai_object_id_t queue_id,
                                uint32_t number_of_counters, const sai_stat_id_t *counter_ids,
                                uint64_t *counters)
{
    return xpSaiGetQueueStatsInReadClearMode(queue_id, number_of_counters,
                                             counter_ids, counters, false);
}

//Func: xpSaiGetQueueStatsExt

sai_status_t xpSaiGetQueueStatsExt(sai_object_id_t queue_id,
                                   uint32_t number_of_counters, const sai_stat_id_t *counter_ids,
                                   sai_stats_mode_t mode, uint64_t *counters)
{
    return xpSaiGetQueueStatsInReadClearMode(queue_id, number_of_counters,
                                             counter_ids, counters, (mode == SAI_STATS_MODE_READ_AND_CLEAR));
}

//Func: xpSaiClearQueueStats

sai_status_t xpSaiClearQueueStats(sai_object_id_t queue_id,
                                  uint32_t number_of_counters, const sai_stat_id_t *counter_ids)
{
    sai_uint32_t portNum   = 0, qNum = 0, wrap = 0;
    sai_uint64_t count     = 0;
    XP_STATUS    retVal    = XP_NO_ERR;
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;
    GT_STATUS    status;
    GT_BOOL      enable    = GT_FALSE;

    if ((number_of_counters < 1) || (!counter_ids))
    {
        XP_SAI_LOG_ERR("%s invalid parameters received.\n", __FUNCNAME__);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(queue_id, SAI_OBJECT_TYPE_QUEUE))
    {
        XP_SAI_LOG_DBG("Wrong object type received (%u)\n",
                       xpSaiObjIdTypeGet(queue_id));
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    xpSaiQosQueueObjIdPortValueGet(queue_id, &portNum, &qNum);

    xpsDevice_t  xpDevId = xpSaiObjIdSwitchGet(queue_id);

    status = cpssHalCncCounterClearByReadEnableGet(xpDevId, &enable);
    if (status == GT_OK && enable == GT_TRUE)
    {
        return xpSaiClearQueueStatsInReadClearMode(queue_id, number_of_counters,
                                                   counter_ids);
    }

    for (uint32_t idx = 0; idx < number_of_counters; idx++)
    {
        switch (counter_ids[idx])
        {
            /* clear tx packet count */
            case SAI_QUEUE_STAT_PACKETS:
                {
                    retVal = xpsQosQcEnableFwdPktCountClearOnRead(xpDevId, 1);
                    if (retVal != XP_NO_ERR)
                    {
                        XP_SAI_LOG_ERR("Error: Couldn't enable fwd pkt count clear on read | retVal = %d\n",
                                       retVal);
                        saiStatus = xpsStatus2SaiStatus(retVal);
                    }

                    retVal = xpsQosQcGetQueueFwdPacketCountForPort(xpDevId, portNum, qNum, &count,
                                                                   &wrap);
                    if (retVal != XP_NO_ERR)
                    {
                        XP_SAI_LOG_ERR("Error: Couldn't get queue fwd packet count for port | retVal = %d\n",
                                       retVal);
                        saiStatus = xpsStatus2SaiStatus(retVal);
                    }

                    retVal = xpsQosQcEnableFwdPktCountClearOnRead(xpDevId, 0);
                    if (retVal != XP_NO_ERR)
                    {
                        XP_SAI_LOG_ERR("Error: Couldn't disable fwd pkt count clear on read | retVal = %d\n",
                                       retVal);
                        saiStatus = xpsStatus2SaiStatus(retVal);
                    }
                    break;
                }
            /* clear tx byte count */
            case SAI_QUEUE_STAT_BYTES:
                {
                    retVal = xpsQosQcEnableFwdPktByteCountClearOnRead(xpDevId, 1);
                    if (retVal != XP_NO_ERR)
                    {
                        XP_SAI_LOG_ERR("Error: Couldn't enable fwd pkt byte count clear on read | retVal = %d\n",
                                       retVal);
                        saiStatus = xpsStatus2SaiStatus(retVal);
                    }

                    retVal = xpsQosQcGetQueueFwdByteCountForPort(xpDevId, portNum, qNum, &count,
                                                                 &wrap);
                    if (retVal != XP_NO_ERR)
                    {
                        XP_SAI_LOG_ERR("Error: Couldn't get queue fwd byte count for port | retVal = %d\n",
                                       retVal);
                        saiStatus = xpsStatus2SaiStatus(retVal);
                    }

                    retVal = xpsQosQcEnableFwdPktByteCountClearOnRead(xpDevId, 0);
                    if (retVal != XP_NO_ERR)
                    {
                        XP_SAI_LOG_ERR("Error: Couldn't enable fwd pkt byte count clear on read | retVal = %d\n",
                                       retVal);
                        saiStatus = xpsStatus2SaiStatus(retVal);
                    }
                    break;
                }
            /* clear dropped packet count */
            case SAI_QUEUE_STAT_DROPPED_PACKETS:
                {
                    retVal = xpsQosQcEnableDropPktCountClearOnRead(xpDevId, 1);
                    if (retVal != XP_NO_ERR)
                    {
                        XP_SAI_LOG_ERR("Error: Couldn't enable drop pkt count clear on read | retVal = %d\n",
                                       retVal);
                        saiStatus = xpsStatus2SaiStatus(retVal);
                    }

                    retVal = xpsQosQcGetQueueDropPacketCountForPort(xpDevId, portNum, qNum, &count,
                                                                    &wrap);
                    if (retVal != XP_NO_ERR)
                    {
                        XP_SAI_LOG_ERR("Error: Couldn't get queue drop packet count for port | retVal = %d\n",
                                       retVal);
                        saiStatus = xpsStatus2SaiStatus(retVal);
                    }

                    retVal = xpsQosQcEnableDropPktCountClearOnRead(xpDevId, 0);
                    if (retVal != XP_NO_ERR)
                    {
                        XP_SAI_LOG_ERR("Error: Couldn't enable drop pkt count clear on read | retVal = %d\n",
                                       retVal);
                        saiStatus = xpsStatus2SaiStatus(retVal);
                    }
                    break;
                }
            /* clear dropped byte count */
            case SAI_QUEUE_STAT_DROPPED_BYTES:
                {
                    retVal = xpsQosQcEnableDropPktByteCountClearOnRead(xpDevId, 1);
                    if (retVal != XP_NO_ERR)
                    {
                        XP_SAI_LOG_ERR("Error: Couldn't enable drop pkt byte count clear on read | retVal = %d\n",
                                       retVal);
                        saiStatus = xpsStatus2SaiStatus(retVal);
                    }

                    retVal = xpsQosQcGetQueueDropByteCountForPort(xpDevId, portNum, qNum, &count,
                                                                  &wrap);
                    if (retVal != XP_NO_ERR)
                    {
                        XP_SAI_LOG_ERR("Error: Couldn't get queue drop packet byte count for port | retVal = %d\n",
                                       retVal);
                        saiStatus = xpsStatus2SaiStatus(retVal);
                    }

                    retVal = xpsQosQcEnableDropPktByteCountClearOnRead(xpDevId, 0);
                    if (retVal != XP_NO_ERR)
                    {
                        XP_SAI_LOG_ERR("Error: Couldn't enable drop pkt byte count clear on read | retVal = %d\n",
                                       retVal);
                        saiStatus = xpsStatus2SaiStatus(retVal);
                    }
                    break;
                }
            /* clear green color tx packets count */
            case SAI_QUEUE_STAT_GREEN_PACKETS:
            /* clear green color tx bytes count */
            case SAI_QUEUE_STAT_GREEN_BYTES:
            /* clear green color dropped packets count */
            case SAI_QUEUE_STAT_GREEN_DROPPED_PACKETS:
            /* clear green color dropped packets count */
            case SAI_QUEUE_STAT_GREEN_DROPPED_BYTES:
            /* clear yellow color tx packets count */
            case SAI_QUEUE_STAT_YELLOW_PACKETS:
            /* clear yellow color tx bytes count */
            case SAI_QUEUE_STAT_YELLOW_BYTES:
            /* clear yellow color drooped packets count */
            case SAI_QUEUE_STAT_YELLOW_DROPPED_PACKETS:
            /* clear yellow color dropped bytes count */
            case SAI_QUEUE_STAT_YELLOW_DROPPED_BYTES:
            /* clear red color tx packets count */
            case SAI_QUEUE_STAT_RED_PACKETS:
            /* clear red color tx bytes count */
            case SAI_QUEUE_STAT_RED_BYTES:
            /* clear red color dropped packets count */
            case SAI_QUEUE_STAT_RED_DROPPED_PACKETS:
            /* clear red color drooped bytes count */
            case SAI_QUEUE_STAT_RED_DROPPED_BYTES:
            /* clear WRED green color dropped packets count */
            case SAI_QUEUE_STAT_GREEN_WRED_DROPPED_PACKETS:
            /* clear WRED green color dropped bytes count */
            case SAI_QUEUE_STAT_GREEN_WRED_DROPPED_BYTES:
            /* clear WRED yellow color dropped packets count */
            case SAI_QUEUE_STAT_YELLOW_WRED_DROPPED_PACKETS:
            /* clear WRED yellow color dropped bytes count */
            case SAI_QUEUE_STAT_YELLOW_WRED_DROPPED_BYTES:
            /* clear WRED red color dropped packets count */
            case SAI_QUEUE_STAT_RED_WRED_DROPPED_PACKETS:
            /* clear WRED red color dropped bytes count */
            case SAI_QUEUE_STAT_RED_WRED_DROPPED_BYTES:
            /* clear WRED dropped packets count */
            case SAI_QUEUE_STAT_WRED_DROPPED_PACKETS:
            /* clear WRED dropped bytes count */
            case SAI_QUEUE_STAT_WRED_DROPPED_BYTES:
            /* clear current queue occupancy in bytes */
            case SAI_QUEUE_STAT_CURR_OCCUPANCY_BYTES:
            /* clear watermark queue occupancy in bytes */
            case SAI_QUEUE_STAT_WATERMARK_BYTES:
            /* clear current queue shared occupancy in bytes */
            case SAI_QUEUE_STAT_SHARED_CURR_OCCUPANCY_BYTES:
            /* clear watermark queue shared occupancy in bytes */
            case SAI_QUEUE_STAT_SHARED_WATERMARK_BYTES:
                {
                    XP_SAI_LOG_DBG("Attribute %d not supported\n", counter_ids[idx]);
                    //saiStatus = SAI_STATUS_ATTR_NOT_SUPPORTED_0 + SAI_STATUS_CODE(idx);
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Undefined attribute %d\n", counter_ids[idx]);
                    //saiStatus = SAI_STATUS_UNKNOWN_ATTRIBUTE_0 + SAI_STATUS_CODE(idx);
                }
        }
    }
    saiStatus = SAI_STATUS_SUCCESS;
    return saiStatus;
}

//Func: xpSaiQueueTxSchedulerProfileInit

sai_status_t xpSaiQueueTxSchedulerProfileInit(xpsDevice_t xpSaiDevId,
                                              sai_uint32_t xpPort)
{
    GT_STATUS    rc         = GT_OK;
    sai_status_t saiStatus  = SAI_STATUS_SUCCESS;
    uint32_t     profileIdx = 0;

    rc = cpssHalBindPortToSchedulerProfileGet(xpSaiDevId, xpPort, &profileIdx);
    if (rc != GT_OK)
    {
        XP_SAI_LOG_ERR("Error: Could not get scheduler profile for port %d | retVal:%d",
                       xpPort, rc);
        return cpssStatus2SaiStatus(rc);
    }

    saiStatus = xpSaiProfileMgrBindProfileToPort(schedulerProfileList[xpSaiDevId],
                                                 profileIdx, xpPort);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    return saiStatus;
}

//Func: xpSaiQueueInit

XP_STATUS xpSaiQueueInit(xpsDevice_t xpSaiDevId)
{
    XP_STATUS retVal = XP_NO_ERR;
    GT_STATUS rc = GT_OK;;
    xpsDevType_t devType = XP_DEV_TYPE_TOTAL;
    uint32_t  idx, qNum;

    // Register a global sai queue database
    if ((retVal = xpsStateRegisterDb(XP_SCOPE_DEFAULT, "Sai Queue Db", XPS_GLOBAL,
                                     &xpSaiQueueKeyComp, queueQosDbHandle)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Couldn't register sai queue state database | retVal: %d\n",
                       retVal);
        return retVal;
    }

    retVal = xpSalGetDeviceType(xpSaiDevId, &devType);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to retrieve devType from devId %d!\n", xpSaiDevId);
        return retVal;
    }

    /* Allocate persistent memory for scheduler profile list */
    schedulerProfileList[xpSaiDevId] = (xpSaiPortProfile_t*)
                                       xpsAllocatorIdBasedPersistentMalloc(XP_SCOPE_DEFAULT,
                                                                           XPS_MEM_POOL_ID_START,
                                                                           XPSAI_SCHEDULER_MAX_PROFILES * sizeof(xpSaiPortProfile_t));
    if (!schedulerProfileList[xpSaiDevId])
    {
        XP_SAI_LOG_ERR("Error: Failed to allocate persistent memory block for scheduler profile list\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }

    for (idx = 0; idx < XPSAI_SCHEDULER_MAX_PROFILES; idx++)
    {
        for (qNum = 0; qNum < XPSAI_MAX_TC_VALUE; qNum++)
        {
            schedulerProfileList[xpSaiDevId][idx].profileObjId[qNum] = SAI_NULL_OBJECT_ID;
        }
        schedulerProfileList[xpSaiDevId][idx].refCount = 0;
    }

    if ((rc=cpssHalCncQueueStatisticInit(xpSaiDevId))!= GT_OK)
    {
        XP_SAI_LOG_ERR("Failed to initialize queue statistics, xpStatus: %d\n", rc);
        return xpsConvertCpssStatusToXPStatus(rc);
    }

    return XP_NO_ERR;
}

//Func: xpSaiQueueDeInit

XP_STATUS xpSaiQueueDeInit(xpsDevice_t xpSaiDevId)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpsDevType_t devType = XP_DEV_TYPE_TOTAL;

    if ((retVal = xpsStateDeRegisterDb(XP_SCOPE_DEFAULT,
                                       &queueQosDbHandle)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Couldn't de-register sai queue state database | retVal: %d\n",
                       retVal);
        return retVal;
    }

    retVal = xpSalGetDeviceType(xpSaiDevId, &devType);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to retrieve devType from devId %d!\n", xpSaiDevId);
        return retVal;
    }

    /* Free persistent memory allocated for scheduler profile list */
    xpsAllocatorIdBasedPersistentFree(schedulerProfileList[xpSaiDevId]);

    return XP_NO_ERR;
}

//Func: xpSaiQueueApiInit

XP_STATUS xpSaiQueueApiInit(uint64_t flag,
                            const sai_service_method_table_t* adapHostServiceMethodTable)
{
    XP_STATUS retVal = XP_NO_ERR;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Calling xpSaiQueueApiInit\n");

    _xpSaiQueueApi = (sai_queue_api_t *) xpMalloc(sizeof(sai_queue_api_t));
    if (NULL == _xpSaiQueueApi)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for _xpSaiQueueApi\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }

    _xpSaiQueueApi->create_queue        = xpSaiCreateQueue;
    _xpSaiQueueApi->remove_queue        = xpSaiRemoveQueue;
    _xpSaiQueueApi->set_queue_attribute = xpSaiSetQueueAttribute;
    _xpSaiQueueApi->get_queue_attribute = xpSaiGetQueueAttributes;
    _xpSaiQueueApi->get_queue_stats     = xpSaiGetQueueStats;
    _xpSaiQueueApi->get_queue_stats_ext = xpSaiGetQueueStatsExt;
    _xpSaiQueueApi->clear_queue_stats   = xpSaiClearQueueStats;

    saiRetVal = xpSaiApiRegister(SAI_API_QUEUE, (void*)_xpSaiQueueApi);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Failed to register queue API\n");
        return XP_ERR_ARRAY_OUT_OF_BOUNDS;
    }

    return retVal;
}

//Func: xpSaiQueueApiDeinit

XP_STATUS xpSaiQueueApiDeinit()
{
    XP_STATUS retVal = XP_NO_ERR;

    XP_SAI_LOG_DBG("Calling xpSaiQueueApiDeinit\n");

    xpFree(_xpSaiQueueApi);
    _xpSaiQueueApi = NULL;

    return  retVal;
}

sai_status_t xpSaiMaxCountQueueAttribute(uint32_t *count)
{
    if (!count)
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }
    *count = SAI_QUEUE_ATTR_END;

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiCountQueueObjects(uint32_t *count)
{
    XP_STATUS  retVal  = XP_NO_ERR;

    if (!count)
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpsStateGetCount(XP_SCOPE_DEFAULT, queueQosDbHandle, count);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get Count. return Value : %d\n", retVal);
    }
    return xpsStatus2SaiStatus(retVal);
}

sai_status_t xpSaiGetQueueObjectList(uint32_t *object_count,
                                     sai_object_key_t *object_list)
{
    XP_STATUS       retVal      = XP_NO_ERR;
    sai_status_t    saiRetVal   = SAI_STATUS_SUCCESS;
    uint32_t        objCount    = 0;

    xpSaiQueueQosInfo_t   *pQueueNext = NULL;

    saiRetVal = xpSaiCountQueueObjects(&objCount);
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
        retVal = xpsStateGetNextData(XP_SCOPE_DEFAULT, queueQosDbHandle, pQueueNext,
                                     (void **)&pQueueNext);
        if (retVal != XP_NO_ERR || pQueueNext == NULL)
        {
            XP_SAI_LOG_ERR("Failed to retrieve queue object, error %d\n", retVal);
            return SAI_STATUS_FAILURE;
        }

        object_list[i].key.object_id = pQueueNext->queueId;
    }

    return SAI_STATUS_SUCCESS;
}

static sai_status_t xpSaiQueueIsWredConfigured(sai_object_id_t queueObjId,
                                               bool *wredConfiguredPtr)
{
    sai_status_t saiRetVal;
    sai_attribute_value_t value;

    saiRetVal = xpSaiGetQueueAttrWredProfileId(queueObjId, &value);
    if (SAI_STATUS_SUCCESS != saiRetVal)
    {
        XP_SAI_LOG_ERR("Failed to get (SAI_QUEUE_ATTR_WRED_PROFILE_ID)\n");
        return saiRetVal;
    }

    if (value.oid==SAI_NULL_OBJECT_ID)
    {
        *wredConfiguredPtr = false;
    }
    else
    {
        *wredConfiguredPtr = true;
    }

    return SAI_STATUS_SUCCESS;

}

static sai_status_t xpSaiQueueIsBufferConfigured(sai_object_id_t queueObjId,
                                                 bool *bufferConfiguredPtr,
                                                 sai_uint32_t *sizePtrInPages)
{
    sai_status_t saiRetVal;
    sai_attribute_value_t value;
    xpSaiBufferProfile_t *pProfileInfo;

    saiRetVal = xpSaiGetQueueAttrBufferProfileId(queueObjId, &value);
    if (SAI_STATUS_SUCCESS != saiRetVal)
    {
        XP_SAI_LOG_ERR("Failed to get (SAI_QUEUE_ATTR_WRED_PROFILE_ID)\n");
        return saiRetVal;
    }

    if (value.oid==SAI_NULL_OBJECT_ID)
    {
        *bufferConfiguredPtr = false;
    }
    else
    {
        *bufferConfiguredPtr = true;


        /* Get the buffer profile state data */
        saiRetVal = xpSaiBufferProfileGetStateData(value.oid, &pProfileInfo);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            return saiRetVal;
        }
        *sizePtrInPages = XPSAI_BYTES_TO_PAGES(pProfileInfo->bufferSize);
    }

    return SAI_STATUS_SUCCESS;

}
