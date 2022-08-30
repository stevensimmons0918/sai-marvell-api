// xpSaiQosMap.h

/*******************************************************************************
* copyright (c) 2021 marvell. all rights reserved. the following file is       *
* subject to the limited use license agreement by and between marvell and you, *
* your employer or other entity on behalf of whom you act. in the absence of   *
* such license agreement the following file is subject to marvell’s standard   *
* limited use license agreement.                                               *
********************************************************************************/

#ifndef _xpSaiQosMap_h_
#define _xpSaiQosMap_h_

#include "xpSai.h"

#ifdef __cplusplus
extern "C" {
#endif

#define XPSAI_QOSMAP_MIN_QUEUE_VALUE            0
#define XPSAI_QOSMAP_MAX_QUEUE_VALUE            8
#define XPSAI_QOSMAP_MAX_PCP_VALUE              8
#define XPSAI_QOSMAP_MIN_PCP_VALUE              0
#define XPSAI_QOSMAP_MAX_DEI_VALUE              2
#define XPSAI_QOSMAP_MIN_DEI_VALUE              0
#define XPSAI_QOSMAP_MAX_DSCP_VALUE             64
#define XPSAI_QOSMAP_MIN_DSCP_VALUE             0
#define XPSAI_QOSMAP_MAX_TC_VALUE               8
#define XPSAI_QOSMAP_MIN_TC_VALUE               0
#define XPSAI_QOSMAP_MAX_DP_VALUE               3
#define XPSAI_QOSMAP_MIN_DP_VALUE               0
#define XPSAI_QOSMAP_MAX_PRIORITY_VALUE         8
#define XPSAI_QOSMAP_MIN_PRIORITY_VALUE         0
#define XPSAI_QOSMAP_MAX_QOSMAP_PROFILES        12
#define XPSAI_QOSMAP_IS_QUEUE_VALUE_VALID(x)    (((x) >= XPSAI_QOSMAP_MIN_QUEUE_VALUE)    && ((x) < XPSAI_QOSMAP_MAX_QUEUE_VALUE))
#define XPSAI_QOSMAP_IS_PRIORITY_VALUE_VALID(x) (((x) >= XPSAI_QOSMAP_MIN_PRIORITY_VALUE) && ((x) < XPSAI_QOSMAP_MAX_PRIORITY_VALUE))
#define XPSAI_QOSMAP_IS_PCP_VALUE_VALID(x)      (((x) >= XPSAI_QOSMAP_MIN_PCP_VALUE)      && ((x) < XPSAI_QOSMAP_MAX_PCP_VALUE))
#define XPSAI_QOSMAP_IS_DSCP_VALUE_VALID(x)     (((x) >= XPSAI_QOSMAP_MIN_DSCP_VALUE)     && ((x) < XPSAI_QOSMAP_MAX_DSCP_VALUE))
#define XPSAI_QOSMAP_IS_DP_VALUE_VALID(x)       (((x) >= XPSAI_QOSMAP_MIN_DP_VALUE)       && ((x) < XPSAI_QOSMAP_MAX_DP_VALUE))
#define XPSAI_QOSMAP_IS_TC_VALUE_VALID(x)       (((x) >= XPSAI_QOSMAP_MIN_TC_VALUE)       && ((x) < XPSAI_QOSMAP_MAX_TC_VALUE))
#define XPSAI_QOSMAP_IS_TYPE_VALID(x)           (((x) >= SAI_QOS_MAP_TYPE_DOT1P_TO_TC)    && ((x) < SAI_QOS_MAP_TYPE_CUSTOM_RANGE_BASE))

typedef struct xpSaiTcToPgMap_t
{
    sai_uint8_t tc;
    sai_uint8_t pg;
} xpSaiTcToPgMap_t;

typedef struct xpSaiQosMap_t
{
    sai_object_id_t qosmapObjId;               // qos map object id
    sai_int32_t     mapType;                   // qos map type
    sai_pointer_t   mapList;                   // pointer to qos map value list
    sai_uint32_t
    numEntries;                // Number of qos map value list entries
    sai_uint32_t
    portCount;                 // counter to count number of ports to which qos map is attached
    sai_uint32_t
    portList[SYSTEM_MAX_PORT]; // port list to which qos map is attached
} xpSaiQosMap_t;

XP_STATUS xpSaiQosMapApiInit(uint64_t flag,
                             const sai_service_method_table_t* adapHostServiceMethodTable);
XP_STATUS xpSaiQosMapApiDeinit();

sai_status_t xpSaiQosMapInit(xpsDevice_t xpDevId);
sai_status_t xpSaiQosMapDeInit(xpsDevice_t xpDevId);
sai_status_t xpSaiQosMapValidateQosMapObject(sai_object_id_t qosmapObjId);
sai_status_t xpSaiQosMapValidateQosMapType(sai_object_id_t qosmapObjId,
                                           sai_int32_t mapType);
sai_status_t xpSaiQosMapAddPortToPortList(sai_object_id_t qosMapObjId,
                                          sai_uint32_t portNum);
sai_status_t xpSaiQosMapRemovePortFromPortList(sai_object_id_t qosmapObjId,
                                               sai_uint32_t portNum);
sai_uint32_t xpSaiQosMapIsIbufferSupported(xpsDevice_t xpDevId);
sai_status_t xpSaiQosMapGetPortIngressProfileId(sai_object_id_t portObjId,
                                                sai_object_id_t dot1pToTcObjId, sai_object_id_t dot1pToDpObjId,
                                                sai_object_id_t dscpToTcObjId, sai_object_id_t dscpToDpObjId,
                                                uint32_t *pProfileId);
sai_status_t xpSaiQosMapClearPortIngressProfileId(sai_object_id_t portObjId,
                                                  sai_object_id_t dot1pToTcObjId, sai_object_id_t dot1pToDpObjId,
                                                  sai_object_id_t dscpToTcObjId, sai_object_id_t dscpToDpObjId);
sai_status_t xpSaiQosMapGetPortEgressProfileId(sai_object_id_t portObjId,
                                               sai_object_id_t tcDpToDot1pObjId,
                                               sai_object_id_t tcDptoDscpObjId, uint32_t *pProfileId);
sai_status_t xpSaiQosMapClearPortEgressProfileId(sai_object_id_t portObjId,
                                                 sai_object_id_t tcDpToDot1pObjId,
                                                 sai_object_id_t tcDptoDscpObjId);
sai_status_t xpSaiMaxCountQosMapsAttribute(uint32_t *count);
sai_status_t xpSaiCountQosMapsObjects(uint32_t *count);
sai_status_t xpSaiGetQosMapsObjectList(uint32_t *object_count,
                                       sai_object_key_t *object_list);
sai_status_t xpSaiGetQosMapAttribute(sai_object_id_t qos_map_id,
                                     uint32_t attr_count, sai_attribute_t *attr_list);
sai_status_t xpSaiQosMapSetTrafficClassForL3QosProfile(xpsDevice_t xpDevId,
                                                       uint32_t profileId, sai_object_id_t qosmapObjId, bool isClear);
sai_status_t xpSaiQosMapSetColorForL3QosProfile(xpsDevice_t xpDevId,
                                                uint32_t profileId, sai_object_id_t qosmapObjId, bool isClear);

sai_status_t xpSaiQosMapGetStateData(sai_object_id_t qosmapObjId,
                                     xpSaiQosMap_t **ppQosMapInfo);


#ifdef __cplusplus
}
#endif

#endif //_xpSaiQosMap_h_
