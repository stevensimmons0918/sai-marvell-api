/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

//
// file cpssHalCascadeMgr.h
//

#ifndef _cpssHalCascadeMgr_h_
#define _cpssHalCascadeMgr_h_

#define B2B_FIRST_DEV 0
#define B2B_SECOND_DEV 1

GT_STATUS cpssHalCascadeMgrInitCascadeTrunk
(
    uint32_t cscdTrunkId
);
void cpssHalCascadeMgrDumpTrunkPorts(int switchId);
XP_STATUS cpssHalCascadeAddPortsToVlan(xpsVlan_t vlanId);
XP_STATUS cpssHalCascadeRemovePortsFromVlan(xpsVlan_t vlanId);

#endif //_cpssHalCascadeMgr_h_
