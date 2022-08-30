// xpSaiCopp.h

/*******************************************************************************
* copyright (c) 2021 marvell. all rights reserved. the following file is       *
* subject to the limited use license agreement by and between marvell and you, *
* your employer or other entity on behalf of whom you act. in the absence of   *
* such license agreement the following file is subject to marvell’s standard   *
* limited use license agreement.                                               *
********************************************************************************/

#ifndef _xpSaiCopp_h_
#define _xpSaiCopp_h_

#include "xpSai.h"

#ifdef __cplusplus
extern "C" {
#endif

#define XPSAI_INVALID_POLICER_ID    (0xFFFFFFFF)

sai_status_t xpSaiCoppInit(xpsDevice_t xpsDevId);
sai_status_t xpSaiCoppDeInit(xpsDevice_t xpsDevId);

sai_status_t xpSaiCoppEntryAdd(xpsDevice_t xpsDevId, const bool *validPort,
                               uint32_t policerId, xpsPktCmd_e pktCmd, uint32_t reasonCode,
                               xpsPktCmd_e defaultPktCmd);
sai_status_t xpSaiCoppEntryRemove(xpsDevice_t xpsDevId, uint32_t reasonCode);

sai_status_t xpSaiCoppQueueMappingSet(xpsDevice_t xpsDevId, uint32_t queue,
                                      uint32_t reasonCode);

sai_status_t xpSaiCoppEntryGetPolicerIdFromPort(xpsDevice_t xpsDevId,
                                                uint32_t reasonCode, uint32_t xpPortIdx, uint32_t *policerId);

#ifdef __cplusplus
}
#endif

#endif //_xpSaiCopp_h_
