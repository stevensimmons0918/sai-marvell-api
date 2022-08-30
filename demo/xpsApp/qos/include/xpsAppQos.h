// xpsAppQos.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#ifndef _xpsAppQos_h_
#define _xpsAppQos_h_

#include "xpsApp.h"

#define MAX_AQM_PROFILES 128

XP_STATUS xpsAppConfigureQosSchedulerEntry(xpDevice_t devId, qosSchedulerDataEntry *qosSchdData);
XP_STATUS xpsAppConfigureQosShaperPortEntry(xpDevice_t devId, qosShaperPortDataEntry *qosShaperPortData);
XP_STATUS xpsAppConfigureQosShaperQueueEntry(xpDevice_t devId, qosShaperQueueDataEntry *qosShaperQueueData);
XP_STATUS xpsAppConfigureQosAqmQProfileEntry(xpDevice_t devId, qosAqmQProfileDataEntry *qosAqmQProfileData);
XP_STATUS xpsAppConfigureEgressQosMapEntry(xpDevice_t devId, qosEgressMapDataEntry *qosEgressMapData); 

#endif // _xpsAppQos_h_

