// xpSaiQosMap.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpSaiQosMap.h"
#include "xpSaiValidationArrays.h"
#include "cpssHalQos.h"

#define XP_SAI_QOS_MAP_MAX_IDS     256
#define XP_SAI_QOS_MAP_RANGE_START 1

XP_SAI_LOG_REGISTER_API(SAI_API_QOS_MAP);

sai_qos_map_api_t* _xpSaiQosMapApi;

typedef struct xpSaiPortQosProfileMap_t
{
    sai_object_id_t dot1pToTcObjId;
    sai_object_id_t dot1pToDpObjId;
    sai_object_id_t dscpToTcObjId;
    sai_object_id_t dscpToDpObjId;
    sai_object_id_t tcDpToDot1pObjId;
    sai_object_id_t tcDpToDscpObjId;
    sai_object_id_t portObjIdList[SYSTEM_MAX_PORT];
    sai_uint32_t    refCount;
} xpSaiPortQosProfileMap_t;

typedef struct xpSaiPortQosProfileList_t
{
    xpSaiPortQosProfileMap_t ingressProfileList[XPSAI_QOSMAP_MAX_QOSMAP_PROFILES];
    xpSaiPortQosProfileMap_t egressProfileList[XPSAI_QOSMAP_MAX_QOSMAP_PROFILES];
} xpSaiPortQosProfileList_t;

typedef struct xpSaiL2QosMap_t
{
    sai_uint8_t        dot1p;
    sai_uint8_t        tc;
    sai_packet_color_t dp;
} xpSaiL2QosMap_t;

typedef struct xpSaiL3QosMap_t
{
    sai_uint8_t        dscp;
    sai_uint8_t        tc;
    sai_packet_color_t dp;
} xpSaiL3QosMap_t;

typedef struct xpSaiTcToQueueMap_t
{
    sai_uint8_t tc;
    sai_uint8_t queue;
} xpSaiTcToQueueMap_t;

typedef struct xpSaiPfcPrioToPgMap_t
{
    sai_uint8_t pfcPriority;
    sai_uint8_t pg;
} xpSaiPfcPrioToPgMap_t;

typedef struct xpSaiPfcPrioToQueueMap_t
{
    sai_uint8_t pfcPriority;
    sai_uint8_t queue;
} xpSaiPfcPrioToQueueMap_t;

typedef struct _xpSaiQosMapAttributesT
{
    sai_attribute_value_t type;
    sai_attribute_value_t mapToValueList;
} xpSaiQosMapAttributesT;

// Qos Map DB handler
static xpsDbHandle_t xpSaiQosMapDbHndl = XPSAI_QOS_MAP_DB_HNDL;

static xpSaiPortQosProfileList_t* gPortQosProfileList[XP_MAX_DEVICES];

static sai_object_id_t globalTctoQueueMapId = 0;

//Func: xpSaiQosMapKeyComp

static sai_int32_t xpSaiQosMapKeyComp(void *key1, void *key2)
{
    sai_int64_t val1 = ((xpSaiQosMap_t*)key1)->qosmapObjId;
    sai_int64_t val2 = ((xpSaiQosMap_t*)key2)->qosmapObjId;

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

#if 0

//Func: xpSaiQosMapUpdatePfcPriorityToQueue

void xpSaiQosMapUpdatePfcPriorityToQueue(xpsDevice_t xpDevId, xpsPort_t portNum,
                                         uint32_t queueNum, uint8_t priority)
{
    sai_uint32_t priorityInHw = 0;
    XP_STATUS    xpStatus     = XP_NO_ERR;

    /* Fetch the pfc priority from HW for a given queue */
    xpStatus = xpsQosFcGetPfcPriority(xpDevId, portNum, queueNum, &priorityInHw);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get pfc priority for port %d, queue %d, xpStatus: %d\n",
                       portNum, queueNum, xpStatus);
        return;
    }

    if (priority != priorityInHw)
    {
        while (priority != priorityInHw)
        {
            usleep(100);
            xpStatus = xpsQosFcSetPfcPriority(xpDevId, portNum, queueNum, priority);
            if (xpStatus != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Failed to set pfc priority %d for port %d and queue %d, xpStatus: %d\n",
                               priority, portNum, queueNum, xpStatus);
                return;
            }

            xpStatus = xpsQosFcGetPfcPriority(xpDevId, portNum, queueNum, &priorityInHw);
            if (xpStatus != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Failed to get pfc priority for port %d, queue %d, xpStatus: %d\n",
                               portNum, queueNum, xpStatus);
                return;
            }
        }


    }

    return;
}

#endif
//Func: xpSaiQosMapInit

