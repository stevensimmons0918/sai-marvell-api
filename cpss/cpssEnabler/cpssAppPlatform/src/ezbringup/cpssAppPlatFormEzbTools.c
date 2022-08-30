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
* @file cpssAppPlatFormEzbTools.c
*
* @brief  hold common function (tools) for the board config files to use to get
*   info from the 'ez_bringup' xml
*
*   NOTE: this file compiled only when 'EZ_BRINGUP' (and DX code) is enabled.
*   meaning that 'stub' (if needed) are implemented elsewhere.
*
* @version   1
********************************************************************************
*/

#include <ezbringup/cpssAppPlatformEzBringupTools.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfoEnhanced.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


#include <cpss/common/smi/cpssGenSmi.h>

#if (defined ASIC_SIMULATION_ENV_FORBIDDEN && defined ASIC_SIMULATION)
    /* 'Forbid' the code to run as ASIC_SIMULATION ... we need 'like' HW compilation ! */
    #undef ASIC_SIMULATION
#endif

#ifdef ASIC_SIMULATION
    #include <asicSimulation/SEmbedded/simFS.h>
#endif /*ASIC_SIMULATION*/

#define END_OF_TABLE 0xFFFFFFFF

#include <pdl/init/pdlInit.h>
#include <pdl/cpu/pdlCpu.h>
#include <pdl/packet_processor/pdlPacketProcessor.h>
#include <pdl/serdes/pdlSerdes.h>
#include <iDbgPdl/init/iDbgPdlInit.h>
#include <cpssAppPlatformPciConfig.h>

typedef struct {
    GT_U32  numOfPortGroups;
    GT_U32  portsBmp;
    GT_U32  maxNumPorts;
} PORT_GROUPS_INFO_STC;

typedef struct {
    CPSS_PP_FAMILY_TYPE_ENT     devFamily;
    GT_U32                      numOfPorts;
    const CPSS_PP_DEVICE_TYPE   *devTypeArray;
    const CPSS_PORTS_BMP_STC    *defaultPortsBmpPtr;
    const PORT_GROUPS_INFO_STC  *portGroupInfoPtr;
    const GT_U32                *activePortGroupsBmpPtr;
}CPSS_SUPPORTED_TYPES_STC;

extern const struct {
    CPSS_PP_FAMILY_TYPE_ENT    devFamily;
    GT_U32                     numOfPorts;
    const CPSS_PP_DEVICE_TYPE *devTypeArray;
    const CPSS_PORTS_BMP_STC  *defaultPortsBmpPtr;
    const GT_VOID_PTR         *coresInfoPtr;
    const GT_U32              *activeCoresBmpPtr;
}cpssSupportedTypes[];

extern GT_U32   prvCpssDxChHwInitNumOfSerdesGet
(
    IN GT_U8 devNum
);

extern GT_STATUS prvCpssAppPlatformGetPciDev
(
    IN  GT_U8  pciBus,
    IN  GT_U8  pciDev,
    IN  GT_U8  pciFunc,
    OUT GT_U16 *vendorId,
    OUT GT_U16 *deviceId
);


/* API to append CPU SDMA ports to portMap array */
static GT_STATUS addCpuSdmaPortMapping
(
        INOUT CPSS_DXCH_PORT_MAP_STC portMapArr[],
        INOUT GT_U32            *actualNumOfPortsPtr
)
{
    PDL_STATUS                      pdlStatus;
    CPSS_DXCH_PORT_MAP_STC *currPortInfoPtr;
    GT_U32 ii = 0,index_target;
    PDL_CPU_SDMA_MAP_INFO_STC       cpuSdmaMapInfo;

    index_target = *actualNumOfPortsPtr;

    currPortInfoPtr    = &portMapArr[index_target];

    for (pdlStatus = pdlCpuSdmaMapDbGetFirst(&cpuSdmaMapInfo);
            pdlStatus == PDL_OK;
            pdlStatus = pdlCpuSdmaMapDbGetNext(&cpuSdmaMapInfo, &cpuSdmaMapInfo))
    {
        cpssOsBzero((GT_CHAR *)currPortInfoPtr,sizeof(CPSS_DXCH_PORT_MAP_STC));
        currPortInfoPtr->physicalPortNumber = cpuSdmaMapInfo.macPort;
        currPortInfoPtr->interfaceNum = cpuSdmaMapInfo.logicalPort;
        currPortInfoPtr->mappingType = CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E;
        ii++;
        currPortInfoPtr++;
    }

    *actualNumOfPortsPtr += ii;

    return GT_OK;
}
/**
* @internal cpssAppPlatformEzbIsXmlWithDevNum function
* @endinternal
*
* @brief  is XML of EZ_BRINGUP hold info about the device.
*
*
* @retval GT_TRUE / GT_FALSE
*/
GT_BOOL cpssAppPlatformEzbIsXmlWithDevNum
(
    IN GT_U32  devNum
)
{
    PDL_STATUS        rc;
    PDL_PP_XML_ATTRIBUTES_STC ppAttributes;
    GT_U32  xmlDevNum;  /* the devNum as appear in the XML */

    xmlDevNum = devNum;

    rc = pdlPpDbAttributesGet(&ppAttributes);
    if(rc != GT_OK)
    {
        return GT_FALSE;
    }
    if(xmlDevNum >= ppAttributes.numOfPps)
    {
        return GT_FALSE;
    }

    return GT_TRUE;
}

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

    CPSS_NULL_PTR_CHECK_MAC(pciDevVendorIdPtr);
    CPSS_NULL_PTR_CHECK_MAC(devFamilyPtr);

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
 * @internal cpsssAppPlatformEzbGetSupportedTypes function
 * @endinternal
 *
 * @brief   Gets supported types for the provided devicefamily.
 *
 * @param[in] ppMapPtr     - pointer to Board params of PP from profile
 *
 * @param[out] cpssSupportedTypesPtr - pointer to CPSS supported types
 * @param[out] existingPortsPtr     - pointer existing ports pointer
 *
 * @retval GT_OK                    - on success,
 * @retval GT_FAIL                  - otherwise.
 */
