/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/********************************************************************************
* cpssHalPhyConfig.h
*
* DESCRIPTION:
*       initialize PHY system
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#ifndef _cpssHalPhyConfig_h_
#define _cpssHalPhyConfig_h_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/generic/cpssTypes.h>

#if defined(INCLUDE_MPD)

GT_STATUS cpssHalPhyMpdInit
(
    GT_U8          devNum,
    XP_DEV_TYPE_T  xpDevType,
    PROFILE_STC    profile[]
);

int cpssHalMpdPortDisable
(
    GT_SW_DEV_NUM            devNum,
    GT_U32                   portNum,
    GT_BOOL                  disable
);

#endif


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*_cpssHalPhyConfig_h_*/



