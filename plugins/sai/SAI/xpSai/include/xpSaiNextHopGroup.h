// xpSaiNextHopGroup.h

/*******************************************************************************
* copyright (c) 2021 marvell. all rights reserved. the following file is       *
* subject to the limited use license agreement by and between marvell and you, *
* your employer or other entity on behalf of whom you act. in the absence of   *
* such license agreement the following file is subject to marvell’s standard   *
* limited use license agreement.                                               *
********************************************************************************/

#ifndef _xpSaiNextHopGroup_h_
#define _xpSaiNextHopGroup_h_

#include "xpSai.h"

#ifdef __cplusplus
extern "C" {
#endif

#define XP_SAI_NHGRP_MAX_SIZE XPS_L3_NHGRP_MAX_SIZE

XP_STATUS xpSaiNextHopGroupApiInit(uint64_t flag,
                                   const sai_service_method_table_t* adapHostServiceMethodTable);
XP_STATUS xpSaiNextHopGroupApiDeinit();

sai_status_t xpSaiConvertNextHopGroupOid(sai_object_id_t next_hop_group_id,
                                         uint32_t* pNhGrpId);
XP_STATUS xpSaiNextHopGroupInit(xpsDevice_t xpSaiDevId);
XP_STATUS xpSaiNextHopGroupDeInit(xpsDevice_t xpSaiDevId);

sai_status_t xpSaiMaxCountNextHopGroupAttribute(uint32_t *count);
sai_status_t xpSaiCountNextHopGroupObjects(uint32_t *count);
sai_status_t xpSaiGetNextHopGroupObjectList(uint32_t *object_count,
                                            sai_object_key_t *object_list);
sai_status_t xpSaiBulkGetNextHopGroupAttributes(sai_object_id_t id,
                                                uint32_t *attr_count, sai_attribute_t *attr_list);

/**
 * \brief API to increment Route reference count of a particular nhGrpId in Sai DB
 *
 * \param [in] nhGrpId
 *
 * \return SAI Status
 */
sai_status_t xpSaiIncrementNextHopGroupRouteRefCount(uint32_t nhGrpId,
                                                     bool isRouteNH);

/**
 * \brief API to decrement Route reference count of a particular nhGrpId in Sai DB
 *
 * \param [in] nhGrpId
 *
 * \return SAI Status
 */
sai_status_t xpSaiDecrementNextHopGroupRouteRefCount(uint32_t nhGrpId,
                                                     bool isRouteNH);

sai_status_t xpSaiIsAclNextHopGroupInUse(uint32_t nhGrpId);

#ifdef __cplusplus
}
#endif

#endif //_xpSaiNextHopGroup_h_