static GT_BOOL cpsssAppPlatformEzbGetSupportedTypes
(
    IN   CPSS_APP_PLATFORM_BOARD_PARAM_PP_MAP_STC   *ppMapPtr,
    IN   CPSS_APP_PLATFORM_PP_PROFILE_STC           *ppProfilePtr,
    OUT  CPSS_SUPPORTED_TYPES_STC                  **cpssSupportedTypesPtr,
    OUT  CPSS_PORTS_BMP_STC                         *existingPortsPtr
)
{
    GT_U32        devFamilyNum = 0;
    GT_BOOL       found = FALSE;
    PCI_INFO_STC  pciInfo;
    GT_STATUS rc = GT_OK;
    CPSS_PP_FAMILY_TYPE_ENT     devFamily;
    /* Get DevFamily */
    /* get the info about our device */
    CPSS_NULL_PTR_CHECK_MAC(existingPortsPtr);
    CPSS_NULL_PTR_CHECK_MAC(ppMapPtr);
    if(!ppProfilePtr->internalCpu)
    {
        rc = prvCpssAppPlatformGetPciDev(ppMapPtr->mngInterfaceAddr.pciAddr.busId
                , ppMapPtr->mngInterfaceAddr.pciAddr.deviceId
                , ppMapPtr->mngInterfaceAddr.pciAddr.functionId
                , &(pciInfo.pciDevVendorId.vendorId)
                , &(pciInfo.pciDevVendorId.devId));
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvCpssAppPlatformGetPciDev);
        rc = getDeviceFamily(&pciInfo.pciDevVendorId, &devFamily);
        if(rc != GT_OK)
        {
            CPSS_APP_PLATFORM_LOG_ERR_MAC("getDeviceFamily rc=%d ret=%d", rc, GT_FAIL);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        /* Internal CPU WA for AC5x */
        devFamily = CPSS_PP_FAMILY_DXCH_AC5X_E;
    }
    devFamilyNum = 0;
    while(cpssSupportedTypes[devFamilyNum].devFamily != END_OF_TABLE)
    {
        if(devFamily == cpssSupportedTypes[devFamilyNum].devFamily)
        {
            found = GT_TRUE;
            break;
        }
        devFamilyNum++;
    }
    if (GT_TRUE == found)
    {
        *cpssSupportedTypesPtr   =(CPSS_SUPPORTED_TYPES_STC*) &cpssSupportedTypes[devFamilyNum];
        *existingPortsPtr        = *cpssSupportedTypes[devFamilyNum].defaultPortsBmpPtr;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

/**
* @internal cpssAppPlatformEzbUpdateProfile function
* @endinternal
*
* @brief  Update provided profile with the data(PortMap,Speed,Interface) read from EzBringUp XML
*
* @param[in] inputProfileListPtr   - PP Profile
*
* @retval
* @retval GT_OK                    - on success,
* @retval GT_NOT_INITIALIZED       - the XML was not loaded.
* @retval GT_NOT_FOUND             - the devNum not exists in the xml.
* @retval GT_NOT_SUPPORTED         - the XML not hold port mapping info for the device
* @retval GT_ALREADY_EXIST         - the DB appDemoPpConfigList[].macGenInfoPtr
*                                    already exists (assuming already initialized)
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS cpssAppPlatformEzbUpdateProfile
(
    IN CPSS_APP_PLATFORM_PROFILE_STC *inputProfileListPtr
)
{
    static CPSS_APP_PLATFORM_PORT_CONFIG_STC    portInitList_fromXml[CPSS_MAX_PORTS_NUM_CNS];
    GT_STATUS                                   rc = GT_OK;
    CPSS_APP_PLATFORM_PROFILE_STC              *profileListPtr      = inputProfileListPtr;
    CPSS_APP_PLATFORM_RUNTIME_PROFILE_STC      *runTimeProfilePtr   = NULL;
    CPSS_APP_PLATFORM_TRAFFIC_ENABLE_STC       *trafficEnablePtr    = NULL;
    CPSS_APP_PLATFORM_BOARD_PARAM_PP_MAP_STC   *boardPpMapPtr       = NULL;
    CPSS_APP_PLATFORM_PP_PROFILE_STC           *ppProfilePtr        = NULL;
    GT_U32                                      i = 0;
#if 0
    CPSS_APP_PLATFORM_PP_PROFILE_STC           *ppProfilePtr        = NULL;
#endif
    CPSS_NULL_PTR_CHECK_MAC(inputProfileListPtr);

    PRV_CPSS_APP_START_LOOP_PROFILE_MAC(profileListPtr, CPSS_APP_PLATFORM_PROFILE_TYPE_PP_E)
    {
            if(profileListPtr->profileValue.ppInfoPtr == NULL)
            {
                /* PP profile is Empty. proceed to next profile */
                continue;
            }
            ppProfilePtr = profileListPtr->profileValue.ppInfoPtr;

    }
    PRV_CPSS_APP_END_LOOP_PROFILE_MAC(profileListPtr, CPSS_APP_PLATFORM_PROFILE_TYPE_PP_E)

    profileListPtr      = inputProfileListPtr;
    PRV_CPSS_APP_START_LOOP_PROFILE_MAC(profileListPtr, CPSS_APP_PLATFORM_PROFILE_TYPE_RUNTIME_E)
    {
        if(profileListPtr->profileValue.runTimeInfoPtr == NULL)
        {
            /* RunTime profile is Empty. proceed to next profile */
            continue;
        }

        runTimeProfilePtr   = profileListPtr->profileValue.runTimeInfoPtr;
        PRV_CPSS_APP_START_LOOP_RUNTIME_PROFILE_MAC(runTimeProfilePtr, CPSS_APP_PLATFORM_RUNTIME_PARAM_TRAFFIC_ENABLE_E)
        {
            trafficEnablePtr    = runTimeProfilePtr->runtimeInfoValue.trafficEnablePtr;
            if(trafficEnablePtr == NULL)
            {
                /* event handle is NULL. proceed to next profile */
                continue;
            }
        }
        PRV_CPSS_APP_END_LOOP_RUNTIME_PROFILE_MAC(runTimeProfilePtr, CPSS_APP_PLATFORM_RUNTIME_PARAM_TRAFFIC_ENABLE_E)
    }
    PRV_CPSS_APP_END_LOOP_PROFILE_MAC(profileListPtr, CPSS_APP_PLATFORM_PROFILE_TYPE_RUNTIME_E)

        profileListPtr      = inputProfileListPtr;
    PRV_CPSS_APP_START_LOOP_PROFILE_MAC(profileListPtr, CPSS_APP_PLATFORM_PROFILE_TYPE_BOARD_E)
    {
        if(profileListPtr->profileValue.boardInfoPtr == NULL)
        {
            continue;
        }
        if(profileListPtr->profileValue.boardInfoPtr->boardParam.ppMapPtr == NULL)
        {
            continue;
        }
        boardPpMapPtr = profileListPtr->profileValue.boardInfoPtr->boardParam.ppMapPtr;
    }
    PRV_CPSS_APP_END_LOOP_PROFILE_MAC(profileListPtr, CPSS_APP_PLATFORM_PROFILE_TYPE_BOARD_E)
    if((boardPpMapPtr == NULL) || (trafficEnablePtr == NULL) || (ppProfilePtr == NULL))
    {
        CPSS_APP_PLATFORM_LOG_ERR_MAC("EzBringup can only be run with Profile \"ALL\", requires BOARD Profile, PP Profile and RUNTIME Profile");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* Update speed and interface modes obtained from */
    rc = cpssAppPlatformEzbPortsSpeedGet(trafficEnablePtr->devNum,
                boardPpMapPtr,
                ppProfilePtr,
                portInitList_fromXml);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformEzbPortsSpeedGet);
    /* the XML not supports and also 'PDL' not support next: CPSS_PORT_INTERFACE_MODE_USX_20G_OXGMII_E */
    /* so convert CPSS_PORT_INTERFACE_MODE_XHGS_E to CPSS_PORT_INTERFACE_MODE_USX_20G_OXGMII_E         */
    {
        static EZB_XML_SPEED_IF_MODE_REPLACE_STC replaceInfo[] =
        {
            /* ability to state 'no speed' and 'no interface' by using '10M' speed from the XML,
               , as PDL not supports 'no speed' and 'no interface'  */
            {
                /*old*/{GT_NA                                    ,CPSS_PORT_SPEED_10_E},
                /*new*/{CPSS_PORT_INTERFACE_MODE_NA_E            ,CPSS_PORT_SPEED_NA_E}
            },
            /* allow to set {USX_20G_OXGMII , *} , as PDL not supports USX_20G_OXGMII */
            {
                /*old*/{CPSS_PORT_INTERFACE_MODE_XHGS_E          ,GT_NA/*all speeds*/},
                /*new*/{CPSS_PORT_INTERFACE_MODE_USX_OUSGMII_E   ,GT_NA}
            },
            /* allow to set {KR , 100G} to become {KR4 , 100G} , as PDL not supports KR4 */
            {
                /*old*/{CPSS_PORT_INTERFACE_MODE_KR_E            ,CPSS_PORT_SPEED_100G_E},
                /*new*/{CPSS_PORT_INTERFACE_MODE_KR4_E           ,CPSS_PORT_SPEED_100G_E}
            },

            /* must be last */
            EZB_XML_SPEED_IF_MODE_REPLACE___MUAST_BE_LAST__CNS
        };
        rc = cpssAppPlatformEzbPortsSpeedAndModeUpdate(portInitList_fromXml,replaceInfo);
        if (GT_OK != rc)
        {
            return rc;
        }
    }

    if ( trafficEnablePtr->portTypeListPtr->ifMode == CPSS_PORT_INTERFACE_MODE_NA_E || trafficEnablePtr->portTypeListPtr->speed == CPSS_PORT_SPEED_NA_E )
    {
        for (i=0; portInitList_fromXml[i].entryType != CPSS_APP_PLATFORM_PORT_LIST_TYPE_EMPTY_E; i++)
        {
            portInitList_fromXml[i].ifMode = CPSS_PORT_INTERFACE_MODE_NA_E;
            portInitList_fromXml[i].speed  = CPSS_PORT_SPEED_NA_E;
        }
    }

    trafficEnablePtr->portTypeListPtr = portInitList_fromXml;

    /* update port mapping in profile */
    rc = cpssAppPlatformEzbPortMapGet(boardPpMapPtr->devNum, boardPpMapPtr, ppProfilePtr);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformEzbPortMapGet);

