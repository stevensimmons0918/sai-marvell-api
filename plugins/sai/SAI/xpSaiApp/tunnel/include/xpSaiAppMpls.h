// xpSaiAppMpls.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#ifndef _xpSaiAppMpls_h_
#define _xpSaiAppMpls_h_

#include "xpSaiApp.h"
#include "xpSaiAppL3.h"
#include "xpSaiAppTnl.h"
#include "xpsMpls.h"

uint8_t xpSaiAppConfigureMplsLabelEntry(xpDevice_t devId, sai_object_id_t switchOid, MplsLabelEntry *xpMplsLabelData);
uint8_t xpSaiAppRemoveMplsLabelEntry(xpDevice_t devId, uint32_t mplsLabel);
uint8_t xpSaiAppConfigureMplsTunnelEntry(xpDevice_t devId, sai_object_id_t switchOid, TunnelMplsEntry *xpTnlMplsData);
uint8_t xpSaiAppUnconfigureMplsTunnelEntry(xpDevice_t devId, uint32_t tunnelId, xpsMplsTunnelKey_t *mplsTnlKey);

#endif // _xpSaiAppMpls_h_

