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
* @file gtAppDemoGenSysConfig.c
*
* @brief General System configuration and initialization control.
*
*/
#include <appDemo/os/appOs.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfigDefaults.h>
#include <appDemo/boardConfig/appDemoBoardConfig.h>

#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <gtExtDrv/drivers/gtGenDrv.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/* flag to indicate wthether this is normal or hir app */
GT_BOOL isHirApp;


/*******************************************************************************
 * Global variables
 ******************************************************************************/

/* becomes TRUE after first initialization of the system, stays TRUE after
 the shutdown */
GT_BOOL systemInitialized = GT_FALSE;

/* appDemoPpConfigList - Holds the Pp's configuration parameters.
   allocate additional entries for multi portgroup devices. Because the PCI/PEX
   scan fill in DB starting from first SW devNum and for all port groups */
APP_DEMO_PP_CONFIG appDemoPpConfigList[APP_DEMO_PP_CONFIG_SIZE_CNS];
/* hold the "system" generic info */
APP_DEMO_SYS_CONFIG_STC appDemoSysConfig;

/* offset used during HW device ID calculation formula */
GT_U8 appDemoHwDevNumOffset = 0x10;

/*****************************************************************************
* Local variables
******************************************************************************/
static GT_BOOL prvAppDemoIsSelectedDevice = GT_FALSE;
static GT_U8 prvAppDemoSelectedDeviceNum;

#define END_OF_TABLE    0xFFFFFFFF

/* DB to hold the device types that the CPSS support */
extern const struct {
    CPSS_PP_FAMILY_TYPE_ENT     devFamily;
    GT_U32                      numOfPorts;
    const CPSS_PP_DEVICE_TYPE   *devTypeArray;
    const CPSS_PORTS_BMP_STC    *defaultPortsBmpPtr;
    const /*CORES_INFO_STC*/void *coresInfoPtr;
    const GT_U32                *activeCoresBmpPtr;
}cpssSupportedTypes[];


/* PEX devices */
PRV_PCI_DEVICE_QUIRKS_ARRAY_MAC

#if (defined CHX_FAMILY)
/* Forward declaration of Dx configuration functions */
extern GT_STATUS appDemoDxLibrariesInit();
extern GT_STATUS appDemoDxPpLogicalInit();
extern GT_STATUS appDemoDxTrafficEnable();
extern GT_STATUS appDemoDxHwPpPhase1Init();
extern GT_STATUS appDemoDxHwPpStartInit();
extern GT_STATUS appDemoDxHwPpPhase2Init();
extern GT_STATUS appDemoDxPpGeneralInit();
#endif /*(defined CHX_FAMILY) */
/* macro to force casting between 2 functions prototypes */
#define FORCE_FUNC_CAST (void*)


/*****************************************************************************
* Public API implementation
******************************************************************************/

/**
* @internal getDevFamily function
* @endinternal
*
* @brief   Gets the device family from vendor Id and device Id (read from PCI bus)
*
* @param[in] pciDevVendorIdPtr        - pointer to PCI/PEX device identification data.
*
* @param[out] devFamilyPtr             - pointer to CPSS PP device family.
* @param[out] isPexPtr                 - pointer to is PEX flag:
*                                      GT_TRUE - device has PEX interface
*                                      GT_FALSE - device has not PEX interface
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS getDevFamily(
    IN GT_PCI_DEV_VENDOR_ID    *pciDevVendorIdPtr,
    OUT CPSS_PP_FAMILY_TYPE_ENT *devFamilyPtr,
    OUT GT_BOOL     *isPexPtr
)
{
    CPSS_PP_DEVICE_TYPE deviceType = pciDevVendorIdPtr->devId << 16 | pciDevVendorIdPtr->vendorId;
    GT_U32                  ii;
    GT_U32                  jj;
    GT_BOOL                 found = GT_FALSE;

    /* get the info about our device */
    ii = 0;
    while (cpssSupportedTypes[ii].devFamily != END_OF_TABLE)
    {
        jj = 0;
        while (cpssSupportedTypes[ii].devTypeArray[jj] != END_OF_TABLE)
        {
            if (deviceType == cpssSupportedTypes[ii].devTypeArray[jj])
            {
                found = GT_TRUE;
                break;
            }
            jj++;
        }

        if (GT_TRUE == found)
        {
            break;
        }
        ii++;
    }

    /* check if deviceType was found */
    if (GT_TRUE == found)
    {
        /* get family type from CPSS DB */
        *devFamilyPtr = cpssSupportedTypes[ii].devFamily;
    }
    else  /* device not managed by CPSS , so check the devId */
    {
        return GT_NOT_SUPPORTED;
    }

    *isPexPtr = GT_FALSE;

    /* loop on the PEX devices */
    for (ii = 0; prvPciDeviceQuirks[ii].pciId != 0xffffffff; ii++)
    {
        if (prvPciDeviceQuirks[ii].pciId == deviceType)
        {
            *isPexPtr = GT_TRUE;
            break;
        }
    }

    return GT_OK;
}