#if 0
    rc = cpssAppPlatformEzbSerdesPolarityGet(boardPpMapPtr->devNum, ppProfilePtr);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformEzbSerdesPolarityGet);
#endif
    return rc;
}
/**
* @internal cpssAppPlatformEzbPortMapGet function
* @endinternal
*
* @brief  SIP6 devices : get EZ_BRINGUP info related to the port mapping of specific device
*
* @param[in] devNum                 - device number
* @param[in] ppMapPtr               - pointer to PP Board params
*
* @retval GT_OK                    - on success,
* @retval GT_NOT_INITIALIZED       - the XML was not loaded.
* @retval GT_NOT_FOUND             - the devNum not exists in the xml.
* @retval GT_NOT_SUPPORTED         - the XML not hold port mapping info for the device.
* @retval GT_FULL                  - the logic could not find physical port for CPU SDMA port with GT_NA.
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS cpssAppPlatformEzbPortMapGet(
    IN  GT_U8                                       devNum,
    IN  CPSS_APP_PLATFORM_BOARD_PARAM_PP_MAP_STC   *ppMapPtr,
    IN  CPSS_APP_PLATFORM_PP_PROFILE_STC           *ppProfilePtr
)
{
    GT_STATUS               rc;
    CPSS_DXCH_PORT_MAP_STC *currPortInfoPtr;
    CPSS_DXCH_PORT_MAP_STC  zeroInfo;
    GT_U32                  ii, jj;
    CPSS_PORTS_BMP_STC      existingPorts;
    EZB_PP_CONFIG          *ezbPpConfigPtr;
    EZB_XML_MAC_INFO_STC   *macInfoPtr;
    CPSS_SUPPORTED_TYPES_STC  *cpssSupportedTypesPtr;
    GT_U32                  maxNumPorts;
    static CPSS_DXCH_PORT_MAP_STC  ezbPortMapArr[CPSS_MAX_PORTS_NUM_CNS];
    GT_U32                  numCpuSdmaPorts = 0;
    GT_U32                  actualNumOfPorts;

    CPSS_NULL_PTR_CHECK_MAC(ppMapPtr);
    ezbPpConfigPtr = &ezbPpConfigList[devNum];

    cpsssAppPlatformEzbGetSupportedTypes(ppMapPtr, ppProfilePtr, &cpssSupportedTypesPtr, &existingPorts);

    maxNumPorts = (cpssSupportedTypesPtr->numOfPorts-1);

    if(maxNumPorts != 0)
    {
        currPortInfoPtr = &ezbPortMapArr[0];
    }
    else
    {
        currPortInfoPtr = &zeroInfo;
    }

    cpssOsBzero((GT_CHAR *)&zeroInfo,sizeof(CPSS_DXCH_PORT_MAP_STC));
    if(GT_FALSE == ezbIsXmlLoaded())
    {
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(GT_NOT_INITIALIZED, cpssAppPlatformEzbPortMapGet);
    }

    if(!ezbPpConfigPtr->macGenInfoPtr)
    {
        /* DB of appDemo was NOT initialized yet */
        rc = cpssAppPlatformEzbDevInfoGet(devNum, ppMapPtr, ppProfilePtr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    macInfoPtr = (EZB_XML_MAC_INFO_STC*)ezbPpConfigPtr->macGenInfoPtr;
    ii = 0;
    for(jj = 0 ; jj < ezbPpConfigPtr->numOfMacPorts ; jj++,macInfoPtr++)
    {
        if(macInfoPtr->isMacUsed == GT_FALSE)
        {
            continue;
        }

        if(ii < maxNumPorts)
        {
            *currPortInfoPtr = zeroInfo;
            currPortInfoPtr->mappingType = CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E;
            currPortInfoPtr->physicalPortNumber = macInfoPtr->physicalPort;
            currPortInfoPtr->interfaceNum       = macInfoPtr->macNum;

            currPortInfoPtr++;
        }
        ii++;
    }

    if(ii == 0)
    {
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(GT_NOT_SUPPORTED, cpssAppPlatformEzbPortMapGet);
    }
    actualNumOfPorts = ii;
    if(maxNumPorts == 0)
    {
        actualNumOfPorts += numCpuSdmaPorts;
        return GT_OK;
    }

    rc = addCpuSdmaPortMapping(ezbPortMapArr ,&actualNumOfPorts);
    if(rc != GT_OK)
        return rc;
    /* Update profile with the port map from EZB */
    ppMapPtr->portMap      = &ezbPortMapArr[0];
    ppMapPtr->portMapSize  = actualNumOfPorts;

    return rc;
}


