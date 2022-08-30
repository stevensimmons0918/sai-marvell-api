// xpSaiSchedulerGroup.h

/*******************************************************************************
* copyright (c) 2021 marvell. all rights reserved. the following file is       *
* subject to the limited use license agreement by and between marvell and you, *
* your employer or other entity on behalf of whom you act. in the absence of   *
* such license agreement the following file is subject to marvell’s standard   *
* limited use license agreement.                                               *
********************************************************************************/

#ifndef _xpSaiSchedulerGroup_h_
#define _xpSaiSchedulerGroup_h_

#include "xpSai.h"

#ifdef __cplusplus
extern "C" {
#endif

#define XPSAI_SG_NUM_HIERARCHY_LEVELS    2
#define XPSAI_SG_MAX_CHILDS_AT_LEVEL0    8
#define XPSAI_SG_MAX_CHILDS_AT_LEVEL1    1
#define XPSAI_SG_MAX_GROUPS_PER_PORT     1 + XPSAI_SG_MAX_CHILDS_AT_LEVEL0

#define XPSAI_SC_NODE_INDEX_OFFSET       0
#define XPSAI_SC_NODE_INDEX_LEN          16
#define XPSAI_SC_LEVEL_OFFSET            16
#define XPSAI_SC_LEVEL_LEN               16

#define XPSAI_SC_NODE_INDEX_VAL_MASK     (((1 << XPSAI_SC_NODE_INDEX_LEN) -1) << XPSAI_SC_NODE_INDEX_OFFSET)
#define XPSAI_SC_LEVEL_VAL_MASK          (((1 << XPSAI_SC_LEVEL_LEN) -1) << XPSAI_SC_LEVEL_OFFSET)

typedef struct _xpSaiSchedulerGroupAttributesT
{
    sai_attribute_value_t
    childCount;         /* Number of queues/groups childs added to scheduler group */
    sai_attribute_value_t
    childList;          /* Scheduler Group child obejct id List */
    sai_attribute_value_t portId;             /* Scheduler group on port */
    sai_attribute_value_t level;              /* Scheduler group level */
    sai_attribute_value_t
    maxChilds;          /* Maximum Number of childs on group */
    sai_attribute_value_t schedulerProfileId; /* Scheucler Id */
    sai_attribute_value_t parentNode;         /* Scheduler group parent node */
} xpSaiSchedulerGroupAttributesT;

typedef struct xpSaiSchedulerGrp_t
{
    sai_object_id_t   schedulerGrpObjId;  /* Scheduler group object id */
    sai_uint32_t
    childCount;         /* Number of queues/groups childs added to scheduler group */
    sai_object_list_t childList;          /* Scheduler Group child obejct id List */
    sai_object_id_t   portId;             /* Scheduler group on port */
    sai_uint32_t      level;              /* Scheduler group level */
    sai_uint32_t      maxChilds;          /* Maximum Number of childs on group */
    sai_object_id_t   schedulerProfileId; /* Scheucler Id */
    sai_object_id_t   parentNode;         /* Scheduler group parent node */
} xpSaiSchedulerGrp_t;

XP_STATUS xpSaiSchedulerGroupApiInit(uint64_t flag,
                                     const sai_service_method_table_t* adapHostServiceMethodTable);
XP_STATUS xpSaiSchedulerGroupApiDeinit();
XP_STATUS xpSaiSchedulerGroupInit(xpsDevice_t xpDevId);
XP_STATUS xpSaiSchedulerGroupDeInit(xpsDevice_t xpDevId);
XP_STATUS xpSaiSchedulerGroupBindChildToParent(sai_object_id_t parentObjId,
                                               sai_object_id_t childObjId);
XP_STATUS xpSaiSchedulerGroupUnbindChildFromParent(sai_object_id_t parentObjId,
                                                   sai_object_id_t childObjId);
XP_STATUS xpSaiSchedulerGroupGetNodeToPortPath(xpsDevice_t xpDevId,
                                               uint32_t nodeKey, xpTxqQueuePathToPort_t* pPath);
XP_STATUS xpSaiSchedulerGroupGetKeyFromLevelAndNodeIdx(uint32_t level,
                                                       uint32_t nodeIdx, uint32_t* key);
XP_STATUS xpSaiSchedulerGroupGetLevelAndNodeIdxFromKey(uint32_t key,
                                                       uint32_t* level, uint32_t* nodeIdx);

sai_status_t xpSaiSchedulerGroupCreate(sai_object_id_t switchObjId,
                                       xpSaiSchedulerGroupAttributesT *attributes,
                                       sai_object_id_t *pSchedulerGrpObjId);
sai_status_t xpSaiSchedulerGroupRemove(sai_object_id_t schedulerGrpObjId);

#ifdef __cplusplus
}
#endif

#endif //_xpSaiSchedulerGroup_h_