GT_VOID appDemoSysOneDeviceSet
(
    IN GT_U8 devNum
)
{
    prvAppDemoIsSelectedDevice = GT_TRUE;

    prvAppDemoSelectedDeviceNum = devNum;
}

GT_BOOL appDemoSysIsDeviceSelected
(
)
{
    return prvAppDemoIsSelectedDevice;
}

GT_BOOL appDemoSysIsSelectedDevice
(
    IN GT_U8 devNum
)
{
    if(prvAppDemoIsSelectedDevice == GT_FALSE)
    {
        /* check was added for Lion2 with 8 cores , that not involved in this 'selected device' system */
        return GT_FALSE;
    }
    return devNum == prvAppDemoSelectedDeviceNum;
}

/**
* @internal appDemoSysGetPciInfo function
* @endinternal
*
* @brief   Gets the Pci info for the mini-application usage.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appDemoSysGetPciInfo
(
    GT_VOID
)
{
    GT_PCI_INFO pciInfo;
    GT_U8       devIdx;
    GT_BOOL     isFirst;
    GT_STATUS   status;
    GT_BOOL     isPex;/* is it PEX device */
    GT_BOOL     selectedDeviceFound = GT_FALSE;


    devIdx = SYSTEM_DEV_NUM_MAC(0);
    isFirst = GT_TRUE;

    status = gtPresteraGetPciDev(isFirst, &pciInfo);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("gtPresteraGetPciDev", status);
    if (GT_OK != status)
    {
        return status;
    }

    while (GT_OK == status)
    {
        appDemoPpConfigList[devIdx].channel = CPSS_CHANNEL_PCI_E;
        appDemoPpConfigList[devIdx].devNum  = devIdx;

        osMemCpy(&(appDemoPpConfigList[devIdx].pciInfo),
                 &pciInfo,
                 sizeof(GT_PCI_INFO));

        /* get family */
        status = getDevFamily(&appDemoPpConfigList[devIdx].pciInfo.pciDevVendorId,
                              &appDemoPpConfigList[devIdx].devFamily,
                              &isPex);
        if (GT_OK != status)
        {
            return GT_FAIL;
        }
        appDemoPpConfigList[devIdx].valid = GT_TRUE;

        /* If this is the device passed in command-line option -dev_num then put it
           slot 0, clean other slots and exit */
        if (appDemoSysIsSelectedDevice(devIdx))
        {
            cpssOsPrintf("Using Device %d (PCI ID %x:%x:%x.%x)\n", devIdx,
                         pciInfo.pciBusNum >> 8, pciInfo.pciBusNum & 0xff,
                         pciInfo.pciIdSel, pciInfo.funcNo);
            selectedDeviceFound = GT_TRUE;
            if (devIdx == SYSTEM_DEV_NUM_MAC(0))
                return GT_OK;

            osMemCpy(&(appDemoPpConfigList[0]), &(appDemoPpConfigList[devIdx]),
                     sizeof(appDemoPpConfigList[0]));

            while (devIdx != SYSTEM_DEV_NUM_MAC(0))
            {
                appDemoPpConfigList[devIdx].valid = GT_FALSE;
                devIdx--;
            }

            return GT_OK;
        }

        devIdx++;

        isFirst = GT_FALSE;

        status = gtPresteraGetPciDev(isFirst, &pciInfo);
    }

    if (appDemoSysIsDeviceSelected() && !selectedDeviceFound)
    {
        cpssOsPrintf("Warning: Selected device number was not detected\n");
    }

    return GT_OK;
}

