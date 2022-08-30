// xpSaiBuffer.h

/*******************************************************************************
* copyright (c) 2021 marvell. all rights reserved. the following file is       *
* subject to the limited use license agreement by and between marvell and you, *
* your employer or other entity on behalf of whom you act. in the absence of   *
* such license agreement the following file is subject to marvell’s standard   *
* limited use license agreement.                                               *
********************************************************************************/

#ifndef _xpSaiBuffer_h_
#define _xpSaiBuffer_h_

#include "xpSai.h"

#ifdef __cplusplus
extern "C" {
#endif

#define XPSAI_INGRESS_PRIORITY_GROUP_INDEX_MAX  (256) /* Based on max possible index value in SAI */

#define XPSAI_BUFFER_DEFAULT_XON_TH             (32767 * XPSAI_BUFFER_PAGE_SIZE_BYTES)
#define XPSAI_BUFFER_DEFAULT_XOFF_TH            (65535 * XPSAI_BUFFER_PAGE_SIZE_BYTES)
#define XPSAI_BUFFER_DEFAULT_DYNAMIC_POOL_TH    (65535 * XPSAI_BUFFER_PAGE_SIZE_BYTES)

#define XPSAI_BUFFER_LENGTH_MAX_THRESHOLD       (0x3FFFF)
#define XPSAI_BUFFER_PAGE_LENGTH_MAX_THRESHOLD  (0xFF)
#define XPSAI_BUFFER_MAP_LIST_SIZE              (128)
#define XPSAI_BUFFER_PAGE_SIZE_BYTES            ((IS_DEVICE_FALCON(xpSaiSwitchDevTypeGet()))?(128):(256))
#define XPSAI_BYTES_TO_PAGES(x)                 ((IS_DEVICE_FALCON(xpSaiSwitchDevTypeGet()))?\
                                                    (((x) / 128) + (!!((x) % 128))):\
                                                    (((x) / 256) + (!!((x) % 256))))
#define XPSAI_BUFFER_TOTAL_BUFFER_BYTES         (xpSaiBufferGetPacketMemorySize() * 1024)

#define XP80SAI_BUFFER_TOTAL_BUFFER_SIZE            (96 * 256) /* Switch total buffer size in KB */
#define XP70SAI_BUFFER_TOTAL_BUFFER_SIZE            (64 * 256) /* Switch total buffer size in KB */
#define FALCON_12_8_SAI_BUFFER_TOTAL_BUFFER_SIZE    (4*12*1024)/*Single tile = 12MB  ,4 tiles= 48 MB*/
#define FALCON_6_4_SAI_BUFFER_TOTAL_BUFFER_SIZE     (2*12*1024)/*Single tile = 12MB  ,2 tiles= 24 MB*/
#define FALCON_3_2_SAI_BUFFER_TOTAL_BUFFER_SIZE     (1*12*1024)/*Single tile = 12MB  ,1 tile = 12 MB*/
#define FALCON_2_SAI_BUFFER_TOTAL_BUFFER_SIZE       (2*12*1024)/*Single tile = 12MB  ,2 tile = 24 MB*/
#define AC5X_SAI_BUFFER_TOTAL_BUFFER_SIZE           (1*3*1024)/* Total buffer size 3MB  */


#define XPSAI_BUFFER_IS_POOL_TYPE_VALID(x)      ((((x) == SAI_BUFFER_POOL_TYPE_INGRESS) || ((x) == SAI_BUFFER_POOL_TYPE_EGRESS)) ? 1 : 0)
#define XPSAI_BUFFER_IS_POOL_THMODE_VALID(x)    ((((x) == SAI_BUFFER_POOL_THRESHOLD_MODE_STATIC) || ((x) == SAI_BUFFER_POOL_THRESHOLD_MODE_DYNAMIC)) ? 1 : 0)

