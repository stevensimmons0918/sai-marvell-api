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
* @file appDemoBoardConfig_ezBringupTools.c
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

#include <appDemo/boardConfig/appDemoBoardConfig_ezBringupTools.h>
#include <appDemo/boardConfig/appDemoBoardConfig_mpdTools.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfoEnhanced.h>

#include <cpss/common/smi/cpssGenSmi.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#if (defined ASIC_SIMULATION_ENV_FORBIDDEN && defined ASIC_SIMULATION)
    /* 'Forbid' the code to run as ASIC_SIMULATION ... we need 'like' HW compilation ! */
    #undef ASIC_SIMULATION
#endif

#ifdef ASIC_SIMULATION
    #include <asicSimulation/SEmbedded/simFS.h>
#endif /*ASIC_SIMULATION*/


#include <pdl/init/pdlInit.h>
#include <pdl/cpu/pdlCpu.h>
#include <pdl/packet_processor/pdlPacketProcessor.h>
#include <pdl/serdes/pdlSerdes.h>
#include <iDbgPdl/init/iDbgPdlInit.h>

/**
* @internal appDemoEzbCpssDevNumToAppDemoIndexConvert function
* @endinternal
*
* @brief   EZ_BRINGUP convert CPSS devNum to appDemo DB index (index to appDemoPpConfigList[])
*
* @param[in] devNum               - the cpss SW devNum
*
* @param[out] appDemoDbIndexPtr   - index in the appDemo DB : appDemoPpConfigList[]
*
* @retval
* @retval GT_OK                    - on success,
* @retval GT_NOT_FOUND             - the devNum not exists in the appDemoPpConfigList[]
*/
GT_STATUS   appDemoEzbCpssDevNumToAppDemoIndexConvert(
    IN GT_U32    devNum ,
    OUT GT_U32  *appDemoDbIndexPtr
)
{
    GT_U32 devIdx;
    APP_DEMO_PP_CONFIG *appDemoPpConfigPtr;
    for (devIdx = SYSTEM_DEV_NUM_MAC(0);
         devIdx < SYSTEM_DEV_NUM_MAC(appDemoPpConfigDevAmount);
         devIdx++)
    {
        appDemoPpConfigPtr = &appDemoPpConfigList[devIdx];
        if(appDemoPpConfigPtr->valid == GT_TRUE &&
           appDemoPpConfigPtr->devNum == devNum)
        {
            *appDemoDbIndexPtr = devIdx;
            return GT_OK;
        }
    }

    return GT_NOT_FOUND;
}

extern GT_U32   prvCpssDxChHwInitNumOfSerdesGet
(
    IN GT_U8 devNum
);