/**
* @internal appDemoHotInsRmvSysGetPciInfo function
* @endinternal
*
* @brief  Gets the Pci info for the mini-application usage.
* IN devIdx                        -   device number
* IN pciBus                        -   PCI Bus number
* IN pciDev                        -   PCI device number
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appDemoHotInsRmvSysGetPciInfo
(
    IN GT_U8 devIdx,
    IN GT_U8 pciBus,
    IN GT_U8 pciDev
)
{
    GT_PCI_INFO pciInfo;
    GT_STATUS   rc;
    GT_BOOL     isPex;/* is it PEX device */
    GT_BOOL     isFirst = GT_TRUE;
    GT_BOOL     isFound = GT_FALSE;

    while((rc = gtPresteraGetPciDev(isFirst, &pciInfo)) == GT_OK)
    {
        isFirst = GT_FALSE;

        if(pciInfo.pciBusNum == pciBus && pciInfo.pciIdSel == pciDev)
        {
            isFound = GT_TRUE;
            break;
        }

    }
#ifndef ASIC_SIMULATION
    if(isFound == GT_TRUE)
    {
        rc = extDrvPciConfigDev(pciBus, pciDev, 0);
        if(rc != GT_OK)
            return rc;
    }
#endif
    if(isFound == GT_TRUE && appDemoPpConfigList[devIdx].valid == GT_FALSE)
    {
        appDemoPpConfigList[devIdx].channel = CPSS_CHANNEL_PCI_E;
        appDemoPpConfigList[devIdx].devNum  = devIdx;

        osMemCpy(&(appDemoPpConfigList[devIdx].pciInfo),
                 &pciInfo,
                 sizeof(GT_PCI_INFO));

        /* get family */
        rc = getDevFamily(&appDemoPpConfigList[devIdx].pciInfo.pciDevVendorId,
                              &appDemoPpConfigList[devIdx].devFamily,
                              &isPex);
        if (GT_OK != rc)
        {
            return GT_FAIL;
        }
        appDemoPpConfigList[devIdx].valid = GT_TRUE;

        cpssOsPrintf("Recognized [0x%8.8x] device on PEX\n",(pciInfo.pciDevVendorId.devId<<16)|(pciInfo.pciDevVendorId.vendorId));
    }

    return rc;
}

