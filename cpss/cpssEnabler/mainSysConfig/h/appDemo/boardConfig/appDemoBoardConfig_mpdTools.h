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
* @file appDemoBoardConfig_mpdTools.h
*
* @brief  hold common function (tools) for the board config files to use to get
*   MPD functions to use. NOTE: EZ_BRINGUP also exists.
*
* @version   1
********************************************************************************
*/
#ifndef __appDemoBoardConfig_mpdTools_H
#define __appDemoBoardConfig_mpdTools_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <mpdTools.h>

#if (defined INCLUDE_MPD) && (defined CHX_FAMILY)

#include <appDemo/boardConfig/appDemoBoardConfig.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>

/**
* @internal prvAppDemoEzbMpdGetNextFreeGlobalIfIndex function
* @endinternal
*
* @brief  allocate EZ_BRINGUP MPD ifIndex to : {devNum,macNum}
*         if already exists , return the existing one
* @param[in] devNum            - the cpss SW devNum
* @param[in] macNum            - the mac number
*
* @retval
* @retval get new MPD ifIndex to the port
*/
GT_U32 prvAppDemoEzbMpdGetNextFreeGlobalIfIndex(
    IN GT_U8  devNum,
    IN GT_U32 macNum
);


/**
* @internal prvAppDemoEzbMpdInitHw function
* @endinternal
*
* @brief   EZ_BRINGUP init the DB of PHY in the MPD
*           call this one only after prvAppDemoEzbMpdPortInit(...) called  an all ports in all devices
*
* @retval
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on fail,
*/
GT_STATUS prvAppDemoEzbMpdInitHw
(
    void
);

/**
* @internal appDemoEzbMpdDevMacToIfIndexConvert function
* @endinternal
*
* @brief  convert EZ_BRINGUP {devNum,macNum} : MPD ifIndex
*
* @param[in] devNum               - the cpss SW devNum
* @param[in] macNum               - the mac number
*
* @param[out] ifIndexPtr           - (pointer to) the ifIndex of MPD for a port
*
* @retval
* @retval GT_OK                    - on success,
* @retval GT_NOT_FOUND             - the {devNum,macNum} not found to have ifIndex.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS appDemoEzbMpdDevMacToIfIndexConvert
(
    IN GT_U32 devNum,
    IN GT_U32 macNum,
    OUT GT_U32 *ifIndexPtr
);

/**
* @internal prvAppDemoEzbMpdPortInit function
* @endinternal
*
* @brief   EZ_BRINGUP init the DB of PHY in the MPD
*           after done an all ports in all devices need to call to : appDemoEzbMpdInitHw()
*
* @param[in] devNum               - the cpss SW devNum
* @param[in] macNum               - the mac number
*
* @param[out] isInitDonePtr       - (pointer to) is the port hold phy and did init
*
* @retval
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on fail,
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS prvAppDemoEzbMpdPortInit
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_U32                   macPort,
    OUT GT_BOOL                 *isInitDonePtr
);

#else /*!INCLUDE_MPD*/
/* implement MACROS STUBs for compilation */
#define prvAppDemoEzbMpdGetNextFreeGlobalIfIndex(a,b)  	GT_NOT_IMPLEMENTED
#define prvAppDemoEzbMpdInitHw()      					GT_NOT_IMPLEMENTED
#define prvAppDemoEzbMpdPortInit(a,b,c) 				GT_NOT_IMPLEMENTED; (*c)=0;

#endif /*!INCLUDE_MPD*/

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /*__appDemoBoardConfig_mpdTools_H*/



