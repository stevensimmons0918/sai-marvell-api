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

#ifndef __INCLUDE_MRVL_HAL_SIM_PRV_H
#define __INCLUDE_MRVL_HAL_SIM_PRV_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef ASIC_SIMULATION

#include <cpssHalExt.h>
#include <cpss/extServices/os/gtOs/gtGenTypes.h>

/*******************************************************************************
* cpssHalInitServicesSimulationFindPCIDev
*
* DESCRIPTION:
*    Binds simulation OS functions
* INPUTS:
*       None.
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
*
*******************************************************************************/
GT_STATUS cpssHalInitServicesSimulationFindPCIDev(
    GT_PCI_DEV_VENDOR_ID *device,
    INOUT    GT_U32         *instance_ptr,
    INOUT   GT_U32          *deviceIdx_ptr,
    GT_U32          *busNo_ptr,
    GT_U32          *deviceNo_ptr,
    GT_U32          *funcNo_ptr
);

/*******************************************************************************
* cpssHalInitServicesSimulationPCIInfoUpdate
*
* DESCRIPTION:
*    Binds simulation OS functions
* INPUTS:
*       None.
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
*
*******************************************************************************/
void cpssHalInitServicesSimulationPCIInfoUpdate(

    INOUT GT_PCI_INFO *pciInfo
);

#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCLUDE_MRVL_HAL_SIM_PRV_H */
