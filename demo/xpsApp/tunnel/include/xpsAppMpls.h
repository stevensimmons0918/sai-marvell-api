// xpsAppMpls.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#ifndef _xpsAppMpls_h_
#define _xpsAppMpls_h_

#include "xpsApp.h"
#include "xpsAppL3.h"
#include "xpsAppTnl.h"
#include "xpsMpls.h"

uint8_t xpsAppConfigureMplsLabelEntry(xpDevice_t devId, MplsLabelEntry *xpMplsLabelData);
uint8_t xpsAppRemoveMplsLabelEntry(xpDevice_t devId, uint32_t mplsLabel);
uint8_t xpsAppConfigureMplsTunnelEntry(xpDevice_t devId, TunnelMplsEntry *xpTnlMplsData);
uint8_t xpsAppUnconfigureMplsTunnelEntry(xpDevice_t devId, uint32_t tunnelId, xpsMplsTunnelKey_t *mplsTnlKey);

#endif // _xpsAppMpls_h_