typedef struct _xpSaiIngressPriorityGroupAttributesT
{
    sai_attribute_value_t bufferProfile;   /* Pointer to buffer profile object id */
} xpSaiIngressPriorityGroupAttributesT;

typedef struct _xpSaiBufferProfileAttributesT
{
    sai_attribute_value_t poolId;          /* Pointer to buffer pool object id */
    sai_attribute_value_t bufferSize;      /* Reserved buffer size in bytes */
    sai_attribute_value_t
    thMode;          /* Shared threshold mode for the buffer profile */
    sai_attribute_value_t
    sharedDynamicTh; /* Dynamic threshold for the shared usage */
    sai_attribute_value_t
    sharedStaticTh;  /* Static threshold for the shared usage in bytes */
    sai_attribute_value_t
    xoffTh;          /* Buffer profile XOFF threshold in bytes */
    sai_attribute_value_t
    xonTh;           /* Buffer profile XON threshold in bytes */
    sai_attribute_value_t
    xonOffsetTh;     /* Buffer profile XON hysteresis threshold in byte */
} xpSaiBufferProfileAttributesT;

typedef struct _xpSaiBufferPoolAttributesT
{
    sai_attribute_value_t sharedSize;  /* Shared buffer size in bytes */
    sai_attribute_value_t
    type;        /* Buffer pool type [sai_buffer_pool_type_t] */
    sai_attribute_value_t size;        /* Buffer pool size in bytes */
    sai_attribute_value_t
    thMode;      /* Shared threshold mode for the buffer pool [sai_buffer_threshold_mode_t] */
    sai_attribute_value_t
    xoffSize;    /* Shared headroom pool size in bytes for lossless traffic */
} xpSaiBufferPoolAttributesT;

typedef struct xpSaiBufferPool_t
{
    sai_object_id_t                   poolObjId;    /* Key: Buffer pool object id */
    sai_uint32_t
    dynPoolId;    /* Dynamic pool id for shared buffer */
    sai_uint32_t
    sharedSize;   /* Shared buffer size in bytes */
    sai_buffer_pool_type_t            type;         /* Buffer pool type */
    sai_uint32_t                      size;         /* Buffer pool size in bytes */
    sai_buffer_pool_threshold_mode_t
    thMode;       /* Shared threshold mode for the buffer pool */
    sai_uint32_t
    xoffSize;     /* Shared headroom pool size in bytes for lossless traffic */

    /* pool statistics */
    sai_uint64_t
    watermark;            /* pool watermark in pages */
    sai_uint64_t
    curOccupancy;         /* pool current occupancy in pages */
    sai_uint64_t
    headroomWatermark;    /* pool's headroom watermark in pages */
    sai_uint64_t
    headroomCurOccupancy; /* pool's headroom current occupancy in pages */

    sai_uint32_t
    profileCount; /* Number of profiles to which pool is assigned */
    sai_object_id_t
    profileList[XPSAI_BUFFER_MAP_LIST_SIZE]; /* List of profile objects */
} xpSaiBufferPool_t;

typedef struct xpSaiBufferProfile_t
{
    sai_object_id_t
    profileObjId;     /* Key: Buffer profile object id*/
    sai_object_id_t
    poolObjId;        /* Pointer to buffer pool object id */
    sai_uint32_t
    bufferSize;       /* Reserved buffer size in bytes */
    sai_buffer_profile_threshold_mode_t
    thMode;           /* Shared threshold mode for the buffer profile */
    sai_int8_t
    sharedDynamicTh;  /* Dynamic threshold for the shared usage */
    sai_uint32_t
    sharedStaticTh;   /* Static threshold for the shared usage in bytes */
    sai_uint32_t
    xoffTh;           /* Buffer profile XOFF threshold in bytes */
    sai_uint32_t
    xonTh;            /* Buffer profile XON threshold in bytes */
    sai_uint32_t
    xonOffsetTh;      /* Buffer profile XON hysteresis threshold in byte */
    sai_uint32_t
    objCount;         /* Number of objects to which buffer profile is assigned */
    sai_object_id_t
    objList[XPSAI_BUFFER_MAP_LIST_SIZE]; /* List of pg/queue objects */
} xpSaiBufferProfile_t;