/**
* @internal appDemoHotInsRmvSysGetSmiInfo function
* @endinternal
*
* @brief   Gets SMI device info.
* IN devIdx                        -   device number
* IN smiBus                        -   SMI Bus number
* IN smiDev                        -   SMI slave device number
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appDemoHotInsRmvSysGetSmiInfo
(
    IN GT_U8 devIdx,
    IN GT_U8 smiBus,
    IN GT_U8 smiDev
)
{
    GT_STATUS   rc;
    GT_SMI_INFO smiInfo;
    GT_BOOL     isFirst = GT_TRUE;
    GT_BOOL     isFound = GT_FALSE;
    GT_PCI_DEV_VENDOR_ID devVendorId;
    GT_BOOL     isPex;/* is it PEX device */

    if(smiBus != 0)
        return GT_BAD_PARAM;

    while((rc = gtPresteraGetSmiDev(isFirst, &smiInfo)) == GT_OK)
    {
        isFirst = GT_FALSE;
        if(smiInfo.smiIdSel == smiDev)
        {
            isFound = GT_TRUE;
            break;
        }
    }

    if(isFound == GT_TRUE && appDemoPpConfigList[devIdx].valid == GT_FALSE)
    {
        appDemoPpConfigList[devIdx].channel = CPSS_CHANNEL_SMI_E;
        appDemoPpConfigList[devIdx].devNum  = devIdx;

        osMemCpy(&(appDemoPpConfigList[devIdx].smiInfo),
                 &smiInfo,
                 sizeof(GT_SMI_INFO));

        devVendorId.vendorId = smiInfo.smiDevVendorId.vendorId;
        devVendorId.devId = smiInfo.smiDevVendorId.devId;

        /* get family */
        rc = getDevFamily(&devVendorId,
                          &appDemoPpConfigList[devIdx].devFamily,
                          &isPex);/* dummy parameter for SMI devices */
        if(GT_OK != rc)
        {
            return GT_FAIL;
        }
        appDemoPpConfigList[devIdx].valid = GT_TRUE;

        cpssOsPrintf("Recognized [0x%8.8x] device on SMI bus\n", (devVendorId.devId << 16)|(devVendorId.vendorId));
    }

    return rc;
}

/**
* @internal appDemoSysGetSmiInfo function
* @endinternal
*
* @brief   Gets the SMI info for the mini-application usage.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appDemoSysGetSmiInfo
(
    GT_VOID
)
{
    GT_STATUS   status;
    GT_SMI_INFO smiInfo;
    GT_U8       devIdx;
    GT_BOOL     isFirst;
    GT_PCI_DEV_VENDOR_ID devVendorId;
    GT_BOOL     isPex;/* is it PEX device */

    isFirst = GT_TRUE;

    /* Find first not valid device */
    for (devIdx = SYSTEM_DEV_NUM_MAC(0); devIdx < PRV_CPSS_MAX_PP_DEVICES_CNS; devIdx++)
    {
        if (GT_FALSE == appDemoPpConfigList[devIdx].valid)
        {
            break;
        }
    }

    if (devIdx == PRV_CPSS_MAX_PP_DEVICES_CNS)
    {
        return GT_FAIL;
    }

    while (GT_OK == gtPresteraGetSmiDev(isFirst, &smiInfo))
    {
        appDemoPpConfigList[devIdx].channel = CPSS_CHANNEL_SMI_E;
        appDemoPpConfigList[devIdx].devNum  = devIdx;

        osMemCpy(&(appDemoPpConfigList[devIdx].smiInfo),
                 &smiInfo,
                 sizeof(GT_SMI_INFO));

        devVendorId.vendorId = smiInfo.smiDevVendorId.vendorId;
        devVendorId.devId = smiInfo.smiDevVendorId.devId;

        /* get family */
        status = getDevFamily(&devVendorId,
                    &appDemoPpConfigList[devIdx].devFamily,
                    &isPex);/* dummy parameter for SMI devices */
        if (GT_OK != status)
        {
            return GT_FAIL;
        }

        appDemoPpConfigList[devIdx].valid = GT_TRUE;
        devIdx++;
        isFirst = GT_FALSE;
    }
    return GT_OK;
}


