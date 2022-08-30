// xpSaiStp.h

/*******************************************************************************
* copyright (c) 2021 marvell. all rights reserved. the following file is       *
* subject to the limited use license agreement by and between marvell and you, *
* your employer or other entity on behalf of whom you act. in the absence of   *
* such license agreement the following file is subject to marvell’s standard   *
* limited use license agreement.                                               *
********************************************************************************/

#ifndef _xpSaiStp_h_
#define _xpSaiStp_h_

#include "xpSai.h"

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

typedef struct _xpSaiStpAttributesT
{
    sai_attribute_value_t vlanList;
    sai_attribute_value_t portList;
} xpSaiStpAttributesT;

typedef struct _xpSaiStpPortAttributesT
{
    sai_attribute_value_t stp;
    sai_attribute_value_t port;
    sai_attribute_value_t state;
} xpSaiStpPortAttributesT;

typedef struct _xpSaiStpPortAttrEntry_t
{
    xpsStp_t stpId;
    xpsPort_t portId;
    xpsInterfaceId_t intfId;
    xpsStpState_e state;
} xpSaiStpPortAttrEntry_t;


XP_STATUS xpSaiStpApiInit(uint64_t flag,
                          const sai_service_method_table_t* adapHostServiceMethodTable);
XP_STATUS xpSaiStpApiDeinit();

sai_status_t xpSaiMaxCountStpAttribute(uint32_t *count);
sai_status_t xpSaiCountStpObjects(uint32_t *count);
sai_status_t xpSaiGetStpObjectList(uint32_t *object_count,
                                   sai_object_key_t *object_list);
sai_status_t xpSaiBulkGetStpAttributes(sai_object_id_t id, uint32_t *attr_count,
                                       sai_attribute_t *attr_list);

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*_xpSaiStp_h_*/