/**
* @internal cpssAppPlatformEzbPortsSpeedGet function
* @endinternal
*
* @brief  get EZ_BRINGUP info related to the {speed,ifMode} of ports of specific device
*
* @param[in] devNum                 - device number
* @param[in] ppMapPtr               - pointer to PP Board params
* @param[out] portsSpeedAndMode    - The array to fill with info.
*
* @retval GT_OK                    - on success,
* @retval GT_NOT_INITIALIZED       - the XML was not loaded.
* @retval GT_NOT_FOUND             - the devNum not exists in the xml.
* @retval GT_NOT_SUPPORTED         - the XML not hold {speed,ifMode} info for the ports in the device.
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS cpssAppPlatformEzbPortsSpeedGet
(
    IN   GT_U8                                      devNum,
    IN   CPSS_APP_PLATFORM_BOARD_PARAM_PP_MAP_STC  *ppMapPtr,
    IN   CPSS_APP_PLATFORM_PP_PROFILE_STC          *ppProfilePtr,
    OUT  CPSS_APP_PLATFORM_PORT_CONFIG_STC          portsSpeedAndMode[]
)
{
    GT_STATUS st;
    CPSS_APP_PLATFORM_PORT_CONFIG_STC *currPortInfoPtr;
    GT_U32                  ii,jj;
    EZB_XML_MAC_INFO_STC   *macInfoPtr;
    EZB_PP_CONFIG          *ezbPpConfigPtr;

    CPSS_NULL_PTR_CHECK_MAC(ppMapPtr);
    ezbPpConfigPtr = &ezbPpConfigList[devNum];
    currPortInfoPtr = &portsSpeedAndMode[0];

    if(!ezbPpConfigPtr->macGenInfoPtr)
    {
        /* DB was NOT initialized yet */
        st = cpssAppPlatformEzbDevInfoGet(devNum, ppMapPtr, ppProfilePtr);
        if(st != GT_OK)
        {
            return st;
        }
    }
    macInfoPtr = (EZB_XML_MAC_INFO_STC*)ezbPpConfigPtr->macGenInfoPtr;
    ii = 0;
    for(jj = 0 ; jj < ezbPpConfigPtr->numOfMacPorts ; jj++,macInfoPtr++)
    {
        if(macInfoPtr->isMacUsed == GT_FALSE)
        {
            continue;
        }

        if(macInfoPtr->defaultSpeedAndIfMode.speed  == CPSS_PORT_SPEED_NA_E ||
           macInfoPtr->defaultSpeedAndIfMode.ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
        {
            /* the port is without proper info ?! */
            continue;
        }

        if(ii < CPSS_MAX_PORTS_NUM_CNS)
        {
            currPortInfoPtr->entryType      = CPSS_APP_PLATFORM_PORT_LIST_TYPE_LIST_E;
            currPortInfoPtr->portList[0]    = macInfoPtr->physicalPort;
            currPortInfoPtr->portList[1]    = APP_INV_PORT_CNS;
            currPortInfoPtr->speed          = macInfoPtr->defaultSpeedAndIfMode.speed;
            currPortInfoPtr->ifMode  = macInfoPtr->defaultSpeedAndIfMode.ifMode;

            currPortInfoPtr++;
        }
        ii++;
    }
    if(ii < CPSS_MAX_PORTS_NUM_CNS)
    {
        currPortInfoPtr->entryType      = CPSS_APP_PLATFORM_PORT_LIST_TYPE_EMPTY_E;
        currPortInfoPtr->portList[0]    = APP_INV_PORT_CNS;
        currPortInfoPtr->speed          = CPSS_PORT_SPEED_NA_E;
        currPortInfoPtr->ifMode  = CPSS_PORT_INTERFACE_MODE_NA_E;
    }

    return GT_OK;
}

