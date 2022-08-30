// xpSaiQueue.h

/*******************************************************************************
* copyright (c) 2021 marvell. all rights reserved. the following file is       *
* subject to the limited use license agreement by and between marvell and you, *
* your employer or other entity on behalf of whom you act. in the absence of   *
* such license agreement the following file is subject to marvell’s standard   *
* limited use license agreement.                                               *
********************************************************************************/

#ifndef _xpSaiQueue_h_
#define _xpSaiQueue_h_

#include "xpSai.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SAI_QOS_Q_OBJID_VALUE_BITS      16
#define SAI_QOS_Q_OBJID_VALUE_MASK      0x000000000000FFFFULL
#define SAI_QOS_Q_OBJID_PORT_MASK      (0x000000000000FFFFULL << SAI_QOS_Q_OBJID_VALUE_BITS)

typedef struct _xpSaiQueueAttributesT
{
    sai_attribute_value_t type;                 /* Queue type [sai_queue_type_t] */
    sai_attribute_value_t port;                 /* Pord id [sai_object_id_t] */
    sai_attribute_value_t index;                /* Queue index [sai_uint8_t] */
    sai_attribute_value_t
    parentSchedulerNode;  /* Parent scheduler node [sai_object_id_t] */
    sai_attribute_value_t
    wredProfileId;        /* WRED ID to queue [sai_object_id_t] */
    sai_attribute_value_t
    bufferProfileId;      /* Buffer profile to queue [sai_object_id_t] */
    sai_attribute_value_t
    schedulerProfileId;   /* Scheduler to queue [sai_object_id_t] */
} xpSaiQueueAttributesT;

typedef struct xpSaiQueueQosInfo_t
{
    sai_object_id_t  queueId;             /* Queue object id */
    sai_object_id_t  switchId;            /* Switch id */
    sai_queue_type_t type;                /* Queue type */
    sai_object_id_t
    parentSchedulerNode; /* Parent scheduler node [sai_object_id_t] */
    sai_object_id_t  schedObjId;          /* Scheduler to Queue map */
    sai_object_id_t  wredObjId;           /* WRED to Queue map */
    sai_object_id_t  bufferProfileId;     /* Buffer profile to queue map */
    /* queue statistics */
    sai_uint64_t passPkts;
    sai_uint64_t passBytes;
    sai_uint64_t dropPkts;
    sai_uint64_t dropBytes;
    sai_uint64_t watermark;
    /* queue statistics for PG */
    sai_uint64_t pg_passPkts;
    sai_uint64_t pg_passBytes;
    sai_uint64_t pg_dropPkts;
    sai_uint64_t pg_dropBytes;
    /* WRED statistics for Queue*/
    sai_uint64_t wredDropPkts;
    sai_uint64_t wredDropBytes;
    /* WRED statistics for the Port that the queue belong to */
    sai_uint64_t port_wredDropPkts;
    sai_uint64_t port_wredDropBytes;

    /* TODO: This is the count of COPP policer drop packets of last read.
       This is a workaround.
       (Policer counter work in read mode, not in read-clear mode) */
    sai_uint64_t CoppPolicerDropPktsOfLastRead;
} xpSaiQueueQosInfo_t;

XP_STATUS xpSaiQueueApiInit(uint64_t flag,
                            const sai_service_method_table_t* adapHostServiceMethodTable);
XP_STATUS xpSaiQueueApiDeinit();
XP_STATUS xpSaiQueueInit(xpsDevice_t xpSaiDevId);
XP_STATUS xpSaiQueueDeInit(xpsDevice_t xpSaiDevId);
sai_status_t xpSaiQosQueueObjIdCreate(sai_object_type_t sai_object_type,
                                      sai_uint32_t xpsIntfId, sai_uint32_t localQId, sai_object_id_t *queue_id);
sai_status_t xpSaiQosQueueObjIdPortValueGet(sai_object_id_t queue_id,
                                            sai_uint32_t *xpsIntfId, sai_uint32_t *localQId);
sai_status_t xpSaiQueueSetSchedulerInfo(sai_object_id_t queueId,
                                        sai_object_id_t schedulerId);
sai_status_t xpSaiMaxCountQueueAttribute(uint32_t *count);
sai_status_t xpSaiCountQueueObjects(uint32_t *count);
sai_status_t xpSaiGetQueueObjectList(uint32_t *object_count,
                                     sai_object_key_t *object_list);
sai_status_t xpSaiBulkGetQueueAttributes(sai_object_id_t id,
                                         uint32_t *attr_count, sai_attribute_t *attr_list);
sai_status_t xpSaiCreateQueue(sai_object_id_t *queue_id,
                              sai_object_id_t switch_id, uint32_t attr_count, const
                              sai_attribute_t *attr_list);
XP_STATUS xpSaiQueueGetPortAndQueueFromKey(uint32_t key, uint32_t* port,
                                           uint32_t* queue);
XP_STATUS xpSaiQueueGetKeyFromPortAndQueue(uint32_t port, uint32_t queue,
                                           uint32_t* key);
sai_status_t xpSaiSetQueueAttrParentSchedulerNode(sai_object_id_t queue_id,
                                                  sai_attribute_value_t value);
sai_status_t xpSaiRemoveQueue(sai_object_id_t queue_id);
sai_status_t xpSaiQueueTxSchedulerProfileInit(xpsDevice_t xpSaiDevId,
                                              sai_uint32_t xpPort);
sai_status_t xpSaiSetQueueAttrBufferProfileId(sai_object_id_t queueObjId,
                                              sai_attribute_value_t value);
sai_status_t xpSaiSetQueueAttrSchedulerProfileId(sai_object_id_t queueObjId,
                                                 sai_attribute_value_t value);
sai_status_t xpSaiQueueApplyBufferProfile(sai_object_id_t queueObjId,
                                          sai_object_id_t profileObjId);
sai_status_t xpSaiGetQueueStats(sai_object_id_t queue_id,
                                uint32_t number_of_counters, const sai_stat_id_t *counter_ids,
                                uint64_t *counters);

sai_status_t xpSaiQueueStatisticsStoreDB(sai_object_id_t queue_id,
                                         xpSaiQueueQosInfo_t **ppQueueQosInfo);


#ifdef __cplusplus
}
#endif

#endif //_xpSaiQueue_h_
