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
* @file cpssAppPlatformMpdAndPp.h
*
* @brief  hold common function (tools) for the board config files to use to get
*         info from the 'ez_bringup' xml
*
* @version   1
********************************************************************************
*/
#ifndef __cpssAppPlatformMpdAndPp_H
#define __cpssAppPlatformMpdAndPp_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if (defined INCLUDE_MPD) && (defined CHX_FAMILY)

/**
* @internal cpssAppPlatformEzbMpdPpPortInit function
* @endinternal
*
* @param[in] trafficEnablePtr               - pointer to traffic profile
*
* @brief  init all the potrs in all the devices to bind those that supports PHY
*   to the MPD (PHY driver) and set all port to 'admin down' (not force link up)
*   (prvAppDemoEzbMpdPortInit)
*   also set the PP MAC ports with : SMI addresses and 'inband AN' with the phy
*
*/
GT_STATUS cpssAppPlatformEzbMpdPpPortInit
(
    IN CPSS_APP_PLATFORM_TRAFFIC_ENABLE_STC *trafficEnablePtr
);

/**
* @internal cpssAppPlatformEzbSwitchMacAutoNegCompleted function
* @endinternal
*
* @brief  notify that got event of 'CPSS_PP_PORT_AN_COMPLETED_E' from the port in the PP
*       and need to synch the PP with the MPD info (with the PHY)
*
* @param[in] devNum               - the cpss SW devNum
* @param[in] portNum              - the physical port number
*
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on fail,
*/
GT_STATUS cpssAppPlatformEzbSwitchMacAutoNegCompleted
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum
);

/**
* @internal cpssAppPlatformEzbPortSpeedSet function
* @endinternal
*
* @brief  Power, Set duplex mode and Set the provided speed of the PHY
*
* @param[in] trafficEnablePtr     - pointer to Traffic profile.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - on fail,
*/
GT_STATUS cpssAppPlatformEzbPortSpeedSet
(
    IN CPSS_APP_PLATFORM_TRAFFIC_ENABLE_STC *trafficEnablePtr
);
#else /*INCLUDE_MPD*/
/* implement MACROS STUBs for compilation */
#define cpssAppPlatformEzbMpdPpPortInit(a)            		GT_NOT_IMPLEMENTED
#define cpssAppPlatformEzbSwitchMacAutoNegCompleted(a,b)	GT_NOT_IMPLEMENTED
#define cpssAppPlatformEzbPortSpeedSet(a)          			GT_NOT_IMPLEMENTED
#endif
#ifdef __cplusplus
}
#endif  /* __cplusplus */
#endif
