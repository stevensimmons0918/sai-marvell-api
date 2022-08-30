// xpSaiPolicer.h

/*******************************************************************************
* copyright (c) 2021 marvell. all rights reserved. the following file is       *
* subject to the limited use license agreement by and between marvell and you, *
* your employer or other entity on behalf of whom you act. in the absence of   *
* such license agreement the following file is subject to marvell’s standard   *
* limited use license agreement.                                               *
********************************************************************************/

#ifndef _xpSaiPolicer_h_
#define _xpSaiPolicer_h_

#include "xpSai.h"
#include "xpsPolicer.h"

#ifdef __cplusplus
extern "C" {
#endif

#define XPSAI_DEFAULT_POLICER_IDX_LIST_SIZE    4
#define XPSAI_BYTES_TO_KILOBYTES(x)    (((x) != 0) ? (uint32_t)ceil((x) / 1.0E3 + 0.5f) : 0)

typedef struct _xpSaiPolicerAttributesT
{
    sai_attribute_value_t meterType;
    sai_attribute_value_t mode;
    sai_attribute_value_t colorSource;
    sai_attribute_value_t cbs;
    sai_attribute_value_t cir;
    sai_attribute_value_t pbs;
    sai_attribute_value_t pir;
    sai_attribute_value_t greenPacketAction;
    sai_attribute_value_t yellowPacketAction;
    sai_attribute_value_t redPacketAction;
    sai_attribute_value_t enableCounterList;
    /* policer statistics */
    sai_uint64_t greenPackets;
    sai_uint64_t greenBytes;
    sai_uint64_t yellowPackets;
    sai_uint64_t yellowBytes;
    sai_uint64_t redPackets;
    sai_uint64_t redBytes;
} xpSaiPolicerAttributes_t;

typedef struct xpSaiPolicerIdxMapList_t
{
    xpsPolicerType_e client;
    sai_uint32_t  index;
    sai_uint32_t  counterIndex;
} xpSaiPolicerIdxMapList_t;

typedef struct xpSaiPolicer_t
{
    sai_object_id_t policerId;
    xpsDevice_t     xpDevId;
    sai_uint16_t    numOfIdx;
    xpSaiPolicerAttributes_t attrInfo;
    xpSaiPolicerIdxMapList_t indexList[XPSAI_DEFAULT_POLICER_IDX_LIST_SIZE];
} xpSaiPolicer_t;

XP_STATUS xpSaiPolicerApiInit(uint64_t flag,
                              const sai_service_method_table_t* adapHostServiceMethodTable);
XP_STATUS xpSaiPolicerApiDeinit();
XP_STATUS xpSaiPolicerInit(xpsDevice_t xpSaiDevId);
XP_STATUS xpSaiPolicerDeInit(xpsDevice_t xpSaiDevId);
XP_STATUS xpSaiGetPolicerAttrInfo(sai_object_id_t policer_id,
                                  xpSaiPolicerAttributes_t *pPolicerAttrInfo);
sai_status_t xpSaiPolicerGetXpsEntry(sai_object_id_t policerOid,
                                     xpsPolicerEntry_t *policerEntry);
XP_STATUS xpSaiPolicerAddIdxToIdxList(sai_object_id_t policerId,
                                      xpsPolicerType_e client, sai_uint32_t index);
XP_STATUS xpSaiPolicerRemoveIdxFromIdxList(sai_object_id_t policerId,
                                           xpsPolicerType_e client, sai_uint32_t index);
sai_status_t xpSaiMaxCountPolicerAttribute(uint32_t *count);
sai_status_t xpSaiCountPolicerObjects(uint32_t *count);
sai_status_t xpSaiGetPolicerObjectList(uint32_t *object_count,
                                       sai_object_key_t *object_list);
sai_status_t xpSaiGetPolicerAttribute(sai_object_id_t policer_id,
                                      uint32_t attr_count, sai_attribute_t *attr_list);
sai_status_t xpSaiConvertPolicerOid(sai_object_id_t policer_oid,
                                    uint32_t* pPolicerId);
sai_status_t xpSaiPolicerValidatePolicerObject(sai_object_id_t policerObjId);

sai_status_t xpSaiGetPolicerStats(sai_object_id_t policer_oid,
                                  uint32_t number_of_counters, const sai_stat_id_t *counter_ids,
                                  uint64_t *counters);


#ifdef __cplusplus
}
#endif

#endif //_xpSaiPolicer_h_
