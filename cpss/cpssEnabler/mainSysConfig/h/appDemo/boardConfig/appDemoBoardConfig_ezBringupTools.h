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
* @file appDemoBoardConfig_ezBringupTools.h
*
* @brief  hold common function (tools) for the board config files to use to get
*         info from the 'ez_bringup' xml
*
* @version   1
********************************************************************************
*/
#ifndef __appDemoBoardConfig_ezBringupTools_H
#define __appDemoBoardConfig_ezBringupTools_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <ezBringupTools.h>
#include <appDemo/boardConfig/appDemoBoardConfig.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>

#if (defined EZ_BRINGUP) && (defined CHX_FAMILY)
#include <appDemo/boardConfig/gtDbDxBobcat2Mappings.h> /*needed for PortInitList_STC */
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortMapping.h>
#include <appDemo/boardConfig/gtDbDxBobcat2Mappings.h> /*needed for PortInitList_STC */
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
);

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
* @param[in] numCpuSdmaPorts       - The number of CPU SDMA ports that the array of cpuSdmaPortMapArr[] hold.
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
);

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
);

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
);


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
);

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
);


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
);

#else /*!EZ_BRINGUP*/
/* implement MACROS STUBs for compilation */
#define appDemoEzbIsXmlWithDevNum(a)                    GT_FALSE
#define appDemoEzbSip6PortMappingGet(a,b,c,d,e,f)       GT_NOT_IMPLEMENTED; a=a;(*d)=0;e=e;
#define appDemoEzbPortsSpeedGet(a,b,c,d)                GT_NOT_IMPLEMENTED; a=a;(*d)=0;
#define appDemoEzbPortsSpeedAndModeUpdate(a,b)          GT_NOT_IMPLEMENTED; b[0]=b[0];
#define appDemoEzbCpssDevNumToAppDemoIndexConvert(a,b)  GT_NOT_IMPLEMENTED; (*b)=0;
#define appDemoEzbFree(a)                               (void)a

#endif /*!EZ_BRINGUP*/

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /*__appDemoBoardConfig_ezBringupTools_H*/