/**
* @internal cpssAppPlatformEzbPortsSpeedAndModeUpdate function
* @endinternal
*
* @brief  as EZ_BRINGUP (XML,PDL) may not support all speeds and modes.
*         of new devices added to the CPSS , we need function that will use 'known values' from XML
*         to be 'alias' to other values of the CPSS.
*         for example: the 'old' can be for CPSS_PORT_INTERFACE_MODE_XHGS_E but
*         it will be converted by 'new' to CPSS_PORT_INTERFACE_MODE_USX_20G_OXGMII_E
*         so in portsSpeedAndMode[] all places of {1G,XHGS} will be converted to {1G,20G_OXGMII}
*
*         NOTEs:
*          1.    old.speed  == CPSS_PORT_SPEED_NA_E means : 'termination' of the replaceInfoArr[].
*          2. if old.speed  == GT_NA then only ifMode changed from old to new
*          2. if old.ifMode == GT_NA then only speed  changed from old to new
*
*
*
* @param[in] portsSpeedAndMode    - The array with the info of ports and {speed,ifMode} before the update.
* @param[in] replaceInfoArr          - (array of entries) each hold the 'old'{speed,ifMode} that if exists in portsSpeedAndMode[]
*                                   should be replaced with info from 'new'{speed,ifMode}.
*
* @param[out] portsSpeedAndMode   - The array with the info of ports and {speed,ifMode} after the update.
*
* @retval
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS cpssAppPlatformEzbPortsSpeedAndModeUpdate
(
    INOUT CPSS_APP_PLATFORM_PORT_CONFIG_STC  portsSpeedAndMode[],
    IN EZB_XML_SPEED_IF_MODE_REPLACE_STC  replaceInfoArr[]
)
{
    CPSS_APP_PLATFORM_PORT_CONFIG_STC *currPortInfoPtr;
    EZB_XML_SPEED_IF_MODE_REPLACE_STC *currReplaceInfoPtr;

    CPSS_NULL_PTR_CHECK_MAC(portsSpeedAndMode);
    CPSS_NULL_PTR_CHECK_MAC(replaceInfoArr);

    currPortInfoPtr = &portsSpeedAndMode[0];

    for (/*init already done*/; currPortInfoPtr->entryType != CPSS_APP_PLATFORM_PORT_LIST_TYPE_EMPTY_E; currPortInfoPtr++)
    {
        currReplaceInfoPtr = &replaceInfoArr[0];
        for(/*init already done*/; currReplaceInfoPtr->old.speed != CPSS_PORT_SPEED_NA_E ; currReplaceInfoPtr++)
        {
            if(currPortInfoPtr->speed           == currReplaceInfoPtr->old.speed &&
               currPortInfoPtr->ifMode   == currReplaceInfoPtr->old.ifMode )
            {
                currPortInfoPtr->speed          = currReplaceInfoPtr->new.speed;
                currPortInfoPtr->ifMode  = currReplaceInfoPtr->new.ifMode;
                break;
            }

            if(GT_NA                            == currReplaceInfoPtr->old.speed &&
               currPortInfoPtr->ifMode   == currReplaceInfoPtr->old.ifMode )
            {
                /* modify only ifMode */
                currPortInfoPtr->ifMode  = currReplaceInfoPtr->new.ifMode;
                break;
            }
            if(currPortInfoPtr->speed           == currReplaceInfoPtr->old.speed &&
               GT_NA                            == currReplaceInfoPtr->old.ifMode)
            {
                /* modify only speed */
                currPortInfoPtr->speed  = currReplaceInfoPtr->new.speed;
                break;
            }
        }

        if(currPortInfoPtr->speed == CPSS_PORT_SPEED_NA_E)
        {
            /* we need to make the entry 'empty' from ports */
            currPortInfoPtr->entryType = CPSS_APP_PLATFORM_PORT_LIST_TYPE_LIST_E;
            currPortInfoPtr->portList[0] = GT_NA;/* kill the first in array */
        }
    }

    return GT_OK;
}

