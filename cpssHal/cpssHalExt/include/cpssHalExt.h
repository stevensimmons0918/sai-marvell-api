/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/********************************************************************************
* cpssHalInitServices.h
*
* DESCRIPTION:
*       Services initialization
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#ifndef __INCLUDE_MRVL_HAL_EXT_PRV_H
#define __INCLUDE_MRVL_HAL_EXT_PRV_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/generic/cpssTypes.h>
#include <gtExtDrv/drivers/gtPciDrv.h>
#include <cpss/extServices/cpssExtServices.h>

/*******************************************************************************
* cpssHalInitServicesGetDefaultExtDrvFuncs
*
* DESCRIPTION:
*       Receives default cpss bind from extDrv
* INPUTS:
*       None.
* OUTPUTS:
*       extDrvFuncBindInfoPtr - (pointer to) set of call back functions
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       Function should reside into CPSS library to resolve correct
*       pointers to functions.
*
*******************************************************************************/
extern GT_STATUS cpssHalInitServicesGetDefaultExtDrvFuncs
(
    CPSS_EXT_DRV_FUNC_BIND_STC  *extDrvFuncBindInfoPtr
);

/*******************************************************************************
* cpssHalInitServicesGetDefaultOsBindFuncs
*
* DESCRIPTION:
*       Receives default cpss bind from OS
* INPUTS:
*       None.
* OUTPUTS:
*       osFuncBindPtr - (pointer to) set of call back functions
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       Function should reside into CPSS library to resolve correct
*       pointers to functions.
*
*******************************************************************************/

GT_STATUS cpssHalInitServicesGetDefaultOsBindFuncs
(
    CPSS_OS_FUNC_BIND_STC *osFuncBindPtr
);

/*******************************************************************************
* cpssHalInitServicesGetDefaultTraceFuncs
*
* DESCRIPTION:
*       Receives default cpss bind from trace
* INPUTS:
*       None.
* OUTPUTS:
*       extDrvFuncBindInfoPtr - (pointer to) set of call back functions
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       Function should reside into CPSS library to resolve correct
*       pointers to functions.
*
*******************************************************************************/
GT_STATUS cpssHalInitServicesGetDefaultTraceFuncs
(
    CPSS_TRACE_FUNC_BIND_STC  *traceFuncBindInfoPtr
);

/*
 * typedef: struct GT_PCI_DEV_VENDOR_ID
 *
 * Description: PCI device and vendor ID struct
 *
 * Fields:
 *   vendorId - The Prestera PCI vendor Id.
 *   devId    - The different Prestera PCI device Id.
 */
typedef struct
{
    GT_U16  vendorId;
    GT_U16  devId;
} GT_PCI_DEV_VENDOR_ID;

/*
 * typedef: struct GT_PCI_INFO
 *
 * Description: PCI device information
 *
 * Fields:
 *      pciDevVendorId - The Prestera PCI vendor Id.
 *      pciBaseAddr    - The different Prestera PCI device Id.
 *      pciIdSel       - The device PCI ID_SEL.
 *      pciBusNum      - The PCI bus number.
 *      funcNo         - The PCI device function number
 *      internalPciBase- Base address to which the internal pci registers
 *                       are mapped to.
 *      resetAndInitControllerBase - Base address to which Reset and Init
 *                       Controller (a.k.a. DFX server) are mapped to.
 *                       (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; Bobcat3)
 *      resourceMapping - resource mapping info (mapping of MG, SWICTH, DFX, SRAM)
 *
 */
typedef struct
{
    GT_PCI_DEV_VENDOR_ID    pciDevVendorId;
    GT_UINTPTR              pciBaseAddr;
    GT_UINTPTR              internalPciBase;
    GT_UINTPTR              resetAndInitControllerBase;
    GT_EXT_DRV_PCI_MAP_STC  resourceMapping;
    GT_U32                  pciIdSel;
    GT_U32                  pciBusNum;
    GT_U32                  funcNo;
    GT_U32                  pciHeaderInfo[16];
} GT_PCI_INFO;

/*******************************************************************************
* cpssHalInitServicesGetPciDev
*
* DESCRIPTION:
*       This routine search for Prestera Devices Over the PCI.
*
* INPUTS:
*       first - whether to bring the first device, if GT_FALSE return the next
*               device.
*       devices - array with PCI device information.
*
* OUTPUTS:
*       pciInfo - the next device PCI info.
*
* RETURNS:
*       GT_OK      - on success.
*       GT_FAIL    - otherwise.
*       GT_NO_MORE - no more prestera devices.
*
* COMMENTS:
*       Last GT_PCI_DEV_VENDOR_ID structure in devices array must be filled with zeros.
*
*******************************************************************************/
GT_STATUS cpssHalInitServicesGetPciDev
(
    IN  GT_BOOL             first,
    IN GT_PCI_DEV_VENDOR_ID *device,
    GT_PCI_INFO         *pciInfo
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*__INCLUDE_MRVL_HAL_EXT_PRV_H*/