typedef struct xpSaiIngressPg_t
{
    sai_object_id_t priorityGroupId;    /* Key */
    sai_object_id_t bufferProfileId;    /* Value */
    sai_object_id_t portId;             /* Value */
    sai_uint8_t     pgIndex;            /* Value */

    /* PG statistics */
    sai_uint64_t    watermark;
    sai_uint64_t    curOccupancy;
    sai_uint64_t    headroomCurOccupancy;
    sai_uint64_t    headroomWatermark;
    sai_uint64_t    packets;
    sai_uint64_t    bytes;
    sai_uint64_t    dropPackets;
    sai_uint64_t    dropBytes;
} xpSaiIngressPg_t;

XP_STATUS xpSaiBufferApiInit(uint64_t flag,
                             const sai_service_method_table_t* adapHostServiceMethodTable);
XP_STATUS xpSaiBufferApiDeinit();

sai_status_t xpSaiBufferInit(xpsDevice_t xpsDevId);
sai_status_t xpSaiBufferDeInit(xpsDevice_t xpsDevId);

sai_status_t xpSaiBufferAddPortToBufferProfile(sai_object_id_t
                                               buffer_profile_id, sai_object_id_t port_id);
sai_status_t xpSaiBufferRemovePortFromBufferProfile(sai_object_id_t
                                                    buffer_profile_id, sai_object_id_t port_id);

sai_status_t xpSaiBufferAddQueueToBufferProfile(sai_object_id_t
                                                buffer_profile_id, sai_object_id_t queue_id);
sai_status_t xpSaiBufferRemoveQueueFromBufferProfile(sai_object_id_t
                                                     buffer_profile_id, sai_object_id_t queue_id);

sai_status_t xpSaiBufferPoolGetDynamicPoolId(sai_object_id_t poolObjId,
                                             sai_uint32_t *pDynPoolId);
sai_status_t xpSaiBufferProfileValidateProfileObject(sai_object_id_t
                                                     bufferProfileObjId);

sai_status_t xpSaiMaxCountBufferPoolAttribute(uint32_t *count);
sai_status_t xpSaiMaxCountBufferProfileAttribute(uint32_t *count);

sai_status_t xpSaiCountBufferPoolObjects(uint32_t *count);
sai_status_t xpSaiCountBufferProfileObjects(uint32_t *count);

sai_status_t xpSaiGetBufferPoolObjectList(uint32_t *object_count,
                                          sai_object_key_t *object_list);
sai_status_t xpSaiGetBufferProfileObjectList(uint32_t *object_count,
                                             sai_object_key_t *object_list);

sai_status_t xpSaiBulkGetBufferPoolAttributes(sai_object_id_t id,
                                              uint32_t *attr_count, sai_attribute_t *attr_list);
sai_status_t xpSaiBulkGetBufferProfileAttributes(sai_object_id_t id,
                                                 uint32_t *attr_count, sai_attribute_t *attr_list);

sai_uint32_t xpSaiBufferGetPacketMemorySize();
sai_status_t xpSaiBufferProfileGetStateData(sai_object_id_t profileObjId,
                                            xpSaiBufferProfile_t **ppProfileInfo);

sai_status_t xpSaiBufferCreateIngressPriorityGroupObject(
    sai_object_id_t *ingress_priority_group_id, sai_object_id_t switch_id,
    uint32_t attr_count, const sai_attribute_t *attr_list);
sai_status_t xpSaiBufferRemoveIngressPriorityGroupObject(
    sai_object_id_t ingress_priority_group_id);
bool         xpSaiBufferTotalThrottleGet(xpSaiBufferProfile_t *pProfileInfo);


#ifdef __cplusplus
}
#endif

#endif //_xpSaiBuffer_h_
