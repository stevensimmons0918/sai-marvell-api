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
* @file cpssAppPlatformEzBringupTools.h
*
* @brief  hold common function (tools) for the board config files to use to get
*         info from the 'ez_bringup' xml
*
* @version   1
********************************************************************************
*/
#ifndef __cpssAppPlatformEzBringupTools_H
#define __cpssAppPlatformEzBringupTools_H
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <ezBringupTools.h>


typedef struct
{
    GT_U32                               portNum;
    CPSS_PORT_SPEED_ENT                  speed;
    CPSS_PORT_INTERFACE_MODE_ENT         interfaceMode;
    CPSS_PORT_FEC_MODE_ENT               fecMode;
} PortInitInternal_STC;

typedef enum
{
    PORT_LIST_TYPE_EMPTY = 0,
    PORT_LIST_TYPE_INTERVAL,
    PORT_LIST_TYPE_LIST,
}PortListType_ENT;

typedef struct
{
    PortListType_ENT             entryType;
    GT_PHYSICAL_PORT_NUM         portList[32];  /* depending on list type */
    /* interval : 0 startPort     */
    /*            1 stopPort      */
    /*            2 step          */
    /*            3 APP_INV_PORT_CNS */
    /* list     : 0...x  ports     */
    /*            APP_INV_PORT_CNS */
    CPSS_PORT_SPEED_ENT          speed;
    CPSS_PORT_INTERFACE_MODE_ENT interfaceMode;
}PortInitList_STC;
#if (defined EZ_BRINGUP) && (defined CHX_FAMILY)

#include <cpss/dxCh/dxChxGen/port/cpssDxChPortMapping.h>
#include <cpssAppPlatform/profiles/cpssAppPlatformRunTimeProfile.h>
#include <cpssAppPlatform/cpssAppPlatformPpUtils.h>
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
);

/**
* @internal cpssAppPlatformEzbPortMapGet function
* @endinternal
*
* @brief  SIP6 devices : get EZ_BRINGUP info related to the port mapping of specific device
*
* @param[in] devNum        			- device number
* @param[in] ppMapPtr           	- pointer to PP Board params
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
);

/**
* @internal cpssAppPlatformEzbPortsSpeedGet function
* @endinternal
*
* @brief  get EZ_BRINGUP info related to the {speed,ifMode} of ports of specific device
*
* @param[in] devNum        			- device number
* @param[in] ppMapPtr           	- pointer to PP Board params
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
);

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
);

/**
* @internal cpssAppPlatformEzbDevInfoGet function
* @endinternal
*
* @brief  get EZ_BRINGUP info related to the specific device
*
* @param[in] devNum        		   - device number of the PP
* @param[in] ppMapPtr        	   - pointer to PP Board params.
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
);

/**
* @internal cpssAppPlatformEzbDevInfoSerdesGet function
* @endinternal
*
* @brief  get EZ_BRINGUP serdes related info related to the specific device
*
* @param[in] devNum        		   - device number of the PP
* @param[in] ppMapPtr        	   - pointer to PP Board params.
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
);

/**
* @internal cpssAppPlatformEzbSerdesPolarityGet function
* @endinternal
*
* @brief  get EZ_BRINGUP serdes polarity info related to the specific device and update the profile with the data
*
* @param[in] devNum        		   - device number of the PP
* @param[in] ppProfilePtr      	   - pointer to PP Platform Profile pointer
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
);

/**
* @internal cpssAppPlatformEzbSerdesMuxSet function
* @endinternal
*
* @brief  Set EZ_BRINGUP serdes mux info related to the specific device
*
* @param[in] devNum        		   - device number of the PP
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
    IN  GT_U32                             devNum
);

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
    IN  CPSS_APP_PLATFORM_PROFILE_STC *inputProfileListPtr
);

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
    IN GT_U32           devNum
);
#else /*EZ_BRINGUP*/
/* implement MACROS STUBs for compilation */
#define cpssAppPlatformEzbIsXmlWithDevNum(a)                GT_FALSE
#define cpssAppPlatformEzbPortMapGet(a,b,c)       		    GT_NOT_IMPLEMENTED; a=a;(*b)=0;(*c)=0;
#define cpssAppPlatformEzbPortsSpeedGet(a,b,c,d)            GT_NOT_IMPLEMENTED; a=a;(*b)=0;(*c)=0;
#define cpssAppPlatformEzbPortsSpeedAndModeUpdate(a,b,c)    GT_NOT_IMPLEMENTED; b[0]=b[0],c[0]=c[0];
#define cpssAppPlatformEzbDevInfoGet(a,b)			        GT_NOT_IMPLEMENTED; (*b)=0;
#define cpssAppPlatformEzbDevInfoSerdesGet(a,b)			    GT_NOT_IMPLEMENTED
#define cpssAppPlatformEzbSerdesPolarityGet(a,b)		    GT_NOT_IMPLEMENTED;
#define cpssAppPlatformEzbSerdesMuxSet(a)				    GT_NOT_IMPLEMENTED
#define cpssAppPlatformEzbUpdateProfile(b)				    GT_NOT_IMPLEMENTED
#define cpssAppPlatformEzbFree(a)                           (void)a
#endif
#ifdef __cplusplus
}
#endif  /* __cplusplus */
#endif /*__cpssAppPlatformEzBringupTools_H*/



