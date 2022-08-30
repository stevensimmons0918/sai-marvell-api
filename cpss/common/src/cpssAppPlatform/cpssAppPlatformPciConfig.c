/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file cpssAppPlatformPciConfig.c
*
* @brief Prestera Devices pci initialization & detection module.
*
* @version   1
********************************************************************************
*/

#include <cpssAppPlatformSysConfig.h>
#include <cpssAppPlatformPciConfig.h>
#include <gtExtDrv/drivers/gtPciDrv.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>



#define END_OF_TABLE    0xFFFFFFFF
/* DB to hold the device types that the CPSS support */
extern const struct {
    CPSS_PP_FAMILY_TYPE_ENT    devFamily;
    GT_U32                     numOfPorts;
    const CPSS_PP_DEVICE_TYPE *devTypeArray;
    const CPSS_PORTS_BMP_STC  *defaultPortsBmpPtr;
    const GT_VOID_PTR         *coresInfoPtr;
    const GT_U32              *activeCoresBmpPtr;
}cpssSupportedTypes[];

/**
* @internal getDeviceFamily function
* @endinternal
*
* @brief   Gets the device family from vendor Id and device Id (read from PCI bus)
*
* @param[in] pciDevVendorIdPtr     - pointer to PCI/PEX device identification data.
*
* @param[out] devFamilyPtr         - pointer to CPSS PP device family.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS getDeviceFamily
(
    IN PCI_DEV_VENDOR_ID_STC    *pciDevVendorIdPtr,
    OUT CPSS_PP_FAMILY_TYPE_ENT *devFamilyPtr
)
{
    CPSS_PP_DEVICE_TYPE deviceType = pciDevVendorIdPtr->devId << 16 | pciDevVendorIdPtr->vendorId;
    GT_U32  i = 0;
    GT_U32  j = 0;
    GT_BOOL found = GT_FALSE;

    for (i=0; cpssSupportedTypes[i].devFamily != END_OF_TABLE; i++)
    {
        for (j=0; cpssSupportedTypes[i].devTypeArray[j] != END_OF_TABLE; j++)
        {
            if (deviceType == cpssSupportedTypes[i].devTypeArray[j])
            {
                found = GT_TRUE;
                break;
            }
        }
        if(found == GT_TRUE)
            break;
    }

    if (GT_TRUE == found)
    {
        *devFamilyPtr = cpssSupportedTypes[i].devFamily;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal prvCpssAppPlatformGetPciDev function
* @endinternal
*
* @brief  Gets the Pci device info.
* IN pciBus           -   PCI Bus number
* IN pciDev           -   PCI device number
* IN pciFunc          -   PCI function number
*
* OUT vendorId        -   PCI vendor id
* OUT deviceId        -   PCI device Id
*
* @retval GT_OK       - on success.
* @retval GT_FAIL     - otherwise.
*/
GT_STATUS prvCpssAppPlatformGetPciDev
(
    IN  GT_U8  pciBus,
    IN  GT_U8  pciDev,
    IN  GT_U8  pciFunc,
    OUT GT_U16 *vendorId,
    OUT GT_U16 *deviceId
)
{
    GT_STATUS rc;

    rc = extDrvPciGetDev(pciBus, pciDev, pciFunc, vendorId, deviceId);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, extDrvPciGetDev);

    return rc;
}

/**
* @internal prvCpssAppPlatformSysGetPciInfo function
* @endinternal
*
* @brief  Gets the Pci device info.
* IN devIdx           -   device number
* IN pciBus           -   PCI Bus number
* IN pciDev           -   PCI device number
* OUT devFamily       -   Device family type
* OUT pciInfo         -   PCI info of the device
*
* @retval GT_OK       - on success,
* @retval GT_FAIL     - otherwise.
*/
GT_STATUS prvCpssAppPlatformSysGetPciInfo
(
    IN GT_U8 devIdx,
    IN GT_U8 pciBus,
    IN GT_U8 pciDev,
    IN GT_U8 pciFunc,
    OUT CPSS_PP_FAMILY_TYPE_ENT *devFamily,
    OUT PCI_INFO_STC *pciInfo
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    devId = 0;
    (void)devIdx;

    rc = prvCpssAppPlatformGetPciDev(pciBus, pciDev, pciFunc, &(pciInfo->pciDevVendorId.vendorId),
            &(pciInfo->pciDevVendorId.devId));
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvCpssAppPlatformGetPciDev);

    rc = extDrvPciConfigDev(pciBus, pciDev, 0);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, extDrvPciConfigDev);

    rc = getDeviceFamily(&pciInfo->pciDevVendorId, devFamily);
    if(rc != GT_OK)
    {
        CPSS_APP_PLATFORM_LOG_ERR_MAC("getDeviceFamily rc=%d ret=%d", rc, GT_FAIL);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    pciInfo->pciDev  = pciDev;
    pciInfo->pciBus  = pciBus;
    pciInfo->pciFunc = pciFunc;

    devId = (pciInfo->pciDevVendorId.devId << 16) | (pciInfo->pciDevVendorId.vendorId);

    CPSS_APP_PLATFORM_LOG_INFO_MAC("Found device:[0x%8.8x] on pciBus %d pciDev %d\n",devId, pciInfo->pciBus, pciInfo->pciDev);

    return rc;
}