/**
* @internal appDemoEzbIsXmlWithDevNum function
* @endinternal
*
* @brief  is XML of EZ_BRINGUP hold info about the device.
*
*
* @retval GT_TRUE / GT_FALSE
*/
GT_BOOL appDemoEzbIsXmlWithDevNum(
    IN GT_U32  devNum
)
{
    PDL_STATUS        rc;
    PDL_PP_XML_ATTRIBUTES_STC ppAttributes;
    GT_U32  xmlDevNum;  /* the devNum as appear in the XML */
    GT_U32  appDemoDbIndex;

    if(GT_OK != appDemoEzbCpssDevNumToAppDemoIndexConvert(devNum,&appDemoDbIndex))
    {
        return GT_FALSE;
    }

    xmlDevNum = appDemoDbIndex;

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
* @internal appDemoEzbDevInfoGet function
* @endinternal
*
* @brief  get EZ_BRINGUP info related to the specific device
*
* @param[in] appDemoDbIndex        - index in the appDemo DB : appDemoPpConfigList[]
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
GT_STATUS appDemoEzbDevInfoGet
(
    IN GT_U32       appDemoDbIndex
)
{
    GT_U8   devNum;     /* the cpss SW devNum              */
    GT_U32  xmlDevNum;  /* the devNum as appear in the XML */
    GT_U32  macNum;     /* the MAC number of the ports to get info from XML ... if exists */
    GT_U32  numOfMacs;  /* max number of MACs in the device */
    GT_U32  serdes;     /* serdes iterator */
    CPSS_PORTS_BMP_STC *existingPortsPtr;
    EZB_PP_CONFIG *ezbPpConfigPtr;

    ezbPpConfigPtr = &ezbPpConfigList[appDemoDbIndex];
    devNum    = appDemoPpConfigList[appDemoDbIndex].devNum;
    xmlDevNum = appDemoDbIndex;
    existingPortsPtr = &(PRV_CPSS_PP_MAC(devNum)->existingPorts);
    numOfMacs = PRV_CPSS_PP_MAC(devNum)->numOfPorts;

    if(GT_FALSE == ezbIsXmlLoaded())
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoEzbIsXmlLoaded", GT_NOT_INITIALIZED);
        return GT_NOT_INITIALIZED;
    }

    if(GT_FALSE == appDemoEzbIsXmlWithDevNum(devNum))
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoEzbIsXmlLoaded", GT_NOT_FOUND);
        return GT_NOT_FOUND;
    }

    if(ezbPpConfigPtr->macGenInfoPtr)
    {
        return GT_ALREADY_EXIST;
    }

    /*************************/
    /* handle 'per MAC' info */
    /*************************/

    ezbPpConfigPtr->macGenInfoPtr = cpssOsMalloc(numOfMacs * sizeof(EZB_GEN_MAC_INFO_STC));
    if(ezbPpConfigPtr->macGenInfoPtr == NULL)
    {
        /* malloc failed */
        CPSS_ENABLER_DBG_TRACE_RC_MAC("malloc failed", GT_OUT_OF_CPU_MEM);
        return GT_OUT_OF_CPU_MEM;
    }

    ezbPpConfigPtr->numOfMacPorts = numOfMacs;

    /* if we allocated the array we must memset it to ZERO */
    cpssOsBzero((GT_VOID*)ezbPpConfigPtr->macGenInfoPtr,
                ezbPpConfigPtr->numOfMacPorts * sizeof(EZB_GEN_MAC_INFO_STC));

    for(macNum = 0 ; macNum < numOfMacs ; macNum++)
    {
        if(0 == CPSS_PORTS_BMP_IS_PORT_SET_MAC(existingPortsPtr , macNum))
        {
            /* the MAC not valid in this device */
            continue;
        }

        /* we not care if the function find the MAC of not */
        /* as not all the macs are needed                  */
        (void)ezbMacAttributesGet(xmlDevNum,devNum,macNum,
            (EZB_XML_MAC_INFO_STC*)(&ezbPpConfigPtr->macGenInfoPtr[macNum]));
    }

    /****************************/
    /* handle 'per SERDES' info */
    /****************************/

    if(ezbPpConfigPtr->serdesGenInfoPtr == NULL ||
       ezbPpConfigPtr->numOfSerdeses    == 0)
    {
        ezbPpConfigPtr->numOfSerdeses = prvCpssDxChHwInitNumOfSerdesGet(devNum) ;
        ezbPpConfigPtr->serdesGenInfoPtr = cpssOsMalloc(ezbPpConfigPtr->numOfSerdeses * sizeof(EZB_GEN_SERDES_INFO_STC));
        if(ezbPpConfigPtr->serdesGenInfoPtr == NULL)
        {
            /* malloc failed */
            CPSS_ENABLER_DBG_TRACE_RC_MAC("malloc failed", GT_OUT_OF_CPU_MEM);
            return GT_OUT_OF_CPU_MEM;
        }
        /* if we allocated the array we must memset it to ZERO */
        cpssOsBzero((GT_VOID*)ezbPpConfigPtr->serdesGenInfoPtr,
                    ezbPpConfigPtr->numOfSerdeses * sizeof(EZB_GEN_SERDES_INFO_STC));
    }

    for(serdes = 0 ; serdes < ezbPpConfigPtr->numOfSerdeses ; serdes++)
    {
        /* we not care if the function find the SERDES of not */
        /* as not all the SERDES are needed for this info     */
        (void)ezbSerdesAttributesGet(xmlDevNum,serdes,
            (EZB_XML_SERDES_INFO_STC*)(&ezbPpConfigPtr->serdesGenInfoPtr[serdes]));
    }


    return GT_OK;
}

