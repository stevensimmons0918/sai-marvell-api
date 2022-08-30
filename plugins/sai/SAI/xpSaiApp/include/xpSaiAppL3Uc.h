// xpSaiAppL3Uc.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#ifndef _xpSaiAppL3Uc_h_
#define _xpSaiAppL3Uc_h_

#include "xpTypes.h"

XP_STATUS xpSaiAppL3UcPLInit(void);
XP_STATUS xpSaiAppL3UcFLInit(void);
XP_STATUS xpSaiAppL3UcDevInit(xpDevice_t devId, XP_PIPE_MODE mode);
XP_STATUS xpSaiAppL3UcAddPrefix (xpDevice_t devId);
XP_STATUS xpSaiAppL3UcDisplayPrefixes (xpDevice_t devId);
XP_STATUS xpSaiAppL3UcAddIpv6Prefix (xpDevice_t devId);
XP_STATUS xpSaiAppL3UcNhProgramming(xpDevice_t devId);
#endif  //_xpSaiAppL3Uc_h_