sai_status_t xpSaiQosMapInit(xpsDevice_t xpDevId)
{
    XP_STATUS    xpStatus  = XP_NO_ERR;

    xpStatus = xpsAllocatorInitIdAllocator(XP_SCOPE_DEFAULT, XP_SAI_ALLOC_QOS_MAP,
                                           XP_SAI_QOS_MAP_MAX_IDS, XP_SAI_QOS_MAP_RANGE_START);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to initialize SAI QoS map allocator\n");
        return xpsStatus2SaiStatus(xpStatus);
    }

    /* Register global qos map state database */
    xpStatus = xpsStateRegisterDb(XP_SCOPE_DEFAULT, "Sai QosMap DB", XPS_GLOBAL,
                                  &xpSaiQosMapKeyComp, xpSaiQosMapDbHndl);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to register sai qos map state database, xpStatus: %d\n",
                       xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    /* Allocate persistent memory for L2/L3 port QoS ingress profile list */
    gPortQosProfileList[xpDevId] = (xpSaiPortQosProfileList_t*)
                                   xpsAllocatorIdBasedPersistentMalloc(XP_SCOPE_DEFAULT,
                                                                       XP_SAI_MEM_POOL_QOS_ING_PORT_PFL,
                                                                       sizeof(xpSaiPortQosProfileList_t));
    if (!gPortQosProfileList[xpDevId])
    {
        XP_SAI_LOG_ERR("Error: Failed to allocate persistent memory block for L2/L3 port QoS ingress profile list\n");
        return SAI_STATUS_NO_MEMORY;
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiQosMapDeInit

sai_status_t xpSaiQosMapDeInit(xpsDevice_t xpDevId)
{
    XP_STATUS xpStatus = XP_NO_ERR;

    /* De-Register global qos map state database */
    xpStatus = xpsStateDeRegisterDb(XP_SCOPE_DEFAULT, &xpSaiQosMapDbHndl);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to deregister sai qos map state database, xpStatus: %d\n",
                       xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    /* Free persistent memory allocated for L2/L3 port QoS ingress profile list */
    xpsAllocatorIdBasedPersistentFree(gPortQosProfileList[xpDevId]);

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiQosMapCreateObject

static sai_status_t xpSaiQosMapCreateObject(xpsDevice_t xpDevId,
                                            sai_object_id_t *pQosMapObjId)
{
    sai_uint32_t qosmapId  = 0;
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;
    XP_STATUS    xpStatus  = XP_NO_ERR;

    /* Allocate qos map id (key) */
    xpStatus = xpsAllocatorAllocateId(XP_SCOPE_DEFAULT, XP_SAI_ALLOC_QOS_MAP,
                                      &qosmapId);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to allocate qos map id, xpStatus: %d\n", xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    /* Create qos map object */
    saiStatus = xpSaiObjIdCreate(SAI_OBJECT_TYPE_QOS_MAP, xpDevId,
                                 (sai_uint64_t)qosmapId, pQosMapObjId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Could not create qos map object, saiStatus: %d\n",
                       saiStatus);
        xpStatus = xpsAllocatorReleaseId(XP_SCOPE_DEFAULT, XP_SAI_ALLOC_QOS_MAP,
                                         qosmapId);
        if (xpStatus != XP_NO_ERR)
        {
            return xpsStatus2SaiStatus(xpStatus);
        }
        return xpStatus;
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiQosMapRemoveObject

static sai_status_t xpSaiQosMapRemoveObject(sai_object_id_t qosmapObjId)
{
    sai_uint32_t qosmapId = 0;
    XP_STATUS    xpStatus = XP_NO_ERR;

    /* Get the qos map id (key) from qos map object */
    qosmapId = (sai_uint32_t)xpSaiObjIdValueGet(qosmapObjId);

    /* Release the qos map id (key) */
    xpStatus = xpsAllocatorReleaseId(XP_SCOPE_DEFAULT, XP_SAI_ALLOC_QOS_MAP,
                                     qosmapId);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to release qos map id, xpStatus: %d\n", xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiQosMapAllocateMemoryForValueList

static sai_status_t xpSaiQosMapAllocateMemoryForValueList(int32_t mapType,
                                                          uint32_t *numEntries, void **ppMapList)
{
    XP_STATUS xpStatus = XP_NO_ERR;

    if ((numEntries == NULL) || (ppMapList == NULL))
    {
        XP_SAI_LOG_ERR("Null pointer provided!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    /* Allocate memory for map value list based on map type */
    switch (mapType)
    {
        case SAI_QOS_MAP_TYPE_DOT1P_TO_TC:
        case SAI_QOS_MAP_TYPE_DOT1P_TO_COLOR:
            {
                xpStatus = xpsStateHeapMalloc(sizeof(xpSaiL2QosMap_t) *
                                              XPSAI_QOSMAP_MAX_PCP_VALUE, (void**)ppMapList);
                if (xpStatus != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("Error: Couldn't allocate heap memory for map list, xpStatus: %d\n",
                                   xpStatus);
                    return xpsStatus2SaiStatus(xpStatus);
                }
                memset(*ppMapList, 0x0, sizeof(xpSaiL2QosMap_t) * XPSAI_QOSMAP_MAX_PCP_VALUE);

                *numEntries = XPSAI_QOSMAP_MAX_PCP_VALUE;
                break;
            }
        case SAI_QOS_MAP_TYPE_DSCP_TO_TC:
        case SAI_QOS_MAP_TYPE_DSCP_TO_COLOR:
            {
                xpStatus = xpsStateHeapMalloc(sizeof(xpSaiL3QosMap_t) *
                                              XPSAI_QOSMAP_MAX_DSCP_VALUE, (void**)ppMapList);
                if (xpStatus != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("Error: Couldn't allocate heap memory for map list, xpStatus: %d\n",
                                   xpStatus);
                    return xpsStatus2SaiStatus(xpStatus);
                }
                memset(*ppMapList, 0x0, sizeof(xpSaiL3QosMap_t) * XPSAI_QOSMAP_MAX_DSCP_VALUE);

                *numEntries = XPSAI_QOSMAP_MAX_DSCP_VALUE;
                break;
            }
        case SAI_QOS_MAP_TYPE_TC_TO_QUEUE:
            {
                xpStatus = xpsStateHeapMalloc(sizeof(xpSaiTcToQueueMap_t) *
                                              XPSAI_QOSMAP_MAX_TC_VALUE, (void**)ppMapList);
                if (xpStatus != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("Error: Couldn't allocate heap memory for map list, xpStatus: %d\n",
                                   xpStatus);
                    return xpsStatus2SaiStatus(xpStatus);
                }
                memset(*ppMapList, 0x0, sizeof(xpSaiTcToQueueMap_t) *
                       XPSAI_QOSMAP_MAX_TC_VALUE);

                *numEntries = XPSAI_QOSMAP_MAX_TC_VALUE;
                break;
            }
        case SAI_QOS_MAP_TYPE_TC_AND_COLOR_TO_DSCP:
            {
                xpStatus = xpsStateHeapMalloc(sizeof(xpSaiL3QosMap_t) *
                                              XPSAI_QOSMAP_MAX_DP_VALUE * XPSAI_QOSMAP_MAX_TC_VALUE, (void**)ppMapList);
                if (xpStatus != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("Error: Couldn't allocate heap memory for map list, xpStatus: %d\n",
                                   xpStatus);
                    return xpsStatus2SaiStatus(xpStatus);
                }
                memset(*ppMapList, 0x0, sizeof(xpSaiL3QosMap_t) * XPSAI_QOSMAP_MAX_DP_VALUE *
                       XPSAI_QOSMAP_MAX_TC_VALUE);

                *numEntries = XPSAI_QOSMAP_MAX_DP_VALUE * XPSAI_QOSMAP_MAX_TC_VALUE;
                break;
            }
        case SAI_QOS_MAP_TYPE_TC_AND_COLOR_TO_DOT1P:
            {
                xpStatus = xpsStateHeapMalloc(sizeof(xpSaiL2QosMap_t) *
                                              XPSAI_QOSMAP_MAX_DP_VALUE * XPSAI_QOSMAP_MAX_TC_VALUE, (void**)ppMapList);
                if (xpStatus != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("Error: Couldn't allocate heap memory for map list, xpStatus: %d\n",
                                   xpStatus);
                    return xpsStatus2SaiStatus(xpStatus);
                }
                memset(*ppMapList, 0x0, sizeof(xpSaiL2QosMap_t) * XPSAI_QOSMAP_MAX_DP_VALUE *
                       XPSAI_QOSMAP_MAX_TC_VALUE);

                *numEntries = XPSAI_QOSMAP_MAX_DP_VALUE * XPSAI_QOSMAP_MAX_TC_VALUE;
                break;
            }
        case SAI_QOS_MAP_TYPE_TC_TO_PRIORITY_GROUP:
            {
                xpStatus = xpsStateHeapMalloc(sizeof(xpSaiTcToPgMap_t) *
                                              XPSAI_QOSMAP_MAX_TC_VALUE, (void**)ppMapList);
                if (xpStatus != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("Error: Couldn't allocate heap memory for map list, xpStatus: %d\n",
                                   xpStatus);
                    return xpsStatus2SaiStatus(xpStatus);
                }
                memset(*ppMapList, 0x0, sizeof(xpSaiTcToPgMap_t) * XPSAI_QOSMAP_MAX_TC_VALUE);

                *numEntries = XPSAI_QOSMAP_MAX_TC_VALUE;
                break;
            }
        case SAI_QOS_MAP_TYPE_PFC_PRIORITY_TO_PRIORITY_GROUP:
            {
                xpStatus = xpsStateHeapMalloc(sizeof(xpSaiPfcPrioToPgMap_t) *
                                              XPSAI_QOSMAP_MAX_PRIORITY_VALUE, (void**)ppMapList);
                if (xpStatus != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("Error: Couldn't allocate heap memory for map list, xpStatus: %d\n",
                                   xpStatus);
                    return xpsStatus2SaiStatus(xpStatus);
                }
                memset(*ppMapList, 0x0, sizeof(xpSaiPfcPrioToPgMap_t) *
                       XPSAI_QOSMAP_MAX_PRIORITY_VALUE);

                *numEntries = XPSAI_QOSMAP_MAX_PRIORITY_VALUE;
                break;
            }
        case SAI_QOS_MAP_TYPE_PFC_PRIORITY_TO_QUEUE:
            {
                xpStatus = xpsStateHeapMalloc(sizeof(xpSaiPfcPrioToQueueMap_t) *
                                              XPSAI_QOSMAP_MAX_PRIORITY_VALUE, (void**)ppMapList);
                if (xpStatus != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("Error: Couldn't allocate heap memory for map list, xpStatus: %d\n",
                                   xpStatus);
                    return xpsStatus2SaiStatus(xpStatus);
                }
                memset(*ppMapList, 0x0, sizeof(xpSaiPfcPrioToQueueMap_t) *
                       XPSAI_QOSMAP_MAX_PRIORITY_VALUE);

                *numEntries = XPSAI_QOSMAP_MAX_PRIORITY_VALUE;
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Error: Couldn't allocate heap memory for map list, invalid map type: %d\n",
                               mapType);
                return SAI_STATUS_INVALID_ATTR_VALUE_0;
            }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiQosMapCreateStateData

static sai_status_t xpSaiQosMapCreateStateData(sai_object_id_t qosmapObjId,
                                               int32_t mapType, xpSaiQosMap_t **ppQosMapInfo)
{
    sai_pointer_t pMapList   = NULL;
    sai_status_t  saiStatus  = SAI_STATUS_SUCCESS;
    XP_STATUS     xpStatus   = XP_NO_ERR;
    uint32_t      numEntries = 0;

    /* Allocate memory for qos map */
    xpStatus = xpsStateHeapMalloc(sizeof(xpSaiQosMap_t), (void**)ppQosMapInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Couldn't allocate heap memory, xpStatus: %d\n",
                       xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }
    memset(*ppQosMapInfo, 0x0, sizeof(xpSaiQosMap_t));

    /* Allocate memory for qos map value list based on map type */
    saiStatus = xpSaiQosMapAllocateMemoryForValueList(mapType, &numEntries,
                                                      (void **)&pMapList);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        xpsStateHeapFree((void*)*ppQosMapInfo);
        return saiStatus;
    }

    /* Populate key value and map value list */
    (*ppQosMapInfo)->qosmapObjId = qosmapObjId;
    (*ppQosMapInfo)->mapList     = pMapList;
    (*ppQosMapInfo)->numEntries  = numEntries;

    /* Insert the state into database */
    xpStatus = xpsStateInsertData(XP_SCOPE_DEFAULT, xpSaiQosMapDbHndl,
                                  (void*)*ppQosMapInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Couldn't insert state into data base, xpStatus: %d\n",
                       xpStatus);
        xpsStateHeapFree((void*)pMapList);
        xpsStateHeapFree((void*)*ppQosMapInfo);
        return xpsStatus2SaiStatus(xpStatus);
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiQosMapRemoveStateData

static sai_status_t xpSaiQosMapRemoveStateData(sai_object_id_t qosmapObjId)
{
    xpSaiQosMap_t  *pQosMapInfo = NULL;
    xpSaiQosMap_t  *key = NULL;
    XP_STATUS       xpStatus    = XP_NO_ERR;

    key = (xpSaiQosMap_t *)xpMalloc(sizeof(xpSaiQosMap_t));
    if (!key)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for key\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }
    memset(key, 0, sizeof(xpSaiQosMap_t));

    /* Populate the key */
    key->qosmapObjId = qosmapObjId;

    /* Delete the corresponding state from state database */
    xpStatus = xpsStateDeleteData(XP_SCOPE_DEFAULT, xpSaiQosMapDbHndl,
                                  (xpsDbKey_t)key, (void**)&pQosMapInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Couldn't not delete qos map state data, xpStatus: %d\n",
                       xpStatus);
        xpFree(key);
        return xpsStatus2SaiStatus(xpStatus);
    }

    /* Free the memory allocated for map value list */
    if (pQosMapInfo && pQosMapInfo->mapList)
    {
        xpStatus = xpsStateHeapFree(pQosMapInfo->mapList);
        if (xpStatus != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error: Couldn't free the memory allocated for map list, xpStatus: %d\n",
                           xpStatus);
            xpFree(key);
            return xpsStatus2SaiStatus(xpStatus);
        }
        pQosMapInfo->mapList = NULL;
    }

    /* Free the memory allocated for the corresponding state */
    xpStatus = xpsStateHeapFree(pQosMapInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Couldn't free heap memory, xpStatus: %d\n", xpStatus);
        xpFree(key);
        return xpsStatus2SaiStatus(xpStatus);
    }

    xpFree(key);
    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSchedulerGroupGetStateData

sai_status_t xpSaiQosMapGetStateData(sai_object_id_t qosmapObjId,
                                     xpSaiQosMap_t **ppQosMapInfo)
{
    xpSaiQosMap_t *key = NULL;
    XP_STATUS     xpStatus = XP_NO_ERR;

    key = (xpSaiQosMap_t *)xpMalloc(sizeof(xpSaiQosMap_t));
    if (!key)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for key\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }
    memset(key, 0, sizeof(xpSaiQosMap_t));

    /* Populate the key */
    key->qosmapObjId = qosmapObjId;

    /* Retrieve the corresponding state from state database */
    xpStatus = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiQosMapDbHndl,
                                  (xpsDbKey_t)key, (void**)ppQosMapInfo);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error: Failed to retrieve the state data, xpStatus: %d\n",
                       xpStatus);
        xpFree(key);
        return xpsStatus2SaiStatus(xpStatus);
    }

    if (!(*ppQosMapInfo))
    {
        XP_SAI_LOG_ERR("Error: QoS map entry does not exist\n");
        xpFree(key);
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    xpFree(key);
    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiQosMapValidateQosMapObject

sai_status_t xpSaiQosMapValidateQosMapObject(sai_object_id_t qosmapObjId)
{
    xpSaiQosMap_t *pQosMapInfo = NULL;
    sai_status_t   saiStatus   = SAI_STATUS_SUCCESS;

    /* Validate non null Objects */
    if (qosmapObjId != SAI_NULL_OBJECT_ID)
    {
        /* Check if qosmap object is valid */
        if (!XDK_SAI_OBJID_TYPE_CHECK(qosmapObjId, SAI_OBJECT_TYPE_QOS_MAP))
        {
            XP_SAI_LOG_ERR("Wrong object type received (%u)\n",
                           xpSaiObjIdTypeGet(qosmapObjId));
            return SAI_STATUS_INVALID_OBJECT_TYPE;
        }

        /* Check if qos map object exist */
        saiStatus = xpSaiQosMapGetStateData(qosmapObjId, &pQosMapInfo);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Error: Qos map does not exist\n");
            return SAI_STATUS_INVALID_OBJECT_ID;
        }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiQosMapValidateQosMapType

sai_status_t xpSaiQosMapValidateQosMapType(sai_object_id_t qosmapObjId,
                                           sai_int32_t mapType)
{
    xpSaiQosMap_t *pQosMapInfo = NULL;
    sai_status_t   saiStatus   = SAI_STATUS_SUCCESS;

    /* Validate non null Objects */
    if (qosmapObjId != SAI_NULL_OBJECT_ID)
    {
        /* Check if qosmap object is valid */
        if (!XDK_SAI_OBJID_TYPE_CHECK(qosmapObjId, SAI_OBJECT_TYPE_QOS_MAP))
        {
            XP_SAI_LOG_ERR("Wrong object type received (%u)\n",
                           xpSaiObjIdTypeGet(qosmapObjId));
            return SAI_STATUS_INVALID_OBJECT_TYPE;
        }

        /* Check if qos map object exist */
        saiStatus = xpSaiQosMapGetStateData(qosmapObjId, &pQosMapInfo);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Error: Qos map does not exist\n");
            return SAI_STATUS_INVALID_OBJECT_ID;
        }

        if (mapType != pQosMapInfo->mapType)
        {
            XP_SAI_LOG_ERR("Error: Qos map type is different\n");
            return SAI_STATUS_INVALID_OBJECT_ID;
        }
    }

    return SAI_STATUS_SUCCESS;
}
//Func: xpSaiQosMapIsIbufferSupported

sai_uint32_t xpSaiQosMapIsIbufferSupported(xpsDevice_t xpDevId)
{
    if ((xpSaiSwitchDevTypeGet() == XP70) || (xpSaiSwitchDevTypeGet() == XP70A1) ||
        (xpSaiSwitchDevTypeGet() == XP70A2))
    {
        return 1;
    }

    return 0;
}

//Func: xpSaiQosMapSetTrafficClassForL2QosProfile

static sai_status_t xpSaiQosMapSetTrafficClassForL2QosProfile(
    xpsDevice_t xpDevId, uint32_t profileId, sai_object_id_t qosmapObjId)
{
    xpSaiQosMap_t *pQosMapInfo = NULL;
    sai_status_t   saiStatus   = SAI_STATUS_SUCCESS;
    GT_STATUS      rc          = GT_OK;

    /* Get the qos map state data */
    saiStatus = xpSaiQosMapGetStateData(qosmapObjId, &pQosMapInfo);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    xpSaiL2QosMap_t *pL2QosMap = (xpSaiL2QosMap_t*)pQosMapInfo->mapList;
    sai_uint32_t pcp = 0, tc = 0;

    for (uint32_t count = 0; count < pQosMapInfo->numEntries; count++)
    {
        pcp = pL2QosMap[count].dot1p;  /* Key */
        tc  = pL2QosMap[count].tc;     /* Value */

        for (uint32_t dei = 0; dei < XPSAI_QOSMAP_MAX_DEI_VALUE; dei++)
        {
            /* Set the traffic class for given dot1p */
            rc = cpssHalQoSDot1pToTcMapSet(xpDevId, profileId, pcp, dei, tc);
            if (rc != GT_OK)
            {
                XP_SAI_LOG_ERR("Error: Failed to set tc for L2 QoS profile, rc: %d\n", rc);
                return cpssStatus2SaiStatus(rc);
            }
        }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiQosMapSetColorForL2QosProfile

static sai_status_t xpSaiQosMapSetColorForL2QosProfile(xpsDevice_t xpDevId,
                                                       uint32_t profileId, sai_object_id_t qosmapObjId)
{
    xpSaiQosMap_t *pQosMapInfo = NULL;
    sai_status_t   saiStatus   = SAI_STATUS_SUCCESS;
    GT_STATUS      rc          = GT_OK;

    /* Get the qos map state data */
    saiStatus = xpSaiQosMapGetStateData(qosmapObjId, &pQosMapInfo);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    xpSaiL2QosMap_t *pL2QosMap = (xpSaiL2QosMap_t*)pQosMapInfo->mapList;
    sai_uint32_t pcp = 0, dp = 0;

    for (uint32_t count = 0; count < pQosMapInfo->numEntries; count++)
    {
        pcp = pL2QosMap[count].dot1p;  /* Key */
        dp  = pL2QosMap[count].dp;     /* Value */

        for (uint32_t dei = 0; dei < XPSAI_QOSMAP_MAX_DEI_VALUE; dei++)
        {
            /* Set dot1p to default color */
            rc = cpssHalQoSDot1pToDpMapSet(xpDevId, profileId, pcp, dei, dp);
            if (rc != GT_OK)
            {
                XP_SAI_LOG_ERR("Error: Failed to set dot1p to dp map, rc: %d\n", rc);
                return cpssStatus2SaiStatus(rc);
            }
        }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiQosMapSetTrafficClassForL3QosProfile

sai_status_t xpSaiQosMapSetTrafficClassForL3QosProfile(xpsDevice_t xpDevId,
                                                       uint32_t profileId, sai_object_id_t qosmapObjId, bool isClear)
{
    xpSaiQosMap_t *pQosMapInfo = NULL;
    sai_status_t   saiStatus   = SAI_STATUS_SUCCESS;
    GT_STATUS      rc          = GT_OK;

    /* Get the qos map state data */
    saiStatus = xpSaiQosMapGetStateData(qosmapObjId, &pQosMapInfo);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    xpSaiL3QosMap_t *pL3QosMap = (xpSaiL3QosMap_t*)pQosMapInfo->mapList;
    sai_uint32_t dscp = 0, tc = 0;

    for (uint32_t count = 0; count < pQosMapInfo->numEntries; count++)
    {
        dscp = pL3QosMap[count].dscp;  /* Key */

        if (isClear)
        {
            tc = 0;
        }
        else
        {
            tc   = pL3QosMap[count].tc;    /* Value */
        }

        /* Set the traffic class for given dscp */
        rc = cpssHalQoSDscpToTcMapSet(xpDevId, profileId, dscp, tc);
        if (rc != GT_OK)
        {
            XP_SAI_LOG_ERR("Error: Failed to set tc for L3 QoS profile, rc: %d\n", rc);
            return cpssStatus2SaiStatus(rc);
        }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiQosMapSetColorForL3QosProfile

sai_status_t xpSaiQosMapSetColorForL3QosProfile(xpsDevice_t xpDevId,
                                                uint32_t profileId, sai_object_id_t qosmapObjId, bool isClear)
{
    xpSaiQosMap_t *pQosMapInfo = NULL;
    sai_status_t   saiStatus   = SAI_STATUS_SUCCESS;
    GT_STATUS      rc          = GT_OK;

    /* Get the qos map state data */
    saiStatus = xpSaiQosMapGetStateData(qosmapObjId, &pQosMapInfo);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    xpSaiL3QosMap_t *pL3QosMap = (xpSaiL3QosMap_t*)pQosMapInfo->mapList;
    sai_uint32_t dscp = 0, dp = 0;

    for (uint32_t count = 0; count < pQosMapInfo->numEntries; count++)
    {
        dscp = pL3QosMap[count].dscp;  /* Key */

        if (isClear)
        {
            dp = 0;
        }
        else
        {
            dp   = pL3QosMap[count].dp;    /* Value */
        }

        /* Set color for given dscp */
        rc = cpssHalQoSDscpToDpMapSet(xpDevId, profileId, dscp, dp);
        if (rc != GT_OK)
        {
            XP_SAI_LOG_ERR("Error: Failed to set dscp to dp map, rc: %d\n", rc);
            return cpssStatus2SaiStatus(rc);
        }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiQosMapSetL3EgressQosProfile

static sai_status_t xpSaiQosMapSetL3EgressQosProfile(xpsDevice_t xpDevId,
                                                     uint32_t profileId, sai_object_id_t qosmapObjId)
{
    xpSaiQosMap_t *pQosMapInfo = NULL;
    sai_status_t   saiStatus   = SAI_STATUS_SUCCESS;
    GT_STATUS      rc          = GT_OK;

    /* Get the qos map state data */
    saiStatus = xpSaiQosMapGetStateData(qosmapObjId, &pQosMapInfo);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    xpSaiL3QosMap_t *pTcDpToDscpMap = (xpSaiL3QosMap_t*)pQosMapInfo->mapList;
    sai_uint32_t tc = 0, dscp = 0, dp = 0;

    for (uint32_t count = 0; count < pQosMapInfo->numEntries; count++)
    {
        tc   = pTcDpToDscpMap[count].tc;      /* Key */
        dp   = pTcDpToDscpMap[count].dp;      /* Key */
        dscp = pTcDpToDscpMap[count].dscp;    /* Value */

        /* Set tc dp to dscp map */
        rc = cpssHalQoSTcDpToDscpMapSet(xpDevId, profileId, tc, dp, dscp);
        if (rc != GT_OK)
        {
            XP_SAI_LOG_ERR("Error: Failed to set tc,dp to dscp map, rc: %d\n", rc);
            return cpssStatus2SaiStatus(rc);
        }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiQosMapSetL2EgressQosProfile

static sai_status_t xpSaiQosMapSetL2EgressQosProfile(xpsDevice_t xpDevId,
                                                     uint32_t profileId, sai_object_id_t qosmapObjId)
{
    xpSaiQosMap_t *pQosMapInfo = NULL;
    sai_status_t   saiStatus   = SAI_STATUS_SUCCESS;
    GT_STATUS      rc          = GT_OK;

    /* Get the qos map state data */
    saiStatus = xpSaiQosMapGetStateData(qosmapObjId, &pQosMapInfo);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    xpSaiL2QosMap_t *pTcDpToDot1pMap = (xpSaiL2QosMap_t*)pQosMapInfo->mapList;
    sai_uint32_t tc = 0, dot1p = 0, dp = 0;

    for (uint32_t count = 0; count < pQosMapInfo->numEntries; count++)
    {
        tc    = pTcDpToDot1pMap[count].tc;      /* Key */
        dp    = pTcDpToDot1pMap[count].dp;      /* Key */
        dot1p = pTcDpToDot1pMap[count].dot1p;    /* Value */

        /* Set tc dp to dot1p map */
        rc = cpssHalQoSTcDpToDot1pMapSet(xpDevId, profileId, tc, dp, dot1p);
        if (rc != GT_OK)
        {
            XP_SAI_LOG_ERR("Error: Failed to set tc,dp to dot1p map, rc: %d\n", rc);
            return cpssStatus2SaiStatus(rc);
        }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiQosMapConfigPortIngressProfileId

static sai_status_t xpSaiQosMapConfigPortIngressProfileId(
    sai_object_id_t portObjId,
    sai_object_id_t dot1pToTcObjId,
    sai_object_id_t dot1pToDpObjId,
    sai_object_id_t dscpToTcObjId,
    sai_object_id_t dscpToDpObjId,
    xpSaiPortQosProfileMap_t *profileList,
    uint32_t *pProfileId)
{
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;
    sai_uint32_t entryFound = 0;
    sai_uint32_t freeIdxFound = 0;
    bool         update_refCount = true;
    xpsDevice_t  xpDevId = xpSaiObjIdSwitchGet(portObjId);

    if ((dot1pToTcObjId == SAI_NULL_OBJECT_ID) &&
        (dot1pToDpObjId == SAI_NULL_OBJECT_ID) &&
        (dscpToTcObjId == SAI_NULL_OBJECT_ID) && (dscpToDpObjId == SAI_NULL_OBJECT_ID))
    {
        *pProfileId = 0;
        return SAI_STATUS_SUCCESS;
    }

    /* Loop through qos ingress port profile list */
    for (uint32_t idx = 0; idx < XPSAI_QOSMAP_MAX_QOSMAP_PROFILES; idx++)
    {
        if (profileList[idx].refCount == 0)
        {
            continue;
        }

        /* Check if dot1pToTc/dscpToTc and dot1pToDp/dscpToDp combination exist */
        if ((profileList[idx].dot1pToTcObjId == dot1pToTcObjId) &&
            (profileList[idx].dot1pToDpObjId == dot1pToDpObjId) &&
            (profileList[idx].dscpToTcObjId == dscpToTcObjId) &&
            (profileList[idx].dscpToDpObjId == dscpToDpObjId))
        {
            // Entry found
            profileList[idx].portObjIdList[profileList[idx].refCount] =
                portObjId; // Update port object list
            profileList[idx].refCount += 1;   // Update refCount
            *pProfileId = idx;
            entryFound = 1;
            break;
        }
    }

    /* Add entry only if it is not found */
    if (entryFound == 0)
    {
        sai_uint32_t idx = 0;

        /* Get a free l2 ingress port qos profile and add entry */
        for (idx = 0; idx < XPSAI_QOSMAP_MAX_QOSMAP_PROFILES; idx++)
        {
            if (profileList[idx].refCount == 0)
            {
                freeIdxFound = 1;
                *pProfileId = idx;
                break;
            }
            else if ((profileList[idx].refCount == 1) &&
                     (profileList[idx].portObjIdList[0] == portObjId))
            {
                freeIdxFound = 1;
                *pProfileId = idx;
                /* Same profile index is overwritten.
                 * So no need to update refCount. */
                update_refCount = false;
                break;
            }
        }

        if (freeIdxFound == 1)
        {
            if (dot1pToTcObjId)
            {
                saiStatus = xpSaiQosMapSetTrafficClassForL2QosProfile(xpDevId, *pProfileId,
                                                                      dot1pToTcObjId);
                if (saiStatus != SAI_STATUS_SUCCESS)
                {
                    return saiStatus;
                }
            }

            if (dot1pToDpObjId)
            {
                saiStatus = xpSaiQosMapSetColorForL2QosProfile(xpDevId, *pProfileId,
                                                               dot1pToDpObjId);
                if (saiStatus != SAI_STATUS_SUCCESS)
                {
                    return saiStatus;
                }
            }

            if (dscpToTcObjId)
            {
                saiStatus = xpSaiQosMapSetTrafficClassForL3QosProfile(xpDevId, *pProfileId,
                                                                      dscpToTcObjId, false);
                if (saiStatus != SAI_STATUS_SUCCESS)
                {
                    return saiStatus;
                }
            }

            if (dscpToDpObjId)
            {
                saiStatus = xpSaiQosMapSetColorForL3QosProfile(xpDevId, *pProfileId,
                                                               dscpToDpObjId, false);
                if (saiStatus != SAI_STATUS_SUCCESS)
                {
                    return saiStatus;
                }
            }

            /* Found empty slot */
            profileList[idx].dot1pToTcObjId = dot1pToTcObjId;
            profileList[idx].dot1pToDpObjId = dot1pToDpObjId;
            profileList[idx].dscpToTcObjId = dscpToTcObjId;
            profileList[idx].dscpToDpObjId = dscpToDpObjId;
            profileList[idx].portObjIdList[profileList[idx].refCount] =
                portObjId; // Update port object list
            if (update_refCount)
            {
                profileList[idx].refCount += 1;    // Update refCount
            }
        }
        else
        {
            XP_SAI_LOG_ERR("Error: No free l2 ingress port qos profile available\n");
            return SAI_STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiQosMapDeletePortIngressProfileId

static sai_status_t xpSaiQosMapDeletePortIngressProfileId(
    sai_object_id_t portObjId,
    sai_object_id_t dot1pToTcObjId,
    sai_object_id_t dot1pToDpObjId,
    sai_object_id_t dscpToTcObjId,
    sai_object_id_t dscpToDpObjId,
    xpSaiPortQosProfileMap_t *profileList)
{
    sai_uint32_t portObjPresent = 0;

    if ((dot1pToTcObjId == SAI_NULL_OBJECT_ID) &&
        (dot1pToDpObjId == SAI_NULL_OBJECT_ID) &&
        (dscpToTcObjId == SAI_NULL_OBJECT_ID) && (dscpToDpObjId == SAI_NULL_OBJECT_ID))
    {
        return SAI_STATUS_SUCCESS;
    }

    /* Loop through qos port profile list */
    for (uint32_t idx = 0; idx < XPSAI_QOSMAP_MAX_QOSMAP_PROFILES; idx++)
    {
        if (profileList[idx].refCount == 0)
        {
            continue;
        }

        /* Check if dot1pToTc and dot1pToDP combination exist */
        if ((profileList[idx].dot1pToTcObjId == dot1pToTcObjId) &&
            (profileList[idx].dot1pToDpObjId == dot1pToDpObjId) &&
            (profileList[idx].dscpToTcObjId == dscpToTcObjId) &&
            (profileList[idx].dscpToDpObjId == dscpToDpObjId))
        {
            // Entry found

            // Update port object list
            for (uint32_t count = 0; count < profileList[idx].refCount; count++)
            {
                if (!portObjPresent)
                {
                    if (profileList[idx].portObjIdList[count] == portObjId)
                    {
                        portObjPresent = 1;
                    }
                    continue;
                }
                else
                {
                    profileList[idx].portObjIdList[count-1] = profileList[idx].portObjIdList[count];
                }
            }

            if (portObjPresent)
            {
                profileList[idx].refCount -= 1; // Update refCount
            }
            break;
        }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiQosMapGetPortIngressProfileId

sai_status_t xpSaiQosMapGetPortIngressProfileId(sai_object_id_t portObjId,
                                                sai_object_id_t dot1pToTcObjId,
                                                sai_object_id_t dot1pToDpObjId,
                                                sai_object_id_t dscpToTcObjId,
                                                sai_object_id_t dscpToDpObjId,
                                                uint32_t *pProfileId)
{
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;
    xpsDevice_t  xpDevId   = xpSaiObjIdSwitchGet(portObjId);

    xpSaiPortQosProfileMap_t *profileList =
        gPortQosProfileList[xpDevId]->ingressProfileList;

    saiStatus = xpSaiQosMapConfigPortIngressProfileId(portObjId, dot1pToTcObjId,
                                                      dot1pToDpObjId,
                                                      dscpToTcObjId, dscpToDpObjId, profileList, pProfileId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    return saiStatus;
}

//Func: xpSaiQosMapClearPortIngressProfileId

sai_status_t xpSaiQosMapClearPortIngressProfileId(sai_object_id_t portObjId,
                                                  sai_object_id_t dot1pToTcObjId,
                                                  sai_object_id_t dot1pToDpObjId,
                                                  sai_object_id_t dscpToTcObjId,
                                                  sai_object_id_t dscpToDpObjId)
{
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;
    xpsDevice_t  xpDevId   = xpSaiObjIdSwitchGet(portObjId);

    xpSaiPortQosProfileMap_t *profileList =
        gPortQosProfileList[xpDevId]->ingressProfileList;

    saiStatus = xpSaiQosMapDeletePortIngressProfileId(portObjId, dot1pToTcObjId,
                                                      dot1pToDpObjId,
                                                      dscpToTcObjId, dscpToDpObjId, profileList);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    return saiStatus;
}

//Func: xpSaiQosMapConfigPortEgressProfileId

static sai_status_t xpSaiQosMapConfigPortEgressProfileId(
    sai_object_id_t portObjId,
    sai_object_id_t tcDpToDot1pObjId,
    sai_object_id_t tcDpToDscpObjId,
    xpSaiPortQosProfileMap_t *profileList,
    uint32_t *pProfileId)
{
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;
    sai_uint32_t entryFound = 0;
    sai_uint32_t freeIdxFound = 0;
    bool         update_refCount = true;
    xpsDevice_t  xpDevId = xpSaiObjIdSwitchGet(portObjId);

    if ((tcDpToDot1pObjId == SAI_NULL_OBJECT_ID) &&
        (tcDpToDscpObjId == SAI_NULL_OBJECT_ID))
    {
        *pProfileId = 0;
        return SAI_STATUS_SUCCESS;
    }

    /* Loop through qos ingress port profile list */
    for (uint32_t idx = 0; idx < XPSAI_QOSMAP_MAX_QOSMAP_PROFILES; idx++)
    {
        if (profileList[idx].refCount == 0)
        {
            continue;
        }

        /* Check if tcDpToDot1p/tcDpToDscp combination exist */
        if ((profileList[idx].tcDpToDot1pObjId == tcDpToDot1pObjId) &&
            (profileList[idx].tcDpToDscpObjId == tcDpToDscpObjId))
        {
            // Entry found
            profileList[idx].portObjIdList[profileList[idx].refCount] =
                portObjId; // Update port object list
            profileList[idx].refCount += 1;   // Update refCount
            *pProfileId = idx;
            entryFound = 1;
            break;
        }
    }

    /* Add entry only if it is not found */
    if (entryFound == 0)
    {
        sai_uint32_t idx = 0;

        /* Get a free l2 ingress port qos profile and add entry */
        for (idx = 0; idx < XPSAI_QOSMAP_MAX_QOSMAP_PROFILES; idx++)
        {
            if (profileList[idx].refCount == 0)
            {
                freeIdxFound = 1;
                *pProfileId = idx;
                break;
            }
            else if ((profileList[idx].refCount == 1) &&
                     (profileList[idx].portObjIdList[0] == portObjId))
            {
                freeIdxFound = 1;
                *pProfileId = idx;
                /* Same profile index is overwritten.
                 * So no need to update refCount. */
                update_refCount = false;
                break;
            }
        }

        if (freeIdxFound == 1)
        {
            if (tcDpToDot1pObjId)
            {
                saiStatus = xpSaiQosMapSetL2EgressQosProfile(xpDevId, *pProfileId,
                                                             tcDpToDot1pObjId);
                if (saiStatus != SAI_STATUS_SUCCESS)
                {
                    return saiStatus;
                }
            }

            if (tcDpToDscpObjId)
            {
                saiStatus = xpSaiQosMapSetL3EgressQosProfile(xpDevId, *pProfileId,
                                                             tcDpToDscpObjId);
                if (saiStatus != SAI_STATUS_SUCCESS)
                {
                    return saiStatus;
                }
            }

            /* Found empty slot */
            profileList[idx].tcDpToDot1pObjId = tcDpToDot1pObjId;
            profileList[idx].tcDpToDscpObjId = tcDpToDscpObjId;
            profileList[idx].portObjIdList[profileList[idx].refCount] =
                portObjId; // Update port object list
            if (update_refCount)
            {
                profileList[idx].refCount += 1;    // Update refCount
            }
        }
        else
        {
            XP_SAI_LOG_ERR("Error: No free l2 ingress port qos profile available\n");
            return SAI_STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiQosMapDeletePortEgressProfileId

static sai_status_t xpSaiQosMapDeletePortEgressProfileId(
    sai_object_id_t portObjId,
    sai_object_id_t tcDpToDot1pObjId,
    sai_object_id_t tcDpToDscpObjId,
    xpSaiPortQosProfileMap_t *profileList)
{
    sai_uint32_t portObjPresent = 0;

    if ((tcDpToDot1pObjId == SAI_NULL_OBJECT_ID) &&
        (tcDpToDscpObjId == SAI_NULL_OBJECT_ID))
    {
        return SAI_STATUS_SUCCESS;
    }

    /* Loop through l2 qos port profile list */
    for (uint32_t idx = 0; idx < XPSAI_QOSMAP_MAX_QOSMAP_PROFILES; idx++)
    {
        if (profileList[idx].refCount == 0)
        {
            continue;
        }

        /* Check if tcDpToDot1p/tcDpToDscp combination exist */
        if ((profileList[idx].tcDpToDot1pObjId == tcDpToDot1pObjId) &&
            (profileList[idx].tcDpToDscpObjId == tcDpToDscpObjId))
        {
            // Entry found

            // Update port object list
            for (uint32_t count = 0; count < profileList[idx].refCount; count++)
            {
                if (!portObjPresent)
                {
                    if (profileList[idx].portObjIdList[count] == portObjId)
                    {
                        portObjPresent = 1;
                    }
                    continue;
                }
                else
                {
                    profileList[idx].portObjIdList[count-1] = profileList[idx].portObjIdList[count];
                }
            }

            if (portObjPresent)
            {
                profileList[idx].refCount -= 1; // Update refCount
            }
            break;
        }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiQosMapGetPortEgressProfileId

sai_status_t xpSaiQosMapGetPortEgressProfileId(sai_object_id_t portObjId,
                                               sai_object_id_t tcDpToDot1pObjId,
                                               sai_object_id_t tcDpToDscpObjId,
                                               uint32_t *pProfileId)
{
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;
    xpsDevice_t  xpDevId   = xpSaiObjIdSwitchGet(portObjId);

    xpSaiPortQosProfileMap_t *profileList =
        gPortQosProfileList[xpDevId]->egressProfileList;

    saiStatus = xpSaiQosMapConfigPortEgressProfileId(portObjId, tcDpToDot1pObjId,
                                                     tcDpToDscpObjId, profileList, pProfileId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    return saiStatus;
}

//Func: xpSaiQosMapClearPortEgressProfileId

sai_status_t xpSaiQosMapClearPortEgressProfileId(sai_object_id_t portObjId,
                                                 sai_object_id_t tcDpToDot1pObjId,
                                                 sai_object_id_t tcDpToDscpObjId)
{
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;
    xpsDevice_t  xpDevId   = xpSaiObjIdSwitchGet(portObjId);

    xpSaiPortQosProfileMap_t *profileList =
        gPortQosProfileList[xpDevId]->egressProfileList;

    saiStatus = xpSaiQosMapDeletePortEgressProfileId(portObjId, tcDpToDot1pObjId,
                                                     tcDpToDscpObjId, profileList);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    return saiStatus;
}

static sai_status_t xpSaiQosMapGetPortQosProfileList(sai_object_id_t
                                                     qosmapObjId, int32_t mapType,
                                                     sai_uint32_t *pProfileList, sai_uint32_t *pCount)
{
    sai_uint32_t idx = 0, count = 0;
    xpsDevice_t  xpDevId = xpSaiObjIdSwitchGet(qosmapObjId);

    memset(pProfileList, 0, sizeof(sai_uint32_t) *
           XPSAI_QOSMAP_MAX_QOSMAP_PROFILES);

    switch (mapType)
    {
        case SAI_QOS_MAP_TYPE_DOT1P_TO_TC:
            {
                xpSaiPortQosProfileMap_t *profileList =
                    gPortQosProfileList[xpDevId]->ingressProfileList;

                for (idx = 0; idx < XPSAI_QOSMAP_MAX_QOSMAP_PROFILES; idx++)
                {
                    if (profileList[idx].dot1pToTcObjId == qosmapObjId)
                    {
                        pProfileList[count] = idx;
                        count++;
                    }
                }
                *pCount = count;
                break;
            }
        case SAI_QOS_MAP_TYPE_DOT1P_TO_COLOR:
            {
                xpSaiPortQosProfileMap_t *profileList =
                    gPortQosProfileList[xpDevId]->ingressProfileList;

                for (idx = 0; idx < XPSAI_QOSMAP_MAX_QOSMAP_PROFILES; idx++)
                {
                    if (profileList[idx].dot1pToDpObjId == qosmapObjId)
                    {
                        pProfileList[count] = idx;
                        count++;
                    }
                }
                *pCount = count;
                break;
            }
        case SAI_QOS_MAP_TYPE_DSCP_TO_TC:
            {
                xpSaiPortQosProfileMap_t *profileList =
                    gPortQosProfileList[xpDevId]->ingressProfileList;

                for (idx = 0; idx < XPSAI_QOSMAP_MAX_QOSMAP_PROFILES; idx++)
                {
                    if (profileList[idx].dscpToTcObjId == qosmapObjId)
                    {
                        pProfileList[count] = idx;
                        count++;
                    }
                }
                *pCount = count;
                break;
            }
        case SAI_QOS_MAP_TYPE_DSCP_TO_COLOR:
            {
                xpSaiPortQosProfileMap_t *profileList =
                    gPortQosProfileList[xpDevId]->ingressProfileList;

                for (idx = 0; idx < XPSAI_QOSMAP_MAX_QOSMAP_PROFILES; idx++)
                {
                    if (profileList[idx].dscpToDpObjId == qosmapObjId)
                    {
                        pProfileList[count] = idx;
                        count++;
                    }
                }
                *pCount = count;
                break;
            }
        case SAI_QOS_MAP_TYPE_TC_AND_COLOR_TO_DOT1P:
            {
                xpSaiPortQosProfileMap_t *profileList =
                    gPortQosProfileList[xpDevId]->egressProfileList;

                for (idx = 0; idx < XPSAI_QOSMAP_MAX_QOSMAP_PROFILES; idx++)
                {
                    if (profileList[idx].tcDpToDot1pObjId == qosmapObjId)
                    {
                        pProfileList[count] = idx;
                        count++;
                    }
                }
                *pCount = count;
                break;
            }
        case SAI_QOS_MAP_TYPE_TC_AND_COLOR_TO_DSCP:
            {
                xpSaiPortQosProfileMap_t *profileList =
                    gPortQosProfileList[xpDevId]->egressProfileList;

                for (idx = 0; idx < XPSAI_QOSMAP_MAX_QOSMAP_PROFILES; idx++)
                {
                    if (profileList[idx].tcDpToDscpObjId == qosmapObjId)
                    {
                        pProfileList[count] = idx;
                        count++;
                    }
                }
                *pCount = count;
                break;
            }
        default:
            {
                *pCount = count;
                break;
            }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiQosMapConfigureValueList

static sai_status_t xpSaiQosMapConfigureValueList(sai_object_id_t qosmapObjId,
                                                  int32_t mapType, sai_qos_map_list_t mapList)
{
    xpSaiQosMap_t *pQosMapInfo = NULL;
    sai_status_t   saiStatus   = SAI_STATUS_SUCCESS;
    XP_STATUS      xpStatus    = XP_NO_ERR;
    GT_STATUS      rc          = GT_OK;

    /* Get the qos map state data */
    saiStatus = xpSaiQosMapGetStateData(qosmapObjId, &pQosMapInfo);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    /* Get the device id from qos map object */
    xpsDevice_t xpDevId = (xpsDevice_t)xpSaiObjIdSwitchGet(qosmapObjId);

    sai_uint32_t profileList[XPSAI_QOSMAP_MAX_QOSMAP_PROFILES], numProfiles = 0;

    /* Get qos ingress port profile list for given qos map object */
    saiStatus = xpSaiQosMapGetPortQosProfileList(qosmapObjId, pQosMapInfo->mapType,
                                                 profileList, &numProfiles);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    switch (mapType)
    {
        /* Qos Map to set DOT1P to Traffic class */
        case SAI_QOS_MAP_TYPE_DOT1P_TO_TC:
            {
                xpSaiL2QosMap_t *pL2QosMap = (xpSaiL2QosMap_t *) pQosMapInfo->mapList;
                int32_t          pcp       = 0;
                int32_t          tc        = 0;

                /* Check map boundaries */
                for (uint32_t count = 0; count < mapList.count; ++count)
                {
                    pcp = mapList.list[count].key.dot1p;  /* Key */
                    tc  = mapList.list[count].value.tc;   /* Value */

                    if (!XPSAI_QOSMAP_IS_PCP_VALUE_VALID(pcp))
                    {
                        XP_SAI_LOG_ERR("Current pcp value is invalid: %" PRIu8 "\n", pcp);
                        return SAI_STATUS_INVALID_ATTR_VALUE_0 + SAI_STATUS_CODE(count);
                    }

                    if (!XPSAI_QOSMAP_IS_TC_VALUE_VALID(tc))
                    {
                        XP_SAI_LOG_ERR("Current tc value is invalid: %" PRIu8 "\n", tc);
                        return SAI_STATUS_INVALID_ATTR_VALUE_0 + SAI_STATUS_CODE(count);
                    }
                }

                /* Update map */
                for (uint32_t count = 0; count < mapList.count; ++count)
                {
                    pcp = mapList.list[count].key.dot1p;  /* Key */
                    tc  = mapList.list[count].value.tc;   /* Value */

                    for (uint32_t idx = 0; idx < numProfiles; ++idx)
                    {
                        for (uint32_t dei = 0; dei < XPSAI_QOSMAP_MAX_DEI_VALUE; ++dei)
                        {
                            /* Set the traffic class for given dot1p */
                            rc = cpssHalQoSDot1pToTcMapSet(xpDevId, profileList[idx], pcp, dei, tc);
                            if (rc != GT_OK)
                            {
                                XP_SAI_LOG_ERR("Error: Failed to set the tc for given dot1p, rc: %d\n", rc);
                                return cpssStatus2SaiStatus(rc);
                            }
                        }
                    }

                    /* Update state data */
                    pL2QosMap[pcp].tc = (uint8_t) tc;
                }
                break;
            }
        case SAI_QOS_MAP_TYPE_DOT1P_TO_COLOR:
            {
                xpSaiL2QosMap_t   *pL2QosMap = (xpSaiL2QosMap_t *) pQosMapInfo->mapList;
                sai_packet_color_t dp        = SAI_PACKET_COLOR_GREEN;
                int32_t            pcp       = 0;

                /* Check map boundaries */
                for (uint32_t count = 0; count < mapList.count; ++count)
                {
                    pcp = mapList.list[count].key.dot1p;     /* Key */
                    dp   = mapList.list[count].value.color;  /* Value */

                    if (!XPSAI_QOSMAP_IS_PCP_VALUE_VALID(pcp))
                    {
                        XP_SAI_LOG_ERR("Current pcp value is invalid: %" PRIu8 "\n", pcp);
                        return SAI_STATUS_INVALID_ATTR_VALUE_0 + SAI_STATUS_CODE(count);
                    }

                    if (!XPSAI_QOSMAP_IS_DP_VALUE_VALID(dp))
                    {
                        XP_SAI_LOG_ERR("Current dp value is invalid: %" PRIu8 "\n", dp);
                        return SAI_STATUS_INVALID_ATTR_VALUE_0 + SAI_STATUS_CODE(count);
                    }
                }

                /* Update map */
                for (uint32_t count = 0; count < mapList.count; ++count)
                {
                    pcp = mapList.list[count].key.dot1p;    /* Key */
                    dp  = mapList.list[count].value.color;  /* Value */

                    for (uint32_t idx = 0; idx < numProfiles; ++idx)
                    {
                        for (uint32_t dei = 0; dei < XPSAI_QOSMAP_MAX_DEI_VALUE; ++dei)
                        {
                            /* Set color for given dot1p */
                            rc = cpssHalQoSDot1pToDpMapSet(xpDevId, profileList[idx], pcp, dei, dp);
                            if (rc != GT_OK)
                            {
                                XP_SAI_LOG_ERR("Error: Failed to set DP for L2 QoS profile, rc: %d\n", rc);
                                return cpssStatus2SaiStatus(rc);
                            }
                        }
                    }

                    /* Update state data */
                    pL2QosMap[pcp].dp = dp;
                }
                break;
            }
        case SAI_QOS_MAP_TYPE_DSCP_TO_TC:
            {
                xpSaiL3QosMap_t *pL3QosMap = (xpSaiL3QosMap_t *) pQosMapInfo->mapList;
                int32_t          dscp      = 0;
                int32_t          tc        = 0;

                /* Validate map boundaries */
                for (uint32_t count = 0; count < mapList.count; ++count)
                {
                    dscp = mapList.list[count].key.dscp;  /* Key */
                    tc   = mapList.list[count].value.tc;  /* Value */

                    if (!XPSAI_QOSMAP_IS_DSCP_VALUE_VALID(dscp))
                    {
                        XP_SAI_LOG_ERR("Current dscp value is invalid: %" PRIu8 "\n", dscp);
                        return SAI_STATUS_INVALID_ATTR_VALUE_0 + SAI_STATUS_CODE(count);
                    }

                    if (!XPSAI_QOSMAP_IS_TC_VALUE_VALID(tc))
                    {
                        XP_SAI_LOG_ERR("Current tc value is invalid: %" PRIu8 "\n", tc);
                        return SAI_STATUS_INVALID_ATTR_VALUE_0 + SAI_STATUS_CODE(count);
                    }
                }

                /* Update map */
                for (uint32_t count = 0; count < mapList.count; ++count)
                {
                    dscp = mapList.list[count].key.dscp;  /* Key */
                    tc   = mapList.list[count].value.tc;  /* Value */

                    for (uint32_t idx = 0; idx < numProfiles; ++idx)
                    {
                        /* Set the traffic cleass for given dscp */
                        rc = cpssHalQoSDscpToTcMapSet(xpDevId, profileList[idx], dscp, tc);
                        if (rc != GT_OK)
                        {
                            XP_SAI_LOG_ERR("Error: Failed to set the tc for given dscp, rc: %d\n", rc);
                            return cpssStatus2SaiStatus(rc);
                        }
                    }

                    /* Update state data */
                    pL3QosMap[dscp].tc = (uint8_t) tc;
                }
                break;
            }
        case SAI_QOS_MAP_TYPE_DSCP_TO_COLOR:
            {
                xpSaiL3QosMap_t   *pL3QosMap = (xpSaiL3QosMap_t *) pQosMapInfo->mapList;
                sai_packet_color_t dp        = SAI_PACKET_COLOR_GREEN;
                int32_t            dscp      = 0;

                /* Check map boundaries */
                for (uint32_t count = 0; count < mapList.count; ++count)
                {
                    dscp = mapList.list[count].key.dscp;     /* Key */
                    dp   = mapList.list[count].value.color;  /* Value */

                    if (!XPSAI_QOSMAP_IS_DSCP_VALUE_VALID(dscp))
                    {
                        XP_SAI_LOG_ERR("Current dscp value is invalid: %" PRIu8 "\n", dscp);
                        return SAI_STATUS_INVALID_ATTR_VALUE_0 + SAI_STATUS_CODE(count);
                    }

                    if (!XPSAI_QOSMAP_IS_DP_VALUE_VALID(dp))
                    {
                        XP_SAI_LOG_ERR("Current dp value is invalid: %" PRIu8 "\n", dp);
                        return SAI_STATUS_INVALID_ATTR_VALUE_0 + SAI_STATUS_CODE(count);
                    }
                }

                /* Update map */
                for (uint32_t count = 0; count < mapList.count; count++)
                {
                    dscp = mapList.list[count].key.dscp;     /* Key */
                    dp   = mapList.list[count].value.color;  /* Value */

                    for (uint32_t idx = 0; idx < numProfiles; idx++)
                    {
                        /* Set the DP for given DSCP */
                        rc = cpssHalQoSDscpToDpMapSet(xpDevId, profileList[idx], dscp, dp);
                        if (rc != GT_OK)
                        {
                            XP_SAI_LOG_ERR("Error: Failed to set drop precedence for L3 QoS profile, rc: %d\n",
                                           rc);
                            return cpssStatus2SaiStatus(rc);
                        }
                    }

                    /* Update state data */
                    pL3QosMap[dscp].dp = dp;
                }
                break;
            }
        case SAI_QOS_MAP_TYPE_TC_TO_QUEUE:
            {
                xpSaiTcToQueueMap_t *pTcToQueueMap = (xpSaiTcToQueueMap_t *)
                                                     pQosMapInfo->mapList;
                uint32_t             qNum          = 0;
                uint32_t             tc            = 0;

                /* Check map boundaries */
                for (uint32_t count = 0; count < mapList.count; ++count)
                {
                    tc   = mapList.list[count].key.tc;
                    qNum = mapList.list[count].value.queue_index;

                    if (!XPSAI_QOSMAP_IS_TC_VALUE_VALID(tc))
                    {
                        XP_SAI_LOG_ERR("Current tc value is invalid: %" PRIu8 "\n", tc);
                        return SAI_STATUS_INVALID_ATTR_VALUE_0 + SAI_STATUS_CODE(count);
                    }

                    if (!XPSAI_QOSMAP_IS_QUEUE_VALUE_VALID(qNum))
                    {
                        XP_SAI_LOG_ERR("Current queue num is invalid: %" PRIu8 "\n", qNum);
                        return SAI_STATUS_INVALID_ATTR_VALUE_0 + SAI_STATUS_CODE(count);
                    }
                }

                /* Update map */
                for (uint32_t count = 0; count < mapList.count; count++)
                {
                    tc   = mapList.list[count].key.tc;             /* Key */
                    qNum = mapList.list[count].value.queue_index;  /* Value */

                    if (pQosMapInfo->portCount > 0)
                    {
                        /* TC to queue map per port is not supported */
                        rc = cpssHalQoSTcRemapTableSet(xpDevId, tc, qNum);
                        if (rc != GT_OK)
                        {
                            XP_SAI_LOG_ERR("Error: Failed to set tc to queue map, rc: %d\n", rc);
                            return cpssStatus2SaiStatus(rc);
                        }
                    }

                    pTcToQueueMap[tc].queue = (uint8_t) qNum;
                }

                break;
            }
        case SAI_QOS_MAP_TYPE_TC_AND_COLOR_TO_DSCP:
            {
                xpSaiL3QosMap_t *  pTcDpToDscpMap = (xpSaiL3QosMap_t *)pQosMapInfo->mapList;
                sai_packet_color_t dp             = SAI_PACKET_COLOR_GREEN;
                int32_t            dscp           = 0;
                int32_t            tc             = 0;

                /* Check map boundaries */
                for (uint32_t count = 0; count < mapList.count; ++count)
                {
                    tc   = mapList.list[count].key.tc;      /* Key */
                    dp   = mapList.list[count].key.color;   /* Key */
                    dscp = mapList.list[count].value.dscp;  /* Value */

                    if (!XPSAI_QOSMAP_IS_TC_VALUE_VALID(tc))
                    {
                        XP_SAI_LOG_ERR("Current tc value is invalid: %" PRIu8 "\n", tc);
                        return SAI_STATUS_INVALID_ATTR_VALUE_0 + SAI_STATUS_CODE(count);
                    }

                    if (!XPSAI_QOSMAP_IS_DP_VALUE_VALID(dp))
                    {
                        XP_SAI_LOG_ERR("Current dp value is invalid: %" PRIu8 "\n", dp);
                        return SAI_STATUS_INVALID_ATTR_VALUE_0 + SAI_STATUS_CODE(count);
                    }

                    if (!XPSAI_QOSMAP_IS_DSCP_VALUE_VALID(dscp))
                    {
                        XP_SAI_LOG_ERR("Current dscp value is invalid: %" PRIu8 "\n", dscp);
                        return SAI_STATUS_INVALID_ATTR_VALUE_0 + SAI_STATUS_CODE(count);
                    }
                }

                /* Update map */
                for (uint32_t count = 0; count < mapList.count; ++count)
                {
                    tc   = mapList.list[count].key.tc;      /* Key */
                    dp   = mapList.list[count].key.color;   /* Key */
                    dscp = mapList.list[count].value.dscp;  /* Value */

                    for (uint32_t idx = 0; idx < numProfiles; idx++)
                    {
                        /* Set the TC,DP to DSCP map */
                        rc = cpssHalQoSTcDpToDscpMapSet(xpDevId, profileList[idx], tc, dp, dscp);
                        if (rc != GT_OK)
                        {
                            XP_SAI_LOG_ERR("Error: Failed to set tc,dp to dscp map, rc: %d\n", rc);
                            return cpssStatus2SaiStatus(rc);
                        }
                    }

                    for (uint32_t i = 0; i < pQosMapInfo->numEntries; ++i)
                    {
                        /* Find a corresponding key */
                        if ((pTcDpToDscpMap[i].tc == tc) && (pTcDpToDscpMap[i].dp == dp))
                        {
                            /* Update state data */
                            pTcDpToDscpMap[i].dscp = (uint8_t) dscp;
                            break;
                        }
                    }
                }
                break;
            }
        case SAI_QOS_MAP_TYPE_TC_AND_COLOR_TO_DOT1P:
            {
                xpSaiL2QosMap_t *  pTcDpToDot1pMap = (xpSaiL2QosMap_t *)pQosMapInfo->mapList;
                sai_packet_color_t dp              = SAI_PACKET_COLOR_GREEN;
                int32_t            pcp             = 0;
                int32_t            tc              = 0;

                /* Check map boundaries */
                for (uint32_t count = 0; count < mapList.count; ++count)
                {
                    tc  = mapList.list[count].key.tc;       /* Key */
                    dp  = mapList.list[count].key.color;    /* Key */
                    pcp = mapList.list[count].value.dot1p;  /* Value */

                    if (!XPSAI_QOSMAP_IS_TC_VALUE_VALID(tc))
                    {
                        XP_SAI_LOG_ERR("Current tc value is invalid: %" PRIu8 "\n", tc);
                        return SAI_STATUS_INVALID_ATTR_VALUE_0 + SAI_STATUS_CODE(count);
                    }

                    if (!XPSAI_QOSMAP_IS_DP_VALUE_VALID(dp))
                    {
                        XP_SAI_LOG_ERR("Current dp value is invalid: %" PRIu8 "\n", dp);
                        return SAI_STATUS_INVALID_ATTR_VALUE_0 + SAI_STATUS_CODE(count);
                    }

                    if (!XPSAI_QOSMAP_IS_DSCP_VALUE_VALID(pcp))
                    {
                        XP_SAI_LOG_ERR("Current dscp value is invalid: %" PRIu8 "\n", pcp);
                        return SAI_STATUS_INVALID_ATTR_VALUE_0 + SAI_STATUS_CODE(count);
                    }
                }

                /* Update map */
                for (uint32_t count = 0; count < mapList.count; ++count)
                {
                    tc  = mapList.list[count].key.tc;       /* Key */
                    dp  = mapList.list[count].key.color;    /* Key */
                    pcp = mapList.list[count].value.dot1p;  /* Value */

                    for (uint32_t idx = 0; idx < numProfiles; idx++)
                    {
                        /* Set the TC,DP to dot1p map */
                        rc = cpssHalQoSTcDpToDot1pMapSet(xpDevId, profileList[idx], tc, dp, pcp);
                        if (rc != GT_OK)
                        {
                            XP_SAI_LOG_ERR("Error: Failed to set tc,dp to dot1p map, rc: %d\n", rc);
                            return cpssStatus2SaiStatus(rc);
                        }
                    }

                    for (uint32_t i = 0; i < pQosMapInfo->numEntries; ++i)
                    {
                        /* Find a corresponding key */
                        if ((pTcDpToDot1pMap[i].tc == tc) && (pTcDpToDot1pMap[i].dp == dp))
                        {
                            /* Update state data */
                            pTcDpToDot1pMap[i].dot1p = (uint8_t) pcp;
                            break;
                        }
                    }
                }

                break;
            }
        case SAI_QOS_MAP_TYPE_TC_TO_PRIORITY_GROUP:
            {
                xpSaiTcToPgMap_t *pTcToPgMap = (xpSaiTcToPgMap_t *) pQosMapInfo->mapList;
                uint8_t           pg         = 0;
                uint32_t          tc         = 0;

                /* Check map boundaries */
                for (uint32_t count = 0; count < mapList.count; ++count)
                {
                    tc = mapList.list[count].key.tc;    /* Key */

                    if (!XPSAI_QOSMAP_IS_TC_VALUE_VALID(tc))
                    {
                        XP_SAI_LOG_ERR("Current tc value is invalid: %" PRIu8 "\n", tc);
                        return SAI_STATUS_INVALID_ATTR_VALUE_0 + SAI_STATUS_CODE(count);
                    }
                }

                /* Update map */
                for (uint32_t count = 0; count < mapList.count; ++count)
                {
                    tc = mapList.list[count].key.tc;    /* Key */
                    pg = mapList.list[count].value.pg;  /* Value */

                    /* Update state data */
                    pTcToPgMap[tc].pg = pg;
                }
                break;
            }
        case SAI_QOS_MAP_TYPE_PFC_PRIORITY_TO_PRIORITY_GROUP:
            {
                xpSaiPfcPrioToPgMap_t *pPfcPrioToPgMap = (xpSaiPfcPrioToPgMap_t *)
                                                         pQosMapInfo->mapList;
                uint32_t               pfcPrio         = 0;
                uint8_t                pg              = 0;

                /* Validate map boundaries */
                for (uint32_t count = 0; count < mapList.count; ++count)
                {
                    pfcPrio = mapList.list[count].key.prio;  /* Key */

                    if (!XPSAI_QOSMAP_IS_PRIORITY_VALUE_VALID(pfcPrio))
                    {
                        XP_SAI_LOG_ERR("Current pfcPrio value is invalid: %" PRIu8 "\n", pfcPrio);
                        return SAI_STATUS_INVALID_ATTR_VALUE_0 + SAI_STATUS_CODE(count);
                    }
                }

                /* Update map */
                for (uint32_t count = 0; count < mapList.count; ++count)
                {
                    pfcPrio = mapList.list[count].key.prio;  /* Key */
                    pg      = mapList.list[count].value.pg;  /* Value */

                    xpStatus = xpsQosFcBindPfcPriorityToTc(xpDevId, pfcPrio, pg);
                    if (xpStatus != XP_NO_ERR)
                    {
                        XP_SAI_LOG_ERR("Failed to set pfc priority to default pfc tc, xpStatus: %d\n",
                                       xpStatus);
                        return xpsStatus2SaiStatus(xpStatus);
                    }

                    /* Update state data */
                    pPfcPrioToPgMap[pfcPrio].pg = pg;
                }
                break;
            }
        case SAI_QOS_MAP_TYPE_PFC_PRIORITY_TO_QUEUE:
            {
                xpSaiPfcPrioToQueueMap_t *pPfcPrioToQueueMap = (xpSaiPfcPrioToQueueMap_t *)
                                                               pQosMapInfo->mapList;
                uint32_t                  portNum            = 0;
                uint8_t                   qNum               = 0;
                uint8_t                   pfcPrio            = 0;

                /* Validate map boundaries */
                for (uint32_t count = 0; count < mapList.count; ++count)
                {
                    pfcPrio = mapList.list[count].key.prio;  /* Key */
                    qNum    = mapList.list[count].value.queue_index;  /* Value */

                    if (!XPSAI_QOSMAP_IS_PRIORITY_VALUE_VALID(pfcPrio))
                    {
                        XP_SAI_LOG_ERR("Current pfcPrio value is invalid: %" PRIu8 "\n", pfcPrio);
                        return SAI_STATUS_INVALID_ATTR_VALUE_0 + SAI_STATUS_CODE(count);
                    }

                    if (!XPSAI_QOSMAP_IS_QUEUE_VALUE_VALID(qNum))
                    {
                        XP_SAI_LOG_ERR("Current queue num is invalid: %" PRIu8 "\n", qNum);
                        return SAI_STATUS_INVALID_ATTR_VALUE_0 + SAI_STATUS_CODE(count);
                    }
                }

                /* Update map */
                for (uint32_t count = 0; count < mapList.count; count++)
                {
                    pfcPrio = mapList.list[count].key.prio;           /* Key */
                    qNum    = mapList.list[count].value.queue_index;  /* Value */

                    for (uint32_t idx = 0; idx < pQosMapInfo->portCount; idx++)
                    {
                        /* Fetch device and port information for this particular port interface */
                        if ((xpStatus = xpsPortGetDevAndPortNumFromIntf(pQosMapInfo->portList[idx],
                                                                        &xpDevId, &portNum)) != XP_NO_ERR)
                        {
                            XP_SAI_LOG_ERR("Fetch device and port information failed, interface id(%d)",
                                           pQosMapInfo->portList[idx]);
                            return xpsStatus2SaiStatus(xpStatus);
                        }

                        rc = cpssHalPortPfcToQueueMapSet(xpDevId, portNum, pfcPrio, qNum);
                        if (rc != GT_OK)
                        {
                            XP_SAI_LOG_ERR("Error: Failed to set PFC priority to queue, rc: %d\n", rc);
                            return cpssStatus2SaiStatus(rc);
                        }

                        /* TODO check if call need */
                        /* xpSaiQosMapUpdatePfcPriorityToQueue(xpDevId, portNum, qNum, pfcPrio); */
                    }

                    for (uint32_t i = 0; i < pQosMapInfo->numEntries; ++i)
                    {
                        if (pPfcPrioToQueueMap[i].queue == qNum)
                        {
                            /* Update state data */
                            pPfcPrioToQueueMap[i].pfcPriority = pfcPrio;
                            break;
                        }
                    }
                }
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Error: Invalid qos map type %d \n", mapType);
                return SAI_STATUS_INVALID_ATTR_VALUE_0;
            }
    }

    return SAI_STATUS_SUCCESS;
}


#if 0
//Func: xpSaiQosMapClearValueList

static sai_status_t xpSaiQosMapClearValueList(sai_object_id_t qosmapObjId,
                                              int32_t mapType)
{
    xpSaiQosMap_t *pQosMapInfo = NULL;
    sai_status_t   saiStatus   = SAI_STATUS_SUCCESS;
    XP_STATUS      xpStatus    = XP_NO_ERR;

    /* Get the qos map state data */
    saiStatus = xpSaiQosMapGetStateData(qosmapObjId, &pQosMapInfo);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    /* Get the device id from qos map object */
    xpsDevice_t xpDevId = (xpsDevice_t)xpSaiObjIdSwitchGet(qosmapObjId);

    sai_uint32_t profileList[XPSAI_QOSMAP_MAX_QOSMAP_PROFILES], numProfiles = 0;

    /* Get qos ingress port profile list for given qos map object */
    saiStatus = xpSaiQosMapGetPortQosProfileList(qosmapObjId, pQosMapInfo->mapType,
                                                 profileList, &numProfiles);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    switch (mapType)
    {
        case SAI_QOS_MAP_TYPE_TC_TO_PRIORITY_GROUP:
            {
                xpSaiTcToPgMap_t *pTcToPgMap = (xpSaiTcToPgMap_t*)pQosMapInfo->mapList;

                for (uint32_t count = 0; count < pQosMapInfo->numEntries; count++)
                {
                    pTcToPgMap[count].tc = 0;
                    pTcToPgMap[count].pg = 0;
                }
                break;
            }
        case SAI_QOS_MAP_TYPE_PFC_PRIORITY_TO_PRIORITY_GROUP:
            {
                xpSaiPfcPrioToPgMap_t *pPfcPrioToPgMap = (xpSaiPfcPrioToPgMap_t*)
                                                         pQosMapInfo->mapList;
                sai_uint32_t priority = 0;

                for (uint32_t count = 0; count < pQosMapInfo->numEntries; count++)
                {
                    priority = pPfcPrioToPgMap[count].pfcPriority;  /* Key */

                    /* XP70 supports only 1:1 mapping */
                    if (!IS_DEVICE_XP70(xpSaiSwitchDevTypeGet()))
                    {
                        xpStatus = xpsQosFcBindPfcPriorityToTc(xpDevId, priority, 0);
                        if (xpStatus != XP_NO_ERR)
                        {
                            XP_SAI_LOG_ERR("Failed to set pfc priority to default pfc tc, xpStatus: %d\n",
                                           xpStatus);
                            return xpsStatus2SaiStatus(xpStatus);
                        }
                    }
                }
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Error: Invalid qos map type %d \n", mapType);
                return SAI_STATUS_INVALID_ATTR_VALUE_0;
            }
    }

    return SAI_STATUS_SUCCESS;
}
#endif

//Func: xpSaiQosMapGetValueListConfiguration

static sai_status_t xpSaiQosMapGetValueListConfiguration(
    sai_object_id_t qosmapObjId, int32_t mapType,
    sai_qos_map_list_t *mapList)
{
    xpSaiQosMap_t *pQosMapInfo = NULL;
    sai_status_t   saiStatus   = SAI_STATUS_SUCCESS;

    /* Get the qos map state data */
    saiStatus = xpSaiQosMapGetStateData(qosmapObjId, &pQosMapInfo);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    switch (mapType)
    {
        /* Qos Map to set DOT1P to Traffic class */
        case SAI_QOS_MAP_TYPE_DOT1P_TO_TC:
            {
                xpSaiL2QosMap_t *pL2QosMap = (xpSaiL2QosMap_t*)pQosMapInfo->mapList;

                if (mapList->count < pQosMapInfo->numEntries)
                {
                    XP_SAI_LOG_ERR("Unable to fit %d entries into the buffer with size %u\n",
                                   pQosMapInfo->numEntries, mapList->count);
                    mapList->count = pQosMapInfo->numEntries;
                    return SAI_STATUS_BUFFER_OVERFLOW;
                }

                mapList->count = pQosMapInfo->numEntries;

                for (uint32_t count = 0; count < mapList->count; count++)
                {
                    mapList->list[count].key.dot1p = pL2QosMap[count].dot1p;
                    mapList->list[count].value.tc  = pL2QosMap[count].tc;
                }
                break;
            }
        case SAI_QOS_MAP_TYPE_DOT1P_TO_COLOR:
            {
                xpSaiL2QosMap_t *pL2QosMap = (xpSaiL2QosMap_t*)pQosMapInfo->mapList;

                if (mapList->count < pQosMapInfo->numEntries)
                {
                    XP_SAI_LOG_ERR("Unable to fit %d entries into the buffer with size %u\n",
                                   pQosMapInfo->numEntries, mapList->count);
                    mapList->count = pQosMapInfo->numEntries;
                    return SAI_STATUS_BUFFER_OVERFLOW;
                }

                mapList->count = pQosMapInfo->numEntries;

                for (uint32_t count = 0; count < mapList->count; count++)
                {
                    mapList->list[count].key.dot1p   = pL2QosMap[count].dot1p;
                    mapList->list[count].value.color = pL2QosMap[count].dp;
                }
                break;
            }
        case SAI_QOS_MAP_TYPE_DSCP_TO_TC:
            {
                xpSaiL3QosMap_t *pL3QosMap = (xpSaiL3QosMap_t*)pQosMapInfo->mapList;

                if (mapList->count < pQosMapInfo->numEntries)
                {
                    XP_SAI_LOG_ERR("Unable to fit %d entries into the buffer with size %u\n",
                                   pQosMapInfo->numEntries, mapList->count);
                    mapList->count = pQosMapInfo->numEntries;
                    return SAI_STATUS_BUFFER_OVERFLOW;
                }

                mapList->count = pQosMapInfo->numEntries;

                for (uint32_t count = 0; count < mapList->count; count++)
                {
                    mapList->list[count].key.dscp = pL3QosMap[count].dscp;
                    mapList->list[count].value.tc = pL3QosMap[count].tc;
                }
                break;
            }
        case SAI_QOS_MAP_TYPE_DSCP_TO_COLOR:
            {
                xpSaiL3QosMap_t *pL3QosMap = (xpSaiL3QosMap_t*)pQosMapInfo->mapList;

                if (mapList->count < pQosMapInfo->numEntries)
                {
                    XP_SAI_LOG_ERR("Unable to fit %d entries into the buffer with size %u\n",
                                   pQosMapInfo->numEntries, mapList->count);
                    mapList->count = pQosMapInfo->numEntries;
                    return SAI_STATUS_BUFFER_OVERFLOW;
                }

                mapList->count = pQosMapInfo->numEntries;

                for (uint32_t count = 0; count < mapList->count; count++)
                {
                    mapList->list[count].key.dscp    = pL3QosMap[count].dscp;
                    mapList->list[count].value.color = pL3QosMap[count].dp;
                }
                break;
            }
        case SAI_QOS_MAP_TYPE_TC_TO_QUEUE:
            {
                xpSaiTcToQueueMap_t *pTcToQueueMap = (xpSaiTcToQueueMap_t*)pQosMapInfo->mapList;

                if (mapList->count < pQosMapInfo->numEntries)
                {
                    XP_SAI_LOG_ERR("Unable to fit %d entries into the buffer with size %u\n",
                                   pQosMapInfo->numEntries, mapList->count);
                    mapList->count = pQosMapInfo->numEntries;
                    return SAI_STATUS_BUFFER_OVERFLOW;
                }

                mapList->count = pQosMapInfo->numEntries;

                for (uint32_t count = 0; count < mapList->count; count++)
                {
                    mapList->list[count].key.tc            = pTcToQueueMap[count].tc;
                    mapList->list[count].value.queue_index = pTcToQueueMap[count].queue;
                }
                break;
            }
        case SAI_QOS_MAP_TYPE_TC_AND_COLOR_TO_DSCP:
            {
                xpSaiL3QosMap_t *pTcDpToDscpMap = (xpSaiL3QosMap_t*)pQosMapInfo->mapList;

                if (mapList->count < pQosMapInfo->numEntries)
                {
                    XP_SAI_LOG_ERR("Unable to fit %d entries into the buffer with size %u\n",
                                   pQosMapInfo->numEntries, mapList->count);
                    mapList->count = pQosMapInfo->numEntries;
                    return SAI_STATUS_BUFFER_OVERFLOW;
                }

                mapList->count = pQosMapInfo->numEntries;

                for (uint32_t count = 0; count < mapList->count; count++)
                {
                    mapList->list[count].key.tc     = pTcDpToDscpMap[count].tc;
                    mapList->list[count].key.color  = pTcDpToDscpMap[count].dp;
                    mapList->list[count].value.dscp = pTcDpToDscpMap[count].dscp;
                }
                break;
            }
        case SAI_QOS_MAP_TYPE_TC_AND_COLOR_TO_DOT1P:
            {
                xpSaiL2QosMap_t *pTcDpToDot1pMap = (xpSaiL2QosMap_t*)pQosMapInfo->mapList;

                if (mapList->count < pQosMapInfo->numEntries)
                {
                    XP_SAI_LOG_ERR("Unable to fit %d entries into the buffer with size %u\n",
                                   pQosMapInfo->numEntries, mapList->count);
                    mapList->count = pQosMapInfo->numEntries;
                    return SAI_STATUS_BUFFER_OVERFLOW;
                }

                mapList->count = pQosMapInfo->numEntries;

                for (uint32_t count = 0; count < mapList->count; count++)
                {
                    mapList->list[count].key.tc      = pTcDpToDot1pMap[count].tc;
                    mapList->list[count].key.color   = pTcDpToDot1pMap[count].dp;
                    mapList->list[count].value.dot1p = pTcDpToDot1pMap[count].dot1p;
                }
                break;
            }
        case SAI_QOS_MAP_TYPE_TC_TO_PRIORITY_GROUP:
            {
                xpSaiTcToPgMap_t *pTcToPgMap = (xpSaiTcToPgMap_t*)pQosMapInfo->mapList;

                if (mapList->count < pQosMapInfo->numEntries)
                {
                    XP_SAI_LOG_ERR("Unable to fit %d entries into the buffer with size %u\n",
                                   pQosMapInfo->numEntries, mapList->count);
                    mapList->count = pQosMapInfo->numEntries;
                    return SAI_STATUS_BUFFER_OVERFLOW;
                }

                mapList->count = pQosMapInfo->numEntries;

                for (uint32_t count = 0; count < mapList->count; count++)
                {
                    mapList->list[count].key.tc   = pTcToPgMap[count].tc;
                    mapList->list[count].value.pg = pTcToPgMap[count].pg;
                }
                break;
            }
        case SAI_QOS_MAP_TYPE_PFC_PRIORITY_TO_PRIORITY_GROUP:
            {
                xpSaiPfcPrioToPgMap_t *pPfcPrioToPgMap = (xpSaiPfcPrioToPgMap_t*)
                                                         pQosMapInfo->mapList;

                if (mapList->count < pQosMapInfo->numEntries)
                {
                    XP_SAI_LOG_ERR("Unable to fit %d entries into the buffer with size %u\n",
                                   pQosMapInfo->numEntries, mapList->count);
                    mapList->count = pQosMapInfo->numEntries;
                    return SAI_STATUS_BUFFER_OVERFLOW;
                }

                mapList->count = pQosMapInfo->numEntries;

                for (uint32_t count = 0; count < mapList->count; count++)
                {
                    mapList->list[count].key.prio = pPfcPrioToPgMap[count].pfcPriority;  /* Key */
                    mapList->list[count].value.pg = pPfcPrioToPgMap[count].pg;;          /* Value */
                }
                break;
            }
        case SAI_QOS_MAP_TYPE_PFC_PRIORITY_TO_QUEUE:
            {
                xpSaiPfcPrioToQueueMap_t *pPfcPrioToQueueMap = (xpSaiPfcPrioToQueueMap_t*)
                                                               pQosMapInfo->mapList;

                if (mapList->count < pQosMapInfo->numEntries)
                {
                    XP_SAI_LOG_ERR("Unable to fit %d entries into the buffer with size %u\n",
                                   pQosMapInfo->numEntries, mapList->count);
                    mapList->count = pQosMapInfo->numEntries;
                    return SAI_STATUS_BUFFER_OVERFLOW;
                }

                mapList->count = pQosMapInfo->numEntries;

                for (uint32_t count = 0; count < mapList->count; count++)
                {
                    mapList->list[count].key.prio          = pPfcPrioToQueueMap[count].pfcPriority;
                    mapList->list[count].value.queue_index = pPfcPrioToQueueMap[count].queue;
                }
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Error: Invalid qos map type %d \n", mapType);
                return SAI_STATUS_INVALID_ATTR_VALUE_0;
            }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetDefaultQosMapAttributeVals

void xpSaiSetDefaultQosMapAttributeVals(xpSaiQosMapAttributesT* attributes)
{
    memset(attributes, 0x0, sizeof(xpSaiQosMapAttributesT));
}

//Func: xpSaiUpdateQosMapAttributeVals

void xpSaiUpdateQosMapAttributeVals(const uint32_t attr_count,
                                    const sai_attribute_t* attr_list, xpSaiQosMapAttributesT* attributes)
{
    for (uint32_t count = 0; count < attr_count; count++)
    {
        switch (attr_list[count].id)
        {
            case SAI_QOS_MAP_ATTR_TYPE:
                {
                    attributes->type = attr_list[count].value;
                    break;
                }
            case SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST:
                {
                    attributes->mapToValueList = attr_list[count].value;
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Error: Unknown attribute %d\n", attr_list[count].id);
                }
        }
    }
}

//Func: xpSaiUpdateAttrListQosMapVals

void xpSaiUpdateAttrListQosMapVals(xpSaiQosMapAttributesT attributes,
                                   const uint32_t attr_count, sai_attribute_t* attr_list)
{
    for (uint32_t count = 0; count < attr_count; count++)
    {
        switch (attr_list[count].id)
        {
            case SAI_QOS_MAP_ATTR_TYPE:
                {
                    attr_list[count].value = attributes.type;
                    break;
                }
            case SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST:
                {
                    attr_list[count].value = attributes.mapToValueList;
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Error: Unknown attribute %d\n", attr_list[count].id);
                }
        }
    }
}

//Func: xpSaiQosMapAddPortToPortList

sai_status_t xpSaiQosMapAddPortToPortList(sai_object_id_t qosmapObjId,
                                          sai_uint32_t portNum)
{
    xpSaiQosMap_t *pQosMapInfo = NULL;
    sai_status_t   saiStatus   = SAI_STATUS_SUCCESS;
    GT_STATUS      rc          = GT_OK;
    xpsDevice_t    xpDevId     = 0;

    /* Get the qos map state data */
    saiStatus = xpSaiQosMapGetStateData(qosmapObjId, &pQosMapInfo);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    /* Update stae data */
    pQosMapInfo->portList[pQosMapInfo->portCount++] = portNum;

    switch (pQosMapInfo->mapType)
    {
        case SAI_QOS_MAP_TYPE_TC_TO_QUEUE:
            {
                xpSaiTcToQueueMap_t *pTcToQueueMap = (xpSaiTcToQueueMap_t *)
                                                     pQosMapInfo->mapList;

                /* TC to Queue map is already configured */
                if (globalTctoQueueMapId == qosmapObjId)
                {
                    break;
                }

                for (uint32_t count = 0; count < pQosMapInfo->numEntries; count++)
                {
                    /* TC to queue map per port is not supported */
                    rc = cpssHalQoSTcRemapTableSet(xpDevId, pTcToQueueMap[count].tc,
                                                   pTcToQueueMap[count].queue);
                    if (rc != GT_OK)
                    {
                        XP_SAI_LOG_ERR("Error: Failed to set tc to queue map, rc: %d\n", rc);
                        return cpssStatus2SaiStatus(rc);
                    }
                }
                globalTctoQueueMapId = qosmapObjId;

                break;
            }
        case SAI_QOS_MAP_TYPE_PFC_PRIORITY_TO_QUEUE:
            {
                xpSaiPfcPrioToQueueMap_t *pPfcPrioToQueueMap = (xpSaiPfcPrioToQueueMap_t *)
                                                               pQosMapInfo->mapList;

                for (uint32_t count = 0; count < pQosMapInfo->numEntries; count++)
                {
                    rc = cpssHalPortPfcToQueueMapSet(xpDevId, portNum,
                                                     pPfcPrioToQueueMap[count].pfcPriority,
                                                     pPfcPrioToQueueMap[count].queue);
                    if (rc != GT_OK)
                    {
                        XP_SAI_LOG_ERR("Error: Failed to set PFC priority to queue, rc: %d\n", rc);
                        return cpssStatus2SaiStatus(rc);
                    }
                }
                break;
            }
        default:  /* Do nothing */
            break;
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiQosMapRemovePortFromPortList

sai_status_t xpSaiQosMapRemovePortFromPortList(sai_object_id_t qosmapObjId,
                                               sai_uint32_t portNum)
{
    xpSaiQosMap_t *pQosMapInfo = NULL;
    sai_uint32_t   portPresent = 0;
    sai_status_t   saiStatus   = SAI_STATUS_SUCCESS;
    GT_STATUS      rc          = GT_OK;
    xpsDevice_t    xpDevId     = 0;

    /* Get the qos map state data */
    saiStatus = xpSaiQosMapGetStateData(qosmapObjId, &pQosMapInfo);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    /* Remove the port from qos map state port list */
    for (uint32_t count = 0; count < pQosMapInfo->portCount; count++)
    {
        if (!portPresent)
        {
            if (pQosMapInfo->portList[count] == portNum)
            {
                portPresent = 1;
            }
            continue;
        }
        else
        {
            pQosMapInfo->portList[count-1] = pQosMapInfo->portList[count];
        }
    }

    if (!portPresent)
    {
        XP_SAI_LOG_ERR("Error: Could not remove port from qos map state port list\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    /* Decrement the portCount count */
    pQosMapInfo->portCount--;

    switch (pQosMapInfo->mapType)
    {
        case SAI_QOS_MAP_TYPE_TC_TO_QUEUE:
            {
                if (globalTctoQueueMapId != 0)
                {
                    for (uint32_t tc = 0; tc < XPSAI_QOSMAP_MAX_TC_VALUE; tc++)
                    {
                        /* Set TC to queue => 1 to 1 mapping */
                        rc = cpssHalQoSTcRemapTableSet(xpDevId, tc, tc);
                        if (rc != GT_OK)
                        {
                            XP_SAI_LOG_ERR("Error: Failed to set tc to queue map, rc: %d\n", rc);
                            return cpssStatus2SaiStatus(rc);
                        }
                    }
                    globalTctoQueueMapId = 0;
                }
                break;
            }
        case SAI_QOS_MAP_TYPE_PFC_PRIORITY_TO_QUEUE:
            {
                for (uint32_t pfcPrio = 0; pfcPrio < XPSAI_QOSMAP_MAX_PRIORITY_VALUE; pfcPrio++)
                {
                    /* set 1-1 mapping for pfc_priority to queue */
                    rc = cpssHalPortPfcToQueueMapSet(xpDevId, portNum, pfcPrio, pfcPrio);
                    if (rc != GT_OK)
                    {
                        XP_SAI_LOG_ERR("Error: Failed to set PFC priority to queue, rc: %d\n", rc);
                        return cpssStatus2SaiStatus(rc);
                    }
                }
                break;
            }
        default:  /* Do nothing */
            break;
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiQosMapValidateAttributeValue

sai_status_t xpSaiQosMapValidateAttributeValue(sai_attr_id_t attrId,
                                               xpSaiQosMapAttributesT *attributes, sai_uint32_t count)
{
    switch (attrId)
    {
        case SAI_QOS_MAP_ATTR_TYPE:
            {
                /* Validate qos map type */
                if (!XPSAI_QOSMAP_IS_TYPE_VALID(attributes->type.s32))
                {
                    XP_SAI_LOG_ERR("Error: Invalid qos map %d provided\n", attributes->type.s32);
                    return SAI_STATUS_ATTR_NOT_SUPPORTED_0 + SAI_STATUS_CODE(count);
                }
                break;
            }
        case SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST:
            {
                if (attributes->type.s32 == SAI_QOS_MAP_TYPE_TC_TO_PRIORITY_GROUP)
                {
                    if (IS_DEVICE_XP70(xpSaiSwitchDevTypeGet()))
                    {
                        for (uint32_t i = 0; i < attributes->mapToValueList.qosmap.count; ++i)
                        {
                            if ((sai_uint8_t) attributes->mapToValueList.qosmap.list[i].key.tc !=
                                attributes->mapToValueList.qosmap.list[i].value.pg)
                            {
                                XP_SAI_LOG_ERR("Device only supports 1 to 1 mapping!\n", attributes->type.s32);
                                return SAI_STATUS_ATTR_NOT_SUPPORTED_0 + SAI_STATUS_CODE(count);
                            }
                        }
                    }
                }

                if (attributes->type.s32 == SAI_QOS_MAP_TYPE_PFC_PRIORITY_TO_PRIORITY_GROUP)
                {
                    if (xpSaiSwitchDevTypeGet() == XP80B0)
                    {
                        /* XP80B0 supports only 4 lossless traffic classes */
                        if (attributes->mapToValueList.qosmap.count > XPSAI_QOSMAP_MAX_PRIORITY_VALUE)
                        {
                            XP_SAI_LOG_ERR("Error: Device supports only 4 lossless priorities\n");
                            return SAI_STATUS_INVALID_ATTR_VALUE_0 + SAI_STATUS_CODE(count);
                        }
                    }
                    else if (IS_DEVICE_XP70(xpSaiSwitchDevTypeGet()))
                    {
                        for (uint32_t i = 0; i < attributes->mapToValueList.qosmap.count; ++i)
                        {
                            if ((sai_uint8_t) attributes->mapToValueList.qosmap.list[i].key.prio !=
                                attributes->mapToValueList.qosmap.list[i].value.pg)
                            {
                                XP_SAI_LOG_ERR("Device only supports 1 to 1 mapping!\n", attributes->type.s32);
                                return SAI_STATUS_ATTR_NOT_SUPPORTED_0 + SAI_STATUS_CODE(count);
                            }
                        }
                    }
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

//Func: xpSaiGetQosMapAttrType

static sai_status_t xpSaiGetQosMapAttrType(sai_object_id_t qosmapObjId,
                                           sai_attribute_value_t* value)
{
    xpSaiQosMap_t *pQosMapInfo = NULL;
    sai_status_t   saiStatus   = SAI_STATUS_SUCCESS;

    /* Get the qos map state data */
    saiStatus = xpSaiQosMapGetStateData(qosmapObjId, &pQosMapInfo);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    value->s32 = pQosMapInfo->mapType;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetQosMapAttrMapToValueList

sai_status_t xpSaiSetQosMapAttrMapToValueList(sai_object_id_t qosmapObjId,
                                              sai_attribute_value_t value)
{
    xpSaiQosMap_t *pQosMapInfo = NULL;
    sai_status_t   saiStatus   = SAI_STATUS_SUCCESS;

    /* Get the qos map state data */
    saiStatus = xpSaiQosMapGetStateData(qosmapObjId, &pQosMapInfo);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    if (value.qosmap.count == 0)
    {
        /* No action needed in case new value list is empty */
        return SAI_STATUS_SUCCESS;
    }

    /* Configure and update state data with qos map list values */
    saiStatus = xpSaiQosMapConfigureValueList(qosmapObjId, pQosMapInfo->mapType,
                                              value.qosmap);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("ERROR: Could not configure qos map attributes, saiStatus: %d\n",
                       saiStatus);
        return saiStatus;
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetQosMapAttrMapToValueList

sai_status_t xpSaiGetQosMapAttrMapToValueList(sai_object_id_t qosmapObjId,
                                              sai_attribute_value_t* value)
{
    xpSaiQosMap_t *pQosMapInfo = NULL;
    sai_status_t   saiStatus   = SAI_STATUS_SUCCESS;

    /* Get the qos map state data */
    saiStatus = xpSaiQosMapGetStateData(qosmapObjId, &pQosMapInfo);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    /* Get the qos map value list from state data */
    saiStatus = xpSaiQosMapGetValueListConfiguration(qosmapObjId,
                                                     pQosMapInfo->mapType, &value->qosmap);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("ERROR: Could not get qos map value list, saiStatus: %d\n",
                       saiStatus);
        return saiStatus;
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiDefaultInitializeMapToValueList

static sai_status_t xpSaiQosMapDefaultInitializeMapToValueList(sai_int32_t type,
                                                               uint32_t mapSize, sai_pointer_t pMapList)
{
    uint32_t i = 0;

    if (pMapList == NULL)
    {
        XP_SAI_LOG_ERR("NULL pointer provided!\n");
        return SAI_STATUS_FAILURE;
    }

    switch (type)
    {
        case SAI_QOS_MAP_TYPE_DOT1P_TO_TC:
        case SAI_QOS_MAP_TYPE_DOT1P_TO_COLOR:
            {
                xpSaiL2QosMap_t *l2Mapping = (xpSaiL2QosMap_t *) pMapList;
                for (i = 0; i < mapSize; ++i)
                {
                    l2Mapping[i].dot1p = (uint8_t) i % XPSAI_QOSMAP_MAX_PCP_VALUE;
                }
                break;
            }
        case SAI_QOS_MAP_TYPE_DSCP_TO_TC:
        case SAI_QOS_MAP_TYPE_DSCP_TO_COLOR:
            {
                xpSaiL3QosMap_t *l3Mapping = (xpSaiL3QosMap_t *) pMapList;
                for (i = 0; i < mapSize; ++i)
                {
                    l3Mapping[i].dscp = (uint8_t) i % XPSAI_QOSMAP_MAX_DSCP_VALUE;
                }
                break;
            }
        case SAI_QOS_MAP_TYPE_TC_TO_QUEUE:
            {
                xpSaiTcToQueueMap_t *tc2QueueMapping = (xpSaiTcToQueueMap_t *) pMapList;
                for (i = 0; i < mapSize; ++i)
                {
                    tc2QueueMapping[i].tc = (uint8_t) i % XPSAI_QOSMAP_MAX_TC_VALUE;
                    tc2QueueMapping[i].queue = (uint8_t) XPSAI_QOSMAP_MIN_QUEUE_VALUE;
                }
                break;
            }
        case SAI_QOS_MAP_TYPE_TC_AND_COLOR_TO_DSCP:
            {
                xpSaiL3QosMap_t *l3Mapping = (xpSaiL3QosMap_t *) pMapList;
                for (i = 0; i < mapSize; ++i)
                {
                    l3Mapping[i].tc = (uint8_t) i % XPSAI_QOSMAP_MAX_TC_VALUE;
                    l3Mapping[i].dp = (sai_packet_color_t)((i / XPSAI_QOSMAP_MAX_TC_VALUE) %
                                                           XPSAI_QOSMAP_MAX_DP_VALUE);
                }
                break;
            }
        case SAI_QOS_MAP_TYPE_TC_AND_COLOR_TO_DOT1P:
            {
                xpSaiL2QosMap_t *l2Mapping = (xpSaiL2QosMap_t *) pMapList;
                for (i = 0; i < mapSize; ++i)
                {
                    l2Mapping[i].tc = (uint8_t) i % XPSAI_QOSMAP_MAX_TC_VALUE;
                    l2Mapping[i].dp = (sai_packet_color_t)((i / XPSAI_QOSMAP_MAX_TC_VALUE) %
                                                           XPSAI_QOSMAP_MAX_DP_VALUE);
                }
                break;
            }
        case SAI_QOS_MAP_TYPE_TC_TO_PRIORITY_GROUP:
            {
                xpSaiTcToPgMap_t *tc2PgMapping = (xpSaiTcToPgMap_t *) pMapList;
                for (i = 0; i < mapSize; ++i)
                {
                    tc2PgMapping[i].tc = (uint8_t) i % XPSAI_QOSMAP_MAX_TC_VALUE;
                }
                break;
            }
        case SAI_QOS_MAP_TYPE_PFC_PRIORITY_TO_PRIORITY_GROUP:
            {
                xpSaiPfcPrioToPgMap_t *pfc2PgMapping = (xpSaiPfcPrioToPgMap_t *) pMapList;
                for (i = 0; i < mapSize; ++i)
                {
                    pfc2PgMapping[i].pfcPriority = (uint8_t) i % XPSAI_QOSMAP_MAX_PRIORITY_VALUE;
                }
                break;
            }
        case SAI_QOS_MAP_TYPE_PFC_PRIORITY_TO_QUEUE:
            {
                xpSaiPfcPrioToQueueMap_t *pfc2QueueMapping = (xpSaiPfcPrioToQueueMap_t *)
                                                             pMapList;

                for (i = 0; i < mapSize; ++i)
                {
                    pfc2QueueMapping[i].pfcPriority = (uint8_t) i % XPSAI_QOSMAP_MAX_PRIORITY_VALUE;
                    pfc2QueueMapping[i].queue = (uint8_t) i % XPSAI_QOSMAP_MAX_QUEUE_VALUE;
                }
                break;
            }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiCreateQosMap

sai_status_t xpSaiCreateQosMap(sai_object_id_t *qos_map_id,
                               sai_object_id_t switch_id,
                               uint32_t attr_count,
                               const sai_attribute_t *attr_list)
{
    sai_object_id_t localOid = SAI_NULL_OBJECT_ID;
    xpSaiQosMapAttributesT attributes;
    xpSaiQosMap_t *pQosMapInfo = NULL;
    sai_status_t   saiStatus   = SAI_STATUS_SUCCESS;

    if (qos_map_id == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    /* Validate qos map attributes */
    saiStatus = xpSaiAttrCheck(attr_count, attr_list,
                               QOS_MAP_VALIDATION_ARRAY_SIZE, qos_map_attribs,
                               SAI_COMMON_API_CREATE);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiStatus);
        return saiStatus;
    }

    xpSaiSetDefaultQosMapAttributeVals(&attributes);
    xpSaiUpdateQosMapAttributeVals(attr_count, attr_list, &attributes);

    /* Validate input attribute values */
    for (uint32_t count = 0; count < attr_count; count++)
    {
        saiStatus = xpSaiQosMapValidateAttributeValue(attr_list[count].id, &attributes,
                                                      count);
        if (saiStatus != SAI_STATUS_SUCCESS)
        {
            return saiStatus;
        }
    }

    /* get the device id from switch object */
    xpsDevice_t xpDevId = xpSaiObjIdSwitchGet(switch_id);

    /* Create qos map object id */
    saiStatus = xpSaiQosMapCreateObject(xpDevId, &localOid);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not create qos map object, saiStatus: %d\n", saiStatus);
        return saiStatus;
    }

    sai_qos_map_list_t mapList = attributes.mapToValueList.qosmap;

    /* Create state database */
    saiStatus = xpSaiQosMapCreateStateData(localOid, attributes.type.s32,
                                           &pQosMapInfo);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not create qos map state, saiStatus: %d\n", saiStatus);
        /* Remove qos map object */
        xpSaiQosMapRemoveObject(localOid);
        return saiStatus;
    }

    /* Update state data with qos map type */
    pQosMapInfo->mapType = attributes.type.s32;

    /* Fill map keys with a default mappings */
    saiStatus = xpSaiQosMapDefaultInitializeMapToValueList(pQosMapInfo->mapType,
                                                           pQosMapInfo->numEntries, pQosMapInfo->mapList);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not initialize map with default mapping, saiStatus: %d\n",
                       saiStatus);
        xpSaiQosMapRemoveStateData(localOid);
        xpSaiQosMapRemoveObject(localOid);
        return saiStatus;
    }

    /* Configure and update state data with qos map list values */
    saiStatus = xpSaiQosMapConfigureValueList(localOid, pQosMapInfo->mapType,
                                              mapList);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not configure qos map value list, saiStatus: %d\n",
                       saiStatus);
        xpSaiQosMapRemoveStateData(localOid);
        xpSaiQosMapRemoveObject(localOid);
        return saiStatus;
    }

    *qos_map_id = localOid;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiRemoveQosMap

sai_status_t xpSaiRemoveQosMap(sai_object_id_t qosmapObjId)
{
    xpSaiQosMap_t *pQosMapInfo = NULL;
    sai_status_t   saiStatus   = SAI_STATUS_SUCCESS;

    /* Validate qos map object */
    saiStatus = xpSaiQosMapValidateQosMapObject(qosmapObjId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    /* Get the qos map state data */
    saiStatus = xpSaiQosMapGetStateData(qosmapObjId, &pQosMapInfo);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    if (pQosMapInfo->portCount > 0)
    {
        XP_SAI_LOG_ERR("Error: Object in use, unmap before remove\n");
        return SAI_STATUS_OBJECT_IN_USE;
    }

    /* No need to clean up HW configurations.
     * This QoS map is no longer used by any ports.
     * When new QoS map is created, HW config will be overwritten.
     * */
#if 0
    /* Clean up hardware configurations */
    saiStatus = xpSaiQosMapClearValueList(qosmapObjId, pQosMapInfo->mapType);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }
#endif

    /* Remove qos map object */
    saiStatus = xpSaiQosMapRemoveObject(qosmapObjId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    /* Remove state database */
    saiStatus = xpSaiQosMapRemoveStateData(qosmapObjId);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetQosMapAttribute

sai_status_t xpSaiSetQosMapAttribute(sai_object_id_t qos_map_id,
                                     const sai_attribute_t *attr)
{
    sai_status_t saiStatus   = SAI_STATUS_SUCCESS;

    /* Validate qos map attributes */
    saiStatus = xpSaiAttrCheck(1, attr,
                               QOS_MAP_VALIDATION_ARRAY_SIZE, qos_map_attribs,
                               SAI_COMMON_API_SET);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiStatus);
        return saiStatus;
    }

    /* Validate qos map object */
    saiStatus = xpSaiQosMapValidateQosMapObject(qos_map_id);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    switch (attr->id)
    {
        case SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST:
            {
                saiStatus = xpSaiSetQosMapAttrMapToValueList(qos_map_id, attr->value);
                if (SAI_STATUS_SUCCESS != saiStatus)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST)\n");
                    return saiStatus;
                }
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Error: Unknown attribute id %d\n", attr->id);
                return SAI_STATUS_INVALID_ATTR_VALUE_0;
            }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetQosMapAttribute

sai_status_t xpSaiGetQosMapAttribute(sai_object_id_t qos_map_id,
                                     uint32_t attr_count, sai_attribute_t *attr_list)
{
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;

    /* Validate qos map attributes */
    saiStatus = xpSaiAttrCheck(attr_count, attr_list,
                               QOS_MAP_VALIDATION_ARRAY_SIZE, qos_map_attribs,
                               SAI_COMMON_API_GET);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiStatus);
        return saiStatus;
    }

    /* Validate qos map object */
    saiStatus = xpSaiQosMapValidateQosMapObject(qos_map_id);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        return saiStatus;
    }

    for (uint32_t count = 0; count < attr_count; count++)
    {
        switch (attr_list[count].id)
        {
            case SAI_QOS_MAP_ATTR_TYPE:
                {
                    saiStatus = xpSaiGetQosMapAttrType(qos_map_id, &attr_list[count].value);
                    if (SAI_STATUS_SUCCESS != saiStatus)
                    {
                        XP_SAI_LOG_ERR("Failed to get (SAI_QOS_MAP_ATTR_TYPE) | retVal = %d\n",
                                       saiStatus);
                        return saiStatus;
                    }
                    break;
                }
            case SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST:
                {
                    saiStatus = xpSaiGetQosMapAttrMapToValueList(qos_map_id,
                                                                 &attr_list[count].value);
                    if (SAI_STATUS_SUCCESS != saiStatus)
                    {
                        XP_SAI_LOG_ERR("Failed to get (SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST) | retval = %d\n",
                                       saiStatus);
                        return saiStatus;
                    }
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Error: Unknown attribute id %d\n", attr_list[count].id);
                    return SAI_STATUS_INVALID_ATTR_VALUE_0;
                }
        }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiQosMapApiInit

XP_STATUS xpSaiQosMapApiInit(uint64_t flag,
                             const sai_service_method_table_t* adapHostServiceMethodTable)
{
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Calling xpSaiQosMapApiInit\n");

    _xpSaiQosMapApi = (sai_qos_map_api_t *) xpMalloc(sizeof(sai_qos_map_api_t));
    if (NULL == _xpSaiQosMapApi)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for _xpSaiQosMapApi\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }

    _xpSaiQosMapApi->create_qos_map        = xpSaiCreateQosMap;
    _xpSaiQosMapApi->remove_qos_map        = xpSaiRemoveQosMap;
    _xpSaiQosMapApi->set_qos_map_attribute = xpSaiSetQosMapAttribute;
    _xpSaiQosMapApi->get_qos_map_attribute = xpSaiGetQosMapAttribute;

    saiStatus = xpSaiApiRegister(SAI_API_QOS_MAP, (void*)_xpSaiQosMapApi);
    if (saiStatus != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Failed to register QoS API\n");
        return XP_ERR_ARRAY_OUT_OF_BOUNDS;
    }

    return XP_NO_ERR;
}

//Func: xpSaiQosMapApiDeinit

XP_STATUS xpSaiQosMapApiDeinit()
{
    XP_STATUS xpStatus = XP_NO_ERR;

    XP_SAI_LOG_DBG("Calling xpSaiQosMapApiDeinit\n");

    xpFree(_xpSaiQosMapApi);
    _xpSaiQosMapApi = NULL;

    return xpStatus;
}



// TO BE IMPLEMENTED
// Returned success due to undefined symbol error
sai_status_t xpSaiGetQosMapsObjectList(uint32_t *object_count,
                                       sai_object_key_t *object_list)
{
    return SAI_STATUS_SUCCESS;
}