/**
* @internal appDemoSysGetTwsiInfo function
* @endinternal
*
* @brief   Gets the SMI info for the mini-application usage.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appDemoSysGetTwsiInfo
(
    GT_VOID
)
{
    return GT_NOT_IMPLEMENTED;
}

/**
* @internal appDemoSysConfigFuncsGet function
* @endinternal
*
* @brief   Gets the system configuration functions accordingly to given device type.
*
* @param[in] deviceType               - device type.
*
* @param[out] sysConfigFuncsPtr        - system configuration functions.
* @param[out] apiSupportedBmpPtr       - (pointer to)bmp of supported type of API to call
*
* @retval GT_OK                    -  on success
* @retval GT_BAD_PTR               -  bad pointer
* @retval GT_NOT_FOUND             -  device type is unknown
* @retval GT_FAIL                  -  on failure
*/
GT_STATUS appDemoSysConfigFuncsGet
(
    IN  GT_U32                      deviceType,
    OUT APP_DEMO_SYS_CONFIG_FUNCS   *sysConfigFuncsPtr,
    OUT GT_U32                      *apiSupportedBmpPtr
)
{
    CPSS_PP_FAMILY_TYPE_ENT devFamily;
    GT_U32                  ii;
    GT_U32                  jj;
    GT_BOOL                 found = GT_FALSE;


    if (NULL == sysConfigFuncsPtr || apiSupportedBmpPtr == NULL)
    {
        return GT_BAD_PTR;
    }

    /* get the info about our device */
    ii = 0;
    while (cpssSupportedTypes[ii].devFamily != END_OF_TABLE)
    {
        jj = 0;
        while (cpssSupportedTypes[ii].devTypeArray[jj] != END_OF_TABLE)
        {
            if (deviceType == cpssSupportedTypes[ii].devTypeArray[jj])
            {
                found = GT_TRUE;
                break;
            }
            jj++;
        }

        if (GT_TRUE == found)
        {
            break;
        }
        ii++;
    }

    /* check if deviceType was found */
    if (GT_FALSE == found)
    {
        return GT_NOT_FOUND;
    }

    /* get family type */
    devFamily = cpssSupportedTypes[ii].devFamily;
    *apiSupportedBmpPtr = 0;

    if(CPSS_IS_DXCH_FAMILY_MAC(devFamily))
    {
#if (defined CHX_FAMILY)
        *apiSupportedBmpPtr = APP_DEMO_DXCH_FUNCTIONS_SUPPORT_CNS;
        /*ch2,3 ... */
        *apiSupportedBmpPtr |= APP_DEMO_DXCH2_FUNCTIONS_SUPPORT_CNS;
        /*ch3 ...*/
        *apiSupportedBmpPtr |= APP_DEMO_DXCH3_FUNCTIONS_SUPPORT_CNS;
        /*xCat ...*/
        *apiSupportedBmpPtr |= APP_DEMO_DXCH_XCAT_FUNCTIONS_SUPPORT_CNS;

        if(devFamily >= CPSS_PP_FAMILY_DXCH_LION2_E)
        {
            /*Lion ...*/
            *apiSupportedBmpPtr |= APP_DEMO_DXCH_LION_FUNCTIONS_SUPPORT_CNS;
        }

        sysConfigFuncsPtr->cpssHwPpPhase1Init   = FORCE_FUNC_CAST appDemoDxHwPpPhase1Init;
        sysConfigFuncsPtr->cpssHwPpStartInit    = FORCE_FUNC_CAST appDemoDxHwPpStartInit;
        sysConfigFuncsPtr->cpssHwPpPhase2Init   = FORCE_FUNC_CAST appDemoDxHwPpPhase2Init;
        sysConfigFuncsPtr->cpssLibrariesInit    = FORCE_FUNC_CAST appDemoDxLibrariesInit;
        sysConfigFuncsPtr->cpssPpLogicalInit    = FORCE_FUNC_CAST appDemoDxPpLogicalInit;
        sysConfigFuncsPtr->cpssTrafficEnable    = FORCE_FUNC_CAST appDemoDxTrafficEnable;
        sysConfigFuncsPtr->cpssPpGeneralInit    = FORCE_FUNC_CAST appDemoDxPpGeneralInit;
#endif /* (defined CHX_FAMILY) */
    }

    if((*apiSupportedBmpPtr) == 0)
    {
        /* Unsupported family type */
        return GT_NOT_IMPLEMENTED;
    }

    if(appDemoOnDistributedSimAsicSide != 0)
    {
        sysConfigFuncsPtr->cpssHwPpStartInit = NULL;
        sysConfigFuncsPtr->cpssTrafficEnable = NULL;
        sysConfigFuncsPtr->cpssPpGeneralInit = NULL;
    }

    return GT_OK;
}

