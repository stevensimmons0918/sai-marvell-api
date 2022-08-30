/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/********************************************************************************
* cpssHalSimulation.h
*
* DESCRIPTION:
*       initialize system
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#ifndef __INCLUDE_MRVL_HAL_SIM_API_H
#define __INCLUDE_MRVL_HAL_SIM_API_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef ASIC_SIMULATION

#include <cpss/extServices/os/gtOs/gtGenTypes.h>
#include <cpssHalProfile.h>

/*******************************************************************************
* cpssHalSimInit
*
* DESCRIPTION:
*       Initialize PP simulation
*
*
* APPLICABLE DEVICES:
*        Bobcat3.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                    - on success
*       GT_FAIL                  - on general error
*
* COMMENTS:
*       Before calling cpssHalSimInit commandLine string must be set in the following format:
*           appName -e "simulation ini file". Example: appDemoSim -e bobcat3_A0_pss_wm.ini.
*
*******************************************************************************/
GT_STATUS cpssHalSimInitApi(
    PROFILE_STC * profile
);

#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCLUDE_MRVL_HAL_SIM_API_H */
