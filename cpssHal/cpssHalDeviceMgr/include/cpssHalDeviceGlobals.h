/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

// cpssHalDeviceGlobals.h


#ifndef _cpssHalDeviceGlobals_h_
#define _cpssHalDeviceGlobal_h_

#define MAX_GLOBAL_DEVICES          2
#define MAX_PP_DEVICES_PER_SWITCH   2
//TODO_RPR
#define MAX_PORTS_PER_PP            1024
#define MAX_CASCADE_PORTS_PER_PP    6

#define CPSS_GLOBAL_SWITCH_ID_0     0

/* Hard coded the max entries as per the observation on HW.
 * Sonic can program more entries than this which will lead to crash.
 * Later, a command will be added in sonic for restricting the entries programmed based on HW capability.
 */
#define ALD_IPV4_ROUTE_ENTRY_MAX 16384
#define ALD_IPV6_ROUTE_ENTRY_MAX 3072


#endif /* _cpssHalDeviceGlobals_h_ */