/**
* @internal cpssAppPlatformEzbDevInfoGet function
* @endinternal
*
* @brief  get EZ_BRINGUP info related to the specific device
*
* @param[in] devNum                - device number of the PP
* @param[in] ppMapPtr              - pointer to PP Board params.
*
* @retval
* @retval GT_OK                    - on success,
* @retval GT_NOT_INITIALIZED       - the XML was not loaded.
* @retval GT_NOT_FOUND             - the devNum not exists in the xml.
* @retval GT_NOT_SUPPORTED         - the XML not hold port mapping info for the device
* @retval GT_ALREADY_EXIST         - the DB appDemoPpConfigList[].macGenInfoPtr
*                                    already exists (assuming already initialized)
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS cpssAppPlatformEzbDevInfoGet
(
    IN  GT_U32                                      devNum,
    IN  CPSS_APP_PLATFORM_BOARD_PARAM_PP_MAP_STC   *ppMapPtr,
    IN  CPSS_APP_PLATFORM_PP_PROFILE_STC           *ppProfilePtr
)
{
    GT_U32                      xmlDevNum;  /* the devNum as appear in the XML */
    GT_U32                      macNum;     /* the MAC number of the ports to get info from XML ... if exists */
    GT_U32                      numOfMacs;  /* max number of MACs in the device */
    CPSS_PORTS_BMP_STC          existingPorts, *existingPortPtr;
    EZB_PP_CONFIG              *ezbPpConfigPtr;
    CPSS_SUPPORTED_TYPES_STC   *cpssSupportedTypesPtr;

    CPSS_NULL_PTR_CHECK_MAC(ppMapPtr);
    ezbPpConfigPtr = &ezbPpConfigList[devNum];
    xmlDevNum = devNum;
    existingPortPtr = &existingPorts;
    /* Read supported modes thats supported by the current PP */
    cpsssAppPlatformEzbGetSupportedTypes(ppMapPtr, ppProfilePtr, &cpssSupportedTypesPtr, &existingPorts);
    numOfMacs = (cpssSupportedTypesPtr->numOfPorts - 1);

    if(GT_FALSE == ezbIsXmlLoaded())
    {
        return GT_NOT_INITIALIZED;
    }

    if(ezbPpConfigPtr->macGenInfoPtr)
    {
        return GT_ALREADY_EXIST;
    }

    /*************************/
    /* handle 'per MAC' info */
    /*************************/
    ezbPpConfigPtr->macGenInfoPtr = cpssOsMalloc(numOfMacs * sizeof(EZB_XML_MAC_INFO_STC));
    if(ezbPpConfigPtr->macGenInfoPtr == NULL)
    {
        /* malloc failed */
        CPSS_APP_PLATFORM_LOG_DBG_MAC("malloc failed");
        return GT_OUT_OF_CPU_MEM;
    }

    ezbPpConfigPtr->numOfMacPorts = numOfMacs;

    /* if we allocated the array we must memset it to ZERO */
    cpssOsBzero((GT_VOID*)ezbPpConfigPtr->macGenInfoPtr,
                ezbPpConfigPtr->numOfMacPorts * sizeof(EZB_XML_MAC_INFO_STC));

    for(macNum = 0 ; macNum < numOfMacs ; macNum++)
    {
        if(0 == CPSS_PORTS_BMP_IS_PORT_SET_MAC(existingPortPtr , macNum))
        {
            /* the MAC not valid in this device */
            continue;
        }

        /* we not care if the function find the MAC of not */
        /* as not all the macs are needed                  */
        (void)ezbMacAttributesGet(xmlDevNum,devNum,macNum,
            (EZB_XML_MAC_INFO_STC*)&ezbPpConfigPtr->macGenInfoPtr[macNum]);
    }

    return GT_OK;
}

