// xpSaiAppQos.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#ifndef _xpSaiAppQos_h_
#define _xpSaiAppQos_h_

#include "xpSaiApp.h"
#include "xpSaiQueue.h"

#define XPSAI_MAX_AQM_PROFILES 128

/* QosMaps */
XP_STATUS xpSaiAppConfigureIngressQosMapEntry(xpDevice_t devId, sai_object_id_t switchOid, qosIngressMapDataEntry *pIngressMapData);
XP_STATUS xpSaiAppConfigureEgressQosMapEntry(xpDevice_t devId, sai_object_id_t switchOid, qosEgressMapDataEntry *pEgressMapData);

/* Scheduler */
XP_STATUS xpSaiAppConfigureQueueSchedulerEntry(xpDevice_t devId, sai_object_id_t switchOid, saiSchedulerDataEntry *pSchedulerData);
XP_STATUS xpSaiAppConfigureShaperPortEntry(xpDevice_t devId, sai_object_id_t switchOid, qosShaperPortDataEntry *pShaperPortData);

/* Wred */
XP_STATUS xpSaiAppConfigureWredEntry(xpDevice_t devId, sai_object_id_t switchOid, qosAqmQProfileDataEntry *pWredData);

/* Policer */
XP_STATUS xpSaiAppConfigurePolicerEntry(xpDevice_t devId, sai_object_id_t switchOid, policerDataEntry *pPolicerData);

#endif // _xpSaiAppQos_h_