/**
* @internal addCpuSdmaPortMapping function
* @endinternal
*
* @brief  fill CPU ports info to the port mapping
*
* @param[in] cpssDevNum            - The CPSS SW device number.
* @param[in] maxNumPorts           - The number of CPU SDMA ports that the array of cpuSdmaPortMapArr[] hold.
* @param[in] cpuSdmaPortMapArr     - The array that hold the CPU SDMA ports.
*                                    NOTE: physicalPortNumber = GT_NA means that any non-used number is ok
*                                       one of those port MUST hold physicalPortNumber = 63
*                                       interfaceNum is mandatory with valid number
* @param[out] portMapArr           - The array to fill with info.
* @param[out] actualNumOfPortsPtr  - (pointer to) The actual number of ports that the XML hold for the device.
*                                  - if the maxNumPorts < (*actualNumOfPortsPtr) than only maxNumPorts are filled
* @retval GT_OK                    - on success,
* @retval GT_NOT_INITIALIZED       - the XML was not loaded.
* @retval GT_NOT_FOUND             - the devNum not exists in the xml.
* @retval GT_NOT_SUPPORTED         - the XML not hold port mapping info for the device.
* @retval GT_FULL                  - the logic could not find physical port for CPU SDMA port with GT_NA.
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS addCpuSdmaPortMapping(
    IN GT_U8                  cpssDevNum,
    IN GT_U32                 numCpuSdmaPorts,
    IN CPSS_DXCH_PORT_MAP_STC cpuSdmaPortMapArr[],
    INOUT CPSS_DXCH_PORT_MAP_STC portMapArr[],
    INOUT GT_U32            *actualNumOfPortsPtr
)
{
    PDL_STATUS                      pdlStatus;
    CPSS_DXCH_PORT_MAP_STC *currPortInfoPtr;
    GT_U32 ii = 0,index_target;
    PDL_CPU_SDMA_MAP_INFO_STC       cpuSdmaMapInfo;

    index_target = *actualNumOfPortsPtr;
    (void)cpssDevNum;
    (void)numCpuSdmaPorts;
    (void)cpuSdmaPortMapArr;

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
* @internal appDemoEzbSip6PortMappingGet function
* @endinternal
*
* @brief  SIP6 devices : get EZ_BRINGUP info related to the port mapping of specific device
*
* @param[in] appDemoDbIndex        - index in the appDemo DB : appDemoPpConfigList[]
* @param[in] maxNumPorts           - The max number of ports that the array of portMapArr[] can hold.
* @param[out] portMapArr           - The array to fill with info.
* @param[out] actualNumOfPortsPtr  - (pointer to) The actual number of ports that the XML hold for the device.
*                                  - if the maxNumPorts < (*actualNumOfPortsPtr) than only maxNumPorts are filled
* @param[in] maxNumPorts           - The number of CPU SDMA ports that the array of cpuSdmaPortMapArr[] hold.
* @param[in] cpuSdmaPortMapArr     - The array that hold the CPU SDMA ports.
*                                    NOTE: physicalPortNumber = GT_NA means that any non-used number is ok
*                                    one of those port MUST hold physicalPortNumber = 63
*                                    interfaceNum is mandatory with valid number
* @retval GT_OK                    - on success,
* @retval GT_NOT_INITIALIZED       - the XML was not loaded.
* @retval GT_NOT_FOUND             - the devNum not exists in the xml.
* @retval GT_NOT_SUPPORTED         - the XML not hold port mapping info for the device.
* @retval GT_FULL                  - the logic could not find physical port for CPU SDMA port with GT_NA.
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appDemoEzbSip6PortMappingGet(
    IN GT_U32           appDemoDbIndex,
    IN GT_U32           maxNumPorts,
    OUT CPSS_DXCH_PORT_MAP_STC portMapArr[],
    OUT GT_U32          *actualNumOfPortsPtr,
    IN GT_U32           numCpuSdmaPorts,
    IN CPSS_DXCH_PORT_MAP_STC cpuSdmaPortMapArr[]
)
{
    GT_STATUS  st;
    CPSS_DXCH_PORT_MAP_STC *currPortInfoPtr;
    CPSS_DXCH_PORT_MAP_STC zeroInfo;
    GT_U32        ii,jj;
    GT_U8   cpssDevNum; /* the cpss SW devNum              */
    EZB_PP_CONFIG *ezbPpConfigPtr;
    EZB_GEN_MAC_INFO_STC   *macInfoPtr;

    ezbPpConfigPtr = &ezbPpConfigList[appDemoDbIndex];
    cpssDevNum = appDemoPpConfigList[appDemoDbIndex].devNum;

    if(maxNumPorts != 0)
    {
        CPSS_NULL_PTR_CHECK_MAC(portMapArr);
        currPortInfoPtr = &portMapArr[0];
    }
    else
    {
        currPortInfoPtr = &zeroInfo;
    }

    CPSS_NULL_PTR_CHECK_MAC(actualNumOfPortsPtr);

    cpssOsBzero((GT_CHAR *)&zeroInfo,sizeof(CPSS_DXCH_PORT_MAP_STC));
    if(GT_FALSE == ezbIsXmlLoaded())
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoEzbIsXmlLoaded", GT_NOT_INITIALIZED);
        return GT_NOT_INITIALIZED;
    }

    if(GT_FALSE == appDemoEzbIsXmlWithDevNum(cpssDevNum))
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoEzbIsXmlLoaded", GT_NOT_FOUND);
        return GT_NOT_FOUND;
    }

    if(!ezbPpConfigPtr->macGenInfoPtr)
    {
        /* DB of appDemo was NOT initialized yet */
        st = appDemoEzbDevInfoGet(appDemoDbIndex);
        if(st != GT_OK)
        {
            return st;
        }
    }

    macInfoPtr = ezbPpConfigPtr->macGenInfoPtr;
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
        CPSS_ENABLER_DBG_TRACE_RC_MAC("pdlPpDbDevAttributesGetFirstPort", GT_NOT_SUPPORTED);
        return GT_NOT_SUPPORTED;
    }

    *actualNumOfPortsPtr = ii;

    if(maxNumPorts == 0)
    {
        *actualNumOfPortsPtr += numCpuSdmaPorts;
        return GT_OK;
    }

    return addCpuSdmaPortMapping(cpssDevNum , numCpuSdmaPorts ,cpuSdmaPortMapArr,
            portMapArr ,actualNumOfPortsPtr);
}

