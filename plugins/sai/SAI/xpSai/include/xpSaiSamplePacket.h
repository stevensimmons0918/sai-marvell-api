// xpSaiSamplePacket.h

/*******************************************************************************
* copyright (c) 2021 marvell. all rights reserved. the following file is       *
* subject to the limited use license agreement by and between marvell and you, *
* your employer or other entity on behalf of whom you act. in the absence of   *
* such license agreement the following file is subject to marvell’s standard   *
* limited use license agreement.                                               *
********************************************************************************/

#ifndef _xpSaiSamplePacket_h_
#define _xpSaiSamplePacket_h_

#include "xpSai.h"

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

#define XPSAI_SLOW_HIGH_RELATIVE_ERROR 1.0
#define XPSAI_SLOW_MID_RELATIVE_ERROR 5.0
#define XPSAI_MAX_SFLOW_M_VALUE 23u
#define XPSAI_MAX_SFLOW_EXP_VALUE 15u
#define XPSAI_MAX_SFLOW_BASE_VALUE 255u
#define XPSAI_MAX_SFLOW_SAMPLE_VALUE 255u
#define XPSAI_SAMPLER_DEFAULT_BASE_VALUE 1u
#define XPSAI_SAMPLER_DEFAULT_SAMPLE_VALUE 1u
#define XPSAI_SAMPLER_CLEAR_ON_READ 1  // Indicates whether the counters should be cleared when read. Ignored for sampling.
#define XPSAI_SAMPLER_WRAP_AROUND 1    // Indicates whether the counters should wrap around or stick at their maximum value. Ignored for sampling.
#define XPSAI_SAMPLER_COUNT_OFFSET 15  // Which event in the 16 event to start sampling.

#define XPSAI_PORT_SAMPLER_DISABLE 0
#define XPSAI_PORT_SAMPLER_ENABLE 1

typedef struct _xpSaiSamplePacketAttributesT
{
    sai_uint32_t            samplePacketAttrRate;
    sai_samplepacket_type_t samplePacketAttrType;
    sai_samplepacket_mode_t samplePacketAttrMode;
} xpSaiSamplePacketAttributesT;

typedef struct xpsSamplePacket_t
{
    uint32_t keyXpsSflowId;
    sai_uint32_t rate;
    sai_uint32_t sampleRefCnt;
    sai_samplepacket_type_t type;
    sai_samplepacket_mode_t mode;
    sai_object_id_t mirrorSessionId;
} xpsSamplePacket_t;

int xpSaiSamplePacketInit(xpsDevice_t xpSaiDevId);
int xpSaiSamplePacketDeinit(xpsDevice_t xpSaiDevId);
XP_STATUS xpSaiSamplePacketApiInit(uint64_t flag,
                                   const sai_service_method_table_t* adapHostServiceMethodTable);
XP_STATUS xpSaiSamplePacketApiDeinit();

sai_status_t xpSaiSamplePacketSessionCreate(sai_object_id_t* session_id,
                                            uint32_t attr_count,
                                            const sai_attribute_t *attr_list);
sai_status_t xpSaiSamplePacketSessionRemove(sai_object_id_t session_id);
sai_status_t xpSaiSetSamplePacketAttribute(sai_object_id_t session_id,
                                           const sai_attribute_t *attr);

sai_status_t xpSaiSamplePacketAttrSampleRateSet(sai_object_id_t session_id,
                                                sai_uint32_t rate);
sai_status_t xpSaiSamplePacketAttrSampleTypeSet(sai_object_id_t session_id,
                                                sai_samplepacket_type_t type);
sai_status_t xpSaiSamplePacketAttrSampleModeSet(sai_object_id_t session_id,
                                                sai_samplepacket_mode_t mode);
sai_status_t xpSaiSamplePacketSampleRateGet(sai_object_id_t session_id,
                                            sai_uint32_t *rate);
sai_status_t xpSaiSamplePacketSampleTypeGet(sai_object_id_t session_id,
                                            sai_samplepacket_type_t *type);
sai_status_t xpSaiSamplePacketSampleModeGet(sai_object_id_t session_id,
                                            sai_samplepacket_mode_t *mode);
sai_status_t xpSaiSamplePacketIngressPortEnable(sai_object_id_t port_id,
                                                sai_attribute_value_t value);
sai_status_t xpSaiSamplePacketEgressPortEnable(sai_object_id_t port_id,
                                               sai_attribute_value_t value);
sai_status_t xpSaiCountSamplePacketObjects(uint32_t *count);
sai_status_t xpSaiMaxCountSamplePacketAttribute(uint32_t *count);
sai_status_t xpSaiGetSamplePacketObjectList(uint32_t *object_count,
                                            sai_object_key_t *object_list);
sai_status_t xpSaiBulkGetSamplePacketAttributes(sai_object_id_t id,
                                                uint32_t *attr_count, sai_attribute_t *attr_list);
sai_status_t xpSaiSamplePacketDbInfoGet(sai_object_id_t session_id,
                                        xpsSamplePacket_t **xpSamplePacketEntry);

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*_xpSaiSamplePacket_h_*/
