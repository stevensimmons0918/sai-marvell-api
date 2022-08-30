/* xpSaiScheduler.h */

/*******************************************************************************
* copyright (c) 2021 marvell. all rights reserved. the following file is       *
* subject to the limited use license agreement by and between marvell and you, *
* your employer or other entity on behalf of whom you act. in the absence of   *
* such license agreement the following file is subject to marvell’s standard   *
* limited use license agreement.                                               *
********************************************************************************/

#ifndef _xpSaiScheduler_h_
#define _xpSaiScheduler_h_

#include "xpSai.h"

#ifdef __cplusplus
extern "C" {
#endif

#define XPSAI_SCHEDULER_DEFAULT_SCHEDULING    SAI_SCHEDULING_TYPE_DWRR
#define XPSAI_SCHEDULER_DEFAULT_WEIGHT        100
#define XPSAI_SCHEDULER_MAP_LIST_SIZE         4

#define XPSAI_SCHEDULER_IS_WEIGHT_VALID(x)    ((((x) > 0) && ((x) <= 100)) ? 1 : 0)

typedef enum xpSaiSchedulerNodeType_e
{
    XPSAI_SCHEDULER_QUEUE_NODE,
    XPSAI_SCHEDULER_LEVEL2_NODE,
    XPSAI_SCHEDULER_LEVEL1_NODE,
    XPSAI_SCHEDULER_LEVEL0_NODE,
    XPSAI_SCHEDULER_PORT_NODE,
    XPSAI_SCHEDULER_MAX_NODE,
} xpSaiSchedulerNodeType_e;

typedef struct xpSaiSchedulerMapList_t
{
    xpSaiSchedulerNodeType_e nodeType;
    sai_uint32_t             nodeIndex;
} xpSaiSchedulerMapList_t;

typedef struct _xpSaiSchedulerAttributesT
{
    sai_attribute_value_t schedulingType;
    sai_attribute_value_t schedulingWeight;
    sai_attribute_value_t meterType;
    sai_attribute_value_t minBandwidthRate;
    sai_attribute_value_t minBandwidthBurstRate;
    sai_attribute_value_t maxBandwidthRate;
    sai_attribute_value_t maxBandwidthBurstRate;
} xpSaiSchedulerAttributesT;

typedef struct xpSaiSchedulerProfile_t
{
    sai_object_id_t           schedObjId;    /* Scheduler object id */
    xpSaiSchedulerAttributesT attrInfo;      /* Scheduler attributes info */
    sai_uint32_t
    numNodes;      /* Number of queues/scheduler groups/ports to which scheduler is attached */
    xpSaiSchedulerMapList_t
    nodeList[XPSAI_SCHEDULER_MAP_LIST_SIZE]; /* List of queues/scheduler groups/ports nodes */
} xpSaiSchedulerProfile_t;

XP_STATUS xpSaiSchedulerApiInit(uint64_t flag,
                                const sai_service_method_table_t* adapHostServiceMethodTable);
XP_STATUS xpSaiSchedulerApiDeinit();
XP_STATUS xpSaiSchedulerInit(xpsDevice_t xpSaiDevId);
XP_STATUS xpSaiSchedulerDeInit(xpsDevice_t xpSaiDevId);
XP_STATUS xpSaiSchedulerBindQueueToScheduler(sai_object_id_t schedObjId,
                                             sai_object_id_t queueObjId);
XP_STATUS xpSaiSchedulerUnbindQueueFromScheduler(sai_object_id_t schedObjId,
                                                 sai_object_id_t queueObjId);
XP_STATUS xpSaiSchedulerBindPortToScheduler(sai_object_id_t schedObjId,
                                            sai_uint32_t xpsIntfId);
XP_STATUS xpSaiSchedulerUnbindPortFromScheduler(sai_object_id_t schedObjId,
                                                sai_uint32_t xpsIntfId);
XP_STATUS xpSaiSchedulerBindSchedulerGrpToScheduler(sai_object_id_t schedObjId,
                                                    sai_object_id_t schedGrpObjId);
XP_STATUS xpSaiSchedulerUnbindSchedulerGrpFromScheduler(
    sai_object_id_t schedObjId, sai_object_id_t schedGrpObjId);
XP_STATUS xpSaiGetSchedulerAttrInfo(sai_object_id_t schedObjId,
                                    xpSaiSchedulerAttributesT *schedulerAttr);
XP_STATUS xpSaiSetSchedulerPortSchedulingType(uint32_t xpDevId, uint32_t xpPort,
                                              uint32_t schedType);

sai_status_t xpSaiSetSchedulerQueueSchedulingType(uint32_t xpDevId,
                                                  uint32_t xpPort, uint32_t queue, uint32_t schedType);
sai_status_t xpSaiSetSchedulerQueueShaperMaxRate(uint32_t xpDevId,
                                                 uint32_t xpPort, uint32_t queue, uint64_t bandwidthRate,
                                                 uint64_t bandwidthBurstRate);
sai_status_t xpSaiSetSchedulerQueueShaperMinRate(uint32_t xpDevId,
                                                 uint32_t xpPort, uint32_t queue, uint64_t bandwidthRate,
                                                 uint64_t bandwidthBurstRate);
sai_status_t xpSaiSetSchedulerPortShaperRate(uint32_t xpDevId, uint32_t xpPort,
                                             uint64_t bandwidthRate, uint64_t bandwidthBurstRate);

sai_status_t xpSaiMaxCountSchedulerAttribute(uint32_t *count);
sai_status_t xpSaiCountSchedulerObjects(uint32_t *count);
sai_status_t xpSaiGetSchedulerObjectList(uint32_t *object_count,
                                         sai_object_key_t *object_list);
sai_status_t xpSaiBulkGetSchedulerAttributes(sai_object_id_t id,
                                             uint32_t *attr_count, sai_attribute_t *attr_list);

sai_status_t xpSaiCreateScheduler(sai_object_id_t *scheduler_id,
                                  sai_object_id_t switch_id, uint32_t attr_count,
                                  const sai_attribute_t *attr_list);

#ifdef __cplusplus
}
#endif

#endif //_xpSaiScheduler_h_
