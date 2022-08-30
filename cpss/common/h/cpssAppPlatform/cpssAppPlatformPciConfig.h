/*******************************************************************************
*              (c), Copyright 2018, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file cpssAppPlatformPciConfig.h
*
* @brief Prestera Devices pci initialization & detection module.
*
* @version   1
********************************************************************************
*/

#ifndef __CPSS_APP_PLATFORM_PCICONFIG_H
#define __CPSS_APP_PLATFORM_PCICONFIG_H

#include <cpss/common/cpssTypes.h>

typedef struct {
    GT_U16 vendorId;
    GT_U16 devId;
} PCI_DEV_VENDOR_ID_STC;

typedef struct {
    PCI_DEV_VENDOR_ID_STC pciDevVendorId;
    GT_U8 pciBus;
    GT_U8 pciDev;
    GT_U8 pciFunc;
    GT_U32 pciHeaderInfo[16];
} PCI_INFO_STC;

/**
* @internal prvCpssAppPlatformSysGetPciInfo function
* @endinternal
*
* @brief  Gets the Pci device info.
*
* IN devIdx           -   device number
* IN pciBus           -   PCI Bus number
* IN pciDev           -   PCI device number
* IN pciFunc          -   PCI Function number
* OUT devFamily       -   Device family type
* OUT pciInfo         -   PCI info of the device
*
* @retval GT_OK       - on success,
* @retval GT_FAIL     - otherwise.
*/
GT_STATUS prvCpssAppPlatformSysGetPciInfo
(
    IN  GT_U8 devIdx,
    IN  GT_U8 pciBus,
    IN  GT_U8 pciDev,
    IN  GT_U8 pciFunc,
    OUT CPSS_PP_FAMILY_TYPE_ENT *devFamily,
    OUT PCI_INFO_STC *pciInfo
);

#endif /* __CPSS_APP_PLATFORM_PCICONFIG_H */