/**
* @internal cpssAppPlatformEzbDevInfoSerdesGet function
* @endinternal
*
* @brief  get EZ_BRINGUP serdes related info related to the specific device
*
* @param[in] devNum                - device number of the PP
* @param[in] ppMapPtr              - pointer to PP Board params.
*
* @retval
* @retval GT_OK                    - on success,
* @retval GT_NOT_INITIALIZED       - the XML was not loaded.
* @retval GT_NOT_FOUND             - the devNum not exists in the xml.
* @retval GT_NOT_SUPPORTED         - the XML not hold port mapping info for the device
* @retval GT_ALREADY_EXIST         - the DB appDemoPpConfigList[].macGenInfoPtr
*                                    already exists (assuming already initialized)
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS cpssAppPlatformEzbDevInfoSerdesGet
(
    IN  GT_U32                                      devNum,
    IN  CPSS_APP_PLATFORM_BOARD_PARAM_PP_MAP_STC   *ppMapPtr
)
{
    GT_U32                      xmlDevNum;  /* the devNum as appear in the XML */
    GT_U32                      serdes;     /* serdes iterator */
    EZB_PP_CONFIG              *ezbPpConfigPtr;

    CPSS_NULL_PTR_CHECK_MAC(ppMapPtr);
    ezbPpConfigPtr = &ezbPpConfigList[devNum];
    xmlDevNum = devNum;
    if(ezbPpConfigPtr->serdesGenInfoPtr)
    {
        return GT_ALREADY_EXIST;
    }

    /****************************/
    /* handle 'per SERDES' info */
    /****************************/
    if(ezbPpConfigPtr->serdesGenInfoPtr == NULL ||
       ezbPpConfigPtr->numOfSerdeses    == 0)
    {
        ezbPpConfigPtr->numOfSerdeses = prvCpssDxChHwInitNumOfSerdesGet(devNum) ;
        ezbPpConfigPtr->serdesGenInfoPtr = cpssOsMalloc(ezbPpConfigPtr->numOfSerdeses * sizeof(EZB_XML_SERDES_INFO_STC));
        if(ezbPpConfigPtr->serdesGenInfoPtr == NULL)
        {
            /* malloc failed */
            CPSS_APP_PLATFORM_LOG_DBG_MAC("malloc failed");
            return GT_OUT_OF_CPU_MEM;
        }
        /* if we allocated the array we must memset it to ZERO */
        cpssOsBzero((GT_VOID*)ezbPpConfigPtr->serdesGenInfoPtr,
                    ezbPpConfigPtr->numOfSerdeses * sizeof(EZB_XML_SERDES_INFO_STC));
    }

    for(serdes = 0 ; serdes < ezbPpConfigPtr->numOfSerdeses ; serdes++)
    {
        /* we not care if the function find the SERDES of not */
        /* as not all the SERDES are needed for this info     */
        (void)ezbSerdesAttributesGet(xmlDevNum,serdes,
            (EZB_XML_SERDES_INFO_STC*)&ezbPpConfigPtr->serdesGenInfoPtr[serdes]);
    }
    return GT_OK;
}