/**
* @internal appDemoHwDevNumOffsetSet function
* @endinternal
*
* @brief   Sets value of appDemoHwDevNumOffset used during device HW ID calculation
*         based on device SW ID..
* @param[in] hwDevNumOffset           - value to save into appDemoHwDevNumOffset.
*                                       GT_OK
*/
GT_STATUS appDemoHwDevNumOffsetSet
(
    IN GT_U8 hwDevNumOffset
)
{
    appDemoHwDevNumOffset = hwDevNumOffset;

    return GT_OK;
}

/*******************************************************************************
* appDemoDbByDevNum
*
* DESCRIPTION:
*       get the AppDemo DB entry for the 'devNum'.
*
* INPUTS:
*       devNum - the 'cpss devNum'
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       pointer to the appDemoDb
*
* COMMENTS:
*       None.
*
*******************************************************************************/
APP_DEMO_PP_CONFIG* appDemoDbByDevNum(IN GT_U8 devNum)
{
    static APP_DEMO_PP_CONFIG dummyDbEntry;
    GT_U32  ii;

    APP_DEMO_PP_CONFIG* currentEntryInAppDemoDbPtr = &appDemoPpConfigList[0];
    for(ii = 0 ; ii < APP_DEMO_PP_CONFIG_SIZE_CNS ; ii++,currentEntryInAppDemoDbPtr++)
    {
        if(currentEntryInAppDemoDbPtr->valid == GT_TRUE &&
           currentEntryInAppDemoDbPtr->devNum == devNum)
        {
            return currentEntryInAppDemoDbPtr;
        }
    }

    cpssOsPrintf("appDemoDbByDevNum: Error not found device [%d] \n",devNum);
    /* should not happen */
    cpssOsMemSet(&dummyDbEntry,0,sizeof(dummyDbEntry));
    return &dummyDbEntry;
}

/**
* @internal appDemoPhase2DefaultInit function
* @endinternal
*
* @brief   Sets default value for CPSS_PP_PHASE2_INIT_PARAMS struct
* @param[out] ppPh2ConfigPtr           - pointer to struct
*/
GT_VOID appDemoPhase2DefaultInit
(
    OUT CPSS_PP_PHASE2_INIT_PARAMS  *ppPh2ConfigPtr
)
{
    const CPSS_NET_IF_CFG_STC netIfCfg = { /* default values of Network Interface struct */
        NULL,                       /* *txDescBlock     */
        4096,                       /* txDescBlockSize  */
        NULL,                       /* *rxDescBlock     */
        4096,                       /* rxDescBlockSize  */
        /* rxBufInfo        */
        {
            CPSS_RX_BUFF_STATIC_ALLOC_E, /* allocMethod */
            { 13, 13, 13, 13,           /* bufferPercentage */
              12, 12, 12, 12 },
            RX_BUFF_SIZE_DEF,           /* rxBufSize    */
            0,                          /* headerOffset */
            GT_FALSE,                   /* buffersInCachedMem */
            /* union buffData   */
            {
                /* struct staticAlloc   */
                {
                    NULL,           /* *rxBufBlock      */
                    16000           /* rxBufBlockSize   */
                }
            }
        }
    };

    const CPSS_AUQ_CFG_STC auqCfg = { /* default values of Address Update Queue struct */
        NULL,                       /* *auDescBlock     */
        4096                        /* auDescBlockSize  */
    };

    /* clean struct */
    cpssOsMemSet(ppPh2ConfigPtr, 0, sizeof(CPSS_PP_PHASE2_INIT_PARAMS));
    ppPh2ConfigPtr->auqCfg = auqCfg;
    ppPh2ConfigPtr->netIfCfg = netIfCfg;
    ppPh2ConfigPtr->rxBufSize = RX_BUFF_SIZE_DEF;
}