/**
* @internal appDemoEzbPortsSpeedGet function
* @endinternal
*
* @brief  get EZ_BRINGUP info related to the {speed,ifMode} of ports of specific device
*
* @param[in] appDemoDbIndex        - index in the appDemo DB : appDemoPpConfigList[]
* @param[in] maxEntries            - The max number of entries that the array of portsSpeedAndMode[] can hold.
*                                    NOTE: this number must include index for the 'Termination entry'
* @param[out] portsSpeedAndMode    - The array to fill with info.
* @param[out] actualNumEntriesPtr  - (pointer to) The actual number of entries that the XML hold for the device.
*                                  - if the maxEntries < (*actualNumEntriesPtr) than only maxNumPorts are filled
* @retval GT_OK                    - on success,
* @retval GT_NOT_INITIALIZED       - the XML was not loaded.
* @retval GT_NOT_FOUND             - the devNum not exists in the xml.
* @retval GT_NOT_SUPPORTED         - the XML not hold {speed,ifMode} info for the ports in the device.
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appDemoEzbPortsSpeedGet(
    IN GT_U32           appDemoDbIndex,
    IN GT_U32           maxEntries,
    OUT PortInitList_STC portsSpeedAndMode[],
    OUT GT_U32          *actualNumEntriesPtr
)
{
    GT_STATUS  st;
    PortInitList_STC *currPortInfoPtr;
    GT_U32        ii,jj;
    EZB_PP_CONFIG *ezbPpConfigPtr;
    EZB_GEN_MAC_INFO_STC   *macInfoPtr;

    ezbPpConfigPtr = &ezbPpConfigList[appDemoDbIndex];

    if(!ezbPpConfigPtr->macGenInfoPtr)
    {
        /* DB of appDemo was NOT initialized yet */
        st = appDemoEzbDevInfoGet(appDemoDbIndex);
        if(st != GT_OK)
        {
            return st;
        }
    }

    currPortInfoPtr = &portsSpeedAndMode[0];
    macInfoPtr = ezbPpConfigPtr->macGenInfoPtr;
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

        if(ii < maxEntries)
        {
            currPortInfoPtr->entryType      = PORT_LIST_TYPE_LIST;
            currPortInfoPtr->portList[0]    = macInfoPtr->physicalPort;
            currPortInfoPtr->portList[1]    = APP_INV_PORT_CNS;
            currPortInfoPtr->speed          = macInfoPtr->defaultSpeedAndIfMode.speed;
            currPortInfoPtr->interfaceMode  = macInfoPtr->defaultSpeedAndIfMode.ifMode;

            currPortInfoPtr++;
        }

        ii++;
    }

    if(ii == 0)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("no ports in DB ?!", GT_NOT_SUPPORTED);
        return GT_NOT_SUPPORTED;
    }

    ii++;/* the 'Termination entry' */
    if(ii < maxEntries)
    {
        currPortInfoPtr->entryType      = PORT_LIST_TYPE_EMPTY;
        currPortInfoPtr->portList[0]    = APP_INV_PORT_CNS;
        currPortInfoPtr->speed          = CPSS_PORT_SPEED_NA_E;
        currPortInfoPtr->interfaceMode  = CPSS_PORT_INTERFACE_MODE_NA_E;

        /*currPortInfoPtr++;*/
    }

    ii++;/* the 'Termination entry' */

    *actualNumEntriesPtr = ii;

    return GT_OK;
}