/**
* @internal cpssAppPlatformEzbSerdesPolarityGet function
* @endinternal
*
* @brief  get EZ_BRINGUP serdes polarity info related to the specific device and update the profile with the data
*
* @param[in] devNum                - device number of the PP
* @param[in] ppProfilePtr          - pointer to PP Platform Profile pointer
*
* @retval
* @retval GT_OK                    - on success,
* @retval GT_NOT_INITIALIZED       - the XML was not loaded.
* @retval GT_NOT_FOUND             - the devNum not exists in the xml.
* @retval GT_NOT_SUPPORTED         - the XML not hold port mapping info for the device
* @retval GT_ALREADY_EXIST         - the DB appDemoPpConfigList[].macGenInfoPtr
*                                    already exists (assuming already initialized)
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS cpssAppPlatformEzbSerdesPolarityGet
(
    IN  GT_U32                             devNum,
    IN  CPSS_APP_PLATFORM_PP_PROFILE_STC  *ppProfilePtr
)
{
    GT_U32 jj;
    EZB_PP_CONFIG *ezbPpConfigPtr;
    GT_STATUS rc = GT_OK;
    GT_U32   polarityArrSize = 0;
    static CPSS_APP_PLATFORM_SERDES_LANE_POLARITY_STC polarityArrFromEzbXml[CPSS_MAX_PORTS_NUM_CNS];

    CPSS_NULL_PTR_CHECK_MAC(ppProfilePtr);

    ezbPpConfigPtr = &ezbPpConfigList[devNum];

    cpssOsPrintf("XML : serdes polarity info : \n");
    cpssOsPrintf("      SERDES   |  TX polarity |   RX polarity \n");
    cpssOsPrintf("==============================================\n");

    ppProfilePtr->polarityPtr = &polarityArrFromEzbXml[0];
    for(jj = 0 ; jj < ezbPpConfigPtr->numOfSerdeses ; jj++)
    {
        if(ezbPpConfigPtr->serdesGenInfoPtr[jj].polarityValid == GT_FALSE)
        {
            continue;
        }

        cpssOsPrintf("        [%3.3d]   |   %s      |   %s  \n"
            ,ezbPpConfigPtr->serdesGenInfoPtr[jj].polarity.laneNum
            ,ezbPpConfigPtr->serdesGenInfoPtr[jj].polarity.invertTx == GT_TRUE ? "true ":"false"
            ,ezbPpConfigPtr->serdesGenInfoPtr[jj].polarity.invertRx == GT_TRUE ? "true ":"false"
            );
        ppProfilePtr->polarityPtr[jj].laneNum = ezbPpConfigPtr->serdesGenInfoPtr[jj].polarity.laneNum;
        ppProfilePtr->polarityPtr[jj].invertTx = ezbPpConfigPtr->serdesGenInfoPtr[jj].polarity.invertTx;
        ppProfilePtr->polarityPtr[jj].invertRx = ezbPpConfigPtr->serdesGenInfoPtr[jj].polarity.invertRx;
        polarityArrSize++;
    }
    ppProfilePtr->polarityArrSize = polarityArrSize;
    if(ezbPpConfigPtr->numOfSerdeses)
    {
        cpssOsPrintf(" -- EZ_BRINGUP : The 'SERDES lane swaping' from XML file [%s] \n",
            ezbXmlName());
    }
    else
    {
        cpssOsPrintf(" -- EZ_BRINGUP : no 'SERDES lane swaping' (polarity invert) defined \n");
    }
    return rc;
}

/**
* @internal cpssAppPlatformEzbSerdesMuxSet function
* @endinternal
*
* @brief  Set EZ_BRINGUP serdes mux info related to the specific device
*
* @param[in] devNum                - device number of the PP
*
* @retval
* @retval GT_OK                    - on success,
* @retval GT_NOT_INITIALIZED       - the XML was not loaded.
* @retval GT_NOT_FOUND             - the devNum not exists in the xml.
* @retval GT_NOT_SUPPORTED         - the XML not hold port mapping info for the device
* @retval GT_ALREADY_EXIST         - the DB appDemoPpConfigList[].macGenInfoPtr
*                                    already exists (assuming already initialized)
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS cpssAppPlatformEzbSerdesMuxSet
(
    IN  GT_U32 devNum
)
{
    GT_U32 ii,jj;
    GT_STATUS rc;
    EZB_PP_CONFIG *ezbPpConfigPtr;
    EZB_XML_MAC_INFO_STC   *macGenInfoPtr;

    ezbPpConfigPtr = &ezbPpConfigList[devNum];
    macGenInfoPtr = (EZB_XML_MAC_INFO_STC*)ezbPpConfigPtr->macGenInfoPtr;

    cpssOsPrintf("XML : mac-to-serdes info : \n");
    cpssOsPrintf("      mac      |      local SERDESes  \n");
    cpssOsPrintf("======================================\n");

    for(ii = 0 ; ii < ezbPpConfigPtr->numOfMacPorts; ii++,macGenInfoPtr++)
    {
        if(macGenInfoPtr->isMacUsed == GT_FALSE ||
           macGenInfoPtr->isSerdesInfoValid == GT_FALSE ||
           macGenInfoPtr->macSerdesInfo.firstMacPtr == GT_FALSE ||
           macGenInfoPtr->macSerdesInfo.holdSerdesMuxingInfo == GT_FALSE)
        {
            continue;
        }

        cpssOsPrintf("      [%3.3d]   |   [%d]",
            macGenInfoPtr->macNum,
            macGenInfoPtr->macSerdesInfo.macToSerdesMap.serdesLanes[0]);

        for(jj = 1 ; jj < macGenInfoPtr->macSerdesInfo.numOfSerdeses; jj++)
        {
            cpssOsPrintf(",[%d]",
                macGenInfoPtr->macSerdesInfo.macToSerdesMap.serdesLanes[jj]);
        }
        cpssOsPrintf("\n");

        rc = cpssDxChPortLaneMacToSerdesMuxSet(CAST_SW_DEVNUM(devNum),
                    macGenInfoPtr->macSerdesInfo.firstMacPtr->physicalPort,/* The physical port that represent the 'group' */
                    &macGenInfoPtr->macSerdesInfo.macToSerdesMap);         /* The array of the lane swaps */
        if (rc != GT_OK &&
            rc != GT_NOT_SUPPORTED)/* HWS not supports the device yet 'mvHwsPortLaneMacToSerdesMuxSet' */
        {
            return rc;
        }
        else if(rc == GT_NOT_SUPPORTED)
        {
            cpssOsPrintf("WARNING : cpssDxChPortLaneMacToSerdesMuxSet not supported yet \n");
            /* no need for all ports to fail */
            break;
        }
    }
    return GT_OK;

}



/**
* @internal cpssAppPlatformEzbFree function
* @endinternal
*
* @brief  API to free momory allocated for MAC and SERDES variables
*
* @retval
* @retval GT_OK                    - on success,
* @retval GT_NOT_INITIALIZED       - the XML was not loaded.
* @retval GT_NOT_FOUND             - the devNum not exists in the xml.
* @retval GT_NOT_SUPPORTED         - the XML not hold port mapping info for the device.
* @retval GT_ALREADY_EXIST         - the DB appDemoPpConfigList[].macGenInfoPtr
*                                    already exists (assuming already initialized)
* @retval GT_FAIL                  - otherwise.
*/
void cpssAppPlatformEzbFree
(
    IN GT_U32                             devNum
)
{
        ezbXmlDestroy();
        FREE_PTR_MAC(ezbPpConfigList[devNum].macGenInfoPtr);
        FREE_PTR_MAC(ezbPpConfigList[devNum].serdesGenInfoPtr);
}
