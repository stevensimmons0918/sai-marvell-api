// xpSaiVirtualRouter.h

/*******************************************************************************
* copyright (c) 2021 marvell. all rights reserved. the following file is       *
* subject to the limited use license agreement by and between marvell and you, *
* your employer or other entity on behalf of whom you act. in the absence of   *
* such license agreement the following file is subject to marvell’s standard   *
* limited use license agreement.                                               *
********************************************************************************/

#ifndef _xpSaiVirtualRouter_h_
#define _xpSaiVirtualRouter_h_

#include "xpSai.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef struct _xpSaiVirtualRouterAttributesT
{
    sai_attribute_value_t adminV4State;
    sai_attribute_value_t adminV6State;
    sai_attribute_value_t srcMacAddress;
    sai_attribute_value_t violationTtl1Action;
    sai_attribute_value_t violationIpOptions;
    sai_attribute_value_t unkL3MCPktAction;
} xpSaiVirtualRouterAttributesT;

sai_status_t xpSaiCreateDefaultVirtualRouter(xpsDevice_t xpSaiDevId);

sai_status_t xpSaiConvertVirtualRouterOid(sai_object_id_t vr_id,
                                          uint32_t* pVrfId);
XP_STATUS xpSaiVirtualRouterApiInit(uint64_t flag,
                                    const sai_service_method_table_t* adapHostServiceMethodTable);
XP_STATUS xpSaiVirtualRouterApiDeinit();

sai_status_t xpSaiMaxCountVirtualRouterAttribute(uint32_t *count);
sai_status_t xpSaiCountVirtualRouterObjects(uint32_t *count);
sai_status_t xpSaiGetVirtualRouterObjectList(uint32_t *object_count,
                                             sai_object_key_t *object_list);
sai_status_t xpSaiBulkGetVirtualRouterAttributes(sai_object_id_t id,
                                                 uint32_t *attr_count, sai_attribute_t *attr_list);


#ifdef __cplusplus
}
#endif

#endif //_xpSaiVirtualRouter_h_