/**
* @internal appDemoEzbPortsSpeedAndModeUpdate function
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
GT_STATUS appDemoEzbPortsSpeedAndModeUpdate
(
    INOUT PortInitList_STC             portsSpeedAndMode[],
    IN EZB_XML_SPEED_IF_MODE_REPLACE_STC  replaceInfoArr[]
)
{
    PortInitList_STC *currPortInfoPtr;
    EZB_XML_SPEED_IF_MODE_REPLACE_STC *currReplaceInfoPtr;

    CPSS_NULL_PTR_CHECK_MAC(portsSpeedAndMode);
    CPSS_NULL_PTR_CHECK_MAC(replaceInfoArr);

    currPortInfoPtr = &portsSpeedAndMode[0];

    for (/*init already done*/; currPortInfoPtr->entryType != PORT_LIST_TYPE_EMPTY; currPortInfoPtr++)
    {
        currReplaceInfoPtr = &replaceInfoArr[0];
        for(/*init already done*/; currReplaceInfoPtr->old.speed != CPSS_PORT_SPEED_NA_E ; currReplaceInfoPtr++)
        {
            if(currPortInfoPtr->speed           == currReplaceInfoPtr->old.speed &&
                    currPortInfoPtr->interfaceMode   == currReplaceInfoPtr->old.ifMode )
            {
                currPortInfoPtr->speed          = currReplaceInfoPtr->new.speed;
                currPortInfoPtr->interfaceMode  = currReplaceInfoPtr->new.ifMode;
                break;
            }

            if(GT_NA                            == currReplaceInfoPtr->old.speed &&
                    currPortInfoPtr->interfaceMode   == currReplaceInfoPtr->old.ifMode )
            {
                /* modify only ifMode */
                currPortInfoPtr->interfaceMode  = currReplaceInfoPtr->new.ifMode;
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
            currPortInfoPtr->entryType = PORT_LIST_TYPE_LIST;
            currPortInfoPtr->portList[0] = GT_NA;/* kill the first in array */
        }
    }

    return GT_OK;
}

/**
* @internal appDemoEzbFree function
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
void appDemoEzbFree
(
    IN GT_U32           appDemoDbIndex
)
{
    ezbXmlDestroy();
    FREE_PTR_MAC(ezbPpConfigList[appDemoDbIndex].macGenInfoPtr);
    FREE_PTR_MAC(ezbPpConfigList[appDemoDbIndex].serdesGenInfoPtr);
}
