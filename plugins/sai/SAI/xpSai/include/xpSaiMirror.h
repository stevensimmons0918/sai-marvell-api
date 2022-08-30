// xpSaiMirror.h

/*******************************************************************************
* copyright (c) 2021 marvell. all rights reserved. the following file is       *
* subject to the limited use license agreement by and between marvell and you, *
* your employer or other entity on behalf of whom you act. in the absence of   *
* such license agreement the following file is subject to marvell’s standard   *
* limited use license agreement.                                               *
********************************************************************************/

#ifndef _xpSaiMirror_h_
#define _xpSaiMirror_h_

#include "xpSai.h"

#define SAI_IPHDR_VERSION_IPV4       4
#define SAI_IPHDR_VERSION_IPV6       6

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

typedef struct xpSaiPortMirrorEnabled
{
    bool ingressEn;
    bool egressEn;
} xpSaiPortMirrorEnabled_t;

typedef struct xpSaiMirrorSession_t
{
    uint32_t                  keySessionId;              // session ID
    sai_mirror_session_type_t sessionType;
    uint32_t                  analyzerId;                // XPS ingress analyzer Id
    uint32_t
    analyzerRefCnt;     // XPS ingress analyzer reference counter
    xpSaiPortMirrorEnabled_t
    mirroredPortList[SYSTEM_MAX_PORT]; // List of mirrored ports
    sai_object_id_t           monitorPortOid;
    xpsInterfaceId_t          monitorIntf;
    bool                      monitorPortListValid;
    uint16_t                  vlanId;
    bool                      vlanHdrValid;
    uint8_t                   vlanPri;
    uint8_t                   vlanCfi;
    uint16_t                  vlanTpid;
    sai_erspan_encapsulation_type_t encapType;
    uint8_t                   ipType;
    sai_ip_address_t          srcIpAddr;
    sai_ip_address_t          dstIpAddr;
    sai_mac_t                 macSA;
    sai_mac_t                 macDA;
    xpsInterfaceId_t
    tnlIntfId;         // Tunnel interface Id used for ERSPAN2 mode
    uint32_t                  tos;
    uint32_t                  greProtoType;
    sai_object_id_t           policer;
    uint8_t                   ttlHopLimit;
    uint32_t                  truncateSize;
    uint8_t                   tc;
    uint32_t                  sampleRate;
} xpSaiMirrorSession_t;


sai_status_t xpSaiMirrorApiInit(uint64_t flag,
                                const sai_service_method_table_t* adapHostServiceMethodTable);
sai_status_t xpSaiMirrorApiDeInit();
sai_status_t xpSaiMirrorSessionInit(xpsDevice_t xpsDevId);
sai_status_t xpSaiMirrorSessionDeinit(xpsDevice_t xpsDevId);
sai_status_t xpSaiCountMirrorObjects(uint32_t *count);
sai_status_t xpSaiGetMirrorObjectList(uint32_t *object_count,
                                      sai_object_key_t *object_list);
sai_status_t xpSaiBulkGetMirrorAttributes(sai_object_id_t id,
                                          uint32_t *attr_count, sai_attribute_t *attr_list);
sai_status_t xpSaiMirrorAttributeSet(sai_object_id_t session_id,
                                     const sai_attribute_t *attr);
sai_status_t xpSaiMirrorSessionPortMirrorEnable(xpsInterfaceId_t portIntf,
                                                sai_object_id_t session_id, bool ingress, bool enable);
sai_status_t xpSaiMaxCountMirrorAttribute(uint32_t *count);

sai_status_t xpSaiMirrorAclMirrorEnable(sai_object_id_t session_id,
                                        bool ingress, bool enable,
                                        uint32_t* analyzerId, sai_mirror_session_type_t *sessionType);

sai_status_t xpSaiMirrorDbInfoCreate(xpsDevice_t xpsDevId,
                                     uint32_t internalSessionId, xpSaiMirrorSession_t **xpSaiMirrorSession);

sai_status_t xpSaiMirrorSessionAdd(xpsDevice_t xpsDevId,
                                   xpSaiMirrorSession_t *session);

sai_status_t xpSaiMirrorDbInfoGet(sai_object_id_t session_id,
                                  xpSaiMirrorSession_t **xpSaiMirrorSessionEntry);

sai_status_t xpSaiMirrorSessionUpdate(xpsDevice_t xpsDevId,
                                      xpSaiMirrorSession_t *session, const sai_attribute_t *attr);

sai_status_t xpSaiMirrorSessionCreate(sai_object_id_t* session_id,
                                      sai_object_id_t switch_id,
                                      uint32_t attr_count,
                                      const sai_attribute_t *attr_list);

sai_status_t xpSaiMirrorSessionRemove(sai_object_id_t session_id);

sai_status_t xpSaiMirrorGetSessionAnalyzerId(sai_object_id_t session_id,
                                             uint32_t *analyzerId);

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /* _xpSaiMirror_h_ */
